#include <iostream>
#include <string>
#include <sstream>

#include <opencv2/core.hpp>
#include <opencv2/highgui.hpp>
#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/objdetect/objdetect.hpp"


using namespace std;
using namespace cv;

CascadeClassifier face_cascade;
CascadeClassifier eye_cascade;

void detectfaces(Mat img) {
  Mat image = img;

  face_cascade.load("haarcascade_frontalface_default.xml");
  eye_cascade.load("haarcascade_eye.xml");
  vector<Rect> faces;
  face_cascade.detectMultiScale(image, faces, 1.1, 3, 0, Size(30, 30) );

  vector<Rect> eyes; 
  eye_cascade.detectMultiScale(image, eyes, 1.1, 3, 0, Size(30, 30) );
 

  for (size_t i = 0; i < faces.size(); i++) {
    Point center( faces[i].x + faces[i].width*0.5, faces[i].y + faces[i].height*0.5 );
    Point eye_center( eyes[i].x + eyes[i].width*0.5, eyes[i].y + eyes[i].height*0.5 );

    ellipse( image, center, Size( faces[i].width*0.5, faces[i].height*0.5), 0, 0, 360, Scalar( 255, 0, 255 ), 4, 8, 0 );
    ellipse( image, eye_center, Size( eyes[i].width*0.5, eyes[i].height*0.5), 0, 0, 360, Scalar( 0, 255, 255 ), 4, 8, 0 );

  }

  imshow("Detected Face", image);
  char key = (char)waitKey(5);

  if (key == ' ')
    return;

  return;
}




int main() 
{
  // declare variables 
  Mat frame;
  Mat image; 
  VideoCapture capture; 
  int dev_id = 0;

  // open webcam
  capture.open(0);
  if (!capture.isOpened()) {
    cout << "Failed to open video device" << endl;
    return -1;
  }


  // capture image and load to frame
  for (;;) {
    capture >> frame;
    if (frame.empty()) 
      continue;

    // copy frame to image
    // if frame not empty, open a "window" and show image
    frame.copyTo(image);
    detectfaces(image);
    // imshow( "window", image );
    // char key = (char)waitKey(5);

    // if (key == ' ')
    //   break;
    
  }
  return 0;
}






