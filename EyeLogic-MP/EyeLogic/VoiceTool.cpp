#include "VoiceTool.hpp"
#include <memory>

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
	std::unique_ptr<boost::thread> t ( new boost::thread(boost::bind(&VoiceTool::monitor, this)));
	// boost::thread* t = new boost::thread(boost::bind(&VoiceTool::monitor, this));	<--original
	return true;
}

void VoiceTool::monitor()
{
	while (!kill)
	{
		std::string command = singleton->readFromJulius();
		boost::algorithm::to_lower(command);

		if(enabled)
		{
			std::string command = singleton->readFromJulius();
			boost::algorithm::to_lower(command);

			if (command.compare("click"))
			{
				singleton->click();
			}
			else if (command.compare("drag") == 0)
			{
				singleton->drag();
			}
			else if (command.compare("double") == 0)
			{
				singleton->click();
				boost::this_thread::sleep(boost::posix_time::microseconds(300));
				singleton->click();
			}
			else if (command.compare("right") == 0)
			{
				singleton->rightClick();
			}
			else if (command.compare("exit") == 0)
			{
				//check if second time saying exit?
				kill = true;
				//exit handler?
			}
		}

		if (command.compare("mute") == 0)
		{
			threadLock.lock();
			enabled != enabled;
			threadLock.unlock();
		}

	}
}
