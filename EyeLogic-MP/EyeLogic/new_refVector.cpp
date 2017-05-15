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

