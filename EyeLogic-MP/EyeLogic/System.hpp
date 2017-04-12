//
//  System.hpp
//  EyeLogic
//
//  Created by Dimitar Vasilev on 3/24/17.
//  Copyright © 2017 Dimitar Vasilev. All rights reserved.
//

#ifndef System_h
#define System_h

class System {
public:
	//Win - 1; Mac - 0
	virtual int getOS() { return -1; };
	virtual bool voiceFork() { return false; };
	virtual std::string readFromJulius() { return std::string(); };
    virtual void setCurPos(float x, float y) {return;};
	virtual bool click() { return false; };
	virtual bool rightClick() { return false; };
	virtual bool drag() { return false; };
};

#endif /* System_h */
