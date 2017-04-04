#ifndef Mac_h
#define Mac_h
#include "System.hpp"
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/objdetect/objdetect.hpp>
#include <opencv2/imgcodecs/imgcodecs.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <ApplicationServices/ApplicationServices.h>

class Mac : public System {
public:
    virtual void setCurPos(float x, float y) override {
        // Move to 200x200
        CGEventRef move1 = CGEventCreateMouseEvent(
                                                   NULL, kCGEventMouseMoved,
                                                   CGPointMake(x, y),
                                                   kCGMouseButtonLeft // ignored
                                                   );
        CGEventPost(kCGHIDEventTap, move1);
        CFRelease(move1);
        
         
    }
    
    void click(float x, float y){
        CGEventRef click1_down = CGEventCreateMouseEvent(
                                                         NULL, kCGEventLeftMouseDown,
                                                         CGPointMake(x, y),
                                                         kCGMouseButtonLeft
                                                         );
        
        // Left button down at 250x250
        // Left button up at 250x250
        CGEventRef click1_up = CGEventCreateMouseEvent(
                                                       NULL, kCGEventLeftMouseUp,
                                                       CGPointMake(x, y),
                                                       kCGMouseButtonLeft
                                                       );

        
        // Now, execute these events with an interval to make them noticeable

        CGEventPost(kCGHIDEventTap, click1_down);
        CGEventPost(kCGHIDEventTap, click1_up);
        CFRelease(click1_down);
        CFRelease(click1_up);

    }
    
    

};

#endif 
