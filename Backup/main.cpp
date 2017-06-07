// EyeLogicB.cpp : Defines the entry point for the console application.
//

#include "Backup.h"
#include <iostream>
using namespace std;


int main()
{
	if (startCap()) {
		averageEyeCenterMethod();
	}
	else {
		cout << "Failed to initialize capture" << endl;
		cin.get();
	}
	return 0;
}

