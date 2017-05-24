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
	if(!systemSingleton->voiceFork())
	{
		//ERROR
	}
	enabled = true;
	std::unique_ptr<boost::thread> t ( new boost::thread(boost::bind(&VoiceTool::monitor, this)));
	return true;
}

void VoiceTool::stopVoice()
{
	threadLock.lock();
	kill = true;
	threadLock.unlock();

}

void VoiceTool::monitor()
{
	while (!kill)
	{
		std::string command = systemSingleton->readFromJulius();
		boost::algorithm::to_lower(command);

		if (command.compare("mute") == 0)
		{
			enabled = !enabled;
		}

		if(enabled)
		{
			if (command.compare("click"))
			{
				systemSingleton->click();
			}
			else if (command.compare("drag") == 0)
			{
				systemSingleton->drag();
			}
			else if (command.compare("double") == 0)
			{
				systemSingleton->doubleClick();
			}
			else if (command.compare("right") == 0)
			{
				systemSingleton->rightClick();
			}
			else if (command.compare("exit") == 0)
			{
				kill = true;
			}
			else if (command.compare("pause") == 0)
			{
				//pause command
			}
		}
	}
}
