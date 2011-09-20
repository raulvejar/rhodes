// License.cpp: implementation of the License class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
//#include "../../PBCore/PBCore/stdafx.h"
#include "../../PBCore/PBCore/AppManager.h"
#include "../../PBCore/PBCore/sync.h"
//#include "../../PBCore/PBCore/pbcore.h"
#include "License.h"

//#include "resource.h"
#include "windef.h"
#include "Wininet.h"
#include "rcmcapi.h"

#define UM_GETLIC	WM_USER + 1 ///> User Message for the getting of the license

extern IMOREF	CreateIMO		(PPBSTRUCT pPBStructure,LPCTSTR pTargetModuleName,LPCTSTR pCallingModName);
extern BOOL	DeleteIMO		(IMOREF IMORef);	
extern BOOL	SetIMOProperty	(IMOREF IMORef,LPCTSTR pParam,LPCTSTR pValue);
extern BOOL	CallIMOMethod	(IMOREF IMORef,LPCTSTR pMethod);
extern BOOL    SetIMOCallBack  (IMOREF IMORef,IMOEVENTPROC IMOEventProc,LPARAM lParam);
extern BOOL Log	(LogTypeInterface logSeverity,LPCTSTR pLogComment, 
									LPCTSTR pFunctionName, DWORD dwLineNumber,LPCTSTR pCallingModule);

extern BOOL	BrowserRestore			(int iAppID,LPCTSTR pCallingModule);

extern CAppManager			*g_pAppManager;
extern CSync				*g_pEventSync;	
extern PPBCORESTRUCT		g_pPBCore;
bool CLicense::m_bLicenseScreenVisible = false;

/**
 * Structure to hold Windows Message and functions
 */
const struct decodeUINT LicenseMessages[] = {
	WM_KEYDOWN, CLicense::DoKeyDown,
	WM_LBUTTONDOWN, CLicense::DoLButtonDown,
	WM_COMMAND, CLicense::DoCommand,
	WM_TIMER, CLicense::DoTimer,
	WM_DRAWITEM, CLicense::DoDrawItem,
	WM_PAINT, CLicense::DoPaint,
	UM_GETLIC, CLicense::DoGetLicense,
};

/**
 * function prototype for the RCM function to get 
 * the uuid
 */
typedef DWORD (*RCM_GETUNIQUEUNITIDEX)(LPUNITID_EX);
RCM_GETUNIQUEUNITIDEX lpfnRCM_GetUniqueUnitIdEx = NULL; ///< pointer to the RCM get uuid function

#define BUTTONHEIGHT scaledpx(16)///< calculated scaled button height

//THIS MUST BE 7 CHARS LONG!!!!!!!!!
#define LIC_KEY     L"MOTNEON" ///< the current product license key
//*************************//

#define CMD_CANCEL	0x6001		///< HMENU id for the cancel button
#define CMD_BACK	0x6002		///< HMENU id for the back button
#define CMD_NEXT	0x6003		///< HMENU id for the next button
#define CMD_WEB		0x6004		///< HMENU id for the internet button
#define CMD_KEYBOARD 0x6005		///< HMENU id for the keyboard button
#define CMD_INPUT	0x6006		///< HMENU id for the input box
#define CMD_TICK	0x6007		///< HMENU id for the tick image
#define CMD_LOGO	0x6008		///< HMENU id for the logo image
#define CMD_MANUAL  0x6009		///< HMENU id for the Manual button

#define BLACK		RGB(0x00,0x00,0x00)	///< define for the black colour used for drawing the background
#define WHITE		RGB(0xFF,0xFF,0xFF)	///< define for the white colour used for drawing the background
#define LIGHTGRAY	RGB(0xEB,0xEB,0xEB) ///< define for the Light gray colour used for drawing the background
#define GRAY		RGB(0x80,0x80,0x80) ///< define for the Gray colour used for drawing the copyright
#define TEXTCOLOR   RGB(0x4B,0x4B,0x4B) ///< define for the Gray colour used for drawing the Text
#define PRODCOLOR   RGB(0x33,0x33,0x33) ///< define for the Gray colour used for drawing the Heading

#define STATE_SPLASHSCREEN	0 ///> state machine state for the splash screen (tap to continue)
#define STATE_OPTIONS		1 ///> state machine state for the screen where the user choses manual of web lisensing
#define STATE_MANUAL_UUID	2 ///> state machine state for the screen showing the uuids and checksum
#define STATE_MANUAL_CONAME	3 ///> state machine state for the screen where the user enters the company name
#define STATE_MANUAL_CODE	4 ///> state machine state for the screen where the user enters the registration code
#define STATE_MANUAL_RESULT	5 ///> state machine state for the screen showing the result of the process
#define STATE_WEB_SERIAL	6 ///> state machine state for the screen where the user enters the serial number
#define STATE_WEB_ORDER		7 ///> state machine state for the screen where the enters the order number
#define STATE_WEB_COMMUNICATING 8 ///> state machine state for the screen when waiting for the server
#define STATE_WEB_RESULT	9 ///> state machine state for the screen showing the result of the process

#define BUTTONCANCEL	0		///> defined reference for the cancel button in the button array
#define BUTTONWEB		1		///> defined reference for the internet button in the button array
#define BUTTONBACK		2		///> defined reference for the back button in the button array
#define BUTTONNEXT		3		///> defined reference for the next button in the button array
#define BUTTONTICK		4		///> defined reference for the tick button in the button array
#define BUTTONINPUT		5		///> defined reference for the input button in the button array
#define BUTTONKEYBOARD	6		///> defined reference for the keyboard button in the button array
#define BUTTONLOGO		7		///> defined reference for the logo button/image in the button array
#define BUTTONMANUAL    8		///> defined reference for the Manual button in the button array


//When adding a string don't forget to increment the OEM_COUNT *****THIS IS NO LONGER A VALID STATEMENT AS THE ARRAY IS SIZED USING THE DIM MACRO ****

const TCHAR ExemptOEM[][20] = {L"MC17",L"MK500",L"MK4000",L"MK3000",};  ///< an array of OEM strings which are expemt from licensing
const TCHAR ValidOEM[][20] = {L"MC75A", L"MC55A", L"MC55N", L"MK500", 
							  L"VC6090", L"MC17", L"MK3000", L"MC2100",
							  L"MK4000", L"MC95", L"MC3100", L"MC9100", 
							  L"MC3190", L"MC9190", L"MC65", L"ES400",
							  L"5090"};///< an array of OEM strings which are validated to run PB

const TCHAR CRC16Table[260] = {
	0x0000,0x1021,0x2042,0x3063,0x4084,0x50A5,0x60C6,0x70E7,0x8108,0x9129,0xA14A,0xB16B,0xC18C,
	0xD1AD,0xE1CE,0xF1EF,0x1231,0x0210,0x3273,0x2252,0x52B5,0x4294,0x72F7,0x62D6,0x9339,0x8318,
	0xB37B,0xA35A,0xD3BD,0xC39C,0xF3FF,0xE3DE,0x2462,0x3443,0x0420,0x1401,0x64E6,0x74C7,0x44A4,
	0x5485,0xA56A,0xB54B,0x8528,0x9509,0xE5EE,0xF5CF,0xC5AC,0xD58D,0x3653,0x2672,0x1611,0x0630,
	0x76D7,0x66F6,0x5695,0x46B4,0xB75B,0xA77A,0x9719,0x8738,0xF7DF,0xE7FE,0xD79D,0xC7BC,0x48C4,
	0x58E5,0x6886,0x78A7,0x0840,0x1861,0x2802,0x3823,0xC9CC,0xD9ED,0xE98E,0xF9AF,0x8948,0x9969,
	0xA90A,0xB92B,0x5AF5,0x4AD4,0x7AB7,0x6A96,0x1A71,0x0A50,0x3A33,0x2A12,0xDBFD,0xCBDC,0xFBBF,
	0xEB9E,0x9B79,0x8B58,0xBB3B,0xAB1A,0x6CA6,0x7C87,0x4CE4,0x5CC5,0x2C22,0x3C03,0x0C60,0x1C41,
	0xEDAE,0xFD8F,0xCDEC,0xDDCD,0xAD2A,0xBD0B,0x8D68,0x9D49,0x7E97,0x6EB6,0x5ED5,0x4EF4,0x3E13,
	0x2E32,0x1E51,0x0E70,0xFF9F,0xEFBE,0xDFDD,0xCFFC,0xBF1B,0xAF3A,0x9F59,0x8F78,0x9188,0x81A9,
	0xB1CA,0xA1EB,0xD10C,0xC12D,0xF14E,0xE16F,0x1080,0x00A1,0x30C2,0x20E3,0x5004,0x4025,0x7046,
	0x6067,0x83B9,0x9398,0xA3FB,0xB3DA,0xC33D,0xD31C,0xE37F,0xF35E,0x02B1,0x1290,0x22F3,0x32D2,
	0x4235,0x5214,0x6277,0x7256,0xB5EA,0xA5CB,0x95A8,0x8589,0xF56E,0xE54F,0xD52C,0xC50D,0x34E2,
	0x24C3,0x14A0,0x0481,0x7466,0x6447,0x5424,0x4405,0xA7DB,0xB7FA,0x8799,0x97B8,0xE75F,0xF77E,
	0xC71D,0xD73C,0x26D3,0x36F2,0x0691,0x16B0,0x6657,0x7676,0x4615,0x5634,0xD94C,0xC96D,0xF90E,
	0xE92F,0x99C8,0x89E9,0xB98A,0xA9AB,0x5844,0x4865,0x7806,0x6827,0x18C0,0x08E1,0x3882,0x28A3,
	0xCB7D,0xDB5C,0xEB3F,0xFB1E,0x8BF9,0x9BD8,0xABBB,0xBB9A,0x4A75,0x5A54,0x6A37,0x7A16,0x0AF1,
	0x1AD0,0x2AB3,0x3A92,0xFD2E,0xED0F,0xDD6C,0xCD4D,0xBDAA,0xAD8B,0x9DE8,0x8DC9,0x7C26,0x6C07,
	0x5C64,0x4C45,0x3CA2,0x2C83,0x1CE0,0x0CC1,0xEF1F,0xFF3E,0xCF5D,0xDF7C,0xAF9B,0xBFBA,0x8FD9,
	0x9FF8,0x6E17,0x7E36,0x4E55,0x5E74,0x2E93,0x3EB2,0x0ED1,0x1EF0}; ///< an array holding the values for the CRC checksum table



//----------------------------------------------------------------------------
//
//  FUNCTION:   BytesToHexStr(LPTSTR, LPBYTE, int)
//
//  PURPOSE:    Convert a sequence of bytes to a string of its hex value 
//
//  PARAMETERS:
//      lpHexStr - pointer to the buffer receives the hex value string
//      lpBytes	 - pointer to the buffer contains the bytes to be converted
//      nSize	 - number of bytes to be converted
//
//  RETURN VALUE:
//      None.
//
//----------------------------------------------------------------------------

void CLicense::BytesToHexStr(LPTSTR lpHexStr, LPBYTE lpBytes, int nSize)
{
	//this function is lifted from the EMDK and converts the UUID into a sensible HEX string
	int		i;
	TCHAR	szByteStr[5];
	
	lpHexStr[0] = 0;

	for (i=0; i<nSize; i++)
	{
		wsprintf(szByteStr, TEXT("%02X"), lpBytes[i]);
		_tcscat(lpHexStr, szByteStr);
	}
}

////////////////////////////////////////////////////////////////////////
// Function:	CLicense
// Description:	Loads the verison info and gets the screen dimensions
// Author:		James Morley-Smith (jnp837)
// Date:		December 2004 initial implimentation (jnp837)
//				November 2009 modified for v3.0 framework (jnp837)
////////////////////////////////////////////////////////////////////////
CLicense::CLicense(HINSTANCE hInstance, HWND hwndParent, int iID)
	:m_iAppID(iID),
	m_hwnd(NULL),
	uTimer(0),
	m_iH(0),
	m_hwndKeyboard(NULL),
	m_hwndInput(NULL),
	m_hwndCancel(NULL),
	m_hwndBack(NULL),
	m_hwndNext(NULL),
	m_hwndWeb(NULL),
	m_hwndManual(NULL),
	m_hwndTick(NULL),
	m_hwndLogo(NULL),
	m_hInstance(hInstance),
	m_hSipWnd(NULL),
	m_lpOutput(NULL),
	m_iStep(0),
	m_iPreStep(-1),
	m_lRegResult(0),
	m_iCurTabStop(0),
	m_ParentHwnd(hwndParent),
	m_ScannerIMO(NULL),
	m_hRCMLib(NULL),
	m_pSIP(NULL),
	m_bIsLicensed(false)

{
	m_szWebErrCode[0] = NULL;
	m_szWebError[0] = NULL;
	m_tcCompany[0] = NULL;
	m_tcCode[0] = NULL;
	m_tcSerial[0] = NULL;
	m_tcOrder[0] = NULL;
	m_tcAPPBuild[0] = NULL;
	m_tcUID[0] = NULL;
	memset(&m_vsAPPVer, 0, sizeof(VS_FIXEDFILEINFO));


}

BOOL CLicense::InitLicenseWizard()
{
	//get the screen dimensions
	m_iH = GetSystemMetrics(SM_CYSCREEN);
	
	//get the application version numbers
	GetVersionInfo(L"", &m_vsAPPVer);

	//load the RCM library
	if(!LoadRCMLib())
		return FALSE;
	
	//get the uuid
	PopulateUID();

	//find the system sip button
	m_hSipWnd = FindWindow(L"SipWndClass", L"");

	m_pSIP = new CSIP();

	//if it's in the exempt list, just return and dont show anything
	if(DontShow())
		return TRUE;

	//initialize the window
	InitInstance();

	//show the splash screen and start the state machine
	ShowSplash();

	return TRUE;
}

////////////////////////////////////////////////////////////////////////
// Function:	~CLicense
// Description:	Destroys the window
// Author:		James Morley-Smith (jnp837)
// Date:		December 2004 initial implimentation (jnp837)
//				November 2009 modified for v3.0 framework (jnp837)
////////////////////////////////////////////////////////////////////////
CLicense::~CLicense()
{
	//if the splash screen still exists
	//destroy the timer and window
	if(m_hwnd) {
		DestroyWindow(m_hwnd);
	}
	m_hwnd = NULL;
	m_bLicenseScreenVisible = false;

	//delete the SIP object
	delete m_pSIP;
	m_pSIP = NULL;

	if (m_ScannerIMO)
		DeleteIMO(m_ScannerIMO);
	m_ScannerIMO = NULL;
}


////////////////////////////////////////////////////////////////////////
// Function:	LicenseWndProc
// Description:	Main message pump for the license process
// Author:		James Morley-Smith (jnp837)
// Date:		December 2004 initial implimentation (jnp837)
//				November 2009 modified for v3.0 framework (jnp837)
////////////////////////////////////////////////////////////////////////
LRESULT CALLBACK CLicense::LicenseWndProc(HWND hWnd, UINT wMsg, WPARAM wParam, LPARAM lParam)
{

	CLicense* pLic = (CLicense*)GetWindowLong(hWnd, GWL_USERDATA);///< pointer to the license object necessary as this is a static function

	if (wMsg == WM_KEYDOWN && wParam == VK_TAB)
		return DoKeyDown(hWnd, wMsg, wParam, lParam);

	//this for loop will process key presses from any of the subclassed buttons
	for(int iButtons = 0; iButtons < dim(pLic->m_TabStops); iButtons++)
	{
		if(hWnd == pLic->m_TabStops[iButtons].hwnd)
		{
			if(pLic->m_hwndInput == GetFocus() || wMsg != WM_KEYDOWN)
				return CallWindowProc(pLic->m_TabStops[iButtons].WndProc, hWnd, wMsg, wParam, lParam);
			
			pLic->m_iCurTabStop = iButtons;
			break;
		}
	}
	
	//usual Doug Boling stuff for handling windows messages
	for (int i = 0; i < dim(LicenseMessages); i++) 
	{
		if(wMsg == LicenseMessages[i].Code)
			return (*LicenseMessages[i].Fxn)(hWnd, wMsg, wParam, lParam);
	}

	return DefWindowProc(hWnd, wMsg, wParam, lParam);

}


////////////////////////////////////////////////////////////////////////
// Function:	DoKeyDown
// Description:	Handles the key processing for the touchless displays
// Author:		James Morley-Smith (jnp837)
// Date:		December 2004 initial implimentation (jnp837)
//				November 2009 modified for v3.0 framework (jnp837)
////////////////////////////////////////////////////////////////////////
LRESULT CLicense::DoKeyDown(HWND hWnd, UINT wMsg, WPARAM wParam, LPARAM lParam)
{
	static RECT rcRedraw; ///< holds the redraw region for this step
	CLicense* pLic = (CLicense*)GetWindowLong(hWnd, GWL_USERDATA); ///< pointer to the license object necessary as this is a static function

	//Specify the region which changes
	rcRedraw.left	= pLic->scaledpx(6);
	rcRedraw.top	= pLic->scaledpx(58);
	rcRedraw.bottom = pLic->scaledpx(246);
	rcRedraw.right  = pLic->scaledpx(240);


	//the first screen where the user can tap anywhere was tapped so increment
	//the step and invalidate the rectangle to progress the state machine
	switch (pLic->m_iStep){
	case STATE_SPLASHSCREEN: 
	case -1:
		//set the next state then invalidate the window to cause a WM_PAINT
		pLic->m_iStep = STATE_OPTIONS;
		InvalidateRect(pLic->m_hwnd, &rcRedraw, TRUE);
		return TRUE;
	}


	HWND hwndFocus = NULL; //used to detect if we are currently in the text input box
	int iter = 10;
	switch (wParam)
	{
		//Move to the previous button
		//if it's the 1st button, goto the second to last
		//(the last button is the logo...only used on the 1st screen)
		//if the window is not visible, we don't want focus on it so move to the next	
		//if the current focus window is the text box then we ignore left, back, space and right
		case VK_LEFT: case VK_BACK:
			hwndFocus = GetFocus();
			if(hwndFocus == pLic->m_hwndInput)
				return DefWindowProc(hWnd, wMsg, wParam, lParam);

		case VK_DOWN: 
			do
			{
				if(pLic->m_iCurTabStop <= 0)
					pLic->m_iCurTabStop = dim(pLic->m_TabStops) - 2;
				else
					pLic->m_iCurTabStop--;
				iter--;	//  EMBPD00043061.  Keys were being handled before license screen was functional
			} while(!IsWindowVisible(pLic->m_TabStops[pLic->m_iCurTabStop].hwnd) && 
				iter > 0);
			break;

		//Move to the next button
		//if it's the last button, goto the first button
		//if the window is not visible, we don't want focus on it so move to the next
		case VK_RIGHT: 
			hwndFocus = GetFocus();
			if(hwndFocus == pLic->m_hwndInput)
				return DefWindowProc(hWnd, wMsg, wParam, lParam);

		case VK_UP: case VK_TAB:
			do
			{
				if(pLic->m_iCurTabStop >= dim(pLic->m_TabStops) - 2)
					pLic->m_iCurTabStop = 0;
				else
					pLic->m_iCurTabStop++;
				iter--;  //  EMBPD00043061.  Keys were being handled before license screen was functional
			} while(!IsWindowVisible(pLic->m_TabStops[pLic->m_iCurTabStop].hwnd) &&
				iter > 0);
			break;

		//send a button click to the window
		case VK_SPACE:
			hwndFocus = GetFocus();
			if(hwndFocus == pLic->m_hwndInput)
				return DefWindowProc(hWnd, wMsg, wParam, lParam);
			else
				return SendMessage(pLic->m_TabStops[pLic->m_iCurTabStop].hwnd, VK_RETURN, 1, 0xA0009);

		case VK_RETURN:
			SendMessage(pLic->m_TabStops[pLic->m_iCurTabStop].hwnd, WM_LBUTTONDOWN, 1, 0xA0009);
			SendMessage(pLic->m_TabStops[pLic->m_iCurTabStop].hwnd, WM_LBUTTONUP, 0, 0x5000B);
			return TRUE;

		//cancel the license screen
		case VK_ESCAPE:
			pLic->Cancel();
			return TRUE;

	}

	//ok, set focus to the next button
	SetFocus(pLic->m_TabStops[pLic->m_iCurTabStop].hwnd);

	return DefWindowProc(hWnd, wMsg, wParam, lParam);
}

////////////////////////////////////////////////////////////////////////
// Function:	DoGetLicense
// Description:	Gets the license from the licensing server
// Author:		James Morley-Smith (jnp837)
// Date:		November 2009 modified for v3.0 framework (jnp837)
////////////////////////////////////////////////////////////////////////
LRESULT CLicense::DoGetLicense(HWND hWnd, UINT wMsg, WPARAM wParam, LPARAM lParam)
{
	static RECT rcRedraw; ///< holds the redraw region for this step
	CLicense* pLic = (CLicense*)GetWindowLong(hWnd, GWL_USERDATA); ///< pointer to the license object necessary as this is a static function

	TCHAR szCo[100], szUrl[1000], szCode[33];
	GetWindowText(pLic->m_hwndInput, pLic->m_tcOrder, 20);
	
	//set up the URLukli
	wsprintf(szUrl, L"/uklicense/lic.asp?ON=%s&SN=%s&DIN=%s", pLic->m_tcOrder, pLic->m_tcSerial, pLic->m_tcDINCpy);

	//make the call to the server
	pLic->m_lRegResult = 0;
	if(pLic->GetInternetFile(L"pocketbrowserlicense.motorolasolutions.com", szUrl)){
		
		//there was a response from the server
		memset(szCode, 0, 33 * sizeof(TCHAR));
		memset(szCo, 0, 100 * sizeof(TCHAR));
		memset(pLic->m_szWebErrCode, 0, 3 * sizeof(TCHAR));
		memset(pLic->m_szWebError, 0, 101 * sizeof(TCHAR));

		wcsncpy(pLic->m_szWebErrCode, pLic->m_lpOutput, 2);

		//the first two chars in the response are the error code
		//if the are anything but 00 it's a failure
		if(wcscmp(pLic->m_szWebErrCode, L"00")){

			//it failed so report it
			wcsncpy(pLic->m_szWebError, &pLic->m_lpOutput[2], 100);
			for(int i=wcslen(pLic->m_szWebError)-1; i>0; i--)
			{
				if(pLic->m_szWebError[i] == ' ') pLic->m_szWebError[i] = NULL;
				else break;
			}
			
			pLic->m_lRegResult = -1;
			
			MessageBeep(MB_ICONINFORMATION);

			
		}
		else
		{
			//it succeeded!

			//chop out the company name
			int i = 0;
			wcsncpy(szCo, &pLic->m_lpOutput[102], 30);
			for(i=wcslen(szCo)-1; i>0; i--)
			{
				if(szCo[i] == ' ') szCo[i] = NULL;
				else break;
			}

			//now find the license code
			wcsncpy(szCode, &pLic->m_lpOutput[132], 32);
			for(i=wcslen(szCode)-1; i>0; i--)
			{
				if(szCode[i] == ' ') szCode[i] = NULL;
				else break;
			}
			
			wcscpy(pLic->m_tcCompany, szCo);
			wcscpy(pLic->m_tcCode, szCode);
			
			//register the unit
			pLic->m_lRegResult = pLic->RegisterUnit();
			
			MessageBeep(MB_ICONINFORMATION);
			
		}
	}
	else{
		//there was a network fault
		MessageBeep(MB_ICONINFORMATION);
		wsprintf(pLic->m_szWebErrCode, L"0x%08X", GetLastError());
		wcscpy(pLic->m_szWebError, L"Communications Error.");
		pLic->m_lRegResult = -2;
	}

	//tidy up
	if(pLic->m_lpOutput !=NULL){
		delete[] pLic->m_lpOutput;
		pLic->m_lpOutput = NULL;
	}

	//show the next step
	pLic->m_iPreStep = STATE_WEB_SERIAL;
	pLic->m_iStep++;

	//Specify the region which changes
	rcRedraw.left	= pLic->scaledpx(6);
	rcRedraw.top	= pLic->scaledpx(58);
	rcRedraw.bottom = pLic->scaledpx(246);
	rcRedraw.right  = pLic->scaledpx(240);

	//invalidate the rectangle so the screen will progress
	InvalidateRect(pLic->m_hwnd, &rcRedraw, FALSE);

	return DefWindowProc(hWnd, wMsg, wParam, lParam);
}

////////////////////////////////////////////////////////////////////////
// Function:	DoLButtonDown
// Description:	Handles the stylus click so that the first screen can be clicked anywhere
// Author:		James Morley-Smith (jnp837)
// Date:		December 2004 initial implimentation (jnp837)
//				November 2009 modified for v3.0 framework (jnp837)
////////////////////////////////////////////////////////////////////////
LRESULT CLicense::DoLButtonDown(HWND hWnd, UINT wMsg, WPARAM wParam, LPARAM lParam)
{
	static RECT rcRedraw; ///< holds the redraw region for this step
	CLicense* pLic = (CLicense*)GetWindowLong(hWnd, GWL_USERDATA); ///< pointer to the license object necessary as this is a static function

	//Specify the region which changes
	rcRedraw.left	= pLic->scaledpx(6);
	rcRedraw.top	= pLic->scaledpx(58);
	rcRedraw.bottom = pLic->scaledpx(246);
	rcRedraw.right  = pLic->scaledpx(240);

	
	//SetEvent(CVars::m_EventSytemTimeout);
	if(pLic->IsLicensed())
		return DefWindowProc(hWnd, wMsg, wParam, lParam);

	//the first screen where the user can tap anywhere was tapped so increment
	//the step and invalidate the rectangle to progress the state machine
	switch (pLic->m_iStep){
	case STATE_SPLASHSCREEN: 
		//set the next state then invalidate the window to cause a WM_PAINT
		pLic->m_iStep = STATE_OPTIONS;
		InvalidateRect(hWnd, &rcRedraw, TRUE);
		break;
	}

	return DefWindowProc(hWnd, wMsg, wParam, lParam);
}


////////////////////////////////////////////////////////////////////////
// Function:	DoCommand
// Description:	Handles the button presses
// Author:		James Morley-Smith (jnp837)
// Date:		December 2004 initial implimentation (jnp837)
//				November 2009 modified for v3.0 framework (jnp837)
////////////////////////////////////////////////////////////////////////	
LRESULT CLicense::DoCommand(HWND hWnd, UINT wMsg, WPARAM wParam, LPARAM lParam)
{
	static RECT rcRedraw; ///< holds the redraw region for this step
	CLicense* pLic = (CLicense*)GetWindowLong(hWnd, GWL_USERDATA); ///< pointer to the license object necessary as this is a static function

	//Specify the region which changes
	rcRedraw.left	= pLic->scaledpx(6);
	rcRedraw.top	= pLic->scaledpx(58);
	rcRedraw.bottom = pLic->scaledpx(246);
	rcRedraw.right  = pLic->scaledpx(240);
	
	//one of the buttons was pressed
	DWORD wmId    = LOWORD(wParam); 
	switch (wmId) {
	case CMD_LOGO:
		//do nothing!!
		break;
	case CMD_MANUAL:
		//remember the last step and increment
		pLic->m_iPreStep = pLic->m_iStep;
		pLic->m_iStep++;
		break;
	case CMD_NEXT:
		//remember the last step and increment
		pLic->m_iPreStep = pLic->m_iStep;
		pLic->m_iStep++;
		break;
	case CMD_BACK:
		//remember the last step and decrement
		
		if(pLic->m_iStep==STATE_OPTIONS) 
			break; //don't want to go back any further

		if(pLic->m_iStep==STATE_WEB_RESULT 
		|| pLic->m_iStep==STATE_MANUAL_RESULT
		|| pLic->m_iStep==STATE_WEB_SERIAL){
			pLic->m_iStep = STATE_OPTIONS;
			pLic->m_iPreStep = -1;
			break;
		}

		pLic->m_iPreStep = pLic->m_iStep;
		pLic->m_iStep--;
		break;
	
	case CMD_KEYBOARD:
		{
		//toggle the SIP
		pLic->m_pSIP->ToggleSIP();
		//  EMBPD00033819 - SIP is appearing on top of the text box on the 
		//                  license screen
		//  Move the SIP window to the bottom of the screen
		HWND hSipWnd = FindWindow(L"SipWndClass", NULL);
		if (hSipWnd)
		{
			RECT rSIPCoords;
			if (GetClientRect(hSipWnd, &rSIPCoords))
			{
				int iScreenHeight = GetSystemMetrics(SM_CYSCREEN);
				MoveWindow(hSipWnd, rSIPCoords.left, iScreenHeight-(rSIPCoords.bottom - rSIPCoords.top), 
					rSIPCoords.right-rSIPCoords.left, rSIPCoords.bottom-rSIPCoords.top, TRUE);
			}
		}
		//  End EMBPD00033819
		//set focus to the input box
		pLic->m_iCurTabStop = BUTTONINPUT;
		SetFocus(pLic->m_TabStops[pLic->m_iCurTabStop].hwnd);
		break;
		}
	case CMD_TICK:
		pLic->Tick ();
		break;

	case CMD_CANCEL: 
		pLic->Cancel();
			
		break;

		//the registration is now complete so destroy the window so it is no longer displayed
		/*ShowWindow(pLic->m_hwnd, SW_HIDE);
		m_bLicenseScreenVisible = false;
		//if the scanner was previously not enabled, suspend it
		//if(m_bScanWasSuspened) {
		//	CScanner::EnableContinuousTrigger(FALSE);
		//	CScanner::StopScan();
		//}
		DestroyWindow(pLic->m_hwnd);
		pLic->m_hwnd = NULL;
		break;*/

	case CMD_WEB:
		//user wants to register over the web
		pLic->m_iPreStep = -1;
		pLic->m_iStep = STATE_WEB_SERIAL;
		break;
	case CMD_INPUT:
		if (wMsg == WM_KEYDOWN)
		{
			DoKeyDown(hWnd, wMsg, wParam, lParam);
			break;
		}
	default:
	   return DefWindowProc(hWnd, wMsg, wParam, lParam);
	}
	
	//invalidate the rectangle so the screen will progress
	InvalidateRect(pLic->m_hwnd, &rcRedraw, FALSE);

	return DefWindowProc(hWnd, wMsg, wParam, lParam);
}

////////////////////////////////////////////////////////////////////////
// Function:	DoTimer
// Description:	Timer to dismiss the license screen if the software is registered
// Author:		James Morley-Smith (jnp837)
// Date:		December 2004 initial implimentation (jnp837)
//				November 2009 modified for v3.0 framework (jnp837)
////////////////////////////////////////////////////////////////////////	
LRESULT CLicense::DoTimer(HWND hWnd, UINT wMsg, WPARAM wParam, LPARAM lParam)
{
	CLicense* pLic = (CLicense*)GetWindowLong(hWnd, GWL_USERDATA);///< pointer to the license object necessary as this is a static function

	if((DWORD)wParam == 1) //licesed - hide splash timer
	{

		g_pAppManager->SwitchAppInst(g_pPBCore->iPreviousInstID);
		g_pEventSync->Run(PB_APPFOCUSEVENT,pLic->m_iAppID,0);
		EnableWindow(pLic->m_ParentHwnd, TRUE);
		SetForegroundWindow(pLic->m_ParentHwnd);


		KillTimer(pLic->m_hwnd, (DWORD)wParam);

		//the unit was licensed so hide it and destroy it
		ShowWindow(pLic->m_hwnd, SW_HIDE);
		m_bLicenseScreenVisible = false;
		DestroyWindow(pLic->m_hwnd);
		pLic->m_hwnd = NULL;

		//  Set focus to the display window in WM6 (resolves EMBPD00029291)
		HWND hDisplayClass = FindWindow(DISPLAYCLASS, NULL);
		if (hDisplayClass != NULL)
		{
			SetForegroundWindow(hDisplayClass);
			SetFocus(hDisplayClass);
		}
		//  End fix for EMBPD00029291
				
	}
	else if ((DWORD)wParam == 2)  //unlicensed - show splash timer
	{
		KillTimer(pLic->m_hwnd, (DWORD)wParam);

		pLic->ShowSplash();
	}
	else if ((DWORD)wParam == 3) //required to set focus to the first screen
	{		
		//set the focus and keep the timer checking until after the first screen
		if(pLic->m_iStep == STATE_SPLASHSCREEN)
		{
			static HWND hwndTemp = GetFocus();

			if(hwndTemp != pLic->m_hwndLogo)
				SetFocus(pLic->m_hwndLogo);
		}
		else
			KillTimer(pLic->m_hwnd, 3);
	}
	return DefWindowProc(hWnd, wMsg, wParam, lParam);
}

////////////////////////////////////////////////////////////////////////
// Function:	DoDrawItem
// Description:	Draws the bitmaps on the buttons
// Author:		James Morley-Smith (jnp837)
// Date:		December 2004 initial implimentation (jnp837)
//				November 2009 modified for v3.0 framework (jnp837)
////////////////////////////////////////////////////////////////////////	
LRESULT CLicense::DoDrawItem(HWND hWnd, UINT wMsg, WPARAM wParam, LPARAM lParam)
{
	CLicense* pLic = (CLicense*)GetWindowLong(hWnd, GWL_USERDATA);///< pointer to the license object necessary as this is a static function

	//draw the buttons
	HDC hdcMemory, hdc;
	HBITMAP hBmp, hOldBmp;
	BITMAP bmp;
	RECT rcDest;
	LPDRAWITEMSTRUCT lpdis = (LPDRAWITEMSTRUCT) lParam;
	
	if (lpdis->CtlType == ODT_BUTTON) {
		hdc = lpdis->hDC;
		GetClientRect(lpdis->hwndItem, &rcDest);
		hdcMemory = CreateCompatibleDC(hdc);

		DWORD dwResourceID = 0;
		
		//get the appropriate resource ID based on the button state (focused|pressed|normal)
		switch (lpdis->CtlID) {
		case CMD_BACK:
			if(lpdis->itemState & ODS_SELECTED)	dwResourceID = IDB_BACK_DOWN;
			else if(lpdis->itemState & ODS_FOCUS) dwResourceID = IDB_BACK_FOCUS;
			else dwResourceID = IDB_BACK;
			break;
		case CMD_NEXT:
			if(lpdis->itemState & ODS_SELECTED)	dwResourceID = IDB_NEXT_DOWN;
			else if(lpdis->itemState & ODS_FOCUS) dwResourceID = IDB_NEXT_FOCUS;
			else dwResourceID = IDB_NEXT;
			break;
		case CMD_CANCEL:
			if(lpdis->itemState & ODS_SELECTED)	dwResourceID = IDB_CANCEL_DOWN;
			else if(lpdis->itemState & ODS_FOCUS) dwResourceID = IDB_CANCEL_FOCUS;
			else dwResourceID = IDB_CANCEL;
			break;
		case CMD_KEYBOARD:
			if(lpdis->itemState & ODS_SELECTED)	dwResourceID = IDB_KEYBOARD_DOWN;
			else if(lpdis->itemState & ODS_FOCUS) dwResourceID = IDB_KEYBOARD_FOCUS;
			else dwResourceID = IDB_KEYBOARD;
			break;
		case CMD_WEB:
			if(lpdis->itemState & ODS_SELECTED)	dwResourceID = IDB_WEB_DOWN;
			else if(lpdis->itemState & ODS_FOCUS) dwResourceID = IDB_WEB_FOCUS;
			else dwResourceID = IDB_WEB;
			break;
		case CMD_MANUAL:
			if(lpdis->itemState & ODS_SELECTED)	dwResourceID = IDB_MANUAL_DOWN;
			else if(lpdis->itemState & ODS_FOCUS) dwResourceID = IDB_MANUAL_FOCUS;
			else dwResourceID = IDB_MANUAL;
			break;
		case CMD_TICK:
			if(lpdis->itemState & ODS_SELECTED)	dwResourceID = IDB_TICK;
			else if(lpdis->itemState & ODS_FOCUS) dwResourceID = IDB_TICK_FOCUS;
			else dwResourceID = IDB_TICK;
			break;
		case CMD_LOGO:
			dwResourceID = IDB_LOGO;
			break;
		}

		hBmp = LoadBitmap(pLic->m_hInstance, MAKEINTRESOURCE(dwResourceID));

		//draw the bitmap and tidy up
		hOldBmp = (HBITMAP)SelectObject(hdcMemory, hBmp);
		GetObject(hBmp, sizeof(BITMAP), &bmp);
		StretchBlt(hdc, 0, 0, rcDest.right, rcDest.bottom, hdcMemory, 0, 0, bmp.bmWidth, bmp.bmHeight, SRCCOPY);
		DeleteObject(SelectObject(hdcMemory, hOldBmp));
		DeleteDC(hdcMemory);

	}
	return DefWindowProc(hWnd, wMsg, wParam, lParam);
}

////////////////////////////////////////////////////////////////////////
// Function:	DrawBackground
// Description:	Draws the slither bitmap on the background
// Author:		James Morley-Smith (jnp837)
// Date:		November 2009 modified for v3.0 framework (jnp837)
////////////////////////////////////////////////////////////////////////	
void CLicense::DrawBackground(HWND hWnd, HDC hdc, RECT rcDest)
{
	CLicense* pLic = (CLicense*)GetWindowLong(hWnd, GWL_USERDATA);///< pointer to the license object necessary as this is a static function

	//draw the buttons
	HDC hdcMemory;
	HBITMAP hBmp, hOldBmp;
	BITMAP bmp;

	GetClientRect(hWnd, &rcDest);
	hdcMemory = CreateCompatibleDC(hdc);
	hBmp = LoadBitmap(pLic->m_hInstance, MAKEINTRESOURCE(IDB_LICBACK));

	//draw the bitmap and tidy up
	hOldBmp = (HBITMAP)SelectObject(hdcMemory, hBmp);
	GetObject(hBmp, sizeof(BITMAP), &bmp);
	StretchBlt(hdc, 0, 0, rcDest.right, rcDest.bottom, hdcMemory, 0, 0, bmp.bmWidth, bmp.bmHeight, SRCCOPY);
	DeleteObject(SelectObject(hdcMemory, hOldBmp));
	DeleteDC(hdcMemory);

}


//converst all the hex code to binary
#define BIN_TO_64(c6bin, binary, pcBase64, c64char) \
	if(!strcmp(c6bin, binary)){strcat(pcBase64, c64char); continue;}

////////////////////////////////////////////////////////////////////////
// Function:	HexToBase64
// Description:	Converst Hex to the proprietory base 64
// Author:		James Morley-Smith
// Date:		December 2004
////////////////////////////////////////////////////////////////////////
void CLicense::HexToBase64(const TCHAR *ptcHex, TCHAR *ptcBase64)
{

	char pcHex[17];
	char pcBase64[14];

	wcstombs(pcHex, ptcHex, wcslen(ptcHex));
	pcHex[wcslen(ptcHex)] = NULL;

	char pcPad[17];
	pcPad[0] = NULL;
	for(UINT x=0; x < (16-strlen(pcHex)); x++)
		strcat(pcPad, "0");

	strcat(pcPad, pcHex);
	strcpy(pcHex, pcPad);

	const char *caBin[] = {"0000", "0001", "0010", "0011", "0100", "0101", "0110", "0111",
				    "1000", "1001", "1010", "1011", "1100", "1101", "1110", "1111"};
	char cBinary[67];
	strcpy(cBinary, "00");

	//convert each char to binnary
	for(int i=0; i<16; i++){
		char cHex[2];
		cHex[0] = pcHex[i];
		cHex[1] = NULL;
		strcat(cBinary, caBin[strtol(cHex, NULL, 16)]);
	}

	char c6bin[7];
	pcBase64[0] = NULL;

	//take the binary sections and make them the base64 equiv
	for(int j=0; j<67; j+=6){
		strncpy(c6bin, &cBinary[j], 6);
		c6bin[6] = NULL;
		BIN_TO_64(c6bin, "000000", pcBase64, "@");
		BIN_TO_64(c6bin, "000001", pcBase64, "1");
		BIN_TO_64(c6bin, "000010", pcBase64, "2");
		BIN_TO_64(c6bin, "000011", pcBase64, "3");
		BIN_TO_64(c6bin, "000100", pcBase64, "4");
		BIN_TO_64(c6bin, "000101", pcBase64, "5");
		BIN_TO_64(c6bin, "000110", pcBase64, "6");
		BIN_TO_64(c6bin, "000111", pcBase64, "7");
		BIN_TO_64(c6bin, "001000", pcBase64, "8");
		BIN_TO_64(c6bin, "001001", pcBase64, "9");
		BIN_TO_64(c6bin, "001010", pcBase64, "A");
		BIN_TO_64(c6bin, "001011", pcBase64, "B");
		BIN_TO_64(c6bin, "001100", pcBase64, "C");
		BIN_TO_64(c6bin, "001101", pcBase64, "D");
		BIN_TO_64(c6bin, "001110", pcBase64, "E");
		BIN_TO_64(c6bin, "001111", pcBase64, "F");
		BIN_TO_64(c6bin, "010000", pcBase64, "G");
		BIN_TO_64(c6bin, "010001", pcBase64, "H");
		BIN_TO_64(c6bin, "010010", pcBase64, "I");
		BIN_TO_64(c6bin, "010011", pcBase64, "J");
		BIN_TO_64(c6bin, "010100", pcBase64, "K");
		BIN_TO_64(c6bin, "010101", pcBase64, "L");
		BIN_TO_64(c6bin, "010110", pcBase64, "M");
		BIN_TO_64(c6bin, "010111", pcBase64, "N");
		BIN_TO_64(c6bin, "011000", pcBase64, "$");
		BIN_TO_64(c6bin, "011001", pcBase64, "P");
		BIN_TO_64(c6bin, "011010", pcBase64, "Q");
		BIN_TO_64(c6bin, "011011", pcBase64, "R");
		BIN_TO_64(c6bin, "011100", pcBase64, "S");
		BIN_TO_64(c6bin, "011101", pcBase64, "T");
		BIN_TO_64(c6bin, "011110", pcBase64, "U");
		BIN_TO_64(c6bin, "011111", pcBase64, "V");
		BIN_TO_64(c6bin, "100000", pcBase64, "W");
		BIN_TO_64(c6bin, "100001", pcBase64, "X");
		BIN_TO_64(c6bin, "100010", pcBase64, "Y");
		BIN_TO_64(c6bin, "100011", pcBase64, "Z");
		BIN_TO_64(c6bin, "100100", pcBase64, "-");
		BIN_TO_64(c6bin, "100101", pcBase64, "a");
		BIN_TO_64(c6bin, "100110", pcBase64, "b");
		BIN_TO_64(c6bin, "100111", pcBase64, "c");
		BIN_TO_64(c6bin, "101000", pcBase64, "d");
		BIN_TO_64(c6bin, "101001", pcBase64, "e");
		BIN_TO_64(c6bin, "101010", pcBase64, "f");
		BIN_TO_64(c6bin, "101011", pcBase64, "g");
		BIN_TO_64(c6bin, "101100", pcBase64, "h");
		BIN_TO_64(c6bin, "101101", pcBase64, "i");
		BIN_TO_64(c6bin, "101110", pcBase64, "j");
		BIN_TO_64(c6bin, "101111", pcBase64, "k");
		BIN_TO_64(c6bin, "110000", pcBase64, "l");
		BIN_TO_64(c6bin, "110001", pcBase64, "m");
		BIN_TO_64(c6bin, "110010", pcBase64, "n");
		BIN_TO_64(c6bin, "110011", pcBase64, "o");
		BIN_TO_64(c6bin, "110100", pcBase64, "p");
		BIN_TO_64(c6bin, "110101", pcBase64, "q");
		BIN_TO_64(c6bin, "110110", pcBase64, "r");
		BIN_TO_64(c6bin, "110111", pcBase64, "s");
		BIN_TO_64(c6bin, "111000", pcBase64, "t");
		BIN_TO_64(c6bin, "111001", pcBase64, "u");
		BIN_TO_64(c6bin, "111010", pcBase64, "v");
		BIN_TO_64(c6bin, "111011", pcBase64, "w");
		BIN_TO_64(c6bin, "111100", pcBase64, "x");
		BIN_TO_64(c6bin, "111101", pcBase64, "y");
		BIN_TO_64(c6bin, "111110", pcBase64, "z");
		BIN_TO_64(c6bin, "111111", pcBase64, "*");

	}

	//set the string
	mbstowcs(ptcBase64, pcBase64, strlen(pcBase64));
	ptcBase64[strlen(pcBase64)] = NULL;

/*const TCHAR *caBin[] = {L"0000", L"0001", L"0010", L"0011", L"0100", L"0101", L"0110", L"0111",
				    L"1000", L"1001", L"1010", L"1011", L"1100", L"1101", L"1110", L"1111"};

const TCHAR *SixBitsTable[] = {
	L"000000", L"000001", L"000010", L"000011", L"000100", L"000101", L"000110", L"000111", L"001000", L"001001", L"001010", 
	L"001011", L"001100", L"001101", L"001110", L"001111", L"010000", L"010001", L"010010", L"010011", L"010100", L"010101", 
	L"010110", L"010111", L"011000", L"011001", L"011010", L"011011", L"011100", L"011101", L"011110", L"011111", L"100000", 
	L"100001", L"100010", L"100011", L"100100", L"100101", L"100110", L"100111", L"101000", L"101001", L"101010", L"101011", 
	L"101100", L"101101", L"101110", L"101111", L"110000", L"110001", L"110010", L"110011", L"110100", L"110101", L"110110", 
	L"110111", L"111000", L"111001", L"111010", L"111011", L"111100", L"111101", L"111110", L"111111", 
};

const TCHAR *Char64Table[] = {
	L"@", L"1", L"2", L"3", L"4", L"5", L"6", L"7", L"8", L"9", L"A", L"B", L"C", L"D", L"E", L"F", L"G", L"H", L"I", L"J", L"K", L"L", L"M", L"N", L"$", L"P", L"Q",
	L"R", L"S", L"T", L"U", L"V", L"W", L"X", L"Y", L"Z", L"-", L"a", L"b", L"c", L"d", L"e", L"f", L"g", L"h", L"i", L"j", L"k", L"l", L"m", L"n", L"o", L"p", L"q",
	L"r", L"s", L"t", L"u", L"v", L"w", L"x", L"y", L"z", L"*",
};

	TCHAR tcBinary[67];
	wcscpy(tcBinary, L"00");

	//convert each char to binnary
	for(int i=0; i<16; i++){
		TCHAR tcHex[2];
		tcHex[0] = ptcHex[i];
		tcHex[1] = NULL;
		wcscat(tcBinary, caBin[wcstol(tcHex, NULL, 16)]);
	}

	TCHAR c6bin[7];
	ptcBase64[0] = NULL;

	//take the binary sections and make them the base64 equiv
	for(int j=0; j<67; j+=6){
		wcsncpy(c6bin, &tcBinary[j], 6);
		c6bin[6] = NULL;
		for(int q=0; q<64; q++)
		{
			if(!wcscmp(c6bin, SixBitsTable[q]))
			{
				wcscat(ptcBase64, Char64Table[q]); 
				break;
			}
		}
	}*/

	
}



////////////////////////////////////////////////////////////////////////
// Function:	InitInstance
// Description:	Creates the windows and starts the message pump
// Author:		James Morley-Smith
// Date:		December 2004
////////////////////////////////////////////////////////////////////////
BOOL CLicense::InitInstance()
{
	int iLeftPos = (GetSystemMetrics(SM_CXSCREEN)/2) - scaledpx(228/2);
	int iTopPos = (GetSystemMetrics(SM_CYSCREEN)/2) - scaledpx(278/2);

	m_hwnd = CreateWindowEx(WS_EX_NOANIMATION|WS_EX_TOPMOST, L"Dialog", L"Motorola License", WS_VISIBLE | WS_CLIPCHILDREN,
		iLeftPos, iTopPos, scaledpx(228), scaledpx(278), NULL, NULL, m_hInstance, NULL);
	
	if (!m_hwnd)
	{	
		//CSpbLog::Log(MEDIUM, SPB_LOG_ERROR, L"Could not create splash window",  L"CLicense", DEVICE_ID, __LINE__); 
		return FALSE;
	}

	
	m_hwndKeyboard = CreateWindowEx(WS_EX_NOANIMATION,_T("Button"), L"", 
			WS_CHILD | BS_OWNERDRAW | BS_PUSHBUTTON | WS_TABSTOP, 
			scaledpx(175), scaledpx(150), scaledpx(40), scaledpx(20), m_hwnd, (HMENU)CMD_KEYBOARD, m_hInstance, NULL);

	m_hwndCancel = CreateWindowEx(WS_EX_NOANIMATION,_T("Button"), L"", 
			WS_CHILD | BS_OWNERDRAW | BS_PUSHBUTTON | WS_TABSTOP , 
			scaledpx(175), scaledpx(8), scaledpx(40), scaledpx(18), m_hwnd, (HMENU)CMD_CANCEL, m_hInstance, NULL);

	m_hwndBack = CreateWindowEx(WS_EX_NOANIMATION,_T("Button"), L"", 
			WS_CHILD | BS_OWNERDRAW | BS_PUSHBUTTON | WS_TABSTOP , 
			scaledpx(20), scaledpx(220), scaledpx(65), scaledpx(25), m_hwnd, (HMENU)CMD_BACK, m_hInstance, NULL);

	m_hwndWeb = CreateWindowEx(WS_EX_NOANIMATION,_T("Button"), L"", 
			WS_CHILD | BS_OWNERDRAW | BS_PUSHBUTTON | WS_TABSTOP , 
			scaledpx(20), scaledpx(220), scaledpx(70), scaledpx(25), m_hwnd, (HMENU)CMD_WEB, m_hInstance, NULL);	

	m_hwndNext = CreateWindowEx(WS_EX_NOANIMATION,_T("Button"), L"", 
			WS_CHILD | BS_OWNERDRAW | BS_PUSHBUTTON | WS_TABSTOP , 
			scaledpx(145), scaledpx(220), scaledpx(65), scaledpx(25), m_hwnd, (HMENU)CMD_NEXT, m_hInstance, NULL);

	m_hwndLogo = CreateWindowEx(WS_EX_NOANIMATION,_T("Button"), L"", 
			WS_CHILD | BS_OWNERDRAW | BS_PUSHBUTTON | WS_TABSTOP , 
			scaledpx(20), scaledpx(45), scaledpx(32), scaledpx(32), m_hwnd, (HMENU)CMD_LOGO, m_hInstance, NULL);

	m_hwndTick = CreateWindowEx(WS_EX_NOANIMATION,_T("Button"), L"", 
			WS_CHILD | BS_OWNERDRAW | BS_PUSHBUTTON | WS_TABSTOP , 
			scaledpx(135), scaledpx(220), scaledpx(75), scaledpx(25), m_hwnd, (HMENU)CMD_TICK, m_hInstance, NULL);

	m_hwndInput = CreateWindowEx(WS_EX_NOANIMATION,_T("Edit"), L"", 
			WS_CHILD | WS_BORDER | ES_AUTOHSCROLL | WS_TABSTOP , 
			scaledpx(20), scaledpx(150), scaledpx(150), scaledpx(20), m_hwnd, (HMENU)CMD_INPUT, m_hInstance, NULL);

	m_hwndManual = CreateWindowEx(WS_EX_NOANIMATION,_T("Button"), L"", 
			WS_CHILD | BS_OWNERDRAW | BS_PUSHBUTTON | WS_TABSTOP , 
			scaledpx(130), scaledpx(220), scaledpx(70), scaledpx(25), m_hwnd, (HMENU)CMD_MANUAL, m_hInstance, NULL);

    SetWindowLong(m_hwnd, GWL_WNDPROC, (LONG) LicenseWndProc);
	SetWindowLong(m_hwnd, GWL_USERDATA, (LONG) this);

	m_TabStops[BUTTONCANCEL].hwnd = m_hwndCancel;
	m_TabStops[BUTTONWEB].hwnd = m_hwndWeb;	
	m_TabStops[BUTTONBACK].hwnd = m_hwndBack;
	m_TabStops[BUTTONNEXT].hwnd = m_hwndNext;
	m_TabStops[BUTTONTICK].hwnd = m_hwndTick;
	m_TabStops[BUTTONINPUT].hwnd = m_hwndInput;
	m_TabStops[BUTTONKEYBOARD].hwnd = m_hwndKeyboard;
	m_TabStops[BUTTONLOGO].hwnd = m_hwndLogo;
	m_TabStops[BUTTONMANUAL].hwnd = m_hwndManual;

	//need to subclass the buttons so that we get key presses
	int iButtons = 0;
	
	for(iButtons; iButtons < dim(m_TabStops); iButtons++)
	{
		m_TabStops[iButtons].WndProc = (WNDPROC) GetWindowLong(m_TabStops[iButtons].hwnd, GWL_WNDPROC);
		SetWindowLong(m_TabStops[iButtons].hwnd, GWL_USERDATA, (LONG) this);
		SetWindowLong(m_TabStops[iButtons].hwnd, GWL_WNDPROC, (LONG) LicenseWndProc);
	}

	m_iCurTabStop = BUTTONLOGO;
	SetFocus(m_TabStops[m_iCurTabStop].hwnd);
	
	return TRUE;
}


////////////////////////////////////////////////////////////////////////
// Function:	DrawRegion
// Description:	Draws a region within the specified coords in the 
//				specified colour
// Author:		James Morley-Smith
// Date:		December 2004
////////////////////////////////////////////////////////////////////////
void CLicense::DrawRegion(HDC hDC, int l, int t, int r, int b, COLORREF cr)
{
	// Create a rectangular region.
	HRGN hRgn = CreateRectRgn (l, t, r, b);
	
	// Create a solid brush.
	HBRUSH hBrush = CreateSolidBrush (cr); 

	// Fill the region out with the created brush.
	FillRgn (hDC, hRgn, hBrush);

	// Delete the rectangular region. 
	DeleteObject (hRgn);
	
	// Delete the brush object and free all resources associated with it.
	DeleteObject (hBrush);

}


////////////////////////////////////////////////////////////////////////
// Function:	scaledpx
// Description:	Scales a pixel value to the appropriate screen size based
//				a reference screen of 320 heigh
// Author:		James Morley-Smith
// Date:		November 2009
////////////////////////////////////////////////////////////////////////
int CLicense::scaledpx( int size)
{
	int retSize = (int)(m_iH/((double)320/(double)size));

	return retSize;
}

////////////////////////////////////////////////////////////////////////
// Function:	DrawSplash
// Description:	Draws the "TheEdge" background
// Author:		James Morley-Smith
// Date:		December 2004
////////////////////////////////////////////////////////////////////////
void CLicense::DrawSplash(HDC hDc)
{
	int left = 0;
	int top = 0;
	int right = 228;	
	int bottom = 278;
	int delta = 5;
	int border = 30;

	DrawRegion(hDc, left, top, scaledpx(right), scaledpx(bottom), BLACK);

	DrawRegion(hDc, scaledpx(0), scaledpx(0), scaledpx(right), scaledpx(delta), PRODCOLOR); //top
	DrawRegion(hDc, scaledpx(0), scaledpx(0), scaledpx(delta), scaledpx(bottom), PRODCOLOR); //left
	DrawRegion(hDc, scaledpx(right - delta), scaledpx(0), scaledpx(right), scaledpx(bottom), PRODCOLOR); //right
	DrawRegion(hDc, scaledpx(delta), scaledpx(0), scaledpx(right), scaledpx(bottom), PRODCOLOR); //bottom

	DrawRegion(hDc, scaledpx(delta), scaledpx(delta),scaledpx(right - delta),scaledpx(border),LIGHTGRAY); //first half
	DrawRegion(hDc, scaledpx(delta), scaledpx(border),scaledpx(right - delta),scaledpx(bottom-delta),WHITE); //second half

}

////////////////////////////////////////////////////////////////////////
// Function:	DrawText
// Description:	displays the text int he specified font and at the specfied
//				coords, in the specified colour
// Author:		James Morley-Smith
// Date:		December 2004
////////////////////////////////////////////////////////////////////////
void CLicense::DrawText(LPCTSTR szText, HDC hdc, COLORREF color, BOOL bBold, int iH, int iX, int iY, int iOrientation = DT_RIGHT)
{
	LOGFONT lf;
	HFONT hh = (HFONT)GetStockObject( SYSTEM_FONT );
	GetObject( hh, sizeof(lf), (LPVOID)&lf );

	lf.lfHeight = iH; 
	lf.lfWeight = (bBold?FW_BOLD:FW_NORMAL); 
	lf.lfUnderline = false;
	lf.lfItalic = false;
    wcscpy( lf.lfFaceName, L"Arial Narrow");

	HFONT hFnt = CreateFontIndirect( &lf ); 
	SelectObject(hdc, hFnt);
	
	RECT rt;
	::SetBkColor(hdc, color);
	SetBkMode(hdc, TRANSPARENT);

	GetClientRect(m_hwnd, &rt);
	SetTextColor(hdc, color);
	
	if(iOrientation==DT_RIGHT)
		rt.right = iX;
	else if(iOrientation==DT_CENTER){
		rt.left = iX-1000;
		rt.right = iX+1000;
	}
	else
		rt.left = iX;

	rt.top = iY;
	::DrawText(hdc, szText, wcslen(szText), &rt,  DT_NOPREFIX|DT_NOCLIP|iOrientation);
	DeleteObject(hFnt);

}


////////////////////////////////////////////////////////////////////////
// Function:	OnPaint
// Description:	The whole state machine is controlled here
//				the step tells the process what to draw and
// Author:		James Morley-Smith
// Date:		December 2004
////////////////////////////////////////////////////////////////////////
LRESULT CLicense::DoPaint(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	CLicense* pLic = (CLicense*)GetWindowLong(hWnd, GWL_USERDATA);
	
	HDC hdc;
	PAINTSTRUCT ps;
	
	//Before drawing, do all the necessary window hiding/showing
	switch (pLic->m_iStep)
	{
	case STATE_SPLASHSCREEN:
		pLic->SWState_SplashScreen();
		break;	
	case STATE_OPTIONS:
		pLic->SWState_Options();
		break;
	case STATE_MANUAL_UUID:
		pLic->SWState_Manual_UUID();
		break;
	case STATE_MANUAL_CONAME:
		pLic->SWState_Manual_COName();
		break;
	case STATE_MANUAL_CODE:
		pLic->SWState_Manual_Code();
		break;
	case STATE_MANUAL_RESULT:
		pLic->SWState_Manual_Result();
		break;
	case STATE_WEB_SERIAL:
		pLic->SWState_Web_Serial();
		break;
	case STATE_WEB_ORDER:
		pLic->SWState_Web_Order();
		break;
	case STATE_WEB_COMMUNICATING:
		pLic->SWState_Web_Communicating();
		break;
	case STATE_WEB_RESULT:
		pLic->SWState_Web_Result();
		break;
	}

	hdc = BeginPaint(hWnd, &ps);
	
	

//draw the background
	pLic->DrawSplash(hdc);
	ShowWindow(pLic->m_hwndLogo, SW_SHOW);

//draw the title
	pLic->DrawText(L"Product Registration", hdc, TEXTCOLOR, TRUE, pLic->scaledpx(13), pLic->scaledpx(125), pLic->scaledpx(12));
	pLic->DrawText(L"RHOELEMENTS", hdc, PRODCOLOR, TRUE, pLic->scaledpx(20), pLic->scaledpx(190), pLic->scaledpx(50));

//draw the copyright
	pLic->DrawText(L"© Copyright 2011 Motorola Solutions, Inc. All rights reserved.", hdc, GRAY, FALSE, 
		pLic->scaledpx(8), pLic->scaledpx(25), pLic->scaledpx(260), DT_LEFT);


//start wizzard
	switch (pLic->m_iStep)
	{
	case STATE_SPLASHSCREEN:
		pLic->DrawState_SplashScreen(hdc, ps);
		break;	
	case STATE_OPTIONS:
		pLic->DrawState_Options(hdc, ps);
		break;
	case STATE_MANUAL_UUID:
		pLic->DrawState_Manual_UUID(hdc, ps);
		break;
	case STATE_MANUAL_CONAME:
		pLic->DrawState_Manual_COName(hdc, ps);
		break;
	case STATE_MANUAL_CODE:
		pLic->DrawState_Manual_Code(hdc, ps);
		break;
	case STATE_MANUAL_RESULT:
		pLic->DrawState_Manual_Result(hdc, ps);
		break;
	case STATE_WEB_SERIAL:
		pLic->DrawState_Web_Serial(hdc, ps);
		break;
	case STATE_WEB_ORDER:
		pLic->DrawState_Web_Order(hdc, ps);
		break;
	case STATE_WEB_COMMUNICATING:
		pLic->DrawState_Web_Communicating(hdc, ps);
		break;
	case STATE_WEB_RESULT:
		pLic->DrawState_Web_Result(hdc, ps);
		break;
	}

	EndPaint(hWnd, &ps);
	

	SetTimer(pLic->m_hwnd, 3, 500, 0);
		

	return DefWindowProc(hWnd, message, wParam, lParam);
}

////////////////////////////////////////////////////////////////////////
// Function:	GetVersionInfo
// Description:	Gets the version info from the resource file of the specified
//				file
// Author:		James Morley-Smith
// Date:		December 2004
////////////////////////////////////////////////////////////////////////
DWORD CLicense::GetVersionInfo(LPCTSTR szFile, VS_FIXEDFILEINFO* lpDest)
{
	DWORD dwLen, dwUseless = 0;
    LPTSTR lpVI;
    DWORD dwVersion = 0;

	wchar_t wcFilename[256];
	if(wcslen(szFile)==0)
		GetModuleFileName(NULL, wcFilename, 255);
	else
		wcscpy(wcFilename, szFile);

    dwLen = GetFileVersionInfoSize(wcFilename, &dwUseless);
    if (dwLen==0) {
		dwLen = GetLastError();
        return dwVersion;
	}
    lpVI = (LPTSTR) GlobalAlloc(GPTR, dwLen);
    if (lpVI)
    {
        DWORD dwBufSize;
        VS_FIXEDFILEINFO* lpFFI;
        GetFileVersionInfo(wcFilename, NULL, dwLen, lpVI);

        if (VerQueryValue(lpVI, _T("\\"), (LPVOID *) &lpFFI, (UINT *) &dwBufSize))
		{
			memcpy(lpDest, lpFFI, sizeof(VS_FIXEDFILEINFO));
            dwVersion = lpFFI->dwFileVersionMS;
		}
        GlobalFree((HGLOBAL)lpVI);
    }

	return dwVersion;
}


/***********************************************************************

FUNCTION: 
  GetInternetFile

PURPOSE: 
  This function demonstrates how to create and submit an HTTP request.
  It requests the default HTML document from the server, and then 
  displays it along with the HTTP transaction headers.
 
***********************************************************************/
BOOL CLicense::GetInternetFile (LPCTSTR lpszServer, LPCTSTR lpszUrl)
{
	BOOL bReturn = TRUE;
	HINTERNET hOpen = NULL, hConnect = NULL, hRequest = NULL;
	LPVOID lpMsgBuf = NULL;

	DWORD dwSize = 0, dwFlags = INTERNET_FLAG_RELOAD | INTERNET_FLAG_NO_CACHE_WRITE | INTERNET_FLAG_NO_AUTH; 
	TCHAR szErrMsg[200];
	wcscpy(szErrMsg, L"");

	char *lpBufferA, *lpHeadersA;
	TCHAR *lpBufferW,*lpHeadersW;

	TCHAR * lpOutputBuff = NULL;

	LPTSTR AcceptTypes[2] = {TEXT("*/*"), NULL};

	hOpen = InternetOpen (m_tcUID, INTERNET_OPEN_TYPE_PRECONFIG, NULL, 0, 0);
	
	if (!hOpen) {

		unsigned long  ierr = GetLastError();
		FormatMessage( FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS, NULL, ierr, 0, (LPTSTR) &lpMsgBuf, 0, NULL );

		wsprintf (szErrMsg, TEXT("%s:\n%s(%x)"), TEXT("InternetOpen Error"), lpMsgBuf, ierr);

		//CSpbLog::Log(MEDIUM, SPB_LOG_ERROR, szErrMsg, L"CLicense", DEVICE_ID, __LINE__); 

		LocalFree( lpMsgBuf );

		return FALSE;
	}

	//this loop is used in plcae of a goto
	bool controlloop = true;
	while(controlloop){

		// Open an HTTP session for a specified site by using lpszServer. 
		hConnect = InternetConnect (hOpen, lpszServer, INTERNET_INVALID_PORT_NUMBER, NULL, NULL, INTERNET_SERVICE_HTTP, 0, 0);
		if (!hConnect) {
			unsigned long  ierr = GetLastError();
			FormatMessage( FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS, NULL, ierr, 0, (LPTSTR) &lpMsgBuf, 0, NULL );

			wsprintf (szErrMsg, TEXT("%s:\n%s(%x)"), TEXT("InternetConnect Error"), lpMsgBuf, ierr);

			//CSpbLog::Log(MEDIUM, SPB_LOG_ERROR, szErrMsg, L"CLicense", DEVICE_ID, __LINE__); 

			LocalFree( lpMsgBuf );
			
			bReturn = FALSE;
			break;
		}

		// Open an HTTP request handle. 
		hRequest = HttpOpenRequest (hConnect, TEXT("GET"), lpszUrl, HTTP_VERSION, NULL, (LPCTSTR*)AcceptTypes, dwFlags, 0);
		if (!hRequest) {
			unsigned long  ierr = GetLastError();
			FormatMessage( FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS, NULL, ierr, 0, (LPTSTR) &lpMsgBuf, 0, NULL );

			wsprintf (szErrMsg, TEXT("%s:\n%s(%x)"), TEXT("HttpOpenRequest Error"), lpMsgBuf, ierr);

			//CSpbLog::Log(MEDIUM, SPB_LOG_ERROR, szErrMsg,L"CLicense", DEVICE_ID, __LINE__); 

			LocalFree( lpMsgBuf );
			bReturn = FALSE;
			break;
		}

		// Send a request to the HTTP server. 
		if (!HttpSendRequest (hRequest, NULL, 0, NULL, 0)) {
			unsigned long  ierr = GetLastError();
			FormatMessage( FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS, NULL, ierr, 0, (LPTSTR) &lpMsgBuf, 0, NULL );

			wsprintf (szErrMsg, TEXT("%s:\n%s(%x)"), TEXT("HttpSendRequest Error"), lpMsgBuf, ierr);

			//CSpbLog::Log(MEDIUM, SPB_LOG_ERROR, szErrMsg,L"CLicense", DEVICE_ID, __LINE__); 

			LocalFree( lpMsgBuf );
			bReturn = FALSE;
			break;
		}
		

		// Call HttpQueryInfo to find out the size of the headers.
		HttpQueryInfo (hRequest, HTTP_QUERY_RAW_HEADERS_CRLF, NULL, &dwSize, NULL);

		// Allocate a block of memory for lpHeadersA.
		lpHeadersA = new CHAR [dwSize];
		if(lpHeadersA ==NULL){
			MessageBox(NULL, L"Out of memory", L"RhoElements", MB_ICONERROR|MB_TOPMOST|MB_APPLMODAL);
			bReturn = FALSE;
			break;
		}

		// Call HttpQueryInfo again to get the headers.
		if (!HttpQueryInfo (hRequest, HTTP_QUERY_RAW_HEADERS_CRLF, (LPVOID) lpHeadersA, &dwSize, NULL)) {
			
			unsigned long  ierr = GetLastError();
			FormatMessage( FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS, NULL, ierr, 0, (LPTSTR) &lpMsgBuf, 0, NULL );

			wsprintf (szErrMsg, TEXT("%s:\n%s(%x)"), TEXT("HttpQueryInfo Error"), lpMsgBuf, ierr);

			//CSpbLog::Log(MEDIUM,SPB_LOG_ERROR, szErrMsg, L"CLicense", DEVICE_ID, __LINE__); 

			LocalFree( lpMsgBuf );
			bReturn = FALSE;
			break;
		}

		// Terminate headers with NULL.
		lpHeadersA [dwSize] = '\0';

		// Get the required size of the buffer that receives the Unicode 
		// string. 
		dwSize = (DWORD)MultiByteToWideChar (CP_ACP, 0, lpHeadersA, -1, NULL, 0);

		// Allocate a block of memory for lpHeadersW.
		lpHeadersW = new TCHAR [dwSize];

		// Convert headers from ASCII to Unicode
		MultiByteToWideChar (CP_ACP, 0, lpHeadersA, -1, lpHeadersW, dwSize);  

		// Free the blocks of memory.
		delete[] lpHeadersA;
		delete[] lpHeadersW;

		// Allocate a block of memory for lpHeadersW.
		lpBufferA = new CHAR [32000];
		if(lpBufferA ==NULL){
			MessageBox(NULL, L"Out of memory", L"RhoElements", MB_ICONERROR|MB_TOPMOST|MB_APPLMODAL);
			bReturn = FALSE;
			break;
		}

		do {
			if (!InternetReadFile (hRequest, (LPVOID)lpBufferA, 32000, &dwSize)) {
				unsigned long  ierr = GetLastError();
				FormatMessage( FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS, NULL, ierr, 0, (LPTSTR) &lpMsgBuf, 0, NULL );

				wsprintf (szErrMsg, TEXT("%s:\n%s(%x)"), TEXT("InternetReadFile Error"), lpMsgBuf, ierr);

				//CSpbLog::Log(MEDIUM,SPB_LOG_ERROR, szErrMsg, L"CLicense", DEVICE_ID, __LINE__); 

				LocalFree( lpMsgBuf );
				bReturn = FALSE;
				break;
			}

			if (dwSize != 0) {
				// Terminate headers with NULL.
				lpBufferA [dwSize] = '\0';                 

				// Get the required size of the buffer which receives the Unicode
				// string. 
				dwSize = (DWORD)MultiByteToWideChar (CP_ACP, 0, lpBufferA, -1, NULL, 0);

				// Allocate a block of memory for lpBufferW.
				lpBufferW = new TCHAR [dwSize];

				if(lpBufferW ==NULL){
					MessageBox(NULL, L"Out of memory", L"RhoElements", MB_ICONERROR|MB_TOPMOST|MB_APPLMODAL);
					bReturn = FALSE;
					break;
				}

				if(m_lpOutput == NULL){
					lpOutputBuff = new TCHAR[1];
					if(lpOutputBuff ==NULL){
						MessageBox(NULL, L"Out of memory", L"RhoElements", MB_ICONERROR|MB_TOPMOST|MB_APPLMODAL);
						bReturn = FALSE;
						break;
					}
					lpOutputBuff[0] = NULL;

				}
				else{
					lpOutputBuff = new TCHAR[wcslen(m_lpOutput)];
					if(lpOutputBuff ==NULL){
						MessageBox(NULL, L"Out of memory", L"RhoElements", MB_ICONERROR|MB_TOPMOST|MB_APPLMODAL);
						bReturn = FALSE;
						break;
					}
					wcscpy(lpOutputBuff, m_lpOutput);
					delete [] m_lpOutput;
					m_lpOutput = NULL;

				}

				m_lpOutput = new TCHAR[wcslen(lpOutputBuff) + dwSize +1];
				if(m_lpOutput ==NULL){
					MessageBox(NULL, L"Out of memory", L"RhoElements", MB_ICONERROR|MB_TOPMOST|MB_APPLMODAL);
					bReturn = FALSE;
					break;
				}
				mbstowcs(m_lpOutput, lpBufferA, strlen(lpBufferA));
				m_lpOutput[strlen(lpBufferA)] = NULL;
				// Convert the buffer from ASCII to Unicode.
				MultiByteToWideChar (CP_ACP, 0, lpBufferA, -1, lpBufferW, dwSize);

				wcscpy(m_lpOutput, lpOutputBuff);
				wcscat(m_lpOutput, lpBufferW);

				delete[] lpOutputBuff;

				// Put the buffer in the edit control.
				//SendMessage (g_hwndMain, WM_PUTTEXT, NULL, (LPARAM) lpBufferW);      
				//RETAILMSG(1, (L"buffer: %s\n", lpBufferW));


				// Free the block of memory.
				delete[] lpBufferW;  
			}    
		} while (dwSize);


		// Free the block of memory.
		delete[] lpBufferA;  

		//this while loop was used as goto's are not good programming practice
		break;
	}
	
	//exit code
	
	// Close the Internet handles.
	if (hOpen) {
		InternetCloseHandle (hOpen);
		
	}

	if (hConnect) {
		InternetCloseHandle (hConnect);
		
	}

	
	if(!bReturn){
		if(m_lpOutput !=NULL){
			delete[] m_lpOutput;
			m_lpOutput = NULL;
		}

		m_lpOutput = new TCHAR[wcslen(szErrMsg)];
		wcscpy(m_lpOutput, szErrMsg);
	}

	return bReturn;
}

////////////////////////////////////////////////////////////////////////
// Function:	RegisterUnit
// Description:	Attempts to register the unit based on the details provided
//				by the user.
// Author:		James Morley-Smith
// Date:		December 2004
////////////////////////////////////////////////////////////////////////
long CLicense::RegisterUnit()
{
	//SetCursor(LoadCursor(m_hInstance, IDC_WAIT));
	//CVars::m_pCursor->StartAnimation();
//	DEBUGMSG(1, (L"NEED TO START THE WAIT CURSOR"));


	//get the device details
	PopulateUID();

	long lReturn = 0;
	int iChecksum = 0;

	//this while is used in place of a goto which is considered a bad programming practice
	bool controlloop = true;
	while(controlloop){
		

		//create a checksum from the company name
		for(UINT iCS=0; iCS<wcslen(m_tcCompany); iCS++)
			iChecksum+=m_tcCompany[iCS];
		
		if(iChecksum > 0xEE2){
			lReturn = 1;
			break;
		}

		TCHAR tcReg[500];
		wcscpy(tcReg, m_tcCode);
		TCHAR tcDecrypted[500];
		TCHAR tcKey[170];
		wcscpy(tcKey, m_tcDIN);

		//decrupt the license code provided
		DecryptPwd(tcReg, tcDecrypted);
		wcscpy(tcReg, m_tcCode);

		TCHAR tcCS[50];
		TCHAR tcLicenseKey[16];
		
		//compare the checksum with the one in the decrypted code
		wsprintf(tcCS, L"%03X:", iChecksum);
		if(wcsncmp(tcCS, tcDecrypted, 4)){
			//if it doesn't compare then better try the corporate license
			wcscpy(m_tcDIN, L"CORPERATELICENSE");
			wcscpy(tcReg, m_tcCode);
			DecryptPwd(tcReg, tcDecrypted);

			//if it fails again it was a duff license
			if(wcsncmp(tcCS, tcDecrypted, 4)){
				lReturn = 1;

				//CSpbLog::Log(MEDIUM,SPB_LOG_INFORMATION, L"Registration failed", L"CLicense", DEVICE_ID, __LINE__); 
				break;
			}
			
			//it seems there has always been a bug with deployment licenses
			//whereby the 11th character of the product code is changed for a
			// capital oh "O".  This has never been noticed before because our
			//products codes in the past have always had a captial oh "O" as 
			//the 11th character.  EG:  for PB 2, it was "INT-SW-SBROW"
			//                                                      ^
			//so, this  means that regardless of the product code on the system
			//for deployment licenses, we must set this character as "O"
			wsprintf(tcLicenseKey, L"%s%d_%dO0", LIC_KEY, HIWORD(m_vsAPPVer.dwFileVersionMS), LOWORD(m_vsAPPVer.dwFileVersionMS),HIWORD(m_vsAPPVer.dwFileVersionLS));
		
		}
		else
			wsprintf(tcLicenseKey, L"%s%d_%d_0", LIC_KEY, HIWORD(m_vsAPPVer.dwFileVersionMS), LOWORD(m_vsAPPVer.dwFileVersionMS),HIWORD(m_vsAPPVer.dwFileVersionLS));
		
		//ok, the company name passes so make sure it's the correct part number for the license
		if(wcscmp(&tcDecrypted[4],tcLicenseKey)){
			lReturn = 2;
			//CSpbLog::Log(MEDIUM,SPB_LOG_INFORMATION,  L"Registraiton Failed", L"CLicense", DEVICE_ID, __LINE__); 
			break;
		}
		wcscpy(tcReg, m_tcCode);

		

		//great, it passed to write it out to the reg file
		HKEY hKey;
		DWORD Disposition;
		RegCreateKeyEx( HKEY_CURRENT_USER, L"Software\\Symbol\\SymbolPB\\Licence", 0, NULL, 0, 0, 0, &hKey, &Disposition ); 
		RegSetValueEx(hKey, L"Code", 0, REG_MULTI_SZ, (const BYTE *) tcReg, (wcslen(tcReg)+1)*2); 
		RegSetValueEx(hKey, L"Company", 0, REG_MULTI_SZ, (const BYTE *) m_tcCompany, (wcslen(m_tcCompany)+1)*2); 
		RegCloseKey(hKey);
			
		// Write value to safe file

		char pcBuf[40];
		DWORD dwBytesWritten = 0;
		

		HANDLE hFile = CreateFile(_T("\\application\\sie_lic.reg"), GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
		if (hFile != INVALID_HANDLE_VALUE) {
			WriteFile(hFile, "[HKEY_CURRENT_USER\\Software\\Symbol\\SymbolPB\\Licence]\r\n"
				, strlen("[HKEY_CURRENT_USER\\Software\\Symbol\\SymbolPB\\Licence]\r\n"), &dwBytesWritten, NULL);
			WriteFile(hFile, "\"Code\"=\"", strlen("\"Code\"=\""), &dwBytesWritten, NULL);
			wcstombs(pcBuf, tcReg, wcslen(tcReg));
			pcBuf[wcslen(tcReg)] = NULL;
			WriteFile(hFile, pcBuf, strlen(pcBuf), &dwBytesWritten, NULL);
			WriteFile(hFile, "\"\r\n\"Company\"=\"", strlen("\"\r\n\"Company\"=\""), &dwBytesWritten, NULL);
			wcstombs(pcBuf, m_tcCompany, wcslen(m_tcCompany));
			pcBuf[wcslen(m_tcCompany)] = NULL;
			WriteFile(hFile, pcBuf, strlen(pcBuf), &dwBytesWritten, NULL);
			WriteFile(hFile, "\"\r\n", strlen("\"\r\n"), &dwBytesWritten, NULL);
			CloseHandle(hFile);
		}
		
		
		lReturn = 0;

		//this while was used in place of a got which is considered a bad programming practice
		break;
	}

	//SetCursor(LoadCursor(m_hInstance, IDC_ARROW));
	//Vars::m_pCursor->StopAnimation();
//	DEBUGMSG(1, (L"NEED TO STOP THE WAIT CURSOR"));

		
	//CSpbLog::Log(MEDIUM,SPB_LOG_INFORMATION, L"Registration Successful", L"CLicense", DEVICE_ID, __LINE__); 
	return lReturn;

}


////////////////////////////////////////////////////////////////////////
// Function:	IsLicensed
// Description:	Checks to see if the unit is already licensed
// Author:		James Morley-Smith
// Date:		December 2004
////////////////////////////////////////////////////////////////////////
BOOL CLicense::IsLicensed()
{
//#define NEON_BETA_PROGRAM 1
#ifdef NEON_BETA_PROGRAM
	wcscpy(m_tcCompany, L"BETA RELEASE - NOT FOR RESALE");
	
	m_bIsLicensed = true;

#else
	//same process as RegisterUnit, near enough
	//PopulateUID();
	TCHAR tcCoName[31];
	TCHAR tcReg[51];
	
	memset(tcCoName, 0, sizeof(TCHAR)*31);
	memset(tcReg, 0, sizeof(TCHAR)*51);
	
	DWORD RetSize = 60;
	DWORD Type;
	HKEY hRegKey;


	if (ERROR_SUCCESS == RegOpenKeyEx(HKEY_CURRENT_USER, L"Software\\Symbol\\SymbolPB\\Licence", 0, 0, &hRegKey)) {
		RegQueryValueEx(hRegKey, _T("Company"), NULL, &Type, (BYTE *) tcCoName, &RetSize);
		RetSize = 70;
		RegQueryValueEx(hRegKey, _T("Code"), NULL, &Type, (BYTE *) tcReg, &RetSize);
		RegCloseKey(hRegKey);
	}
	
	int iChecksum = 0;
	for(UINT iCS=0; iCS<wcslen(tcCoName); iCS++)
		iChecksum+=tcCoName[iCS];
	
	if(iChecksum > 0xEE2)
		return FALSE;
	
	TCHAR tcDecrypted[50];
	
	DecryptPwd(tcReg, tcDecrypted);
	
	TCHAR tcCS[5];
	TCHAR tcLicenseKey[16];

	wsprintf(tcCS, L"%03X:", iChecksum);
	if(wcsncmp(tcCS, tcDecrypted, 4)){
		wcscpy(m_tcDIN, L"CORPERATELICENSE");
		if (ERROR_SUCCESS == RegOpenKeyEx(HKEY_CURRENT_USER, L"Software\\Symbol\\SymbolPB\\Licence", 0, 0, &hRegKey)) {
			RetSize = 70;
			RegQueryValueEx(hRegKey, _T("Code"), NULL, &Type, (BYTE *) tcReg, &RetSize);
		
			RegCloseKey(hRegKey);
		}
		DecryptPwd(tcReg, tcDecrypted);
		if(wcsncmp(tcCS, tcDecrypted, 4))
			return FALSE;

		wsprintf(tcLicenseKey, L"%s%d_%dO0", LIC_KEY, HIWORD(m_vsAPPVer.dwFileVersionMS), LOWORD(m_vsAPPVer.dwFileVersionMS),HIWORD(m_vsAPPVer.dwFileVersionLS));
	}
	else
		wsprintf(tcLicenseKey, L"%s%d_%d_0", LIC_KEY, HIWORD(m_vsAPPVer.dwFileVersionMS), LOWORD(m_vsAPPVer.dwFileVersionMS),HIWORD(m_vsAPPVer.dwFileVersionLS));

		
	if(wcscmp(&tcDecrypted[4],tcLicenseKey ))
		return FALSE;

	wcscpy(m_tcCompany, tcCoName);

	//CSpbLog::Log(MEDIUM,SPB_LOG_INFORMATION, L"Licensed to...", L"CLicense", DEVICE_ID, __LINE__); 
	//CSpbLog::Log(MEDIUM,SPB_LOG_INFORMATION, tcCoName, L"CLicense", DEVICE_ID, __LINE__);  
	
	m_bIsLicensed = true;
#endif
	return TRUE;
}



////////////////////////////////////////////////////////////////////////
// Function:	PopulateUID
// Description:	gets the UUID from the device
// Author:		James Morley-Smith
// Date:		December 2004
////////////////////////////////////////////////////////////////////////
void CLicense::PopulateUID()
{
	TCHAR serial[128];
	TCHAR tcUID[33];

	memset(serial, 0, 128);
	DWORD dwRes = 0;
	wcscpy(m_tcUID, L"GetSerialNumber: error!");	
	wcscpy(m_tcDIN, L"ERR_NOTSUPPORTED");	
	wcscpy(m_tcDINCpy, L"ERR_NOTSUPPORTED");

	BytesToHexStr(m_tcUID, m_UUID.byUUID, m_UUID.StructInfo.dwUsed - sizeof(STRUCT_INFO)); 
	TCHAR tc1[17], tc2[17];
	
	if(m_tcUID[16] == 0)
	{
		wcscpy(tc1, m_tcUID);
		wcscpy(&tc1[8], L"00000000");
		wcscpy(tc2, &m_tcUID[8]);
		wcscpy(&tc2[8], L"00000000");
		wcscpy(m_tcUID, tc1);
		wcscat(m_tcUID, tc2);
	}

	wsprintf(tcUID, TEXT("%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c"),
				m_tcUID[30],
				m_tcUID[31],
				m_tcUID[28],
				m_tcUID[29],
				m_tcUID[26],
				m_tcUID[27],
				m_tcUID[24],
				m_tcUID[25],
				m_tcUID[22],
				m_tcUID[23],
				m_tcUID[20],
				m_tcUID[21],
				m_tcUID[18],
				m_tcUID[19],
				m_tcUID[16],
				m_tcUID[17],
				m_tcUID[14],
				m_tcUID[15],
				m_tcUID[12],
				m_tcUID[13],
				m_tcUID[10],
				m_tcUID[11],
				m_tcUID[8],
				m_tcUID[9],
				m_tcUID[6],
				m_tcUID[7],
				m_tcUID[4],
				m_tcUID[5],
				m_tcUID[2],
				m_tcUID[3],
				m_tcUID[0],
				m_tcUID[1]);

	TCHAR tc641[17], tc642[17];
	wcsncpy(tc1, tcUID, 10);
	tc1[10] = NULL;
	HexToBase64(tc1, tc641);
	wcsncpy(tc2, &tcUID[10], 10);
	tc2[10] = NULL;
	HexToBase64(tc2, tc642);	
	wsprintf(m_tcDIN, L"%s%s", &tc641[3], &tc642[3]);
	wsprintf(m_tcDINCpy, L"%s%s", &tc641[3], &tc642[3]);
	

	HANDLE hFile = CreateFile(_T("\\DIN.txt"), GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	if (hFile != INVALID_HANDLE_VALUE) {
		char pcDIN[50];
		DWORD dwBytesWritten = 0;
		wcstombs(pcDIN, m_tcDIN, wcslen(m_tcDIN));
		pcDIN[wcslen(m_tcDIN)] = NULL;
		WriteFile(hFile, "Device D.I.N: ", strlen("Device D.I.N: "), &dwBytesWritten, NULL);
		WriteFile(hFile, pcDIN, strlen(pcDIN), &dwBytesWritten, NULL);
		
		CloseHandle(hFile);
	}


}


////////////////////////////////////////////////////////////////////////
// Function:	DecryptPwnd
// Description:	decrypts the license key
// Author:		James Morley-Smith
// Date:		December 2004
////////////////////////////////////////////////////////////////////////
void CLicense::DecryptPwd(TCHAR *ptcPwd, TCHAR *ptcDecrypted)
{
	long x;
	TCHAR tcHex[3];
	TCHAR tcChar;
	int x1 = 0;

	long l = wcslen(m_tcDIN);
	long l1 = wcslen(ptcPwd);


	//decode
	for(x=0; x < l1; x+=2){
		wcsncpy(tcHex, &ptcPwd[x], 2);
		tcHex[2] = NULL;
		ptcPwd[x1++] = (TCHAR)wcstol(tcHex, NULL, 16);
	}

	//decrypt
	TCHAR tcTemp[1000];
	ptcDecrypted[0] = NULL;
	for(x=0; x < x1; x++){
		tcChar = m_tcDIN[(x % l) - l * (int)((x+1 % l)==0)]; 
		wsprintf(tcTemp, _T("%c"), ptcPwd[x] ^ tcChar);
		wcscat(ptcDecrypted, tcTemp);
	}
}

////////////////////////////////////////////////////////////////////////
// Function:	Crc16Add
// Description:	part of the crc process
// Author:		James Morley-Smith
// Date:		December 2004
////////////////////////////////////////////////////////////////////////
TCHAR CLicense::Crc16Add(UINT crc, TCHAR c)
{
	return (TCHAR)(crc<<8)^CRC16Table[(TCHAR)(crc>>8)^c];
}


////////////////////////////////////////////////////////////////////////
// Function:	Crc16Str
// Description:	part of the crc process
// Author:		James Morley-Smith
// Date:		December 2004
////////////////////////////////////////////////////////////////////////
int CLicense::Crc16Str(const TCHAR * lpstr)
{
  int n;
  int len = wcslen(lpstr);
  int crc;

  crc=0;
  for (n=0; n<len; n++)
  {
    crc= (int)Crc16Add((UINT)crc,lpstr[n]);
  }
  return crc;
}


////////////////////////////////////////////////////////////////////////
// Function:	Step0
// Description:	The splash screen state
// Author:		James Morley-Smith
// Date:		December 2004
////////////////////////////////////////////////////////////////////////
void CLicense::DrawState_SplashScreen(HDC hdc, PAINTSTRUCT ps)
{
	//display the version information
	TCHAR tcVersion[100];
	DWORD dwRet = 12;
	wsprintf(tcVersion, L"Version %d.%d.%d.%d ", HIWORD(m_vsAPPVer.dwFileVersionMS), LOWORD(m_vsAPPVer.dwFileVersionMS),
		HIWORD(m_vsAPPVer.dwFileVersionLS),LOWORD(m_vsAPPVer.dwFileVersionLS));
	DrawText(tcVersion, hdc, TEXTCOLOR, TRUE, scaledpx(12), scaledpx(190), scaledpx(75));
	
	//if the unit is licened then display the licensee and set the timer to close the window automatically
	//else start the wizzard
	if(IsLicensed())
	{
		//Licensed to:
		DrawText(L"Licensed to:", hdc, TEXTCOLOR, FALSE, scaledpx(14), scaledpx(20), scaledpx(147), DT_LEFT);
		DrawText(m_tcCompany, hdc, TEXTCOLOR, TRUE, scaledpx(14), scaledpx(20), scaledpx(170), DT_LEFT);
		uTimer = SetTimer(m_hwnd, 1, 1500, NULL);
		m_iStep = -1; //end wizard
	}
	else 
	{
		//  Extra check here to resolve EMBPD00032196 and EMBPD30453
		if (!m_bIsLicensed)
		{
			DrawText(L"This is unlicensed software for", hdc, TEXTCOLOR, FALSE, scaledpx(14), scaledpx(20), scaledpx(147), DT_LEFT);
			DrawText(L"DEMONSTRATION USE ONLY", hdc, TEXTCOLOR, TRUE, scaledpx(14), scaledpx(20), scaledpx(168), DT_LEFT);
			DrawText(L"Press any key or tap the screen ", hdc, TEXTCOLOR, TRUE, scaledpx(12), scaledpx(20), scaledpx(190), DT_LEFT);
			DrawText(L"to license your software.", hdc, TEXTCOLOR, TRUE, scaledpx(12), scaledpx(20), scaledpx(203), DT_LEFT);
		}		
	}

	
	m_iCurTabStop = BUTTONLOGO;
	SetFocus(m_TabStops[m_iCurTabStop].hwnd);
}


////////////////////////////////////////////////////////////////////////
// Function:	DrawState_Options
// Description:	The option to auto register or not
// Author:		James Morley-Smith
// Date:		December 2004
////////////////////////////////////////////////////////////////////////
void CLicense::DrawState_Options(HDC hdc, PAINTSTRUCT ps)
{
	
	//prompt the user
	int iLP = 100;
	DrawText(L"If this device is connected to the internet", hdc, TEXTCOLOR, FALSE, scaledpx(12), scaledpx(20),  scaledpx(iLP),     DT_LEFT); //61
	DrawText(L"you may be able to automatically license   ", hdc, TEXTCOLOR, FALSE, scaledpx(12), scaledpx(20),  scaledpx(iLP+=13), DT_LEFT); //76
	DrawText(L"this unit online.", hdc, TEXTCOLOR, FALSE, scaledpx(12), scaledpx(20),  scaledpx(iLP+=13), DT_LEFT); //91

	iLP+=13;
	DrawText(L"- Press \"Internet\" if this device is connected. ", hdc, TEXTCOLOR, FALSE, scaledpx(11), scaledpx(20),  scaledpx(iLP+=12), DT_LEFT); //76
	DrawText(L"- Press \"Manual\" if this device is not connected", hdc, TEXTCOLOR, FALSE, scaledpx(11), scaledpx(20),  scaledpx(iLP+=12), DT_LEFT); //91
	
	SetFocus(m_TabStops[m_iCurTabStop].hwnd);	
}

////////////////////////////////////////////////////////////////////////
// Function:	Step2
// Description:	displays the uuid parts and checksome for the manual
//				process
// Author:		James Morley-Smith
// Date:		December 2004
////////////////////////////////////////////////////////////////////////
void CLicense::DrawState_Manual_UUID(HDC hdc, PAINTSTRUCT ps)
{
	int iLP = 90;

	DrawText(L"To license this unit you will need to", hdc, TEXTCOLOR, FALSE, scaledpx(12), scaledpx(20), scaledpx(iLP), DT_LEFT); //61
	DrawText(L"obtain a registration code from the", hdc, TEXTCOLOR, FALSE, scaledpx(12), scaledpx(20), scaledpx(iLP+=13), DT_LEFT); //76
	DrawText(L"Motorola software licensing Site. Below", hdc, TEXTCOLOR, FALSE, scaledpx(12), scaledpx(20), scaledpx(iLP+=13), DT_LEFT); //91
	DrawText(L"are the details you need to do this.", hdc, TEXTCOLOR, FALSE, scaledpx(12), scaledpx(20), scaledpx(iLP+=13), DT_LEFT); //106	
	
	DrawRegion(hdc, scaledpx(25), scaledpx(155), scaledpx(180), scaledpx(205),	LIGHTGRAY); 

	DrawText(L"UUID Part 1:", hdc, TEXTCOLOR, FALSE, scaledpx(11), scaledpx(30), scaledpx(iLP+=30), DT_LEFT); //136
	DrawText(L"UUID Part 2:", hdc, TEXTCOLOR, FALSE, scaledpx(11), scaledpx(30), scaledpx(iLP+=15), DT_LEFT); //151
	DrawText(L"Checksum:", hdc, TEXTCOLOR, FALSE, scaledpx(11), scaledpx(30), scaledpx(iLP+=15), DT_LEFT); //166

	TCHAR uuidP1[17], uuidP2[17];
	
	TCHAR checksum[5];
	wcsncpy(uuidP1, m_tcUID, 16);
	uuidP1[16] = NULL;
	wcscpy(uuidP2, &m_tcUID[16]);
	
	int crc = Crc16Str(m_tcUID);

	wsprintf(checksum, L"%04X", crc);

	DrawText(uuidP1, hdc, TEXTCOLOR, TRUE, scaledpx(11), scaledpx(85), scaledpx(iLP-=30), DT_LEFT); //136
	DrawText(uuidP2, hdc, TEXTCOLOR, TRUE, scaledpx(11), scaledpx(85), scaledpx(iLP+=15), DT_LEFT); //151
	DrawText(checksum, hdc, TEXTCOLOR, TRUE, scaledpx(11), scaledpx(85), scaledpx(iLP+=15), DT_LEFT); //166	

	//Status
	DrawText(L"1 / 4", hdc, TEXTCOLOR, TRUE, scaledpx(12), scaledpx(117), scaledpx(225), DT_CENTER);	
}

////////////////////////////////////////////////////////////////////////
// Function:	Step3
// Description:	Get the company name step
// Author:		James Morley-Smith
// Date:		December 2004
////////////////////////////////////////////////////////////////////////
void CLicense::DrawState_Manual_COName(HDC hdc, PAINTSTRUCT ps)
{
	
	
	int iLP = 100;

	DrawText(L"Scan the company name barcode you", hdc, TEXTCOLOR, FALSE, scaledpx(12), scaledpx(20), scaledpx(iLP), DT_LEFT); //61
	DrawText(L"obtained from the licensing site or", hdc, TEXTCOLOR, FALSE, scaledpx(12), scaledpx(20), scaledpx(iLP+=13), DT_LEFT); //76
	DrawText(L"enter the code manually.", hdc, TEXTCOLOR, FALSE, scaledpx(12), scaledpx(20), scaledpx(iLP+=13), DT_LEFT); //91
		
	if(m_iPreStep == STATE_MANUAL_CODE){
		GetWindowText(m_hwndInput, m_tcCode, 50);
		SetWindowText(m_hwndInput, m_tcCompany);
		SendMessage(m_hwndInput, EM_SETSEL, wcslen(m_tcCompany), wcslen(m_tcCompany));
		SendMessage(m_hwndInput, EM_LIMITTEXT, 30, 0);
		m_iPreStep = m_iStep;
	}
	else if(m_iPreStep == STATE_MANUAL_UUID){
		SetWindowText(m_hwndInput, m_tcCompany);
		SendMessage(m_hwndInput, EM_SETSEL, wcslen(m_tcCompany), wcslen(m_tcCompany));
		SendMessage(m_hwndInput, EM_LIMITTEXT, 30, 0);
		m_iPreStep = m_iStep;
	}
	
	DrawText(L"2 / 4", hdc, TEXTCOLOR, TRUE, scaledpx(12), scaledpx(117), scaledpx(225), DT_CENTER);	
}


////////////////////////////////////////////////////////////////////////
// Function:	Step4
// Description:	Get the registration code step
// Author:		James Morley-Smith
// Date:		December 2004
////////////////////////////////////////////////////////////////////////
void CLicense::DrawState_Manual_Code(HDC hdc, PAINTSTRUCT ps)
{
	int iLP = 100;
	
	DrawText(L"Scan the registration barcode you", hdc, TEXTCOLOR, FALSE, scaledpx(12), scaledpx(20), scaledpx(iLP), DT_LEFT); //61
	DrawText(L"obtained from the licensing site or", hdc, TEXTCOLOR, FALSE, scaledpx(12), scaledpx(20), scaledpx(iLP+=13), DT_LEFT); //76
	DrawText(L"enter the code manually.", hdc, TEXTCOLOR, FALSE, scaledpx(12), scaledpx(20), scaledpx(iLP+=13), DT_LEFT); //91
		
	if(m_iPreStep == STATE_MANUAL_CONAME){
		GetWindowText(m_hwndInput, m_tcCompany, 30);
		SetWindowText(m_hwndInput, m_tcCode);
		SendMessage(m_hwndInput, EM_SETSEL, wcslen(m_tcCode), wcslen(m_tcCode)); 
		SendMessage(m_hwndInput, EM_LIMITTEXT, 50, 0);
		
		m_iPreStep = m_iStep;
	}
	else if(m_iPreStep == STATE_MANUAL_RESULT){
		SetWindowText(m_hwndInput, m_tcCode);
		SendMessage(m_hwndInput, EM_SETSEL, wcslen(m_tcCode), wcslen(m_tcCode));
		SendMessage(m_hwndInput, EM_LIMITTEXT, 50, 0);
		
		m_iPreStep = m_iStep;
	}
	
	DrawText(L"3 / 4", hdc, TEXTCOLOR, TRUE, scaledpx(12), scaledpx(117), scaledpx(225), DT_CENTER);
}

////////////////////////////////////////////////////////////////////////
// Function:	Step5
// Description:	Show the result
// Author:		James Morley-Smith
// Date:		December 2004
////////////////////////////////////////////////////////////////////////
void CLicense::DrawState_Manual_Result(HDC hdc, PAINTSTRUCT ps)
{
	
	
	int iLP = 100;

	DrawRegion(hdc, scaledpx(20), scaledpx(145), scaledpx(210), scaledpx(185),	LIGHTGRAY); 

	if(!m_lRegResult){
		//registration was successful	
	
		DrawText(L"Registration Successful.", hdc, TEXTCOLOR, TRUE, scaledpx(12), scaledpx(20), scaledpx(iLP), DT_LEFT); //61
		DrawText(L"Licensed to:", hdc, TEXTCOLOR, FALSE, scaledpx(11), scaledpx(30), scaledpx(iLP+=50), DT_LEFT); 
		DrawText(m_tcCompany, hdc, TEXTCOLOR, TRUE, scaledpx(11), scaledpx(100), scaledpx(iLP), DT_LEFT); //136
	
	}
	else{
		//registration failed		

		switch (m_lRegResult){
		case 1:
			DrawText(L"Registration Failed!", hdc, TEXTCOLOR, TRUE, scaledpx(12), scaledpx(20), scaledpx(iLP), DT_LEFT); //61
			DrawText(L"Company name does not match", hdc, TEXTCOLOR, FALSE, scaledpx(11), scaledpx(30), scaledpx(iLP+=50), DT_LEFT); //91
			DrawText(L"the registration code.", hdc, TEXTCOLOR, FALSE, scaledpx(11), scaledpx(30), scaledpx(iLP+=12), DT_LEFT); //106
			break;
		
		case 2:
			DrawText(L"Registration Failed!", hdc, TEXTCOLOR, TRUE, scaledpx(12), scaledpx(20), scaledpx(iLP), DT_LEFT); //61
			DrawText(L"Invalid registration code.", hdc, TEXTCOLOR, FALSE, scaledpx(11), scaledpx(30), scaledpx(iLP+=50), DT_LEFT); //91
			break;
		}
	}
	DrawText(L"4 / 4", hdc, TEXTCOLOR, TRUE, scaledpx(12), scaledpx(117), scaledpx(225), DT_CENTER);	
}

////////////////////////////////////////////////////////////////////////
// Function:	StepA
// Description:	Get the serial number
// Author:		James Morley-Smith
// Date:		December 2004
////////////////////////////////////////////////////////////////////////
void CLicense::DrawState_Web_Serial(HDC hdc, PAINTSTRUCT ps)
{
	int iLP = 100;

	DrawText(L"Key in the device Serial Number.", hdc, TEXTCOLOR, FALSE, scaledpx(12), scaledpx(20), scaledpx(iLP), DT_LEFT); //61
	DrawText(L"This is usually printed on the back", hdc, TEXTCOLOR, FALSE, scaledpx(12), scaledpx(20), scaledpx(iLP+=13), DT_LEFT); //76
	DrawText(L"of the device or under the battery.", hdc, TEXTCOLOR, FALSE, scaledpx(12), scaledpx(20), scaledpx(iLP+=13), DT_LEFT); //91
		
	if(m_iPreStep == STATE_WEB_ORDER){
		GetWindowText(m_hwndInput, m_tcOrder, 20);
		SetWindowText(m_hwndInput, m_tcSerial);
		SendMessage(m_hwndInput, EM_SETSEL, wcslen(m_tcSerial), wcslen(m_tcSerial));
		SendMessage(m_hwndInput, EM_LIMITTEXT, 20, 0);
		m_iPreStep = m_iStep;
	}
	else if(m_iPreStep == -1){
		SetWindowText(m_hwndInput, m_tcSerial);
		SendMessage(m_hwndInput, EM_SETSEL, wcslen(m_tcSerial), wcslen(m_tcSerial));
		SendMessage(m_hwndInput, EM_LIMITTEXT, 20, 0);
		m_iPreStep = m_iStep;
	}
	
	DrawText(L"1 / 4", hdc, TEXTCOLOR, TRUE, scaledpx(12), scaledpx(117), scaledpx(225), DT_CENTER);	
}

////////////////////////////////////////////////////////////////////////
// Function:	StepB
// Description:	Get the order number step
// Author:		James Morley-Smith
// Date:		December 2004
////////////////////////////////////////////////////////////////////////
void CLicense::DrawState_Web_Order(HDC hdc, PAINTSTRUCT ps)
{

	int iLP = 100;
	
	DrawText(L"Key in your order number that you", hdc, TEXTCOLOR, FALSE, scaledpx(12), scaledpx(20), scaledpx(iLP), DT_LEFT); //61
	DrawText(L"obtained when you purchased", hdc, TEXTCOLOR, FALSE, scaledpx(12), scaledpx(20), scaledpx(iLP+=13), DT_LEFT); //76
	DrawText(L"your licenses.", hdc, TEXTCOLOR, FALSE, scaledpx(12), scaledpx(20), scaledpx(iLP+=13), DT_LEFT); //91
	
	if(m_iPreStep == STATE_WEB_SERIAL){
		GetWindowText(m_hwndInput, m_tcSerial, 20);
		SetWindowText(m_hwndInput, m_tcOrder);
		SendMessage(m_hwndInput, EM_SETSEL, wcslen(m_tcOrder), wcslen(m_tcOrder)); 
		SendMessage(m_hwndInput, EM_LIMITTEXT, 20, 0);
		
		m_iPreStep = m_iStep;
	}
	else if(m_iPreStep == STATE_WEB_RESULT){
		SetWindowText(m_hwndInput, m_tcOrder);
		SendMessage(m_hwndInput, EM_SETSEL, wcslen(m_tcOrder), wcslen(m_tcOrder));
		SendMessage(m_hwndInput, EM_LIMITTEXT, 20, 0);
		
		m_iPreStep = m_iStep;
	}
	
	DrawText(L"2 / 4", hdc, TEXTCOLOR, TRUE, scaledpx(12), scaledpx(117), scaledpx(225), DT_CENTER);
	
}

////////////////////////////////////////////////////////////////////////
// Function:	StepC
// Description:	Show the result
// Author:		James Morley-Smith
// Date:		December 2004
////////////////////////////////////////////////////////////////////////
void CLicense::DrawState_Web_Result(HDC hdc, PAINTSTRUCT ps)
{
	
	int iLP = 100;

	DrawRegion(hdc, scaledpx(20), scaledpx(145), scaledpx(210), scaledpx(185),	LIGHTGRAY); 

	TCHAR szLic[500];
	
	if(!m_lRegResult){
		//registration successful		
	
		DrawText(L"Registration Successful.", hdc, TEXTCOLOR, TRUE, scaledpx(12), scaledpx(20), scaledpx(iLP), DT_LEFT); //61
		DrawText(L"Licensed to:", hdc, TEXTCOLOR, FALSE, scaledpx(11), scaledpx(30), scaledpx(iLP+=50), DT_LEFT); 
		DrawText(m_tcCompany, hdc, TEXTCOLOR, TRUE, scaledpx(11), scaledpx(100), scaledpx(iLP), DT_LEFT); //136
	
	}
	else{
		
		//registration failed

		switch (m_lRegResult){
		case -1: case -2:
			wsprintf(szLic, L"Error: %s (%s)", m_szWebError, m_szWebErrCode);
			
			DrawText(L"Registration Failed!", hdc, TEXTCOLOR, TRUE, scaledpx(12), scaledpx(20), scaledpx(iLP), DT_LEFT); //61
			DrawText(L"An error occured.", hdc, TEXTCOLOR, FALSE, scaledpx(11), scaledpx(30), scaledpx(iLP+=50), DT_LEFT); //91
			DrawText(szLic, hdc, TEXTCOLOR, FALSE, scaledpx(11), scaledpx(30), scaledpx(iLP+=12), DT_LEFT); //106
			break;
			break;
		
		case 1:
			DrawText(L"Registration Failed!", hdc, TEXTCOLOR, TRUE, scaledpx(12), scaledpx(20), scaledpx(iLP), DT_LEFT); //61
			DrawText(L"Company name does not match", hdc, TEXTCOLOR, FALSE, scaledpx(11), scaledpx(30), scaledpx(iLP+=50), DT_LEFT); //91
			DrawText(L"the registration code.", hdc, TEXTCOLOR, FALSE, scaledpx(11), scaledpx(30), scaledpx(iLP+=12), DT_LEFT); //106
			break;
		
		case 2:
			DrawText(L"Registration Failed!", hdc, TEXTCOLOR, TRUE, scaledpx(12), scaledpx(30), scaledpx(iLP), DT_LEFT); //61
			DrawText(L"Invalid registration code.", hdc, TEXTCOLOR, FALSE, scaledpx(11), scaledpx(30), scaledpx(iLP+=50), DT_LEFT); //91
			break;
		}
	}
}

////////////////////////////////////////////////////////////////////////
// Function:	StepC
// Description:	Show the result
// Author:		James Morley-Smith
// Date:		December 2004
////////////////////////////////////////////////////////////////////////
void CLicense::DrawState_Web_Communicating(HDC hdc, PAINTSTRUCT ps)
{
	
	int iLP = 100;
	DrawText(L"Communicating with server", hdc, TEXTCOLOR, TRUE, scaledpx(12), scaledpx(20), scaledpx(iLP), DT_LEFT); //61
	DrawText(L"Please wait...", hdc, TEXTCOLOR, FALSE, scaledpx(11), scaledpx(20), scaledpx(iLP+=15), DT_LEFT); //106

	DrawText(L"3 / 4", hdc, TEXTCOLOR, TRUE, scaledpx(12), scaledpx(117), scaledpx(225), DT_CENTER);	
	
}

////////////////////////////////////////////////////////////////////////
// Function:	Step0
// Description:	The splash screen state
// Author:		James Morley-Smith
// Date:		December 2004
////////////////////////////////////////////////////////////////////////
void CLicense::SWState_SplashScreen()
{
	ShowWindow(m_hwndLogo, SW_SHOW);

	CallIMOMethod(m_ScannerIMO, L"disable");
	//SendMessage(m_hwndLogo, WM_LBUTTONDOWN, 1, 0xA0009);
	//SendMessage(m_hwndLogo, WM_LBUTTONUP, 0, 0x5000B);
			
}


////////////////////////////////////////////////////////////////////////
// Function:	SWState_Options
// Description:	The option to auto register or not
// Author:		James Morley-Smith
// Date:		December 2004
////////////////////////////////////////////////////////////////////////
void CLicense::SWState_Options()
{
	ShowWindow(m_hwndKeyboard, SW_HIDE);
	ShowWindow(m_hwndInput, SW_HIDE);
	ShowWindow(m_hwndCancel, SW_SHOW);
	ShowWindow(m_hwndBack, SW_HIDE);
	ShowWindow(m_hwndWeb, SW_SHOW);
	ShowWindow(m_hwndTick, SW_HIDE);
	ShowWindow(m_hwndManual, SW_SHOW);
	ShowWindow(m_hwndNext, SW_HIDE);

	m_iCurTabStop = BUTTONMANUAL;
	SetFocus(m_TabStops[m_iCurTabStop].hwnd);	



	if(m_ScannerIMO == NULL)
		m_ScannerIMO = CreateIMO(&m_pbStruct, L"scanner", L"License Wizard");

	if(m_ScannerIMO == NULL)
	{
		//NEED TO GET TO THE BOTTOM OF THIS, BUT THE FIRST TIME WE CREATE THE SCANNER IMO
		//IT FAILS, SO WILL TRY AGAIN AFTER A LITTLE REST!!
		//THIS ONLY HAPPENS ON THE FIRST ATTEMPT!!
		Sleep(200);
		m_ScannerIMO = CreateIMO(&m_pbStruct, L"scanner", L"License Wizard");
	}


	if(!m_ScannerIMO)
	{
		Log(PB_LOG_ERROR, L"Could not create Scanner IMO", _T(__FUNCTION__), __LINE__, L"License Wizard");
	}
	else
	{
		SetIMOProperty(m_ScannerIMO, L"decodeevent", L"Javascript://IMO");
		SetIMOProperty(m_ScannerIMO, L"all_decoders", L"disable");
		SetIMOProperty(m_ScannerIMO, L"Code128", L"enabled");
		SetIMOCallBack(m_ScannerIMO, DecodeBarcodeEvent, (LPARAM)this);
	}

	CallIMOMethod(m_ScannerIMO, L"disable");
}

////////////////////////////////////////////////////////////////////////
// Function:	Step2
// Description:	displays the uuid parts and checksome for the manual
//				process
// Author:		James Morley-Smith
// Date:		December 2004
////////////////////////////////////////////////////////////////////////
void CLicense::SWState_Manual_UUID()
{
	ShowWindow(m_hwndKeyboard, SW_HIDE);
	ShowWindow(m_hwndInput, SW_HIDE);
	ShowWindow(m_hwndCancel, SW_SHOW);
	ShowWindow(m_hwndWeb, SW_HIDE);
	ShowWindow(m_hwndManual, SW_HIDE);
	ShowWindow(m_hwndBack, SW_SHOW);
	ShowWindow(m_hwndTick, SW_HIDE);
	ShowWindow(m_hwndNext, SW_SHOW);
	m_iCurTabStop = BUTTONNEXT;
	SetFocus(m_TabStops[m_iCurTabStop].hwnd);	

	CallIMOMethod(m_ScannerIMO, L"disable");
}

////////////////////////////////////////////////////////////////////////
// Function:	Step3
// Description:	Get the company name step
// Author:		James Morley-Smith
// Date:		December 2004
////////////////////////////////////////////////////////////////////////
void CLicense::SWState_Manual_COName()
{
	MoveWindow(m_hwndKeyboard, scaledpx(175), scaledpx(150), scaledpx(40), scaledpx(20), FALSE);
	ShowWindow(m_hwndKeyboard, SW_SHOW);
	ShowWindow(m_hwndInput, SW_SHOW);
	ShowWindow(m_hwndCancel, SW_SHOW);
	ShowWindow(m_hwndWeb, SW_HIDE);
	ShowWindow(m_hwndManual, SW_HIDE);
	ShowWindow(m_hwndBack, SW_SHOW);
	ShowWindow(m_hwndTick, SW_HIDE);
	ShowWindow(m_hwndNext, SW_SHOW);
//	m_iCurTabStop = BUTTONINPUT;
//	SetFocus(m_TabStops[m_iCurTabStop].hwnd);

	CallIMOMethod(m_ScannerIMO, L"enabled");
	
}


////////////////////////////////////////////////////////////////////////
// Function:	Step4
// Description:	Get the registration code step
// Author:		James Morley-Smith
// Date:		December 2004
////////////////////////////////////////////////////////////////////////
void CLicense::SWState_Manual_Code()
{
	MoveWindow(m_hwndKeyboard, scaledpx(175), scaledpx(150), scaledpx(40), scaledpx(20), FALSE);
	ShowWindow(m_hwndKeyboard, SW_SHOW);
	ShowWindow(m_hwndInput, SW_SHOW);
	ShowWindow(m_hwndCancel, SW_SHOW);
	ShowWindow(m_hwndWeb, SW_HIDE);
	ShowWindow(m_hwndManual, SW_HIDE);
	ShowWindow(m_hwndBack, SW_SHOW);
	ShowWindow(m_hwndTick, SW_HIDE);
	ShowWindow(m_hwndNext, SW_SHOW);
//	m_iCurTabStop = BUTTONINPUT;
//	SetFocus(m_TabStops[m_iCurTabStop].hwnd);

	CallIMOMethod(m_ScannerIMO, L"enabled");
	

}

////////////////////////////////////////////////////////////////////////
// Function:	Step5
// Description:	Show the result
// Author:		James Morley-Smith
// Date:		December 2004
////////////////////////////////////////////////////////////////////////
void CLicense::SWState_Manual_Result()
{
	ShowWindow(m_hwndKeyboard, SW_HIDE);
	ShowWindow(m_hwndInput, SW_HIDE);
	ShowWindow(m_hwndNext, SW_HIDE);

	CallIMOMethod(m_ScannerIMO, L"disable");

	if(m_iPreStep == STATE_MANUAL_CODE){
		GetWindowText(m_hwndInput, m_tcCode, 50);
		m_iPreStep = m_iStep;
		m_lRegResult = RegisterUnit();
		MessageBeep((m_lRegResult?MB_ICONHAND:MB_ICONASTERISK));
	}

	if(!m_lRegResult){
		//registration was successful
		ShowWindow(m_hwndCancel, SW_HIDE);
		ShowWindow(m_hwndWeb, SW_HIDE);
		ShowWindow(m_hwndManual, SW_HIDE);
		ShowWindow(m_hwndBack, SW_HIDE);
		ShowWindow(m_hwndTick, SW_SHOW);		
		m_iCurTabStop = BUTTONTICK;
		SetFocus(m_TabStops[m_iCurTabStop].hwnd);
	}
	else{
		//registration failed
		ShowWindow(m_hwndCancel, SW_SHOW);
		ShowWindow(m_hwndWeb, SW_HIDE);
		ShowWindow(m_hwndManual, SW_HIDE);
		ShowWindow(m_hwndBack, SW_SHOW);
		ShowWindow(m_hwndTick, SW_HIDE);
	   	m_iCurTabStop = BUTTONBACK;
		SetFocus(m_TabStops[m_iCurTabStop].hwnd);
	}
}

////////////////////////////////////////////////////////////////////////
// Function:	StepA
// Description:	Get the serial number
// Author:		James Morley-Smith
// Date:		December 2004
////////////////////////////////////////////////////////////////////////
void CLicense::SWState_Web_Serial()
{
	
	MoveWindow(m_hwndKeyboard, scaledpx(175), scaledpx(150), scaledpx(40), scaledpx(20), FALSE);
	ShowWindow(m_hwndKeyboard, SW_SHOW);
	ShowWindow(m_hwndInput, SW_SHOW);
	ShowWindow(m_hwndCancel, SW_SHOW);
	ShowWindow(m_hwndWeb, SW_HIDE);
	ShowWindow(m_hwndManual, SW_HIDE);
	ShowWindow(m_hwndBack, SW_SHOW);
	ShowWindow(m_hwndTick, SW_HIDE);
	ShowWindow(m_hwndNext, SW_SHOW);
	m_iCurTabStop = BUTTONINPUT;
	SetFocus(m_TabStops[m_iCurTabStop].hwnd);

	CallIMOMethod(m_ScannerIMO, L"enabled");
}

////////////////////////////////////////////////////////////////////////
// Function:	StepB
// Description:	Get the order number step
// Author:		James Morley-Smith
// Date:		December 2004
////////////////////////////////////////////////////////////////////////
void CLicense::SWState_Web_Order()
{

	MoveWindow(m_hwndKeyboard, scaledpx(175), scaledpx(150), scaledpx(40), scaledpx(20), FALSE);
	ShowWindow(m_hwndKeyboard, SW_SHOW);
	ShowWindow(m_hwndInput, SW_SHOW);
	ShowWindow(m_hwndCancel, SW_SHOW);
	ShowWindow(m_hwndWeb, SW_HIDE);
	ShowWindow(m_hwndManual, SW_HIDE);
	ShowWindow(m_hwndBack, SW_SHOW);
	ShowWindow(m_hwndTick, SW_HIDE);
	ShowWindow(m_hwndNext, SW_SHOW);
	m_iCurTabStop = BUTTONINPUT;
	SetFocus(m_TabStops[m_iCurTabStop].hwnd);

	CallIMOMethod(m_ScannerIMO, L"enabled");
}

////////////////////////////////////////////////////////////////////////
// Function:	StepC
// Description:	Show the result
// Author:		James Morley-Smith
// Date:		December 2004
////////////////////////////////////////////////////////////////////////
void CLicense::SWState_Web_Result()
{
	ShowWindow(m_hwndKeyboard, SW_HIDE);
	ShowWindow(m_hwndInput, SW_HIDE);
	ShowWindow(m_hwndNext, SW_HIDE);

	CallIMOMethod(m_ScannerIMO, L"disable");

	if(!m_lRegResult){
		//registration successful
		ShowWindow(m_hwndCancel, SW_HIDE);
		ShowWindow(m_hwndWeb, SW_HIDE);
		ShowWindow(m_hwndManual, SW_HIDE);
		ShowWindow(m_hwndBack, SW_HIDE);
		ShowWindow(m_hwndTick, SW_SHOW);

		m_iCurTabStop = BUTTONTICK;
		SetFocus(m_TabStops[m_iCurTabStop].hwnd);
	}
	else{
		//registration failed
		ShowWindow(m_hwndCancel, SW_SHOW);
		ShowWindow(m_hwndWeb, SW_HIDE);
		ShowWindow(m_hwndManual, SW_HIDE);
		ShowWindow(m_hwndBack, SW_SHOW);
		ShowWindow(m_hwndTick, SW_HIDE);

		m_iCurTabStop = BUTTONBACK;
		SetFocus(m_TabStops[m_iCurTabStop].hwnd);
	}
		
}

////////////////////////////////////////////////////////////////////////
// Function:	StepC
// Description:	Show the result
// Author:		James Morley-Smith
// Date:		December 2004
////////////////////////////////////////////////////////////////////////
void CLicense::SWState_Web_Communicating()
{
	ShowWindow(m_hwndKeyboard, SW_HIDE);
	ShowWindow(m_hwndInput, SW_HIDE);
	ShowWindow(m_hwndNext, SW_HIDE);
	ShowWindow(m_hwndBack, SW_HIDE);
	ShowWindow(m_hwndCancel, SW_HIDE);

	PostMessage(m_hwnd, UM_GETLIC, 0, 0);

	CallIMOMethod(m_ScannerIMO, L"disable");
		
}

int CLicense::DecodeBarcodeEvent(PVARSTRUCT pVars,int iTABID,LPARAM lParam)
{
	CLicense* pLic = (CLicense*) lParam;

	pLic->SetInputText(pVars->pStr);

	SendMessage(pLic->m_hwndNext, WM_LBUTTONDOWN, 1, 0xA0009);
	SendMessage(pLic->m_hwndNext, WM_LBUTTONUP, 0, 0x5000B);
	
	return 1;
}

////////////////////////////////////////////////////////////////////////
// Function:	ShowSplash
// Description:	Shows the splash screen
// Author:		James Morley-Smith
// Date:		December 2004
////////////////////////////////////////////////////////////////////////
void CLicense::ShowSplash()
{
	//if the hwnd is null then the unit is licensed so just return
	if(!m_hwnd) return;

	//  SR EMBPD00029851, hide the SIP when the license screen is displayed
	m_pSIP->HideSIP(m_hwnd, false);

	//SipShowIM(SIPF_OFF);
	//SHSipPreference(m_hwnd, SIP_FORCEDOWN);
	m_iStep = 0;
	m_iPreStep = -1;
	m_bLicenseScreenVisible = true;
	//PopulateUID();
	ShowWindow(m_hwndCancel, SW_HIDE);
	ShowWindow(m_hwndWeb, SW_HIDE);
	ShowWindow(m_hwndManual, SW_HIDE);
	ShowWindow(m_hwndBack, SW_HIDE);
	ShowWindow(m_hwndNext, SW_HIDE);
	ShowWindow(m_hwndTick, SW_HIDE);
	ShowWindow(m_hwndInput, SW_HIDE);
	ShowWindow(m_hwndKeyboard, SW_HIDE);

	EnableWindow(m_ParentHwnd, FALSE);

	BrowserRestore(m_iAppID, L"License Wizard");
	g_pAppManager->SwitchAppInst(m_iAppID);
	//g_pEventSync->Run(PB_APPFOCUSEVENT, 0, m_iAppID);

	m_pbStruct.bInvoked = TRUE;
	m_pbStruct.hInstance = m_hInstance;
	m_pbStruct.hWnd = m_hwnd;
	m_pbStruct.iTabID = m_iAppID;

	ShowWindow(m_hwnd, SW_SHOW);
	SetForegroundWindow(m_hwnd);
	SetFocus(m_TabStops[m_iCurTabStop].hwnd);
	
	InvalidateRect(m_hwnd, NULL, TRUE);
	UpdateWindow(m_hwnd);
	
	//CSpbLog::Log(LOW,SPB_LOG_INFORMATION,L"Shown splash screen", L"CLicense", DEVICE_ID, __LINE__); 
}




////////////////////////////////////////////////////////////////////////
// Function:	SetInputtext
// Description:	Sets the input text of the wizzard
// Author:		James Morley-Smith
// Date:		December 2004
////////////////////////////////////////////////////////////////////////
void CLicense::SetInputText(LPCTSTR tcText)
{
	SetWindowText(m_hwndInput, tcText);
}

////////////////////////////////////////////////////////////////////////
// Function:	Cancel
// Description:	Cancels the license screen
// Author:		James Morley-Smith
// Date:		December 2004
////////////////////////////////////////////////////////////////////////
void CLicense::Cancel()
{
	Tick ();

	SetTimer(m_hwnd, 2, 300000 /*5 minutes */, NULL);
	//SetTimer(m_hwnd, 2, 10000 /*5 minutes */, NULL);

}

////////////////////////////////////////////////////////////////////////
// Function:	Tick
// Description:	Cancels the license screen after tick is clicked
// Author:		James Morley-Smith
// Date:		May 2010
////////////////////////////////////////////////////////////////////////
void CLicense::Tick()
{
	//  Hide the SIP (fixes issue where SIP is still present when PB NOT run 
	//  from debugger.
	HideSIPButton();

	g_pAppManager->SwitchAppInst(g_pPBCore->iPreviousInstID);
	g_pEventSync->Run(PB_APPFOCUSEVENT,m_iAppID,0);

	SetForegroundWindow(m_ParentHwnd);
	ShowWindow(m_hwnd, SW_HIDE);
	m_bLicenseScreenVisible = false;
	EnableWindow(m_ParentHwnd, TRUE);
	SetForegroundWindow(m_ParentHwnd);

	CallIMOMethod(m_ScannerIMO, L"disable");
//	if (m_ScannerIMO)
//		DeleteIMO(m_ScannerIMO);
//	m_ScannerIMO = NULL;
}

////////////////////////////////////////////////////////////////////////
// Function:	ResizeSplash
// Description:	Refreshes the sizes of the windows if the screen rotates
// Author:		James Morley-Smith
// Date:		March 2005
////////////////////////////////////////////////////////////////////////
void CLicense::ResizeSplash()
{

	int iSW = GetSystemMetrics(SM_CXSCREEN);

	m_iH = GetSystemMetrics(SM_CYSCREEN);
		
	MoveWindow(m_hwnd, scaledpx(6), scaledpx(16), scaledpx(228), scaledpx(278), FALSE);
	MoveWindow(m_hwndKeyboard, scaledpx(187), scaledpx(181), scaledpx(20), scaledpx(20), FALSE);
	MoveWindow(m_hwndCancel, scaledpx(41), scaledpx(200), scaledpx(32), scaledpx(32), FALSE);
	MoveWindow(m_hwndBack, scaledpx(130), scaledpx(200), scaledpx(32), scaledpx(32), FALSE);
	MoveWindow(m_hwndWeb, scaledpx(130), scaledpx(200), scaledpx(32), scaledpx(32), FALSE);
	MoveWindow(m_hwndManual, scaledpx(187), scaledpx(200), scaledpx(32), scaledpx(32), FALSE);
	MoveWindow(m_hwndNext, scaledpx(187), scaledpx(200), scaledpx(32), scaledpx(32), FALSE);
	MoveWindow(m_hwndTick, scaledpx(187), scaledpx(200), scaledpx(32), scaledpx(32), FALSE);
	MoveWindow(m_hwndInput, scaledpx(10), scaledpx(151), scaledpx(182), scaledpx(22), FALSE);

}

int CLicense::GetStepNo()
{
	return m_iStep;
}




/////////////////////////////////////////////////////////////////////////////////////////////////
//
//	returns TRUE if the OEM string matches any that are defined in License.h --- 'ExemptStrs' 
//
////////////////////////////////////////////////////////////////////////////////////////////////
BOOL CLicense::DontShow()
{
	TCHAR szPlatform[MAX_PATH+1];
	memset(szPlatform, 0, MAX_PATH*sizeof(TCHAR));
	SystemParametersInfo(SPI_GETOEMINFO, MAX_PATH, szPlatform, 0);
	_wcsupr(szPlatform);
	int loops;
	for(loops = 0;loops < dim(ExemptOEM);loops++){
		if(wcsstr(szPlatform,ExemptOEM[loops]) != NULL)
		{
			if(m_hwnd)
			{
				DestroyWindow(m_hwnd);
			}
			m_bLicenseScreenVisible = false;
			m_hwnd = NULL;
			return TRUE;
		}
	}
	return FALSE;
	
}


////////////////////////////////////////////////////////////////////////
// Function:	LoadRCMLib
// Description:	Loads the RCM library and gets pointers to the required
//				functions
// Authors:		Dave Sheppard - James MorleySmith
// Date:		11/1/2005
////////////////////////////////////////////////////////////////////////
bool CLicense::LoadRCMLib()
{
	//link to the keyboard dll for the alpha key
	m_hRCMLib = LoadLibrary(TEXT("Rcm2API32.dll"));
	if (m_hRCMLib != NULL)
	{
		lpfnRCM_GetUniqueUnitIdEx = (RCM_GETUNIQUEUNITIDEX)GetProcAddress(m_hRCMLib,L"RCM_GetUniqueUnitIdEx");
		memset(&m_UUID, 0, sizeof m_UUID);
		m_UUID.StructInfo.dwAllocated = sizeof m_UUID;
		DWORD dwRes = (*lpfnRCM_GetUniqueUnitIdEx) (&m_UUID);
		if( dwRes == E_RCM_NOTSUPPORTED )
		{
			FreeLibrary (m_hRCMLib);
			m_hRCMLib = LoadLibrary(TEXT("RcmAPI32.dll"));
			if (m_hRCMLib != NULL)
			{
				lpfnRCM_GetUniqueUnitIdEx = (RCM_GETUNIQUEUNITIDEX)GetProcAddress(m_hRCMLib,L"RCM_GetUniqueUnitIdEx");
				if(	lpfnRCM_GetUniqueUnitIdEx )
				{
					DWORD dwRes = (*lpfnRCM_GetUniqueUnitIdEx) (&m_UUID);
					if( dwRes != E_RCM_SUCCESS )
					{
						FreeLibrary (m_hRCMLib);
						return false;
					}
				}
				else
				{
					FreeLibrary (m_hRCMLib);
					return false;
				}
			}

		}
		FreeLibrary (m_hRCMLib);
		return true;
	}
	
	return false;
}

/////////////////////////////////////////////////////////////////////////////////////////////////
//
//	returns TRUE if the OEM string matches any that are defined in License.h --- 'ExemptStrs' 
//
////////////////////////////////////////////////////////////////////////////////////////////////
BOOL CLicense::IsSymbolDevice()
{
	TCHAR szPlatform[MAX_PATH+1];
	memset(szPlatform, 0, MAX_PATH*sizeof(TCHAR));
	SystemParametersInfo(SPI_GETOEMINFO, MAX_PATH, szPlatform, 0);
	_wcsupr(szPlatform);

	int loops;

	for(loops = 0;loops < dim(ValidOEM);loops++){
		if(wcsstr(szPlatform,ValidOEM[loops]) != NULL)
		{
			break;
		}
		
	}

	if(loops < dim(ValidOEM))
		return TRUE;
	else
		return FALSE;


	return FALSE;
	
}