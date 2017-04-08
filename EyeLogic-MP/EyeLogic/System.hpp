//
//  System.hpp
//  EyeLogic
//
//  Created by Dimitar Vasilev on 3/24/17.
//  Copyright © 2017 Dimitar Vasilev. All rights reserved.
//

#ifndef System_h
#define System_h

#include "VoiceTool.hpp"

class System {
public:
	//Win - 1; Mac - 0
	virtual int getOS() { return -1; };
	virtual bool voiceFork() { return false; };
    virtual void setCurPos(float x, float y) {return;};
};

#endif /* System_h */
