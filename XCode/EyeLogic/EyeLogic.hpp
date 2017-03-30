#ifndef EYELOGIC_H
#define EYELOGIC_H

#include "System.hpp"


//OPENCV
#ifdef __APPLE__
#include "Mac.hpp"
#define MAC 1
#else
#include "Win.hpp"
#define MAC 0
#endif

//STD
#include <iostream>
#include <vector>
#include <math.h>
#include <unistd.h>

using namespace std;
using namespace cv;

Mat loadImageAtPath(string path);
Mat cameraCapture();
cv::Point getCurPos();


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
    
    cv::Point * getEyeVector(){return &eyeVector;};
    void setEyeVector(float x, float y);
    bool getBlink();
    
    bool leftEye;
private:
    CascadeClassifier detector;
    
    //technically right eye ---> ;)
    Mat original;
    Mat filtered;
    Mat filtforIris;
    
    cv::Point eyeCorner;
    cv::Point eyeCenter;
	int eyeRadius;
    cv::Point eyeVector = cv::Point(0,0);   //set default to 0,0
    
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
    bool getCursorXY(cv::Point * result);
    Eye getLeftEye(){return leftEye;};
    Eye getRightEye(){return rightEye;};
    
    //0 = None, 1 = Left, 2 = Right, 3 = Both/No Eyes Detected
    int getBlink();

    void getReferenceImages();
    
private:
    cv::Point screenResolution;
    CascadeClassifier faceDetector;
    Eye leftEye, rightEye;

    Mat ref_topLeft, ref_bottomLeft, ref_center, ref_topRight, ref_bottomRight;
};

#endif
