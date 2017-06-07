#include "Backup.h"
#include <time.h> 
#include <iostream>
#include <windows.h>
#include <vector>

cv::VideoCapture cap;
cv::Mat capture;
cv::CascadeClassifier eyeDetector;
cv::CascadeClassifier faceDetector;
cv::Point screenRes;
cv::Mat ref_farLeft, ref_farRight, ref_top, ref_bottom;

//template variables
cv::Point templateCoordinates;
cv::Mat foreheadTemplate;

cv::Point findPupil(cv::Mat eyeCrop){
	//Image preprocessing
	cv::Mat eyeCropGray;
	cvtColor(eyeCrop, eyeCropGray, CV_BGR2GRAY);
	cv::equalizeHist(eyeCropGray, eyeCropGray);
	cv::add(eyeCropGray, cv::Scalar(50), eyeCropGray);
	cv::equalizeHist(eyeCropGray, eyeCropGray);
	cv::threshold(eyeCropGray, eyeCropGray, 10, 255, cv::THRESH_BINARY);
	cv::GaussianBlur(eyeCropGray, eyeCropGray, cv::Size(9, 9), 0, 0);
	cv::Mat erodeElement = getStructuringElement(cv::MORPH_ELLIPSE, cv::Size(4, 4));
	cv::dilate(eyeCropGray, eyeCropGray, erodeElement);

	std::vector<cv::Vec4i> hierarchy;
	std::vector<std::vector<cv::Point> > contours;
	findContours(eyeCropGray, contours, hierarchy, CV_RETR_TREE, CV_CHAIN_APPROX_SIMPLE, cv::Point(0, 0));
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
		cv::Moments mo = moments(contours[largest], false);
		cv::Point eyeCenter = cv::Point((int)(mo.m10 / mo.m00), (int)(mo.m01 / mo.m00));

		cv::Rect bounding = boundingRect(contours[largest]);
		//Point eyeCenter = cv::Point(cvRound(bounding.x + bounding.width / 2), cvRound(bounding.y + bounding.height / 2));	
		int eyeRadius = cvRound(bounding.height*1.05);
		return eyeCenter;
	}
	std::cerr << "findPupil: COULDN'T DETERMINE IRIS" << std::endl;
	return cv::Point(-1, -1);
}

int getReferenceImage(std::vector<cv::Rect_<int>>& eyes, cv::Point center, cv::Mat & refImage, std::string name, cv::Point & _far , int distanceBetweenPupil) {
	//pupil vector to keep track of pupil location for each eye
	cv::Point pupil[2];
	//pre-processing Matrices
	cv::Mat eyeCrop, eyeCropGray;
	short releaseESC = 1;
	short captureWindowShowing = 0;

	SetConsoleDisplayMode(GetStdHandle(STD_OUTPUT_HANDLE), CONSOLE_FULLSCREEN_MODE, 0);

	int horizontal = screenRes.x;
	int vertical = screenRes.y;
	//Loop until adequate data is collected for callibration
	while (1) {
		//Check if ESC pressed
		if ((GetKeyState(VK_ESCAPE) & 0x8000)) {
			if (releaseESC != GetKeyState(VK_ESCAPE)) {
				// if previous toggle state was 0 and current state is 1 OR previous toggle state was 1 and current state is 0
				releaseESC = (GetKeyState(VK_ESCAPE) << 7) >> 7;
				//exit program
				return 1;
			}
		}

		//clears vector for eye detection
		eyes.clear();
		//grab relevant reference image
		cv::Mat cue(vertical, horizontal, CV_8UC3);
		cv::Mat flash(vertical, horizontal, CV_8UC3);
		flash = cv::Scalar(255, 255, 255);
		cue = cv::Scalar(0, 0, 0);
		//Center Bottom
		cue = cv::Scalar(0, 0, 0);
		cv::circle(cue, center, horizontal / 50, cv::Scalar(0, 255, 0), -1);
		cv::imshow("", cue);
		cv::moveWindow("", 0, 0);
		cv::waitKey(2000);
		cv::moveWindow("", 0, 0);
		cv::imshow("", flash);
		cap >> refImage;
		cap >> refImage;
		cv::waitKey(50);
		cvDestroyWindow("");

		//Detect eyes in relevant region
		eyeDetector.detectMultiScale(refImage, eyes);
		if (eyes.size() == 2) {
			//Loop to operate stuff for each eye
			for (int i = 0; i < 2; i++) {
				//Preprocessing for pupil detection
				eyeCrop = cv::Mat(refImage, eyes[i]);//crop eye region
				//cvtColor(eyeCrop, eyeCropGray, CV_BGR2GRAY);
				//cv::equalizeHist(eyeCropGray, eyeCropGray);

				//pupil detection
				pupil[i] = findPupil(eyeCrop);
				//Making sure pupil is detected - if not, repeat process
				if (pupil[i].x == -1 && pupil[i].y == -1) {
					std::cout << "Fail " << name << " : Cannot find pupil" << std::endl;
					i--;
					continue;
				}
				//Make pupil coordinates relative to whole capture image and not just eye bounding box
				pupil[i].x += eyes[i].x;
				pupil[i].y += eyes[i].y;
			}
			//Calculate average pupil location for looking at the reference point
			distanceBetweenPupil = abs(pupil[0].x - pupil[1].x) + abs(pupil[0].y - pupil[1].y);
			_far = cv::Point((pupil[0].x + pupil[1].x) / 2, (pupil[0].y + pupil[1].y) / 2);
			break;
		}
	}
	return 0;
}

void updateBoundaryWindows(std::vector<cv::Rect_<int>>& eyes , cv::Rect_<int>& rightEyeBounds , cv::Rect_<int>& leftEyeBounds) {
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
	rightEyeBounds.x = std::max(rightEyeBounds.x - (rightEyeBounds.width / 2), 0);
	rightEyeBounds.y = std::max(rightEyeBounds.y - (rightEyeBounds.height / 2), 0);
	leftEyeBounds.x = std::min(leftEyeBounds.x - (leftEyeBounds.width / 2), ref_top.cols);
	leftEyeBounds.y = std::max(leftEyeBounds.y - (leftEyeBounds.height / 2), 0);

	//Adjust size of bounding box
	rightEyeBounds.width = 2 * rightEyeBounds.width;
	leftEyeBounds.width = 2 * leftEyeBounds.width;
	if (leftEyeBounds.x + leftEyeBounds.width >ref_top.cols) {
		leftEyeBounds.width = ref_top.cols - leftEyeBounds.x;
	}

	//This bit assumes that the eyes wont hit boundaries
	//TODO: boundary checking for y coordinates/height
	rightEyeBounds.height = 2 * rightEyeBounds.height;
	leftEyeBounds.height = 2 * leftEyeBounds.height;
}

int calibrate(std::vector<cv::Rect_<int>>& eyes , cv::Point& top, cv::Point& bottom, cv::Point& farLeft, cv::Point& farRight, 	cv::Point& distance, cv::Point& referenceMean, cv::Rect_<int>& rightEyeBounds, cv::Rect_<int>& leftEyeBounds, int& distanceBetweenPupil) {
	//make forehead template. use it if eyeaverage moves outside of bounding box
	foreheadTemplate = createForeheadTemplate();
	
	//do reference image things
	if ((getReferenceImage(eyes, cv::Point(screenRes.x / 2, 0), ref_top, "Top", top, distanceBetweenPupil)) == 1) { return 1; }
	if ((getReferenceImage(eyes, cv::Point(screenRes.x / 2, screenRes.y - screenRes.x / 50), ref_bottom, "Bottom", bottom, distanceBetweenPupil)) == 1) { return 1; }
	if ((getReferenceImage(eyes, cv::Point(screenRes.x / 50, screenRes.y / 2), ref_farLeft, "Left", farLeft, distanceBetweenPupil)) == 1) { return 1; }
	if ((getReferenceImage(eyes, cv::Point(screenRes.x - screenRes.x / 50, screenRes.y / 2), ref_farRight, "Right", farRight, distanceBetweenPupil)) == 1) { return 1; }

	distance.x = farLeft.x - farRight.x;
	distance.y = bottom.y - top.y;
	if (distance.x == 0 || distance.y == 0) {
		return 0;
	}
	//reference mean might be wholly unecessary
	referenceMean = cv::Point((farLeft.x + farRight.x) / 2, (top.y + bottom.y) / 2);

	//redefining rectangle search region for eyes
	updateBoundaryWindows(eyes, rightEyeBounds, leftEyeBounds);
	return 0;
}

void headMoveOut() {
	//when the head moves out away from screen
	//Distance betweent he pupils decreases
	//so new 
}

void averageEyeCenterMethod() {
	// get pupil centers
	//calculate average
	//map to screen
	//TODO: make robust against head movement.
	short releaseR = 1;
	short releaseESC = 1;
	short releaseSpace = 1;
	short captureWindowShowing = 0;
	int distanceBetweenPupils = 0 , tempDistanceBetweenPupils;
	std::vector<cv::Rect_<int>> eyes, eyeLeft, eyeRight;
	std::vector<cv::Vec3f> circles;
	cv::Point averageLocal, screenMap, destinationOld(-1, -1), destinationNew, delta(0, 0), direction(0, 0);
	cv::Point pupil[2];
	cv::Point distance, newTemplateLocation;
	cv::Mat eyeCrop, eyeCropGray;
	//Rect roi;
	cv::Rect leftEyeBounds, rightEyeBounds; //Rectangles where the eye must be (after Boundary windows are updated)
	//let leftEye  be the user's left eye, therefore, for analytical purposes, 
	//it is the right-most eye in the matrix

	cv::Point farLeft, farRight, top, bottom;
	cv::Point referenceMean;

	if ((calibrate(eyes, top, bottom, farLeft, farRight, distance, referenceMean, rightEyeBounds, leftEyeBounds, distanceBetweenPupils)) == 1) { return; }

	//Forever loop to move cursor
	while (1) {	
		//Check if R pressed
		if ((GetKeyState(0x52) & 0x8000)) {
			if (releaseR != GetKeyState(0x52)) {
				// if previous toggle state was 0 and current state is 1 OR previous toggle state was 1 and current state is 0
				releaseR = (GetKeyState(0x52) << 7) >> 7;

				//Recalibrate
				calibrate(eyes, top, bottom, farLeft, farRight, distance, referenceMean, rightEyeBounds, leftEyeBounds, distanceBetweenPupils);
			}
		}
 
		//Check if ESC pressed
		if ((GetKeyState(VK_ESCAPE) & 0x8000)) {
			if (releaseESC != GetKeyState(VK_ESCAPE)) {
				// if previous toggle state was 0 and current state is 1 OR previous toggle state was 1 and current state is 0
				releaseESC = (GetKeyState(VK_ESCAPE) << 7) >> 7; 
				//exit program
				return;
			}
		}

		//Check if Spacebar pressed
		if ((GetKeyState(VK_SPACE) & 0x8000)) {
 			if (releaseSpace != GetKeyState(VK_SPACE)) {
				// if previous toggle state was 0 and current state is 1 OR previous toggle state was 1 and current state is 0
				releaseSpace = (GetKeyState(VK_SPACE) << 7) >> 7;
				//exit program
      			if (captureWindowShowing == 1) {
					//destroy capture window
					cv::destroyWindow("CAPTURE");
					captureWindowShowing = 0;
				}
				else {
					cv::imshow("CAPTURE", capture);
					cv::waitKey(1);
					captureWindowShowing = 1;
				}
			}
		}
		if (captureWindowShowing == 1) {
			imshow("CAPTURE", capture);
			cv::waitKey(1);
		}

		cap >> capture;	
		//detect eyes in subboxes
		eyeLeft.clear();
		eyeRight.clear();
		eyeDetector.detectMultiScale(cv::Mat(capture, leftEyeBounds ), eyeLeft);
		eyeDetector.detectMultiScale(cv::Mat(capture, rightEyeBounds), eyeRight);

		cvtColor(capture, capture, CV_BGR2GRAY);
		if (eyeLeft.size() != 1 || eyeRight.size() != 1) {
			eyeLeft.clear();
			eyeRight.clear();
			cv::equalizeHist(capture, capture);
			cv::rectangle(capture, leftEyeBounds, cv::Scalar(0xff, 0xff, 0x0));
			cv::rectangle(capture, rightEyeBounds, cv::Scalar(0xde, 0xe8, 0x0c));
		}

		if (eyeLeft.size() == 1 && eyeRight.size() == 1) {
			eyes.clear();
			eyeLeft[0].x += leftEyeBounds.x;
			eyeLeft[0].y += leftEyeBounds.y;
			eyeRight[0].x += rightEyeBounds.x;
			eyeRight[0].y += rightEyeBounds.y;
			eyes.push_back(eyeLeft[0]);
			eyes.push_back(eyeRight[0]);
			tempDistanceBetweenPupils = std::abs(eyeLeft[0].x - eyeRight[0].x) + std::abs(eyeLeft[0].y - eyeRight[0].y);
			if (tempDistanceBetweenPupils > 5 + distanceBetweenPupils) {
				//Head moved in towards screen
			}
			else if (tempDistanceBetweenPupils < distanceBetweenPupils - 5) {
				//head moved out away from screen
			}
			else {
				distanceBetweenPupils = tempDistanceBetweenPupils;
			}
			updateBoundaryWindows(eyes, rightEyeBounds, leftEyeBounds);
		}
		else {
			//get new rectangles for eye things??? 
			//TODO: Need to figure out how to deal with this 
			// ^^ maybe check if head is detected: if yes, probably a blink, can just continue. Otherwise, may need to recalibrate. 
			//		Maybe recalibrate after several consecutive failures of the eyes
			//for now just recalibrrate ....
			//calibrate(eyes, top, bottom, farLeft, farRight, distance, referenceMean, rightEyeBounds, leftEyeBounds, distanceBetweenPupils );
			continue;
		}

		cvtColor(capture, capture, CV_GRAY2BGR);
		cv::rectangle(capture, leftEyeBounds, cv::Scalar(0xff, 0xff, 0x0));
		cv::rectangle(capture, rightEyeBounds, cv::Scalar(0xde, 0xe8, 0x0c));

			//Detect pupil for each image
		for (int i = 0; i < eyes.size(); i++) {
			
			eyeCrop = cv::Mat(capture, eyes[i]);//crop eye region
			//find pupil
			pupil[i] = findPupil(eyeCrop);
			cvtColor(eyeCrop, eyeCropGray, CV_BGR2GRAY);
			cvtColor(eyeCropGray, eyeCropGray, CV_GRAY2BGR);
			if (pupil[i].x == -1 && pupil[i].y == -1) {
				std::cout << "pupil not found" << std::endl;
				break; //breaks out of inner loop, the for loop, and continues in the while loop
			}
			cv::circle(eyeCropGray, pupil[i], 3, cv::Scalar(0, 255, 0));
			//Make pupil coordinates relative to whole capture image and not just eye bounding box
			pupil[i].x += eyes[i].x;
			pupil[i].y += eyes[i].y;

		}

			//calculate difference between current gaze, and far left, divide by num pixels, and multiply by resolution
			//average pupil location for current frame
			averageLocal = cv::Point((int)((pupil[0].x + pupil[1].x) / 2), (int)((pupil[0].y + pupil[1].y) / 2));
			cv::circle(capture, averageLocal, 2, cv::Scalar(0x0c, 0x8f, 0xe8), -1);
			cv::rectangle(capture, cv::Rect(farRight.x, top.y, farLeft.x - farRight.x, bottom.y - top.y), cv::Scalar(0x80, 0x0, 0xff));
			if (averageLocal.x < farRight.x || averageLocal.x > farLeft.x || averageLocal.y < top.y || averageLocal.y > bottom.y) {	
				//Eye average is outside of bounds. Most likely happens because head moves
				//Template matching to find out how far head moved, and adjust bounding box accordingly
				newTemplateLocation = matchTemplate(foreheadTemplate);
				farRight.x += newTemplateLocation.x - templateCoordinates.x;
				farLeft.x += newTemplateLocation.x - templateCoordinates.x;
				top.y += newTemplateLocation.y - templateCoordinates.y;
				bottom.y += newTemplateLocation.y - templateCoordinates.y;
				templateCoordinates = newTemplateLocation;
				continue;
			}


			//destination point on screen: to develop gradual moving of cursor
			//TODO: Improve... it's still jumpy
			
			destinationNew.x = (screenRes.x - ((averageLocal.x - farRight.x) * screenRes.x / distance.x));
			destinationNew.y = (averageLocal.y - top.y) * screenRes.y / distance.y; 

			if (destinationOld != destinationNew) {
				destinationOld = destinationNew;
				delta = cv::Point((destinationNew.x - screenMap.x) / screenRes.x * 80, (destinationNew.y - screenMap.y) / screenRes.y * 80);
				if (delta.x > 0) { direction.x = 1; }
				else { direction.x = -1; }
				if (delta.y > 0) { direction.y = 1; }
				else { direction.y = -1; }
			}
			
			delta = direction;
			if (direction.x > 0) {
				screenMap.x = std::min(destinationNew.x, screenMap.x + delta.x);
			}
			else {
				screenMap.x = std::max(destinationNew.x, screenMap.x + delta.x);
			}
			if (direction.y > 0) {
				screenMap.y = std::min(destinationNew.y, screenMap.y + delta.y);
			}
			else {
				screenMap.y = std::max(destinationNew.y, screenMap.y + delta.y);
			}

			//point on screen: 
			screenMap.x =(screenRes.x - (( averageLocal.x - farRight.x) * screenRes.x / distance.x ));
			screenMap.y = ( averageLocal.y - top.y) * screenRes.y /distance.y;

			//Enforce screen resolution as boundaries for movement of cursor
			if (screenMap.x >= 0 && screenMap.y >= 0 && screenMap.x <= screenRes.x && screenMap.y <= screenRes.y) {
				SetCursorPos(screenMap.x, screenMap.y);
				//draw circle instead of moving cursor
				cv::circle(capture, screenMap, 5, cv::Scalar(235,244,66) , -1);

			}
			else {
				//Out of bounds coordinates
			}
	}//while Loop
}


/*
Function to do preliminary capture things
*/
bool startCap() {
	eyeDetector.load("haarcascade_eye_tree_eyeglasses.xml");
	faceDetector.load("haarcascade_frontalface_default.xml");
	//Get screen resolution
	RECT desktop;
	const HWND hDesktop = GetDesktopWindow();
	GetWindowRect(hDesktop, &desktop);
	screenRes.x = desktop.right;
	screenRes.y = desktop.bottom;


	bool videofeed = cap.open(0);
	if (videofeed) {
		cap.set(CV_CAP_PROP_FRAME_WIDTH, 1920);
		cap.set(CV_CAP_PROP_FRAME_HEIGHT, 1080);
	}

	return videofeed;
}


cv::Mat createForeheadTemplate() {
	cv::Mat frame;
	cap >> frame;
	std::vector<cv::Rect_<int>> faces;
	faceDetector.detectMultiScale(frame, faces); //Need to apply minimum size
	if (faces.size() == 0)
	{
		//logError("Error in insertFrame: No faces detected.");
		return cv::Mat(); //No Faces!!
	}
	cv::Rect faceCrop = faces[0];
	faceCrop.height = 0.2 * faceCrop.height;
	cv::rectangle(frame, faceCrop, cv::Scalar(0, 0, 255));

	templateCoordinates = cv:: Point(faceCrop.x, faceCrop.y);
	return cv::Mat(frame, faceCrop);
}

cv::Point matchTemplate(cv::Mat templ) {
	cv::Mat frame, result;
	cap >> frame;

	/// Create the result matrix
	int result_cols = frame.cols - templ.cols + 1;
	int result_rows = frame.rows - templ.rows + 1;

	result.create(result_rows, result_cols, CV_32FC1);

	/// Do the Matching and Normalize
	matchTemplate(frame, templ, result, CV_TM_CCORR_NORMED);
	normalize(result, result, 0, 1, cv::NORM_MINMAX, -1, cv::Mat());

	/// Localizing the best match with minMaxLoc
	double minVal; double maxVal; cv::Point minLoc; cv::Point maxLoc;
	cv::Point matchLoc;

	minMaxLoc(result, &minVal, &maxVal, &minLoc, &maxLoc, cv::Mat());
	matchLoc = maxLoc;

	cv::waitKey(1);

	return matchLoc;
}