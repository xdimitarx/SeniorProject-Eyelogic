//
//  Mouse-C-Interface.h
//  Tutorial1
//
//  Created by Dimitar Vasilev on 4/11/17.
//
//

#ifndef Mouse_C_Interface_h
#define Mouse_C_Interface_h
struct SC{
    float a;
    float b;
};

// This is the C "trampoline" function that will be used
// to invoke a specific Objective-C method FROM C++
SC *getMouseCoordinates(void *myObjectInstance);

#endif /* Mouse_C_Interface_h */
