//
//  User.hpp
//  EyeLogic
//
//  Created by Dimitar Vasilev on 2/28/17.
//  Copyright © 2017 Dimitar Vasilev. All rights reserved.
//

#ifndef User_h
#define User_h
#include "RefImage.hpp"
#include <iostream>
#include <vector>
class User {
public:
    int user_id;
    std::vector<RefImage> user_images;

};

#endif /* User_h */
