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
	threadLock.lock();
	kill = false;
	threadLock.unlock();

	if(!systemSingleton->voiceFork())
	{
		return false;
	}
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
	threadLock.lock();
	while (!kill)
	{
		threadLock.unlock();
		std::string command = systemSingleton->readFromJulius();
		boost::algorithm::to_lower(command);

		if (command.compare("mute") == 0)
		{
			threadLock.lock();
			enabled = !enabled;
			threadLock.unlock();
		}

		if (enabled)
		{
			if (command.compare("click") == 0)
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
				RUN = false;
			}
		}
		threadLock.lock();
	}
	threadLock.unlock();
}
