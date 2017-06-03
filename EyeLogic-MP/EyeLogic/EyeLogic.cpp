#include "EyeLogic.hpp"


/*
 *  Reads in image from given path
 */
Mat loadImageAtPath(string path){
    Mat result = imread(path, CV_LOAD_IMAGE_COLOR);
    return result;
}

/*
*  insertFrame
*  Description: Detects pupils and sets the avg eye vector
*  Note: forceNewTemplate = true will ignore my template matching alg <- Dom

*  returns true if all an avg eye vector was successfully calculated
*/
bool EyeLogic::insertFrame(Mat frame, bool forceNewTemplate)
{
	if (frame.empty())
	{
		cerr << "Error in insertFrame: Frame was empty." << endl;
	}

	currentFrame = frame;

	cv::Rect faceCrop;
	cv::Point frameDiff = cv::Point(0, 0);

	//Create template or check template
	if (!faceTemplateExists || !eyeTemplatesExists || forceNewTemplate)
	{
		//Presumably slower then template matching
		vector<cv::Rect_<int>> faces;
		faceExtractor.detectMultiScale(frame, faces); //Need to apply minimum size
		if (faces.size() == 0)
		{
			cerr << "Error in insertFrame: No faces detected." << endl;
			return false; //No Faces!!
		}
		faceCrop = faces[0];
		storeTemplate(frame, faceCrop);
		eyeTemplatesExists = false; //create new eyes if new face template
	}
	else
	{
		// This uses template matching to determine the corresponding faceCrop in the new frame
		// the data gathered is used to adjust the eyebounds accordingly for much faster processing then haarcascades
		if (!checkTemplate(frame, &faceCrop, &frameDiff))
		{
			cerr << "Error in insertFrame: Template matching failed." << endl;
			return false;
		}
	}

	cv::Mat cropFace = frame(faceCrop);

	// Check for force or if eye template does not exist
	if (!eyeTemplatesExists || forceNewTemplate)
	{
		// Generates the eyebounds from the image and sets them as a template
		if (!createEyeBounds(cropFace))
		{
			cerr << "Error in insertFrame: Could not create eye bounds." << endl;
			return false;
		}
	}

	cv::Rect lEyeB = leftEyeBound; //This math accounts for shifting from template
	lEyeB.x += frameDiff.x; //if the current frame is the one where the template is generated then frameDiff will be 0
	lEyeB.y += frameDiff.y; // otherwise frameDiff will be the difference between the original frame and the inserted one

	cv::Rect rEyeB = rightEyeBound;
	rEyeB.x += frameDiff.x;
	rEyeB.y += frameDiff.y;

	//filter and detect pupils
	cv::Point leftPupil = findPupil(applyPupilFilters(cropFace(lEyeB)));
	cv::Point rightPupil = findPupil(applyPupilFilters(cropFace(rEyeB)));

	if (leftPupil == cv::Point(-1, -1) || rightPupil == cv::Point(-1, -1))
	{
		cerr << "Error in insertFrame: Could not detect pupils!" << endl;
		return false;
	}

	leftPupil.x += lEyeB.x;
	leftPupil.y += lEyeB.y;
	rightPupil.x += rEyeB.x;
	rightPupil.y += rEyeB.y;

	eyeVector = cv::Point((leftPupil.x + rightPupil.x)/2, (leftPupil.y + rightPupil.y)/2);
    
	return true;
}

/*
*  getTemplate
*  Description: interface to get templates for pupil detection
*  Note: used in dimitri's calibration
*  returns Mat.empty() on error
*/
cv::Mat EyeLogic::getTemplate(cv::Rect * faceCrop, cv::Rect * leftEyeCrop, cv::Rect * rightEyeCrop)
{
	if (!faceTemplateExists || !eyeTemplatesExists) return Mat();

	*faceCrop = faceRect;
	*leftEyeCrop = leftEyeBound;
	*rightEyeCrop = rightEyeBound;
	return userTemplate;
}

/*
*	storeTemplate
*	Description: takes in templateImage, face and eye crops and sets them as the new template
*	Note: has secondary internal use to just create the face template and store it
*   leaving left and right eye crops as empty rects will allow this to happen
*/
void EyeLogic::storeTemplate(cv::Mat image, cv::Rect faceBound, cv::Rect leftEyeCrop, cv::Rect rightEyeCrop)
{
	//if true then a new template will be created, otherwise is trying to load old template
	if (leftEyeCrop == cv::Rect() || rightEyeCrop == cv::Rect())
	{
		faceRect = faceBound; // rect set before template is extracted, template and rect have same width

		faceBound.y += floor(faceBound.height*0.6);
		faceBound.height = faceBound.height*0.2;

		userTemplate = image(faceBound); //userTemplate is a region around the upper lip that has been converted to gray scale and equalized historgrams
		cvtColor(userTemplate, userTemplate, CV_BGR2GRAY);
		equalizeHist(userTemplate, userTemplate);
		
		faceTemplateExists = true;
	}
	else
	{
		//direct copy from external source
		userTemplate = image;
		faceRect = faceBound;
		leftEyeBound = leftEyeCrop;
		rightEyeBound = rightEyeCrop;
		faceTemplateExists = true;
		eyeTemplatesExists = true;
	}
}

//To be implemented
cv::Point EyeLogic::eyeVectorToScreenCoord()
{
    distance = cv::Point(ref_Left.x - ref_Right.x, ref_Bottom.y - ref_Top.y);
	if (!Calibrated(true))
	{
		cerr << "Error in eyeVectorToScreenCoord: EyeLogic not calibrated." << endl;
		return cv::Point(-1, -1);
	}

	//distance = Point(ref_Left.x - ref_Right.x, ref_Bottom.y - ref_Top.y);

	cv::Point averageLocal = getEyeVector();
	cv::Point destinationNew;

	//check if detected point is out of maximum bounds
	if (averageLocal.x < ref_Right.x || averageLocal.x > ref_Left.x || averageLocal.y < ref_Top.y || averageLocal.y > ref_Bottom.y) {
		//imshow("CAPTURE", capture);
		//cv::waitKey(1);
		//TODO: head moving things
		cerr << "Error in eyeVectorToScreenCoord: EyeVector not in bounds of reference images." << endl;
		/*
		cerr << "\taverageLocal.x  " << averageLocal.x << "\tref_Right.x" << ref_Right.x << endl;
		cerr << "\taverageLocal.x  " << averageLocal.x << "\tref_Left.x" << ref_Left.x << endl;
		cerr << "\taverageLocal.y  " << averageLocal.y << "\tref_Top.y" << ref_Top.y << endl;
		cerr << "\taverageLocal.y  " << averageLocal.y << "ref_Bottom.y" << ref_Bottom.y << endl;
		*/
        return cv::Point(-1, -1);
	}

	destinationNew.x = (screenResolution.x - ((averageLocal.x - ref_Right.x) * screenResolution.x / distance.x));
	destinationNew.y = (averageLocal.y - ref_Top.y) * screenResolution.y / distance.y; //screenRes.y / 2;

	if (destinationOld != destinationNew) {
		destinationOld = destinationNew;
		delta = cv::Point((destinationNew.x - screenMap.x) / screenResolution.x * 80, (destinationNew.y - screenMap.y) / screenResolution.y * 80);
		if (delta.x > 0) { direction.x = 1; }
		else { direction.x = -1; }
		if (delta.y > 0) { direction.y = 1; }
		else { direction.y = -1; }
	}

	if (direction.x > 0) {
		screenMap.x = min(destinationNew.x, screenMap.x + delta.x);
	}
	else {
		screenMap.x = std::max(destinationNew.x, screenMap.x + delta.x);
	}
	if (direction.y > 0) {
		screenMap.y = min(destinationNew.y, screenMap.y + delta.y);
	}
	else {
		screenMap.y = std::max(destinationNew.y, screenMap.y + delta.y);
	}

	//point on screen: 
	screenMap.x = (screenResolution.x - ((averageLocal.x - ref_Right.x) * screenResolution.x / distance.x));
	screenMap.y = (averageLocal.y - ref_Top.y) * screenResolution.y / distance.y;

	//Enforce screen resolution as boundaries for movement of cursor
	if (screenMap.x >= 0 && screenMap.y >= 0 && screenMap.x <= screenResolution.x && screenMap.y <= screenResolution.y) {
		return screenMap;

	}
	else {
		cerr << "Error in eyeVectorToScreenCoord: Coordinates not within screen bounds." << endl;
		cerr << "\tScreenMap.x: " << screenMap.x << "\tScreenMap.y: " << screenMap.y << std::endl;
	}
	return cv::Point(-1, -1);
}

cv::Point EyeLogic::getEyeVector()
{
	return eyeVector;
}

/*
* setReferencePoint
* Description: sets the reference point for the given enum
*/
void EyeLogic::setReferencePoint(cv::Point point, RefPoint refPosition)
{
	if (point.x < 0 || point.y < 0)
	{
		cerr << "Error in setReferencePoint: Valid point was not given." << endl;
		return;
	}
	switch (refPosition)
	{
	case RefPoint::LEFT:
		ref_Left = point;
		break;
	case RefPoint::RIGHT:
		ref_Right = point;
		break;
	case RefPoint::TOP:
		ref_Top = point;
		break;
	case RefPoint::BOTTOM:
		ref_Bottom = point;
		break;
	default:
		cerr << "Error in setReferencePoint: enum is not valid." << endl;
		break;
	}
}

// returns vector of reference point data in (LEFT,RIGHT,TOP,BOTTOM) order
vector <cv::Point>  EyeLogic::getReferencePointData()
{
	vector <cv::Point>  referencePoints;
	if (!Calibrated(false))
	{
		cerr << "Error in getReferencePointData: EyeLogic not calibrated." << endl;
		return referencePoints;
	}
	referencePoints.push_back(ref_Left);
	referencePoints.push_back(ref_Right);
	referencePoints.push_back(ref_Top);
	referencePoints.push_back(ref_Bottom);
	return referencePoints;
}

// sets reference points, must be a vector in (LEFT,RIGHT,TOP,BOTTOM) order
void EyeLogic::setReferencePointData(vector <cv::Point> * data)
{
	if (data->size() != 4)
	{
		cerr << "Error in setReferencePointData: Input vector does not contain exactly 4 reference points." << endl;
		return;
	}

	ref_Left = data->at(0);
	ref_Right = data->at(1);
	ref_Top = data->at(2);
	ref_Bottom = data->at(3);
}

// checks whether reference points have been set
// by setting valid true there is also a simple check to make sure that the bounds actually define a box
bool EyeLogic::Calibrated(bool valid)
{
	if (valid)
	{
		if (ref_Bottom.y > ref_Top.y || ref_Left.x > ref_Right.x)
		{
			cerr << "Error in Calibrated: Valid check failed." << endl;
			cerr << "ref_Bottom.y\t" << ref_Bottom.y << "\t ref_Top.y\t" << ref_Top.y << endl;
			cerr << "ref_Left.x\t" << ref_Left.x << "\t ref_Right.x\t" << ref_Right.x << endl;
			return false;
		}
	}
	return (ref_Bottom != cv::Point(-1, -1) && ref_Left != cv::Point(-1, -1) && ref_Top != cv::Point(-1, -1) && ref_Right != cv::Point(-1, -1));
}

//loads cascades and stores screen resolution
EyeLogic::EyeLogic(cv::Point screenres)
{
	screenResolution = screenres;

    destinationOld = cv::Point(-1, -1);
    direction = cv::Point(0, 0);
    delta = cv::Point(0,0);

	ref_Right = cv::Point(-1, -1);
	ref_Left = cv::Point(-1, -1);
	ref_Top = cv::Point(-1, -1);
	ref_Bottom = cv::Point(-1, -1);

	faceExtractor.load("haarcascade_frontalface_default.xml");
	rightEyeExtractor.load("haarcascade_lefteye_2splits.xml");
	leftEyeExtractor.load("haarcascade_lefteye_2splits.xml");
}

/*
*	createEyeBounds
*	Description: users haar cascades to detect the eyes, then performs additional cropping to remove eyebrows
*	  and then stores this data in reference to faceCrop which is a face cutout using faceRect*
*
*	returns false if eyes could not be detected
*/
bool EyeLogic::createEyeBounds(cv::Mat faceCrop)
{
	cv::Mat leftHalf = faceCrop(cv::Rect(0, (int)(faceCrop.rows*0.15), (int)(faceCrop.cols*0.5), (int)(faceCrop.rows*0.8)));
	cv::Mat rightHalf = faceCrop(cv::Rect((int)(faceCrop.cols*0.5), (int)(faceCrop.rows*0.15), (int)(faceCrop.cols*0.5), (int)(faceCrop.rows*0.8)));

	//Capture (our right) User's Left Eye Bound box
	vector<cv::Rect_<int>> eyes;
	rightEyeExtractor.detectMultiScale(rightHalf, eyes);

	if (eyes.size() == 0)
	{
		cerr << "Error in createEyeBounds: Right Eye could not be detected." << endl;
		return false;
	}

	rightEyeBound = eyes[0]; //Class Variable
	rightEyeBound.x += faceCrop.cols / 2; // this is to account for split in half above
	rightEyeBound.y += (int)(faceCrop.rows*0.15);

	rightEyeBound.y = rightEyeBound.y + (int)(rightEyeBound.height*0.4); //Crop Eyebrow
	rightEyeBound.height /= 2; //Crop Eyebrow

	//Capture (our left) User's Right Eye Bound box
	eyes.clear();
	leftEyeExtractor.detectMultiScale(leftHalf, eyes);

	if (eyes.size() == 0)
	{
		cerr << "Error in createEyeBounds: Left Eye could not be detected." << endl;
		return false;
	}

	leftEyeBound = eyes[0];
	leftEyeBound.y += (int)(faceCrop.rows*0.15);

	leftEyeBound.y = leftEyeBound.y + (int)(leftEyeBound.height*0.4);
	leftEyeBound.height /= 2;

	eyeTemplatesExists = true;

	return true;
}

/*
*	applyPupilFilters
*	Description: takes an eyeCrop, makes it gray scale, performs histogram equalization, thresholds for the darkest pixels
*	uses Dom's formula to reduce noise, and then dilates to exaggerate the results and make sure that they accumulate properly
*	for find pupil
*
*	returns filtered image
*/
cv::Mat EyeLogic::applyPupilFilters(cv::Mat eyeCrop)
{

	cv::Mat result;

	cvtColor(eyeCrop, result, CV_BGR2GRAY);
	equalizeHist(result, result);
	threshold(result, result, 10, 255, THRESH_BINARY_INV); //Only keeps darkest pixels

	//get farthest left noise
	cv::Point left(-1,-1), right(-1,-1);
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

    if (left == cv::Point(-1, -1))
	{
		cerr << "Error in applyPupilFilters: No valid black pixels detected. SERIOUS ERROR." << endl;
	}

	//get farthest right noise
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

    if (right == cv::Point(-1, -1))
	{
		cerr << "Error in applyPupilFilters: No valid black pixels detected. SERIOUS ERROR." << endl;
	}

	int slopeY = (right.y - left.y);
	int slopeX = (right.x - left.x);

	double deltY = (double)slopeY / (double)slopeX;

	double limitY = left.y;

	//create a line between the two points above and make everything above it white
	//this appears to cut the top portions of the eye and eyebrow off due to the shape of our eye sockets
	// the bottome portion can then be flipped which usually means the bottom half of the pupil mirrored
	for (int i = left.x; i < right.x; i++)
	{
		for (int j = 0; j < round(limitY); j++)
		{
			result.at<uchar>(j, i) = 0;
		}
		limitY += deltY;
	}

	//take the bottom half of the image and flip it up
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

	//dilates all black pixels in the image in a hopes to connect them, find pupil will look for the largest mass 
	// and until this step the image looks grainy, this turns it more into a solid blob
	Mat erodeElement = getStructuringElement(MORPH_ELLIPSE, cv::Size(4, 4));
	dilate(result, result, erodeElement);

	return result;
}

/*
*	findPupil
*	Description: uses image moments to determince the center of the pupil from the largest contour of black pixels
*
*	returns the pupils location relative to faceRect*
*/
cv::Point EyeLogic::findPupil(cv::Mat eyeCrop) {

	cv::Mat eyeCropGray;
	eyeCrop.copyTo(eyeCropGray);
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

		cv::Rect bounding = boundingRect(contours[largest]);
		//Point eyeCenter = cv::Point(cvRound(bounding.x + bounding.width / 2), cvRound(bounding.y + bounding.height / 2));	
		int eyeRadius = cvRound(bounding.height*1.05);

		return eyeCenter;
	}


	/*
	std::vector<cv::Vec4i> hierarchy;
	std::vector<std::vector<cv::Point> > contours;

	//obtain contours of all black blobs in the image and determine the largest
	findContours(eyeCrop, contours, hierarchy, CV_RETR_TREE, CV_CHAIN_APPROX_SIMPLE, cv::Point(0, 0));
	if (contours.size() > 0)
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
		// once largest is found, calculate image moments to approximate pupil location
		cv::Moments mo = moments(contours[largest], false);
		cv::Point eyeCenter = cv::Point((int)(mo.m10 / mo.m00), (int)(mo.m01 / mo.m00));

		return eyeCenter;
	}
	*/

	cerr << "Error in findPupil: No contours detected." << endl;

	return cv::Point(-1, -1);
}

/*
*	checkTemplate
*	Description: checks new frame against template and if good match is found returns the
*	location of the new faceCrop and the difference in locations between the faceCrop and template
*
*	return false if a valid match was not found
*/
bool EyeLogic::checkTemplate(cv::Mat frame, cv::Rect * faceCrop, cv::Point * frameDifference)
{
	//simple filtering
	cv::Mat filteredFrame;

	if (frame.empty())
	{
		cerr << "Error in checkTemplate: input frame was empty." << endl;
		return false;
	}

	cvtColor(frame, filteredFrame, CV_BGR2GRAY);
	equalizeHist(filteredFrame, filteredFrame);

	int result_cols = filteredFrame.cols - userTemplate.cols + 1;
	int result_rows = filteredFrame.rows - userTemplate.rows + 1;

	cv::Mat result;
	result.create(result_rows, result_cols, CV_32FC1);

	matchTemplate(filteredFrame, userTemplate, result, CV_TM_CCOEFF_NORMED);
	
	//gather the match location and other metrics
	double minVal; double maxVal; cv::Point minLoc; cv::Point matchLoc;
	minMaxLoc(result, &minVal, &maxVal, &minLoc, &matchLoc, cv::Mat());

	/*if (maxVal < 0.9) //this threshold needs to be tinkered with!!!
	{
		cerr << "Error in checkTemplate: No suitable match was found." << endl;
		return false;
	}*/

	//set return arguments
	*faceCrop = faceRect;
	faceCrop->x = matchLoc.x;
	faceCrop->y = matchLoc.y - floor(faceRect.height*0.6); //this is because the actual template and faceRect don't share the same y value
	frameDifference->x = matchLoc.x - faceRect.x;
	frameDifference->y = matchLoc.y - floor(faceRect.height*0.6) - faceRect.y;
	return true;
}
