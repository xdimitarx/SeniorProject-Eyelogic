#include "stdafx.h"
#include "EyeLogic.hpp"




Mat loadImageAtPath(string path)
{
    Mat result = imread(path, CV_LOAD_IMAGE_COLOR);
    return result;
}

Mat cameraCapture(){
	VideoCapture cap;
	Mat capture;
	if (!cap.open(0))
		return capture;
	cap >> capture;
	//imshow("hi" ,capture);
	//waitKey(5000);
    return capture;
}

//Pouneh Aghababazadeh (whole function for getting reference images)
void ImgFrame::getReferenceImages()
{
    
    //    SetConsoleDisplayMode(GetStdHandle(STD_OUTPUT_HANDLE), CONSOLE_FULLSCREEN_MODE, 0);
    
    VideoCapture cap;
    if (!cap.open(0)) {
        cerr << "FAIL" << endl;
        return ;
    }
    
    int horizontal = 0;
    int vertical = 0;
    //    RECT desktop;
    //    const HWND hDesktop = GetDesktopWindow();
    //    GetWindowRect(hDesktop, &desktop);
    
    horizontal = screenResolution.x;
    vertical = screenResolution.y;
    
    Mat cue(vertical, horizontal, CV_8UC3);
    Mat flash(vertical, horizontal, CV_8UC3);
    flash = Scalar(255, 255, 255);

	cue = Scalar(0, 0, 0);

    //Top Left
    cue = Scalar(0, 0, 0);
    circle(cue, Point(0 + horizontal / 20, 0 + horizontal / 20), horizontal / 20, Scalar(0, 255, 0), -1);
    imshow("", cue);
    waitKey(2000);
    imshow("", flash);
	waitKey(50);
    
    //Bottom Left
    cue = Scalar(0, 0, 0);
    circle(cue, Point(0 + horizontal / 20, vertical - horizontal / 20), horizontal / 20, Scalar(0, 255, 0), -1);
    imshow("", cue);
    waitKey(2000);
    imshow("", flash);
	cap >> ref_topLeft;
	waitKey(50);
    
    //Center
    cue = Scalar(0, 0, 0);
    circle(cue, Point(horizontal / 2, vertical / 2), horizontal / 20, Scalar(0, 255, 0), -1);
    imshow("", cue);
    waitKey(2000);
    imshow("", flash);
	cap >> ref_bottomLeft;
	waitKey(50);
    
    //Top Right
    cue = Scalar(0, 0, 0);
    circle(cue, Point(horizontal - horizontal / 20, 0 + vertical / 20), horizontal / 20, Scalar(0, 255, 0), -1);
    imshow("", cue);
    waitKey(2000);
    imshow("", flash);
	cap >> ref_center;
	waitKey(50);
    
    //Bottom Right
    cue = Scalar(0, 0, 0);
    circle(cue, Point(horizontal - horizontal / 20, vertical - vertical / 20), horizontal / 20, Scalar(0, 255, 0), -1);
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

	imshow("", ref_topLeft);
	waitKey(1000);
	imshow("", ref_bottomLeft);
	waitKey(1000);
	imshow("", ref_center);
	waitKey(1000);
	imshow("", ref_topRight);
	waitKey(1000);
	imshow("", ref_bottomRight);
	waitKey(1000);
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
    if(leftEye){
        eyeVector.x = eyeCenter.x - eyeCorner.x;
        eyeVector.y = eyeCenter.y - eyeCenter.y;
    }
    else {
        eyeVector.x = eyeCorner.x - eyeCenter.x;
        eyeVector.y = eyeCorner.y - eyeCenter.y;
    }
}

void Eye::setEyeVector(float x, float y){
    eyeVector.x = (int) x;
    eyeVector.y = (int) y;
}
bool Eye::detectKeyFeatures(Mat input)
{
    vector<Rect_<int> > eyesCoord;
    
    detector.detectMultiScale(input, eyesCoord, 1.1, 3, 0, CvSize(40,40));
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
    
    //Cutout Eyebrow
    Rect eyebrowCrop = Rect(0, (int)(original.rows*0.4), (int)(original.cols), (int)(original.rows*0.5));
    original = Mat(original, eyebrowCrop);
    
    cvtColor(original, filtered, CV_BGR2GRAY);
    equalHist();
    
    binaryThreshForIris();
    Mat erodeElement = getStructuringElement( MORPH_ELLIPSE,Size(4,4));
    dilate(filtforIris,filtforIris,erodeElement);


    //applyGaussian();    
    equalHist();
    addLighting(-80);
    binaryThreshForSc();
    imshow("after dilate", filtforIris);
    imshow("filtered", filtered);
    waitKey(10);
    
    
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
    vector<vector<Point> > contours;
    findContours(filtforIris, contours, hierarchy, CV_RETR_TREE, CV_CHAIN_APPROX_SIMPLE, Point(0,0));
    waitKey(0);
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
        Rect bounding = boundingRect(contours[largest]);
        eyeCenter = Point(cvRound(bounding.x+bounding.width/2), cvRound(bounding.y+bounding.height/2));
        eyeRadius = cvRound(bounding.height*1.05);
        circle(filtered, eyeCenter, eyeRadius, Scalar(122,122,122), 2);
        rectangle(filtered, bounding,  Scalar(122,122,122),2, 8,0);
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
    imshow("eyeCorner", filtered);
    waitKey(0);
    cout << eyeCenter.y << " " << eyeRadius << endl;
    size_t extreme;
    size_t rowVal = 0;
    if(leftEye)
    {
        extreme = eyeCenter.x+eyeRadius;
        for(int y = (int)(eyeCenter.y-eyeRadius*0.5); y < eyeCenter.y+eyeRadius; y++)
        {
            bool white = false;
            for(int x = eyeCenter.x+eyeRadius; x < filtered.cols; x++)
            {
                if(white)
                {
                    if(filtered.at<uchar>(y,x) == 0)
                    {
                        // If setting the extreme, don't check for past eyecorner
                        if(x > extreme)
                        {
                            extreme = x;
                            rowVal = y;
                        } 
                        else if((extreme-eyeCenter.x)/10 + eyeCenter.x > x)
                        {
                            y = eyeCenter.y + (int)eyeRadius;
                        }
                        x = filtered.cols;
                    }
                }
                else
                {
                    if(filtered.at<uchar>(y,x) == 255)
                    {
                        white = true;
                    }
                }
            }
            
        }
    }
    else
    {
        extreme = eyeCenter.x-eyeRadius;
        for(int y = eyeCenter.y; y < eyeCenter.y+eyeRadius; y++)
        {
            bool white = false;
            for(int x = eyeCenter.x - eyeRadius; x > 0; x--)
            {
                if(white)
                {
                    if(filtered.at<uchar>(y,x) == 0)
                    {
                        if(x < extreme)
                        {
                            extreme = x;
                            rowVal = y;
                        }
                        //                        else if(extreme != eyeCenter.x)
                        //                        {
                        //                            if(x > extreme + (eyeCenter.x - extreme)/3)
                        //                            {
                        //                                x = filtered.cols;
                        //                                y = eyeCenter.y-eyeRadius;
                        //                            }
                        //                        }
                        break;
                    }
                }
                else
                {
                    if(filtered.at<uchar>(y, x) == 255)
                    {
                        white = true;
                    }
                }
            }
            
        }
    }
    cout << rowVal << endl;
    if(rowVal == 0)
    {
        return false;
    }
    eyeCorner = Point((int)extreme, (int)rowVal);
    cout << "extreme = " << extreme << endl;
    cout << "rowVal = " << rowVal << endl;
    circle(filtered, eyeCorner, 4, Scalar(122,122,122), 1);
    imshow("Final", filtered);
    waitKey(10);
    return true;
	*/
	Mat threshmat, dest, dest_norm, dest_norm_scaled;

	// detector parameters 
	int thresh = 200;
	int max_thresh = 255;
	int blockSize = 2;
	int apertureSize = 5;
	double k = 0.01;

	/*
	threshold(filtered, threshmat, 20, 255, THRESH_BINARY);
	// detect corners
	cornerHarris(thresh, dest, blockSize, apertureSize, k, BORDER_DEFAULT);

	// Normalize 
	normalize(dest, dest_norm, 0, 255, NORM_MINMAX, CV_32FC1, Mat());
	convertScaleAbs(dest_norm, dest_norm_scaled);

	// Draw circle around coners detected
	for (int j = 0; j < dest_norm.rows; j++)
	{
		for (int i = 0; i < dest_norm.cols; i++)
		{
			if (thresh < dest_norm.at<float>(j, i) )
			{
				circle(dest_norm_scaled, Point(i, j), 5, Scalar(0), 2, 8, 0);
			}
		}
	}

	namedWindow("corner", CV_WINDOW_AUTOSIZE);
	imshow("corner", dest_norm_scaled);
	waitKey();
	*/

	return true;


}

ImgFrame::ImgFrame(Point resolution) : leftEye("haarcascade_lefteye_2splits.xml", true), rightEye("haarcascade_righteye_2splits.xml", false)
{
    faceDetector.load("haarcascade_frontalface_default.xml");
    screenResolution = resolution;
}

ImgFrame::~ImgFrame()
{
    
}

bool ImgFrame::insertFrame(Mat frame)
{
    vector<Rect_<int> > faceCoord;

	//THIS LINE IS BREAKING THE PROGRAM when i try to run it (exceptions) -Pouneh
    faceDetector.detectMultiScale(frame, faceCoord, 1.2, 3, 0, CvSize(150,150));
	cout << faceCoord.size() << endl;

    if(faceCoord.size() < 1)
    {
        cerr << "insertFrame: DID NOT FIND ANY FACES" << endl;
        return false;
    }

	
    Mat cutoutFace = Mat(frame, faceCoord[0]);
	Rect roiL = Rect(0, (int)(cutoutFace.rows*0.15), (int)(cutoutFace.cols*0.5), (int)(cutoutFace.rows*0.8));
	Rect roiR = Rect((int)(cutoutFace.cols*0.5), (int)(cutoutFace.rows*0.15), (int)(cutoutFace.cols*0.5), (int)(cutoutFace.rows*0.8));
	Mat leftHalf = Mat(cutoutFace, roiL);
	Mat rightHalf = Mat(cutoutFace, roiR);
	return (leftEye.detectKeyFeatures(leftHalf) && rightEye.detectKeyFeatures(rightHalf));
}

bool ImgFrame::getCursorXY(Point *  result)
{
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
