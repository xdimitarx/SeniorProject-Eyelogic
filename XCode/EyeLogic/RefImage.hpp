//
//  RefImage.h
//  EyeLogic
//
//  Created by Dimitar Vasilev on 2/28/17.
//  Copyright © 2017 Dimitar Vasilev. All rights reserved.
//

#ifndef RefImage_h
#define RefImage_h
#ifdef __APPLE__
#include <opencv2/core/core.hpp>
#else
#include <opencv2/core.hpp>
#endif

struct RefImage {
    Mat *image;
    Point loc;
    Point coord;
};

#endif /* RefImage_h */
