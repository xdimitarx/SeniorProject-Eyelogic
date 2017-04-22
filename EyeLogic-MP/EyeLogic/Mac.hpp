#ifndef Mac_h
#define Mac_h
#include "System.hpp"

class Mac : public System {
public:
    // data member
    cv::Point *currentPos;
    
    void setCurPos(cv::Point setCursor) override {
//         
//         float x = setCursor.x;
//         float y = setCursor.y;
//         
//        // Move to 200x200
//        CGEventRef move1 = CGEventCreateMouseEvent(
//                                                   NULL, kCGEventMouseMoved,
//                                                   CGPointMake(x, y),
//                                                   kCGMouseButtonLeft // ignored
//                                                   );
//        *currentPos = {static_cast<int>(x), static_cast<int>(y)};
//        CGEventPost(kCGHIDEventTap, move1);
//        CFRelease(move1);
//        
        
    }
    
    cv::Point getCurPos() override {
        return *currentPos;
    }
    
    void click() override {
        
//        float x = currentPos->x;
//        float y = currentPos->y;
//        
//        CGEventRef click1_down = CGEventCreateMouseEvent(
//                                                         NULL, kCGEventLeftMouseDown,
//                                                         CGPointMake(x, y),
//                                                         kCGMouseButtonLeft
//                                                         );
//        
//        // Left button down at 250x250
//        // Left button up at 250x250
//        CGEventRef click1_up = CGEventCreateMouseEvent(
//                                                       NULL, kCGEventLeftMouseUp,
//                                                       CGPointMake(x, y),
//                                                       kCGMouseButtonLeft
//                                                       );
//
//        
//        // Now, execute these events with an interval to make them noticeable
//
//        CGEventPost(kCGHIDEventTap, click1_down);
//        CGEventPost(kCGHIDEventTap, click1_up);
//        CFRelease(click1_down);
//        CFRelease(click1_up);

    }
    
    
    

};

#endif 
