#include "EyeLogic.h"  

Mat* loadImageAtPath(string path)
{
	Mat * result = new Mat();
	*result = imread(path, CV_LOAD_IMAGE_COLOR);
	return result;
}

void loadReferenceImages()
{

}

bool detectEyes()
{
	CascadeClassifier eyeDetector;
	Mat * eyesDetected = (Mat *)malloc(sizeof(Mat)*2);
	vector< Rect_<int> > eyes;

	eyeDetector.load("haarcascade_eye_tree_eyeglasses.xml");
	string path = "Test1.jpg";
	Mat * image = loadImageAtPath(path);
	eyeDetector.detectMultiScale((*image), eyes);

	Mat eyeL, eyeR;
	eyeL = (*image)(eyes[0]);
	//eyeR = (*image)(eyes[1]);

	path = "Test2.jpg";
	imwrite(path, eyeL);
}

void approximateAngle()
{

}

void angleToMouseMov()
{

}

bool initEyeLogic()
{
	//If previous storage
	loadReferenceImages();
}

bool setupEyeLogic()
{
	//runs setup sequence
}

bool runEyeLogic(/* Other Arguments */float * mouseMoveValues)
{
	if(referenceLoaded)
	{
		detectEyes();
		approximateAngle();
		angleToMouseMov();
	}
}