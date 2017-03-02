#include "EyeLogic.hpp"
#include "User.hpp"
#include "EyePair.hpp"
#include <iostream>
#include <fstream>
#include <boost/filesystem.hpp>


int numRefs = 6;
Point screenres(1920, 1080);
std::string filenames [] {"camera.jpg", "topleft.jpg", "bottomleft.jpg", "center.jpg", "topright.jpg", "bottomright.jpb"};

namespace fs = boost::filesystem;
using namespace std;

int main(int argc, char *argv[])
{
    
    vector<const Mat *>reference_images;
    vector<const EyePair *>reference_vectors;
    
    /**************
     * Calibration *
     ***************/
    
    /* Save user information */
    std::string user_name = "dimitri";
    fs::path curr_path(fs::current_path());
    std::string imagedir = curr_path.string() + "/" + user_name + "_images/";
    std::string filepath = imagedir + "parameters.txt";
    std::ofstream outfile(filepath);
    cout << imagedir << endl;
    cout << filepath << endl;
    
    /***************
     * Calibration *
     ***************/
//    if(!fs::is_directory(imagedir)){
//        
//        fs::create_directory(imagedir);
//        
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
//            outfile << image_path << std::endl;
//            outfile << pair.leftVector.x << " " << pair.leftVector.y << std::endl;
//            outfile << pair.rightVector.x << " " << pair.rightVector.y << std::endl;
//        }
//        
//        outfile.close();
//        
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
