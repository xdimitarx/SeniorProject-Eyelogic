#pragma once

#ifndef POUNEH
#define POUNEH
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

bool startCap();
void getReferenceImages();

void majumder();
void averageEyeCenterMethod();
void lotsOfTheProgram();
void getReferenceTop();
void getReferenceBottom();
void getReferenceLeft();
void getReferenceRight();
void updateBoundaryWindows(std::vector<cv::Rect_<int>>& eyes, cv::Rect_<int>& rightEyeBounds, cv::Rect_<int>& leftEyeBounds);
void getReferenceImage(std::vector<cv::Rect_<int>>& eyes, void(*function)(void), cv::Mat & refImage, std::string name, cv::Point & _far);

#endif