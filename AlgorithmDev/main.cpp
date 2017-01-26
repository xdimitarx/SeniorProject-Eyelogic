#include "EyeLogic.h"
#include <iostream>

int main()
{
	vector<Mat> eyes;
	int flags = 0;
	if(!detectEyes("camera.jpg", &eyes, &flags))
		return 1;
	
	//check flags and print accordingly?
	imwrite("Test2.jpg", eyes[0]);
	imwrite("Test3.jpg", eyes[1]);

	cout << "finito" << endl;
	return 0;
}