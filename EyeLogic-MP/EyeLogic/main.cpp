#include "EyeLogic.hpp"

System * getSystem()
{
#ifdef __APPLE__
    return new Mac();
#else
    return new Win();
#endif
    return NULL;
}

namespace fs = boost::filesystem;
using namespace std;
using namespace std::chrono;

/***********
 * GLOBALS *
 ***********/
int NUMREFS = 6;                                             // number of reference points
int FRAMES = 40;                                             // number of ref frames per ref point
int THRESHOLD = 10;                                          // max deviation ref frames inside buffer
int MAXFRAMES = 100;

cv::Point screenres(1920, 1080);                                                            // screen resolution
Mat ref_camera, ref_topLeft, ref_bottomLeft, ref_center, ref_topRight, ref_bottomRight;     // reference images
Mat *refArray [] {&ref_camera, &ref_topLeft, &ref_bottomLeft, &ref_center, &ref_topRight, &ref_bottomRight};
std::string filenames [] {"camera.jpg", "topleft.jpg", "bottomleft.jpg", "center.jpg", "topright.jpg", "bottomright.jpg"};
std::map<Mat *, EyePair> RefImageVector;                     // Map to retrive EyePair based on the image.
fs::path curr_path(fs::current_path());
std::string imagedir = curr_path.string() + "/images/";      // image file path

std::unique_ptr<System> singleton (getSystem());             // global singleton variable used for different OS calls


enum Coordinate{
    X,
    Y
};


/*
 *  Find the maximum of a set of points
 *
 *  Input: vector of Points, X or Y coordinate
 *
 *  Output: min
 */
cv::Point Max(std::vector<cv::Point>data, Coordinate a){
    cv::Point max;
    
    // x coordinate
    if(a == X){
        max = data[0];
        for(auto pt: data){
            if(pt.x > max.x){
                max = pt;
            }
        }
        
    }
    
    // y coordinate
    if (a == Y){
        max = data[0];
        for(auto pt: data){
            if(pt.y > max.y){
                max = pt;
            }
        }
    }
    
    return max;
    
}

/*
 *  Find the minimum of a set of points
 *
 *  Input: vector of Points, X or Y coordinate
 *
 *  Output: min
 */
cv::Point Min(std::vector<cv::Point>data, Coordinate a){
    
    cv::Point min;
    // x coordinate
    if(a == X){
        min = data[0];
        for(auto pt: data){
            if(pt.x < min.x){
                min = pt;
            }
        }
        
    }
    
    // y coordinate
    if (a == Y){
        min = data[0];
        for(auto pt: data){
            if(pt.y < min.y){
                min = pt;
            }
        }
    }
    
    return min;
    
}

/*
 *  checks if half of values in RefVectors are within a certain threshold (currently set to 10) of each other
 *  performs check first on x values and then y values and returns the average of those values if it finds and x and y
 *
 *  Input: vector of Points
 *
 *  Output: pointer to average coordinates or nullptr
 */
cv::Point *getStabalizedCoord(std::vector<cv::Point>RefVectors){
    
    if(RefVectors.empty()){
        return nullptr;
    }
    
    int buffer = floor(FRAMES/2) + 1;
    
    // sort by x coordinate
    std::sort(RefVectors.begin(), RefVectors.end(),
              [](const cv::Point p1, const cv::Point p2){return (p1.x!=p2.x)?(p1.x < p2.x):(p1.y < p2.y);});
    
    for(int i = 0; i < FRAMES - buffer; i++){
        vector<cv::Point>tmp;
        
        // vector of size buffer
        for(int j = 0; j < buffer; j++){
            tmp.push_back(RefVectors[i+j]);
        }
        
        
        cv::Point Xmin = tmp[0];;
        cv::Point Xmax = tmp[tmp.size()-1];
        
        
        
        //        std::vector<cv::Point>::iterator Xmin_itr = std::min_element(tmp.begin(), tmp.end(), []( cv::Point p1,  cv::Point p2){return p1.x < p2.x;});
        //        std::vector<cv::Point>::iterator Xmax_itr = std::max_element(tmp.begin(), tmp.end(), []( cv::Point p1,  cv::Point p2){return p1.x < p2.x;});
        //
        //        cv::Point Xmin = *Xmin_itr;
        //        cv::Point Xmax = *Xmax_itr;
        
        if(Xmax.x - Xmin.x <= THRESHOLD){
            
            //            cv::Point Ymin = *std::min_element(tmp.begin(), tmp.end(), []( cv::Point p1,  cv::Point p2){return p1.y < p2.y;});
            //            cv::Point Ymax = *std::max_element(tmp.begin(), tmp.end(), []( cv::Point p1,  cv::Point p2){return p1.y > p2.y;});
            
            cv::Point Ymax = Max(tmp, Y);
            cv::Point Ymin = Min(tmp, Y);
            if(Ymax.y - Ymin.y <= THRESHOLD){
                float sumX = 0, sumY = 0;
                std::for_each(tmp.begin(), tmp.end(), [&sumX, &sumY](const cv::Point pt) {sumX += pt.x; sumY += pt.y;});
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
 *  with left and right vectors are found or until MAX_FRAMES has been reached
 *
 *  Output: EyePair with left and right eye vectors for the associated reference image or nullptr
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
    int count = 0;
    
    
    // grab 40 images and store in images vector
    for(int j = 0; j < FRAMES; j++){
        
        count++;
        
        // break if 80 images are taken and vectors for left and right can't be found
        if(count == MAXFRAMES){
            cout << "could not find " << FRAMES << " frames within a reasonable time frame" << endl;
            return nullptr;
        }
        
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
    
    // close camera
    cap.release();
    
    // 40 valid frames with eye vectors should be found at this point
    assert(leftVectors.size() == FRAMES && rightVectors.size() == FRAMES);
    
    //**************************** TEST DATA
    //    std::vector<cv::Point>data = {
    //                                    {22,50},{47,29},{56,75},{40,26},{47,28},{41,34},{15,17},{81,77},{45,26},{22,44},
    //                                    {44,32},{42,27},{75,62},{52,99},{32,33},{42,34},{40,34},{45,26},{25,30},{46,30},
    //                                    {41,33},{14,69},{47,32},{42,26},{50,50},{36,17},{42,29},{41,31},{62,29},{20,25},
    //                                    {40,30},{77,19},{28,28},{44,32},{41,35},{27,29},{46,27},{50,61},{91,2},{47,26}
    //                                };
    //
    //    cv::Point *oneEye = getStabalizedCoord(data);
    //    cv::Point *noEye = getStabalizedCoord({});
    //
    //    if(!noEye){
    //        cout << "null pointer" << endl;
    //    }
    //****************************
    
    // get reference vector for the left and right eye
    cv::Point *left = getStabalizedCoord(leftVectors);
    cv::Point *right = getStabalizedCoord(rightVectors);
    
    
    // keep taking new set of 40 images until left and right eye Vector can be found <-- TOO HARSH? MAYBE VALID FRAME IF LEFT "OR" RIGHT VECTOR
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
    
    // create image directory and
    fs::create_directory(imagedir);
    std::ofstream outfile(imagedir + "parameters.txt", std::ios::out);
    
    for(int i = 0; i < NUMREFS; i++){
        
        // create image folder
        std::string image_path = imagedir + filenames[i];
        
        // get eyeVector pair for that image
        EyePair *refPair = getRefVector();
        
        
        if(refPair){
            
            // insert eyePair and corresponding image into global map
            RefImageVector.insert(std::pair<Mat *, EyePair>(refArray[i], *refPair));
            
            // store in file
            outfile << refPair->leftVector.x << " " << refPair->leftVector.y << std::endl;
            outfile << refPair->rightVector.x << " " << refPair->rightVector.y << std::endl;
            outfile << std::endl;
            
        }
        else {
            cout << "could not calibrate. Please try again" << endl;
        }
    }
    
    
}


/*
 *  MAIN PROGRAM LOOP
 */
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
//    // if folder already exists, just read in eyeVectors
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
//    
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
