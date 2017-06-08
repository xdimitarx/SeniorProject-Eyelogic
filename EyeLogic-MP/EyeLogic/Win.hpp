#ifndef Win_h
#define Win_h

#include "System.hpp"

using namespace std;


class Win : public System {
public:

	HANDLE g_hChildStd_OUT_Rd = NULL;

	virtual bool voiceFork() override
	{
		HANDLE g_hChildStd_OUT_Wr = NULL;

		SECURITY_ATTRIBUTES saAttr;

		saAttr.nLength = sizeof(SECURITY_ATTRIBUTES);
		saAttr.bInheritHandle = TRUE;
		saAttr.lpSecurityDescriptor = NULL;

		if (!CreatePipe(&g_hChildStd_OUT_Rd, &g_hChildStd_OUT_Wr, &saAttr, 0))
			return false;

		PROCESS_INFORMATION piProcInfo;
		STARTUPINFO siStartInfo;

		ZeroMemory(&siStartInfo, sizeof(STARTUPINFO));
		siStartInfo.cb = sizeof(STARTUPINFO);
		siStartInfo.hStdError = g_hChildStd_OUT_Wr;
		siStartInfo.hStdOutput = g_hChildStd_OUT_Wr;
		siStartInfo.dwFlags |= STARTF_USESTDHANDLES;

		TCHAR szCmdline[] = TEXT("Grammar\\julius-4.3.1.exe -C Grammar\\Voice.jconf"); //might not work due to working directory

		bool bSuccess = CreateProcess(NULL,
			szCmdline,     // command line 
			NULL,          // process security attributes 
			NULL,          // primary thread security attributes 
			TRUE,          // handles are inherited 
			CREATE_NO_WINDOW,             // creation flags 
			NULL,          // use parent's environment 
			NULL,          // use parent's current directory 
			&siStartInfo,  // STARTUPINFO pointer 
			&piProcInfo);  // receives PROCESS_INFORMATION
		return bSuccess;		
	}

	virtual string readFromJulius() override
	{
		string valueRead = "\0";
		DWORD dwRead;
		CHAR chBuf[999];

		for (;;)
		{
			bool bSuccess = ReadFile(g_hChildStd_OUT_Rd, chBuf, 999, &dwRead, NULL);
			if (!bSuccess || dwRead == 0)
			{
				break;
			}
			else
			{
				string converted(chBuf);
				size_t startIndex = converted.rfind("sentence1: <s> ");
				size_t endIndex = converted.find(" </s>", startIndex);
				if (startIndex != string::npos && endIndex != string::npos && endIndex > startIndex)
				{
					startIndex += 15;
					valueRead.append(converted.substr(startIndex, endIndex - startIndex));
					return valueRead;
				}
			}

		}
		return valueRead;
	}

	//Working
	//Tester: Dom
    void setCurPos(cv::Point point) {
		if (point.x < 0 || point.y < 0)
		{
			cout << "Bad cursor coordinates!" << endl;
			return;
		}
       // Windows way to set the cursor position to some x,y coordinate on the screen
        SetCursorPos(point.x, point.y);
        return;
    }

	//Working
	//Tester: Dom
    cv::Point getCurPos() {
        //Windows way to get the cursor position
		POINT windowsPoint;
        GetCursorPos(&windowsPoint);
        cv::Point curr((int)windowsPoint.x, (int)windowsPoint.y);
        return curr;
    }
    
    //Working
	//Tester: Dom
    cv::Point getScreenResolution () override {
		SetProcessDPIAware();
        cv::Point screenres;
        screenres.x = (int)GetSystemMetrics(SM_CXSCREEN);
        screenres.y = (int)GetSystemMetrics(SM_CYSCREEN);
		return screenres;
    }
    
	//Working
	//Tester: Dom
	void drag() override
	{
		cv::Point cursor = getCurPos();
		if (!dragging)
		{
			dragStart = cursor;
			dragging = true;
		}
		else
		{
			setCurPos(dragStart);
			mouse_event(MOUSEEVENTF_LEFTDOWN, dragStart.x, dragStart.y, 0, 0);
			Sleep(50);
			setCurPos(cursor);
			mouse_event(MOUSEEVENTF_LEFTUP, cursor.x, cursor.y, 0, 0);
			dragging = false;
		}		
	}

	//Working
	//Tester: Dom
	void click() override
	{
		if (dragging)
		{
			return;
		}
		cv::Point cursor = getCurPos();

		mouse_event(MOUSEEVENTF_LEFTDOWN, cursor.x, cursor.y, 0, 0);
		Sleep(50);
		mouse_event(MOUSEEVENTF_LEFTUP, cursor.x, cursor.y, 0, 0);
	}

	//Working
	//Tester: Dom
	void doubleClick() override
	{
		if (dragging)
		{
			return;
		}

		click();
		Sleep(50);
		click();
	}

	//Working
	//Tester: Dom
	void rightClick() override
	{
		if (dragging)
		{
			return;
		}
		cv::Point cursor = getCurPos();

		mouse_event(MOUSEEVENTF_RIGHTDOWN, cursor.x, cursor.y, 0, 0);
		Sleep(50);
		mouse_event(MOUSEEVENTF_RIGHTUP, cursor.x, cursor.y, 0, 0);
	}

	void sleep(int timeMS) override
	{
		Sleep((unsigned long)timeMS);
		return;
	}

};



#endif /* Win_h */
