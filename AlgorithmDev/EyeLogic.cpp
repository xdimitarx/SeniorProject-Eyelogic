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
	//Mat * eyesDetected = (Mat *)malloc(sizeof(Mat)*2);
	vector<Rect_<int> > eyes;

	eyeDetector.load("haarcascade_eye_tree_eyeglasses.xml");
	string path = "Test1.jpg";
	Mat * image = loadImageAtPath(path);
	eyeDetector.detectMultiScale((*image), eyes);
	cout << eyes.capacity()-1 << endl;

	/*int i = 0;
	for (vector<Rect>::iterator itr = eyes.begin(); itr != eyes.end(); itr++)
	{
		cout << i++ << endl;
	}
	*/

	for(int i = 0; i < eyes.capacity()-1; i++)
	{
		cout << i << "     " << eyes[i] << endl;
		string path = "Test";
		path.push_back((i+2)+'0');
		path.append(".jpg");
		Mat result = (*image)(eyes[i]);
		imwrite(path, result);
	}
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