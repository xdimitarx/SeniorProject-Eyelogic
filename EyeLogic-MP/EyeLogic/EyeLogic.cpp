#include "EyeLogic.hpp"

Mat loadImageAtPath(string path)
{
    Mat result = imread(path, CV_LOAD_IMAGE_COLOR);
    return result;
}

bool startCam()
{
	if (!cap.open(0))
	{
		return false;
	}
	try
	{
		cap.set(CV_CAP_PROP_FRAME_WIDTH, 1080);
		cap.set(CV_CAP_PROP_FRAME_HEIGHT, 1080);
	}
	catch(Exception ex)
	{
		return false;
	}
	singleton->sleep(4000);
	return true;
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
    vectorPupilToLeftCorner.x = eyeCenter.x - eyeCornerLeft.x;
    vectorPupilToLeftCorner.y = eyeCenter.y - eyeCornerLeft.y;
    
    vectorPupilToRightCorner.x = eyeCenter.x - eyeCornerRight.x;
    vectorPupilToRightCorner.y = eyeCenter.y - eyeCornerRight.y;
    
    
    if(leftEye){
        eyeVector.x = eyeCenter.x - eyeCornerLeft.x;
        eyeVector.y = eyeCenter.y - eyeCornerLeft.y;
    }
    else {
        eyeVector.x = eyeCornerRight.x - eyeCenter.x;
        eyeVector.y = eyeCornerRight.y - eyeCenter.y;
    }
    
	cout << "Eye Vector" << eyeVector.x << "\t" << eyeVector.y << endl;
    
}

bool Eye::detectKeyFeatures(Mat input)
{
    vector<cv::Rect_<int> > eyesCoord;
    
    detector.detectMultiScale(input, eyesCoord, 1.2, 3, 0, CvSize(40,20));
    if(eyesCoord.capacity() < 1)
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
	//doesn't crop width, height is 40%-90%
	//basically top 40% and bottom 10% are cut off
    cv::Rect eyebrowCrop = cv::Rect(0, (int)(original.rows*0.4), (int)(original.cols), (int)(original.rows*0.5));
    original = Mat(original, eyebrowCrop); //eyebrow crop stored
	//eyeLocationOnImageHalf should be adjusted here accordingly DIMITRI/POUNEH
    
    if(findPupil())
    {
        blink = false;
        if(findEyeCorner()){
            createEyeVector();
			return true;
		}      
    }
	return false;
}

bool Eye::getBlink()
{
    return blink;
}

Mat Eye::filterForPupil(Mat input)
{
	Mat result;

	cvtColor(input, result, CV_BGR2GRAY);
	equalizeHist(result, result);
	threshold(result, result, 10, 255, THRESH_BINARY_INV); //Only keeps darkest pixels

	cv::Point left, right;
	for (int i = 0; i < result.cols; i++)
	{
		for (int j = 0; j < result.rows; j++)
		{
			if (result.at<uchar>(j, i) == 255)
			{
				left = cv::Point(i, j);
				j = result.rows;
				i = result.cols;
			}
		}
	}

	for (int i = result.cols-1; i >= 0; i--)
	{
		for (int j = 0; j < result.rows; j++)
		{
			if (result.at<uchar>(j, i) == 255)
			{
				right = cv::Point(i, j);
				j = result.rows;
				i = 0;
			}
		}
	}

	int slopeY = (right.y - left.y);
	int slopeX = (right.x - left.x);

	double deltY = (double)slopeY / (double)slopeX;

	double limitY = left.y;

	for (int i = left.x; i < right.x; i++)
	{
		for (int j = 0; j < round(limitY); j++)
		{
			result.at<uchar>(j, i) = 0;
		}
		limitY += deltY;
	}

	cv::Rect crop = cv::Rect(0, result.rows/2, result.cols, result.rows - result.rows/2);
	Mat bottomHalf = Mat(result, crop);
	Mat topHalf;
	flip(bottomHalf, topHalf, 0);
	
	for (int i = 0; i < bottomHalf.cols; i++)
	{
		for (int j = 0; j < bottomHalf.rows; j++)
		{
			result.at<uchar>(j, i) = topHalf.at<uchar>(j, i);
		}
	}

	Mat erodeElement = getStructuringElement(MORPH_ELLIPSE, cv::Size(4, 4));
	dilate(result, result, erodeElement);

	return result;
}

bool Eye::findPupil()
{
	Mat preparedImage = filterForPupil(original.clone());

	imshow("filtered", preparedImage);
	waitKey(10000);

    vector<Vec4i> hierarchy;
    vector<vector<cv::Point> > contours; //find contours of blob in preparedImage
    findContours(preparedImage, contours, hierarchy, CV_RETR_TREE, CV_CHAIN_APPROX_SIMPLE, cv::Point(0,0));

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
		Moments mo = moments(contours[largest], false);
        cv::Rect bounding = boundingRect(contours[largest]);
		eyeCenter = cv::Point(mo.m10 / mo.m00, mo.m01 / mo.m00);
        eyeRadius = cvRound(bounding.height);
        //cv::circle(original, eyeCenter, eyeRadius, Scalar(122,122,122), 2);
		//cv::circle(original, eyeCenter, 3, Scalar(122, 122, 122), 2);

        return true;
    }
    cerr << "findPupil: COULDN'T DETERMINE IRIS" << endl;
    return false;
}

bool Eye::findEyeCorner()
{
	Mat framegray, destLeft, destRight, leftCornerRoi, rightCornerRoi, edges;
	cv::cvtColor(original, framegray, CV_BGR2GRAY);
	int yOffset = (eyeRadius < eyeCenter.y) ? eyeCenter.y - eyeRadius : 0;

	cv::Rect crop = cv::Rect(0, yOffset, framegray.cols, framegray.rows - yOffset);
	framegray = Mat(framegray, crop);

	cv::waitKey(2000);
	int thresh = 150;
	int max_thresh = 255;
	int blockSize = 2;
	int apertureSize = 5;
	double k = 0.01;
	//int buffer = 8; //buffer space away from pupil

	//increase contrast
	cv::equalizeHist(framegray, framegray);

	//circles[0] is the pupil
	//circle[0][0] = eyecenter.x
	//circle[0][1] = eyecenter.y + (int)(original.rows*0.4);
	//circle[0][2] = eyeradius

	int leftbuffer;
	int rightbuffer;

	int pupil[] = { eyeCenter.x ,  eyeCenter.y ,  eyeRadius };
	if (leftEye) {
		leftbuffer = eyeCenter.x / 2 - 1;
		rightbuffer = (framegray.cols - eyeCenter.x) / 2 + 2;
	}
	else {
		leftbuffer = eyeCenter.x / 2 - 2;
		rightbuffer = (framegray.cols - eyeCenter.x) / 2 ;
	}

    while ((cvRound(pupil[0]) - leftbuffer) <= 0 ) {
        leftbuffer--;
    }

	while ((cvRound(pupil[0]) + rightbuffer) > framegray.cols) {
		rightbuffer--;
	}

	cv::Rect leftroi = cv::Rect(0, 0, (cvRound(pupil[0]) - leftbuffer), framegray.rows);
    cv::Rect rightroi = cv::Rect((cvRound(pupil[0]) + rightbuffer), 0, framegray.cols - (cvRound(pupil[0]) + rightbuffer), framegray.rows);
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
                cornerRight = cv::Point(i + (cvRound(pupil[0]) + rightbuffer), j);
                verticalDistanceRight = abs(j - pupil[1]);
            }
        }
    }
    
	Mat threechanelFramegray;
	cv::cvtColor(framegray, threechanelFramegray, CV_GRAY2BGR);

	//cv::line(threechanelFramegray, Point((cvRound(pupil[0]) - leftbuffer), 0), Point((cvRound(pupil[0]) - leftbuffer), framegray.rows), Scalar(0, 255, 255));
	//cv::line(threechanelFramegray, Point((cvRound(pupil[0]) + rightbuffer), 0), Point((cvRound(pupil[0]) + rightbuffer), framegray.rows), Scalar(0, 0, 255));

    cv::circle(threechanelFramegray, cornerLeft, 3, Scalar(255,0,0), -1);
    cv::circle(threechanelFramegray, cornerRight, 3, Scalar(0,255,0), -1);
	//cv::circle(threechanelFramegray, eyeCenter, eyeRadius, Scalar(0, 0, 255), -1);
    
    eyeCornerLeft = cornerLeft;
    eyeCornerRight = cornerRight;
    cv::imshow("With corners", threechanelFramegray);
	waitKey(1000);
    
    destLeft.release();
    destRight.release();
    framegray.release();
    cv::destroyWindow("With corners");
    //eyeCropColor.release();
    //eyeCropGray.release();
    std::cout << "END FIND CORNERS" << endl;
    return true;
}

ImgFrame::ImgFrame() : leftEye("haarcascade_righteye_2splits.xml", true), rightEye("haarcascade_lefteye_2splits.xml", false)
{
    faceDetector.load("haarcascade_frontalface_default.xml");
}

ImgFrame::~ImgFrame()
{
    
}

bool ImgFrame::insertFrame(Mat frame)
{
    vector<cv::Rect_<int>> faceCoord;
    
    //THIS LINE IS BREAKING THE PROGRAM when i try to run it (exceptions) -Pouneh
    faceDetector.detectMultiScale(frame, faceCoord, 1.2, 3, 0, CvSize(300,300));
    std::cout << faceCoord.capacity() << endl;
    
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
    
    cv::Point newcoord;
    //
    //    // calculate new X coordinate
    //    // center image has screen coordinates: (width/2, height/2)
    float deltaVx = (float)(leftEye.getEyeVector().x - RefImageVector[&ref_center].leftVector.x);
    float changeInEyeX = float(RefImageVector[&ref_bottomLeft].leftVector.x - RefImageVector[&ref_bottomRight].leftVector.x);
    
    newcoord.x = deltaVx / changeInEyeX * screenres.x + screenres.x/2;
    
    
    // calculate new Y coordinate
    float deltaVy = (float)(leftEye.getEyeVector().y - RefImageVector[&ref_center].leftVector.y);
    float changeInEyeY = (float)(RefImageVector[&ref_bottomLeft].leftVector.x - RefImageVector[&ref_bottomRight].leftVector.x);
    
    newcoord.y = deltaVy / changeInEyeY * screenres.y + screenres.y/2;
    
    singleton->setCurPos(newcoord);    
    
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

/*void lotsOfTheProgram() {
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

	int thresh = 100;
	int max_thresh = 255;
	int blockSize = 2;
	int apertureSize = 5;
	double k = 0.01;


	//Mat framegray, destLeft, destRight, leftCornerRoi, rightCornerRoi, edges;


	Mat framegray, eyeCropGray, eyeCropColor, destLeft, destRight, leftCornerRoi, rightCornerRoi;
	vector<Rect_<int>> eyes, eyes2;
	vector<Vec3f> circles;
	cv::Point eyeCornerRight, eyeCornerLeft;

	CascadeClassifier eyeDetector;
	eyeDetector.load("haarcascade_eye_tree_eyeglasses.xml");

	while (1)
	{
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

		std::cout << "eye size  " << eyes.size() << endl;

		if (eyes.size() == 0) {
			std::cout << " FUCK " << endl;
			cv::imshow("capture", capture);
			cv::waitKey(1000);
			continue;
		}
		for (int i = 0; i < eyes.size(); i++) {
			//draw rectangels around eye
            rectangle(capture, cv::Point(eyes[i].x, eyes[i].y), cv::Point(eyes[i].x + eyes[i].width, eyes[i].y + eyes[i].height), Scalar(255, 0, 0));
			cv::Rect roiRect = cv::Rect(eyes[i].x, eyes[i].y, eyes[i].width, eyes[i].height);
			eyeCropGray = cv::Mat(framegray, roiRect);
			eyeCropColor = cv::Mat(capture, roiRect);

			//detect pupil for eye
			HoughCircles(eyeCropGray, circles, CV_HOUGH_GRADIENT, 1, eyeCropGray.rows / 3 * 2, 10, 10, eyeCropGray.rows / 10, eyeCropGray.rows / 5);


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
		


			//if only one pupil
			//circles[0] is the pupil
			//circles[0][0] = eyecenter.x
			//circles[0][1] = eyecenter.y + (int)(original.rows*0.4);
			//circles[0][2] = eyeradius

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

			eyeCornerLeft = cv::Point(cornerLeft.x + roiRect.x, cornerLeft.y + roiRect.y);
			eyeCornerRight = cv::Point(cornerRight.x + rightroi.x + roiRect.x, cornerRight.y + roiRect.y);

			cout << "Left  corner: " << cornerLeft.x << "    " << cornerLeft.y << endl;
			cout << "Right corner: " << cornerRight.x << "    " << cornerRight.y << endl;

			cv::circle(capture, eyeCornerLeft, 3, Scalar(255, 0, 0), -1);
			cv::circle(capture, eyeCornerRight, 3, Scalar(0, 255, 0), -1);

			imshow("cap", capture);
			//cv::imshow("With corners", threechanelFramegray);
		}//for
		if (cv::waitKey(100) == 27) { break; }
	}// end forever loop

}//lotsoftheprogram */
