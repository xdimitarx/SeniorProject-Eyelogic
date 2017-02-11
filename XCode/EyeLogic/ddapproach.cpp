#include "EyeLogic.h"

using namespace std;
using namespace cv;

bool EyeLogicAlg::approximateAngle(Point_<int> *xyValues)
{
    vector<Mat> filtered;
    for (int i = 0; i < captureEyes.size(); ++i)
    {
        Mat* tobeFiltered = &captureEyes.at(i);
        cvtColor(*tobeFiltered, *tobeFiltered, CV_BGR2GRAY);
        equalizeHist(*tobeFiltered, *tobeFiltered);
        
        //blur(*tobeFiltered, *tobeFiltered, CvSize(70,70));
        add(*tobeFiltered, Scalar(30,30,30), *tobeFiltered);
        threshold(*tobeFiltered, *tobeFiltered, 122, 255, THRESH_BINARY);
        GaussianBlur(*tobeFiltered, *tobeFiltered, CvSize(3,3), 0, 0);
        vector<Vec3f> circles;
        if(i == 0)
        {
            imshow("sup",*tobeFiltered);

        }
        else
        {
            imshow("sup1",*tobeFiltered);
        }
        HoughCircles(*tobeFiltered, circles, CV_HOUGH_GRADIENT, 2, (*tobeFiltered).rows, 70, 50, 20, (*tobeFiltered).rows/3.0);
        cout << circles.capacity() << endl;
        Point center(cvRound(circles[0][0]), cvRound(circles[0][1]));
        int radius = cvRound(circles[0][2]);
        circle(*tobeFiltered, center, radius, Scalar(255,255,255), 3, 8, 0);
        circle(*tobeFiltered, center, 3, Scalar(255,255,255), 3, 8, 0);
        cout << tobeFiltered->cols << ":" << center.x << ":" << center.y << endl;
        filtered.push_back(*tobeFiltered);
        imshow(to_string(i), filtered.at(i));
        if(i == 1)
        {
            Rect roi = Rect((size_t)tobeFiltered->cols*0.18,(size_t)tobeFiltered->rows*0.5, (size_t)tobeFiltered->cols*0.25, (size_t)tobeFiltered->rows*0.27);
            Mat corner = Mat(*tobeFiltered, roi);
            imshow("yay", corner);
            vector<Vec2f> lines;
            HoughLines(corner, lines, 1, CV_PI/180, 100, 0, 0 );
            //vector<Point2f> corners;
            //goodFeaturesToTrack(corner, corners, 1, 0.5, 0.5, Mat(), 2);
            // corners[0].x = corners[0].x + tobeFiltered->cols*0.18;
            // corners[0].y = corners[0].y + tobeFiltered->rows*0.5;
            //cout << corners[0].x << ":" << corners[0].y << endl;
            //circle(corner,corners[0],5,Scalar(255,255,255), 3, 8, 0);
            Vec2f l = lines[0];
            Vec2f a = lines[1];
            line(corner, Point(l[0],l[1]), Point(a[2],a[3]), Scalar(255,255,255), 3, CV_AA);
            imshow("corner1", corner);
            waitKey(0);
        }
        else
        {
            Rect roi = Rect((size_t)tobeFiltered->cols*0.58,(size_t)tobeFiltered->rows*0.5, (size_t)tobeFiltered->cols*0.25, (size_t)tobeFiltered->rows*0.27);
            Mat final = Mat(*tobeFiltered, roi);
            imshow("yay1", final);
        }
        
    }
    waitKey(0);
    
    return true;
}

void EyeLogicAlg::angleToMouseMov()
{
    
}
