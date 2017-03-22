// Eyelogic.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"

//#include "GradientAlgorithm.h"
#include <opencv2/imgcodecs.hpp>
#include <opencv2/highgui.hpp>
#include "opencv2/objdetect/objdetect.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include <iostream>
#include <cmath>
#include <algorithm>
#include <list>

using namespace cv;
using namespace std;

#define MAX_BRIGHTNESS 255;

string file = "C:/Users/spark/Desktop/193/testframe.png";
Mat cannyEdge(Mat image, Mat& Gx, Mat& Gy);


void cornerDetectionWithCanny() {
	Mat edges, Gx, Gy;
	Mat image = imread(file, CV_LOAD_IMAGE_COLOR);

	edges = cannyEdge(image, Gx, Gy);
	if (edges.rows == 0 || edges.cols == 0) {
		cout << "Empty image file found" << endl;
		return;
	}

	imshow("Edges" , edges);
	waitKey(50000);
}

void circleDetectWithCanny(Mat image) {
	//NEEDS A GUARANTEE OF ONE CIRCLE IN THE MATRIX
	Mat  edges, Gx, Gy;
	//image = imread(file, CV_LOAD_IMAGE_COLOR);
	cout << "Cake" << endl;

	CascadeClassifier eyeDetector;
	eyeDetector.load("haarcascade_eye_tree_eyeglasses.xml");
	vector<Rect_<int>> eyes;
	eyeDetector.detectMultiScale(image, eyes);
	cout << "number of eyes found " << eyes.size() << endl;

	Rect roiRect = Rect(eyes[0].x, eyes[0].y, eyes[0].width, eyes[0].height);
	edges = cannyEdge(Mat(image, roiRect), Gx, Gy);

	if (edges.rows == 0 || edges.cols == 0) {
		cout << "Empty image file found" << endl;
		return;
	}


	imshow("orig", Mat(image, roiRect));
	imshow("edges", edges);
	waitKey(2000);

	cout << "Canny works" << endl;
	return;
	Mat bins(edges.rows, edges.cols, CV_8UC1);

	list<Point> listOfCenters;
	Point first, second;
	LineIterator gradLine(edges, Point(0, 0), Point(0, 0), 8);
	double m, pixel_x, pixel_y;
	int yalpha, ybeta, xalpha, xbeta;
	bool firstPointSet = false;

	char maxCurrent = 0;

	bins = Scalar(0);
	//bins = edges.clone();

	for (int y = 1; y <= edges.rows - 2; y++) {
		for (int x = 1; x <= edges.cols - 2; x++) {
			if (0 < edges.at<uchar>(y, x)) {
				pixel_x = (double)Gx.at<char>(y, x);
				pixel_y = (double)Gy.at<char>(y, x);
				m = (pixel_y) / (pixel_x);
				yalpha = (int)(m*(0 - x) + y);
				ybeta = (int)(m*(edges.cols - x) + y);
				xalpha = (int)(((0 - y) / m) + x);
				xbeta = (int)(((edges.rows - y) / m) + x);

				if (yalpha >= 0 && yalpha <= edges.rows) {
					if (firstPointSet) {
						second = Point(0, yalpha);
					}
					else {
						first = Point(0, yalpha);
						firstPointSet = true;
					}
				}
				if (ybeta >= 0 && ybeta <= edges.rows) {
					if (firstPointSet) {
						second = Point(edges.cols, ybeta);
					}
					else {
						first = Point(edges.cols, ybeta);
						firstPointSet = true;
					}
				}
				if (xalpha >= 0 && xalpha <= edges.cols) {
					if (firstPointSet) {
						second = Point(xalpha, 0);
					}
					else {
						first = Point(xalpha, 0);
						firstPointSet = true;
					}
				}
				if (xbeta >= 0 && xbeta <= edges.cols) {
					if (firstPointSet) {
						second = Point(xbeta, edges.rows);
					}
					else {
						first = Point(xbeta, edges.rows);
						firstPointSet = true;
					}
				}
				gradLine = LineIterator(edges, first, second, 8);

				for (int i = 0; i < gradLine.count; i++, ++gradLine)
				{
					if (255 > bins.at<uchar>(gradLine.pos())) {
						bins.at<uchar>(gradLine.pos()) = (bins.at<uchar>(gradLine.pos()) + 1);
					}
					if (bins.at<uchar>(gradLine.pos()) > maxCurrent) {
						maxCurrent = bins.at<uchar>(gradLine.pos());
					}
				}
				firstPointSet = false;
			}
		}
	}

	for (int y = 1; y <= edges.rows - 2; y++) {
		for (int x = 1; x <= edges.cols - 2; x++) {
			if (bins.at<uchar>(y, x) > 0.5 *maxCurrent && bins.at<uchar>(y, x) < maxCurrent) {
				listOfCenters.push_front(Point(x,y));
			}
		}
	}

	int meanx =0, meany = 0;

	for (list<Point>::iterator it = listOfCenters.begin(); it != listOfCenters.end(); it++) {
		//circle(bins, *it , 10, Scalar(255), 5, 8);
		meanx += (*it).x;
		meany += (*it).y;
	}
	if (listOfCenters.size() > 0) {
		meanx = meanx / listOfCenters.size();
		meany = meany / listOfCenters.size();
	}
	else {
		cout << "No circles found" << endl;
	}
	circle(image, Point(meanx, meany), 10, Scalar(255), 5, 8);

	cout << "number of detected centers  " << listOfCenters.size() << endl;


	//imshow("edges", edges);
	imshow("mod image", image);
	imshow("bins", bins);
	waitKey(20000);

}

Mat cannyEdge(Mat image, Mat& Gx, Mat& Gy) {
	Mat gray;
	int maximum = -5, minimum = 1000, thresh1, thresh2; //nonsupression max

	if (!image.data)                              // Check for invalid input
	{
		cout << "Could not open or find the image" << std::endl;
		return image;
	}

	cvtColor(image, gray, CV_BGR2GRAY);
	// Mat Gx(gray.rows, gray.cols, CV_8SC1);
	// Mat Gy(gray.rows, gray.cols, CV_8SC1);
	Gx = Mat(gray.rows, gray.cols, CV_8SC1);
	Gy = Mat(gray.rows, gray.cols, CV_8SC1);
	Mat N(gray.rows, gray.cols, CV_8UC1);
	Mat out(gray.rows, gray.cols, CV_8UC1);
	Mat edges(gray.rows, gray.cols, CV_8UC1);

	Gx = Scalar(0);
	Gy = Scalar(0);
	N = Scalar(0);
	out = Scalar(0);
	
	char kernelx[3][3] = { { -1,  0,  1 },{ -2,  0,  2 },{ -1, 0, 1 } };
	char kernely[3][3] = { { -1, -2, -1 },{ 0,  0,  0 },{ 1, 2, 1 } };

	//gaussian filter
	medianBlur(gray, gray, 9);
	//GaussianBlur(gray, gray, Size(27, 27), 5, 5);
	imshow("Gray", gray);
	waitKey(2000);
	
	//intensity gradient matrices (Gx, Gy)
	for (int x = 1; x <= gray.cols - 2; x++) {
		for (int y = 1; y <= gray.rows - 2; y++) {
			Gx.at<char>(Point(x, y)) = (kernelx[0][0] * gray.at<char>(y - 1, x - 1)) + (kernelx[0][1] * gray.at<char>(y - 1, x)) + (kernelx[0][2] * gray.at<char>(y - 1, x + 1)) +
				(kernelx[1][0] * gray.at<char>(y, x - 1)) + (kernelx[1][1] * gray.at<char>(y, x)) + (kernelx[1][2] * gray.at<char>(y, x + 1)) +
				(kernelx[2][0] * gray.at<char>(y + 1, x - 1)) + (kernelx[2][1] * gray.at<char>(y + 1, x)) + (kernelx[2][2] * gray.at<char>(y + 1, x + 1));

			Gy.at<char>(Point(x, y)) = (kernely[0][0] * gray.at<char>(y - 1, x - 1)) + (kernely[0][1] * gray.at<char>(y - 1, x)) + (kernely[0][2] * gray.at<char>(y - 1, x + 1)) +
				(kernely[1][0] * gray.at<char>(y, x - 1)) + (kernely[1][1] * gray.at<char>(y, x)) + (kernely[1][2] * gray.at<char>(y, x + 1)) +
				(kernely[2][0] * gray.at<char>(y + 1, x - 1)) + (kernely[2][1] * gray.at<char>(y + 1, x)) + (kernely[2][2] * gray.at<char>(y + 1, x + 1));

		}//inner for
	}//outer for


	 //non max suppression as described by https://rosettacode.org/wiki/Canny_edge_detector
	uchar pa, pb, pg;

	for (int x = 1; x <= gray.cols - 2; x++) {
		for (int y = 1; y <= gray.rows - 2; y++) {
			pg = abs(Gx.at<char>(y, x)) + abs(Gy.at<char>(y, x));
			if (Gx.at<char>(y, x) == 0) {
				//theta = 90
				pa = abs(Gx.at<char>(y - 1, x)) + abs(Gy.at<char>(y, x));
				pb = abs(Gx.at<char>(y + 1, x)) + abs(Gy.at<char>(y + 1, x));
			}
			else {
				double ratio = Gy.at<char>(y, x) / Gx.at<char>(y, x);

				if (ratio <= -1.8 || ratio >= 1.8) {
					//theta = 90
					pa = abs(Gx.at<char>(y - 1, x)) + abs(Gy.at<char>(y - 1, x));
					pb = abs(Gx.at<char>(y + 1, x)) + abs(Gy.at<char>(y + 1, x));
				}
				else if (ratio >= -0.2 && ratio <= 0.2) {
					//theta = 0
					pa = abs(Gx.at<char>(y, x + 1)) + abs(Gy.at<char>(y, x + 1));
					pb = abs(Gx.at<char>(y, x - 1)) + abs(Gy.at<char>(y, x - 1));
				}
				else if (ratio >= 0.2 && ratio <= 1.8) {
					//theta = 45
					pa = abs(Gx.at<char>(y - 1, x + 1)) + abs(Gy.at<char>(y - 1, x + 1));
					pb = abs(Gx.at<char>(y + 1, x - 1)) + abs(Gy.at<char>(y + 1, x - 1));
				}
				else if (ratio >= -1.8 && ratio <= -0.2) {
					//theta = 135
					pa = abs(Gx.at<char>(y - 1, x - 1)) + abs(Gy.at<char>(y - 1, x - 1));
					pb = abs(Gx.at<char>(y + 1, x + 1)) + abs(Gy.at<char>(y + 1, x + 1));
				}
			}

			if ((pa <= pg && pg <= pb) || (pb >= pg && pg >= pa)) {
				if (pg > maximum) { maximum = pg; }
				if (pg < minimum) { minimum = pg; }
				N.at<uchar>(Point(x, y)) = pg;
			}
			else {
				N.at<uchar>(Point(x, y)) = 0;
			}
		}//inner for
	}//outer for

	//calculate thresholds for hystersis
	thresh1 =  (int)((minimum + maximum) / 5);
	thresh2 =  (int)((minimum + maximum) / 8);

	//Tracing edges with hysteresis as described by  https://rosettacode.org/wiki/Canny_edge_detector
	//*********************************************//

	unsigned char c = 1;
	for (int j = 1; j < N.rows - 1; j++) { // y
		for (int i = 1; i < N.cols - 1; i++) { //x
			if (N.at<uchar>(j, i) >= thresh1 && out.at<uchar>(j, i) == 0) { // trace edges
				out.at<uchar>(j, i) = MAX_BRIGHTNESS;
				int nedges = 1;
				edges.at<uchar>(j, i) = c;
				do {
					nedges--;
					//const int t = edges[nedges];
					for (int a = -1; a < 2; a++) {
						for (int b = -1; b < 2; b++) {
							if (a == 0 && b == 0) {
								continue;
							}
							else {
								if (i + a >= N.cols || j + b >= N.rows) {
									break;
								}
								Point neighbor = Point(i + a, j + b);
								if (N.at<uchar>(neighbor) >= thresh2 && out.at<uchar>(neighbor)) {
									out.at<uchar>(neighbor) = MAX_BRIGHTNESS;
									edges.at<uchar>(neighbor) = N.at<uchar>(neighbor);
									if (i + 1 == N.cols) {
										i = 1;
										j++;;
									}
									else {
										i++;
									}
									nedges++;
								}
							}
						}
					}
				} while (nedges > 0);
			}
			c++;
		}
	}
	return out;
}


/*Alex Soong's code*/
vector<Point> apply_harris(Mat src)
{
	vector<Point> returnVal;

	CascadeClassifier eye_cascade;
	eye_cascade.load("haarcascade_eye_tree_eyeglasses.xml");
	//eye_cascade.load("haarcascade_eye.xml");
	// vector to store eyes found
	vector<Rect> eyes;
	vector<Mat> cutout_eyes;
	// find eyes
	//Rect cake = new Rect() x, y, width height

	eye_cascade.detectMultiScale(src, eyes);
	cout << eyes.size() << endl;
	if (eyes.size() < 2) {
		cout << "well fuck this didn't work" << endl;
		return returnVal;
	}
	// cut out eyes from source image using coordinates from detectMultiScale
	Mat eyeL, eyeR;
	eyeL = Mat(src, eyes[0]);
	eyeR = Mat(src, eyes[1]);

	// get size of the cutout of the eyes
	Size sL = eyeL.size();
	int rowsL = (int)sL.height;
	int colsL = (int)sL.width;

	Size sR = eyeR.size();
	int rowsR = (int)sR.height;
	int colsR = (int)sR.width;

	// dictate region of interest
	Rect roiL = Rect(0, rowsL*0.3, colsL, rowsL*0.5);
	Rect roiR = Rect(0, rowsR*0.3, colsR, rowsR*0.5);

	// cut out the eyes from the original image 
	Mat cut_eyeL = Mat(eyeL, roiL);
	Mat cut_eyeR = Mat(eyeR, roiR);

	cvtColor(cut_eyeL, cut_eyeL, CV_BGR2GRAY);
	cvtColor(cut_eyeR, cut_eyeR, CV_BGR2GRAY);

	// store and return the eyes
	cutout_eyes.push_back(cut_eyeL);
	cutout_eyes.push_back(cut_eyeR);

	Mat dest, dest_norm, dest_norm_scaled;

	// detector parameters 
	int thresh = 150;
	int max_thresh = 255;
	int blockSize = 2;
	int apertureSize = 3;
	double k = 0.01;

	// detect corners
	for (int i = 0; i < cutout_eyes.size(); i++) {
		cornerHarris(cutout_eyes[i], dest, blockSize, apertureSize, k, BORDER_DEFAULT);

		// Normalize 
		normalize(dest, dest_norm, 0, 255, NORM_MINMAX, CV_32FC1, Mat());
		convertScaleAbs(dest_norm, dest_norm_scaled);

		// Draw circle around coners detected
		for (int j = 0; j < dest_norm.rows; j++)
		{
			for (int k = 0; i < dest_norm.cols; i++)
			{
				if ((int)dest_norm.at<float>(j, k) > thresh)
				{
					returnVal.push_back(Point(k + eyes[i].x, j + eyes[i].y +  0.3*eyes[i].height));
					//circle(dest_norm_scaled, Point(i, j), 5, Scalar(0), 2, 8, 0);
				}
			}
		}
	}
	return returnVal;

}


int main()
{
	/*
	if (-1 == gradientAlgo()) {
	cerr << "Failed to get camera feed" << endl;
	}
	*/

	//testSobel();
	//cornerDetectionWithCanny();
	VideoCapture cap;
	Mat image;
	if (!cap.open(0))
		return 0;
	cap >> image;


	vector<Point> cornerLocals = apply_harris(image);
	if (cornerLocals.size() > 0) {
		cout << "Yay" << endl;
		for (int i = 0; i < cornerLocals.size(); i++) {
			circle(image, cornerLocals[i], 5, Scalar(0), 2, 8, 0);
		}
	}
	else {
		cout << "NO FUCK SHIT AHHH" << endl;
	}

	imshow("Hello", image);
	waitKey(5000);
	//circleDetectWithCanny(image);
	cin.get();
	return 0;
}

