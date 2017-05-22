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

void getReferenceImage(std::vector<cv::Rect_<int>>& eyes, void(*function)(void), cv::Mat & refImage, std::string name, cv::Point & _far) {
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

void getReferenceImage2(std::vector<cv::Rect_<int>>& eyes, cv::Point center, cv::Mat & refImage, std::string name, cv::Point & _far , int distanceBetweenPupil) {
	//pupil vector to keep track of pupil location for each eye
	cv::Point pupil[2];
	//pre-processing Matrices
	cv::Mat eyeCrop, eyeCropGray;


	SetConsoleDisplayMode(GetStdHandle(STD_OUTPUT_HANDLE), CONSOLE_FULLSCREEN_MODE, 0);

	int horizontal = screenRes.x;
	int vertical = screenRes.y;

	//Loop until adequate data is collected for callibration
	while (1) {
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
		cv::waitKey(1000);
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
			distanceBetweenPupil = abs(pupil[0].x - pupil[1].x) + abs(pupil[0].y - pupil[1].y);
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

void calibrate(std::vector<cv::Rect_<int>>& eyes , cv::Point top, cv::Point bottom, cv::Point farLeft, cv::Point farRight, 
				cv::Point distance, cv::Point referenceMean, cv::Rect_<int>& rightEyeBounds, cv::Rect_<int>& leftEyeBounds, int& distanceBetweenPupil) {
	getReferenceImage2(eyes, cv::Point(screenRes.x / 2, 0), ref_top, "Top", top, distanceBetweenPupil);
	getReferenceImage2(eyes, cv::Point(screenRes.x / 2, screenRes.y - screenRes.x / 50), ref_bottom, "Center", bottom, distanceBetweenPupil);
	getReferenceImage2(eyes, cv::Point(screenRes.x / 50, screenRes.y / 2), ref_farLeft, "Left", farLeft, distanceBetweenPupil);
	getReferenceImage2(eyes, cv::Point(screenRes.x - screenRes.x / 50, screenRes.y / 2), ref_farRight, "Right", farRight , distanceBetweenPupil);


	distance.x = farLeft.x - farRight.x;
	distance.y = bottom.y - top.y;
	if (distance.x == 0 || distance.y == 0) {
		return;
	}
	//reference mean might be wholly unecessary
	referenceMean = cv::Point((farLeft.x + farRight.x) / 2, (top.y + bottom.y) / 2);

	//redefining rectangle search region for eyes
	updateBoundaryWindows(eyes, rightEyeBounds, leftEyeBounds);
}

void averageEyeCenterMethod() {
	// get pupil centers
	//calculate average
	//map to screen
	//TODO: make robust against head movement.
	int distanceBetweenPupils = 0 , tempDistanceBetweenPupils;
	std::vector<cv::Rect_<int>> eyes, eyeLeft, eyeRight;
	std::vector<cv::Vec3f> circles;
	cv::Point averageLocal, screenMap, destinationOld(-1, -1), destinationNew, delta(0, 0), direction(0, 0);
	cv::Point pupil[2];
	cv::Point distance;
	cv::Mat eyeCrop, eyeCropGray;
	//Rect roi;
	cv::Rect leftEyeBounds, rightEyeBounds;
	//let leftEye  be the user's left eye, therefore, for analytical purposes, 
	//it is the right-most eye in the matrix

	cv::Point farLeft, farRight, top, bottom;
	cv::Point referenceMean;

	calibrate(eyes, top, bottom, farLeft, farRight, distance, referenceMean, rightEyeBounds, leftEyeBounds, distanceBetweenPupils);

	//Forever loop to move cursor
	while (1) {	
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
			std::cout << "Failure of the eyes *sadnesssss* " << std::endl;
			//get new rectangles for things??? 
			//TODO: Need to figure out how to deal with this
			//for now just recalibrate ....
			calibrate(eyes, top, bottom, farLeft, farRight, distance, referenceMean, rightEyeBounds, leftEyeBounds, distanceBetweenPupils );
			continue;
		}
		if (cv::waitKey(2) == 'r' || cv::waitKey(2) == 'R') {
			//Recalibrate
			calibrate(eyes, top, bottom, farLeft, farRight, distance, referenceMean, rightEyeBounds, leftEyeBounds, distanceBetweenPupils);
		}
		else if (cv::waitKey(2) == 27) {
			//exit program
			return;
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
				std::cout << (int)(averageLocal.x < farRight.x) << std::endl;
				std::cout << (int)(averageLocal.x > farLeft.x) << std::endl;
				std::cout << (int)(averageLocal.y < top.y) << std::endl;
				std::cout << (int)(averageLocal.y > bottom.y) << std::endl;
				//TODO: head moving things
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
			screenMap.y = ( averageLocal.y - top.y) * screenRes.y /distance.y; 

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


	cv::Mat framegray, eyeCropGray, eyeCropColor, destLeft, destRight, leftCornerRoi, rightCornerRoi;
	std::vector<cv::Rect_<int>> eyes, eyes2;
	std::vector<cv::Vec3f> circles;
	cv::Point eyeCornerRight, eyeCornerLeft;


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
			std::cout << "rectangles" << std::endl;
			rectangle(capture, cv::Point(eyes[i].x, eyes[i].y), cv::Point(eyes[i].x + eyes[i].width, eyes[i].y + eyes[i].height), cv::Scalar(255, 0, 0));
			cv::Rect roiRect = cv::Rect(eyes[i].x, eyes[i].y, eyes[i].width, eyes[i].height);
			eyeCropGray = cv::Mat(framegray, roiRect);
			eyeCropColor = cv::Mat(capture, roiRect);

			//detect pupil for eye
			std::cout << "pupil" << std::endl;
			HoughCircles(eyeCropGray, circles, CV_HOUGH_GRADIENT, 1, eyeCropGray.rows / 3 * 2, 10, 10, eyeCropGray.rows / 10, eyeCropGray.rows / 5);
			std::cout << "cookie mookie" << std::endl;

			// Draw the pupils detected
			/*
			for (size_t i = 0; i < circles.size(); i++)
			{
			cv::Point center(cvRound(circles[i][0]), cvRound(circles[i][1]));
			int radius = cvRound(circles[i][2]);
			std::cout << "(" << center.x << ", " << center.y << ")" << endl;
			// circle center
			cv::circle(eyeCropColor, center, 3,cv::Scalar(0, 255, 0), 1, 8, 0);
			// circle outline
			cv::circle(eyeCropColor, center, radius,cv::Scalar(0, 0, 255), 1, 8, 0);
			}
			*/


			//if only one pupil
			//circles[0] is the pupil
			//circles[0][0] = eyecenter.x
			//circles[0][1] = eyecenter.y 
			//circles[0][2] = eyeradius

			std::cout << circles.size() << std::endl;
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
				cornerHarris(leftCornerRoi, destLeft, blockSize, apertureSize, k, cv::BORDER_DEFAULT);
				normalize(destLeft, destLeft, 0, 255, cv::NORM_MINMAX, CV_32FC1, cv::Mat());
				convertScaleAbs(destLeft, destLeft);
			}

			//cout << rightCornerRoi.rows << "\t\t" << rightCornerRoi.cols << "\t\t" << destRight << "\t\t" << blockSize << "\t\t" << apertureSize << "\t\t" << k << endl;
			if (rightCornerRoi.rows && rightCornerRoi.cols) {
				cornerHarris(rightCornerRoi, destRight, blockSize, apertureSize, k, cv::BORDER_DEFAULT);
				normalize(destRight, destRight, 0, 255, cv::NORM_MINMAX, CV_32FC1, cv::Mat());
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

			cv::circle(leftCornerRoi, cornerLeft, 3, cv::Scalar(0), -1);

			for (int j = 0; j < destRight.rows; j++) {
				for (int i = 0; i < destRight.cols; i++) {
					if (thresh < destRight.at<uchar>(j, i) && verticalDistanceRight > abs(j - circles[0][1])) {
						cornerRight = cv::Point(i, j);
						verticalDistanceRight = abs((int)(j - circles[0][1]));
					}
				}
			}

			cv::circle(rightCornerRoi, cornerRight, 3, cv::Scalar(0), -1);

			eyeCornerLeft = cv::Point(cornerLeft.x + roiRect.x, cornerLeft.y + roiRect.y);
			eyeCornerRight = cv::Point(cornerRight.x + rightroi.x + roiRect.x, cornerRight.y + roiRect.y);

			std::cout << "Left  corner: " << cornerLeft.x << "    " << cornerLeft.y << std::endl;
			std::cout << "Right corner: " << cornerRight.x << "    " << cornerRight.y << std::endl;

			cv::circle(capture, eyeCornerLeft, 3, cv::Scalar(255, 0, 0), -1);
			cv::circle(capture, eyeCornerRight, 3, cv::Scalar(0, 255, 0), -1);

			imshow("cap", capture);
			//cv::imshow("With corners", threechanelFramegray);
		}//for
		if (cv::waitKey(100) == 27) { break; }
	}// end forever loop

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
		cap.set(CV_CAP_PROP_FRAME_WIDTH, 1200);
		cap.set(CV_CAP_PROP_FRAME_HEIGHT, 1080);
	}

	cv::waitKey(5000);

	//getReferenceImages();

	return videofeed;
}
