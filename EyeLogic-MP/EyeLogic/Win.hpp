#ifndef Win_h
#define Win_h

#include "System.hpp"

using namespace std;


class Win : public System {
public:
    
    void setCurPos(cv::Point point) override {
        // Windows way to set the cursor position to some x,y coordinate on the screen
        SetCursorPos(point.x, point.y);
        //cout << point.x << "    " << point.y << endl;
        return;
    }
    
    virtual cv::Point getCurPos() override {
        //Windows way to get the cursor position
        LPPOINT windowsPoint;
        GetCursorPos((LPPOINT)&windowsPoint);
        cv::Point curr((int)windowsPoint->x, (int)windowsPoint->y);
        return curr;
    }





};
#endif /* Win_h */
