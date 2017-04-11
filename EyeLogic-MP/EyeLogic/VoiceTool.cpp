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
		//ERROR
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
			std::string command = singleton->readFromJulius();
			boost::algorithm::to_lower(command);

			if (command.compare("click"))
			{
				//singleton->click();
			}
			else if (command.compare("drag") == 0)
			{
				//singleton->drag();
			}
			else if (command.compare("double") == 0)
			{
				//singleton->click();
				boost::this_thread::sleep(boost::posix_time::microseconds(300));
				//singleton->click();
			}
			else if (command.compare("right") == 0)
			{
				//singleton->rightClick();
			}
			else if (command.compare("exit") == 0)
			{
				//exit handler?
			}
		}
	}
	cleanUp();
}

void VoiceTool::cleanUp()
{

}
