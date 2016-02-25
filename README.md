

<!-- README.md is generated from README.Rmd. Please edit that file -->



# fastAdaboost [![Travis-CI Build Status](https://travis-ci.org/souravc83/fastAdaboost.svg?branch=master)](https://travis-ci.org/souravc83/fastAdaboost) 
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
set.seed(9999)

num_each <- 1000
fakedata <- data.frame( X=c(rnorm(num_each,0,1),rnorm(num_each,1.5,1)), Y=c(rep(0,num_each),rep(1,num_each) ) )
fakedata$Y <- factor(fakedata$Y)
#run adaboost
test_adaboost <- adaboost(Y~X, fakedata, 10)
#print(A)
pred <- predict( test_adaboost, newdata=fakedata)
print(paste("Adaboost Error on fakedata:",pred$error))
#> [1] "Adaboost Error on fakedata: 0.1225"
print(table(pred$class,fakedata$Y))
#>    
#>       0   1
#>   0 848  93
#>   1 152 907

test_real_adaboost <- real_adaboost(Y~X, fakedata, 10)
pred_real <- predict(test_real_adaboost,newdata=fakedata)
print(paste("Real Adaboost Error on fakedata:", pred_real$error))
#> [1] "Real Adaboost Error on fakedata: 0.1105"
print(table(pred_real$class,fakedata$Y))
#>    
#>       0   1
#>   0 906 127
#>   1  94 873
```

## Performance Benchmarking
How fast is fastAdaboost compared to native R implementations? I used the microbenchmark package to 
compare the running times of fastAdaboost with Adabag, which is one of the most popular native 
R based libraries which implements the Adaboost algorithm.
The benchmarking indicates that fastAdaboost is about ~45-50 times faster than
R based implementation. This is a huge benefit when data sizes are large.
```r
library(microbenchmark)
library(adabag)
library(MASS)

#using fastAdaboost
data(bacteria)
print(
  microbenchmark
  ( 
    boost_obj <- adaboost(y~.,bacteria , 10),
    pred <- predict(boost_obj,bacteria) 
  )
  )
#> Unit: milliseconds
#>                                        expr      min       lq    mean
#>  boost_obj <- adaboost(y ~ ., bacteria, 10) 58.01665 58.69384 60.6658
#>        pred <- predict(boost_obj, bacteria) 26.91593 27.41415 29.5689
#>    median       uq      max neval cld
#>  59.20298 60.13180 74.54155   100   b
#>  27.91902 32.50484 37.58375   100  a

#using adabag
print(
  microbenchmark
  ( 
    adabag_obj <-boosting(y~.,bacteria,boos=F,mfinal=10),
    pred_adabag <- predict(adabag_obj, bacteria)
  )
  )
#> Unit: milliseconds
#>                                                            expr        min
#>  adabag_obj <- boosting(y ~ ., bacteria, boos = F, mfinal = 10) 2497.55208
#>                    pred_adabag <- predict(adabag_obj, bacteria)   34.50564
#>          lq       mean     median         uq       max neval cld
#>  2659.99737 2848.80065 2809.39769 2988.49017 3629.1527   100   b
#>    35.72336   45.21379   37.16913   42.22947  242.7932   100  a
```
