#include "EyeLogic.hpp"
#include <string>
#include <iostream>
#include <fstream>
#include <boost/filesystem.hpp>
#include <chrono>
#include <map>

namespace fs = boost::filesystem;
using namespace std;
using namespace std::chrono;

/***********
 * GLOBALS *
 ***********/
int NUMREFS = 6;                                                                            // number of reference points
int FRAMES = 40;                                                                            // number of ref frames per ref point
int THRESHOLD = 10;                                                                          // max deviation ref frames inside buffer

cv::Point screenres(1920, 1080);                                                            // screen resolution
Mat ref_camera, ref_topLeft, ref_bottomLeft, ref_center, ref_topRight, ref_bottomRight;     // reference images
Mat *refArray [] {&ref_camera, &ref_topLeft, &ref_bottomLeft, &ref_center, &ref_topRight, &ref_bottomRight};
std::string filenames [] {"camera.jpg", "topleft.jpg", "bottomleft.jpg", "center.jpg", "topright.jpg", "bottomright.jpg"};
std::map<Mat *, EyePair> RefImageVector;                                                    // Map to retrive EyePair based on the image.
fs::path curr_path(fs::current_path());
std::string imagedir = curr_path.string() + "/images/";                                     // image file path

/* change the value of singleton to corresponding OS */
System *singleton = new Mac();


/*
 *  checks if half of values in RefVectors are within a certain threshold (currently set to 10) of each other
 *  performs check first on x values and then y values and returns the average of those values if it finds and x and y
 *
 *  Input: vector of Points
 *
 *  Output: pointer to average coordinates or nullptr
 */
cv::Point *getStabalizedCoord(std::vector<cv::Point>RefVectors){
    
    int buffer = floor(FRAMES/2) + 1;
    
    // sort by x coordinate
    std::sort(RefVectors.begin(), RefVectors.end(), [](const cv::Point p1, const cv::Point p2){return p1.x <= p2.x;});
    
    for(int i = 0; i < FRAMES - buffer; i++){
        vector<cv::Point>tmp;
        
        // vector of size buffer
        for(int j = 0; j < buffer; j++){
            tmp.push_back(RefVectors[i+j]);
        }
        cv::Point Xmin = *std::min_element(tmp.begin(), tmp.end(), [](const cv::Point p1, const cv::Point p2){return p1.x <= p2.x;});
        cv::Point Xmax = *std::max_element(tmp.begin(), tmp.end(), [](const cv::Point p1, const cv::Point p2){return p1.x >= p2.x;});

        if(Xmax.x - Xmin.x <= THRESHOLD){
            
            cv::Point Ymin = *std::min_element(tmp.begin(), tmp.end(), [](const cv::Point p1, const cv::Point p2){return p1.y <= p2.y;});
            cv::Point Ymax = *std::max_element(tmp.begin(), tmp.end(), [](const cv::Point p1, const cv::Point p2){return p1.y >= p2.y;});
            
            if(Ymax.y - Ymin.y <= THRESHOLD){
                float sumX = 0, sumY = 0;
                std::for_each(tmp.begin(), tmp.end(), [sumX, sumY](const cv::Point pt) mutable {sumX += pt.x; sumY += pt.y;});
                sumX /= tmp.size();
                sumY /= tmp.size();
                return new cv::Point(sumX, sumY);
            }
            
        }
    }
    
    return nullptr;
}



/*
 *  Takes 40 images and calculates the eyeVector for each image frame. Keeps looping until 40 valid frames
 *  with left and right vectors are found.
 *
 *  Output: EyePair with left and right eye vectors for the associated reference image
 */
EyePair *getRefVector(){
    VideoCapture cap;
    
    if (!cap.open(0)){
        cout << "camera is not available" << endl;
        return nullptr;
    }
    std::vector<Mat>images;
    std::vector<cv::Point>leftVectors;
    std::vector<cv::Point>rightVectors;
    
    // grab 40 images and store in images vector
    for(int j = 0; j < FRAMES; j++){
        //take image
        Mat capture;
        cap >> capture;
        images.push_back(capture);

        // calculate eyeVector
        ImgFrame camera_frame(screenres);
        camera_frame.insertFrame(images.at(j));
        EyePair pair(camera_frame.getLeftEye().getEyeVector(), camera_frame.getRightEye().getEyeVector());
        
        // dropped frame if can't calculate both left and right eye vectors
        if(pair.leftVector.x < 0 || pair.rightVector.x < 0){
            j--;
            continue;
        }
        
        // store in array
        leftVectors.push_back(cv::Point(pair.leftVector.x, pair.leftVector.y));
        rightVectors.push_back(cv::Point(pair.rightVector.x, pair.rightVector.y));
    
    }
    
    assert(leftVectors.size() == FRAMES && rightVectors.size() == FRAMES);
    
    cv::Point *left = getStabalizedCoord(leftVectors);
    cv::Point *right = getStabalizedCoord(rightVectors);

    
    // keep taking images until a stabalized set of coordinates can be found
    if(!left || !right){
        getRefVector();
    }
    
    return new EyePair(*left, *right);

}


/*
 *  Calibration method that will start calibration process
 *  Tied to event listener and called when button is clicked
 */
void calibrate(){
    
    fs::create_directory(imagedir);
    
    std::ofstream outfile(imagedir + "parameters.txt", std::ios::out);
    
    for(int i = 0; i < NUMREFS; i++){
        
        std::string image_path = imagedir + filenames[i];
        
        EyePair *refPair = getRefVector();
        
        RefImageVector.insert(std::pair<Mat *, EyePair>(refArray[i], *refPair));
        
        // store in file
        outfile << refPair->leftVector.x << " " << refPair->leftVector.y << std::endl;
        outfile << refPair->rightVector.x << " " << refPair->rightVector.y << std::endl;
        outfile << std::endl;
    }
    
    
}

int main(int argc, char *argv[])
{
    vector<const Mat *>reference_images;
    vector<const EyePair *>reference_vectors;
    /************************
     * For Testing Purposes *
     ************************/
    // delete directory if it already exists
//    if(fs::exists(imagedir)){
//        fs::remove_all(imagedir);
//    }
//    calibrate();
    
    
    /***************
     * Calibration *
     ***************/
    
//    // create folder and store reference images
//    if(!fs::exists(imagedir)){
//        calibrate();
//    }
//    // if folder already exists, just read in images
//    else {
//        std::ifstream inputfile(imagedir + "parameters.txt", std::ios::out);
//        
//        for(int i = 0; i < NUMREFS; i++){
//            Mat image = imread(imagedir + filenames[i]);
//            *refArray[i] = image;
//            
//            std::string line;
//            getline(inputfile, line);
//
//            std::string x, y;
//            std::stringstream iss;
//            iss.str(line);
//            iss >> x >> y;
//            cv::Point leftEye(std::stof(x), std::stof(y));
//            
//            iss.clear();
//            getline(inputfile, line);
//            iss >> x >> y;
//            cv::Point rightEye(std::stof(x), std::stof(y));
//            
//            EyePair refPair(leftEye, rightEye);
//            
//            RefImageVector.insert(std::pair<Mat *, EyePair>(refArray[i], refPair));
//            
//        }
//    }
//    

    
    
    /****************
     * Main Program *
     ****************/

    ImgFrame mainEntryPoint(screenres);

    if (argc == 2)
    {
        mainEntryPoint.insertFrame(loadImageAtPath(argv[1]));
    }
    else
    {
        size_t i = 0;
        high_resolution_clock::time_point start, end;
        VideoCapture cap;
        Mat capture;
        
        if (!cap.open(0))
            return 0;
        
        
        while (1) {
            //Code to calculate time it takes to do insertFrame operation
            //As of 3/22/2017, it takes approximately 1 whole second to get and process a frame
            //As of 3/26/2017, it takes approximately .08 seconds to get and process a frame
            
            start = high_resolution_clock::now();
            sleep(5);
            cap >> capture;
            end = high_resolution_clock::now();
            auto duration = duration_cast<microseconds>(end - start).count();
            cout << "Camera time: " << duration << endl;
            
            start = high_resolution_clock::now();
            mainEntryPoint.insertFrame(capture);
            end = high_resolution_clock::now();
            duration = duration_cast<microseconds>(end - start).count();
            cout << duration << endl;
            
            
            if (waitKey(30) == '9') { break; }
            cin.get();
        }
        
        cap.release();
    }
    

    
    cout << "finito" << endl;
    return 0;

}
