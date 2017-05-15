#include "EyeLogic.hpp"


/*
 *  Reads in image from 
 */
Mat loadImageAtPath(string path){
    Mat result = imread(path, CV_LOAD_IMAGE_COLOR);
    return result;
}


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
        std::vector<cv::Point> tmp;
        
        // vector of size buffer
        for(int j = 0; j < buffer; j++){
            tmp.push_back(RefVectors[i+j]);
        }
        
        
        cv::Point Xmin = tmp[0];;
        cv::Point Xmax = tmp[tmp.size()-1];
        
        
        if(Xmax.x - Xmin.x <= THRESHOLD){
            
            
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


cv::Point findPupil(cv::Mat eyeCrop){
    std::vector<cv::Vec4i> hierarchy;
    std::vector<std::vector<cv::Point> > contours;
    findContours(eyeCrop, contours, hierarchy, CV_RETR_TREE, CV_CHAIN_APPROX_SIMPLE, cv::Point(0, 0));
    if (contours.size() )
    {
        double area = 0;
        int largest = 0;
        for (int i = 0; i < contours.size(); ++i)
        {
            double calculatedArea = contourArea(contours[i]);
            if (calculatedArea > area)
            {
                largest = i;
                area = calculatedArea;
            }
        }
        cv::Moments mo = moments(contours[largest], false);
        cv::Point eyeCenter = cv::Point((int)(mo.m10 / mo.m00), (int)(mo.m01 / mo.m00));
        
        cv::Rect bounding = boundingRect(contours[largest]);
        //Point eyeCenter = cv::Point(cvRound(bounding.x + bounding.width / 2), cvRound(bounding.y + bounding.height / 2));
        int eyeRadius = cvRound(bounding.height*1.05);
        
        return eyeCenter;
    }
    std::cerr << "findPupil: COULDN'T DETERMINE IRIS" << std::endl;
    return cv::Point(-1, -1);
}


void updateBoundaryWindows() {
    //determine which eye in the "eyes" vector is the left/right eye
    //Because the camera flips the image, the User's right eye is the leftmost in the image
    if (eyes[0].x < eyes[1].x) {
        rightEyeBounds = eyes[0];
        leftEyeBounds = eyes[1];
    }
    else {
        leftEyeBounds = eyes[0];
        rightEyeBounds = eyes[1];
    }
    
    //redefining rectangle search region for eyes
    //Determine x and y coordinate of each bounding box (make it bigger so that if eye moves, it can still be detected)
    //Boundary check so that rectangle stays within bounds of image
    rightEyeBounds.x = std::max(rightEyeBounds.x - (rightEyeBounds.width / 2), 0);
    rightEyeBounds.y = std::max(rightEyeBounds.y - (rightEyeBounds.height / 2), 0);
    leftEyeBounds.x = std::min(leftEyeBounds.x - (leftEyeBounds.width / 2), ref_top.getImage().cols);
    leftEyeBounds.y = std::max(leftEyeBounds.y - (leftEyeBounds.height / 2), 0);
    
    //Adjust size of bounding box
    rightEyeBounds.width = 2 * rightEyeBounds.width;
    leftEyeBounds.width = 2 * leftEyeBounds.width;
    if (leftEyeBounds.x + leftEyeBounds.width > ref_top.getImage().cols) {
        leftEyeBounds.width = ref_top.getImage().cols - leftEyeBounds.x;
    }
    
    //This bit assumes that the eyes wont hit boundaries
    //TODO: boundary checking for y coordinates/height
    rightEyeBounds.height = 2 * rightEyeBounds.height;
    leftEyeBounds.height = 2 * leftEyeBounds.height;
}


bool getReferenceImage() {
    
    //pupil vector to keep track of pupil location for each eye
    cv::Point pupil[2];
    //pre-processing Matrices
    cv::Mat eyeCrop, eyeCropGray;
    
    std::vector<cv::Point>data;
    
    //Loop until adequate data is collected for callibration
//    while (1) {
    
    // counts how many images have been taken for this reference point
    int count = 0;
    
    for(int i = 0; i < FRAMES; i++){
        
        // clears vector for eye detection
        eyes.clear();
        
        // calls relevant reference image function using function pointers
        // detect eyes in relevant region
        eyeDetector.detectMultiScale(refArray[imageCount]->getImage(), eyes);
        
        if (eyes.size() == 2) {
            
            // loop to operate stuff for each eye
            for (int i = 0; i < 2; i++) {
                
                count++;
                
                // break if 80 images are taken and vectors for left and right can't be found
                if(count == 2 * MAXFRAMES){
                    cout << "could not find " << FRAMES << " frames in " << MAXFRAMES << " tries." << endl;
                    return false;
                }
                
                // preprocessing for pupil detection
                eyeCrop = cv::Mat(refArray[imageCount]->getImage(), eyes[i]);//crop eye region
                cvtColor(eyeCrop, eyeCropGray, CV_BGR2GRAY);
                cv::equalizeHist(eyeCropGray, eyeCropGray);
                
                // pupil detection
                pupil[i] = findPupil(eyeCropGray);
                
                // making sure pupil is detected - if not, repeat process
                if (pupil[i].x == -1 && pupil[i].y == -1) {
                    std::cout << "Cannot find pupil" << std::endl;
                    i--;
                    continue;
                }
                
                // make pupil coordinates relative to whole capture image and not just eye bounding box
                pupil[i].x += eyes[i].x;
                pupil[i].y += eyes[i].y;
                
                
            }
            
            // calculate average pupil location for looking at the reference point
            cv::Point avg((pupil[0].x + pupil[1].x) / 2, (pupil[0].y + pupil[1].y) / 2);
            
            // get stabalized set of coordinates
            data.push_back(avg);
            
        }
    }
    
    cv::Point *ref_point = getStabalizedCoord(data);
    
    refArray[imageCount]->setPupilAvg(*ref_point);
    
    return true;
}


void ImgFrame::calculateAverageEyeMethod(){
    
    std::vector<cv::Rect_<int>> eyeLeft, eyeRight;
    std::vector<cv::Vec3f> circles;
    cv::Point averageLocal, screenMap , destinationOld(-1,-1), destinationNew, delta(0,0), direction(0,0);
    cv::Point pupil[2];
    cv::Point distance ;
    cv::Mat eyeCrop, eyeCropGray;
    
    int i = 0;
    //Forever loop to move cursor
    while (1) {
        std::cout << "Loop number:  " << i++ << std::endl;
        cap >> capture;
        //detect eyes in subboxes
        eyeLeft.clear();
        eyeRight.clear();
        eyeDetector.detectMultiScale(cv::Mat(capture, leftEyeBounds ), eyeLeft);
        eyeDetector.detectMultiScale(cv::Mat(capture, rightEyeBounds), eyeRight);
        if (eyeLeft.size() == 1 && eyeRight.size() == 1) {
            eyes.clear();
            eyeLeft[0].x += leftEyeBounds.x;
            eyeLeft[0].y += leftEyeBounds.y;
            eyeRight[0].x += rightEyeBounds.x;
            eyeRight[0].y += rightEyeBounds.y;
            eyes.push_back(eyeLeft[0]);
            eyes.push_back(eyeRight[0]);
            updateBoundaryWindows();
        }
        else {
            std::cout << "Failure of the eyes *sadnesssss* " << std::endl;
            //get new rectangles for things???
            //TODO: Need to figure out how to deal with this
            continue;
        }
        
        //Detect pupil for each image
        for (int i = 0; i < eyes.size(); i++) {
            //Preprocessing steps for pupil detection
            eyeCrop = cv::Mat(capture, eyes[i]);//crop eye region
            cvtColor(eyeCrop, eyeCropGray, CV_BGR2GRAY);
            equalizeHist(eyeCropGray, eyeCropGray);
            //find pupil
            pupil[i] = findPupil(eyeCropGray);
            //Toss frame if pupil isn't detected, and go to next frame
            if (pupil[i].x == -1 && pupil[i].y == -1) {
                std::cout << "pupil not found" << std::endl;
                break; //breaks out of inner loop, the for loop, and continues in the while loop
            }
            //Make pupil coordinates relative to whole capture image and not just eye bounding box
            pupil[i].x += eyes[i].x;
            pupil[i].y += eyes[i].y;
        }
        
        //calculate difference between current gaze, and far left, divide by num pixels, and multiply by resolution
        //average pupil location for current frame
        averageLocal = cv::Point((int)((pupil[0].x + pupil[1].x) / 2), (int)((pupil[0].y + pupil[1].y) / 2));
        if (averageLocal.x < ref_right.getPupilAvg().x || averageLocal.x > ref_left.getPupilAvg().x || averageLocal.y < ref_top.getPupilAvg().y || averageLocal.y > ref_bottom.getPupilAvg().y) {
            //TODO: head moving things
            std::cout << (int)(averageLocal.x < ref_right.getPupilAvg().x) << std::endl;
            std::cout << (int)(averageLocal.x > ref_left.getPupilAvg().x) << std::endl;
            std::cout << (int)(averageLocal.y < ref_top.getPupilAvg().y) << std::endl;
            std::cout << (int)(averageLocal.y > ref_bottom.getPupilAvg().y) << std::endl;
            continue;
        }
        
        //destination point on screen: to develop gradual moving of cursor
        //TODO: Improve... it's still jumpy
        
        destinationNew.x = (screenres.x - ((averageLocal.x - ref_right.getPupilAvg().x) * screenres.x / distance.x));
        destinationNew.y = (averageLocal.y - ref_top.getPupilAvg().y) * screenres.y / distance.y; //screenRes.y / 2;
        
        if (destinationOld != destinationNew) {
            destinationOld = destinationNew;
            delta = cv::Point((destinationNew.x - screenMap.x) / screenres.x * 80, (destinationNew.y - screenMap.y) / screenres.y * 80);
            if (delta.x > 0) { direction.x = 1; }
            else { direction.x = -1; }
            if (delta.y > 0) { direction.y = 1; }
            else { direction.y = -1; }
        }
        
        
        if (direction.x > 0) {
            screenMap.x = std::min(destinationNew.x, screenMap.x + delta.x);
        }
        else {
            screenMap.x = std::max(destinationNew.x, screenMap.x + delta.x);
        }
        if (direction.y > 0) {
            screenMap.y = std::min(destinationNew.y, screenMap.y + delta.y);
        }
        else {
            screenMap.y = std::max(destinationNew.y, screenMap.y + delta.y);
        }
        
        //point on screen:
        screenMap.x =(screenres.x - ( ( averageLocal.x - ref_right.getPupilAvg().x) * screenres.x / distance.x ));
        //TODO: implement y shifting
        screenMap.y = ( averageLocal.y - ref_top.getPupilAvg().y) * screenres.y /distance.y; //screenRes.y / 2;//
        
        
        //Enforce screen resolution as boundaries for movement of cursor
        if (screenMap.x >= 0 && screenMap.y >= 0 && screenMap.x <= screenres.x && screenMap.y <= screenres.y) {
            //SetCursorPos(screenMap.x, screenMap.y);
            //draw circle instead of moving cursor
            //TODO: Make program good enough with escape sequence so that we can actually use the cursor
            //		instead of the circle drawn below
//            singleton->setCurPos(screenMap);
            cv::circle(capture, screenMap, 5, cv::Scalar(235,244,66) , -1);
            imshow("CAPTURE", capture);
            cv::waitKey(1);
        }
        else {
            std::cout << "FUCK OUTOFBOUNDS COORDINATES" << std::endl;
            std::cout << "\tScreenMap.x: " << screenMap.x << "\tScreenMap.y: " << screenMap.y << std::endl;
        }
        
    }//while Loop
}




void RefImage::setImage(cv::Mat img){
    image = img;
}

void RefImage::setPupilAvg(cv::Point avg){
    pupilAvg = avg;
}

cv::Mat RefImage::getImage(){
    return image;
}

cv::Point RefImage::getPupilAvg(){
    return pupilAvg;
}
