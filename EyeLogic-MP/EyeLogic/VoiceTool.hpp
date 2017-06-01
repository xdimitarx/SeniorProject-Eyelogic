#ifndef VOICETOOL_H
#define VOICETOOL_H

#include "System.hpp"
#include <boost/thread.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/algorithm/string.hpp>

extern std::unique_ptr<System> systemSingleton;
extern bool RUN;

class VoiceTool
{
public:

	static VoiceTool& voiceSingleton()
	{
		static VoiceTool instance;
		return instance;
	}
	
	bool enableVoice();
	bool disableVoice();
	bool initVoice();
	void stopVoice();

private:

	VoiceTool() {};

	//State Variables
	bool kill = false;
	bool newData = false;
	bool enabled = false;

	//Mutex Lock
	boost::mutex threadLock;

	//Command
	std::string command;

	void monitor();
};


#endif






