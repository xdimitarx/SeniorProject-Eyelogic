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
		//logError("Error in insertFrame: Frame was empty.");
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
			//logError("Error in insertFrame: No faces detected.");
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
			//logError("Error in insertFrame: Template matching failed.");
			return false;
		}
	}

	if (faceCrop.x < 0 || faceCrop.x > frame.cols || faceCrop.y < 0 || faceCrop.y > frame.rows ||  faceCrop.width <= 0 || faceCrop.height <= 0) {
		logError("Facecrop is a bad ROI for cropping mmatrix");
		return false;
	}
	
	cv::Mat cropFace = frame(faceCrop);

	

	// Check for force or if eye template does not exist
	if (!eyeTemplatesExists || forceNewTemplate)
	{
		// Generates the eyebounds from the image and sets them as a template
		if (!createEyeBounds(cropFace))
		{
			//logError("Error in insertFrame: Could not create eye bounds.");
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
		//logError("Error in insertFrame: Could not detect pupils!");
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

		faceBound.y += floor(faceBound.height*0.55);
		faceBound.height = faceBound.height*0.15;

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

cv::Point EyeLogic::eyeVectorToScreenCoord()
{
	//cout << "ScreenMap: " << screenMap.x << "  " << screenMap.y << "\t";
    distance = cv::Point(ref_Left.x - ref_Right.x, ref_Bottom.y - ref_Top.y);
	if (!Calibrated(true))
	{
		//logError("Error in eyeVectorToScreenCoord: EyeLogic not calibrated.");
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
		//logError("Error in eyeVectorToScreenCoord: EyeVector not in bounds of reference images.");
		/*
		cerr << "\taverageLocal.x  " << averageLocal.x << "\tref_Right.x" << ref_Right.x << endl;
		cerr << "\taverageLocal.x  " << averageLocal.x << "\tref_Left.x" << ref_Left.x << endl;
		cerr << "\taverageLocal.y  " << averageLocal.y << "\tref_Top.y" << ref_Top.y << endl;
		cerr << "\taverageLocal.y  " << averageLocal.y << "ref_Bottom.y" << ref_Bottom.y << endl;
		*/
		cout << endl;
        return cv::Point(-1, -1);
	}

	destinationNew.x = (screenResolution.x - ((averageLocal.x - ref_Right.x) * screenResolution.x / distance.x));
	destinationNew.y = (averageLocal.y - ref_Top.y) * screenResolution.y / distance.y; //screenRes.y / 2;

	if (destinationOld != destinationNew) {
		destinationOld = destinationNew;
		delta = cv::Point((destinationNew.x - screenMap.x) / screenResolution.x * 80, (destinationNew.y - screenMap.y) / screenResolution.y * 80);
	}

	if (delta.x > 0) {
		//screenMap.x = min(destinationNew.x, screenMap.x + delta.x);
		screenMap.x = min(destinationNew.x, screenMap.x + 1);
	}
	else {
		//screenMap.x = std::max(destinationNew.x, screenMap.x + delta.x);
		screenMap.x = std::max(destinationNew.x, screenMap.x - 1);
	}
	if (delta.y > 0) {
		//screenMap.y = min(destinationNew.y, screenMap.y + delta.y);
		screenMap.y = min(destinationNew.y, screenMap.y + 1);
	}
	else {
		//screenMap.y = std::max(destinationNew.y, screenMap.y + delta.y);
		screenMap.y = std::max(destinationNew.y, screenMap.y - 1);
	}

	cv::rectangle(currentFrame, cv::Rect(ref_Right.x, ref_Top.y, ref_Left.x - ref_Right.x, ref_Bottom.y - ref_Top.y), cv::Scalar(0x80, 0x0, 0xff));

	//Enforce screen resolution as boundaries for movement of cursor
	if (screenMap.x >= 0 && screenMap.y >= 0 && screenMap.x <= screenResolution.x && screenMap.y <= screenResolution.y) {
		cout << screenMap.x << "   " << screenMap.y << endl;
		return screenMap;

	}
	else {
		//logError("Error in eyeVectorToScreenCoord: Coordinates not within screen bounds.");
		cerr << "\tScreenMap.x: " << screenMap.x << "\tScreenMap.y: " << screenMap.y << endl;
	}
	cout << endl;
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

// returns a stabalized eyeVector given a vector of eyeVectors
bool EyeLogic::setStabalizedPoint(std::vector<cv::Point>data, RefPoint refPosition){

    std::vector<cv::Point>subData;
    
    if(data.size() == 0){
        logError("Error in setStabalizedPoint: size of data is 0");
        return false;
    }
    
    // middle value index
    int index = ceil(data.size() / 2);
    
    /*******************************
     * get mean of larger X values *
     *******************************/
    if(refPosition == RefPoint::LEFT){
        
        // sort array by x
        std::sort(data.begin(), data.end(),
                      [](const cv::Point p1, const cv::Point p2){return p1.x < p2.x;});
    
        // Inputs: sub vector from [index, END] ; reference point
        cv::Point newPoint  = findMean( std::vector<cv::Point>(data.begin() + index, data.end()), RefPoint::LEFT );
        
        // set reference point
        if(newPoint != cv::Point(-1, -1)){
            ref_Left = newPoint;
        } else {
            return false;
        }
        

    /********************************
     * get mean of smaller X values *
     ********************************/
    }  else if(refPosition == RefPoint::RIGHT){

        // sort array by x
        std::sort(data.begin(), data.end(),
                  [](const cv::Point p1, const cv::Point p2){return p1.x < p2.x;});

        // Inputs: sub vector from [index, END] ; reference point
        
        cv::Point newPoint = findMean( std::vector<cv::Point>(data.begin() , data.begin() + index), RefPoint::RIGHT );
        
        // set reference point
        if(newPoint != cv::Point(-1, -1)){
            ref_Right = newPoint;
        } else {
            return false;
        }
        
    /********************************
     * get mean of smaller Y values *
     ********************************/
    }  else if(refPosition == RefPoint::TOP){

        // sort array by y
        std::sort(data.begin(), data.end(),
                  [](const cv::Point p1, const cv::Point p2){return p1.y < p2.y; });
        
        // Inputs: sub vector from [index, END] ; reference point
        cv::Point newPoint  = findMean(  std::vector<cv::Point>(data.begin(), data.begin() + index), RefPoint::TOP );
        
        
        // set reference point
        if(newPoint != cv::Point(-1, -1)){
            ref_Top = newPoint;
        } else {
            return false;
        }
        
    /*******************************
     * get mean of larger Y values *
     *******************************/
    } else if(refPosition == RefPoint::BOTTOM){
        
        // sort array by y
        std::sort(data.begin(), data.end(),
                  [](const cv::Point p1, const cv::Point p2){return p1.y < p2.y; });
        
        // Inputs: sub vector from [index, END] ; reference point
        cv::Point newPoint  = findMean(   std::vector<cv::Point>(data.begin() + index, data.end()), RefPoint::BOTTOM );
        
        // set reference point
        if(newPoint != cv::Point(-1, -1)){
            ref_Bottom = newPoint;
        } else {
            return false;
        }

    }
}

// returns vector of reference point data in (LEFT,RIGHT,TOP,BOTTOM) order
vector <cv::Point>  EyeLogic::getReferencePointData()
{
	vector <cv::Point>  referencePoints;
	if (!Calibrated(false))
	{
		//logError("Error in getReferencePointData: EyeLogic not calibrated.");
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
		//logError("Error in setReferencePointData: Input vector does not contain exactly 4 reference points.");
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
		if (ref_Bottom.y < ref_Top.y || ref_Left.x < ref_Right.x) //CERTIFIED RIGHT PLEASE DON'T CHANGE, THINK ABOUT IT REALLY HARD TRUE ON ERROR
		{
			//logError("Error in Calibrated: Valid check failed.");
			//cerr << "ref_Bottom.y\t" << ref_Bottom.y << "\t ref_Top.y\t" << ref_Top.y << endl;
			//cerr << "ref_Left.x\t" << ref_Left.x << "\t ref_Right.x\t" << ref_Right.x << endl;
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
	cv::Mat leftHalf = faceCrop(cv::Rect(0, 0, (int)(faceCrop.cols*0.5), faceCrop.rows));
	cv::Mat rightHalf = faceCrop(cv::Rect((int)(faceCrop.cols*0.5), 0, (int)(faceCrop.cols*0.5), faceCrop.rows));

	//Capture (our right) User's Left Eye Bound box
	vector<cv::Rect_<int>> eyes;
	rightEyeExtractor.detectMultiScale(rightHalf, eyes);

	if (eyes.size() == 0)
	{
		//logError("Error in createEyeBounds: Right Eye could not be detected.");
		return false;
	}

	rightEyeBound = eyes[0]; //Class Variable
	rightEyeBound.x += faceCrop.cols / 2; // this is to account for split in half above
	rightEyeBound.y = rightEyeBound.y + (int)(rightEyeBound.height*0.4); //Crop Eyebrow
	rightEyeBound.height *= 0.6; //Crop Eyebrow

	//Capture (our left) User's Right Eye Bound box
	eyes.clear();
	leftEyeExtractor.detectMultiScale(leftHalf, eyes);

	if (eyes.size() == 0)
	{
		//logError("Error in createEyeBounds: Left Eye could not be detected.");
		return false;
	}

	leftEyeBound = eyes[0];
	leftEyeBound.y = leftEyeBound.y + (int)(leftEyeBound.height*0.4);
	leftEyeBound.height *= 0.6;

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

	//logError("apply pupil before", result);

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
		//logError("Error in applyPupilFilters: No valid black pixels detected. SERIOUS ERROR.");
		return cv::Mat();
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
		//logError("Error in applyPupilFilters: No valid black pixels detected. SERIOUS ERROR.");
		return cv::Mat();
	}

	int slopeY = (right.y - left.y);
	int slopeX = (right.x - left.x);

	double deltY = (double)slopeY / (double)slopeX;

	double limitY = left.y;

	//create a line between the two points above and make everything above it white
	//this appears to cut the top portions of the eye and eyebrow off due to the shape of our eye sockets
	// the bottome portion can then be flipped which usually means the bottom half of the pupil mirrored
	for (int i = left.x; i <= right.x; i++)
	{
		for (int j = 0; j <= round(limitY); j++)
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
			result.at<uchar>(j, i) = max(topHalf.at<uchar>(j, i), result.at<uchar>(j,i));
		}
	}

	if (result.empty())
	{
		//logError("Error in applyPupilFilters: Empty result crop", result);
		return cv::Mat();
	}
	
	//logError("Debug-Apply Pupil After", result);

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
cv::Point EyeLogic::findPupil(cv::Mat eyeCrop) 
{
	//cv::Mat eyeCropGray;
	//eyeCrop.copyTo(eyeCropGray);
	//cv::equalizeHist(eyeCropGray, eyeCropGray);
	//cv::add(eyeCropGray, cv::Scalar(50), eyeCropGray);
	//cv::equalizeHist(eyeCropGray, eyeCropGray);
	//cv::threshold(eyeCropGray, eyeCropGray, 10, 255, cv::THRESH_BINARY);
	//cv::GaussianBlur(eyeCropGray, eyeCropGray, cv::Size(9, 9), 0, 0);
	//cv::Mat erodeElement = getStructuringElement(cv::MORPH_ELLIPSE, cv::Size(4, 4));
	//cv::dilate(eyeCropGray, eyeCropGray, erodeElement);

	if (eyeCrop.empty())
	{
		return cv::Point(-1, -1);
	}

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

		//cv::Rect bounding = boundingRect(contours[largest]);
		//int eyeRadius = cvRound(bounding.height*1.05);

		return eyeCenter;
	}
	//logError("Error in findPupil: No contours detected.", eyeCrop);
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
		//logError("Error in checkTemplate: input frame was empty.");
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

	if (maxVal < 0.73) //initial testing showed that good matches are .75 up
	{
		//logError("Error in checkTemplate: No suitable match was found.");
		return false;
	}

	//set return arguments
	*faceCrop = faceRect;
	faceCrop->x = matchLoc.x;
	faceCrop->y = matchLoc.y - floor(faceRect.height*0.55); //this is because the actual template and faceRect don't share the same y value
	frameDifference->x = matchLoc.x - faceRect.x;
	frameDifference->y = matchLoc.y - floor(faceRect.height*0.55) - faceRect.y;

	//logError(to_string(maxVal));
	return true;
}

void EyeLogic::logError(std::string message, cv::Mat image)
{
    string fileName = std::to_string(rand());
    string logPath = fileName + ".txt";
    
    cerr << fileName << " : " << message << endl;
    
    std::ofstream outputfile(logPath, std::ios::out);
    
    outputfile << fileName << " : " << message << endl;
    
    outputfile << "FaceRect (x,y,w,h)" << endl;
    outputfile << faceRect.x << " " << faceRect.y << " " << faceRect.width << " " << faceRect.height << endl;
    
    outputfile << "LeftEyeBound (x,y,w,h)" << endl;
    outputfile << leftEyeBound.x << " " << leftEyeBound.y << " " << leftEyeBound.width << " " << leftEyeBound.height << endl;
    
    outputfile << "RightEyeBound (x,y,w,h)" << endl;
    outputfile << rightEyeBound.x << " " << rightEyeBound.y << " " << rightEyeBound.width << " " << rightEyeBound.height << endl;
    
    if (Calibrated(false))
    {
        outputfile << "Bounds -- Left, Right, Top, Bottom" << endl;
        outputfile << ref_Left.x << ", " << ref_Right.x << ", " << ref_Top.y << ", " << ref_Bottom.y;
    }
    
    outputfile.close();
    
    if (faceTemplateExists)
    {
        imwrite(fileName + "t.jpg", userTemplate);
    }
    
    imwrite(fileName + ".jpg", currentFrame);
    
    if (!image.empty())
    {
        imwrite(fileName + "cust.jpg", image);
    }
}

// return mean of data based on ref
cv::Point EyeLogic::findMean(std::vector<cv::Point>subData, RefPoint refPosition){
    double sum;
    cv::Point newPoint;
    
    if(subData.size() <= 0){
        logError("Error in findMean: vector of size 0");
        return cv::Point(-1, -1);
    }
    
    if( (refPosition == RefPoint::LEFT) || (refPosition == RefPoint::RIGHT) ){
        
        for(int i = 0; i < subData.size(); i++){
            sum += subData[i].x;
        }
        
        newPoint.x = sum / subData.size();
        newPoint.y = -1;

    } else if( (refPosition == RefPoint::TOP) || (refPosition == RefPoint::BOTTOM) ){
        
        for(int i = 0; i < subData.size(); i++){
            sum += subData[i].y;
        }
        
        newPoint.x = -1;
        newPoint.y = sum / subData.size();
    }
    
    return newPoint;
}

