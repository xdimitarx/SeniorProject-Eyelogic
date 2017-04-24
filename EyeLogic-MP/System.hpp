//
//  System.hpp
//  EyeLogic
//
//  Created by Dimitar Vasilev on 3/24/17.
//  Copyright © 2017 Dimitar Vasilev. All rights reserved.
//

#ifndef System_h
#define System_h

#ifdef __APPLE__
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/objdetect/objdetect.hpp>
#include <opencv2/imgcodecs/imgcodecs.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <ApplicationServices/ApplicationServices.h>
#else
#include <opencv2/core.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/objdetect.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/imgproc.hpp>
#include "wtypes.h"
#include "stdafx.h"
#include "windows.h"
#include "targetver.h"
#endif

class System {
public:
    virtual void setCurPos(cv::Point setCursor){return;};
    virtual cv::Point getCurPos(){return cv::Point();};
    virtual void click(){return;};
    virtual bool voiceFork(){return false;};
    virtual std::string readFromJulius(){return "";};
    virtual int getOs(){return -1;};
};

#endif /* System_h */
