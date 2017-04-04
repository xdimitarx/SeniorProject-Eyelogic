#ifndef EYELOGIC_H
#define EYELOGIC_H

#include "System.hpp"
//#include "Mac.hpp"

#include "Win.hpp"
//#include "windows.h"


//OPENCV
/*
#ifdef __APPLE__
#define MAC 1
#else
#define MAC 0
#include "windows.h"
#endif
*/

//STD
#include <iostream>
#include <vector>
#include <math.h>
#include <unistd.h>
#include <map>

using namespace std;
using namespace cv;

Mat loadImageAtPath(string path);
void lotsOfTheProgram(); //please see function definition for complete description of function's operatives
extern System *singleton;


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
    bool detectKeyFeatures(Mat input);
    
    cv::Point &getEyeVector(){return eyeVector;};
    void setEyeVector(float x, float y);
    bool getBlink();
    
    bool leftEye;
private:
    CascadeClassifier detector;
    
    //technically right eye ---> ;)
	cv::Rect_<int> eyeLocationOnImageHalf;
    Mat original;
    Mat filtered;
    Mat filtforIris;
	Mat faceHalf;
    
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
    ImgFrame(cv::Point resolution);
    ~ImgFrame();
    
    bool insertFrame(Mat frame);
    bool setCursor();
    Eye getLeftEye(){return leftEye;};
    Eye getRightEye(){return rightEye;};
    
    //0 = None, 1 = Left, 2 = Right, 3 = Both/No Eyes Detected
    int getBlink();

    void getReferenceImages();
    
private:
    cv::Point screenResolution;
    CascadeClassifier faceDetector;
    Eye leftEye, rightEye;

};

#endif
