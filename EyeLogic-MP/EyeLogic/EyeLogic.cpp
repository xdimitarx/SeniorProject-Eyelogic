#include "EyeLogic.hpp"


/*
 *  Reads in image from 
 */
Mat loadImageAtPath(string path){
    Mat result = imread(path, CV_LOAD_IMAGE_COLOR);
    return result;
}

bool EyeLogic::insertFrame(Mat frame, bool forceNewTemplate)
{
	currentFrame = frame;

	Rect faceCrop;

	cv::Point frameDiff = cv::Point(0, 0);

	if (!faceTemplateExists || !eyeTemplatesExists || forceNewTemplate)
	{
		//Presumably slower then template matching
		vector<cv::Rect_<int>> faces;
		faceExtractor.detectMultiScale(frame, faces); //Need to apply minimum size
		if (faces.size() == 0) return false; //No Faces!!
		faceCrop = faces[0];
		storeTemplate(frame, faceCrop);
		eyeTemplatesExists = false;
	}
	else
	{
		if (!checkTemplate(frame, &faceCrop, &frameDiff)) return false; //No suitable matches from template
	}

	Mat cropFace = frame(faceCrop);

	if (!eyeTemplatesExists || forceNewTemplate)
	{
		Mat leftHalf = cropFace(Rect(0, 0, cropFace.cols / 2, cropFace.rows));
		Mat rightHalf = cropFace(Rect(cropFace.cols / 2, 0, cropFace.cols - cropFace.cols / 2, cropFace.rows));
		
		//Capture (our right) User's Left Eye Bound box
		vector<cv::Rect_<int>> eyes;
		rightEyeExtractor.detectMultiScale(rightHalf, eyes);
		rightEyeBound = eyes[0]; //Class Variable
		rightEyeBound.x += cropFace.cols / 2;

		//Capture (our left) User's Right Eye Bound box
		eyes.clear();
		leftEyeExtractor.detectMultiScale(leftHalf,eyes);
		leftEyeBound = eyes[0];

		eyeTemplatesExists = true;
	}

	Rect lEyeB = leftEyeBound; //This math accounts for shifting from template
	lEyeB.x += frameDiff.x;
	lEyeB.y += frameDiff.y;

	Rect rEyeB = rightEyeBound;
	rEyeB.x += frameDiff.x;
	rEyeB.y += frameDiff.y;

	//filter and detect pupils
	cv::Point leftPupil = findPupil(applyPupilFilters(cropFace(lEyeB)));
	cv::Point rightPupil = findPupil(applyPupilFilters(cropFace(rEyeB)));

	leftPupil.x += lEyeB.x;
	leftPupil.y += lEyeB.y;
	rightPupil.x += rEyeB.x;
	rightPupil.y += rEyeB.y;

	eyeVector = cv::Point((leftPupil.x + rightPupil.x)/2, (leftPupil.y + rightPupil.y)/2);
	return true;
}

//check for Mat.empty()
Mat EyeLogic::getTemplate(Rect * faceCrop, Rect * leftEyeCrop, Rect * rightEyeCrop)
{
	if (!faceTemplateExists || !eyeTemplatesExists) return Mat();

	faceCrop = new Rect(faceRect);
	leftEyeCrop = new Rect(leftEyeBound);
	rightEyeCrop = new Rect(rightEyeBound);
	return userTemplate;
}

void EyeLogic::storeTemplate(Mat image, Rect faceBound, Rect leftEyeCrop, Rect rightEyeCrop)
{
	if (leftEyeCrop == Rect() || rightEyeCrop == Rect())
	{
		faceRect = faceBound;

		faceBound.y += faceBound.height*0.6;
		faceBound.height = faceBound.height*0.2;
		userTemplate = image(faceBound);
		cvtColor(userTemplate, userTemplate, CV_BGR2GRAY);
		equalizeHist(userTemplate, userTemplate);
		
		faceTemplateExists = true;
	}
	else
	{
		userTemplate = image;
		faceRect = faceBound;
		leftEyeBound = leftEyeCrop;
		rightEyeBound = rightEyeCrop;
		faceTemplateExists = true;
		eyeTemplatesExists = true;
	}
}

cv::Point EyeLogic::eyeVectorToScreenCoord()
{
	return Point();
}

cv::Point EyeLogic::getEyeVector()
{
	return eyeVector;
}

void EyeLogic::setReferencePoint(cv::Point eyeVector, RefPoint refPosition)
{
	switch (refPosition)
	{
	case RefPoint::LEFT:
		ref_Left = eyeVector;
		break;
	case RefPoint::RIGHT:
		ref_Right = eyeVector;
		break;
	case RefPoint::TOP:
		ref_Top = eyeVector;
		break;
	case RefPoint::BOTTOM:
		ref_Bottom = eyeVector;
		break;
	default:
		break;
	}
}

//Points are in Enum Order
vector <cv::Point> * EyeLogic::getReferencePointData()
{
	vector <cv::Point> * referencePoints;
	referencePoints->push_back(ref_Left);
	referencePoints->push_back(ref_Right);
	referencePoints->push_back(ref_Top);
	referencePoints->push_back(ref_Bottom);
	return referencePoints;
}

void EyeLogic::setReferencePointData(vector <cv::Point> * data)
{
	if (data->size() != 4) return;

	ref_Left = data->at(0);
	ref_Right = data->at(1);
	ref_Top = data->at(2);
	ref_Bottom = data->at(3);
}

EyeLogic::EyeLogic(cv::Point screenres)
{
	screenResolution = screenres;
	faceExtractor.load("haarcascade_frontalface_default.xml");
	rightEyeExtractor.load("haarcascade_lefteye_2splits.xml");
	leftEyeExtractor.load("haarcascade_lefteye_2splits.xml");
}

Mat EyeLogic::applyPupilFilters(Mat eyeCrop)
{
	Mat result;

	cvtColor(eyeCrop, result, CV_BGR2GRAY);
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

	for (int i = result.cols - 1; i >= 0; i--)
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

	cv::Rect crop = cv::Rect(0, result.rows / 2, result.cols, result.rows - result.rows / 2);
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

cv::Point EyeLogic::findPupil(cv::Mat eyeCrop) {

	std::vector<cv::Vec4i> hierarchy;
	std::vector<std::vector<cv::Point> > contours;

	findContours(eyeCrop, contours, hierarchy, CV_RETR_TREE, CV_CHAIN_APPROX_SIMPLE, cv::Point(0, 0));
	if (contours.size())
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

		return eyeCenter;
	}
	std::cerr << "findPupil: COULDN'T DETERMINE IRIS" << std::endl;
	return cv::Point(-1, -1);
}

bool EyeLogic::checkTemplate(Mat frame, Rect * faceCrop, cv::Point * frameDifference)
{
	Mat filteredFrame;
	cvtColor(frame, filteredFrame, CV_BGR2GRAY);
	equalizeHist(filteredFrame, filteredFrame);

	int result_cols = filteredFrame.cols - userTemplate.cols + 1;
	int result_rows = filteredFrame.rows - userTemplate.rows + 1;

	Mat result;
	result.create(result_rows, result_cols, CV_32FC1);

	matchTemplate(filteredFrame, userTemplate, result, 5);
	
	double minVal; double maxVal; cv::Point minLoc; cv::Point matchLoc;
	minMaxLoc(result, &minVal, &maxVal, &minLoc, &matchLoc, Mat());

	//Check match score for plausability?

	*faceCrop = faceRect;
	faceCrop->x = matchLoc.x;
	faceCrop->y = matchLoc.y;
	frameDifference->x = matchLoc.x - faceRect.x;
	frameDifference->y = matchLoc.y - faceRect.y;
	return true;
}
