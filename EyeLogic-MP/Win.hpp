#ifndef Win_h
#define Win_h

#include "System.hpp"

using namespace std;


class Win : public System {
public:

	HANDLE g_hChildStd_OUT_Rd = NULL;

    /*virtual void setCurPos(float x, float y) override {
        // Windows way to set the cursor position to some x,y coordinate on the screen
        SetCursorPos((int)x, (int)y);
        //cout << x << "    " << y << endl;
        return;
    }*/

	virtual bool voiceFork() override
	{
		try
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
			TCHAR path[300];
			GetCurrentDirectory(300, path);

			bool bSuccess = CreateProcess(NULL,
				szCmdline,     // command line 
				NULL,          // process security attributes 
				NULL,          // primary thread security attributes 
				TRUE,          // handles are inherited 
				0,             // creation flags 
				NULL,          // use parent's environment 
				NULL,          // use parent's current directory 
				&siStartInfo,  // STARTUPINFO pointer 
				&piProcInfo);  // receives PROCESS_INFORMATION
			return true;
		}
		catch (...)
		{
			return false;
		}
		return false;		
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
				int startIndex = converted.rfind("sentence1: <s> ");
				int endIndex = converted.find(" </s>", startIndex);
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

    void setCurPos(cv::Point point) {
       // Windows way to set the cursor position to some x,y coordinate on the screen
        SetCursorPos(point.x, point.y);
        return;
    }

    cv::Point getCurPos() {
        //Windows way to get the cursor position
        LPPOINT windowsPoint;
        GetCursorPos((LPPOINT)&windowsPoint);
        cv::Point curr((int)windowsPoint->x, (int)windowsPoint->y);
        return curr;
    }
    
    
    
    cv::Point getScreenResolution () override {
        cv::Point screenres;
        RECT desktop;
        const HWND hDesktop = GetDesktopWindow();
        GetWindowRect(hDesktop, &desktop);
        screenres.x = desktop.right;
        screenres.y = desktop.bottom;
        return screenres
    }
    

};



#endif /* Win_h */
