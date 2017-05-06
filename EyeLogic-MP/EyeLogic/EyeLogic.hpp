#ifndef EYELOGIC_H
#define EYELOGIC_H

//STD
#include <iostream>
#include <vector>
#include <math.h>
#include <map>
#include <string>
#include <fstream>
#include <chrono>
#include <algorithm>
#include <memory>

//BOOST
#include <boost/thread.hpp>
#include <boost/filesystem.hpp>
#include <boost/chrono.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/algorithm/string.hpp>

#include "System.hpp"

#ifdef __APPLE__
#include "Mac.hpp"
#else
#include "Win.hpp"
#endif

#include "VoiceTool.hpp"


using namespace std;
using namespace cv;


//Dev:
//void lotsOfTheProgram(); //please see function definition for complete description of function's operatives


//Helper Functions
Mat loadImageAtPath(string path);
bool startCam();


extern std::unique_ptr<System> singleton;
extern cv::Point screenres;
extern VideoCapture cap;

struct EyePair {
    cv::Point leftVector;
    cv::Point rightVector;
    
    EyePair(cv::Point l, cv::Point r){ this->leftVector = l; this->rightVector = r;}
    EyePair(){}
};

extern std::map<Mat *, EyePair> RefImageVector;
extern Mat ref_camera, ref_topLeft, ref_bottomLeft, ref_center, ref_topRight, ref_bottomRight;

class Eye
{
public:
    Eye(string pathToClassifier, bool left);
    Eye();
    ~Eye();
    
    //Cuts Out Eye from half image
    //left is true if using the users left half
    //of their face when looking at them
    bool detectKeyFeatures(Mat &input);
    
    cv::Point getEyeVector(){return eyeVector;};
    bool getBlink();
    
    bool leftEye;
    //private:
    CascadeClassifier detector;
    
    //technically right eye ---> ;)
    Rect_<int> eyeLocationOnImageHalf;
    Mat original;
    Mat filtered;
    Mat filtforIris;
    
    cv::Point vectorPupilToLeftCorner;
    cv::Point vectorPupilToRightCorner;
    cv::Point eyeCornerLeft;
    cv::Point eyeCornerRight;
    cv::Point eyeCenter;
    int eyeRadius;
    cv::Point eyeVector = cv::Point(-1,-1);   //set default to 0,0
    
    bool blink;
    
    void equalHist();
    
    //+ brightens the eye, -darkens
    void addLighting(int intensity);
    
    void binaryThreshForSc();
    
    void binaryThreshForIris();
    
    void applyGaussian();
    
    inline bool findPupil();
    
    inline bool findEyeCorner();
    
    void createEyeVector();
};

class ImgFrame
{
public:
    ImgFrame();
    ~ImgFrame();
    
    bool insertFrame(Mat &frame);
    bool setCursor();
    Eye getLeftEye(){return leftEye;};
    Eye getRightEye(){return rightEye;};
    
    //0 = None, 1 = Left, 2 = Right, 3 = Both/No Eyes Detected
    int getBlink();
    
    
private:
    CascadeClassifier faceDetector;
    Eye leftEye, rightEye;
    
};

#endif
