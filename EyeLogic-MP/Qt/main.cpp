#include "widget.h"
#include "../EyeLogic/EyeLogic.hpp"
#include "../EyeLogic/VoiceTool.hpp"

/**************************
 * NAMESPACE DECLARATIONS *
 **************************/
using namespace std;
using namespace std::chrono;

System * getSystem()
{
#ifdef __APPLE__
	return new Mac();
#else
	return new Win();
#endif
}

std::unique_ptr<System> systemSingleton(getSystem());
EyeLogic * mainEntryPoint;

/*********************
 * GLOBAL VARIABLES *
 *********************/

int imageCount = 0;               // which reference image calibration is currently on
bool CALIBRATED = false;            // global variable that determines if user ran calibration before running program
bool RUN = false;                   // flag to determine whether to track eyes or not
bool RunOnce = false;

//OpenCV Camera
VideoCapture cap;
cv::Mat capture;

// error message box size
QPoint msgBoxSize(500, 300);

// calibration options for tracking eyes + method for clicking
int trackEye = 0;
int voiceOption = 0;

// screen resolution
cv::Point screenres;

// user directory path
QString user_path;

// reference images
const std::string refImageNames [] = {"left", "right", "top", "bottom"};

// reference images path
QString ref_images_path;


/********************
 * GLOBAL FUNCTIONS *
 ********************/

// Converts QString to std::string
std::string toString(QString qs){
    return qs.toStdString();
//    return qs.toUtf8().constData();
}

// reset calibration parameters
void restartCalibration(){
    imageCount = 0;
    
    // remove directory
    QDir dir(user_path);
    dir.removeRecursively();
    
}


/*
 *  Calibration method that will start calibration process
 *  Tied to event listener and called when button is clicked
 */
void runCalibrate(){
    
    cv::Point distance;
    cv::Point referenceMean;
    std::vector<cv::Point> *data;
    
    bool frame_count = 0;
	bool found_reference = false;
	while (!found_reference)
	{
        frame_count++;
        
        // wasn't able to calibrate for a reference point in MAXFRAMES attempts
        if(frame_count == MAXFRAMES){
            restartCalibration();
            return;
        } else {
            cap >> capture;
            if(frame_count == 0){
                found_reference = mainEntryPoint->insertFrame(capture, true);
            } else {
                found_reference = mainEntryPoint->insertFrame(capture);

            }
        }
	}


	cv::Point refPoint = mainEntryPoint->getEyeVector();
	switch (imageCount) {
	case 0:
		mainEntryPoint->setReferencePoint(refPoint, RefPoint::LEFT);
		break;
	case 1:
		mainEntryPoint->setReferencePoint(refPoint, RefPoint::RIGHT);
		break;
	case 2:	
		mainEntryPoint->setReferencePoint(refPoint, RefPoint::TOP);
		break;
	case 3:
		mainEntryPoint->setReferencePoint(refPoint, RefPoint::BOTTOM);
		break;
	}
    

    // If calibration is on last image, store to file
    
    // write all reference points out to file
    // write faceCrop, leftEyeCrop, rightEyeCrop
    // store faceStrip in file
    if(imageCount == REFIMAGES - 1){
        
        std::ofstream outfile(toString(user_path) + "/parameters.txt", std::ios::app);

        data = mainEntryPoint->getReferencePointData();
    
        for(auto ref : *data){
            outfile << ref.x << " " << ref.y << endl;
            outfile << endl;
        }
        
        if (distance.x == 0 || distance.y == 0) {
            return;
        }
        
        cv::Rect faceCrop;
        cv::Rect leftEyeCrop;
        cv::Rect rightEyeCrop;
        
        
        cv::Mat faceStrip =  mainEntryPoint->getTemplate(&faceCrop, &leftEyeCrop, &rightEyeCrop);
        
        if(faceStrip.empty()){
            restartCalibration();
        }
        
        // faceCrop
        outfile << faceCrop.x << " " << faceCrop.y;
        outfile << faceCrop.width << std::endl;
        outfile << faceCrop.height << std::endl;
        outfile << std::endl;
        
        // leftEyeCrop
        outfile << leftEyeCrop.x << " " << leftEyeCrop.y;
        outfile << leftEyeCrop.width << std::endl;
        outfile << leftEyeCrop.height << std::endl;
        outfile << std::endl;
        
        // rightEyeCrop
        outfile << rightEyeCrop.x << " " << rightEyeCrop.y;
        outfile << rightEyeCrop.width << std::endl;
        outfile << rightEyeCrop.height << std::endl;
        outfile << std::endl;
        
        imwrite(toString(user_path) + "/template.png", faceStrip);

        
        CALIBRATED = true;
    }


} 

/*
 * Starts global camera
 */
bool startCam(){
    if (!cap.open(0)){
        return false;
    }
    try{
        
        // defaults to max size of camera
        cap.set(CV_CAP_PROP_FRAME_WIDTH, 1280);
        cap.set(CV_CAP_PROP_FRAME_HEIGHT, 720);
    }
    catch(Exception ex){
        return false;
    }
    systemSingleton->sleep(2000);
    return true;
}


/*
 * Runs main program
 */
void runMain(){

    // read in eye vectors from parameters.txt
    std::string filePath = toString(user_path);
    qDebug() << user_path << endl;
    filePath.append("/parameters.txt");
    
    std::ifstream inputfile(filePath, std::ios::in);

    
    if(!CALIBRATED){
    
        std::string line;
        std::string x, y;
        int width, height;
        
        vector <cv::Point> data;
        for(int i = 0; i < REFIMAGES; i++){
            getline(inputfile, line);
            std::stringstream iss(line);
            iss >> x >> y;
            cv::Point *pupilAvg = new cv::Point(std::stof(x), std::stof(y));

			data.push_back(*pupilAvg);
        }
        
		mainEntryPoint->setReferencePointData(&data);

        // read in image
        cv::Mat image = imread(toString(user_path) + "/template.png");
        
        cv::Rect faceCrop;
        cv::Rect leftEyeBounds;
        cv::Rect rightEyeBounds;
        
        // faceCrop
        getline(inputfile, line);
        std::stringstream iss(line);
        iss >> x >> y;
        
        getline(inputfile, line);
        width = std::stoi(line);
        
        getline(inputfile, line);
        height = std::stoi(line);
        
        faceCrop = cv::Rect(std::stoi(x), std::stoi(y), width, height);
        
        
        // leftEyeBounds
        getline(inputfile, line);
        iss.clear();
        iss.str(line);
        iss >> x >> y;
        
        getline(inputfile, line);
        width = std::stoi(line);
        
        getline(inputfile, line);
        height = std::stoi(line);
        
        leftEyeBounds = cv::Rect(std::stoi(x), std::stoi(y), width, height);
        
        
        // rightEyeBounds
        getline(inputfile, line);
        iss.clear();
        iss.str(line);
        iss >> x >> y;
        
        getline(inputfile, line);
        width = std::stoi(line);
        
        getline(inputfile, line);
        height = std::stoi(line);
        
        rightEyeBounds = cv::Rect(std::stoi(x), std::stoi(y), width, height);
                
        mainEntryPoint->storeTemplate(image, faceCrop, leftEyeBounds, rightEyeBounds);
    
                                      
    }

    while(RUN)
    {
        cap >> capture;
        if (mainEntryPoint->insertFrame(capture))
            
        {
            systemSingleton->setCurPos(mainEntryPoint->eyeVectorToScreenCoord());
        }
    }

	VoiceTool::voiceSingleton().disableVoice();
}


/*
 * Creates all references images needed for calibration
 * Output: folder containing all reference image jpgs
 */
void generateRefImages(){
    int horizontal = screenres.x;
    int vertical = screenres.y;
    string image_path;

    Mat cue(vertical, horizontal, CV_8UC3);
    Mat flash(vertical, horizontal, CV_8UC3);
    
    // make directory
    QDir().mkdir(ref_images_path);

    // top before
    cue = Scalar(0, 0, 0);
    cv::circle(cue, cv::Point(horizontal/2, 0), 30, Scalar(0, 0, 255), -1);
    image_path = toString(QDir::currentPath()) + "/ref_images/topBefore.jpg";
    cv::imwrite(image_path, cue);

    // top after
    cue = Scalar(0, 0, 0);
    cv::circle(cue, cv::Point(horizontal/2, 0), 30, Scalar(0, 255, 0), -1);
    image_path = toString(QDir::currentPath()) + "/ref_images/topAfter.jpg";
    cv::imwrite(image_path, cue);

    // left before
    cue = Scalar(0, 0, 0);
    cv::circle(cue, cv::Point(0, vertical/2), 30, Scalar(0, 0, 255), -1);
    image_path = toString(QDir::currentPath()) + "/ref_images/leftBefore.jpg";
    cv::imwrite(image_path, cue);
    
    // left after
    cue = Scalar(0, 0, 0);
    cv::circle(cue, cv::Point(0, vertical/2), 30, Scalar(0, 255, 0), -1);
    image_path = toString(QDir::currentPath()) + "/ref_images/leftAfter.jpg";
    cv::imwrite(image_path, cue);

    // bottom before
    cue = Scalar(0, 0, 0);
    cv::circle(cue, cv::Point(horizontal/2, vertical), 30, Scalar(0,0, 255), -1);
    image_path = toString(QDir::currentPath()) + "/ref_images/bottomBefore.jpg";
    cv::imwrite(image_path, cue);

    // bottom after
    cue = Scalar(0, 0, 0);
    cv::circle(cue, cv::Point(horizontal/2, vertical), 30, Scalar(0, 255, 0), -1);
    image_path = toString(QDir::currentPath()) + "/ref_images/bottomAfter.jpg";
    cv::imwrite(image_path, cue);

    // right before
    cue = Scalar(0, 0, 0);
    cv::circle(cue, cv::Point(horizontal, vertical/2), 30, Scalar(0, 0, 255), -1);
    image_path = toString(QDir::currentPath()) + "/ref_images/rightBefore.jpg";
    cv::imwrite(image_path, cue);

    // right after
    cue = Scalar(0, 0, 0);
    cv::circle(cue, cv::Point(horizontal, vertical/2), 30, Scalar(0, 255, 0), -1);
    image_path = toString(QDir::currentPath()) + "/ref_images/rightAfter.jpg";
    cv::imwrite(image_path, cue);

}

/****************
 * MAIN PROGRAM *
 ****************/
int main(int argc, char *argv[])
{
	// acquire camera
	if (!startCam()) return -1;

	// initialize voice
//	if (!VoiceTool::voiceSingleton().initVoice()) cerr << "Voice could not be started" << endl;

    screenres = systemSingleton->getScreenResolution();
	mainEntryPoint = new EyeLogic(screenres);

	// ref images path
    ref_images_path = QDir::currentPath() + "/ref_images/";
    
    // Create reference images if folder does not exist
    if (!QDir(ref_images_path).exists()){
        generateRefImages();
    }

    // Start application
    QApplication app(argc, argv);
    Widget w;
    w.setWindowTitle("Welcome to Eyelogic Setup");
    w.show();
    app.exec();
    
    
    
	//Clean Up
	VoiceTool::voiceSingleton().stopVoice();
	cap.release();
	return 0;
}
