#ifndef Win_h
#define Win_h

#include "stdafx.h"
#include "windows.h"

#include "System.hpp"

//#include <iostream>

#include <opencv2/core.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/objdetect.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/imgproc.hpp>

using namespace std;


class Win : public System {
public:

    virtual void setCurPos(float x, float y) override {
        // Windows way to set the cursor position to some x,y coordinate on the screen
        SetCursorPos(x, y);
        //cout << x << "    " << y << endl;
        return;
    }

    void setCurPos(cv::Point point) {
        // Windows way to set the cursor position to some x,y coordinate on the screen
        SetCursorPos(point.x, point.y);
        //cout << point.x << "    " << point.y << endl;
        return;
    }

    cv::Point getCurPos() {
        //Windows way to get the cursor position
        LPPOINT windowsPoint;
        GetCursorPos((LPPOINT)&windowsPoint);
        cv::Point curr((int)windowsPoint->x, (int)windowsPoint->y);
        return curr;
    }

};
#endif /* Win_h */
