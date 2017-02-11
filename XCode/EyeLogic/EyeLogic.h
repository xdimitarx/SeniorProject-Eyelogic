
//OPENCV
#ifdef __APPLE__
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/objdetect/objdetect.hpp>
#include <opencv2/imgcodecs/imgcodecs.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#else
#include <opencv2/core.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/objdetect.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/imgproc.hpp>
#endif



//STD
#include <iostream>
#include <vector>
#include <math.h>

using namespace std;
using namespace cv;

static bool referenceLoaded = false;

Mat *loadImageAtPath(string path);
void loadReferenceImages();


class EyeLogicAlg
{
    
public:
    vector<Mat> referenceLibrary;
    vector<Mat> captureEyes; //Mat[0] Left Eye, Mat[1] Right Eye
    
    EyeLogicAlg();
    ~EyeLogicAlg() {};
    bool detectEyes(Mat *image);
    //bool approximateAngle(Point_<int> *xyValues);
    //void angleToMouseMov();
    bool approximateAngle(Point_<int> *xyValues);
    void angleToMouseMov();
    Mat *cameraCapture();
    
private:
    CascadeClassifier eyeLDetector, eyeRDetector;
    CascadeClassifier faceDetector;
    
};

void angleToMouseMov();

bool initEyeLogic();

bool setupEyeLogic();

bool runEyeLogic(/* Other Arguments */float * mouseMoveValues);
