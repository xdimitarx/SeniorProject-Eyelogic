#ifndef Mac_h
#define Mac_h
#include "System.hpp"

using namespace std;

class Mac : public System {
public:
    
    cv::Point *currentPos;

	virtual int getOs()
	{
		return 1;
	}

	int ipcChannel[2];

	virtual bool voiceFork() override
	{
		
		if (pipe(ipcChannel) == -1)
		{
			return false;
		}

		pid_t id = fork();

		if (id == -1)
		{
			return false;
		}
		else if (id == 0)
		{
			//child
			close(ipcChannel[0]);
			dup2(ipcChannel[1], 1); // redirect stdout

			char* arguments[] = { "julius", "-C", "Voice.jconf", NULL };
			execvp("julius", arguments);
		}

		close(ipcChannel[1]); //just reads

		return true;
	}

	virtual std::string readFromJulius() override
	{
		std::string valueRead = "\0";

		char reading_buf[999];
		while (read(ipcChannel[0], reading_buf, 999) > 0)
		{
			string converted(reading_buf);
			int startIndex = converted.find("<s>");
			int endIndex = converted.find("</s>");
			if (startIndex != string::npos && endIndex != string::npos && endIndex > startIndex)
			{
				startIndex += 3;
				valueRead.append(converted.substr(startIndex, endIndex - startIndex));
			}
		}

		return valueRead;
	}

    virtual void setCurPos(cv::Point setCursor) override
    {
            
        float x = setCursor.x;
        float y = setCursor.y;
            
            
            
        // Move to 200x200
        CGEventRef move1 = CGEventCreateMouseEvent(
                                                   NULL, kCGEventMouseMoved,
                                                   CGPointMake(x, y),
                                                   kCGMouseButtonLeft // ignored
                                                   );
        CGEventPost(kCGHIDEventTap, move1);
        CFRelease(move1);
        
        currentPos->x = x;
        currentPos->y = y;
        
         
    }
    
    virtual void click() override {
        
        float x = currentPos->x;
        float y = currentPos->y;
        
        CGEventRef click1_down = CGEventCreateMouseEvent(
                                                         NULL, kCGEventLeftMouseDown,
                                                         CGPointMake(x, y),
                                                         kCGMouseButtonLeft
                                                         );
        
        // Left button down at 250x250
        // Left button up at 250x250
        CGEventRef click1_up = CGEventCreateMouseEvent(
                                                       NULL, kCGEventLeftMouseUp,
                                                       CGPointMake(x, y),
                                                       kCGMouseButtonLeft
                                                       );

        
        // Now, execute these events with an interval to make them noticeable

        CGEventPost(kCGHIDEventTap, click1_down);
        CGEventPost(kCGHIDEventTap, click1_up);
        CFRelease(click1_down);
        CFRelease(click1_up);

    }
    
    

};

#endif 
