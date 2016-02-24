#include <Rcpp.h>
using namespace Rcpp;



// [[Rcpp::export]]
double calculate_test_error_(IntegerVector vardep, IntegerVector predicted_class_int) {
   int num_examples = vardep.size();
   int num_errors = 0;
   for(int i=0;i<num_examples;i++)
   {
     if(vardep[i]!=predicted_class_int[i])
      num_errors++;
   }
   double error = (double)num_errors/(double)num_examples;
   return error;
}
