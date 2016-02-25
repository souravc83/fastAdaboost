#include <Rcpp.h>
#include<sstream>
using namespace Rcpp;


// [[Rcpp::export]]
List call_rpart_(SEXP formula_obj, Function wrap_rpart, DataFrame newdata, 
                  NumericVector weight_vec,SEXP classname_map) {
   
   SEXP return_list = wrap_rpart(formula_obj, newdata, weight_vec,classname_map);
   List rcpp_return_list = as<List>(return_list);
   
   //NumericVector rpart_class = as<NumericVector>(rcpp_return_list["pred"]);
   
   //Rcout<<rpart_class<<std::endl;
   return rcpp_return_list;
}

//utility functions 
double calculate_error(IntegerVector dep_variable, IntegerVector tree_prediction, 
                       NumericVector weight_vec )
{
  int dep_size = dep_variable.size();
  int pred_size = tree_prediction.size();
  int weight_size = weight_vec.size();
  if(dep_size!=pred_size || pred_size!=weight_size)
    stop("three vector sizes should be the same");
  
  double error = 0.;
  for(int i=0;i<dep_size;i++)
  {
    if(dep_variable[i]!=tree_prediction[i])
      error+=weight_vec[i];
  }
  return error;
  
}

NumericVector update_weights(IntegerVector dep_variable, IntegerVector tree_prediction, 
                             NumericVector weight_vec, double alpha)
{
  int dep_size = dep_variable.size();
  int pred_size = tree_prediction.size();
  int weight_size = weight_vec.size();
  if(dep_size!=pred_size || pred_size!=weight_size)
    stop("three vector sizes should be the same");
  
  NumericVector updated_wt(clone(weight_vec));
  
  for(int i=0;i<dep_size;i++)
  {
    if(dep_variable[i]!=tree_prediction[i])
      updated_wt[i] = updated_wt[i]*exp(alpha);
  }
  double element_sum = 0.;
  for(int i=0;i<dep_size;i++)
    element_sum +=updated_wt[i];
  //Rcout<<element_sum<<std::endl;
 
  for(int i=0;i<dep_size;i++)
    updated_wt[i] = updated_wt[i]/element_sum;
  
  return updated_wt;
}

//This function can potentially become more complicated with multiclass
IntegerVector convert_factor_to_int(IntegerVector factor_vec)
{
  int vec_size = factor_vec.size();
  for(int i=0;i<vec_size;i++)
  {
    factor_vec[i] = factor_vec[i]-1;
  }
  return factor_vec;
}


//This function updates the weights after every iteration for a real
// adaboost.
// tree_real pred is prob(Y=0|X). that is how it is defined in 
//wrap rpart
//this function implements Algorithm 4, equation(d), Pg 10 in 
// Zhu et. al. "Multi Class Adaboost", 2006
//for K=2
//https://web.stanford.edu/~hastie/Papers/samme.pdf
NumericVector update_weights_real_ada(IntegerVector dep_variable, 
                                      NumericVector tree_real_pred, 
                                      NumericVector weight_vec)
{
  int num_examples = dep_variable.size();
  double eps= 1.e-5; //small number
  // first correct zeros so that log function works fine
  for(int i=0;i<num_examples;i++)
  {
    if(tree_real_pred[i]<eps)
      tree_real_pred[i]=eps;
    if( tree_real_pred[i]>(1.-eps) )
      tree_real_pred[i] = 1.-eps;
  }
  
  NumericVector updated_weight(clone(weight_vec));
  double exp_factor;
  int y_k[2] ={0,0} ;
  //can do away with y_k assignment, but used for clarity
  // and possible multi-class expanstion
  // y_k=1 if class = k, otherwise -1
  for(int i=0;i<num_examples;i++)
  {
    if(dep_variable[i]==0)
    {
      y_k[0]=1;
      y_k[1]=-1;
    }
    else
    {
      y_k[0]=-1;
      y_k[1]=1;
    }
    exp_factor = -0.5*( y_k[0]*log(tree_real_pred[i]) +
                        y_k[1]*log(1.-tree_real_pred[i]));
    updated_weight[i] = weight_vec[i]*exp(exp_factor);
  }
  //now normalize the weights
  double sum_of_wt = 0.;
  for(int i=0;i<num_examples;i++)
    sum_of_wt+=updated_weight[i];
  
  for(int i=0;i<num_examples;i++)
    updated_weight[i] = updated_weight[i]/sum_of_wt;
  
  return updated_weight;
  
  
}

//This runs a single iteration or real adaboost
//The weights are always 1 for real adaboost
List real_boost_iteration(SEXP formula_obj, DataFrame data_df, IntegerVector vardep,
                              NumericVector weight_numvec, 
                              Function wrap_rpart, SEXP classname_map)
{
  List rcpp_result = call_rpart_(formula_obj, wrap_rpart, data_df, weight_numvec, classname_map);
  SEXP this_tree = rcpp_result["tree"];
  NumericVector tree_real_pred = as<NumericVector>(rcpp_result["prob"]);
  
  //the error prediction does not change
  //simply no. of incorrect answers, in SAMME.R too
  IntegerVector tree_prediction = as<IntegerVector>(rcpp_result["pred"]);
  double err = calculate_error(vardep, tree_prediction, weight_numvec);
  
  
  
  //no update weights if error is okay
  if(err<0.5 && err!=0)
     weight_numvec =  update_weights_real_ada(vardep, tree_real_pred, weight_numvec);
  
  
  List boost_result;
  boost_result["tree"] = this_tree;
  boost_result["error"] = err;
  boost_result["weight"] = weight_numvec;
  boost_result["coeff"] = 1.; //coefficient 1 for real adaboost
  
  return boost_result;
}


//This runs one single iteration of discrete boost
// it updates the weight, calculates error and 
// finds the coefficient alpha
List discrete_boost_iteration(SEXP formula_obj, DataFrame data_df, IntegerVector vardep,
                              NumericVector weight_numvec, 
                              Function wrap_rpart, SEXP classname_map)
{
  List rcpp_result = call_rpart_(formula_obj, wrap_rpart, data_df, weight_numvec, classname_map);
  SEXP this_tree = rcpp_result["tree"];
  
  IntegerVector tree_prediction = as<IntegerVector>(rcpp_result["pred"]);
  double err = calculate_error(vardep, tree_prediction, weight_numvec);
  double alpha;
  
  if (err>0.5 || err == 0)
  {
    if( err == 0)
      alpha = 1.0;
    else
    {
      Rcout<<"Weak Learner has error greater than 0.5, exiting.."<<std::endl;
      alpha = 0.0;
    }
    
  }
  else
  {
    alpha = 0.5*log((1.-err)/err);
    weight_numvec =  update_weights(vardep, tree_prediction, weight_numvec, alpha);
  }
  
  
  List boost_result;
  boost_result["tree"] = this_tree;
  boost_result["error"] = err;
  boost_result["weight"] = weight_numvec;
  boost_result["coeff"] = alpha;
  
  return boost_result;
  
}


// [[Rcpp::export]]
List adaboost_main_loop_(SEXP formula_obj, DataFrame data_df, int nIter, Function wrap_rpart,
                         IntegerVector vardep, SEXP classname_map, std::string boost_method)
{
  //Dataframe should be doubles, checked at R upstream??
  // How about passing data_df as SEXP??
  
  int num_examples = vardep.size();
  List tree_list = List::create();
  std::vector<double> coeff_vector(nIter,0.0);
  // initialize weight vector
  double init_weight_val = 1./num_examples;
  std::vector<double> weight_vec(num_examples,init_weight_val);
  NumericVector weight_numvec(weight_vec.begin(), weight_vec.end() );
  
  //initialize to avoid creation of variables in every loop
  IntegerVector tree_prediction;
  double err;
  List boost_result;
  
  for(int i=0;i<nIter;i++)
  {
    if( boost_method == "M1")
    {
      boost_result = discrete_boost_iteration(formula_obj, data_df, vardep, weight_numvec, 
                                             wrap_rpart, classname_map);
    }
    else
    {
      boost_result = real_boost_iteration(formula_obj, data_df, vardep, weight_numvec, 
                                             wrap_rpart, classname_map);      
    }
    
    //because rcpp will not accept integers as list names
    //std::to_string() does not pass win_build, due to C++11 issues
    // hence replacing with sstream based implementation
    std::ostringstream ss;
    ss << i;
    std::string list_name =  ss.str();
    //std::string list_name = std::to_string(i);
    tree_list[list_name] = boost_result["tree"];
    
    err = boost_result["error"];
    //Rcout<<"Error:"<<err<<std::endl;
    coeff_vector[i] = boost_result["coeff"];
    if(err>=0.5 || err == 0)
    {
      coeff_vector.erase( coeff_vector.begin()+i+1,coeff_vector.end() );
      break;
    }
   
    weight_numvec =  boost_result["weight"]; 
  }

  List adaboost_list;
  adaboost_list["trees"] = tree_list;
  adaboost_list["weights"] = coeff_vector;
  
  return adaboost_list;
}

