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
    addLighting(40);
    binaryThresh();
    applyGaussian();

    if(findPupil())
    {
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
    Rect eyebrowCrop = Rect(0, (size_t)filtered.rows*0.4, (size_t)filtered.cols, (size_t)filtered.rows*0.5);
    filtered = Mat(filtered, eyebrowCrop);

    vector<Vec3f> circles;
    HoughCircles(filtered, circles, CV_HOUGH_GRADIENT, 1.5, filtered.rows/10.0);
    if(circles.capacity() > 0)
    {
        eyeCenter = Point(cvRound(circles[0][0]), cvRound(circles[0][1]));
        eyeRadius = cvRound(circles[0][2]);
        circle(filtered, eyeCenter, eyeRadius, Scalar(255,255,255), 2);
        imshow("eye", filtered);
        waitKey(0);
        return true;
    }
    cerr << "findPupil: COULDN'T DETERMINE IRIS" << endl;
    return false;

}

bool Eye::findEyeCorner()
{
    circle(filtered, eyeCenter, eyeRadius, Scalar(0,0,0), -2);
    size_t extreme = eyeCenter.x;
    size_t rowVal = 0;
    if(leftEye)
    {
        bool white = false;
        for(int i = eyeCenter.y-eyeRadius; i > eyeCenter.y+eyeRadius; i++)
        {
            for(int j = eyeCenter.x; j < filtered.cols; j++)
            {
                if(white)
                {
                    if(filtered.at<int>(i,j) == 0)
                    {
                        // If setting the extreme, don't check for past eyecorner
                        if(j > extreme)
                        {
                            extreme = j;
                            rowVal = i;
                        }
                        else if(extreme != eyeCenter.x)
                        {
                            if(j < extreme - (extreme - eyeCenter.x)/3)
                            {
                                j = filtered.cols;
                                i = eyeCenter.y-eyeRadius;
                            }
                        }
                        break;
                    }
                }
                else
                {
                    if(filtered.at<int>(i,j) == 255)
                    {
                        white = true;
                    }
                }
            }
            
        }
    }
    else
    {
        bool white = false;
        for(int i = eyeCenter.y-eyeRadius; i > eyeCenter.y+eyeRadius; i++)
        {
            for(int j = eyeCenter.x; j > 0; j--)
            {
                if(white)
                {
                    if(filtered.at<int>(i,j) == 0)
                    {
                        if(j < extreme)
                        {
                            extreme = j;
                            rowVal = i;
                        }
                        else if(extreme != eyeCenter.x)
                        {
                            if(j > extreme + (eyeCenter.x - extreme)/3)
                            {
                                j = filtered.cols;
                                i = eyeCenter.y-eyeRadius;
                            }
                        }
                        break;
                    }
                }
                else
                {
                    if(filtered.at<int>(i,j) == 255)
                    {
                        white = true;
                    }
                }
            }
            
        }
    }
    if(rowVal == 0)
    {
        return false;
    }
    eyeCorner = Point(extreme, rowVal);
    circle(filtered, eyeCorner, 4, Scalar(255,255,255), 1);
    imshow("Final", filtered);
    waitKey(0);
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