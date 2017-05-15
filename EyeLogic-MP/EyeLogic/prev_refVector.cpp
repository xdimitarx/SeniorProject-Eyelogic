/*
 *  Takes 40 images and calculates the eyeVector for each image frame. Keeps looping until 40 valid frames
 *  with left and right vectors are found or until MAX_FRAMES has been reached
 *
 *  Output: EyePair with left and right eye vectors for the associated reference image or nullptr
 */
EyePair *getRefVector(){

    if (!startCam())
	{

        cout << "camera is not available" << endl;
        return nullptr;
    }

    std::vector<Mat>images;
    std::vector<cv::Point>leftVectors;
    std::vector<cv::Point>rightVectors;
    int count = 0;
    
    
    // grab 40 images and store in images vector
    for(int j = 0; j < FRAMES; j++)
	{
        

        count++;

        // break if 80 images are taken and vectors for left and right can't be found
        if(count == MAXFRAMES)
		{
            cout << "could not find " << FRAMES << " frames in " << MAXFRAMES << " tries." << endl;
            return nullptr;
        }

        //take image
        Mat capture;
		
        cap >> capture;
        images.push_back(capture);

        // calculate eyeVector
        ImgFrame camera_frame;
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
    assert((int)leftVectors.size() == FRAMES && (int)rightVectors.size() == FRAMES);

//    **************************** TEST DATA
    std::vector<cv::Point>data = {
                                        {22,50},{47,29},{56,75},{40,26},{47,28},{41,34},{15,17},{81,77},{45,26},{22,44},
                                        {44,32},{42,27},{75,62},{52,99},{32,33},{42,34},{40,34},{45,26},{25,30},{46,30},
                                        {41,33},{14,69},{47,32},{42,26},{50,50},{36,17},{42,29},{41,31},{62,29},{20,25},
                                        {40,30},{77,19},{28,28},{44,32},{41,35},{27,29},{46,27},{50,61},{91,2},{47,26}
                                    };

        cv::Point *oneEye = getStabalizedCoord(data);
        cv::Point *noEye = getStabalizedCoord({});

        if(!noEye){
            cout << "null pointer" << endl;
        }

    return new EyePair(*oneEye, *noEye);
//    ****************************

//    // get reference vector for the left and right eye
//    cv::Point *left = getStabalizedCoord(leftVectors);
//    cv::Point *right = getStabalizedCoord(rightVectors);
//
//
//    // keep taking new set of 40 images until left and right eye Vector can be found <-- TOO HARSH?
//    if(!left || !right){
//        getRefVector();
//    }

//    return new EyePair(*left, *right);


}