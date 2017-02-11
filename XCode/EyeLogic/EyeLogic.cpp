#include "EyeLogic.h"

Mat loadImageAtPath(string path)
{
    Mat result = imread(path, CV_LOAD_IMAGE_COLOR);
    return result;
}

Mat cameraCapture(){
    Mat capture;
    VideoCapture cap(0);
    sleep(2);
    cap.read(capture);
    return capture;
}


Eye::Eye(string pathToClassifier, bool left)
{
    detector.load(pathToClassifier.c_str());
    leftEye = left;
}

Eye::~Eye()
{

}

bool Eye::detectKeyFeatures(Mat input)
{
    vector<Rect_<int> > eyesCoord;
    detector.detectMultiScale(input, eyesCoord, 1.1, 3, 0, CvSize(40,40));
    if(eyesCoord.capacity() <= 0)
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
    cvtColor(original, filtered, CV_BGR2GRAY);
    equalHist();
    addLighting(10);
    binaryThresh();
    applyGaussian();
    if(findPupil())
    {
        imshow("cropped1", filtered);
        waitKey(0);
        findEyeCorner();
    }
    else
    {
        //Blink??
    }

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

void Eye::binaryThresh()
{
    threshold(filtered, filtered, 122, 255, THRESH_BINARY);
}


void Eye::applyGaussian()
{
    GaussianBlur(filtered, filtered, CvSize(3,3), 0, 0);
}

bool Eye::findPupil()
{
    vector<Vec3f> circles;
    imshow("pupil", filtered);
    waitKey(0);
    HoughCircles(filtered, circles, CV_HOUGH_GRADIENT, 2, filtered.rows, 70, 50, 20, filtered.rows/3.0);
    if(circles.capacity() > 0)
    {
        eyeCenter = Point(cvRound(circles[0][0]), cvRound(circles[0][1]));
        return true;
    }
    cerr << "findPupil: COULDN'T DETERMINE IRIS" << endl;
    return false;

}

bool Eye::findEyeCorner()
{
    /*
    *
    * Need to accurately determine ROI for Eye
    *
    */
    Mat corner; // <------ NEED TO REPLACE
    vector<vector<Point> > contours;
    vector<Vec4i> hierarchy;
    findContours(corner, contours, hierarchy, CV_RETR_TREE, CV_CHAIN_APPROX_SIMPLE, Point(0,0));
    for (int i = 0; i < contours.capacity(); ++i)
    {
        if(contourArea(contours[i]) > 30)
        {
            size_t extreme;
            if(leftEye)
            {
                extreme = 0;
            }
            else
            {
                extreme = corner.cols;
            }
            
            for (int j = 0; j < contours[i].capacity(); ++j)
            {
                Point pt = contours[i][j];
                if(leftEye)
                {
                    if(pt.x > extreme)
                    {
                        extreme = pt.x;
                        eyeCorner = contours[i][j];
                    }
                }
                else
                {
                    if(pt.x < extreme)
                    {
                        extreme = pt.x;
                        eyeCorner = contours[i][j];
                    }
                }
                
            }
        }
    }
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
    faceDetector.detectMultiScale(frame, faceCoord, 1.2, 3, 0, CvSize(150,150));
    if(faceCoord.capacity() < 1)
    {
        cerr << "insertFrame: DID NOT FIND ANY FACES" << endl;
        return false;
    }
    Mat cutoutFace = Mat(frame, faceCoord[0]);
    Rect roiL = Rect(0, (size_t)cutoutFace.rows*0.15, (size_t)cutoutFace.cols*0.5, (size_t)cutoutFace.rows*0.8);
    Rect roiR = Rect((size_t)cutoutFace.cols*0.5, (size_t)cutoutFace.rows*0.15, (size_t)(cutoutFace.cols*0.5), (size_t)cutoutFace.rows*0.8);
    Mat leftHalf = Mat(cutoutFace, roiL);
    Mat rightHalf = Mat(cutoutFace, roiR);
    return (leftEye.detectKeyFeatures(leftHalf) && rightEye.detectKeyFeatures(rightHalf));
}

Point ImgFrame:: getCursorXY()
{
    /*
    *
    * To be implemented
    *
    */
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