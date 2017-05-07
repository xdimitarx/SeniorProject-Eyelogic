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
void getReferenceLeft();
void getReferenceRight();

#endif