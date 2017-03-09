// Eyelogic.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"

#include "GradientAlgorithm.h"

void cannyEdge() {
	string file = "C:/Users/spark/Desktop/193/testframe.png";
	Mat image, gray;
	image = imread( file, CV_LOAD_IMAGE_COLOR);

	if (!image.data)                              // Check for invalid input
	{
		cout << "Could not open or find the image" << std::endl;
		return;
	}

	cvtColor(image, gray, CV_BGR2GRAY);
	Mat Gx(gray.rows, gray.cols, CV_8S);
	Mat Gy(gray.rows, gray.cols, CV_8S);
	Mat N(gray.rows, gray.cols, CV_8U);

	float kernelx[3][3] = { { -1,  0,  1 },{ -2,  0,  2 },{ -1, 0, 1 } };
	float kernely[3][3] = { { -1, -2, -1 },{ 0,  0,  0 },{ 1, 2, 1 } };


	//gaussian filter
	GaussianBlur(gray, gray, Size(9,9), 5, 5);

	//intensity gradient matrices (Gx, Gy)
	for (int x = 1; x <= gray.cols - 2; x++) {
		for (int y = 1; y <= gray.rows - 2; y++) {
			
			Gx.at<char>(Point( y,x))= (kernelx[0][0] * gray.at<char>(y - 1, x - 1)) + (kernelx[0][1] * gray.at<char>(y - 1, x)) + (kernelx[0][2] * gray.at<char>(y - 1, x + 1)) +
				(kernelx[1][0] * gray.at<char>(y, x - 1)) + (kernelx[1][1] * gray.at<char>(y, x)) + (kernelx[1][2] * gray.at<char>(y, x + 1)) +
				(kernelx[2][0] * gray.at<char>(y + 1, x - 1)) + (kernelx[2][1] * gray.at<char>(y + 1, x)) + (kernelx[2][2] * gray.at<char>(y + 1, x + 1));

			Gy.at<char>(Point(y, x)) = (kernely[0][0] * gray.at<char>(y - 1, x - 1)) + (kernely[0][1] * gray.at<char>(y, x - 1)) + (kernely[0][2] * gray.at<char>(y + 1, x - 1)) +
				(kernely[1][0] * gray.at<char>(y - 1, x)) + (kernely[1][1] * gray.at<char>(y, x)) + (kernely[1][2] * gray.at<char>(y + 1, x)) +
				(kernely[2][0] * gray.at<char>(y - 1, x + 1)) + (kernely[2][1] * gray.at<char>(y, x + 1)) + (kernely[2][2] * gray.at<char>(y + 1, x + 1));


		}//inner for
	}//outer for


	//non max suppression as described by https://rosettacode.org/wiki/Canny_edge_detector
	uchar pa, pb, pg;

	for (int x = 1; x <= gray.cols - 2; x++) {
		for (int y = 1; y <= gray.rows - 2; y++) {
			pg = abs(Gx.at<char>(y, x)) + Gy.at<char>(y, x);
			if (Gx.at<char>(y, x) == 0) {
				//theta = 90
				pa = abs(Gx.at<char>(y - 1, x)) + abs(Gy.at<char>(y, x));
				pb = abs(Gx.at<char>(y + 1, x)) + abs(Gy.at<char>(y + 1, x));
			}

			double ratio = Gy.at<char>(y, x) / Gx.at<char>(y, x);

			if (ratio < -1.8 || ratio > 1.8) {
				//theta = 90
				pa = abs(Gx.at<char>(y - 1, x)) + abs(Gy.at<char>(y - 1, x));
				pb = abs(Gx.at<char>(y + 1, x)) + abs(Gy.at<char>(y + 1, x));
			}
			else if (ratio > -0.2 && ratio < 0.2) {
				//theta = 0
				pa = abs(Gx.at<char>(y , x + 1)) + abs(Gy.at<char>(y, x + 1));
				pb = abs(Gx.at<char>(y , x - 1)) + abs(Gy.at<char>(y, x - 1));
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


			if ((pa < pg && pg < pb) || (pb > pg && pg > pa)){
				N.at<uchar>(Point(y, x)) = pg;
			}
			else {
				N.at<uchar>(Point(y, x)) = 0;
			}
		}//inner for
	}//outer for


	//Tracing edges with hysteresis as described by  https://rosettacode.org/wiki/Canny_edge_detector


	
	imshow("Hello motherfucker", gray);
	waitKey(2000);

}


int main()
{
	/*
	if (-1 == gradientAlgo()) {
		cerr << "Failed to get camera feed" << endl;
	}
	*/

	//testSobel();

	cannyEdge();
	return 0;
}

