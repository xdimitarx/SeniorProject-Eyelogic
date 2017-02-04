#include "EyeLogic.h"  

Mat* loadImageAtPath(string path)
{
	Mat* result = new Mat();
	*result = imread(path, CV_LOAD_IMAGE_COLOR);
	return result;
}

void loadReferenceImages()
{

}

bool detectEyes(string pathToImage, vector<Mat> *eyes, int *eyesFlagged)
{
	//Testing
	//pathToImage = "Test1.jpg";

	//Code
	CascadeClassifier eyeDetector;
	CascadeClassifier faceDetector;
	eyeDetector.load("haarcascade_eye_tree_eyeglasses.xml");
	faceDetector.load("haarcascade_frontalface_default.xml");
	//Mat * eyesDetected = (Mat *)malloc(sizeof(Mat)*2);
	vector<Rect_<int> > faceCoord;
	vector<Rect_<int> > eyesCoord;
	
	Mat *image = loadImageAtPath(pathToImage);
	if(image->empty())
	{
		cerr << "Cannot load image" << endl;
		return false;
	}

	//Detect faces in picture
	double myTime = getTickCount();
	faceDetector.detectMultiScale((*image), faceCoord, 1.2, 3, 0, CvSize(150,150));
	myTime = getTickCount() - myTime;
	cout << myTime/getTickFrequency() << endl;

	if(faceCoord.capacity() < 1)
	{
		cerr << "Did not find any faces" << endl;
		return false;
	}
	Mat cutoutFace = Mat((*image),faceCoord[0]);
	imshow("My face", cutoutFace);
	waitKey(0);
	delete image;
	Rect roiL = Rect((size_t)(cutoutFace.cols*0.1), (size_t)(cutoutFace.rows*0.2), (size_t)(cutoutFace.cols*0.4), (size_t)(cutoutFace.rows*0.30));
	Rect roiR = Rect((size_t)(cutoutFace.cols*0.5), (size_t)(cutoutFace.rows*0.2), (size_t)(cutoutFace.cols*0.4), (size_t)(cutoutFace.rows*0.30));
	Mat cutoutLFace = Mat(cutoutFace,roiL);
	Mat cutoutRFace = Mat(cutoutFace,roiR);
	imshow("Winning", cutoutLFace);
	imshow("Winning Right", cutoutRFace);
	waitKey(0);
	eyeDetector.detectMultiScale(cutoutLFace, eyesCoord, 1.1, 3, 0, CvSize(40,40));
	if(eyesCoord.capacity() < 1)
	{
		//Set Left Blink
	}
	else
	{
		Rect eyeRoiL = Rect();
		imshow("EyeL", (cutoutLFace)(eyesCoord[0]));
		//eyes->push_back(Mat(cutoutLFace, ));
	}
	//Clear and reuse matrix
	eyesCoord.clear();
	eyeDetector.detectMultiScale(cutoutRFace, eyesCoord, 1.1, 3, 0, CvSize(40,40));
	if(eyesCoord.capacity() < 1)
	{
		//Set Right Blink
	}
	else
	{
		Rect eyeRoiR = Rect();
		imshow("EyeR", (cutoutRFace)(eyesCoord[0]));
		waitKey(0);
		//eyes->push_back(Mat(cutoutRFace, Rect(eyesCoord[0].x, eyesCoord[0].height*0.33, eyesCoord[0].width, eyesCoord[0].height*0.67)));
	}
	return true;
}

bool initEyeLogic()
{
	//If previous storage
	loadReferenceImages();
	return false;
}

bool setupEyeLogic()
{
	//runs setup sequence
	return false;
}

bool runEyeLogic(/* Other Arguments */float * mouseMoveValues)
{
	if(referenceLoaded)
	{
		//detectEyes();
		//approximateAngle();
		//angleToMouseMov();
	}
	return false;
}