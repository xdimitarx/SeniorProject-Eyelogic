//
//  Mouse.h
//  Tutorial1
//
//  Created by Dimitar Vasilev on 4/10/17.
//
//

#ifndef Mouse_h
#define Mouse_h
#import "Mouse-C-Interface.h"
#import <Foundation/Foundation.h>

// An Objective-C class that needs to be accessed from C++
@interface Mouse : NSObject
{
    NSPoint mouseloc;
}

// The Objective-C member function you want to call from C++
- (NSPoint) getMouseCoordinates:(void *) aParameter;
@end


#endif /* Mouse_h */
