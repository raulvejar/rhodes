//#include "StdAfx.h"
#include "AppManager.h"
#include "..\..\Common\Private\license.h"
#include "Sync.h"
#include "message.h"
#include "paint.h"
#include "meta.h"
#include "Generic.h"
#include "sipapi.h"

extern BOOL Log	(LogTypeInterface logSeverity,LPCTSTR pLogComment, 
			LPCTSTR pFunctionName, DWORD dwLineNumber,LPCTSTR pCallingModule);

extern CMeta			*g_pPlgManager;
extern CSync			*g_pEventSync;
extern CAppManager		*g_pAppManager;
extern LRESULT CALLBACK onNavEvent(EngineEventID eeID, LPARAM value, int tabIndex);
extern LRESULT CALLBACK onTopmostWnd(EngineEventID eeID, LPARAM value, int tabIndex);
extern LRESULT CALLBACK onMeta(EngineEventID eeID, LPARAM value, int tabIndex);
extern LRESULT CALLBACK onFieldFocus(EngineEventID eeID, LPARAM value, int tabIndex);
extern LRESULT CALLBACK onConsoleMessage(EngineEventID eeID, LPARAM value, int tabIndex);

extern CLicense			*g_pLicense;
extern HINSTANCE		g_hInst;
extern HWND				g_hBaseWnd;
extern CMessageManager	*g_pMessageManager;
extern CPaintManager	*g_pPaintManager;
extern CGenericManager	*g_pGenericManager;

extern CConfig			*g_pConfig;
typedef BOOL (__stdcall *UnregisterFunc1Proc)( UINT, UINT );
extern CWebKitEngine    *g_pEngine;
PB_ScreenMode			g_bFullScreenApp = PB_NO_TASKBAR;
//  Note: The following Variables were added during RC2 but most likely 
//  break the multiple instance architecture.  They are used when resizing the 
//  RhoElements window when hiding / showing the SIP
BOOL					bResizeOnSIP = FALSE;	//  Whether or not the SIP should resize the browser window (received from SIP module)
UINT					iIgnoreNextSettingChange = 0;	//  When the browser window is moved the SIP disappears and needs to be forcably shown again... this prevents us actioning the setting change from that reshowing
RECT					g_rcBrowserSize;	//  The Rectangle of the browser window as specified by the command areas (top and bottom command areas), we need to take account of this when hiding / showing the SIP.  Presumably this is application specific as the buttons are application specific.

CAppManager::CAppManager()
{
	m_pEngine			= NULL;
	m_hMainWnd			= NULL;
	memset(m_pApp,0,sizeof(CApp*)* (PB_APP_ARRSIZE));
	m_hRegTopWnd		= NULL;
	m_pLoadFile			= NULL;
	m_iLastID			= 0;
	//  Initialise the browser window to NOT show the command areas.
	//  Note that this will have no effect on task bar visibility.
	int iScreenWidth = GetSystemMetrics(SM_CXSCREEN);
	int iScreenHeight = GetSystemMetrics(SM_CYSCREEN);
	g_rcBrowserSize.right = iScreenWidth;
	g_rcBrowserSize.bottom = iScreenHeight - g_rcBrowserSize.top;
	g_rcBrowserSize.top = 0;
	g_rcBrowserSize.left = 0;
	m_bUsingLegacyEngine = FALSE;
		
}

CAppManager::~CAppManager(void)
{
	int iIndex;
	
	for(iIndex= 0;iIndex < m_iLastID + 1;iIndex++)
	{
		RemoveApp(iIndex);
		delete  m_pApp[iIndex];
	}

	delete [] m_pLoadFile;
	delete [] m_pEngDLLFilePath;

}


BOOL CAppManager::Init(HINSTANCE hInstance,HWND hWndParent,
					   PPBCORESTRUCT pbCoreStruct, CWebKitEngine *pEngine,LPCTSTR pAppName,LPCTSTR pStartPage)
{
	int iLen = 0;
	
	
	m_hInstance		= hInstance;
	m_hMainWnd		= hWndParent;
	m_pbCoreStruct	= pbCoreStruct;
	m_pEngine		= pEngine;

	m_pbCoreStruct->iCurrentInstID		= PB_INVALID_APPID;
	m_pbCoreStruct->iPreviousInstID		= PB_INVALID_APPID;
	
	LPCTSTR pTemp = g_pConfig->GetSettingPtr(L"Configuration\\Screen\\FullScreen",L"Value");
	if(pTemp){
		if (*pTemp==L'1')
			g_bFullScreenApp = PB_NO_TASKBAR;
		else
			g_bFullScreenApp = PB_WITH_TASKBAR;
	}

	//build LoadPage.html path 	
	iLen = wcslen(L"file://")+wcslen(m_pbCoreStruct->szInstallDirectory)+ wcslen(PB_HTML_INSTDIR)+ wcslen(PB_LOADPAGE)+ 1;
	m_pLoadFile = new WCHAR[iLen];
	if(m_pLoadFile){
		wcscpy(m_pLoadFile,L"file://");
		//wcscpy(m_pLoadFile,pbCoreStruct->szInstallDirectory);
		wcscat(m_pLoadFile,pbCoreStruct->szInstallDirectory);
		wcscat(m_pLoadFile,PB_HTML_INSTDIR);
		wcscat(m_pLoadFile,PB_LOADPAGE);
	}

	iLen = wcslen(m_pbCoreStruct->szInstallDirectory) + 
			wcslen(PB_BIN_INSTDIR) + wcslen(PB_ENG_DLL) + wcslen(L"_CTRL") + 1;
	m_pEngDLLFilePath = new WCHAR[iLen];
	if(!m_pEngDLLFilePath){
		return FALSE;
	}
	
	wcscpy(m_pEngDLLFilePath,m_pbCoreStruct->szInstallDirectory);
	wcscat(m_pEngDLLFilePath,PB_BIN_INSTDIR);

	//  Dynamically load the engine.  The engine to load will depend on the
	//  operating system and the 'CompatibilityModeEnabled configuration setting
	//  PBEngine_WK is the Default
	//  WTG_Engine_CTRL, based on the WC_HTML control:
	//  If running on Windows Mobile and InternetExplorer specified as Engine type
	//  WTG_Engine_COM, based on the IWebBrowser2 COM Object:
	//  If running on Windows CE and InternetExplorer specified as Engine type

	//  Read the Engine Selection Setting
	LPCTSTR tcCompatibilityMode = 
		g_pConfig->GetSettingPtr(L"Configuration\\Engine\\EngineInUse",L"Value");
	OSVERSIONINFO osvi;
	memset(&osvi, 0, sizeof(OSVERSIONINFO));
	osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
	GetVersionEx(&osvi);
	bool bIsCE = (osvi.dwMajorVersion == 5 && osvi.dwMinorVersion == 0) ||
				(osvi.dwMajorVersion == 6 && osvi.dwMinorVersion == 0);

	if (bIsCE && tcCompatibilityMode && wcsicmp(tcCompatibilityMode, L"InternetExplorer") == 0)
	{
		//  Device is CE and user has specified to use Internet Explorer, Use the COM engine.
		wcscat(m_pEngDLLFilePath, L"WTG_Engine_COM.dll");
		Log(PB_LOG_INFO, L"Engine in use is the IE COM engine (Device is CE and user selected Internet Explorer)", 
			_T(__FUNCTION__), __LINE__, L"CAppManager");
		m_bUsingLegacyEngine = TRUE;
	}
	else if (!bIsCE && tcCompatibilityMode && wcsicmp(tcCompatibilityMode, L"InternetExplorer") == 0)
	{
		//  Use the Ekioh WebKit Engine
		wcscat(m_pEngDLLFilePath, L"WTG_Engine_CTRL.dll");
		Log(PB_LOG_INFO, L"Engine in use is the IE HTML Control (Device is WM and user selected Internet Explorer)",
			_T(__FUNCTION__), __LINE__, L"CAppManager");
		m_bUsingLegacyEngine = TRUE;
	}
	else
	{
		//  Use the Motorola WebKit engine by default
		wcscat(m_pEngDLLFilePath, L"PBEngine_WK.dll");
		Log(PB_LOG_INFO, L"Engine is use is the Motorola WebKit engine (default)",
			_T(__FUNCTION__), __LINE__, L"CAppManager");

	}

	//  bool isWM50 =(osvi.dwMajorVersion == 5 && osvi.dwMinorVersion == 2 && osvi.dwBuildNumber < 14000) ||
	//	     		 (osvi.dwMajorVersion == 5 && osvi.dwMinorVersion == 1 && osvi.dwBuildNumber < 14000);
	//	bool isWM65 =(osvi.dwMajorVersion == 5 && osvi.dwMinorVersion == 2 && osvi.dwBuildNumber >= 21234) ||
	//				 (osvi.dwMajorVersion == 5 && osvi.dwMinorVersion == 1 && osvi.dwBuildNumber >= 21234);

	if(!InitApp(0,pStartPage,pAppName)){
		Log(PB_LOG_ERROR,L"Failed Initialise the application object",L"AddApp",__LINE__,L"CAppManager");
		goto _cleanup;
		
	}

	if(!m_pEngine->Init(m_pEngDLLFilePath))
	{
		return FALSE;
	}

	if(!m_pEngine->InitEngine(0,&HTMLWndProc,&m_pApp[0]->m_OwnerProc,m_pApp[0]->m_bScrollBarsEnabled, &GetEngineConfig))
	{
		Log(PB_LOG_ERROR,L"Unable to initialised specified engine",
			_T(__FUNCTION__), __LINE__, L"CAppManager");

		//  Failed to initialise the Engine
		goto _cleanup;
	}

	if(!m_pEngine->RegisterForEvent(EEID_TOPMOSTHWNDAVAILABLE, onTopmostWnd))
	{
		Log(PB_LOG_ERROR,L"Could not register 'Topmost Window' on the engine", 
			L"Init", __LINE__, L"CApp");
		goto _cleanup;
	}

	m_iLastID = 0;
	return TRUE;

_cleanup:
	delete m_pApp[0];
	m_pApp[0] = NULL;
	return FALSE;
}

BOOL CAppManager::InitApp(int iInstID,LPCTSTR pStartPage, LPCTSTR pAppName)
{
	//create the new CApp object
	m_pApp[iInstID]= new CApp();
	if(m_pApp[iInstID]){
		return m_pApp[iInstID]->Init(m_hInstance,iInstID,pAppName,pStartPage);
		
	}
	
	Log(PB_LOG_ERROR,PB_MEM_ERR_STR,L"CAppManager::InitApp",__LINE__,L"Core");
	return FALSE;

}


LRESULT CAppManager::ProcessOnTopMostWnd(HWND hWnd, int tabIndex)
{
	int iID;
	CApp *pApp;
	
	if(hWnd){
		g_pAppManager->m_pApp[tabIndex]->m_pbStruct.hWnd = hWnd;
		//now check if we have the topMostWindow for each application
		
		for(iID = 0;iID < (m_iLastID + 1);iID++)
		{
			if(g_pAppManager->m_pApp[tabIndex]->m_pbStruct.hWnd == NULL){
				//not all instances have a valid topmost window yet
				return S_OK;
			}

		}
		
		
		// all PB applications have had a page loaded
		// We should have the topmost window for all
		// switch on the meta tag event and Nav event and navigate to each app's start page
		{	
		if(!m_pEngine->RegisterForEvent(EEID_METATAG,onMeta)){
			Log(PB_LOG_ERROR,L"Could not register 'On Meta tag' on the engine",L"CAppManager::ProcessOnTopMostWnd",__LINE__,L"Core");
			return S_FALSE;
		}
	
		if(!m_pEngine->RegisterForEvent(EEID_DOCUMENTCOMPLETE,onNavEvent)){
			Log(PB_LOG_ERROR,L"Could not register 'Document Complete' on the engine",L"CAppManager::ProcessOnTopMostWnd",__LINE__,L"Core");
			return S_FALSE;
		}
		if(!m_pEngine->RegisterForEvent(EEID_NAVIGATECOMPLETE,onNavEvent)){
			Log(PB_LOG_ERROR,L"Could not register 'Document Complete' on the engine",L"CAppManager::ProcessOnTopMostWnd",__LINE__,L"Core");
			return S_FALSE;
		}
		if(!m_pEngine->RegisterForEvent(EEID_BEFORENAVIGATE,onNavEvent)){
			Log(PB_LOG_ERROR,L"Could not register 'Document Complete' on the engine",L"CAppManager::ProcessOnTopMostWnd",__LINE__,L"Core");
			return S_FALSE;
		}
		//Fired when the page load takes longer than the value specified by 'NavTimeout' in the config file
		if(!m_pEngine->RegisterForEvent(EEID_NAVIGATIONTIMEOUT,onNavEvent)){
			Log(PB_LOG_ERROR,L"Could not register 'Navigation Timeout' on the engine",L"CAppManager::ProcessOnTopMostWnd",__LINE__,L"Core");
			return S_FALSE;
		}
		
		//  Fired when the engine indicates an editable field has been clicked on
		if (!m_pEngine->RegisterForEvent(EEID_SETSIPSTATE, onFieldFocus))
		{
			Log(PB_LOG_INFO, L"Could not register Editable field focus notification on the engine", _T(__FUNCTION__), __LINE__, L"Core");
		}

		//  Fired when the engine gives us a console log message (JS error or window.console.log)
		if (!m_pEngine->RegisterForEvent(EEID_CONSOLEMESSAGE, onConsoleMessage))
		{
			Log(PB_LOG_INFO, L"Could not register Console log message notification on the engine", _T(__FUNCTION__), __LINE__, L"Core");
		}

		//Fired if there is an error navigating to the page, for example attempting to navigate to http://www.motorola.com if the device does not have a network connection.
		if(!m_pEngine->RegisterForEvent(EEID_NAVIGATIONERROR,onNavEvent)){
			Log(PB_LOG_ERROR,L"Could not register 'Navigation Error' on the engine",L"CAppManager::ProcessOnTopMostWnd",__LINE__,L"Core");
			return S_FALSE;
		}
		
		//////////////////////////////////////////////////////////////////////////////////////////////////
		/// The message manager routes messages to plug-ins that have registered an interest in a message 
		//////////////////////////////////////////////////////////////////////////////////////////////////
		
		if(!(g_pMessageManager = new CMessageManager ())){ 
			return FALSE;
		}

		//////////////////////////////////////////////////////////////////////////////////////////////////
		// The paint manager routes paint messages to plug-ins that have registered an interest in them
		// and will provide the plugin with the current paint structure
		//////////////////////////////////////////////////////////////////////////////////////////////////

		if(!(g_pPaintManager = new CPaintManager ())){
			return FALSE;
		}

		//////////////////////////////////////////////////////////////////////////////////////////////////
		// The generic manager runs a thread waiting for requests from the Generic ActiveX control.
		//////////////////////////////////////////////////////////////////////////////////////////////////
		
		if(!(g_pGenericManager = new CGenericManager ())){
			return FALSE;
		}
		
		g_pGenericManager->Start ();
		for(iID = 0;iID < (m_iLastID + 1);iID++)
		{
			pApp = g_pAppManager->m_pApp[iID];
			if(!pApp){
				Log(PB_LOG_ERROR,L"Application cannot be found",L"ProcessOnTopMostWnd",__LINE__,L"Core");
				return S_FALSE;
			}
			if(!pApp->DoPreloads()){
				Log(PB_LOG_ERROR,L"A plug-in could not be preloaded",L"ProcessOnTopMostWnd",__LINE__,L"Core");
				return S_FALSE;
			}
			if(!pApp->ShowDebugButtons()) 
			{
				Log(PB_LOG_WARNING, 
					L"Unable to read value of Debug Buttons from Configuration", 
					_T(__FUNCTION__), __LINE__, L"Core");
				return S_FALSE;
			}

			// GD - Show Scrollbars on WM6.5 Devices
			//OSVERSIONINFO osvi;
			//memset(&osvi, 0, sizeof(OSVERSIONINFO));
			//osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
			//GetVersionEx(&osvi);
			//bool isWM65 =(osvi.dwMajorVersion == 5 && osvi.dwMinorVersion == 2 && osvi.dwBuildNumber >= 21234) ||
			//	(osvi.dwMajorVersion == 5 && osvi.dwMinorVersion == 1 && osvi.dwBuildNumber >= 21234);
			// In compatibility mode if setting missing, or non-zero
			//LPCWSTR pvalue;
			//pvalue = g_pConfig->GetSettingPtr (L"Configuration\\CompatibilityMode\\CompatibilityModeEnabled", L"Value");
			//bool is_compatibility = (!pvalue || (pvalue [0] != L'0' && pvalue [0] != L'W'));
			//if (isWM65 && !is_compatibility)
			//{
			//	if(!pApp->ShowScrollBars()) 
			//	{
			//		Log(PB_LOG_WARNING, 
			//			L"Unable to read value of Scroll Bars from Configuration", 
			//			_T(__FUNCTION__), __LINE__, L"Core");
			//		return S_FALSE;
			//	}
			//}
			// End GD

			if(!pApp->LoadBadlink()) 
			{
				Log(PB_LOG_ERROR,L"Incorrect Badlink settings",L"ProcessOnTopMostWnd",__LINE__,L"Core");
				return S_FALSE;
			}
			if(!m_pEngine->SetNavigationTimeout(pApp->m_dwNavTimeoutMs)){
				Log(PB_LOG_ERROR,L"Could not set the navigation timeout on the engine, using default",L"CAppManager::ProcessOnTopMostWnd",__LINE__,L"Core");
			}



		}
		for(iID = 0;iID < (m_iLastID + 1);iID++)
		{
			PostMessage(g_pAppManager->m_hMainWnd,
				PB_NAVIGATETAB,(WPARAM)iID,(LPARAM)g_pAppManager->m_pApp[iID]->m_pHomePage); 
			
		}
		
		// GD - this is now done in KeyCapture
		/*
		//  Only want to do this once in the application.
		if (tabIndex == 0)
		{
			//  For the First tab register for Function and Application hotkeys
			//  We only want to do this once, not per application.
			if (!BlockHotKeys(g_pAppManager->m_pApp[tabIndex]->m_pbStruct.hWnd))
				return S_FALSE;
		}
		*/

		return S_OK;
		}
	}
	return S_FALSE;

}

 
LRESULT CALLBACK CAppManager::HTMLWndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	HDC hdc;
	BOOL processed;
	int index;
	LRESULT result;

	if(!g_pAppManager){
		//return DefWindowProc(hwnd, message, wParam, lParam);
		if(message == 0xF){
			return 1;
		}
		return 0;
	}
	
	// Assume that the message is not already processed
	processed = FALSE;

	// Find original wndproc for this window
	for (index = 0; index < PB_APP_ARRSIZE; index++)
	{
		if (g_pAppManager->m_pApp[index] && g_pAppManager->m_pApp[index]->m_pbStruct.hWnd == hwnd)
			break;
	}

	// Quit if not found - shouldn't ever happen
	if (index == PB_APP_ARRSIZE)
	{
		return 0;
	}

	// Pass message to any interested plugins - only pass on if not processed by plugin
	if (g_pMessageManager->Process (hwnd, message, wParam, lParam))
	{
		return 0;
	}

    if (message == WM_SETFOCUS) 
    {
		//  When RhoElements receives Focus, i.e. a user clicks in RhoElements.
		//  This may be a result of returning from another application, if so 
		//  The SIP and task bar may need to be rehidden (do not test to see
		//  whether they are visible or not as this would waste CPU cycles)
		PBScreenMode(g_bFullScreenApp, bResizeOnSIP, FALSE);

	}

	// Pass message to original window procedure
	result = CallWindowProc (g_pAppManager->m_pApp[index]->m_OwnerProc, hwnd, message, wParam, lParam);

	// Process paint messages after window's built-in painting
	if (message == WM_PAINT)
	{
		if ((hdc = GetDC (hwnd)) != NULL)
		{
			g_pPaintManager->Process (hwnd, hdc);
			ReleaseDC (hwnd, hdc);
		}
	}

	return result;
} 

LRESULT	CAppManager::SwitchAppInst(int iInstID)
{
	CApp  *pApp = NULL;
	//m_hAppSwitchThread  = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE) AppSwitchThread,(LPVOID) this, 0, 0);
	if(iInstID == m_pbCoreStruct->iCurrentInstID)
		return S_FALSE;
	
	m_pbCoreStruct->iPreviousInstID = m_pbCoreStruct->iCurrentInstID; 
	m_pbCoreStruct->iCurrentInstID = iInstID;
	
	if(iInstID >= 0){//are we switching to an application or a reserved PB screen 
		pApp = GetApp(iInstID);	
	}
	
		
	//inform the plug-ins of an application switch
	g_pEventSync->LockObj();
	g_pEventSync->Run(PB_APPFOCUSEVENT,m_pbCoreStruct->iPreviousInstID,m_pbCoreStruct->iCurrentInstID);
	g_pEventSync->UnlockObj();
	//wait for the plug-ins to handle the onAppFocus notification
	if(WaitForSingleObject(m_pbCoreStruct->pEventStructure->PBAppFocusHandledEvent,3000)==WAIT_TIMEOUT){
		//error at least one plug-in took too long to handle the onAppFocus call
		Log(PB_LOG_ERROR,
			L"at least one plug-in took too long to handle the onAppFocus call",
			L"SwitchAppInst",__LINE__,L"Core");
	}
	if(pApp){	
		pApp->Activate(TRUE);
		
	}
	return S_OK;//handled
	
}


void CAppManager::AddLicenceApp()
{
	g_pLicense = new CLicense(g_hInst, g_hBaseWnd, PB_LICENCE_SCREEN);
	if(g_pLicense){
		//  Log which version of PB is being used:
		VS_FIXEDFILEINFO m_vsAPPVer;
		memset(&m_vsAPPVer, 0, sizeof(VS_FIXEDFILEINFO));
		g_pLicense->GetVersionInfo(L"", &m_vsAPPVer);
		TCHAR tcVersion[100];
		DWORD dwRet = 12;
		wsprintf(tcVersion, L"RhoElements Core Version: %d.%d.%d.%d ", HIWORD(m_vsAPPVer.dwFileVersionMS), LOWORD(m_vsAPPVer.dwFileVersionMS),HIWORD(m_vsAPPVer.dwFileVersionLS),LOWORD(m_vsAPPVer.dwFileVersionLS));
		Log(PB_LOG_INFO, tcVersion, _T(__FUNCTION__), __LINE__, L"CAppManager");	
		if(g_pLicense->InitLicenseWizard()){
			return;
		}
	}
	delete g_pLicense;
}

// 
//Creates a record for each application
//
//returns the pointer to the instance ID
int CAppManager::AddApp(LPCTSTR pAppName,LPCTSTR PStartPage)
{
	int iIndex,iID = PB_INVALID_APPID;
	
		
	RECT rcViewSize;
	rcViewSize.top = 50;
	//	add the new record to the array.
	
	for(iIndex = 0;iIndex < PB_APP_ARRSIZE;iIndex++)
	{
		if(!m_pApp[iIndex]){
			if(InitApp(iIndex,PStartPage,pAppName)){
				iID = iIndex;
				
				if(!m_pEngine->NewTab(iID,1,PStartPage,m_pApp[iID]->m_bScrollBarsEnabled,&HTMLWndProc,&m_pApp[iID]->m_OwnerProc)){
					Log(PB_LOG_ERROR,L"Failed to create a new instance",L"AddApp",__LINE__,L"CAppManager");
					goto _cleanup;
				}
				if(!m_pEngine->ResizeOnTab(iID,rcViewSize)){
					Log(PB_LOG_ERROR,L"Failed to resize the window",L"AddApp",__LINE__,L"CAppManager");
					goto _cleanup;
				}
				
				m_iLastID = iID > m_iLastID ? iID: m_iLastID;
				return iID;
				
			}
			return PB_INVALID_APPID;

		}
		
	}
	
_cleanup:
	delete m_pApp[iID];
	return PB_INVALID_APPID;

}


void CAppManager::LoadDummy()
{
	int iID;
	for(iID = 0;iID < (m_iLastID + 1);iID++)
	{
		if(!m_pEngine->NavigateOnTab(m_pLoadFile,iID)){
			Log(PB_LOG_ERROR,L"Could not navigate to the load page",L"CAppManager::LoadDummy",__LINE__,L"Core");
			break;
		}

	}
	
}


void CAppManager::RemoveApp(int iID)
{
	if(m_pApp){
		if(m_pApp[iID]){
			g_pPlgManager->RemoveApp(&m_pApp[iID]->m_pbStruct);
			delete m_pApp[iID];
			m_pApp[iID] = NULL;
		}
	}
	
}

/**
* \author	Darryn Campbell (DCC, JRQ768)
* \date		February 2010 (First Created, DCC)
*/
LRESULT CALLBACK CAppManager::GetEngineConfig(int iInstID, LPCTSTR tcSetting, TCHAR* tcValue)
{
	LPCTSTR tcValueRead;
	tcValueRead = g_pConfig->GetAppSettingPtr(iInstID, tcSetting, L"Value");
	if (tcValueRead != NULL)
		wcscpy(tcValue, tcValueRead);
	else
		tcValue = NULL;
	return S_OK;
} 

//  Accessor for g_rcBrowserSize, used to determine the browser size that the 
//  command areas (top and bottom) want.
void SetBrowserSize(RECT rcBrowser)
{
	g_rcBrowserSize.bottom = rcBrowser.bottom;
	g_rcBrowserSize.left = rcBrowser.left;
	g_rcBrowserSize.top = rcBrowser.top;
	g_rcBrowserSize.right = rcBrowser.right;
}

/*****************************************************************************************/
//	Put the window into fullscreen/non fullscreen mode
//*****************************************************************************************/
void PBScreenMode(PB_ScreenMode screenMode, BOOL bAccountForSIP, BOOL bAccountForIgnoreSettingChange)
{
	//  WebKit engine introduces different behaviour when handling the SIP, 
	//  added a bAccountForIgnoreSetting to handle the WebKit differences
	//  but this defaults to TRUE for IE engine.
	if (g_pAppManager->m_bUsingLegacyEngine)
		bAccountForIgnoreSettingChange = TRUE;
	bResizeOnSIP = bAccountForSIP;
	if (!bAccountForIgnoreSettingChange)
		iIgnoreNextSettingChange = 0;
	if (iIgnoreNextSettingChange == 1)
	{
		iIgnoreNextSettingChange = 0;
		return;
	}

	HWND hTaskBar = FindWindow(_T("HHTaskBar"), NULL);
	if (!hTaskBar)
		return;

	SIPINFO sipinfo;
	BOOL bSipShown = FALSE;
	if (bAccountForSIP)
	{
		//  Browser window needs to be resized around the SIP
		//  Get the dimensions of the SIP
		memset(&sipinfo, 0, sizeof(SIPINFO));
		sipinfo.cbSize = sizeof(SIPINFO);
		sipinfo.dwImDataSize = 0;
		SipGetInfo(&sipinfo);
	}

	int iScreenWidth = GetSystemMetrics(SM_CXSCREEN);
	int iScreenHeight = GetSystemMetrics(SM_CYSCREEN);
	RECT rcBrowserSize;
	rcBrowserSize.top = g_rcBrowserSize.top;
	rcBrowserSize.bottom = g_rcBrowserSize.bottom;
	rcBrowserSize.left = g_rcBrowserSize.left;
	rcBrowserSize.right = g_rcBrowserSize.right;
	//  Account for rotated screen, if the width of the browser is greater than 
	//  the height and the width of the screen is the other way around the swap
	//  the browser orientation.
	//  NOTE: The following code was deprecated by the changes caused by
	//  SR EMBPD00022091
//	if (((rcBrowserSize.right - rcBrowserSize.left) > (rcBrowserSize.bottom - rcBrowserSize.top)
//		&& (iScreenWidth < iScreenHeight))
//		||
//		((rcBrowserSize.right - rcBrowserSize.left) < (rcBrowserSize.bottom - rcBrowserSize.top)
//		&& (iScreenWidth > iScreenHeight)))
//	{
//		//  Swap the Browser coordinates
//		int tempTop = rcBrowserSize.top;
//		int tempBottom = rcBrowserSize.bottom;
//		rcBrowserSize.top = rcBrowserSize.left;
//		rcBrowserSize.bottom = rcBrowserSize.right;
//		rcBrowserSize.left = tempTop;
//		rcBrowserSize.right = tempBottom;
//	}
	RECT rcTaskBar;
	GetWindowRect(hTaskBar, &rcTaskBar);
	if(screenMode == PB_NO_TASKBAR)
	{
		//  RhoElements should cover the Task Bar
		EnableWindow(hTaskBar, FALSE); 
		ShowWindow(hTaskBar, SW_HIDE);

		int iParentWindowTop = 0;
		int iParentWindowLeft = 0;
		int iParentWindowRight = iScreenWidth - g_rcBrowserSize.left;
		int iParentWindowBottom = iScreenHeight;

		//  Take account of the SIP (if requested)
		if (bAccountForSIP && (sipinfo.fdwFlags & SIPF_ON))
		{
			bSipShown = TRUE;
			rcBrowserSize.bottom = iScreenHeight;
//			rcBrowserSize.top = 0;
			//  Work out if the SIP is at the top of the screen
			int iMidPointOfSIP = (sipinfo.rcSipRect.bottom - sipinfo.rcSipRect.top) 
								+ sipinfo.rcSipRect.top;
			if (iMidPointOfSIP > (iScreenHeight / 2))
			{
				//  SIP is below the Browser
				//iParentWindowBottom = sipinfo.rcSipRect.top;  <<  Moving the entire window
				rcBrowserSize.bottom = rcBrowserSize.bottom - (iScreenHeight - sipinfo.rcSipRect.top);
			}
			else
			{
				//  SIP is Above the Browser
				//iParentWindowTop = sipinfo.rcSipRect.bottom;  <<  Moving the entire window
				rcBrowserSize.top = rcBrowserSize.top + (sipinfo.rcSipRect.bottom);
			}
			if (bAccountForIgnoreSettingChange)
				iIgnoreNextSettingChange = 1;
		}

		MoveWindow(g_pEngine->GetParentHWND(), iParentWindowLeft, 
			iParentWindowTop, iParentWindowRight - iParentWindowLeft, 
			iParentWindowBottom - iParentWindowTop, TRUE);
		g_pEngine->ResizeOnTab(0, rcBrowserSize);
		
		if (bSipShown)
		{
			HWND sipHWND = FindWindow(L"SipWndClass", NULL);
			if (sipHWND)
			{
				SetFocus(sipHWND);
				SetForegroundWindow(sipHWND);
				ShowWindow(sipHWND, SW_SHOW);
			}
			Sleep(10);
			SipShowIM(SIPF_ON);
		}
		HideSIPButton();
	}
	else if(screenMode == PB_WITH_TASKBAR)
	{
		//  The position of the task bar is different on WM and CE
		OSVERSIONINFO osvi;
		memset(&osvi, 0, sizeof(OSVERSIONINFO));
		osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
		GetVersionEx(&osvi);
		bool isWM = (osvi.dwMajorVersion == 5 && osvi.dwMinorVersion == 2) ||
		     (osvi.dwMajorVersion == 5 && osvi.dwMinorVersion == 1);

		//  RhoElements should not cover the Task Bar
		int iParentWindowTop = 0;  //  (rcTaskBar.bottom - rcTaskBar.top) in WM, 0 in CE
		int iParentWindowLeft = 0;  //  always 0
		int iParentWindowRight = iScreenWidth - g_rcBrowserSize.left;
		int iParentWindowBottom = iScreenHeight - (rcTaskBar.bottom - rcTaskBar.top);
		if (isWM)
		{
			iParentWindowTop = (rcTaskBar.bottom - rcTaskBar.top);
			iParentWindowBottom = iScreenHeight;
			//  The CTRL engine needs to be moved down 
			//  <taskbar height> pixels and the COM engine needs to be shrunk
			//  by that many pixels from the bottom
			if (wcsstr(g_pAppManager->m_pEngDLLFilePath, L"WTG_Engine_CTRL") != NULL)
				rcBrowserSize.top = rcBrowserSize.top + rcTaskBar.bottom;
			else
				rcBrowserSize.bottom = 
					rcBrowserSize.bottom - (rcTaskBar.bottom - rcTaskBar.top);


		}

		//  Take account of the SIP (if requested)
		if (bAccountForSIP && (sipinfo.fdwFlags & SIPF_ON))
		{
			bSipShown = TRUE;
			rcBrowserSize.bottom = iScreenHeight;
			rcBrowserSize.top = 0;
			//  Work out if the SIP is at the top of the screen
			int iMidPointOfSIP = (sipinfo.rcSipRect.bottom - sipinfo.rcSipRect.top) 
								+ sipinfo.rcSipRect.top;
			if (iMidPointOfSIP > ((iScreenHeight - (rcTaskBar.bottom - rcTaskBar.top)) / 2))
			{
				//  SIP is below the Browser
				//iParentWindowBottom = sipinfo.rcSipRect.top;  <<  Moving the entire window
				rcBrowserSize.bottom = rcBrowserSize.bottom - (iScreenHeight - sipinfo.rcSipRect.top);
			}
			else
			{
				//  SIP is Above the Browser
				//iParentWindowTop = sipinfo.rcSipRect.bottom;  << Moving the entire window
				if (isWM && wcsstr(g_pAppManager->m_pEngDLLFilePath, L"WTG_Engine_COM") != NULL)
					rcBrowserSize.top = rcBrowserSize.top + (sipinfo.rcSipRect.bottom) - rcTaskBar.bottom;
				else
					rcBrowserSize.top = rcBrowserSize.top + (sipinfo.rcSipRect.bottom);
			}
			if (bAccountForIgnoreSettingChange)
				iIgnoreNextSettingChange = 1;
		}



		MoveWindow(g_pEngine->GetParentHWND(), iParentWindowLeft, 
			iParentWindowTop, iParentWindowRight - iParentWindowLeft, 
			iParentWindowBottom - iParentWindowTop, TRUE);

		g_pEngine->ResizeOnTab(0, rcBrowserSize);
		if (bSipShown)
		{
			HWND sipHWND = FindWindow(L"SipWndClass", NULL);
			if (sipHWND)
			{
				SetFocus(sipHWND);
				SetForegroundWindow(sipHWND);
				ShowWindow(sipHWND, SW_SHOW);
			}
			Sleep(10);
			SipShowIM(SIPF_ON);
		}
		HideSIPButton();

		//  Enable the Task Bar
		EnableWindow(hTaskBar, TRUE); 
		ShowWindow(hTaskBar, SW_SHOW);
	}
	else if (screenMode == PB_NOT_SHOWN)
	{
		//  Enable the Task Bar
		EnableWindow(hTaskBar, TRUE); 
		ShowWindow(hTaskBar, SW_SHOW);
	}
 
}

