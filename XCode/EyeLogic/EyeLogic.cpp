#include "EyeLogic.h"
#include <unistd.h>

Mat* loadImageAtPath(string path)
{
    Mat* result = new Mat();
    *result = imread(path, CV_LOAD_IMAGE_COLOR);
    return result;
}

void loadReferenceImages()
{
    
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

Mat *EyeLogicAlg::cameraCapture(){
    Mat capture;
    VideoCapture cap(0);
    sleep(2);
    cap.read(capture);
    Mat *image = new Mat();
    *image = capture;
    imshow("ma face", *image);
    waitKey(0);
    return image;
}


EyeLogicAlg::EyeLogicAlg()
{
    eyeLDetector.load("haarcascade_lefteye_2splits.xml");
    eyeRDetector.load("haarcascade_righteye_2splits.xml");
    faceDetector.load("haarcascade_frontalface_default.xml");
}

bool EyeLogicAlg::detectEyes(Mat *image)
{

    vector<Rect_<int> > faceCoord;
    vector<Rect_<int> > eyesCoord;

//    image = loadImageAtPath(pathToImage);
//    if(image->empty())
//    {
//        cerr << "Cannot load image" << endl;
//        return false;
//    }
    
    //Detect faces in picture
    double myTime = getTickCount();
    imshow("face", *image);
    waitKey(0);
    faceDetector.detectMultiScale(*image, faceCoord, 1.2, 3, 0, CvSize(150,150));
    myTime = getTickCount() - myTime;
    cout << myTime/getTickFrequency() << endl;
    
    if(faceCoord.capacity() < 1)
    {
        cerr << "Did not find any faces" << endl;
        return false;
    }
    Mat cutoutFace = Mat(*image,faceCoord[0]);
    Rect roiL = Rect((size_t)(cutoutFace.cols*0.1), (size_t)(cutoutFace.rows*0.2), (size_t)(cutoutFace.cols*0.4), (size_t)(cutoutFace.rows*0.30));
    Rect roiR = Rect((size_t)(cutoutFace.cols*0.5), (size_t)(cutoutFace.rows*0.2), (size_t)(cutoutFace.cols*0.4), (size_t)(cutoutFace.rows*0.30));
    Mat cutoutLFace = Mat(cutoutFace,roiL);
    Mat cutoutRFace = Mat(cutoutFace,roiR);
    eyeLDetector.detectMultiScale(cutoutLFace, eyesCoord, 1.1, 3, 0, CvSize(40,40));
    if(eyesCoord.capacity() < 1)
    {
        //Set Left Blink
    }
    else
    {
        Mat leftEye = Mat(cutoutLFace,eyesCoord[0]);
        Rect eyeRoiL = Rect(0,(size_t)(leftEye.rows*0.18), leftEye.cols, (size_t)(leftEye.rows-(leftEye.rows*0.18)));
        captureEyes.push_back(Mat(leftEye, eyeRoiL));
    }
    //Clear and reuse matrix
    eyesCoord.clear();
    eyeRDetector.detectMultiScale(cutoutRFace, eyesCoord, 1.1, 3, 0, CvSize(40,40));
    if(eyesCoord.capacity() < 1)
    {
        //Set Right Blink
    }
    else
    {
        Mat rightEye = Mat(cutoutRFace,eyesCoord[0]);
        Rect eyeRoiR = Rect(0,(size_t)(rightEye.rows*0.18), rightEye.cols, (size_t)(rightEye.rows-(rightEye.rows*0.18)));
        captureEyes.push_back(Mat(rightEye,eyeRoiR));
    }
    return true;
}
