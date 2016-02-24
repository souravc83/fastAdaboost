#include <Rcpp.h>
using namespace Rcpp;


//this function implements Algorithm 4, equation(c), Pg 9 in 
// Zhu et. al. "Multi Class Adaboost", 2006
//for K=2
//implemented so that it can easily be extended to 
// multi-class
double calculate_hk(double* prob_arr, int class_int)
{
  double eps= 1.e-12; //small number
  double this_log_prob = log(prob_arr[class_int]+eps);
  double sum_log_prob = 0.;
  for(int i=0;i<2;i++)
    sum_log_prob += log(prob_arr[i]+eps);

  double h_k = this_log_prob -0.5*sum_log_prob;//eqn(c), page 9, Zhu et al.
  return h_k;  
}




// [[Rcpp::export]]
List predict_real_adaboost_(List tree_list, NumericVector coeff_vector, 
                        DataFrame newdata, int num_examples, Function wrap_rpart_predict_real)
{
  int nIter = coeff_vector.size();
  // this gives the prob for each example, each tree that class =0
   //pred_class_prob is P(class=0|x)
  NumericMatrix pred_mat(num_examples, nIter);
  for(int i =0;i<nIter;i++)
  {  
    NumericVector predict_class_prob = as<NumericVector>(wrap_rpart_predict_real(tree_list[i],
                                                                       newdata));
    for(int j=0;j<num_examples;j++)
      pred_mat(j,i) = predict_class_prob[j];
  }
  
  // for each class, calculate C_k(x) = \sum_1^M h_k^{(m)}(x)
  int num_classes=2;
  NumericMatrix weighted_class_prob(num_examples, num_classes);
  

  for(int j=0;j<num_examples;j++)
  {
    for(int k=0;k<nIter;k++)
    {
      double prob_arr[2] ={pred_mat(j,k), 1.0 - pred_mat(j,k)};
      for(int i=0;i<num_classes;i++)
        weighted_class_prob(j,i) += calculate_hk(prob_arr,i);
    }  
  }
  
  // TODO: Refactor later. This part afterwards 
  // is the same as adaboost_M1
  
  NumericVector predicted_class(num_examples);
  
  int this_class;
  int max_val;
  for(int i=0;i<num_examples;i++)
  {
    max_val = 0.;
    for(int j=0;j<num_classes;j++)
    {
      if(weighted_class_prob(i,j)>max_val)
        this_class = j;
        max_val = weighted_class_prob(i,j);
    }
    predicted_class[i] = this_class;
  }
  
  NumericMatrix prob_mat(num_examples, num_classes);
  double row_sum = 0.;
  for(int i=0;i<num_examples;i++)
  {
    row_sum =0.;
    for(int j=0;j<num_classes;j++)
      row_sum+=weighted_class_prob(i,j);
    
    for(int j=0;j<num_classes;j++)
      prob_mat(i,j) = weighted_class_prob(i,j)/row_sum;
  }
  
  List predict_object;
  predict_object["votes"] = weighted_class_prob;
  predict_object["class"] = predicted_class;
  predict_object["prob"] = prob_mat;
  
  return predict_object;


}

