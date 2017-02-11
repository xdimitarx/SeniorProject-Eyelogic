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
            vector<vector<Point> > contours;
            vector<Vec4i> hierarchy;
            findContours(corner, contours, hierarchy, CV_RETR_TREE, CV_CHAIN_APPROX_SIMPLE, Point(0,0));
            Point theCorner;
            for (int i = 0; i < contours.capacity(); ++i)
            {
                if(contourArea(contours[i]) > 30)
                {
                    size_t extreme = corner.cols;
                    for (int j = 0; j < contours[i].capacity(); ++j)
                    {
                        Point pt = contours[i][j];
                        if(pt.x < extreme)
                        {
                            extreme = pt.x;
                            theCorner = contours[i][j];
                        }
                    }
                }
            }
            circle(corner,theCorner,5,Scalar(255,255,255), 3, 8, 0);
            imshow("corner1", corner);
        }
        else
        {
            Rect roi = Rect((size_t)tobeFiltered->cols*0.58,(size_t)tobeFiltered->rows*0.5, (size_t)tobeFiltered->cols*0.25, (size_t)tobeFiltered->rows*0.27);
            Mat corner = Mat(*tobeFiltered, roi);
            vector<vector<Point> > contours;
            vector<Vec4i> hierarchy;
            findContours(corner, contours, hierarchy, CV_RETR_TREE, CV_CHAIN_APPROX_SIMPLE, Point(0,0));
            Point theCorner;
            for (int i = 0; i < contours.capacity(); ++i)
            {
                if(contourArea(contours[i]) > 30)
                {
                    size_t extreme = 0;
                    for (int j = 0; j < contours[i].capacity(); ++j)
                    {
                        Point pt = contours[i][j];
                        if(pt.x > extreme)
                        {
                            extreme = pt.x;
                            theCorner = contours[i][j];
                        }
                    }
                }
            }
            circle(corner,theCorner,5,Scalar(255,255,255), 3, 8, 0);
            imshow("corner2", corner);

        }
        
    }
    waitKey(0);
    
    return true;
}

void EyeLogicAlg::angleToMouseMov()
{
    
}
