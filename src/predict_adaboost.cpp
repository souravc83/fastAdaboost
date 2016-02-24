#include <Rcpp.h>
using namespace Rcpp;

//NumericMatrix discrete_predict_matrix(Function wrap_rpart_predict, SEXP this_tree, 
//                                      DataFrame newdata, Numericatrix pred_mat)
//{
//  NumericVector predict_class = as<NumericVector>(wrap_rpart_predict(this_tree,newdata));
//  for(int j=0;j<num_examples;j++)
//    pred_mat(j,i) = predict_class[j];
//  
//  return pred_mat;
//}

// modify the wrap_rpart_predict to get class probabilities.
//this function implements Algorithm 4, equation(c), Pg 9 in 
// Zhu et. al. "Multi Class Adaboost", 2006
//for K=2

//NumericMatrix real_predict_matrix(Function wrap_rpart_predict_real, SEXP this_tree, 
//                                      DataFrame newdata, Numericatrix pred_mat)
//{
//  //predict prob is P(class=0|x)
//  double eps= 1.e-5; //small number
//  NumericVector predict_prob = as<NumericVector>(wrap_rpart_predict_real(this_tree,newdata));
//  for(int j=0;j<num_examples;j++)
//    pred_mat[j,i] = log()
//  
//}


// [[Rcpp::export]]
List predict_adaboost_(List tree_list, NumericVector coeff_vector, 
                        DataFrame newdata, int num_examples, Function wrap_rpart_predict,
                        SEXP classnames_map)
{
  int nIter = coeff_vector.size();
  NumericMatrix pred_mat(num_examples, nIter);

  
  for(int i =0;i<nIter;i++)
  {  
    NumericVector predict_class = as<NumericVector>(wrap_rpart_predict(tree_list[i],
                                                                       newdata, classnames_map));
    for(int j=0;j<num_examples;j++)
      pred_mat(j,i) = predict_class[j];
  }
  //keep in mind multi-class
  int num_classes = 2;
  NumericMatrix final_class(num_examples, num_classes);
  int indicator = 0;
  double class_vote = 0.;
  for(int i=0;i<num_classes;i++)
  {
    
    for(int j=0;j<num_examples;j++)
    {
      class_vote = 0.;
      for(int k=0;k<nIter;k++)
      {
        if(pred_mat(j,k)== i)
          indicator = 1;
        class_vote += indicator*coeff_vector[k];
        indicator = 0; //reset indicator variable
      }
      final_class(j,i)=class_vote;
    }
  }
  
  
  NumericVector predicted_class(num_examples);
  int this_class;
  int max_val;
  for(int i=0;i<num_examples;i++)
  {
    max_val = 0.;
    for(int j=0;j<num_classes;j++)
    {
      if(final_class(i,j)>max_val)
        this_class = j;
        max_val = final_class(i,j);
    }
    predicted_class[i] = this_class;
  }
  
  NumericMatrix prob_mat(num_examples, num_classes);
  double row_sum = 0.;
  for(int i=0;i<num_examples;i++)
  {
    row_sum =0.;
    for(int j=0;j<num_classes;j++)
      row_sum+=final_class(i,j);
    
    for(int j=0;j<num_classes;j++)
      prob_mat(i,j) = final_class(i,j)/row_sum;
  }
  
  
  List predict_object;
  predict_object["votes"] = final_class;
  predict_object["class"] = predicted_class;
  predict_object["prob"] = prob_mat;
  
  return predict_object;
  
   
}
