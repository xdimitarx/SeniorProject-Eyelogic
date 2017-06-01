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
    return qs.toUtf8().constData();
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
    
    std::ofstream outfile(toString(user_path) + "/parameters.txt", std::ios::app);

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
            found_reference = mainEntryPoint->insertFrame(capture);
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
        cap.set(CV_CAP_PROP_FRAME_WIDTH, 10000);
        cap.set(CV_CAP_PROP_FRAME_HEIGHT, 10000);
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
    std::ifstream inputfile(toString(user_path) + "/parameters.txt", std::ios::in);

    
    if(!CALIBRATED){
    
		vector <cv::Point> data;
        for(int i = 0; i < REFIMAGES; i++){

            std::string line;
            getline(inputfile, line);

            std::string x, y;
            std::stringstream iss;
            iss.str(line);
            iss >> x >> y;
            cv::Point *pupilAvg = new cv::Point(std::stof(x), std::stof(y));

			data.push_back(*pupilAvg);
        }
		mainEntryPoint->setReferencePointData(&data);
		// mainEntryPoint->storeTemplate(imageStrip, faceRect, leftEyeBound, rightEyeBound); will need to pass in
    }

	//ErrorLimits
	int errorCount = 0;

    while(RUN)
    {
		if (errorCount > 200)
		{
			RUN = false;
			//Display Error
		}
		else if (errorCount > 100)
		{
			cap >> capture;
			if (mainEntryPoint->insertFrame(capture, true))
			{
				systemSingleton->setCurPos(mainEntryPoint->eyeVectorToScreenCoord());
				errorCount = 0;
			}
			else
				errorCount++;

		}
		else
		{
			cap >> capture;
			if (mainEntryPoint->insertFrame(capture, true))
			{
				systemSingleton->setCurPos(mainEntryPoint->eyeVectorToScreenCoord());
				errorCount = 0;
			}
			else
				errorCount++;
		}
    }

	//VoiceTool::voiceSingleton().disableVoice();
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

	// voice only works on windows
	//if (!VoiceTool::voiceSingleton().initVoice()) cerr << "Voice could not be started" << endl;
	//VoiceTool::voiceSingleton().enableVoice();

    screenres = systemSingleton->getScreenResolution();
	mainEntryPoint = new EyeLogic(screenres);

	//POUNEH THIS IS HOW I TEST vvv
	//capture = loadImageAtPath("dom.jpg");
	//mainEntryPoint->insertFrame(capture);
	//mainEntryPoint->insertFrame(capture);

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
	//VoiceTool::voiceSingleton().stopVoice();
	cap.release();
	return 0;
}
