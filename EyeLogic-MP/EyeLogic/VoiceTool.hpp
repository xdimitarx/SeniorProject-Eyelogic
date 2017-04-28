#ifndef VOICETOOL_H
#define VOICETOOL_H

#include "EyeLogic.hpp"

class VoiceTool
{
public:
	bool enableVoice();
	bool disableVoice();



private:

	//State Variables
	bool kill = false;
	bool newData = false;
	bool enabled = false;

	//Mutex Lock
	boost::mutex threadLock;

	//Command
	std::string command;

	bool initVoice();
	void monitor();
};


#endif






