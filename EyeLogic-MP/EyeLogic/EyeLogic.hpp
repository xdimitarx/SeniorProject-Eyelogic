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

/********************
 * HELPER FUNCTIONS *
 ********************/
Mat loadImageAtPath(string path);
bool startCam();
bool getReferenceImage();
void updateBoundaryWindows();

/*********************
 * CLASS DEFINITIONS *
 *********************/

// RefImage class
class RefImage {
public:
    // getters
    cv::Mat getImage();
    cv::Point getPupilAvg();
    
    // setters
    void setImage(cv::Mat img);
    void setPupilAvg(cv::Point avg);
    
private:
    cv::Point pupilAvg;
    cv::Mat image;

};

// ImgFrame class
class ImgFrame {
public:
    void calculateAverageEyeMethod();
private:
};

/***************
 * GLOBAL ENUM *
 ***************/
enum Coordinate{
    X,
    Y
};

/**********************
 * EXTERNAL VARIABLES *
 **********************/
extern std::unique_ptr<System> singleton;
extern cv::Point screenres;
extern VideoCapture cap;
extern cv::Mat capture;
extern RefImage ref_left, ref_right, ref_top, ref_bottom;;
extern RefImage *refArray [];
extern int imageCount;
extern cv::CascadeClassifier eyeDetector;
extern cv::Rect_<int>rightEyeBounds;
extern cv::Rect_<int>leftEyeBounds;
extern std::vector<cv::Rect_<int>> eyes;
extern int REFIMAGES;
extern int FRAMES;
extern int THRESHOLD;
extern int MAXFRAMES;
extern int imageCount;

#endif
