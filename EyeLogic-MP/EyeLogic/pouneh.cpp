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
		Point eyeCenter = cv::Point((int)(mo.m10 / mo.m00), (int)(mo.m01 / mo.m00));

		cv::Rect bounding = boundingRect(contours[largest]);
		//Point eyeCenter = cv::Point(cvRound(bounding.x + bounding.width / 2), cvRound(bounding.y + bounding.height / 2));	
		int eyeRadius = cvRound(bounding.height*1.05);

		return eyeCenter;
	}
	cerr << "findPupil: COULDN'T DETERMINE IRIS" << endl;
	return Point(-1, -1);
}

void getReferenceImage(vector<Rect_<int>>& eyes , void(* function)(void) , Mat & refImage, string name , Point & _far) {
	//pupil vector to keep track of pupil location for each eye
	Point pupil[2];
	//pre-processing Matrices
	Mat eyeCrop, eyeCropGray;

	//Loop until adequate data is collected for callibration
	while (1) {
		//clears vector for eye detection
		eyes.clear();
		//Calls relevant reference image function using function pointers
		(*function)();
		//Detect eyes in relevant region
		eyeDetector.detectMultiScale(refImage, eyes);
		if (eyes.size() == 2) {
			//Loop to operate stuff for each eye
			for (int i = 0; i < 2; i++) {
				//Preprocessing for pupil detection
				eyeCrop = Mat(refImage, eyes[i]);//crop eye region
				cvtColor(eyeCrop, eyeCropGray, CV_BGR2GRAY);
				cv::equalizeHist(eyeCropGray, eyeCropGray);
				//pupil detection
				pupil[i] = findPupil(eyeCropGray);
				//Making sure pupil is detected - if not, repeat process
				if (pupil[i].x == -1 && pupil[i].y == -1) {
					std::cout << "Fail " << name << " : Cannot find pupil" << endl;
					i--;
					continue;
				}
				//Make pupil coordinates relative to whole capture image and not just eye bounding box
				pupil[i].x += eyes[i].x;
				pupil[i].y += eyes[i].y;
			}
			//Calculate average pupil location for looking at the reference point
			_far = Point((pupil[0].x + pupil[1].x) / 2, (pupil[0].y + pupil[1].y) / 2);
			break;
		}
	}
}

void updateBoundaryWindows(vector<Rect_<int>>& eyes , Rect_<int>& rightEyeBounds , Rect_<int>& leftEyeBounds) {
	//determine which eye in the "eyes" vector is the left/right eye
	//Because the camera flips the image, the User's right eye is the leftmost in the image
	if (eyes[0].x < eyes[1].x) {
		rightEyeBounds = eyes[0];
		leftEyeBounds = eyes[1];
	}
	else {
		leftEyeBounds = eyes[0];
		rightEyeBounds = eyes[1];
	}

	//redefining rectangle search region for eyes
	//Determine x and y coordinate of each bounding box (make it bigger so that if eye moves, it can still be detected)
	//Boundary check so that rectangle stays within bounds of image
	rightEyeBounds.x = max(rightEyeBounds.x - (rightEyeBounds.width / 2), 0);
	rightEyeBounds.y = max(rightEyeBounds.y - (rightEyeBounds.height / 2), 0);
	leftEyeBounds.x = min(leftEyeBounds.x - (leftEyeBounds.width / 2), ref_topRight.cols);
	leftEyeBounds.y = max(leftEyeBounds.y - (leftEyeBounds.height / 2), 0);

	//Adjust size of bounding box
	rightEyeBounds.width = 2 * rightEyeBounds.width;
	leftEyeBounds.width = 2 * leftEyeBounds.width;
	if (leftEyeBounds.x + leftEyeBounds.width >ref_topRight.cols) {
		leftEyeBounds.width = ref_topRight.cols - leftEyeBounds.x;
	}

	//This bit assumes that the eyes wont hit boundaries
	//TODO: boundary checking for y coordinates/height
	rightEyeBounds.height = 2 * rightEyeBounds.height;
	leftEyeBounds.height = 2 * leftEyeBounds.height;
}

void averageEyeCenterMethod() {
	// get pupil centers
	//calculate average
	//map to screen
	//TODO: add in functionality for Y axis movement
	//TODO: make robust against head movement.

	vector<Rect_<int>> eyes, eyeLeft, eyeRight;
	vector<Vec3f> circles;
	Point averageLocal, screenMap , destinationOld(-1,-1), destinationNew, delta(0,0), direction(0,0); 
	Point pupil[2];
	int distance ;
	Mat eyeCrop, eyeCropGray;
	//Rect roi;
	Rect leftEyeBounds, rightEyeBounds;
	//let leftEye  be the user's left eye, therefore, for analytical purposes, 
	//it is the right-most eye in the matrix

	Point farLeft, farRight;
	Point referenceMean;

	getReferenceImage(eyes, getReferenceLeft, ref_topLeft, "Left", farLeft);
	getReferenceImage(eyes, getReferenceRight, ref_topRight, "Right", farRight);

	distance = farLeft.x - farRight.x;
	if (distance == 0) {
		return;
	}
	//reference mean might be wholly unecessary
	referenceMean = Point((farLeft.x + farRight.x) / 2, (farLeft.y + farRight.y) / 2);

	//redefining rectangle search region for eyes
	updateBoundaryWindows(eyes, rightEyeBounds, leftEyeBounds);

	//Forever loop to move cursor
	while (1) {
		eyes.clear();
		eyeLeft.clear();
		eyeRight.clear();
		cap >> capture;	
		eyeDetector.detectMultiScale(Mat(capture, leftEyeBounds ), eyeLeft);
		eyeDetector.detectMultiScale(Mat(capture, rightEyeBounds), eyeRight);
		if (eyeLeft.size() == 1 && eyeRight.size() == 1) {
			eyeLeft[0].x += leftEyeBounds.x;
			eyeLeft[0].y += leftEyeBounds.y;
			eyeRight[0].x += rightEyeBounds.x;
			eyeRight[0].y += rightEyeBounds.y;
			eyes.push_back(eyeLeft[0]);
			eyes.push_back(eyeRight[0]);
			updateBoundaryWindows(eyes, rightEyeBounds, leftEyeBounds);
		}
		else {
			//cout << "Failure of the eyes *sadnesssss* " << endl;
			//get new rectangles for things??? Need to figure out how to deal with this
			continue;
		}

		//eyeDetector.detectMultiScale(capture, eyes);
		if (eyes.size() == 2) {
			//Detect pupil for each image
			for (int i = 0; i < eyes.size(); i++) {
				//Preprocessing steps for pupil detection
				eyeCrop = Mat(capture, eyes[i]);//crop eye region
				cvtColor(eyeCrop, eyeCropGray, CV_BGR2GRAY);
				equalizeHist(eyeCropGray, eyeCropGray);
				//find pupil
				pupil[i] = findPupil(eyeCropGray);
				//Toss frame if pupil isn't detected, and go to next frame
				if (pupil[i].x == -1 && pupil[i].y == -1) {
					break; //breaks out of inner loop, the for loop, and continues in the while loop
				}
				//Make pupil coordinates relative to whole capture image and not just eye bounding box
				pupil[i].x += eyes[i].x;
				pupil[i].y += eyes[i].y;
			}

			//calculate difference between current gaze, and far left, divide by num pixels, and multiply by resolution
			//average pupil location for current frame
			averageLocal = Point((int)((pupil[0].x + pupil[1].x) / 2), (int)((pupil[0].y + pupil[1].y) / 2));
			if (averageLocal.x < farRight.x || averageLocal.x > farLeft.x) {
				continue;
			}

			//destination point on screen: to develop gradual moving of cursor
			//TODO: Improve... it's still jumpy
			destinationNew.x = (screenRes.x - ((averageLocal.x - farRight.x) * screenRes.x / distance));
			destinationNew.y = screenRes.y / 2;//( averageLocal.y - farRight.y) * screenRes.y / capture.rows;
			if (destinationOld != destinationNew) {
				destinationOld = destinationNew;
				delta = Point((destinationNew.x - screenMap.x) / screenRes.x * 80, (destinationNew.y - screenMap.y) / screenRes.y * 80);
				if (delta.x > 0) { direction.x = 1; }
				else { direction.x = -1; }
				if (delta.y > 0) { direction.y = 1; }
				else { direction.y = -1; }
			}
			if (direction.x > 0) {
				screenMap.x = min(destinationNew.x, screenMap.x + delta.x);
			}
			else {
				screenMap.x = max(destinationNew.x, screenMap.x + delta.x);
			}
			if (direction.y > 0) {
				screenMap.y = min(destinationNew.y, screenMap.y + delta.y);
			}
			else {
				screenMap.y = max(destinationNew.y, screenMap.y + delta.y);
			}

			//point on screen: 
			screenMap.x =(screenRes.x - ( ( averageLocal.x - farRight.x) * screenRes.x / distance ));
			//TODO: implement y shifting 
			screenMap.y = screenRes.y / 2;//( averageLocal.y - farRight.y) * screenRes.y / capture.rows;

			//Enforce screen resolution as boundaries for movement of cursor
			if (screenMap.x >= 0 && screenMap.y >= 0 && screenMap.x <= screenRes.x && screenMap.y <= screenRes.y) {
				//SetCursorPos(screenMap.x, screenMap.y);
				//draw circle instead of moving cursor
				//TODO: Make program good enough with escape sequence so that we can actually use the cursor
				//		instead of the circle drawn below
				cv::circle(capture, screenMap, 5, Scalar(235,244,66) , -1);
				imshow("CAPTURE", capture);
				waitKey(1);
			}
			else {
				std::cout << "FUCK OUTOFBOUNDS COORDINATES" << endl;
			}
		}
		else {
			std::cout << "Did not find exactly 2 eyes in this frame" << endl;
			continue;
		}
	}//while Loop
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
	waitKey(1000);
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
	waitKey(1000);
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
