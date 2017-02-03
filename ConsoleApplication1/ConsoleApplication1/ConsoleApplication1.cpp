// ConsoleApplication1.cpp : Defines the entry point for the console application.
//
#include <stdafx.h>

#include <opencv2/core.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/highgui.hpp>
#include "opencv2/objdetect/objdetect.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"

#include <iostream>
#include <cmath>
using namespace cv;
using namespace std;
int main(int argc, char** argv)
{
	VideoCapture cap;
	// open the default camera, use something different from 0 otherwise;
	// Check VideoCapture documentation.
	if (!cap.open(0))
		return 0;

	CascadeClassifier eyeDetector;
	eyeDetector.load("haarcascade_eye_tree_eyeglasses.xml");
	vector<Rect_<int>> eyes;
	
	Size ksize;
	ksize.height = 5;
	ksize.width = ksize.height;
	int sigmax = 5;
	int sigmay = 0;

	for (;;)
	{

		Mat framegray, frame;
		eyes.clear();
		cap >> frame;
		//cap >> frameColor;
		if (frame.empty()) break; // end of video stream	
		//if (frameColor.empty()) break; // end of video stream	
		
		cvtColor(frame, framegray, CV_BGR2GRAY);
		GaussianBlur(framegray, framegray, ksize, sigmax, sigmay);

		eyeDetector.detectMultiScale(framegray, eyes);
		cout << eyes.size() << endl;
		for (int i = 0; i < eyes.size(); i++) {
			rectangle(framegray, Point(eyes[i].x, eyes[i].y), Point(eyes[i].x + eyes[i].width, eyes[i].y + eyes[i].height), Scalar(255));
		}		
		
		imshow("this is you, smile! :)", framegray);
		//imshow("this is you, smile! :)", frameColor);
		if (waitKey(10) == 27) break; // stop capturing by pressing ESC 
	}
	return 0;
}