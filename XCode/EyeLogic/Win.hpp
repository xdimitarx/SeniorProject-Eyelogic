#ifndef Win_h
#define Win_h
#include "stdafx.h"
#include "windows.h"

#include "System.hpp"
#include <opencv2/core.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/objdetect.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/imgproc.hpp>


class Win : public System {
public:

    virtual System * getSystem() override {
        return new Win();
    }

    virtual void setCurPos(float x, float y) override {

     /*   // Windows way to set the cursor position to some x,y coordinate on the screen
        SetCursorPos(x, y);
        cout << x << "    " << y << endl;
      */
        return;
    }

    void setCurPos(cv::Point point) {
        
        // Windows way to set the cursor position to some x,y coordinate on the screen
        // SetCursorPos(point.x, point.y);
        //cout << point.x << "    " << point.y << endl;
        return;
    }

    cv::Point getCurPos() {
        //Windows way to get the cursor position
        cv::Point windowsPoint;
        //GetCursorPos(&windowscv::Point);
        //cv::Point curr((int)windowsPoint.x, (int)windowscv::Point.y);

        return windowsPoint;
    }

};
#endif /* Win_h */
