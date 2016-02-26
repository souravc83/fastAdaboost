## Test environments
* local OS X install, R 3.1.2
* ubuntu 12.04 (on travis-ci), R 3.1.2
* win-builder (devel and release)
## Resubmission
This is a resubmission. The comment on the last submission was:
"> Description: This library implements a C++ based blazingly fast

This surely is a package and not a library?

Pls start with

  Implements ..."

I have now addressed the comments by:
* Changing the first line of the Description field in the DESCRIPTION file to:
"Implements Adaboost based on C++ backend code."
* Changing all occurrences of the word "library" to the word "package"


## R CMD check results
There were no ERRORs or WARNINGs or NOTES in local OSX install.
win-builder:
* checking CRAN incoming feasibility ... NOTE
Maintainer: 'Sourav Chatterjee <souravc83@gmail.com>'
New submission

License components with restrictions and base license permitting such:
MIT + file LICENSE
File 'LICENSE':
YEAR: 2016
COPYRIGHT HOLDER: Sourav Chatterjee

Possibly mis-spelled words in DESCRIPTION:
Adaboost (3:33, 5:17, 9:41, 10:14)
SAMME (10:23)
blazingly (4:50)


travis build passed.
