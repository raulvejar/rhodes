// License.h: interface for the License class.
//
//////////////////////////////////////////////////////////////////////

#pragma once
#if !defined(AFX_LICENSE_H__DDDCACD0_485F_43D7_BA88_6590EF18ECE4__INCLUDED_)
#define AFX_LICENSE_H__DDDCACD0_485F_43D7_BA88_6590EF18ECE4__INCLUDED_


#if _MSC_VER > 1000

#endif // _MSC_VER > 1000

#pragma comment (lib,"wininet.lib")
//#ifdef DEBUG
//#pragma comment (lib,"../../common/Public/debug/PBPlugin.lib")
//#else 
//#pragma comment (lib,"../../common/Public/PBPlugin.lib")
//#endif



//calculates the number of elements in an array
#ifndef dim
	#define dim(x) (sizeof(x) / sizeof(x[0]))
#endif


#include "sip.h"
#include "..\public\pbplugin.h"
#include "licenseres\licenseres.h"
//#include "../../PBCore/PBCore/pbcore.h"
#include "rcmcapi.h"


/**
*  used in the wndproc
*/
struct decodeUINT {
	UINT Code;
	LRESULT (*Fxn)(HWND, UINT, WPARAM, LPARAM);
};

/**
*  used to store the previous wndprocs
*  during subclassing
*/
struct buttonStruct {
	HWND hwnd;
	WNDPROC WndProc;
};

/**
*  Class to handle everything to do with licensing the device for PB.
*  it also handles the validation of whether the device has been T&Vd
*  The class is fully autonomous.  All that is required is to create 
*  an instance of the class.
*  The checking for T&V status is a static function and should be called
*  before an instance of the class is created using the return value
*  true: continue, false: just quit the application
*/
class CLicense
{
public:
	
	/**
	*  Standard constructor
	*  \param hInstance - containing application Instance Handle
	*  \param hwndParent - Handle to the parent hwnd
	*/
	CLicense(HINSTANCE hInstance, HWND hwndParent, int iID);

	/**
	*  Initializes the wizard
	*  \return TRUE - wizard started successfully
	*  \return FALSE - init failed
	*/
	BOOL InitLicenseWizard();

	/**
	*  Standard destructor
	*/
	virtual ~CLicense();

	/**
	*  Called to determine if the device is exempt from licensing
	*  \return TRUE - exempt from licensing
	*  \return FALSE - not exepmt from licensing
	*/
	static BOOL IsSymbolDevice();

	/**
	*  Accessor for m_bLicenseScreenVisible
	*/
	static bool GetLicenseScreenVisible() {return m_bLicenseScreenVisible;};

	/**
	*  Handles the key down messages for the nontouch devices
	*  so that the licensing can be fully operable via the keyboard
	*  \param hwnd Handle to the window.
	*  \param uMsg Specifies the message.
	*  \param wParam Specifies additional message information. The contents of this parameter depend on the value of the uMsg parameter.
	*  \param lParam Specifies additional message information. The contents of this parameter depend on the value of the uMsg parameter.
	*  \return The return value is the result of the message processing and depends on the message sent.
	*/
	static LRESULT DoKeyDown(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
	
	/**
	*  Handles the mouse down messages for the "tap to continue"
	*  \param hwnd Handle to the window.
	*  \param uMsg Specifies the message.
	*  \param wParam Specifies additional message information. The contents of this parameter depend on the value of the uMsg parameter.
	*  \param lParam Specifies additional message information. The contents of this parameter depend on the value of the uMsg parameter.
	*  \return The return value is the result of the message processing and depends on the message sent.
	*/
	static LRESULT DoLButtonDown(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
	
	/**
	*  Handles the draw item messages drawing the buttons
	*  \param hwnd Handle to the window.
	*  \param uMsg Specifies the message.
	*  \param wParam Specifies additional message information. The contents of this parameter depend on the value of the uMsg parameter.
	*  \param lParam Specifies additional message information. The contents of this parameter depend on the value of the uMsg parameter.
	*  \return The return value is the result of the message processing and depends on the message sent.
	*/
	static LRESULT DoDrawItem(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
	
	/**
	*  Handles the timer messages 
	*  \param hwnd Handle to the window.
	*  \param uMsg Specifies the message.
	*  \param wParam Specifies additional message information. The contents of this parameter depend on the value of the uMsg parameter.
	*  \param lParam Specifies additional message information. The contents of this parameter depend on the value of the uMsg parameter.
	*  \return The return value is the result of the message processing and depends on the message sent.
	*/
	static LRESULT DoTimer(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
	
	/**
	*  Handles the command messages for handling the button clicks
	*  \param hwnd Handle to the window.
	*  \param uMsg Specifies the message.
	*  \param wParam Specifies additional message information. The contents of this parameter depend on the value of the uMsg parameter.
	*  \param lParam Specifies additional message information. The contents of this parameter depend on the value of the uMsg parameter.
	*  \return The return value is the result of the message processing and depends on the message sent.
	*/
	static LRESULT DoCommand(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
	
	/**
	*  Handles the paint messages so we can draw the splash screen
	*  and process the wizard
	*  \param hwnd Handle to the window.
	*  \param uMsg Specifies the message.
	*  \param wParam Specifies additional message information. The contents of this parameter depend on the value of the uMsg parameter.
	*  \param lParam Specifies additional message information. The contents of this parameter depend on the value of the uMsg parameter.
	*  \return The return value is the result of the message processing and depends on the message sent.
	*/
	static LRESULT DoPaint(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
	
	/**
	*  Handles the user message UM_GETLIC 
	*  so that the getting of the license from the licensing server
	*  is displayed on the screen
	*  \param hwnd Handle to the window.
	*  \param uMsg Specifies the message.
	*  \param wParam Specifies additional message information. The contents of this parameter depend on the value of the uMsg parameter.
	*  \param lParam Specifies additional message information. The contents of this parameter depend on the value of the uMsg parameter.
	*  \return The return value is the result of the message processing and depends on the message sent.
	*/
	static LRESULT DoGetLicense(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

	DWORD GetVersionInfo(LPCTSTR szFile, VS_FIXEDFILEINFO* lpDest);

	static bool m_bLicenseScreenVisible;	///< Whether or not the license screen is currently shown, used to determine accelerator key behaviour.

private:

	/**
	*  Called to see if the device is exempt from licensing
	*  \return TRUE - don't show the license screen
	*  \return FALSE - not exempt, show the license and splash
	*/
	BOOL DontShow();

	/**
	*  Accessor method for the m_iStep member
	*  \return value of m_iStep - the current step of the state machine
	*/
	int GetStepNo();
	
	/**
	*  Called when the screen rotates in order to resize everything
	*/
	void ResizeSplash();

	/**
	*  Hides the license screen
	*/
	void Cancel();

	/**
	*  Hides the license screen after tick is clicked
	*/
	void Tick();

	/**
	*  Sets the text of the input box
	*  \param tcText - the text to be entered in the text box
	*/
	void SetInputText(LPCTSTR tcText);
	
	/**
	*  Used to determine if the device is licensed or not
	*  \return TRUE - unit is licensed
	*  \return FALSE - unit is not licensed
	*/
	BOOL IsLicensed();

	/**
	*  Shows the splash screen and starts the process
	*/
	void ShowSplash();

	/**
	*  returns a scaled version of the input based on the screen height
	*  and a reference size of 240 * 320
	*  \param size - required pixel size on the reference screen
	*  \return scaled pixl value
	*/
	int scaledpx(int size);
	
	/**
	*  Loads the RCM library
	*  \return result of the load library.
	*/
	bool CLicense::LoadRCMLib();

	/**
	*  draws the background of the license screen
	*  \param hWnd window to draw on.
	*  \param hdc handle to a valid device context.
	*  \param rcRest detination rectangle
	*/
	void CLicense::DrawBackground(HWND hWnd, HDC hdc, RECT rcDest);
	
	/**
	*  draws the splash screen
	*  \param hWnd window to draw on.
	*  \param hdc handle to a valid device context.
	*  \param rcRest detination rectangle
	*/
	void DrawState_SplashScreen(HDC hdc, PAINTSTRUCT ps);
	
	/**
	*  draws the options screen
	*  \param hWnd window to draw on.
	*  \param hdc handle to a valid device context.
	*  \param rcRest detination rectangle
	*/
	void DrawState_Options(HDC hdc, PAINTSTRUCT ps);
	
	/**
	*  draws the UUID entry screen
	*  \param hWnd window to draw on.
	*  \param hdc handle to a valid device context.
	*  \param rcRest detination rectangle
	*/
	void DrawState_Manual_UUID(HDC hdc, PAINTSTRUCT ps);
	
	/**
	*  draws the company name entry screen
	*  \param hWnd window to draw on.
	*  \param hdc handle to a valid device context.
	*  \param rcRest detination rectangle
	*/
	void DrawState_Manual_COName(HDC hdc, PAINTSTRUCT ps);
	
	/**
	*  draws the license code screen
	*  \param hWnd window to draw on.
	*  \param hdc handle to a valid device context.
	*  \param rcRest detination rectangle
	*/
	void DrawState_Manual_Code(HDC hdc, PAINTSTRUCT ps);
	
	/**
	*  draws the results screen
	*  \param hWnd window to draw on.
	*  \param hdc handle to a valid device context.
	*  \param rcRest detination rectangle
	*/
	void DrawState_Manual_Result(HDC hdc, PAINTSTRUCT ps);

	/**
	*  draws the draws the serial entry screen
	*  \param hWnd window to draw on.
	*  \param hdc handle to a valid device context.
	*  \param rcRest detination rectangle
	*/
	void DrawState_Web_Serial(HDC hdc, PAINTSTRUCT ps);
	
	/**
	*  draws the order entry screen
	*  \param hWnd window to draw on.
	*  \param hdc handle to a valid device context.
	*  \param rcRest detination rectangle
	*/
	void DrawState_Web_Order(HDC hdc, PAINTSTRUCT ps);
	
	/**
	*  draws the communicating with server screen
	*  \param hWnd window to draw on.
	*  \param hdc handle to a valid device context.
	*  \param rcRest detination rectangle
	*/
	void DrawState_Web_Communicating(HDC hdc, PAINTSTRUCT ps);
	
	/**
	*  draws the results screen
	*  \param hWnd window to draw on.
	*  \param hdc handle to a valid device context.
	*  \param rcRest detination rectangle
	*/
	void DrawState_Web_Result(HDC hdc, PAINTSTRUCT ps);

	/**
	*  sets up the buttons on the splash screen
	*/
	void SWState_SplashScreen();
	
	/**
	*  sets up the buttons on the options screen
	*/
	void SWState_Options();
	
	/**
	*  sets up the buttons on the uuid screen
	*/
	void SWState_Manual_UUID();
	
	/**
	*  sets up the buttons on the coname screen
	*/
	void SWState_Manual_COName();
	
	/**
	*  sets up the buttons on the code screen
	*/
	void SWState_Manual_Code();
	
	/**
	*  sets up the buttons on the result screen
	*/
	void SWState_Manual_Result();

	
	/**
	*  sets up the buttons on the serial screen
	*/
	void SWState_Web_Serial();
	
	/**
	*  sets up the buttons on the order screen
	*/
	void SWState_Web_Order();
	
	/**
	*  sets up the buttons on the commincating screen
	*/
	void SWState_Web_Communicating();
	
	/**
	*  sets up the buttons on the result screen
	*/
	void SWState_Web_Result();


	/**
	*  part of the crc process
	*/
	TCHAR Crc16Add(UINT crc, TCHAR c);
	
	/**
	*  part of the crc process
	*/
	int Crc16Str(const TCHAR * lpstr);


	
	
	void BytesToHexStr(LPTSTR lpHexStr, LPBYTE lpBytes, int nSize);

	void HexToBase64(const TCHAR *ptcHex, TCHAR *ptcBase64);
	void DecryptPwd(TCHAR *ptcPwd, TCHAR* ptcDecrypted);
	void PopulateUID();
	long RegisterUnit();
	DWORD GetLSVersionInfo(LPCTSTR szFile);
	void DrawText(LPCTSTR szText, HDC hdc, COLORREF color, BOOL bBold, int iH, int iX, int iY, int iOrientation);
	void DrawSplash(HDC hDc);
	BOOL InitInstance();
	static LRESULT CALLBACK LicenseWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);


	void DrawRegion(HDC hDC, int l, int t, int r, int b, COLORREF cr);
	BOOL GetInternetFile (LPCTSTR lpszServer, LPCTSTR lpszUrl);

	int m_iAppID;
	HWND m_hwnd;
	UINT uTimer;
	TCHAR m_tcDIN[17];
	TCHAR m_tcDINCpy[17];
	long m_lRegResult;
	TCHAR m_szWebErrCode[20], m_szWebError[101];

	CSIP * m_pSIP;
	

	int m_iStep;
	int m_iPreStep;
	TCHAR *m_lpOutput;

	HMODULE m_hRCMLib; 
	UNITID_EX m_UUID;

	TCHAR m_tcCompany[31];
	TCHAR m_tcCode[51];
	TCHAR m_tcSerial[21];
	TCHAR m_tcOrder[21];
	TCHAR m_tcAPPBuild[6];

	//BOOL m_bScanWasSuspened;
	HINSTANCE m_hInstance;
	HWND m_ParentHwnd;

	
	int m_iH;
	VS_FIXEDFILEINFO m_vsAPPVer;
	TCHAR m_tcUID[33];

	IMOREF m_ScannerIMO;
		
	//BOOL m_bWantScan;
	HWND m_hwndKeyboard;
	HWND m_hwndInput;
	HWND m_hwndCancel;
	HWND m_hwndBack;
	HWND m_hwndNext;
	HWND m_hwndWeb;
	HWND m_hwndManual;	
	HWND m_hwndTick;
	HWND m_hSipWnd;
	HWND m_hwndLogo;

	PBSTRUCT m_pbStruct;

	
	buttonStruct m_TabStops[9];
	int m_iCurTabStop;
	bool m_bIsLicensed;


	static int DecodeBarcodeEvent(PVARSTRUCT pVars, int iTABID, LPARAM lParam);
};

#endif // !defined(AFX_LICENSE_H__DDDCACD0_485F_43D7_BA88_6590EF18ECE4__INCLUDED_)

