#pragma once

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

//Sobel filter kernelx
float kernelx[3][3] = { { -1,  0,  1 },{ -2,  0,  2 },{ -1, 0, 1 } };
float kernely[3][3] = { { -1, -2, -1 },{ 0,  0,  0 },{ 1, 2, 1 } };
int valueThresh = 20;


Mat Sobel(Mat eyebox) {
	float pixel_x, pixel_y;
	float val;
	Mat edge(eyebox.rows, eyebox.cols, CV_8UC1);
	Mat bins(eyebox.rows, eyebox.cols, CV_8UC1);
	bins = Scalar(0);


	//Point 
	// line( img,	start,		end,		Scalar(0, 0, 0),		thickness,		8 );

	for ( int x = 1; x < eyebox.rows - 2; x++) {
		for (int y = 1; y < eyebox.cols - 2; y ++) {
			pixel_x = (kernelx[0][0] * eyebox.at<char>(x - 1, y - 1)) + (kernelx[0][1] * eyebox.at<char>(x, y - 1)) + (kernelx[0][2] * eyebox.at<char>(x + 1, y - 1)) +
				(kernelx[1][0] * eyebox.at<char>(x - 1, y)) + (kernelx[1][1] * eyebox.at<char>(x, y)) + (kernelx[1][2] * eyebox.at<char>(x + 1, y)) +
				(kernelx[2][0] * eyebox.at<char>(x - 1, y + 1)) + (kernelx[2][1] * eyebox.at<char>(x, y + 1)) + (kernelx[2][2] * eyebox.at<char>(x + 1, y + 1));

			pixel_y = (kernely[0][0] * eyebox.at<char>(x - 1, y - 1)) + (kernely[0][1] * eyebox.at<char>(x, y - 1)) + (kernely[0][2] * eyebox.at<char>(x + 1, y - 1)) +
				(kernely[1][0] * eyebox.at<char>(x - 1, y)) + (kernely[1][1] * eyebox.at<char>(x, y)) + (kernely[1][2] * eyebox.at<char>(x + 1, y)) +
				(kernely[2][0] * eyebox.at<char>(x - 1, y + 1)) + (kernely[2][1] * eyebox.at<char>(x, y + 1)) + (kernely[2][2] * eyebox.at<char>(x + 1, y + 1));

			val = (abs(pixel_x) + abs(pixel_y));
			if (val > valueThresh) {
				val = (abs(pixel_x) + abs(pixel_y));
			}
			else { val = 0; }
			edge.at<uchar>(Point(y, x)) = (uchar)(int)val;
		}
	}


	return edge;
}

void testSobel() {

	VideoCapture cap;
	if (!cap.open(0))
		return;
	namedWindow("", WINDOW_NORMAL);
	Mat color, gray, edges;
	for (;;) {
		cap >> color;
		cvtColor(color, gray, CV_BGR2GRAY);
		edges = Sobel(gray);
		imshow("this is you, smile! :)", edges);
		if (waitKey(10) == 27) break;
	}
	return;
}

void irisdetect() {
	VideoCapture cap;
	if (!cap.open(0))
		return;

	CascadeClassifier eyeDetector;
	eyeDetector.load("haarcascade_eye_tree_eyeglasses.xml");
	vector<Rect_<int>> eyes;

	//Gaussian filter parameters
	Size ksize(9,9);
	//ksize.height = 5;
	//ksize.width = ksize.height;
	int sigmax = 5;
	int sigmay = 0;
	double lowthresh = 15;

	vector<Vec3f> circles;

	//Loop for getting continuous image feed and processing it
	for (;;)
	{
		Mat framegray, frame, canny, roi;
		eyes.clear();
		cap >> frame;
		//cap >> frameColor;
		if (frame.empty()) break; // end of video stream	
								  //if (frameColor.empty()) break; // end of video stream	

		cvtColor(frame, framegray, CV_BGR2GRAY);
		GaussianBlur(framegray, framegray, ksize, sigmax, sigmay);
		circle(frame, Point(100, 100), 3, Scalar(255, 0, 166), -3);

		
		//detect eyes using Haas classification
		eyeDetector.detectMultiScale(framegray, eyes);

		cout << eyes.size() << endl;
		//draw rectangles around eyes
		for (int i = 0; i < eyes.size(); i++) {
			Rect roiRect = Rect(eyes[i].x, eyes[i].y, eyes[i].width, eyes[i].height);
			HoughCircles(roiRect, circles, CV_HOUGH_GRADIENT, 1, 10, 20, 20, 0, framegray.cols);
			circles.size();

			for (size_t i = 0; i < circles.size(); i++)
			{
				Point center(cvRound(circles[i][0]) + eyes[i].x, cvRound(circles[i][1]) + eyes[i].y);
				int radius = cvRound(circles[i][2]);
				// circle center
				circle(frame, center, 3, Scalar(0, 255, 0), -1, 8, 0);
				// circle outline
				circle(frame, center, radius, Scalar(0, 0, 255), 3, 8, 0);
			}
			//roi.copyTo(framegray.rowRange(eyes[i].y, eyes[i].y + eyes[i].height).colRange(eyes[i].x, eyes[i].x + eyes[i].width));
		}
		

		imshow("this is you, smile! :)", frame);
		if (waitKey(10) == 27) break; // stop capturing by pressing ESC 
	}



}

int gradientAlgo()
{
	VideoCapture cap;
	// open the default camera, use something different from 0 otherwise;
	// Check VideoCapture documentation.
	if (!cap.open(0))
		return 0;

	CascadeClassifier eyeDetector;
	eyeDetector.load("haarcascade_eye_tree_eyeglasses.xml");
	vector<Rect_<int>> eyes;

	//Gaussian filter parameters
	Size ksize;
	ksize.height = 5;
	ksize.width = ksize.height;
	int sigmax = 5;
	int sigmay = 0;
	double lowthresh = 15;

	//Loop for getting continuous image feed and processing it
	for (;;)
	{
		Mat framegray, frame, canny, roi;
		eyes.clear();
		cap >> frame;
		//cap >> frameColor;
		if (frame.empty()) break; // end of video stream	
								  //if (frameColor.empty()) break; // end of video stream	

		cvtColor(frame, framegray, CV_BGR2GRAY);
		GaussianBlur(framegray, framegray, ksize, sigmax, sigmay);

		//detect eyes using Haas classification
		eyeDetector.detectMultiScale(framegray, eyes);

		cout << eyes.size() << endl;

		//draw rectangles around eyes
		for (int i = 0; i < eyes.size(); i++) {
			Rect roiRect = Rect(eyes[i].x, eyes[i].y, eyes[i].width, eyes[i].height);
			roi = Sobel(Mat(framegray, roiRect));
			roi.copyTo(framegray.rowRange(eyes[i].y, eyes[i].y + eyes[i].height).colRange(eyes[i].x, eyes[i].x + eyes[i].width));
		}
		imshow("this is you, smile! :)", framegray);
		if (waitKey(10) == 27) break; // stop capturing by pressing ESC 
	}
	return 0;
}