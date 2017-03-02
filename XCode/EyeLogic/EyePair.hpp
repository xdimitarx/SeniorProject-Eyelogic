//
//  Header.h
//  EyeLogic
//
//  Created by Dimitar Vasilev on 3/1/17.
//  Copyright © 2017 Dimitar Vasilev. All rights reserved.
//

#ifndef EyePair_h
#define EyePair_h
#include "EyeLogic.hpp"
struct EyePair {
    Point leftVector;
    Point rightVector;
    
    EyePair(Point l, Point r){ this->leftVector = l; this->rightVector = r;}
};

#endif /* EyePair_h */
