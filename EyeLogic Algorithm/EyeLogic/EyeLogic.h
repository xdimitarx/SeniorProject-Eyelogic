#pragma once

#ifdef EYELOGIC_EXPORTS  
#define EYELOGIC __declspec(dllexport)   
#else  
#define EYELOGIC __declspec(dllimport)   
#endif  

#include <opencv2/core/core.hpp>
//#include <opencv2\core\highgui.hpp>

using namespace cv;
using namespace std;

int dummyFunction();

extern "C"
{
	const EYELOGIC int initEyeLogic()
	{
		return dummyFunction();
	}
}
