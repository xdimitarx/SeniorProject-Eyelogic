#include "stdafx.h"

#include "EyeLogic.hpp"

Mat loadImageAtPath(string path)
{
    Mat result = imread(path, CV_LOAD_IMAGE_COLOR);
    return result;
}

//Pouneh Aghababazadeh (whole function for getting reference images)
void ImgFrame::getReferenceImages()
{   
    //SetConsoleDisplayMode(GetStdHandle(STD_OUTPUT_HANDLE), CONSOLE_FULLSCREEN_MODE, 0);   
    VideoCapture cap;
    if (!cap.open(0)) {
        cerr << "FAIL" << endl;
        return ;
    }
    
    int horizontal = 0;
    int vertical = 0;

    horizontal = screenResolution.x;
    vertical = screenResolution.y;
    
    Mat cue(vertical, horizontal, CV_8UC3);
    Mat flash(vertical, horizontal, CV_8UC3);
    flash = Scalar(255, 255, 255);

	cue = Scalar(0, 0, 0);

    //Top Left
    cue = Scalar(0, 0, 0);
    circle(cue, cv::Point(0 + horizontal / 20, 0 + horizontal / 20), horizontal / 20, Scalar(0, 255, 0), -1);
    imshow("", cue);
    //waitKey(2000);
    imshow("", flash);
	//waitKey(50);
    
    //Bottom Left
    cue = Scalar(0, 0, 0);
    circle(cue, cv::Point(0 + horizontal / 20, vertical - horizontal / 20), horizontal / 20, Scalar(0, 255, 0), -1);
    //imshow("", cue);
    //waitKey(2000);
	 //imshow("", flash);
	cap >> ref_topLeft;
	//waitKey(50);
    
    //Center
    cue = Scalar(0, 0, 0);
    circle(cue, cv::Point(horizontal / 2, vertical / 2), horizontal / 20, Scalar(0, 255, 0), -1);
	//imshow("", cue);
	//waitKey(2000);
	//imshow("", flash);
	cap >> ref_bottomLeft;
	//waitKey(50);
    
    //Top Right
    cue = Scalar(0, 0, 0);
    circle(cue, cv::Point(horizontal - horizontal / 20, 0 + vertical / 20), horizontal / 20, Scalar(0, 255, 0), -1);
	//imshow("", cue);
	//waitKey(2000);
	//imshow("", flash);
	cap >> ref_center;
	//waitKey(50);
    
    //Bottom Right
    cue = Scalar(0, 0, 0);
    circle(cue, cv::Point(horizontal - horizontal / 20, vertical - vertical / 20), horizontal / 20, Scalar(0, 255, 0), -1);
	//imshow("", cue);
	//waitKey(2000);
	//imshow("", flash);
	cap >> ref_topRight;
	//waitKey(50);


	//hacky placeholder to get correct ordering of images
	cue = Scalar(0, 0, 0);
	//imshow("", cue);
	//waitKey(1000);
	cap >> ref_bottomRight;
	//waitKey(500);

	//imshow("", ref_topLeft);
	//waitKey(1000);
	//imshow("", ref_bottomLeft);
	//waitKey(1000);
	//imshow("", ref_center);
	//waitKey(1000);
	//imshow("", ref_topRight);
	//waitKey(1000);
	//imshow("", ref_bottomRight);
	//waitKey(1000);
}

Eye::Eye(){return;};

Eye::Eye(string pathToClassifier, bool left)
{
    detector.load(pathToClassifier.c_str());
    leftEye = left;
}

Eye::~Eye()
{
    
}

void Eye::createEyeVector(){
	//There will be 2 eyeVectors, one based on the left corner and one based on the right corner.
	//This function needs to be modified/fixed to reflect this change

	
    if(leftEye){
        eyeVector.x = eyeCenter.x - eyeCornerLeft.x;
        eyeVector.y = eyeCenter.y - eyeCornerLeft.y;
    }
    else {
        eyeVector.x = eyeCornerRight.x - eyeCenter.x;
        eyeVector.y = eyeCornerRight.y - eyeCenter.y;
    }
	

}

void Eye::setEyeVector(float x, float y){
    eyeVector.x = (int) x;
    eyeVector.y = (int) y;
}
bool Eye::detectKeyFeatures(Mat input)
{
	faceHalf = input;
    vector<cv::Rect_<int> > eyesCoord;
    
    detector.detectMultiScale(input, eyesCoord, 1.2, 3, 0, CvSize(40,20));
    if(eyesCoord.size() <= 0)
    {
        string text = "right";
        if(leftEye)
        {
            text = "left";
        }
        cerr << "detectKeyFeatures-" << text << ": NO EYE DETECTED" << endl;
        return false;
    }
    original = Mat(input, eyesCoord[0]);
	eyeLocationOnImageHalf = eyesCoord[0];
    
    //Cutout Eyebrow
    cv::Rect eyebrowCrop = cv::Rect(0, (int)(original.rows*0.4), (int)(original.cols), (int)(original.rows*0.5));

    original = Mat(original, eyebrowCrop);

	//calculate offset from original eye box coordinates to the CROPPED VERSION
	//Same width (number of cols/starting x)
	//starting y = eyeLocationOnImageHalf.y + (int)(original.rows*0.4)
	//vertical height =  (int)(original.rows*0.5))
	//basicall top 40% and bottom 10% are cut off
    
    cvtColor(original, filtered, CV_BGR2GRAY);
    equalHist();
    
    binaryThreshForIris();
    Mat erodeElement = getStructuringElement( MORPH_ELLIPSE, cv::Size(4,4));
    dilate(filtforIris,filtforIris,erodeElement);

    //applyGaussian();    
    equalHist();
    addLighting(-80);
    binaryThreshForSc();
//    imshow("after dilate", filtforIris);
//    imshow("filtered", filtered);
//    waitKey(10);
    
    if(findPupil())
    {
        blink = false;
        if(findEyeCorner()){
            createEyeVector();
        }
        
    }
    else
    {
        blink = true;
    }
    return true;
}

bool Eye::getBlink()
{
    return blink;
}

void Eye::equalHist()
{
    equalizeHist(filtered,filtered);
}

void Eye::addLighting(int intensity)
{
    add(filtered, Scalar(intensity,intensity,intensity), filtered);
}

void Eye::binaryThreshForSc()
{
    threshold(filtered, filtered, 10, 255, THRESH_BINARY);
}

void Eye::binaryThreshForIris()
{
    threshold(filtered, filtforIris, 10, 255, THRESH_BINARY_INV);
}

void Eye::applyGaussian()
{
    GaussianBlur(filtforIris, filtforIris, CvSize(3,3), 0, 0);
}



bool Eye::findPupil()
{
    vector<Vec4i> hierarchy;
    vector<vector<cv::Point> > contours;
    findContours(filtforIris, contours, hierarchy, CV_RETR_TREE, CV_CHAIN_APPROX_SIMPLE, cv::Point(0,0));
    if(contours.size() != 0)
    {
        double area = 0;
        int largest = 0;
        for (int i = 0; i < contours.size(); ++i)
        {
            double calculatedArea = contourArea(contours[i]);
            if(calculatedArea > area)
            {
                largest = i;
                area = calculatedArea;
            }
        }
        cv::Rect bounding = boundingRect(contours[largest]);
        eyeCenter = cv::Point(cvRound(bounding.x+bounding.width/2), cvRound(bounding.y+bounding.height/2));
        eyeRadius = cvRound(bounding.height*1.05);
        cv::circle(filtered, eyeCenter, eyeRadius, Scalar(122,122,122), 2);
        //rectangle(filtered, bounding,  Scalar(122,122,122),2, 8,0);
        // imshow("eye", original);
        // waitKey(0);
        return true;
    }
    cerr << "findPupil: COULDN'T DETERMINE IRIS" << endl;
    return false;
}

bool Eye::findEyeCorner()
{
	/*
	Mat threshmat, dest, dest_norm, dest_norm_scaled;
	Mat eyeCrop, newfiltered = filtered.clone();


	medianBlur(filtered, newfiltered, 9);
	newfiltered = newfiltered - filtered;

	cv::Rect roi;
	int left = eyeCenter.x + eyeRadius;
	int right = eyeCenter.x - eyeRadius;
	int offset;

	if (leftEye){
	offset = left;
	roi = cv::Rect(left, (int)filtered.rows*0.3, filtered.cols - left, (int)(filtered.rows - filtered.rows*0.3));
	eyeCrop = Mat(newfiltered, roi);
	}
	else {
	offset = 0;
	roi = cv::Rect(0, (int)filtered.rows*0.3, right, (int)(filtered.rows - filtered.rows*0.3));
	eyeCrop = Mat(newfiltered, roi);
	}
	// detector parameters
	int thresh = 200;
	int max_thresh = 255;
	int blockSize = 2;
	int apertureSize = 5;
	double k = 0.01;

	// detect corners
	cornerHarris(eyeCrop, dest, blockSize, apertureSize, k, BORDER_DEFAULT);

	// Normalize
	normalize(dest, dest_norm, 0, 255, NORM_MINMAX, CV_32FC1, Mat());
	convertScaleAbs(dest_norm, dest_norm_scaled);


	cout << "filtered height: " << filtered.rows << endl;
	cout << "filtered width: " << filtered.cols << endl;
	cout << "Top bound for box: " << (int)filtered.rows*0.3 << endl;

	// Draw circle around coners detected
	for (int j = 0; j < dest_norm.rows; j++)
	{
	for (int i = 0; i < dest_norm.cols; i++)
	{
	if (thresh < dest_norm.at<float>(j, i) )
	{
	circle(newfiltered, cv::Point(offset + i, j + (int)filtered.rows*0.3), 4, Scalar(122, 122, 122), 1);
	cout << offset + i << ", " << j << endl;
	}
	}
	}

	//	namedWindow("corner", CV_WINDOW_AUTOSIZE);
	imshow("corner", newfiltered);
	waitKey(0);
	return true;
	*/

	//Pouneh Aghababazadeh
	Mat framegray, destLeft, destRight, leftCornerRoi, rightCornerRoi;
	cvtColor(original, framegray, CV_BGR2GRAY);
	int thresh = 200;
	int max_thresh = 255;
	int blockSize = 2;
	int apertureSize = 5;
	double k = 0.01;
	int buffer = 8; //buffer space away from pupil


	//circles[0] is the pupil
	//circle[0][0] = eyecenter.x
	//circle[0][1] = eyecenter.y + (int)(original.rows*0.4);
	//circle[0][2] = eyeradius

	int pupil[] = { eyeCenter.x ,  eyeCenter.y /* + (int)(original.rows*0.4) */,  eyeRadius };

	while ((cvRound(pupil[0]) - pupil[2] - buffer) <= 0 || (cvRound(pupil[0]) + pupil[2] + buffer) > framegray.cols) {
		buffer--;
	}
    cv::Rect leftroi = cv::Rect(0, 0, (cvRound(pupil[0]) - pupil[2] - buffer), framegray.rows);
    cv::Rect rightroi = cv::Rect((cvRound(pupil[0]) + pupil[2] + buffer), 0, framegray.cols - (cvRound(pupil[0]) + pupil[2] + buffer), framegray.rows);
    leftCornerRoi = cv::Mat(framegray, leftroi);
	rightCornerRoi = cv::Mat(framegray, rightroi);

	//cout << leftCornerRoi.rows << "\t\t" << leftCornerRoi.cols << "\t\t" << destLeft << "\t\t" << blockSize << "\t\t" << apertureSize << "\t\t" << k << endl;
	if (leftCornerRoi.rows > 0 && leftCornerRoi.cols) {
		cornerHarris(leftCornerRoi, destLeft, blockSize, apertureSize, k, BORDER_DEFAULT);
		normalize(destLeft, destLeft, 0, 255, NORM_MINMAX, CV_32FC1, Mat());
		convertScaleAbs(destLeft, destLeft);
	}

	//cout << rightCornerRoi.rows << "\t\t" << rightCornerRoi.cols << "\t\t" << destRight << "\t\t" << blockSize << "\t\t" << apertureSize << "\t\t" << k << endl;
	if (rightCornerRoi.rows > 0 && rightCornerRoi.cols) {
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
			if (thresh < destLeft.at<uchar>(j, i) && verticalDistanceLeft > abs(j - pupil[1])) {
				cornerLeft = cv::Point(i, j);
				verticalDistanceLeft = abs(j - pupil[1]);
			}
		}
	}

	for (int j = 0; j < destRight.rows; j++) {
		for (int i = 0; i < destRight.cols; i++) {
			if (thresh < destRight.at<uchar>(j, i) && verticalDistanceRight > abs(j - pupil[1])) {
				cornerRight = cv::Point(i + (cvRound(pupil[0]) + pupil[2] + buffer), j);
				verticalDistanceRight = abs(j - pupil[1]);
			}
		}
	}

	cv::circle(framegray, cornerLeft, 3, Scalar(127), 1);
	cv::circle(framegray, cornerRight, 3, Scalar(127), 1);
	std::cout << "Final left corner    " << cornerLeft.x << "    " << cornerLeft.y << endl;
	std::cout << "Final right corner    " << cornerRight.x << "    " << cornerRight.y << endl;

	eyeCornerLeft = cornerLeft;
	eyeCornerRight = cornerRight;
	cv::imshow("With corners", framegray);
	cv::waitKey(5000);

	destLeft.release();
	destRight.release();
	//eyeCropColor.release();
	//eyeCropGray.release();
	cout << "END FIND CORNERS" << endl;
	return true;
}

ImgFrame::ImgFrame(cv::Point resolution) : leftEye("haarcascade_righteye_2splits.xml", true), rightEye("haarcascade_lefteye_2splits.xml", false)
{
    faceDetector.load("haarcascade_frontalface_default.xml");
    screenResolution = resolution;
}

ImgFrame::~ImgFrame()
{
    
}

bool ImgFrame::insertFrame(Mat frame)
{
    vector<cv::Rect_<int>> faceCoord;

	//THIS LINE IS BREAKING THE PROGRAM when i try to run it (exceptions) -Pouneh
    faceDetector.detectMultiScale(frame, faceCoord, 1.2, 3, 0, CvSize(150,150));
	cout << faceCoord.capacity() << endl;

    if(faceCoord.capacity() < 1)
    {
        cerr << "insertFrame: DID NOT FIND ANY FACES" << endl;
        return false;
    }

	
    Mat cutoutFace = Mat(frame, faceCoord[0]);
	cv::Rect roiL = cv::Rect(0, (int)(cutoutFace.rows*0.15), (int)(cutoutFace.cols*0.5), (int)(cutoutFace.rows*0.8));
	cv::Rect roiR = cv::Rect((int)(cutoutFace.cols*0.5), (int)(cutoutFace.rows*0.15), (int)(cutoutFace.cols*0.5), (int)(cutoutFace.rows*0.8));
	Mat leftHalf = Mat(cutoutFace, roiL);
	Mat rightHalf = Mat(cutoutFace, roiR);
	return (leftEye.detectKeyFeatures(leftHalf) && rightEye.detectKeyFeatures(rightHalf));
}

// works with leftEyeVector because I don't know what else to use
bool ImgFrame::setCursor()
{
    
    float newX, newY;
//
//    // calculate new X coordinate
//    // center image has screen coordinates: (width/2, height/2)
    float deltaVx = (float)(leftEye.getEyeVector().x - RefImageVector[&ref_center].leftVector.x);
    float changeInEyeX = float(RefImageVector[&ref_bottomLeft].leftVector.x - RefImageVector[&ref_bottomRight].leftVector.x);

    newX = deltaVx / changeInEyeX * screenResolution.x + screenResolution.x/2;
    
    
    // calculate new Y coordinate
    float deltaVy = (float)(leftEye.getEyeVector().y - RefImageVector[&ref_center].leftVector.y);
    float changeInEyeY = (float)(RefImageVector[&ref_bottomLeft].leftVector.x - RefImageVector[&ref_bottomRight].leftVector.x);
    
    newY = deltaVy / changeInEyeY * screenResolution.y + screenResolution.y/2;
    
    singleton->setCurPos(newX, newY);
    
    
    
	//Pouneh: I commented out what this function originally did so that I can get cursor location stuff
	/*
    if(!leftEye.getBlink())
    {
        result = leftEye.getEyeVector();
        return true;
    }
    else if (!rightEye.getBlink())
    {
        result = rightEye.getEyeVector();
        return true;
    }
    return false;
	*/


	return true;
}

int ImgFrame::getBlink()
{
    int total = (int)leftEye.getBlink() + (int)rightEye.getBlink();
    if(total == 0)
    {
        return 0;
    }
    else if(total == 2)
    {
        return 3;
    }
    else
    {
        if(leftEye.getBlink())
        {
            return 1;
        }
        return 2;
    }
    return 2;
}

void lotsOfTheProgram() {
	//The following function:
	//	takes a frame from video input
	//	preprocesses the video input
	//	detects the eyes in the image
	//	detects the pupils in each eye
	//	detects the eyecorners for each eye
	//	desplays the findings on the frame

	VideoCapture cap;
	Mat capture;
	if (!cap.open(0))
		return;

	int thresh = 200;
	int max_thresh = 255;
	int blockSize = 2;
	int apertureSize = 5;
	double k = 0.01;


	Mat framegray, eyeCropGray, eyeCropColor, destLeft, destRight, leftCornerRoi, rightCornerRoi;
	vector<Rect_<int>> eyes, eyes2;
	vector<Vec3f> circles;

	while (1)
	{
		cap >> capture;
		cvtColor(capture, framegray, CV_BGR2GRAY);

		int buffer = 8; //buffer space away from pupil

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
		CascadeClassifier eyeDetector;
		eyeDetector.load("haarcascade_eye_tree_eyeglasses.xml");
		eyeDetector.detectMultiScale(framegray, eyes);

		std::cout << "eye size  " << eyes.size() << endl;

		if (eyes.size() == 0) {
			std::cout << " FUCK " << endl;
			//cv::imshow("capture", capture);
			cv::waitKey(1000);
		}
		for (int i = 0; i < eyes.size(); i++) {
			//draw rectangels around eye
			//rectangle(capture, Point(eyes[i].x, eyes[i].y), Point(eyes[i].x + eyes[i].width, eyes[i].y + eyes[i].height), Scalar(255, 0, 0));
			cv::Rect roiRect = cv::Rect(eyes[i].x, eyes[i].y, eyes[i].width, eyes[i].height);
			eyeCropGray = cv::Mat(framegray, roiRect);
			eyeCropColor = cv::Mat(capture, roiRect);

			//detect pupil for eye
			HoughCircles(eyeCropGray, circles, CV_HOUGH_GRADIENT, 1, eyeCropGray.rows / 3 * 2, 10, 10, eyeCropGray.rows / 10, eyeCropGray.rows / 5);

			// Draw the pupils detected

			for (size_t i = 0; i < circles.size(); i++)
			{
				cv::Point center(cvRound(circles[i][0]), cvRound(circles[i][1]));
				int radius = cvRound(circles[i][2]);
				cout << "(" << center.x << ", " << center.y << ")" << endl;
				// circle center
				//cv::circle(eyeCropColor, center, 3, Scalar(0, 255, 0), 1, 8, 0);
				// circle outline
				//cv::circle(eyeCropColor, center, radius, Scalar(0, 0, 255), 1, 8, 0);
			}


			//if only one pupil
			if (circles.size() >= 1) {

				int xoffset, yoffset;
				//left corner
				xoffset = eyes[0].x;
				yoffset = eyes[0].y;

				while ((cvRound(circles[0][0]) - circles[0][2] - buffer) <= 0 || cvRound(circles[0][0] + circles[0][2] + buffer) > eyeCropGray.cols) {
					buffer--;
				}
				cv::Rect leftroi = cv::Rect(0, 0, (cvRound(circles[0][0]) - circles[0][2] - buffer), eyeCropGray.rows);
				cv::Rect rightroi = cv::Rect((cvRound(circles[0][0]) + circles[0][2] + buffer), 0, (eyeCropGray.cols - (cvRound(circles[0][0]) + circles[0][2] + buffer)), eyeCropGray.rows);
				//cout << "EyecropGray size" << eyeCropGray.rows << "  " <<  eyeCropGray.cols << endl;
				//cout << "\tleftroi size  " << leftroi.width << "\t\t" << leftroi.height << endl;
				//cout << "\trightroi size  " << rightroi.width << "\t\t" << rightroi.height << endl;
				leftCornerRoi = cv::Mat(eyeCropGray, leftroi);
				rightCornerRoi = cv::Mat(eyeCropGray, rightroi);

				//cout << leftCornerRoi.rows << "\t\t" << leftCornerRoi.cols << "\t\t" << destLeft << "\t\t" << blockSize << "\t\t" << apertureSize << "\t\t" << k << endl;
				if (leftCornerRoi.rows > 0 && leftCornerRoi.cols) {
					cornerHarris(leftCornerRoi, destLeft, blockSize, apertureSize, k, BORDER_DEFAULT);
					normalize(destLeft, destLeft, 0, 255, NORM_MINMAX, CV_32FC1, Mat());
					convertScaleAbs(destLeft, destLeft);
				}

				//cout << rightCornerRoi.rows << "\t\t" << rightCornerRoi.cols << "\t\t" << destRight << "\t\t" << blockSize << "\t\t" << apertureSize << "\t\t" << k << endl;
				if (rightCornerRoi.rows > 0 && rightCornerRoi.cols) {
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
							verticalDistanceLeft = abs(j - (int)(circles[0][1]));
						}
					}
				}

				for (int j = 0; j < destRight.rows; j++) {
					for (int i = 0; i < destRight.cols; i++) {
						if (thresh < destRight.at<uchar>(j, i) && verticalDistanceRight > abs(j - circles[0][1])) {
							cornerRight = cv::Point(i + (int)(cvRound(circles[0][0]) + circles[0][2] + buffer), j);
							verticalDistanceRight = abs(j - (int)(circles[0][1]));
						}
					}
				}

				cv::circle(eyeCropColor, cornerLeft, 3, Scalar(255), -1);
				cv::circle(eyeCropColor, cornerRight, 3, Scalar(255), -1);

				cv::imshow("cap", capture);

				destLeft.release();
				destRight.release();
				eyeCropColor.release();
				eyeCropGray.release();
			}
			else {
				std::cout << "fuck this shit" << endl;
			}
		}//for
		if (waitKey(100) == 27) { break; }
	}// end forever loop

}
