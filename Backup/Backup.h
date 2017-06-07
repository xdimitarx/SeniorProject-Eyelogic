#pragma once

#ifndef BACKUP
#define BACKUP
#include "windows.h"

#include <opencv2/core.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/objdetect.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/features2d.hpp>

#include <iostream>
#include <vector>
#include <stdlib.h>





cv::Point findPupil(cv::Mat eyeCrop);
void averageEyeCenterMethod();
void updateBoundaryWindows(std::vector<cv::Rect_<int>>& eyes, cv::Rect_<int>& rightEyeBounds, cv::Rect_<int>& leftEyeBounds);
int getReferenceImage(std::vector<cv::Rect_<int>>& eyes, cv::Point center, cv::Mat & refImage, std::string name, cv::Point & _far, int distanceBetweenPupil);
int calibrate(std::vector<cv::Rect_<int>>& eyes, cv::Point& top, cv::Point& bottom, cv::Point& farLeft, cv::Point& farRight, cv::Point& distance, cv::Point& referenceMean, cv::Rect_<int>& rightEyeBounds, cv::Rect_<int>& leftEyeBounds, int& distanceBetweenPupil);
bool startCap();

cv::Mat createForeheadTemplate();
cv::Point matchTemplate( cv::Mat templ);


#endif