#include "EyeLogic.hpp"
#include "User.hpp"
#include "EyePair.hpp"
#include <string>
#include <iostream>
#include <fstream>
#include <boost/filesystem.hpp>

namespace fs = boost::filesystem;
using namespace std;

// global variables
Mat ref_topLeft, ref_bottomLeft, ref_center, ref_topRight, ref_bottomRight;
Mat *refArray [] {new Mat(ref_topLeft), new Mat(ref_bottomLeft), new Mat(ref_center), new Mat(ref_topRight), new Mat(ref_bottomRight)};
int numRefs = 6;
Point screenres(1920, 1080);
std::string filenames [] {"camera.jpg", "topleft.jpg", "bottomleft.jpg", "center.jpg", "topright.jpg", "bottomright.jpg"};



int main(int argc, char *argv[])
{
    
    vector<const Mat *>reference_images;
    vector<const EyePair *>reference_vectors;
    
    /* Save user information */
    std::string user_name = "dimitri";
    fs::path curr_path(fs::current_path());
    std::string imagedir = curr_path.string() + "/images/";
    
    
    /***************
     * Calibration *
     ***************/
    
    // create folder and store reference images
//    if(!fs::exists(imagedir)){
//        fs::create_directory(imagedir);
//        cout << "directory created " << endl;
//        
//        std::ofstream outfile(imagedir + "parameters.txt", std::ios::out);
//
//        for(int i = 0; i < numRefs; i++){
//            // take image
//            Mat camera = cameraCapture();
//            cout << "next" << endl;
//            std::string image_path = imagedir + filenames[i];
//            imwrite(image_path, camera);
//            ImgFrame camera_frame(screenres);
//            
//            // calculate eye Vector
//            camera_frame.insertFrame(camera);
//            EyePair pair(camera_frame.getLeftEye().getEyeVector(), camera_frame.getRightEye().getEyeVector());
//            
//            // store in file
//            outfile << pair.leftVector.x << " " << pair.leftVector.y << std::endl;
//            outfile << pair.rightVector.x << " " << pair.rightVector.y << std::endl;
//            outfile << std::endl;
//        }
//        
//    }
//    // if folder already exists, just read in images
//    else {
//        std::ifstream inputfile(imagedir + "parameters.txt", std::ios::out);
//        
//        for(int i = 0; i < numRefs; i++){
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
//            Eye leftEye = Eye();
//            leftEye.leftEye = true;
//            leftEye.setEyeVector(std::stof(x), std::stof(y));
//            
//            iss.clear();
//            getline(inputfile, line);
//            iss >> x >> y;
//            Eye rightEye = Eye();
//            rightEye.leftEye = false;
//            rightEye.setEyeVector(std::stof(x), std::stof(y));
//            
//        }
//    }
    
    
    
    /****************
     * Main Program *
     ****************/
    
    ImgFrame mainEntryPoint(Point(1920,1080));
    
    if(argc == 2)
    {
        mainEntryPoint.insertFrame(loadImageAtPath(argv[1]));
    }
    else
    {
        mainEntryPoint.insertFrame(cameraCapture());
    }
    
    cout << "finito" << endl;
    return 0;
}
