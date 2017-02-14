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
int	valueThresh = 50;


Mat Sobel(Mat eyebox) {
	float pixel_x, pixel_y;
	float val;
	double m;
	int yalpha, ybeta, xalpha, xbeta;
	Point first, second;
	Mat edge(eyebox.rows, eyebox.cols, CV_8UC1);
	Mat bins(eyebox.rows, eyebox.cols, CV_8UC1);
	bins = Scalar(0);
	bool firstPointSet = false;
	LineIterator gradLine(eyebox, Point(0,0), Point(0,0), 8);

	for ( int x = 1; x <= eyebox.cols - 2; x++) {
		for (int y = 1; y <= eyebox.rows - 2; y++) {
			//pixel_x = eyebox.at<char>(y, x+1);
			
			firstPointSet = false;
			//cout << "   " << x << "    " << y << endl;

			pixel_x = (kernelx[0][0] * eyebox.at<char>(y - 1, x - 1)) + (kernelx[0][1] * eyebox.at<char>(y, x - 1)) + (kernelx[0][2] * eyebox.at<char>(y + 1, x - 1)) +
				(kernelx[1][0] * eyebox.at<char>(y - 1, x)) + (kernelx[1][1] * eyebox.at<char>(y, x)) + (kernelx[1][2] * eyebox.at<char>(y + 1, x)) +
				(kernelx[2][0] * eyebox.at<char>(y - 1, x + 1)) + (kernelx[2][1] * eyebox.at<char>(y, x + 1)) + (kernelx[2][2] * eyebox.at<char>(y + 1, x + 1));
			

			pixel_y = (kernely[0][0] * eyebox.at<char>(y - 1, x - 1)) + (kernely[0][1] * eyebox.at<char>(y, x - 1)) + (kernely[0][2] * eyebox.at<char>(y + 1, y - 1)) +
				(kernely[1][0] * eyebox.at<char>(y - 1, x)) + (kernely[1][1] * eyebox.at<char>(y, x)) + (kernely[1][2] * eyebox.at<char>(y + 1, x)) +
				(kernely[2][0] * eyebox.at<char>(y - 1, x + 1)) + (kernely[2][1] * eyebox.at<char>(y, x + 1)) + (kernely[2][2] * eyebox.at<char>(y + 1, x + 1));
			
			if (abs(pixel_x) + abs(pixel_y) > valueThresh) {
			
				m = (pixel_y) / (pixel_x);
				yalpha = (int)(m*(0 - x) + y);
				ybeta = (int)(m*(eyebox.cols - x) + y);
				xalpha = (int)(((0 - y) / m) + x);
				xbeta = (int)(((eyebox.rows - y) / m) + x);

			
				if (yalpha >= 0 && yalpha <= eyebox.rows) {
					if (firstPointSet) {
						second = Point(0, yalpha);
					}
					else {
						first = Point(0, yalpha);
						firstPointSet = true;
					}
				}
				if (ybeta >= 0 && ybeta <= eyebox.rows) {
					if (firstPointSet) {
						second = Point(eyebox.cols, ybeta);
					}
					else {
						first = Point(eyebox.cols, ybeta);
						firstPointSet = true;
					}
				}
				if (xalpha >= 0 && xalpha <= eyebox.cols) {
					if (firstPointSet) {
						second = Point(xalpha, 0);
					}
					else {
						first = Point(xalpha, 0);
						firstPointSet = true;
					}
				}
				if (xbeta >= 0 && xbeta <= eyebox.cols) {
					if (firstPointSet) {
						second = Point(xbeta, eyebox.rows);
					}
					else {
						first = Point(xbeta, eyebox.rows);
						firstPointSet = true;
					}
				}
			
			
					gradLine = LineIterator(eyebox, first, second, 8);
					for (int i = 0; i < gradLine.count; i++, ++gradLine)
					{
						bins.at<uchar>(gradLine.pos()) = (bins.at<uchar>(gradLine.pos()) + 1);
					}
			} // value threshold condition

		


			/*
			val = (abs(pixel_x) + abs(pixel_y));
			if (val > valueThresh) {
				val = (abs(pixel_x) + abs(pixel_y));
			}
			else { val = 0; }
			edge.at<uchar>(Point(y, x)) = (uchar)(int)val;
			*/

		}//inner for
	}//outer for
	cout << "Exit Sobel function" << endl;


	return bins;
	//return edge;
}

void testSobel() {
	
	VideoCapture cap;
	if (!cap.open(0))
		return;
	namedWindow("", WINDOW_NORMAL);
	Mat color, gray, edges;
	//for (;;) {
		cap >> color;
		cvtColor(color, gray, CV_BGR2GRAY);
		edges = Sobel(gray);
		imshow("this is you, smile! :)", edges);
		imshow("original", color);
		waitKey(10000);
	//	if (waitKey(10) == 27) break;
	//}
	

	return;
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