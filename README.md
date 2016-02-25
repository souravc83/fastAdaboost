

<!-- README.md is generated from README.Rmd. Please edit that file -->



# fastAdaboost 
fastAdaboost is a blazingly fast implementation of adaboost for R. It uses C++ code in the backend to provide an implementation of adaboost that is about 100 times faster than native R based libraries currently available. This is especially useful if your data size is large. fastAdaboost works only 
for binary classification tasks presently. It implements Freund and Schapire's Adaboost.M1 and 
Zhu et. al's SAMME.R (real adaboost) algorithms.

## Install
It is not submitted to CRAN yet.

```r
devtools::install_github("souravc83/fastAdaboost")
```

## Quick Demo

```r
library("fastAdaboost")

num_each <- 1000
fakedata <- data.frame( X=c(rnorm(num_each,0,1),rnorm(num_each,1.5,1)), Y=c(rep(0,num_each),rep(1,num_each) ) )
fakedata$Y <- factor(fakedata$Y)
#run adaboost
test_adaboost <- adaboost(Y~X, fakedata, 10)
#print(A)
pred <- predict( test_adaboost, newdata=fakedata)
print(paste("Adaboost Error on fakedata:",pred$error))
#> [1] "Adaboost Error on fakedata: 0.1235"
print(table(pred$class,fakedata$Y))
#>    
#>       0   1
#>   0 838  85
#>   1 162 915

test_real_adaboost <- real_adaboost(Y~X, fakedata, 10)
pred_real <- predict(test_real_adaboost,newdata=fakedata)
print(paste("Real Adaboost Error on fakedata:", pred_real$error))
#> [1] "Real Adaboost Error on fakedata: 0.1085"
print(table(pred_real$class,fakedata$Y))
#>    
#>       0   1
#>   0 918 135
#>   1  82 865
```



