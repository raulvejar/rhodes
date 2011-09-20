#include "StdAfx.h"
#include "Eng.h"
// #include "gpswrapper/gpswrapper.h"
#include "../../Engine/common/pbengine_defs.h"

// extern BOOL Log			(LogTypeInterface logSeverity,LPCTSTR pLogComment, LPCTSTR pFunctionName, DWORD dwLineNumber,LPCTSTR pCallingModule);
// extern HWND				g_hBaseWnd; //Used to re-synch the GpsWrapper thread with the core main thread

CWebKitEngine::CWebKitEngine(HWND hParentWnd, HINSTANCE hInstance)
{
	m_hInstance					= hInstance;
	m_hwndParent				= hParentWnd;
	m_hEngineLib				= NULL;
	memset(&m_rcViewSize,0,sizeof(RECT));
	m_lpCreateEngine			= NULL;
	m_lpInitEngine				= NULL;
	m_lpDeinitEngine			= NULL;
	m_lpGetHTMLHWND				= NULL;
	m_lpGetParentHWND			= NULL;
	m_lpRegisterForEvent		= NULL;
	m_lpPreprocessMessage		= NULL;
	m_lpNavigate				= NULL;
	m_lpNavigateOnTab			= NULL;
	m_lpStop					= NULL;
	m_lpStopOnTab				= NULL;
	m_lpReload					= NULL;
	m_lpReloadOnTab				= NULL;
	m_lpZoom					= NULL;
	m_lpZoomOnTab				= NULL;
	m_lpZoomText				= NULL;
	m_lpZoomTextOnTab			= NULL;
	m_lpGetZoomTextOnTab		= NULL;
	m_lpScrollbars_HPosSet		= NULL;
	m_lpScrollbars_HPosSetOnTab	= NULL;
	m_lpScrollbars_HPosGet		= NULL;
	m_lpScrollbars_HPosGetOnTab	= NULL;
	m_lpScrollbars_VPosSet		= NULL;
	m_lpScrollbars_VPosSetOnTab	= NULL;
	m_lpScrollbars_VPosGet		= NULL;
	m_lpScrollbars_VPosGetOnTab	= NULL;
	m_lpSetScrollbars			= NULL;
	m_lpSetScrollbarsOnTab		= NULL;
	m_lpGetScrollbars			= NULL;
	m_lpGetScrollbarsOnTab		= NULL;
	m_lpTab_New					= NULL;
	m_lpTab_Close				= NULL;
	m_lpTab_CloseCurrent		= NULL;
	m_lpTab_Switch				= NULL;
	m_lpTab_Resize				= NULL;
	m_lpTab_Lock				= NULL;
	m_lpTab_Count				= NULL;
	
	
	m_lpTab_GetTitle			= NULL;
	m_lpTab_GetTitleOnTab		= NULL;
	m_lpTab_GetIconOnTab		= NULL;
	m_lpTab_ShowSwitcher		= NULL;
	m_lpJS_Invoke				= NULL;
	m_lpJS_InvokeOnTab			= NULL;
	m_lpJS_Exists				= NULL;
	m_lpJS_ExistsOnTab			= NULL;
	m_lpHistory_GoForward		= NULL;
	m_lpHistory_GoForwardOnTab	= NULL;
	m_lpHistory_GoBack			= NULL;
	m_lpHistory_GoBackOnTab		= NULL;
	m_lpSetFitToScreen			= NULL;
	m_lpGetFitToScreen			= NULL;
	m_lpSetClearType			= NULL;
	m_lpGetClearType			= NULL;
	m_lpSetJavaScript			= NULL;
	m_lpGetJavaScript			= NULL;
	m_lpSetImages				= NULL;
	m_lpGetImages				= NULL;
	m_lpSetSounds				= NULL;
	m_lpGetSounds				= NULL;
	m_lpSetActiveX				= NULL;
	m_lpGetActiveX				= NULL;
	m_lpSetAcceleratorMode		= NULL;
	m_lpGetAcceleratorMode		= NULL;
	m_lpSetBrowserGesturing		= NULL;
	m_lpGetBrowserGesturing		= NULL;
	m_lpSetNavigationTimeout	= NULL;
	m_lpGetNavigationTimeout	= NULL;
	m_lpSetLocationInterface	= NULL;
	m_lpUpdateLocation			= NULL;
	// m_pGpsWrapper				= NULL;
}

CWebKitEngine::~CWebKitEngine(void)
{
	FreeLibrary(m_hEngineLib);
}


// Load the library and initialise the function pointers
BOOL CWebKitEngine::Init(LPCTSTR pDLLPathName)
{
	return LoadEngineDLL(pDLLPathName);	
}

// Create the engine and the first application tab 
BOOL CWebKitEngine::InitEngine(int iInstID,HTMLWndPROC_T pSubClassProc, WNDPROC* pOwnerProc,BoolSettingValue bScrollBars, ReadEngineConfigParameter_T configFunction)
{
	BOOL res = FALSE;
	res = (m_lpInitEngine(m_hInstance, m_hwndParent, iInstID,L"",bScrollBars,pSubClassProc,pOwnerProc, configFunction)==S_OK);
	// if (res)
	//	InitGpsWrapper();
	return res;
}

BOOL CWebKitEngine::DeInitEngine()
{
	// if (m_pGpsWrapper)
	//	DeInitGpsWrapper();
	return S_OK;
}

BOOL CWebKitEngine::CloseTab(int iInstID)
{
	return (m_lpTab_Close(iInstID)== S_OK);
}



BOOL CWebKitEngine::JavaScriptInvoke(int iInstID,LPCTSTR pfunctionJS)
{
	return (m_lpJS_InvokeOnTab(pfunctionJS,iInstID)== S_OK);
}

BOOL CWebKitEngine::JavaScript_Exist(int iInstID,LPCTSTR pfunctionJS)
{
	return (m_lpJS_ExistsOnTab(pfunctionJS,iInstID)== S_OK);
}


BOOL CWebKitEngine::Navigate(LPCTSTR szURL)
{
	return(m_lpNavigate(szURL)== S_OK);
}
BOOL CWebKitEngine::NavigateOnTab (LPCTSTR szURL, UINT iTab)
{
	return(m_lpNavigateOnTab(szURL,iTab)== S_OK);
}

BOOL CWebKitEngine::RegisterForEvent(EngineEventID EventID,ENGINEEVENTPROC pCBFunc)
{
	return (m_lpRegisterForEvent(EventID,pCBFunc) == S_OK);
}

BOOL CWebKitEngine::NewTab(int iInstID,BOOL bForeground,LPCTSTR pHomeURI,BoolSettingValue bScrollBars,HTMLWndPROC_T pSubClassProc, WNDPROC* pOwnerProc)
{
	return (m_lpTab_New(iInstID,bForeground,pHomeURI,bScrollBars,pSubClassProc,pOwnerProc, NULL)== S_OK);
}

BOOL CWebKitEngine::SwitchTabs(int iNewTab)
{
	return (m_lpTab_Switch(iNewTab)==S_OK);
}

BOOL CWebKitEngine::ResizeOnTab(int iInstID,RECT rcNewSize)
{
	return (m_lpTab_Resize(rcNewSize,iInstID)==S_OK);
}

BoolSettingValue CWebKitEngine::SetFitToScreen(BoolSettingValue bsvOn)
{
	return m_lpSetFitToScreen(bsvOn);
}
BoolSettingValue CWebKitEngine::SetClearType(BoolSettingValue bsvOn )
{
	return m_lpSetClearType(bsvOn);
}
BoolSettingValue CWebKitEngine::SetJavaScript(BoolSettingValue bsvOn)
{
	return m_lpSetJavaScript(bsvOn);
}


//Zoom functions
#pragma region PB_BrowserCommands

BOOL CWebKitEngine::ZoomPageOnTab(double dFactor, UINT iTab)
{
	return (m_lpZoomOnTab(dFactor, iTab) == S_OK);
}
/*double GetPageZoomOnTab (double dFactor, UINT iTab)
{

}
*/
BOOL CWebKitEngine::ZoomTextOnTab (TextZoomValue ZoomLevel, UINT iTab)
{
	return (m_lpZoomTextOnTab(ZoomLevel,iTab)==S_OK);
}


BOOL CWebKitEngine::GetTextZoomOnTab (TextZoomValue* pZoomLevel, UINT iTab)
{
	
	return (m_lpGetZoomTextOnTab(pZoomLevel,iTab)==S_OK);
}

BOOL CWebKitEngine::SetAccelerator (AcceleratorValue eAcceleratorValue)
{
	return (m_lpSetAcceleratorMode(eAcceleratorValue) == S_OK);
}

BOOL CWebKitEngine::BackOnTab(int iInstID,int iPagesBack)
{
	return(m_lpHistory_GoBackOnTab(iPagesBack, iInstID) == S_OK);
}

BOOL CWebKitEngine::ForwardOnTab(int iInstID)
{
	return (m_lpHistory_GoForwardOnTab(1, iInstID) == S_OK);
}


BOOL CWebKitEngine::ReloadOnTab(BOOL bFromCache, UINT iTab)
{
	bool bCached = bFromCache >0 ?true:false;//have to do this to avoid a compiler warning
	return (m_lpReloadOnTab(bCached,iTab)==S_OK);
}


BOOL CWebKitEngine::StopOnTab (UINT iTab)
{
	return (m_lpStopOnTab(iTab)==S_OK);
}


BOOL CWebKitEngine::SetNavigationTimeout(DWORD dwMilliseconds)
{
	return (m_lpSetNavigationTimeout(dwMilliseconds) == S_OK);

}

BOOL CWebKitEngine::GetSize (int *pwidth, int *pheight)
{
	return (*m_lpGetSize) (pwidth, pheight);
}

BOOL CWebKitEngine::GetSizeOnTab (int *pwidth, int *pheight, int tabid)
{
	return (*m_lpGetSizeOnTab) (pwidth, pheight, tabid);
}

BOOL CWebKitEngine::SetVScrollOnTab (int scroll, int tab)
{
	return m_lpScrollbars_VPosSetOnTab (scroll, tab);
}

BOOL CWebKitEngine::SetHScrollOnTab (int scroll, int tab)
{
	return m_lpScrollbars_HPosSetOnTab (scroll, tab);
}

BOOL CWebKitEngine::GetTitleOnTab (LPTSTR szURL, UINT iMaxLen, UINT iTab)
{
	return m_lpTab_GetTitleOnTab(szURL, iMaxLen, iTab);
}

#pragma endregion


BOOL CWebKitEngine::LoadEngineDLL(LPCTSTR pDLLPathName)
{
	m_hEngineLib = LoadLibrary(pDLLPathName);
	if (m_hEngineLib == NULL)
	{
		// Log(PB_LOG_ERROR,L"PBEngine DLL Not Found",L"CWebKitEngine::LoadEngineDLL",__LINE__,L"Core");
		return FALSE;
		
	}
	else
	{
		//  Engine DLL Successfully Loaded, Load the Functions
		m_lpCreateEngine				= (tPB_CreateEngine)GetProcAddress(m_hEngineLib, TEXT("PB_CreateEngine"));
		m_lpInitEngine					= (tPB_InitEngine)GetProcAddress(m_hEngineLib, TEXT("PB_InitEngine"));
		m_lpDeinitEngine				= (tPB_DeinitEngine)GetProcAddress(m_hEngineLib, TEXT("PB_DeinitEngine"));
		m_lpGetHTMLHWND					= (tPB_GetHTMLHWND)GetProcAddress(m_hEngineLib, TEXT("PB_GetHTMLHWND"));
		m_lpGetHTMLHWNDOnTab			= (tPB_GetHTMLHWNDOnTab)GetProcAddress(m_hEngineLib, TEXT("PB_GetHTMLHWND"));
		m_lpGetParentHWND				= (tPB_GetParentHWND)GetProcAddress(m_hEngineLib, TEXT("PB_GetParentHWND"));
		m_lpRegisterForEvent			= (tPB_RegisterForEvent)GetProcAddress(m_hEngineLib, TEXT("PB_RegisterForEvent"));
		m_lpPreprocessMessage			= (tPB_PreprocessMessage)GetProcAddress(m_hEngineLib, TEXT("PB_PreprocessMessage"));
		m_lpNavigate					= (tPB_Navigate)GetProcAddress(m_hEngineLib, TEXT("PB_Navigate"));
		m_lpNavigateOnTab				= (tPB_NavigateOnTab)GetProcAddress(m_hEngineLib, TEXT("PB_NavigateOnTab"));
		m_lpStop						= (tPB_Stop)GetProcAddress(m_hEngineLib, TEXT("PB_Stop"));
		m_lpStopOnTab					= (tPB_StopOnTab)GetProcAddress(m_hEngineLib, TEXT("PB_StopOnTab"));
		m_lpReload						= (tPB_Reload)GetProcAddress(m_hEngineLib, TEXT("PB_Reload"));
		m_lpReloadOnTab					= (tPB_ReloadOnTab)GetProcAddress(m_hEngineLib, TEXT("PB_ReloadOnTab"));
		m_lpZoom						= (tPB_Zoom)GetProcAddress(m_hEngineLib, TEXT("PB_Zoom"));
		m_lpZoomOnTab					= (tPB_ZoomOnTab)GetProcAddress(m_hEngineLib, TEXT("PB_ZoomOnTab"));
		m_lpZoomText					= (tPB_ZoomText)GetProcAddress(m_hEngineLib, TEXT("PB_ZoomText"));
		m_lpZoomTextOnTab				= (tPB_ZoomTextOnTab)GetProcAddress(m_hEngineLib, TEXT("PB_ZoomTextOnTab"));
		m_lpGetZoomTextOnTab			= (tPB_GetZoomTextOnTab)GetProcAddress(m_hEngineLib, TEXT("PB_GetZoomTextOnTab"));
		m_lpScrollbars_HPosSet		= (tPB_Scrollbars_HPosSet)GetProcAddress(m_hEngineLib, TEXT("PB_Scrollbars_HPosSet"));
		m_lpScrollbars_HPosSetOnTab	= (tPB_Scrollbars_HPosSetOnTab)GetProcAddress(m_hEngineLib, TEXT("PB_Scrollbars_HPosSetOnTab"));
		m_lpScrollbars_HPosGet		= (tPB_Scrollbars_HPosGet)GetProcAddress(m_hEngineLib, TEXT("PB_Scrollbars_HPosGet"));
		m_lpScrollbars_HPosGetOnTab	= (tPB_Scrollbars_HPosGetOnTab)GetProcAddress(m_hEngineLib, TEXT("PB_Scrollbars_HPosGetOnTab"));
		m_lpScrollbars_VPosSet		= (tPB_Scrollbars_VPosSet)GetProcAddress(m_hEngineLib, TEXT("PB_Scrollbars_VPosSet"));
		m_lpScrollbars_VPosSetOnTab	= (tPB_Scrollbars_VPosSetOnTab)GetProcAddress(m_hEngineLib, TEXT("PB_Scrollbars_VPosSetOnTab"));
		m_lpScrollbars_VPosGet		= (tPB_Scrollbars_VPosGet)GetProcAddress(m_hEngineLib, TEXT("PB_Scrollbars_VPosGet"));
		m_lpScrollbars_VPosGetOnTab	= (tPB_Scrollbars_VPosGetOnTab)GetProcAddress(m_hEngineLib, TEXT("PB_Scrollbars_VPosGetOnTab"));
		m_lpSetScrollbars				= (tPB_SetScrollbars)GetProcAddress(m_hEngineLib, TEXT("PB_SetScrollBars"));
		m_lpSetScrollbarsOnTab		= (tPB_SetScrollbarsOnTab)GetProcAddress(m_hEngineLib, TEXT("PB_SetScrollBarsOnTab"));
		m_lpGetScrollbars				= (tPB_GetScrollbars)GetProcAddress(m_hEngineLib, TEXT("PB_GetScrollBars"));
		m_lpGetScrollbarsOnTab		= (tPB_GetScrollbarsOnTab)GetProcAddress(m_hEngineLib, TEXT("PB_GetScrollBarsOnTab"));
		m_lpTab_New					= (tPB_Tab_New)GetProcAddress(m_hEngineLib, TEXT("PB_Tab_New"));
		m_lpTab_Close					= (tPB_Tab_Close)GetProcAddress(m_hEngineLib, TEXT("PB_Tab_Close"));
		m_lpTab_CloseCurrent			= (tPB_Tab_CloseCurrent)GetProcAddress(m_hEngineLib, TEXT("PB_Tab_CloseCurrent"));
		m_lpTab_Switch				= (tPB_Tab_Switch)GetProcAddress(m_hEngineLib, TEXT("PB_Tab_Switch"));
		m_lpTab_Count					= (tPB_Tab_Count)GetProcAddress(m_hEngineLib, TEXT("PB_Tab_Count"));
		m_lpTab_GetTitle				= (tPB_Tab_GetTitle)GetProcAddress(m_hEngineLib, TEXT("PB_Tab_GetTitle"));
		m_lpTab_ShowSwitcher			= (tPB_Tab_ShowSwitcher)GetProcAddress(m_hEngineLib, TEXT("PB_Tab_ShowSwitcher"));
		m_lpTab_Resize					= (tPB_Tab_Resize)GetProcAddress(m_hEngineLib, TEXT("PB_Tab_Resize"));
		m_lpJS_Invoke					= (tPB_JS_Invoke)GetProcAddress(m_hEngineLib, TEXT("PB_JS_Invoke"));
		m_lpJS_InvokeOnTab			= (tPB_JS_InvokeOnTab)GetProcAddress(m_hEngineLib, TEXT("PB_JS_InvokeOnTab"));
		m_lpJS_Exists					= (tPB_JS_Exists)GetProcAddress(m_hEngineLib, TEXT("PB_JS_Exists"));
		m_lpJS_ExistsOnTab			= (tPB_JS_ExistsOnTab)GetProcAddress(m_hEngineLib, TEXT("PB_JS_ExistsOnTab"));
		m_lpHistory_GoForward			= (tPB_History_GoForward)GetProcAddress(m_hEngineLib, TEXT("PB_History_GoForward"));
		m_lpHistory_GoForwardOnTab	= (tPB_History_GoForwardOnTab)GetProcAddress(m_hEngineLib, TEXT("PB_History_GoForwardOnTab"));
		m_lpHistory_GoBack			= (tPB_History_GoBack)GetProcAddress(m_hEngineLib, TEXT("PB_History_GoBack"));
		m_lpHistory_GoBackOnTab		= (tPB_History_GoBackOnTab)GetProcAddress(m_hEngineLib, TEXT("PB_History_GoBackOnTab"));
		m_lpSetFitToScreen			= (tPB_SetFitToScreen)GetProcAddress(m_hEngineLib, TEXT("PB_SetFitToScreen"));
		m_lpGetFitToScreen			= (tPB_GetFitToScreen)GetProcAddress(m_hEngineLib, TEXT("PB_GetFitToScreen"));
		m_lpSetClearType				= (tPB_SetClearType)GetProcAddress(m_hEngineLib, TEXT("PB_SetClearType"));
		m_lpGetClearType				= (tPB_GetClearType)GetProcAddress(m_hEngineLib, TEXT("PB_GetClearType"));
		m_lpSetJavaScript				= (tPB_SetJavaScript)GetProcAddress(m_hEngineLib, TEXT("PB_SetJavaScript"));
		m_lpGetJavaScript				= (tPB_GetJavaScript)GetProcAddress(m_hEngineLib, TEXT("PB_GetJavaScript"));
		m_lpSetImages					= (tPB_SetImages)GetProcAddress(m_hEngineLib, TEXT("PB_SetImages"));
		m_lpGetImages					= (tPB_GetImages)GetProcAddress(m_hEngineLib, TEXT("PB_GetImages"));
		m_lpSetSounds					= (tPB_SetSounds)GetProcAddress(m_hEngineLib, TEXT("PB_SetSounds"));
		m_lpGetSounds					= (tPB_GetSounds)GetProcAddress(m_hEngineLib, TEXT("PB_GetSounds"));
		m_lpSetActiveX				= (tPB_SetActiveX)GetProcAddress(m_hEngineLib, TEXT("PB_SetActiveX"));
		m_lpGetActiveX				= (tPB_GetActiveX)GetProcAddress(m_hEngineLib, TEXT("PB_GetActiveX"));
		m_lpSetAcceleratorMode		= (tPB_SetAcceleratorMode)GetProcAddress(m_hEngineLib, TEXT("PB_SetAcceleratorMode"));
		m_lpGetAcceleratorMode		= (tPB_GetAcceleratorMode)GetProcAddress(m_hEngineLib, TEXT("PB_GetAcceleratorMode"));
		m_lpSetBrowserGesturing		= (tPB_SetBrowserGesturing)GetProcAddress(m_hEngineLib, TEXT("PB_SetBrowserGesturing"));
		m_lpGetBrowserGesturing		= (tPB_GetBrowserGesturing)GetProcAddress(m_hEngineLib, TEXT("PB_GetBrowserGesturing"));
		m_lpSetNavigationTimeout		= (tPB_SetNavigationTimeout)GetProcAddress(m_hEngineLib, TEXT("PB_SetNavigationTimeout"));
		m_lpGetNavigationTimeout		= (tPB_GetNavigationTimeout)GetProcAddress(m_hEngineLib, TEXT("PB_GetNavigationTimeout"));
		m_lpGetSize					= (tPB_GetSize)GetProcAddress(m_hEngineLib, TEXT("PB_Scrollbars_SizeGet"));
		m_lpGetSizeOnTab			= (tPB_GetSizeOnTab)GetProcAddress(m_hEngineLib, TEXT("PB_Scrollbars_SizeGetOnTab"));
		m_lpTab_GetTitleOnTab		= (tPB_Tab_GetTitleOnTab)GetProcAddress(m_hEngineLib, TEXT("PB_Tab_GetTitleOnTab"));
//		m_lpDCCTestMethod				= (tPB_DCC_TestMethod)GetProcAddress(m_hEngineLib, TEXT("PB_DCC_TestMethod"));
		m_lpSetLocationInterface	= (tPB_SetLocationInterface)GetProcAddress(m_hEngineLib, TEXT("PB_SetLocationInterface"));
		m_lpUpdateLocation			= (tPB_UpdateLocation)GetProcAddress(m_hEngineLib, TEXT("PB_UpdateLocation"));

		if (m_lpCreateEngine				== NULL ||
			m_lpInitEngine				== NULL ||
			m_lpDeinitEngine				== NULL ||
			m_lpGetHTMLHWND				== NULL ||
			m_lpGetHTMLHWNDOnTab		== NULL ||
			m_lpGetParentHWND				== NULL ||
			m_lpRegisterForEvent			== NULL ||
			m_lpPreprocessMessage			== NULL ||
			m_lpNavigate					== NULL ||
			m_lpNavigateOnTab				== NULL ||
			m_lpStop						== NULL ||
			m_lpStopOnTab					== NULL ||
			m_lpReload					== NULL ||
			m_lpReloadOnTab				== NULL ||
			m_lpZoom						== NULL ||
			m_lpZoomOnTab					== NULL ||
			m_lpZoomText					== NULL ||
			m_lpZoomTextOnTab				== NULL ||
			m_lpScrollbars_HPosSet		== NULL ||
			m_lpScrollbars_HPosSetOnTab	== NULL ||
			m_lpScrollbars_HPosGet		== NULL ||
			m_lpScrollbars_HPosGetOnTab	== NULL ||
			m_lpScrollbars_VPosSet		== NULL ||
			m_lpScrollbars_VPosSetOnTab	== NULL ||
			m_lpScrollbars_VPosGet		== NULL ||
			m_lpScrollbars_VPosGetOnTab	== NULL ||
			m_lpSetScrollbars				== NULL ||
			m_lpSetScrollbarsOnTab		== NULL ||
			m_lpGetScrollbars				== NULL ||
			m_lpGetScrollbarsOnTab		== NULL ||
			m_lpTab_New					== NULL ||
			m_lpTab_Close					== NULL ||
			m_lpTab_CloseCurrent			== NULL ||
			m_lpTab_Switch				== NULL ||
		//	m_lpTab_Index					== NULL ||
			m_lpTab_Count					== NULL ||
			m_lpTab_GetTitle				== NULL ||
			m_lpTab_ShowSwitcher			== NULL ||
			m_lpJS_Invoke					== NULL ||
			m_lpJS_InvokeOnTab			== NULL ||
			m_lpJS_Exists					== NULL ||
			m_lpJS_ExistsOnTab			== NULL ||
			m_lpHistory_GoForward			== NULL ||
			m_lpHistory_GoForwardOnTab	== NULL ||
			m_lpHistory_GoBack			== NULL ||
			m_lpHistory_GoBackOnTab		== NULL ||
			m_lpSetFitToScreen			== NULL ||
			m_lpGetFitToScreen			== NULL ||
			m_lpSetClearType				== NULL ||
			m_lpGetClearType				== NULL ||
			m_lpSetJavaScript				== NULL ||
			m_lpGetJavaScript				== NULL ||
			m_lpSetImages					== NULL ||
			m_lpGetImages					== NULL ||
			m_lpSetSounds					== NULL ||
			m_lpGetSounds					== NULL ||
			m_lpSetActiveX				== NULL ||
			m_lpGetActiveX				== NULL ||
			m_lpSetBrowserGesturing		== NULL ||
			m_lpGetBrowserGesturing		== NULL ||
			m_lpSetNavigationTimeout		== NULL ||
			m_lpGetNavigationTimeout		== NULL ||
			m_lpTab_GetTitleOnTab			== NULL
			)
		{
			// Log(PB_LOG_ERROR,L"Failed to GetProcAddress for Some or All Engine DLL Functions",L"CWebKitEngine::LoadEngineDLL",__LINE__,L"Core");
			
		}
		else
		{
			//  All Exported functions successfully loaded, Create the Engine
			//  & Initialise the DLL
			GetWindowRect(m_lpGetHTMLHWND(), &m_rcViewSize);
						
			m_rcViewSize.top = 22;
			
			return TRUE;
		}
	}
	return FALSE;
}

/**
*  \author	Darryn Campbell
*  \date	January 2010 (Initial Creation)
*/
tPB_PreprocessMessage	CWebKitEngine::GetlpPreprocessMessage()
{
	return m_lpPreprocessMessage;
}

/**
*  \author	Darryn Campbell
*  \date	March 2010 (Initial Creation)
*/
BOOL CWebKitEngine::UnloadEngineDLL()
{
	m_lpCreateEngine			= NULL;
	m_lpInitEngine				= NULL;
	m_lpDeinitEngine			= NULL;
	m_lpGetHTMLHWND				= NULL;
	m_lpGetHTMLHWNDOnTab		= NULL;
	m_lpGetParentHWND			= NULL;
	m_lpRegisterForEvent		= NULL;
	m_lpPreprocessMessage		= NULL;
	m_lpNavigate				= NULL;
	m_lpNavigateOnTab			= NULL;
	m_lpStop					= NULL;
	m_lpStopOnTab				= NULL;
	m_lpReload					= NULL;
	m_lpReloadOnTab				= NULL;
	m_lpZoom					= NULL;
	m_lpZoomOnTab				= NULL;
	m_lpZoomText				= NULL;
	m_lpZoomTextOnTab			= NULL;
	m_lpScrollbars_HPosSet		= NULL;
	m_lpScrollbars_HPosSetOnTab	= NULL;
	m_lpScrollbars_HPosGet		= NULL;
	m_lpScrollbars_HPosGetOnTab	= NULL;
	m_lpScrollbars_VPosSet		= NULL;
	m_lpScrollbars_VPosSetOnTab	= NULL;
	m_lpScrollbars_VPosGet		= NULL;
	m_lpScrollbars_VPosGetOnTab	= NULL;
	m_lpSetScrollbars			= NULL;
	m_lpSetScrollbarsOnTab		= NULL;
	m_lpGetScrollbars			= NULL;
	m_lpGetScrollbarsOnTab		= NULL;
	m_lpTab_New					= NULL;
	m_lpTab_Close				= NULL;
	m_lpTab_CloseCurrent		= NULL;
	m_lpTab_Switch				= NULL;
	m_lpTab_Count				= NULL;
	m_lpTab_GetTitle			= NULL;
	m_lpTab_ShowSwitcher		= NULL;
	m_lpJS_Invoke				= NULL;
	m_lpJS_InvokeOnTab			= NULL;
	m_lpJS_Exists				= NULL;
	m_lpJS_ExistsOnTab			= NULL;
	m_lpHistory_GoForward		= NULL;
	m_lpHistory_GoForwardOnTab	= NULL;
	m_lpHistory_GoBack			= NULL;
	m_lpHistory_GoBackOnTab		= NULL;
	m_lpSetFitToScreen			= NULL;
	m_lpGetFitToScreen			= NULL;
	m_lpSetClearType			= NULL;
	m_lpGetClearType			= NULL;
	m_lpSetJavaScript			= NULL;
	m_lpGetJavaScript			= NULL;
	m_lpSetImages				= NULL;
	m_lpGetImages				= NULL;
	m_lpSetSounds				= NULL;
	m_lpGetSounds				= NULL;
	m_lpSetActiveX				= NULL;
	m_lpGetActiveX				= NULL;
	m_lpSetBrowserGesturing		= NULL;
	m_lpGetBrowserGesturing		= NULL;
	m_lpSetNavigationTimeout	= NULL;
	m_lpGetNavigationTimeout	= NULL;
	m_lpTab_GetTitleOnTab		= NULL;
	FreeLibrary(m_hEngineLib);
	m_hEngineLib = NULL;
	return TRUE;
}

//Position change notifier
//void CWebKitEngine::OnGpsPositionReceived(EngineLocation* pPosition)
//{
//	Log(PB_LOG_INFO,
//			L"New position data received from GPS",
//			L"OnGpsPositionReceived",__LINE__,L"CWebKitEngine");
//	
//	SendMessage(g_hBaseWnd, PB_NEWGPSDATA, NULL, NULL);
//}
//GPS state change notifier
//void CWebKitEngine::OnGpsStateReceived(PGPS_DEVICE pDevice)
//{
//	Log(PB_LOG_DEBUG,
//			L"New GPS state received",
//			L"OnGpsStateReceived",__LINE__,L"CWebKitEngine");
//}

/**
 * Initialise GPS Wrapper for HTML5 Geolocation support
 */
//LRESULT CWebKitEngine::InitGpsWrapper()
//{
//	LRESULT iRes = S_OK;
//	m_pLocationInterface = NULL;
//	m_pGpsWrapper = NULL;
//	if (!m_pGpsWrapper)
//	{
//		m_pGpsWrapper = new CGpsWrapper(*this);
//		if ( (!m_pGpsWrapper) || (!m_pGpsWrapper->Init()) )
//		{
//			Log(PB_LOG_WARNING, L"Problem during GpsWrapper initialization, your device does not support GPS", _T(__FUNCTION__), __LINE__, L"Core");
//			return S_FALSE;
//		}
//		m_pLocationInterface = new EngineLocationInterface();
//		m_pLocationInterface->startLocationUpdates = &startLocationUpdates;
//		m_pLocationInterface->stopLocationUpdates = &stopLocationUpdates;
//		m_pLocationInterface->getCachedLocation = &getCachedLocation;
//		m_pLocationInterface->coreContext = this;
//	}
//	if (!m_lpSetLocationInterface) //The web engine does not support Geolocation (i.e. IExplorer is in use)
//		return S_FALSE;
//	LRESULT iSetLocationIntRes = m_lpSetLocationInterface(m_pLocationInterface);
//	if (iRes == S_OK) //If GpsWrapper was initialised correctly
//		iRes = iSetLocationIntRes;
//	if (iRes == S_OK)
//		Log(PB_LOG_DEBUG, L"Everything OK", _T(__FUNCTION__), __LINE__, L"Core");
//	else
//		Log(PB_LOG_ERROR, L"An error has occurred", _T(__FUNCTION__), __LINE__, L"Core");
//	return iRes;
//}

//LRESULT CWebKitEngine::DeInitGpsWrapper()
//{
//	delete m_pGpsWrapper;
//	m_pGpsWrapper = NULL;
//
//	delete m_pLocationInterface;
//	m_pLocationInterface = NULL;
//
//	return S_OK;
//}

//LRESULT CWebKitEngine::SendLocationDatoToEngine()
//{
//	if (!m_lpUpdateLocation)
//		return S_FALSE;
//	m_lpUpdateLocation(CGpsWrapper::s_pEkCachedPosition);
//	return S_OK;
//}

BOOL CWebKitEngine::Reload(bool bFromCache)
{
	return (m_lpReload(bFromCache)==S_OK);
}
