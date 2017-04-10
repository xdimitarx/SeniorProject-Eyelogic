#include "VoiceTool.hpp"

bool VoiceTool::enableVoice()
{
	threadLock.lock();
	enabled = true;
	threadLock.unlock();
	return true;
}

bool VoiceTool::disableVoice()
{
	threadLock.lock();
	enabled = false;
	threadLock.unlock();
	return true;
}

bool VoiceTool::initVoice()
{
	//Check OS
	if(!singleton->voiceFork())
	{

	}
	enabled = true;
	boost::thread* t = new boost::thread(boost::bind(&VoiceTool::monitor, this));
	return true;
}

void VoiceTool::monitor()
{
	while (!kill)
	{
		if(enabled)
		{

		}
	}
	cleanUp();
}

void VoiceTool::cleanUp()
{

}
