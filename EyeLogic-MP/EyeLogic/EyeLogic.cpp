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
		cap.set(CV_CAP_PROP_FRAME_WIDTH, 1280);
		cap.set(CV_CAP_PROP_FRAME_HEIGHT, 720);
	}
	catch(Exception ex)
	{
		return false;
	}
	singleton->sleep(2000);
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
	original = Mat(original, eyebrowCrop);
	//eyeLocationOnImageHalf should be adjusted here accordingly DIMITRI/POUNEH
    
    if(findPupil())
    {
        blink = false;
        if(findEyeCornerTMatching()){
            //createEyeVector();
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

//Eye Corner with template matching
bool Eye::findEyeCornerTMatching()
{
	Mat filteredImage;
	cvtColor(original, filteredImage, CV_BGR2GRAY);
	equalizeHist(filteredImage, filteredImage);

	if(!templateLoaded)
	{
		if (leftEye)
		{
            cornerTemplate = Mat(filteredImage, cv::Rect(filteredImage.cols*0.05, filteredImage.rows*0.2, filteredImage.cols*0.2, filteredImage.rows*0.6));
		}
		else
		{
			cornerTemplate = Mat(filteredImage, cv::Rect(filteredImage.cols*0.65, filteredImage.rows*0.25, filteredImage.cols*0.3, filteredImage.rows*0.5));
		}
		templateLoaded = true;
	}

	int result_cols = filteredImage.cols - cornerTemplate.cols + 1;
	int result_rows = filteredImage.rows - cornerTemplate.rows + 1;

	Mat result;
	result.create(result_rows, result_cols, CV_32FC1);

	matchTemplate(filteredImage, cornerTemplate, result, 5);
	normalize(result, result, 0, 1, NORM_MINMAX, -1, Mat());

	double minVal; double maxVal; cv::Point minLoc; cv::Point matchLoc;
	minMaxLoc(result, &minVal, &maxVal, &minLoc, &matchLoc, Mat());

	cv::Rect templateLoc = cv::Rect(matchLoc.x, matchLoc.y, 2, 2);
	
	/*
	Mat output = original.clone();
	cv::circle(output, eyeCenter, 2, Scalar(0, 255, 0), 1, 8, 0);
	cv::circle(output, matchLoc, 2, Scalar(0, 0, 255), 1, 8, 0);
	imwrite("orig.jpg", output); 
	int x = 0;
	int y = 0;
	*/
	if (leftEye)
	{
		//x = eyeCenter.x - matchLoc.x;
		//y = eyeCenter.y - matchLoc.y;
	}
	else
	{
		cout << "wooR" << endl;
	}
	return true;
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
	waitKey(10000);
    
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
    return (leftEye.detectKeyFeatures(leftHalf) /*&& rightEye.detectKeyFeatures(rightHalf)*/);
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
