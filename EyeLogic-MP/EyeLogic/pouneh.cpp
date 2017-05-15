#include "pouneh.h"
#include <time.h> 



cv::VideoCapture cap;
cv::Mat capture;
cv::CascadeClassifier eyeDetector;
cv::Point screenRes;
cv::Mat ref_topLeft, ref_topRight, ref_bottomLeft, ref_bottomRight, ref_center;
cv::Mat ref_farLeft, ref_farRight, ref_top, ref_bottom;




cv::Point findPupil(cv::Mat eyeCrop)
{
	std::vector<cv::Vec4i> hierarchy;
	std::vector<std::vector<cv::Point> > contours;
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

void getReferenceImage(std::vector<cv::Rect_<int>>& eyes , void(* function)(void) , cv::Mat & refImage, std::string name , cv::Point & _far) {
	//pupil vector to keep track of pupil location for each eye
	cv::Point pupil[2];
	//pre-processing Matrices
	cv::Mat eyeCrop, eyeCropGray;

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
				eyeCrop = cv::Mat(refImage, eyes[i]);//crop eye region
				cvtColor(eyeCrop, eyeCropGray, CV_BGR2GRAY);
				cv::equalizeHist(eyeCropGray, eyeCropGray);
				//pupil detection
				pupil[i] = findPupil(eyeCropGray);
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
			_far = cv::Point((pupil[0].x + pupil[1].x) / 2, (pupil[0].y + pupil[1].y) / 2);
			break;
		}
	}
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

void averageEyeCenterMethod() {
	// get pupil centers
	//calculate average
	//map to screen
	//TODO: add in functionality for Y axis movement
	//TODO: make robust against head movement.

	std::vector<cv::Rect_<int>> eyes, eyeLeft, eyeRight;
	std::vector<cv::Vec3f> circles;
	cv::Point averageLocal, screenMap , destinationOld(-1,-1), destinationNew, delta(0,0), direction(0,0);
	cv::Point pupil[2];
	cv::Point distance ;
	cv::Mat eyeCrop, eyeCropGray;
	//Rect roi;
	cv::Rect leftEyeBounds, rightEyeBounds;
	//let leftEye  be the user's left eye, therefore, for analytical purposes, 
	//it is the right-most eye in the matrix

	cv::Point farLeft, farRight, top, bottom;
	cv::Point referenceMean;

	getReferenceImage(eyes, getReferenceTop		, ref_top		, "Top"		, top);
	getReferenceImage(eyes, getReferenceBottom	, ref_bottom	, "Center"	, bottom);
	getReferenceImage(eyes, getReferenceLeft	, ref_farLeft	, "Left"	, farLeft);
	getReferenceImage(eyes, getReferenceRight	, ref_farRight	, "Right"	, farRight);

	distance.x = farLeft.x - farRight.x;
	distance.y = bottom.y - top.y;
	if (distance.x == 0 || distance.y == 0) {
		return;
	}
	//reference mean might be wholly unecessary
	referenceMean = cv::Point((farLeft.x + farRight.x) / 2, (top.y + bottom.y) / 2);

	//redefining rectangle search region for eyes
	updateBoundaryWindows(eyes, rightEyeBounds, leftEyeBounds);
	int i = 0;
	//Forever loop to move cursor
	while (1) {	
		std::cout << "Loop number:  " << i++ << std::endl;
		cap >> capture;	
		//detect eyes in subboxes
		eyeLeft.clear();
		eyeRight.clear();
		eyeDetector.detectMultiScale(cv::Mat(capture, leftEyeBounds ), eyeLeft);
		eyeDetector.detectMultiScale(cv::Mat(capture, rightEyeBounds), eyeRight);
		if (eyeLeft.size() == 1 && eyeRight.size() == 1) {
			eyes.clear();
			eyeLeft[0].x += leftEyeBounds.x;
			eyeLeft[0].y += leftEyeBounds.y;
			eyeRight[0].x += rightEyeBounds.x;
			eyeRight[0].y += rightEyeBounds.y;
			eyes.push_back(eyeLeft[0]);
			eyes.push_back(eyeRight[0]);
			updateBoundaryWindows(eyes, rightEyeBounds, leftEyeBounds);
		}
		else {
			std::cout << "Failure of the eyes *sadnesssss* " << std::endl;
			//get new rectangles for things??? 
			//TODO: Need to figure out how to deal with this
			continue;
		}

			//Detect pupil for each image
			for (int i = 0; i < eyes.size(); i++) {
				//Preprocessing steps for pupil detection
				eyeCrop = cv::Mat(capture, eyes[i]);//crop eye region
				cvtColor(eyeCrop, eyeCropGray, CV_BGR2GRAY);
				equalizeHist(eyeCropGray, eyeCropGray);
				//find pupil
				pupil[i] = findPupil(eyeCropGray);
				//Toss frame if pupil isn't detected, and go to next frame
				if (pupil[i].x == -1 && pupil[i].y == -1) {
					std::cout << "pupil not found" << std::endl;
					break; //breaks out of inner loop, the for loop, and continues in the while loop
				}
				//Make pupil coordinates relative to whole capture image and not just eye bounding box
				pupil[i].x += eyes[i].x;
				pupil[i].y += eyes[i].y;
			}

			//calculate difference between current gaze, and far left, divide by num pixels, and multiply by resolution
			//average pupil location for current frame
			averageLocal = cv::Point((int)((pupil[0].x + pupil[1].x) / 2), (int)((pupil[0].y + pupil[1].y) / 2));
			if (averageLocal.x < farRight.x || averageLocal.x > farLeft.x || averageLocal.y < top.y || averageLocal.y > bottom.y) {
				//TODO: head moving things
				std::cout << (int)(averageLocal.x < farRight.x) << std::endl;
				std::cout << (int)(averageLocal.x > farLeft.x) << std::endl;
				std::cout << (int)(averageLocal.y < top.y) << std::endl;
				std::cout << (int)(averageLocal.y > bottom.y) << std::endl;
				continue;
			}

			//destination point on screen: to develop gradual moving of cursor
			//TODO: Improve... it's still jumpy
			
			destinationNew.x = (screenRes.x - ((averageLocal.x - farRight.x) * screenRes.x / distance.x));
			destinationNew.y = (averageLocal.y - top.y) * screenRes.y / distance.y; //screenRes.y / 2;

			if (destinationOld != destinationNew) {
				destinationOld = destinationNew;
				delta = cv::Point((destinationNew.x - screenMap.x) / screenRes.x * 80, (destinationNew.y - screenMap.y) / screenRes.y * 80);
				if (delta.x > 0) { direction.x = 1; }
				else { direction.x = -1; }
				if (delta.y > 0) { direction.y = 1; }
				else { direction.y = -1; }
			}

			
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
			screenMap.x =(screenRes.x - ( ( averageLocal.x - farRight.x) * screenRes.x / distance.x ));
			//TODO: implement y shifting 
			screenMap.y = ( averageLocal.y - top.y) * screenRes.y /distance.y; //screenRes.y / 2;//


			//Enforce screen resolution as boundaries for movement of cursor
			if (screenMap.x >= 0 && screenMap.y >= 0 && screenMap.x <= screenRes.x && screenMap.y <= screenRes.y) {
				//SetCursorPos(screenMap.x, screenMap.y);
				//draw circle instead of moving cursor
				//TODO: Make program good enough with escape sequence so that we can actually use the cursor
				//		instead of the circle drawn below
				cv::circle(capture, screenMap, 5, cv::Scalar(235,244,66) , -1);
				imshow("CAPTURE", capture);
				cv::waitKey(1);
			}
			else {
				std::cout << "FUCK OUTOFBOUNDS COORDINATES" << std::endl;
				std::cout << "\tScreenMap.x: " << screenMap.x << "\tScreenMap.y: " << screenMap.y << std::endl;
			}

	}//while Loop
}


void getReferenceTop() {
	SetConsoleDisplayMode(GetStdHandle(STD_OUTPUT_HANDLE), CONSOLE_FULLSCREEN_MODE, 0);

	int horizontal = screenRes.x;
	int vertical = screenRes.y;

	cv::Mat cue(vertical, horizontal, CV_8UC3);
	cv::Mat flash(vertical, horizontal, CV_8UC3);
	flash = cv::Scalar(255, 255, 255);

	cue = cv::Scalar(0, 0, 0);

	//CenterTop
	cue = cv::Scalar(0, 0, 0);
	cv::circle(cue, cv::Point(horizontal / 2, 0), horizontal / 50, cv::Scalar(0, 255, 0), -1);
	cv::imshow("", cue);
	cv::moveWindow("", 0, 0);
	cv::waitKey(1000);
	cv::moveWindow("", 0, 0);
	cv::imshow("", flash);
	cap >> ref_top;
	cap >> ref_top;
	cv::waitKey(50);
	cvDestroyWindow("");
}

void getReferenceBottom() {
	SetConsoleDisplayMode(GetStdHandle(STD_OUTPUT_HANDLE), CONSOLE_FULLSCREEN_MODE, 0);

	int horizontal = screenRes.x;
	int vertical = screenRes.y;

	cv::Mat cue(vertical, horizontal, CV_8UC3);
	cv::Mat flash(vertical, horizontal, CV_8UC3);
	flash = cv::Scalar(255, 255, 255);

	cue = cv::Scalar(0, 0, 0);

	//Center Bottom
	cue = cv::Scalar(0, 0, 0);
	cv::circle(cue, cv::Point(horizontal / 2, vertical - horizontal/50), horizontal / 50, cv::Scalar(0, 255, 0), -1);
	cv::imshow("", cue);
	cv::moveWindow("", 0, 0);
	cv::waitKey(1000);
	cv::moveWindow("", 0, 0);
	cv::imshow("", flash);
	cap >> ref_bottom;
	cap >> ref_bottom;
	cv::waitKey(50);
	cvDestroyWindow("");
}

void getReferenceLeft() {
	SetConsoleDisplayMode(GetStdHandle(STD_OUTPUT_HANDLE), CONSOLE_FULLSCREEN_MODE, 0);

	int horizontal = screenRes.x;
	int vertical = screenRes.y;

	cv::Mat cue(vertical, horizontal, CV_8UC3);
	cv::Mat flash(vertical, horizontal, CV_8UC3);
	flash = cv::Scalar(255, 255, 255);

	cue = cv::Scalar(0, 0, 0);

	//Far Left
	cue = cv::Scalar(0, 0, 0);
	cv::circle(cue, cv::Point(horizontal / 50, vertical / 2), horizontal / 50, cv::Scalar(0, 255, 0), -1);
	cv::imshow("", cue);
	cv::moveWindow("", 0, 0);
	cv::waitKey(1000);
	cv::moveWindow("", 0, 0);
	cv::imshow("", flash);
	cap >> ref_farLeft;
	cap >> ref_farLeft;
	cv::waitKey(50);
	cvDestroyWindow("");
}

void getReferenceRight() {
	SetConsoleDisplayMode(GetStdHandle(STD_OUTPUT_HANDLE), CONSOLE_FULLSCREEN_MODE, 0);

	int horizontal = screenRes.x;
	int vertical = screenRes.y;

	cv::Mat cue(vertical, horizontal, CV_8UC3);
	cv::Mat flash(vertical, horizontal, CV_8UC3);
	flash = cv::Scalar(255, 255, 255);

	cue = cv::Scalar(0, 0, 0);

	//Far Right
	cue =cv::Scalar(0, 0, 0);
	cv::circle(cue, cv::Point(horizontal - horizontal / 50, vertical / 2), horizontal / 50,cv::Scalar(0, 255, 0), -1);
	cv::imshow("", cue);
	cv::waitKey(1000);
	cv::moveWindow("", 0, 0);
	cv::imshow("", flash);
	cap >> ref_farRight;
	cap >> ref_farRight;
	cv::waitKey(50);
	cv::moveWindow("", 0, 0);
	cvDestroyWindow("");
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

	cv::waitKey(5000);

	//getReferenceImages();

	return videofeed;
}
