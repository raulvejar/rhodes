/******************************************************************************/
#pragma once

/******************************************************************************/
#include <windows.h>
#include <exdisp.h>

/******************************************************************************/
class CScrollNotify
{
public:
	CScrollNotify (IWebBrowser2 *pbrowser, HWND htarget);
	~CScrollNotify ();

	void NotifyProc (void);

private:
	IWebBrowser2 *pBrowser;
	HWND hTarget;
	HANDLE hQuit, hThread;

	static DWORD StaticNotifyProc (LPVOID pparam);
};
