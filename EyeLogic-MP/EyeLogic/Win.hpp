#ifndef Win_h
#define Win_h

#include "stdafx.h"
#include "windows.h"
#include "targetver.h"
#include "System.hpp"

//#include <iostream>

#include <opencv2/core.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/objdetect.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/imgproc.hpp>

using namespace std;


class Win : public System {
public:

	virtual int getOS() override
	{
		return 1;
	}

    virtual void setCurPos(float x, float y) override {
        // Windows way to set the cursor position to some x,y coordinate on the screen
        SetCursorPos((int)x, (int)y);
        //cout << x << "    " << y << endl;
        return;
    }

	virtual bool voiceFork() override
	{
		/*
		HANDLE g_hChildStd_OUT_Wr = NULL;
		HANDLE g_hChildStd_OUT_Rd = NULL;

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
		siStartInfo.hStdInput = g_hChildStd_IN_Rd;
		siStartInfo.dwFlags |= STARTF_USESTDHANDLES;

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
		*/
		return false;
	}

    void setCurPos(cv::Point point) {
        // Windows way to set the cursor position to some x,y coordinate on the screen
        SetCursorPos(point.x, point.y);
        //cout << point.x << "    " << point.y << endl;
        return;
    }

    cv::Point getCurPos() {
        //Windows way to get the cursor position
        LPPOINT windowsPoint;
        GetCursorPos((LPPOINT)&windowsPoint);
        cv::Point curr((int)windowsPoint->x, (int)windowsPoint->y);
        return curr;
    }

};



#endif /* Win_h */
