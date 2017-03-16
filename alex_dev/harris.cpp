#include <iostream>
#include <string>
#include <sstream>
#include <cmath>

#include <opencv2/core.hpp>
#include <opencv2/highgui.hpp>
#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/objdetect/objdetect.hpp"

using namespace cv; 
using namespace std;

CascadeClassifier eye_cascade;

void apply_harris(Mat src_gray)
{
  Mat dest, dest_norm, dest_norm_scaled;

  // detector parameters 
  int thresh = 200; 
  int max_thresh = 255; 
  int blockSize = 2; 
  int apertureSize = 5; 
  double k = 0.01; 

  // detect corners
  cornerHarris(src_gray, dest, blockSize, apertureSize, k, BORDER_DEFAULT);

  // Normalize 
  normalize(dest, dest_norm, 0, 255, NORM_MINMAX, CV_32FC1, Mat());
  convertScaleAbs(dest_norm, dest_norm_scaled);

  // Draw circle around coners detected
  for (int j = 0; j < dest_norm.rows; j++)
  {
    for (int i = 0; i < dest_norm.cols; i++)
    {
      if ((int)dest_norm.at<float>(j, i) > thresh)
      {
        circle(dest_norm_scaled, Point(i, j), 5, Scalar(0), 2, 8, 0);
      }
    }
  }
  namedWindow("corner", CV_WINDOW_AUTOSIZE);
  imshow("corner", dest_norm_scaled);
  waitKey();

}

Mat apply_threshold(Mat src)
{
  Mat dest;
  threshold(src, dest, 20, 255, THRESH_BINARY);
  return dest;
}

Mat apply_threshold_inv(Mat src)
{
  Mat dest;
  threshold(src, dest, 10, 255, THRESH_BINARY_INV);
  return dest;
}


vector<Mat> detect_eyes(Mat src)
{ 
  cout << "IN DETECT EYES" << endl;
  // load cascade sheet
  eye_cascade.load("haarcascade_eye.xml");
  // vector to store eyes found
  vector<Rect> eyes; 
  vector<Mat> cutout_eyes;
  // find eyes
  eye_cascade.detectMultiScale(src, eyes, 1.1, 3, 0, Size(30, 30) );
  
  // cut out eyes from source image using coordinates from detectMultiScale
  Mat eyeL, eyeR;
  eyeL = Mat(src, eyes[0]);
  eyeR = Mat(src, eyes[1]);

  // get size of the cutout of the eyes
  Size sL = eyeL.size();
  int rowsL = (int)sL.height; 
  int colsL = (int)sL.width;

  Size sR = eyeR.size();
  int rowsR = (int)sR.height;
  int colsR = (int)sR.width;

  // dictate region of interest
  Rect roiL = Rect(0, rowsL*0.3, colsL, rowsL*0.5);
  Rect roiR = Rect(0, rowsR*0.3, colsR, rowsR*0.5);

  // cut out the eyes from the original image 
  Mat cut_eyeL = Mat(eyeL, roiL);
  // namedWindow("cuteyeL");
  // imshow("cuteyeL", cut_eyeL);
  Mat cut_eyeR = Mat(eyeR, roiR);

  // namedWindow("cuteyeR");
  // imshow("cuteyeR", cut_eyeR);
  // waitKey();

  // store and return the eyes
  cutout_eyes.push_back(cut_eyeL);
  cutout_eyes.push_back(cut_eyeR);

  return cutout_eyes;
}

void get_cam()
{
  VideoCapture capture; 
  int dev_id = 0;

  // open webcam
  capture.open(0);
  if (!capture.isOpened()) {
    cout << "Failed to open video device" << endl;
  }

  return;
}

Mat load_image(string path)
{
  Mat image, gray_image;
  image = imread(path, CV_LOAD_IMAGE_COLOR);
  cvtColor( image, gray_image, CV_BGR2GRAY );
  return gray_image;
}


int main() 
{
  // dictate test image to run 
  string test1 = "alex.jpg";
  string test2 = "dom2.jpg";
  // load the image
  Mat alex = load_image(test1);
  Mat dom = load_image(test2);

  Mat alext = apply_threshold(alex);
  Mat domt = apply_threshold(dom);
  // imshow("alex", alext);
  imshow("dom", domt);
  // imshow("test2", test);



  // get the cutout of the eyes 
  vector<Mat> cutout_eyes = detect_eyes(dom);
  // imshow("cutout", cutout_eyes[0]);
  Mat thresh = apply_threshold(cutout_eyes[0]);
  Mat thresh_inv = apply_threshold_inv(cutout_eyes[0]);
  // imshow("thresh", thresh);
  // apply_harris(thresh);
  // apply_harris(thresh_inv);

  apply_harris(cutout_eyes[0]);
  apply_harris(cutout_eyes[1]);
  waitKey();

  return 0;
}





