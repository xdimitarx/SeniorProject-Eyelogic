#include <iostream>
#include <string>
#include <sstream>
#include <cmath>

#include <opencv2/core.hpp>
#include <opencv2/highgui.hpp>
#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/objdetect/objdetect.hpp"


using namespace std; 
using namespace cv; 

// Computes the x component of the gradient vector
// at a given point in a image.
// returns gradient in the x direction
int xGradient(Mat image, int x, int y)
{
  return image.at<uchar>(y-1, x-1) + 
    2*image.at<uchar>(y, x-1) + 
    image.at<uchar>(y+1, x-1) - 
    image.at<uchar>(y-1, x+1) - 
    2*image.at<uchar>(y, x+1) - 
    image.at<uchar>(y+1, x+1);
}

// Computes the y component of the gradient vector
// at a given point in a image
// returns gradient in the y direction

int yGradient(Mat image, int x, int y)
{
  return image.at<uchar>(y-1, x-1) + 
    2*image.at<uchar>(y-1, x) + 
    image.at<uchar>(y-1, x+1) - 
    image.at<uchar>(y+1, x-1) - 
    2*image.at<uchar>(y+1, x) - 
    image.at<uchar>(y+1, x+1);
}

int main()
{
  Mat source;
  Mat dest;
  int gx, gy, sum;
  // Load an image
  source = imread("noah.jpg", CV_LOAD_IMAGE_GRAYSCALE);
  dest = source.clone();

  // Size ksize;
  // ksize.height = 5;
  // ksize.width = ksize.height;
  // int sigmax = 10; 
  // int sigmay = 0;
  // int lowthresh = 15;
  // GaussianBlur(dest, dest, ksize, sigmax, sigmay);



  if( !source.data )
  { 
    return -1; 
  }


  for(int y = 0; y < source.rows; y++)
  {
    for(int x = 0; x < source.cols; x++)
    {
      dest.at<uchar>(y,x) = 0.0;
    }
  }

  for(int y = 1; y < source.rows - 1; y++)
  {
    for(int x = 1; x < source.cols - 1; x++)
    {
      gx = xGradient(source, x, y);
      gy = yGradient(source, x, y);
      sum = abs(gx) + abs(gy);
      sum = sum > 255 ? 255:sum;
      sum = sum < 0 ? 0 : sum;
      dest.at<uchar>(y,x) = sum;
    }
  }

  namedWindow("final");
  imshow("final", dest);
  namedWindow("initial");
  imshow("initial", source);
  waitKey();
  return 0;
}