#include "EyeLogic.h"
#include <iostream>

int main()
{
    EyeLogicAlg eyeAlg;
    Mat *image = eyeAlg.cameraCapture();
    if(!eyeAlg.detectEyes(image)){
        return 1;
    }
    
    
    //check flags and print accordingly?
    // imwrite("Test2.jpg", eyeAlg.captureEyes.at(0));
    // imwrite("Test3.jpg", eyeAlg.captureEyes.at(1));
    
    Point_<int> xyValues;
    eyeAlg.approximateAngle(&xyValues);
    
    cout << "finito" << endl;
    return 0;
}
