
//OPENCV
#include <opencv2/core.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/objdetect.hpp>
#include <opencv2/imgcodecs.hpp>

//STD
#include <iostream>
#include <vector>

using namespace std;
using namespace cv;

static vector<Mat *> referenceLibrary; //Mat[0] Left Eye, Mat[1] Right Eye
static Mat * captureEyes; //Mat[0] Left Eye, Mat[1] Right Eye\

static bool referenceLoaded = false;

Mat* loadImageAtPath(string path);
void loadReferenceImages();
bool detectEyes();
void approximateAngle();
void angleToMouseMov();

bool initEyeLogic();

bool setupEyeLogic();

bool runEyeLogic(/* Other Arguments */float * mouseMoveValues);
