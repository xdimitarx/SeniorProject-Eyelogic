#include "EyeLogic.h"

#include <iostream>

int main(int argc, char *argv[])
{
    ImgFrame mainEntryPoint(Point(1920,1080));

    if(argc == 2)
    {
        mainEntryPoint.insertFrame(loadImageAtPath(argv[1]));
    }
    else
    {
        mainEntryPoint.insertFrame(cameraCapture());
    }
    
    cout << "finito" << endl;
    return 0;
}
