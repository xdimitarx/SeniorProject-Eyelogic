# SeniorProject-EyeLogic


# Setup Instructions

\1. Install Unity

\2. (Windows) Install Visual Studio

*** GET THE LATEST VERSION OF OPENCV ***

\3. (Windows) Install OpenCV
http://thomasmountainborn.com/2016/09/11/unity-and-opencv-part-one-install/

\3. (Mac) Install OpenCV
http://www.pyimagesearch.com/2015/06/15/install-opencv-3-0-and-python-2-7-on-osx/

\4. (Windows) Compile EyeLogicAlgorithm dll to root folder
http://thomasmountainborn.com/2016/09/12/unity-and-opencv-part-two-project-setup/

\4. (Mac) Compile EyeLogicAlgorithm dll to root folder
http://yeticrabgames.blogspot.com/2014/03/creating-dll-in-monodevelop.html


# Directory Structure

/EyeLogic/EyeLogic Algorithm/EyeLogic/
--- EyeTracking Module w/ openCV ---
EyeLogic.h
EyeLogic.cpp

/EyeLogic/Assets/EyeLogic/
--- Main Unity Interface, Calls EyeTracking Module ---
EyeLogic.cs

/EyeLogic/Assets/
--- Main Unity Scene ---
*Note: Button currently self destructs and returns Mat::total()
main.unity




