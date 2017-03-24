#ifndef RefImage_h
#define RefImage_h
#ifdef __APPLE__
#include <opencv2/core/core.hpp>
#else
#include <opencv2/core.hpp>
#endif

#include "EyePair.hpp"

struct RefImage {
    Mat *image;
    EyePair pair;
    
};

#endif /* RefImage_h */
