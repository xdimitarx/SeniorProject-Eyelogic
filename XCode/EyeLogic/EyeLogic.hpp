/*
 * Author: Dimitri Vasilev, Dominik Konecny
 * File: EyeLogic.h
 * Eye Tracking Library
 * v0.5
 */

#ifndef EYELOGIC_H
#define EYELOGIC_H

//OPENCV
#ifdef __APPLE__
#include "MacInclude.hpp"
#else
#include "WinInclude.hpp"
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
void setCurPos(int x, int y);
void setCurPos(Point point);
Point getCurPos();


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
    
    Point * getEyeVector(){return &eyeVector;};
    void setEyeVector(float x, float y);
    bool getBlink();
    
    bool leftEye;
private:
    CascadeClassifier detector;
    
    //technically right eye ---> ;)
    Mat original;
    Mat filtered;
    Mat filtforIris;
    
    Point eyeCorner;
    Point eyeCenter;
    //size_t eyeRadius;
	int eyeRadius;
    Point eyeVector;
    
    bool blink;
   
    void equalHist();
    
    //+ brightens the eye, -darkens
    void addLighting(int intensity);
    
    void binaryThreshForSc();
    
    void binaryThreshForIris();
    
    void applyGaussian();
    
    bool findPupil();
    
    bool findEyeCorner();
    
    void createEyeVector();
};

class ImgFrame
{
public:
    ImgFrame(Point resolution);
    ~ImgFrame();
    
    bool insertFrame(Mat frame);
    bool getCursorXY(Point * result);
    Eye getLeftEye(){return leftEye;};
    Eye getRightEye(){return rightEye;};
    
    //0 = None, 1 = Left, 2 = Right, 3 = Both/No Eyes Detected
    int getBlink();

    void getReferenceImages();
    
private:
    Point screenResolution;
    CascadeClassifier faceDetector;
    Eye leftEye, rightEye;

    Mat ref_topLeft, ref_bottomLeft, ref_center, ref_topRight, ref_bottomRight;
};

#endif
