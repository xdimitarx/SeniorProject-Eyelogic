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

    //Cutout Eyebrow
    Rect eyebrowCrop = Rect(0, (size_t)original.rows*0.4, (size_t)original.cols, (size_t)original.rows*0.5);
    original = Mat(original, eyebrowCrop);

    cvtColor(original, filtered, CV_BGR2GRAY);
    equalHist();
    binaryThreshForIris();
    applyGaussian();
    // imshow("original", original);
    // imshow("yaymofo", filtforIris);
    // waitKey(0);
    addLighting(40);
    binaryThresh();

    if(findPupil())
    {
        findEyeCorner();
    }
    else
    {
        //Blink??
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

void Eye::binaryThresh()
{
    threshold(filtered, filtered, 122, 255, THRESH_BINARY);
}

void Eye::binaryThreshForIris()
{
    threshold(filtered, filtforIris, 5, 255, THRESH_BINARY_INV);
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
    if(contours.size() != 0)
    {
        double area = 0;
        int largest = 0;
        for (int i = 0; i < contours.size(); ++i)
        {
            double calculatedArea = contourArea(contours[i], false);
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
    imshow("eyeCorner", filtered);
    waitKey(0);
    cout << eyeCenter.y << " " << eyeRadius << endl;
    size_t extreme;
    size_t rowVal;
    if(leftEye)
    {
        extreme = eyeCenter.x+eyeRadius;
        for(int y = eyeCenter.y; y < eyeCenter.y+eyeRadius; y++)
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
//                        else if(extreme != eyeCenter.x+eyeRadius)
//                        {
//                            if(x < extreme - (extreme - (eyeCenter.x+eyeRadius))/3)
//                            {
//                                x = filtered.cols;
//                                y = eyeCenter.y+eyeRadius;
//                            }
//                        }
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
    eyeCorner = Point(extreme, rowVal);
    cout << "extreme = " << extreme << endl;
    cout << "rowVal = " << rowVal << endl;
    circle(filtered, eyeCorner, 4, Scalar(122,122,122), 1);
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
    return Point();
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
