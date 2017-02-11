#include "EyeLogic.h"

#include <iostream>

int main()
{
    ImgFrame mainEntryPoint(Point(1920,1080));
    if(true)
    {
        mainEntryPoint.insertFrame(loadImageAtPath("camera.jpg"));
    }
    else
    {
        mainEntryPoint.insertFrame(cameraCapture());
    }
    

    
    cout << "finito" << endl;
    return 0;
}
