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
int REFIMAGES = 4;                      // number of reference points
//int FRAMES = 40;                        // number of ref frames per ref point
//int THRESHOLD = 10;                     // max deviation ref frames inside buffer
//int MAXFRAMES = 100;                    // max number of tries to find 40 valid frames
int imageCount = 0;                     // which reference image calibration is currently on

// global variable that determines if user ran calibration before running program
bool CALIBRATED = false;

// flag to determine whether to track eyes or not
bool RUN = false;

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

// bounds for eyes
cv::Rect_<int>rightEyeBounds;
cv::Rect_<int>leftEyeBounds;
std::vector<cv::Rect_<int>> eyes;


/********************
 * GLOBAL FUNCTIONS *
 ********************/

/*
 * Converts QString to std::string
 */
std::string toString(QString qs){
    return qs.toUtf8().constData();
}

void disableVoice(){
    VoiceTool::voiceSingleton().disableVoice();
}

void enableVoice(){
	VoiceTool::voiceSingleton().enableVoice();
}

void stopVoice(){
	VoiceTool::voiceSingleton().stopVoice();
}




/*
 *  Calibration method that will start calibration process
 *  Tied to event listener and called when button is clicked
 */
void runCalibrate(){
    
    cv::Point distance;
    cv::Point referenceMean;
    
    std::ofstream outfile(toString(user_path) + "/parameters.txt", std::ios::app);

	bool found_reference = false;
	while (!found_reference)
	{
		cap >> capture;
		found_reference = mainEntryPoint->insertFrame(capture);
	}

	// wasn't able to calibrate for a reference point
	if(!found_reference){
		imageCount = 0;

		// remove directory
		QDir dir(user_path);
		dir.removeRecursively();

		return;
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
    
    /*
    // if calibration is on last image
    if(imageCount == REFIMAGES - 1){
        
        for(auto ref : refArray){
            outfile << ref->x << " " << ref->y << std::endl;
            outfile << std::endl;
        }
        
        distance.x = ref_left.x - ref_right.x;
        distance.y = ref_bottom.y - ref_right.y;
        
        
        if (distance.x == 0 || distance.y == 0) {
            return;
        }
        
        //reference mean might be wholly unecessary
        referenceMean = cv::Point((ref_left.x + ref_right.x) / 2,
                                  (ref_top.y + ref_bottom.y) / 2);
        
        //updateBoundaryWindows();
        
        outfile << rightEyeBounds.x << " " << rightEyeBounds.y;
        outfile << rightEyeBounds.width << " " << rightEyeBounds.height;
        
        outfile << std::endl;
        
        outfile << leftEyeBounds.x << " " << leftEyeBounds.y;
        outfile << leftEyeBounds.width << " " << leftEyeBounds.height;
        
        CALIBRATED = true;
    }
	*/

} 

/*
 * Starts global camera
 */
bool startCam(){
    if (!cap.open(0)){
        return false;
    }
    try{
        cap.set(CV_CAP_PROP_FRAME_WIDTH, 10000);         //setting to arbitrarily large size will get max available resolution
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
    
        for(int i = 0; i < REFIMAGES; i++){

            std::string line;
            getline(inputfile, line);

            std::string x, y;
            std::stringstream iss;
            iss.str(line);
            iss >> x >> y;
            cv::Point *pupilAvg = new cv::Point(std::stof(x), std::stof(y));


            //refArray[i] = pupilAvg;

        }
    }

    if (!startCam())
	{
		//camera failed to start
		cout << "Failed to acquire camera." << endl;
		return;
	}
    
    while(RUN){
		cap >> capture;
		if (mainEntryPoint->insertFrame(capture))
		{
			systemSingleton->setCurPos(mainEntryPoint->eyeVectorToScreenCoord());
		}
    }
	cap.release();

	VoiceTool::voiceSingleton().stopVoice();

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
	mainEntryPoint = new EyeLogic(systemSingleton->getScreenResolution());

    // start camera
	startCam();

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
    return app.exec();
}
