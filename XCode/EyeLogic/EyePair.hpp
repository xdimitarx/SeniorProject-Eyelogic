#ifndef EyePair_h
#define EyePair_h
#include "EyeLogic.hpp"
struct EyePair {
    cv::Point leftVector;
    cv::Point rightVector;
    
    EyePair(cv::Point l, cv::Point r){ this->leftVector = l; this->rightVector = r;}
};

#endif /* EyePair_h */
