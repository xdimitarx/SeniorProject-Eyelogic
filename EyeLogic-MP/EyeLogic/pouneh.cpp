#include "pouneh.h"
#include <time.h> 

using namespace std;
using namespace cv;

VideoCapture cap;
Mat capture;
CascadeClassifier eyeDetector;
Point screenRes;
Mat ref_topLeft, ref_topRight, ref_bottomLeft, ref_bottomRight, ref_center;


//www.sciencedirect.com/science/article/pii/S2212017313005689
void majumder() {
	//Crop face area from Color face image
	//Convert faceCrop to grayscale
	//increase grayscale contrast
	//apply fast corner detector to gray scale image

	Mat colorface, grayface, gray3channel;

	Mat framegray;

	CascadeClassifier faceDetector;
	faceDetector.load("haarcascade_frontalface_default.xml");
	vector<Rect_<int>> face;

	//FastFeatureDetector ffd();
	Ptr<FastFeatureDetector> ffd = cv::FastFeatureDetector::create();

	//vector<vector<KeyPoint> > keyCorner;
	vector<KeyPoint> keyCorner;

	while (1) {
		cap >> capture;

		//crop face area from Color face image
		faceDetector.detectMultiScale(capture, face);

		if (face.size() > 0) {
			rectangle(capture, face[0], Scalar(0, 255, 255));
			colorface = Mat(capture, face[0]);

			//Convert faceCrop to grayscale
			cvtColor(colorface, grayface, CV_BGR2GRAY);
			//increase grayscale contrast
			//equalizeHist(grayface, grayface);

			//apply fast corner detector to gray scale image

			FAST(grayface, keyCorner, 30, true, FastFeatureDetector::TYPE_9_16);
			drawKeypoints(grayface, keyCorner, gray3channel, Scalar(0, 255, 0));

			cv::imshow("Key points", gray3channel);
			waitKey(500);
		}
	}

}



//function
//detect eye
//store rect around eye
//store corner
//compare corner location to next pass
//if within certain threshold, keep


Point findPupil(Mat eyeCrop)
{
	vector<Vec4i> hierarchy;
	vector<vector<cv::Point> > contours;
	findContours(eyeCrop, contours, hierarchy, CV_RETR_TREE, CV_CHAIN_APPROX_SIMPLE, cv::Point(0, 0));
	if (contours.size() )
	{
		double area = 0;
		int largest = 0;
		for (int i = 0; i < contours.size(); ++i)
		{
			double calculatedArea = contourArea(contours[i]);
			if (calculatedArea > area)
			{
				largest = i;
				area = calculatedArea;
			}
		}
		Moments mo = moments(contours[largest], false);
		Point eyeCenter = cv::Point(mo.m10 / mo.m00, mo.m01 / mo.m00);

		cv::Rect bounding = boundingRect(contours[largest]);
		//Point eyeCenter = cv::Point(cvRound(bounding.x + bounding.width / 2), cvRound(bounding.y + bounding.height / 2));	
		int eyeRadius = cvRound(bounding.height*1.05);

		//cv::circle(filtered, eyeCenter, eyeRadius, Scalar(122, 122, 122), 2);
		//rectangle(filtered, bounding, Scalar(122, 122, 122), 2, 8, 0);
		// imshow("eye", original);
		// waitKey(0);
		return eyeCenter;
	}
	cerr << "findPupil: COULDN'T DETERMINE IRIS" << endl;
	return Point(-1, -1);
}



void averageEyeCenterMethod() {
	// get pupil centers
	//calculate average
	//map to screen

	vector<Rect_<int>> eyes;
	vector<Vec3f> circles;
	Point averageLocal, screenMap;
	Point pupil[2];
	int distance ;
	Mat eyeCrop, eyeCropGray;
	//Rect roi;

	Point farLeft, farRight;
	Point referenceMean;


	//TIMER things
	time_t start;
	time(&start);
	time_t end;
	time(&end);
	auto diff = difftime(start,end);


	//get far left reference, image is flipped so value for farLeft is less than farRight
	while (1) {
		eyes.clear();
		getReferenceLeft();
		eyeDetector.detectMultiScale(ref_topLeft, eyes);
		if (eyes.size() >= 2) {
			for (int i = 0; i < 2; i++) {
				eyeCrop = Mat(ref_topLeft, eyes[i]);//crop eye region
				cvtColor(eyeCrop, eyeCropGray, CV_BGR2GRAY);
				equalizeHist(eyeCropGray, eyeCropGray);

				/*
				HoughCircles(eyeCropGray, circles, CV_HOUGH_GRADIENT, 1, eyeCrop.rows / 3 * 2, 10, 10, eyeCrop.rows / 10, eyeCrop.rows / 5);
				if (circles.size() == 1) {
					pupil[i] = Point(eyes[i].x +circles[0][0], eyes[i].y + circles[0][1]);
				}
				else {
					std::cout << "Fail 1: cirles.size() =  " << circles.size() << endl;	
					continue;
				}
				*/

				pupil[i] = findPupil(eyeCropGray);

				if (pupil[i].x == -1 && pupil[i].y == -1) {
					std::cout << "Fail 1: cirles.size() =  " << circles.size() << endl;
					i--;
					continue;
				}
				pupil[i].x += eyes[i].x;
				pupil[i].y += eyes[i].y;
			}
			farLeft = Point((pupil[0].x + pupil[1].x) / 2, (pupil[0].y + pupil[1].y) / 2);
			//farLeft = pupil[0];
			break;
		}
		else {
			std::cout << "Fail 1: eyes.size() =  " << eyes.size() << endl;
		}
		cv::waitKey(5000);
	}

	//get far right reference,,image is flipped so value for farRight is more than farLeft
	while (1) {
		eyes.clear();
		getReferenceRight();
		eyeDetector.detectMultiScale(ref_topRight, eyes);
		if (eyes.size() >= 2) {
			for (int i = 0; i < 2; i++) {
				eyeCrop = Mat(ref_topRight, eyes[i]);//crop eye region
				cvtColor(eyeCrop, eyeCropGray, CV_BGR2GRAY);
				equalizeHist(eyeCropGray, eyeCropGray);
				/*
				HoughCircles(eyeCropGray, circles, CV_HOUGH_GRADIENT, 1, eyeCrop.rows / 3 * 2, 10, 10, eyeCrop.rows / 10, eyeCrop.rows / 5);
				if (circles.size() == 1) {
					pupil[i] = Point(eyes[i].x + circles[0][0], eyes[i].y + circles[0][1]);
				}
				else {
					std::cout << "Fail 2: cirles.size() =  " << circles.size() << endl;
					continue;
				}
				*/
				pupil[i] = findPupil(eyeCropGray);
				if (pupil[i].x == -1 && pupil[i].y == -1) {
					std::cout << "Fail 2: cirles.size() =  " << circles.size() << endl;
					i--;
					continue;
				}
				pupil[i].x += eyes[i].x;
				pupil[i].y += eyes[i].y;
			}
			farRight = Point((pupil[0].x + pupil[1].x) / 2, (pupil[0].y + pupil[1].y) / 2);
			//farRight = pupil[0];
			break;
		}
		else {
			std::cout << "Fail 2: eyes.size() =  " << eyes.size() << endl;
		}
		cv:: waitKey(5000);
	}

	distance = farLeft.x - farRight.x;
	//reference mean might be wholly unecessary
	referenceMean = Point((farLeft.x + farRight.x) / 2, (farLeft.y + farRight.y) / 2);

	std::cout << "Distance: " << distance << endl;
	std::cout << "left x   " << farLeft.x << endl;
	std::cout << "right x   " << farRight.x << endl;

	cv::circle(ref_topRight, farRight, 3, Scalar(0, 255, 0));
	cv::circle(ref_topRight, farLeft, 3, Scalar(0, 255, 255));
	cv::circle(ref_topLeft, farRight, 3, Scalar(0, 255, 0));
	cv::circle(ref_topLeft, farLeft, 3, Scalar(0, 255, 255));
	//imshow("right", ref_topRight);
	//imshow("left", ref_topLeft);
	//waitKey(20000);
	if (distance == 0) {
		return;
	}

	//Forever loop to move cursor
	while (1) {
		time(&start);
		eyes.clear();
		cap >> capture;
		eyeDetector.detectMultiScale(capture, eyes);
		if (eyes.size() == 2) {
			for (int i = 0; i < eyes.size(); i++) {
				eyeCrop = Mat(capture, eyes[i]);//crop eye region
				cvtColor(eyeCrop, eyeCropGray, CV_BGR2GRAY);
				equalizeHist(eyeCropGray, eyeCropGray);

				//find pupil
				pupil[i] = findPupil(eyeCropGray);
				if (pupil[i].x == -1 && pupil[i].y == -1) {
					std::cout << "cant find current pupil for eye number " << i << endl;
					break; //breaks out of inner loop, the for loop, and continues in the while loop
				}
				pupil[i].x += eyes[i].x;
				pupil[i].y += eyes[i].y;
			}

			//calculate difference between current gaze, and far left, divide by num pixels, and multiply by resolution
			//average pupil location for current frame
			averageLocal = Point((int)((pupil[0].x + pupil[1].x) / 2), (int)((pupil[0].y + pupil[1].y) / 2));
			if (averageLocal.x < farRight.x || averageLocal.x > farLeft.x) {
				continue;
			}
			std::cout << "Current pupil center:  " << averageLocal.x << "\t" << averageLocal.y << endl;
			
			//circle(capture, farLeft, 3, Scalar(0, 255, 0));
			//circle(capture, farRight, 3, Scalar(255, 0, 0)); //left most circle if data is correct
			//circle(capture, averageLocal, 3, Scalar(0, 255, 255));

			//point on screen: 
			screenMap.x =(screenRes.x - ( ( averageLocal.x - farRight.x) * screenRes.x / distance ));
			screenMap.y = screenRes.y / 2;//( averageLocal.y - farRight.y) * screenRes.y / capture.rows;

			if (screenMap.x >= 0 && screenMap.y >= 0 && screenMap.x <= screenRes.x && screenMap.y <= screenRes.y) {
				//SetCursorPos(screenMap.x, screenMap.y);
				cv::circle(capture, screenMap, 5, Scalar(235,244,66) , -1);
				imshow("CAPTURE", capture);
				waitKey(2);
			}
			else {
				//imshow("cap", capture);
				//waitKey(5000);
				std::cout << "FUCK OUTOFBOUNDS COORDINATES" << endl;
			}
		}
		else {
			std::cout << "Did not find exactly 2 eyes in this frame" << endl;
			time(&end);
			auto seconds = difftime(end, start);
			std::cout << "Cycle:  " << seconds << endl;
			continue;
			//cv::imshow("cap", capture);
		}
		//if (cv:: waitKey(5)) { break; }
	}//while Loop
	time(&end);
	auto seconds = difftime(end, start);
	std::cout << "Cycle:  " << seconds << endl;

}

void lotsOfTheProgram() {
	//The following function:
	//	takes a frame from video input
	//	preprocesses the video input
	//	detects the eyes in the image
	//	detects the pupils in each eye
	//	detects the eyecorners for each eye
	//	desplays the findings on the frame

	int thresh = 100;
	int max_thresh = 255;
	int blockSize = 2;
	int apertureSize = 5;
	double k = 0.01;


	//Mat framegray, destLeft, destRight, leftCornerRoi, rightCornerRoi, edges;


	Mat framegray, eyeCropGray, eyeCropColor, destLeft, destRight, leftCornerRoi, rightCornerRoi;
	vector<Rect_<int>> eyes, eyes2;
	vector<Vec3f> circles;
	Point eyeCornerRight, eyeCornerLeft;


	while (1)
	{
		cap >> capture;
		cap >> capture;
		cv::cvtColor(capture, framegray, CV_BGR2GRAY);

		//increase contrast
		cv::equalizeHist(framegray, framegray);

		//increase contrast of grayscale image
		int change = 10;
		for (int i = 0; i < capture.cols; i++) {
			for (int j = 0; j < capture.rows; j++) {
				if (framegray.at<uchar>(cv::Point(i, j)) > 20) {
					if (framegray.at<uchar>(cv::Point(i, j)) < 255 - change) {
						framegray.at<uchar>(cv::Point(i, j)) += change;
					}
					else {
						framegray.at<uchar>(cv::Point(i, j)) = 255;
					}
				}
				else {
					if (framegray.at<uchar>(cv::Point(i, j)) > change)
					{
						framegray.at<uchar>(cv::Point(i, j)) -= change;
					}
					else {
						framegray.at<uchar>(cv::Point(i, j)) = 0;
					}
				}
			}
		}

		//detect Eyes
		eyeDetector.detectMultiScale(framegray, eyes);
		if (eyes.capacity() == 0) {
			cv::imshow("capture", capture);
			cv::waitKey(1000);
			continue;
		}
		for (int i = 0; i < eyes.size(); i++) {
			//draw rectangels around eye
			std::cout << "rectangles" << endl;
			rectangle(capture, Point(eyes[i].x, eyes[i].y), Point(eyes[i].x + eyes[i].width, eyes[i].y + eyes[i].height), Scalar(255, 0, 0));
			cv::Rect roiRect = cv::Rect(eyes[i].x, eyes[i].y, eyes[i].width, eyes[i].height);
			eyeCropGray = cv::Mat(framegray, roiRect);
			eyeCropColor = cv::Mat(capture, roiRect);

			//detect pupil for eye
			std::cout << "pupil" << endl;
			HoughCircles(eyeCropGray, circles, CV_HOUGH_GRADIENT, 1, eyeCropGray.rows / 3 * 2, 10, 10, eyeCropGray.rows / 10, eyeCropGray.rows / 5);
			std::cout << "cookie mookie" << endl;

			// Draw the pupils detected
			/*
			for (size_t i = 0; i < circles.size(); i++)
			{
			cv::Point center(cvRound(circles[i][0]), cvRound(circles[i][1]));
			int radius = cvRound(circles[i][2]);
			std::cout << "(" << center.x << ", " << center.y << ")" << endl;
			// circle center
			cv::circle(eyeCropColor, center, 3, Scalar(0, 255, 0), 1, 8, 0);
			// circle outline
			cv::circle(eyeCropColor, center, radius, Scalar(0, 0, 255), 1, 8, 0);
			}
			*/


			//if only one pupil
			//circles[0] is the pupil
			//circles[0][0] = eyecenter.x
			//circles[0][1] = eyecenter.y 
			//circles[0][2] = eyeradius

			std::cout << circles.size() << endl;
			if (circles.size() < 1) { continue; }

			int yOffset = (int)((circles[0][2] < circles[0][0]) ? circles[0][0] - circles[0][2] : 0);
			cv::Rect crop = cv::Rect(0, yOffset, eyeCropGray.cols, eyeCropGray.rows - yOffset);
			//eyeCropGray = Mat(eyeCropGray, crop);

			int leftbuffer;
			int rightbuffer;
			//int pupili] = { eyeCenter.x ,  eyeCenter.y ,  eyeRadius };

			if (0 == i) {
				leftbuffer = (int)(circles[0][0] / 2 - 1);
				rightbuffer = (int)((eyeCropGray.cols - circles[0][0]) / 2 + 2);
			}
			else {
				leftbuffer = (int)(circles[0][0] / 2 - 2);
				rightbuffer = (int)((eyeCropGray.cols - circles[0][0]) / 2);
			}

			while ((cvRound(circles[0][0]) - leftbuffer) <= 0) {
				leftbuffer--;
			}

			while ((cvRound(circles[0][0]) + rightbuffer) > eyeCropGray.cols) {
				rightbuffer--;
			}

			cv::Rect leftroi = cv::Rect(0, 0, (cvRound(circles[0][0]) - leftbuffer), eyeCropGray.rows);
			cv::Rect rightroi = cv::Rect((cvRound(circles[0][0]) + rightbuffer), 0, eyeCropGray.cols - (cvRound(circles[0][0]) + rightbuffer), eyeCropGray.rows);
			leftCornerRoi = cv::Mat(eyeCropGray, leftroi);
			rightCornerRoi = cv::Mat(eyeCropGray, rightroi);

			//cout << leftCornerRoi.rows << "\t\t" << leftCornerRoi.cols << "\t\t" << destLeft << "\t\t" << blockSize << "\t\t" << apertureSize << "\t\t" << k << endl;
			if (leftCornerRoi.rows && leftCornerRoi.cols) {
				cornerHarris(leftCornerRoi, destLeft, blockSize, apertureSize, k, BORDER_DEFAULT);
				normalize(destLeft, destLeft, 0, 255, NORM_MINMAX, CV_32FC1, Mat());
				convertScaleAbs(destLeft, destLeft);
			}

			//cout << rightCornerRoi.rows << "\t\t" << rightCornerRoi.cols << "\t\t" << destRight << "\t\t" << blockSize << "\t\t" << apertureSize << "\t\t" << k << endl;
			if (rightCornerRoi.rows && rightCornerRoi.cols) {
				cornerHarris(rightCornerRoi, destRight, blockSize, apertureSize, k, BORDER_DEFAULT);
				normalize(destRight, destRight, 0, 255, NORM_MINMAX, CV_32FC1, Mat());
				convertScaleAbs(destRight, destRight);
			}

			//variables to keep track of most likely coordinate to be corner
			cv::Point cornerLeft;
			cv::Point cornerRight;
			int verticalDistanceLeft = 10000;
			int verticalDistanceRight = 10000;

			for (int j = 0; j < destLeft.rows; j++) {
				for (int i = 0; i < destLeft.cols; i++) {
					if (thresh < destLeft.at<uchar>(j, i) && verticalDistanceLeft > abs(j - circles[0][1])) {
						cornerLeft = cv::Point(i, j);
						verticalDistanceLeft = abs((int)(j - circles[0][1]));
					}
				}
			}

			cv::circle(leftCornerRoi, cornerLeft, 3, Scalar(0), -1);

			for (int j = 0; j < destRight.rows; j++) {
				for (int i = 0; i < destRight.cols; i++) {
					if (thresh < destRight.at<uchar>(j, i) && verticalDistanceRight > abs(j - circles[0][1])) {
						cornerRight = cv::Point(i, j);
						verticalDistanceRight = abs((int)(j - circles[0][1]));
					}
				}
			}

			cv::circle(rightCornerRoi, cornerRight, 3, Scalar(0), -1);

			eyeCornerLeft = Point(cornerLeft.x + roiRect.x, cornerLeft.y + roiRect.y);
			eyeCornerRight = Point(cornerRight.x + rightroi.x + roiRect.x, cornerRight.y + roiRect.y);

			std::cout << "Left  corner: " << cornerLeft.x << "    " << cornerLeft.y << endl;
			std::cout << "Right corner: " << cornerRight.x << "    " << cornerRight.y << endl;

			cv::circle(capture, eyeCornerLeft, 3, Scalar(255, 0, 0), -1);
			cv::circle(capture, eyeCornerRight, 3, Scalar(0, 255, 0), -1);

			imshow("cap", capture);
			//cv::imshow("With corners", threechanelFramegray);
		}//for
		if (cv::waitKey(100) == 27) { break; }
	}// end forever loop

}

void getReferenceLeft() {
	SetConsoleDisplayMode(GetStdHandle(STD_OUTPUT_HANDLE), CONSOLE_FULLSCREEN_MODE, 0);

	int horizontal = screenRes.x;
	int vertical = screenRes.y;

	Mat cue(vertical, horizontal, CV_8UC3);
	Mat flash(vertical, horizontal, CV_8UC3);
	flash = Scalar(255, 255, 255);

	cue = Scalar(0, 0, 0);

	//Far Left
	cue = Scalar(0, 0, 0);
	cv::circle(cue, Point(horizontal / 50, vertical / 2), horizontal / 50, Scalar(0, 255, 0), -1);
	cv::imshow("", cue);
	cv::moveWindow("", 0, 0);
	waitKey(2000);
	cv::moveWindow("", 0, 0);
	cv::imshow("", flash);
	cap >> ref_topLeft;
	cap >> ref_topLeft;
	waitKey(50);
	cvDestroyWindow("");
}

void getReferenceRight() {
	SetConsoleDisplayMode(GetStdHandle(STD_OUTPUT_HANDLE), CONSOLE_FULLSCREEN_MODE, 0);

	int horizontal = screenRes.x;
	int vertical = screenRes.y;

	Mat cue(vertical, horizontal, CV_8UC3);
	Mat flash(vertical, horizontal, CV_8UC3);
	flash = Scalar(255, 255, 255);

	cue = Scalar(0, 0, 0);

	//Far Right
	cue = Scalar(0, 0, 0);
	cv::circle(cue, Point(horizontal - horizontal / 50, vertical / 2), horizontal / 50, Scalar(0, 255, 0), -1);
	cv::imshow("", cue);
	waitKey(2000);
	cv::moveWindow("", 0, 0);
	cv::imshow("", flash);
	cap >> ref_topRight;
	cap >> ref_topRight;
	waitKey(50);
	cv::moveWindow("", 0, 0);
	cvDestroyWindow("");
}

void getReferenceImages(){

	SetConsoleDisplayMode(GetStdHandle(STD_OUTPUT_HANDLE), CONSOLE_FULLSCREEN_MODE, 0);

	int horizontal = screenRes.x;
	int vertical = screenRes.y;

	Mat cue(vertical, horizontal, CV_8UC3);
	Mat flash(vertical, horizontal, CV_8UC3);
	flash = Scalar(255, 255, 255);

	cue = Scalar(0, 0, 0);

	//Top Left
	cue = Scalar(0, 0, 0);
	cv::circle(cue, Point(0 + horizontal / 20, 0 + horizontal / 20), horizontal / 20, Scalar(0, 255, 0), -1);
	imshow("", cue);
	waitKey(2000);
	imshow("", flash);
	waitKey(50);

	//Bottom Left
	cue = Scalar(0, 0, 0);
	cv::circle(cue, Point(0 + horizontal / 20, vertical - horizontal / 20), horizontal / 20, Scalar(0, 255, 0), -1);
	imshow("", cue);
	waitKey(2000);
	imshow("", flash);
	cap >> ref_topLeft;
	waitKey(50);

	//Center
	cue = Scalar(0, 0, 0);
	cv::circle(cue, Point(horizontal / 2, vertical / 2), horizontal / 20, Scalar(0, 255, 0), -1);
	imshow("", cue);
	waitKey(2000);
	imshow("", flash);
	cap >> ref_bottomLeft;
	waitKey(50);

	//Top Right
	cue = Scalar(0, 0, 0);
	cv::circle(cue, Point(horizontal - horizontal / 20, 0 + vertical / 20), horizontal / 20, Scalar(0, 255, 0), -1);
	imshow("", cue);
	waitKey(2000);
	imshow("", flash);
	cap >> ref_center;
	waitKey(50);

	//Bottom Right
	cue = Scalar(0, 0, 0);
	cv::circle(cue, Point(horizontal - horizontal / 20, vertical - vertical / 20), horizontal / 20, Scalar(0, 255, 0), -1);
	imshow("", cue);
	waitKey(2000);
	imshow("", flash);
	cap >> ref_topRight;
	waitKey(50);


	//hacky placeholder to get correct ordering of images
	cue = Scalar(0, 0, 0);
	imshow("", cue);
	waitKey(1000);
	cap >> ref_bottomRight;
	waitKey(500);

	imshow("top left", ref_topLeft);
	waitKey(1000);
	imshow("bottom left", ref_bottomLeft);
	waitKey(1000);
	imshow("center", ref_center);
	waitKey(1000);
	imshow("top right", ref_topRight);
	waitKey(1000);
	imshow("bottom right", ref_bottomRight);
	waitKey(60000);
}

/*
Function to do preliminary capture things
*/
bool startCap() {

	eyeDetector.load("haarcascade_eye_tree_eyeglasses.xml");

	//Get screen resolution
	RECT desktop;
	const HWND hDesktop = GetDesktopWindow();
	GetWindowRect(hDesktop, &desktop);
	screenRes.x = desktop.right;
	screenRes.y = desktop.bottom;


	bool videofeed = cap.open(0);
	if (videofeed) {
		cap.set(CV_CAP_PROP_FRAME_WIDTH, 1080);
		cap.set(CV_CAP_PROP_FRAME_HEIGHT, 1080);
	}

	waitKey(5000);

	//getReferenceImages();

	return videofeed;
}
