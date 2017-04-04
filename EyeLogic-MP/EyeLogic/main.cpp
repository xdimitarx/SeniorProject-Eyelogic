

//Comment this to run any operating system other than windows


//Ours
#include "EyeLogic.hpp"

//BOOST
#include <boost/filesystem.hpp>


namespace fs = boost::filesystem;
using namespace std;
using namespace std::chrono;


// global variables
const int NUMREFS = 6, NUM_REF_IMAGES = 40, THRESHOLD = 5;
Mat ref_topLeft, ref_bottomLeft, ref_center, ref_topRight, ref_bottomRight;
Mat *refArray [] {&ref_topLeft, &ref_bottomLeft, &ref_center, &ref_topRight, &ref_bottomRight};
cv::Point screenres(1920, 1080);
std::string filenames [] {"camera.jpg", "topleft.jpg", "bottomleft.jpg", "center.jpg", "topright.jpg", "bottomright.jpg"};
std::map<Mat *, EyePair> RefImageVector;
System *singleton;

float getAverage(std::vector<float>data){
    float sum = 0;
    for(auto t : data){
        sum += t;
    }
    sum /= data.size();
    
    return sum;
}

// checks if half of values in RefVectors are within a certain threshold (currently set to 5) of each other
// performs check first on x values and then y values and returns the average of those values if it finds and x and y
cv::Point *getStabalizedCoord(cv::Point RefVectors []){
    bool foundCoord = false;
    float x, y;
    std::vector<float>x_values;
    std::vector<float>y_values;
    for(int i = 0; i < NUM_REF_IMAGES; i++){
        x_values.push_back(RefVectors[i].x);
        y_values.push_back(RefVectors[i].y);
    }
    
    // sort x and y values in increasing order
    std::sort(x_values.begin(), x_values.end(), [](float x1, float x2){ return x1 <= x2;});
    std::sort(y_values.begin(), y_values.end(), [](float y1, float y2){ return y1 <= y2;});
    
    
    // 20 values
    int buffer = round(NUM_REF_IMAGES/2);
    
    // find x value
    for(int i = 0; i < NUM_REF_IMAGES - buffer; i++){
        vector<float>tmp;
        for(int j = 0; j < buffer; j++){
            tmp.push_back(x_values[i+j]);
        }
        float min = *std::min_element(tmp.begin(), tmp.end());
        float max = *std::max_element(tmp.begin(), tmp.end());
        if(max - min <= THRESHOLD){
            x = getAverage(tmp);
            foundCoord = true;
            break;        
        }
    }
    
    // find y value
    for(int i = 0; i < NUM_REF_IMAGES - buffer; i++){
        vector<float>tmp;
        for(int j = 0; j < buffer; j++){
            tmp.push_back(y_values[i+j]);
        }
        float min = *std::min_element(tmp.begin(), tmp.end());
        float max = *std::max_element(tmp.begin(), tmp.end());
        if(max - min <= THRESHOLD){
            y = getAverage(tmp);
            foundCoord = true;
            break;
            
        }
    }

    if(x && y){
        cv::Point *stabalized = new cv::Point(x, y);
        return stabalized;
    } else {
        return nullptr;
    }

}

EyePair *getRefVector(){
    VideoCapture cap;
    
    if (!cap.open(0)){
        cout << "camera is not available" << endl;
        return nullptr;
    }
    
    cv::Point leftVectors [NUM_REF_IMAGES];
    cv::Point rightVectors [NUM_REF_IMAGES];
    
    for(int j = 0; j < NUM_REF_IMAGES; j++){
    
        //take image
        Mat capture;
        cap >> capture;
        
        // calculate eyeVector
        ImgFrame camera_frame(screenres);
        camera_frame.insertFrame(capture);
        EyePair pair(camera_frame.getLeftEye().getEyeVector(), camera_frame.getRightEye().getEyeVector());
        
        // store in array
        leftVectors[j] = cv::Point(pair.leftVector.x, pair.leftVector.y);
        rightVectors[j] = cv::Point(pair.rightVector.x, pair.rightVector.y);
    
    }
    
    
    cv::Point *left = getStabalizedCoord(leftVectors);
    cv::Point *right = getStabalizedCoord(rightVectors);
    
    // keep taking images until a stabalized set of coordinates can be found
    if(!left || !right){
        getRefVector();
    }
    
    return new EyePair(*left, *right);

}

int main(int argc, char *argv[])
{
    singleton = getSystem();
    
    vector<const Mat *>reference_images;
    vector<const EyePair *>reference_vectors;
    
    /* Save user information */
    fs::path curr_path(fs::current_path());
    std::string imagedir = curr_path.string() + "/images/";
    
    /***************
     * Calibration *
     ***************/
    
    // create folder and store reference images
    /*if(!fs::exists(imagedir)){
        fs::create_directory(imagedir);
        cout << "directory created " << endl;
        
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
    // if folder already exists, just read in images
    else {
        std::ifstream inputfile(imagedir + "parameters.txt", std::ios::out);
        
        for(int i = 0; i < NUMREFS; i++){
            Mat image = imread(imagedir + filenames[i]);
            *refArray[i] = image;
            
            std::string line;
            getline(inputfile, line);

            std::string x, y;
            std::stringstream iss;
            iss.str(line);
            iss >> x >> y;
            cv::Point leftEye(std::stof(x), std::stof(y));
            
            iss.clear();
            getline(inputfile, line);
            iss >> x >> y;
            cv::Point rightEye(std::stof(x), std::stof(y));
            
            EyePair refPair(leftEye, rightEye);
            
            RefImageVector.insert(std::pair<Mat *, EyePair>(refArray[i], refPair));
            
        }
    }*/
    
    /****************
     * Main Program *
     ****************/

    ImgFrame mainEntryPoint(screenres);

    
    VideoCapture cap;
    Mat capture;
    if (!cap.open(0))
        return 0;

    if (argc == 2)
    {
        mainEntryPoint.insertFrame(loadImageAtPath(argv[1]));
    }
    else
    {
        size_t i = 0;
        high_resolution_clock::time_point start, end;
        
        while (1) {
            //Code to calculate time it takes to do insertFrame operation
            //As of 3/22/2017, it takes approximately 1 whole second to get and process a frame
            //As of 3/26/2017, it takes approximately .08 seconds to get and process a frame
            
            start = high_resolution_clock::now();
            cap >> capture;
            end = high_resolution_clock::now();
            auto duration = duration_cast<microseconds>(end - start).count();
            cout << "Camera time: " << duration << endl;
            
            start = high_resolution_clock::now();
			imshow("cap", capture);
			waitKey(0);
            mainEntryPoint.insertFrame(capture);
            end = high_resolution_clock::now();
            duration = duration_cast<microseconds>(end - start).count();
            cout << duration << endl;
            
            
            if (waitKey(30) == '9') { break; }
            cin.get();
        }
    }
    
    
    
    cout << "finito" << endl;
    return 0;

}
