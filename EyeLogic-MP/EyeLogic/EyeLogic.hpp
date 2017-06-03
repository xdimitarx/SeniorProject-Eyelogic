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
#include <boost/filesystem.hpp>
#include <boost/chrono.hpp>
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

extern int imageCount;
extern std::vector<cv::Point>leftVectors;
extern std::vector<cv::Point>rightVectors;
extern std::vector<cv::Point>topVectors;
extern std::vector<cv::Point>downVectors;

/********************
 * HELPER FUNCTIONS *
 ********************/
Mat loadImageAtPath(string path);

/*********************
 * CLASS DEFINITIONS *
 *********************/

enum class RefPoint { LEFT, RIGHT, TOP, BOTTOM };

class EyeLogic {

public:

	//basic public function, forceNewTemplate forces a creation of new template images
	bool insertFrame(Mat frame, bool forceNewTemplate = false);

	// returns Mat.empty() if not all templates are available
	cv::Mat getTemplate(cv::Rect * faceCrop, cv::Rect * leftEyeCrop, cv::Rect * rightEyeCrop);

	/*
	version 1
	storeTemplate(image, faceBound)
	Calculates template matching strip and stores it in faceRect

	version 2
	storeTemplate(image, faceBound, leftEyeCrop, rightEyeCrop);
	Take in image as template strip and store corresponding cropping information to be used in insertframe
	*/
    void storeTemplate(cv::Mat image, cv::Rect faceBound, cv::Rect leftEyeCrop = cv::Rect(), cv::Rect rightEyeCrop = cv::Rect());

	// returns point bounded by screen size
	cv::Point eyeVectorToScreenCoord();

	// returns Avg Eye Position in reference to faceRect
	cv::Point getEyeVector();

	// sets reference positions individually (LEFT, RIGHT, TOP, BOTTOM)
	void setReferencePoint(cv::Point point, RefPoint refPosition);

	// returns vector of points in order (LEFT, RIGHT, TOP, BOTTOM)
	vector <cv::Point>  getReferencePointData();

	// sets reference positions according to input vector ^^^
	void setReferencePointData(vector <cv::Point> * data);

	// valid = true also checks if reference points make a valid bounding box
	bool Calibrated(bool valid);

	EyeLogic(cv::Point screenres);

	cv::Point distance;

private:

	//Template Matching Vars
    cv::Mat userTemplate; //strip of face including the nose used for template matching
	bool faceTemplateExists = false;
    cv::Rect faceRect; // face bounding box

	bool eyeTemplatesExists = false;
    cv::Rect leftEyeBound; //relative to face rect
    cv::Rect rightEyeBound; //relative to face rect

	//frame that is inserted
    cv::Mat currentFrame;

	//Current Avg Pupil Value
	cv::Point eyeVector;

	//Outer limits for EyeVector
	cv::Point screenResolution;

	//Initialized all as (-1,-1)
	cv::Point ref_Right, ref_Top, ref_Left, ref_Bottom;
	
	CascadeClassifier faceExtractor;
	CascadeClassifier leftEyeExtractor;
	CascadeClassifier rightEyeExtractor;

	//Points that track previous frame information
	cv::Point screenMap, destinationOld, direction, delta;

	//finds and sets eye bounds from faceCrop and sets eyeTemplatesExists = true
	bool createEyeBounds(cv::Mat faceCrop);
	
	//applies dom filters for pupil detection
	Mat applyPupilFilters(Mat eyeCrop);

	//uses image moments to detect center of pupil from filtered image
	cv::Point findPupil(Mat filteredEyeCrop);

	//returns a faceCrop that matches the template if true and the difference between the two planes
	bool checkTemplate(cv::Mat frame, cv::Rect * faceCrop, cv::Point * frameDifference);

	//creates an error file
	void logError(std::string message, cv::Mat image = cv::Mat());
};

#endif
