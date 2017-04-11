//
//  Mouse.m
//  Tutorial1
//
//  Created by Dimitar Vasilev on 4/10/17.
//
//
#import "Mouse.h"
#import <ApplicationServices/ApplicationServices.h>
#import <Appkit/Appkit.h>


@implementation Mouse

// C "trampoline" function to invoke Objective-C method
SC *getMouseCoordinates(void *self){
    return [self getMousePosition];
    
}

// The Objective-C function you wanted to call from C++.
-(SC *) getMousePosition {
    mouseloc = [NSEvent mouseLocation];
    struct SC *sc = nullptr;
    sc->a = mouseloc.x;
    sc->b = mouseloc.y;
    return sc;
}


@end
