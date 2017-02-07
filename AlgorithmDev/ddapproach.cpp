#include "EyeLogic.h"

using namespace std;
using namespace cv;

bool EyeLogicAlg::approximateAngle(Point_<int> *xyValues)
{
	vector<Mat> filtered;
	for (int i = 0; i < captureEyes.size(); ++i)
	{
		Mat* tobeFiltered = &captureEyes.at(i);
		cvtColor(*tobeFiltered, *tobeFiltered, CV_BGR2GRAY);
		equalizeHist(*tobeFiltered, *tobeFiltered);
		
		//blur(*tobeFiltered, *tobeFiltered, CVSize(70,70));
		add(*tobeFiltered, Scalar(-10,-10,-10), *tobeFiltered);
		threshold(*tobeFiltered, *tobeFiltered, 122, 255, THRESH_BINARY);
		GaussianBlur(*tobeFiltered, *tobeFiltered, CvSize(3,3), 0, 0);
		vector<Vec3f> circles;
		cout << "here" << endl;
		HoughCircles(*tobeFiltered, circles, CV_HOUGH_GRADIENT, 1, 1);
		cout << "here2" << endl;
		cout << circles.capacity() << endl;
		Point center(cvRound(circles[0][0]), cvRound(circles[0][1]));
		int radius = cvRound(circles[0][2]);
		circle(*tobeFiltered, center, radius, Scalar(0,0,255), 3, 8, 0);
		filtered.push_back(*tobeFiltered);
		imshow(to_string(i), filtered.at(i));
	}
	waitKey(0);
}

void EyeLogicAlg::angleToMouseMov()
{

}