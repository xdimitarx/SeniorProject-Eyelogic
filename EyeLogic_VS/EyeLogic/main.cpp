#include <iostream>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/core/core.hpp>
#include <vector>


using namespace cv;
using namespace std;

/***** captures webcam footage and filters image hue *****/
//int main(){
//    
//    VideoCapture cap(0); // creates video capture object from webcam. Default webcam is 0
//    
//    if (cap.isOpened()){
//        cout << "Webcam is not open" << endl;
//    }
//    
//    while(true){
//        Mat image; // raw webcam footage
//        Mat HSVimage; // convert RGB image into Huge Saturation Value
//        Mat processedImage;
//        
//        cap.read(image); // assigns mat image to raw webcam footage
//        cvtColor(image, HSVimage, CV_BGR2HSV); // convert mac image to hsv under mat HSVimage
//        inRange(HSVimage, Scalar(0,0,0), Scalar(100,255,255), processedImage);  // processes HSVimage and gets only pixels in scalar range to processedImage ***** Scalar(hue, saturation, value)
//        
//        imshow("Original", image);
//        imshow("Processed", processedImage);
//        
//        int numPixels = countNonZero(processedImage); // count all white pixels in processedImage
//        cout << numPixels << endl;
//    }
// 
//    
//}
/*****  End tutorial *****/
 
//****  Example Tutorial: capture's webcam footage and displays ****
int main(){
    
    VideoCapture cap(0); // creates video capture object from webcam. Default webcam is 0
    
    if (cap.isOpened()){
        cout << "Webcam is not open" << endl;
    }
    while(true){
        Mat webcam;             // empty matrix
        cap.read(webcam);      // read video capture into webcam matrix
        imshow("Webcam", webcam);
    }
}
//**** End tutorial ****
