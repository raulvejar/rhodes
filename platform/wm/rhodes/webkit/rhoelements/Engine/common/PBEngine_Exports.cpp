// pbengine.cpp : Defines the entry point for the DLL application.
//

#include "PBEngine_Exports.h"

//  Define these in the project settings then use different projects to build
//  either the WebKit or Internet Explorer Engines.
//  Create an engine of the applicable type for this engine build.
#ifdef PB_ENGINE_IE_CE
	#include "../IEEngine/common/IEEngine.h"
	CIEEngine* pEngine = NULL;
#elif defined (PB_ENGINE_IE_MOBILE)
	#include "../IEEngine/common/IEEngine.h"
	CIEEngine* pEngine = NULL;
#elif defined (PB_ENGINE_IE_WM65)
	#include "../IEEngine/common/IEEngine.h"
	CIEEngine* pEngine = NULL;
#elif defined (PB_ENGINE_WEBKIT)
	#include "WebKitEngine.h"
	CWebKitEngine* pEngine = NULL;
#elif defined (PB_ENGINE_EKIOH)
	#include "EkiohPBEngine.h"
	CEkiohPBEngine* pEngine = NULL;
#endif

//  Determine whether or not to allow Multiple Instances in the Engine, 
//  depending on whether PB_ENGINE_SINGLE_INSTANCE is defined or not, if it 
//  is not defined then assume the engine is being built to be multiple 
//  instance compatible.
#ifdef PB_ENGINE_SINGLE_INSTANCE
	BOOL g_MultipleInstanceCompatible = FALSE;
#else
	BOOL g_MultipleInstanceCompatible = TRUE;
#endif


BOOL APIENTRY DllMain( HANDLE hModule, 
                       DWORD  ul_reason_for_call, 
                       LPVOID lpReserved
					 )
{
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
	case DLL_THREAD_ATTACH:
	case DLL_THREAD_DETACH:
	case DLL_PROCESS_DETACH:
		break;
	}
    return TRUE;
}




//////////////////////////////////////////////
//											//
//		Set up								//
//											//
//////////////////////////////////////////////

/**
* \author	Darryn Campbell (DCC, JRQ768)
* \date		October 2009
*/
LRESULT PB_InitEngine(HINSTANCE hInstance, HWND hwndParent, int iTabID, 
					  LPCTSTR tcIconURL, BoolSettingValue bsvScrollbars, 
					  HTMLWndPROC_T hWndProc, WNDPROC* ownerProc, 
					  ReadEngineConfigParameter_T configFunction)
{
	//  Only Initialise the Engine if we have no current engine
	if (pEngine == NULL)
	{
#ifdef PB_ENGINE_EKIOH
		pEngine = new CEkiohPBEngine();
#else
		pEngine = new CIEEngine();
#endif
	
		return pEngine->InitEngine(hInstance, hwndParent, iTabID, 
									tcIconURL, bsvScrollbars, hWndProc, 
									ownerProc, configFunction);
	}
	else
		return S_FALSE;
}

/**
* \author	Darryn Campbell (DCC, JRQ768)
* \date		October 2009
*/
LRESULT PB_DeinitEngine()
{
	if (pEngine == NULL)
		return S_FALSE;
	else
	{
		pEngine->DeinitEngine();
		delete pEngine;
		pEngine = NULL;
		return S_OK;
	}
}

/**
* \author	Darryn Campbell (DCC, JRQ768)
* \date		October 2009
*/
LRESULT PB_CreateEngine(ReadEngineConfigParameter_T configFunction)
{
	if (pEngine != NULL)
		return pEngine->CreateEngine(configFunction);
	else
		return S_FALSE;
}

/**
* \author	Darryn Campbell (DCC, JRQ768)
* \date		October 2009
*/
LRESULT PB_RegisterForEvent(EngineEventID eeidEventID, ENGINEEVENTPROC pEventFunc) 
{ 
	if (pEngine == NULL)
		return S_FALSE;
	else
		return pEngine->RegisterForEvent(eeidEventID, pEventFunc);
}

/**
* \author	Darryn Campbell (DCC, JRQ768)
* \date		December 2009
*/
LRESULT PB_PreprocessMessage(MSG msg)
{
	if (pEngine == NULL)
		return S_FALSE;
	else
		return pEngine->PreprocessMessage(msg);
}

//////////////////////////////////////////////
//											//
//		Navigation							//
//											//
//////////////////////////////////////////////

/**
* \author	Darryn Campbell (DCC, JRQ768)
* \date		October 2009
*/
LRESULT PB_Navigate(LPCTSTR tcAddress)
{
	if (pEngine == NULL)
		return S_FALSE;
	else
		return pEngine->Navigate(tcAddress);
}

/**
* \author	Darryn Campbell (DCC, JRQ768)
* \date		October 2009
*/
LRESULT PB_NavigateOnTab(LPCTSTR tcAddress, int iTabID)
{
	if (pEngine == NULL)
		return S_FALSE;
	else
		return pEngine->NavigateOnTab(tcAddress, iTabID);
}

/**
* \author	Darryn Campbell (DCC, JRQ768)
* \date		October 2009
*/
LRESULT PB_Stop(void)
{
	if (pEngine == NULL)
		return S_FALSE;
	else
		return pEngine->Stop();
}

/**
* \author	Darryn Campbell (DCC, JRQ768)
* \date		October 2009
*/
LRESULT PB_StopOnTab (int iTabID)
{
	if (pEngine == NULL)
		return S_FALSE;
	else
		return pEngine->StopOnTab(iTabID);
}

/**
* \author	Darryn Campbell (DCC, JRQ768)
* \date		October 2009
*/
LRESULT PB_Reload(bool bFromCache)
{
	if (pEngine == NULL)
		return S_FALSE;
	else
		return pEngine->Reload(bFromCache);
}

/**
* \author	Darryn Campbell (DCC, JRQ768)
* \date		October 2009
*/
LRESULT PB_ReloadOnTab (bool bFromCache, int iTabID)
{
	if (pEngine == NULL)
		return S_FALSE;
	else
		return pEngine->ReloadOnTab(bFromCache, iTabID);
}

/**
* \author	Darryn Campbell (DCC, JRQ768)
* \date		October 2009
*/
LRESULT PB_Zoom (double dFactor)
{
	if (pEngine == NULL)
		return S_FALSE;
	else
		return pEngine->Zoom(dFactor);
}

/**
* \author	Darryn Campbell (DCC, JRQ768)
* \date		October 2009
*/
LRESULT PB_ZoomOnTab (double dFactor, int iTabID)
{
	if (pEngine == NULL)
		return S_FALSE;
	else
		return pEngine->ZoomOnTab(dFactor, iTabID);
}

/**
* \author	Darryn Campbell (DCC, JRQ768)
* \date		October 2009
*/
LRESULT PB_GetZoomOnTab (double *dFactor, int iTabID)
{
	if (pEngine == NULL)
		return S_FALSE;
	else
		return pEngine->GetZoomOnTab(dFactor, iTabID);
}

/**
* \author	Darryn Campbell (DCC, JRQ768)
* \date		October 2009
*/
LRESULT PB_ZoomText(TextZoomValue dwZoomLevel)
{
	if (pEngine == NULL)
		return S_FALSE;
	else
		return pEngine->ZoomText(dwZoomLevel);

}

/**
* \author	Darryn Campbell (DCC, JRQ768)
* \date		October 2009
*/
LRESULT PB_ZoomTextOnTab (TextZoomValue dwZoomLevel, int iTabID)
{
	if (pEngine == NULL)
		return S_FALSE;
	else
		return pEngine->ZoomTextOnTab(dwZoomLevel, iTabID);
}

/**
* \author	Darryn Campbell (DCC, JRQ768)
* \date		October 2009
*/
LRESULT PB_GetZoomTextOnTab (TextZoomValue *dwZoomLevel, int iTabID)
{
	if (pEngine == NULL)
		return S_FALSE;
	else
		return pEngine->GetZoomTextOnTab(dwZoomLevel, iTabID);
}


//////////////////////////////////////////////
//											//
//		HWND Accessors						//
//											//
//////////////////////////////////////////////

/**
* \author	Darryn Campbell (DCC, JRQ768)
* \date		October 2009
*/
HWND PB_GetHTMLHWND() 
{
	//  Return Visible HWND
	if (pEngine == NULL)
		return NULL;
	else
		return pEngine->GetHTMLHWND();
}

/**
* \author	Darryn Campbell (DCC, JRQ768)
* \date		October 2009
*/
HWND PB_GetHTMLHWNDOnTab(int iTabID)
{
	//  Return the HTML HWND of the specified tab
	if (pEngine == NULL)
		return NULL;
	else
		return pEngine->GetHTMLHWNDOnTab(iTabID);
}

/**
* \author	Darryn Campbell (DCC, JRQ768)
* \date		October 2009
*/
HWND PB_GetParentHWND() 
{
	if (pEngine == NULL)
		return NULL;
	else
		return pEngine->GetParentHWND();
}


//////////////////////////////////////////////
//											//
//		Scrollbars							//
//											//
//////////////////////////////////////////////

/**
* \author	Darryn Campbell (DCC, JRQ768)
* \date		October 2009
*/
BoolSettingValue PB_SetScrollBars(BoolSettingValue dwBoolSettingValue)
{ 
	if (pEngine == NULL)
		return NOT_IMPLEMENTED;
	else
		return pEngine->SetScrollBars(dwBoolSettingValue);
}

/**
* \author	Darryn Campbell (DCC, JRQ768)
* \date		October 2009
*/
BoolSettingValue PB_SetScrollBarsOnTab (BoolSettingValue dwBoolSettingValue, 
										int iTabID)
{
	if (pEngine == NULL)
		return NOT_IMPLEMENTED;
	else
		return pEngine->SetScrollBarsOnTab(dwBoolSettingValue, iTabID);
}

/**
* \author	Darryn Campbell (DCC, JRQ768)
* \date		October 2009
*/
BoolSettingValue PB_GetScrollBars()
{ 
	if (pEngine == NULL)
		return NOT_IMPLEMENTED;
	else
		return pEngine->GetScrollBars();
}

/**
* \author	Darryn Campbell (DCC, JRQ768)
* \date		October 2009
*/
BoolSettingValue PB_GetScrollBarsOnTab (int iTabID)
{
	if (pEngine == NULL)
		return NOT_IMPLEMENTED;
	else
		return pEngine->GetScrollBarsOnTab(iTabID);
}

/**
* \author	Darryn Campbell (DCC, JRQ768)
* \date		October 2009
*/
LONG PB_Scrollbars_HPosSet (LONG lPos)
{
	if (pEngine == NULL)
		return NOT_IMPLEMENTED;
	else
		return pEngine->Scrollbars_HPosSet(lPos);
}

/**
* \author	Darryn Campbell (DCC, JRQ768)
* \date		October 2009
*/
LONG PB_Scrollbars_HPosSetOnTab (LONG lPos, int iTabID)
{
	if (pEngine == NULL)
		return NOT_IMPLEMENTED;
	else
		return pEngine->Scrollbars_HPosSetOnTab(lPos, iTabID);
}

/**
* \author	Darryn Campbell (DCC, JRQ768)
* \date		October 2009
*/
LONG PB_Scrollbars_HPosGet ()
{
	if (pEngine == NULL)
		return NOT_IMPLEMENTED;
	else
		return pEngine->Scrollbars_HPosGet();
}

/**
* \author	Darryn Campbell (DCC, JRQ768)
* \date		October 2009
*/
LONG PB_Scrollbars_HPosGetOnTab (int iTabID)
{
	if (pEngine == NULL)
		return NOT_IMPLEMENTED;
	else
		return pEngine->Scrollbars_HPosGetOnTab(iTabID);
}

/**
* \author	Darryn Campbell (DCC, JRQ768)
* \date		October 2009
*/
LONG PB_Scrollbars_VPosSet (LONG lPos)
{
	if (pEngine == NULL)
		return NOT_IMPLEMENTED;
	else
		return pEngine->Scrollbars_VPosSet(lPos);
}

/**
* \author	Darryn Campbell (DCC, JRQ768)
* \date		October 2009
*/
LONG PB_Scrollbars_VPosSetOnTab (LONG lPos, int iTabID)
{
	if (pEngine == NULL)
		return NOT_IMPLEMENTED;
	else
		return pEngine->Scrollbars_VPosSetOnTab(lPos, iTabID);
}

/**
* \author	Darryn Campbell (DCC, JRQ768)
* \date		October 2009
*/
LONG PB_Scrollbars_VPosGet ()
{
	if (pEngine == NULL)
		return NOT_IMPLEMENTED;
	else
		return pEngine->Scrollbars_VPosGet();
}

/**
* \author	Darryn Campbell (DCC, JRQ768)
* \date		October 2009
*/
LONG PB_Scrollbars_VPosGetOnTab (int iTabID)
{
	if (pEngine == NULL)
		return NOT_IMPLEMENTED;
	else
		return pEngine->Scrollbars_VPosGetOnTab(iTabID);
}

/**
* \author	Geoff Day (GRD, XFH386)
* \date		March 2010
*/
LONG PB_Scrollbars_SizeGet (int *pwidth, int *pheight)
{
	if (pEngine == NULL)
		return NOT_IMPLEMENTED;
	else
		return pEngine->Scrollbars_SizeGet(pwidth, pheight);
}

/**
* \author	Geoff Day (GRD, XFH386)
* \date		March 2010
*/
LONG PB_Scrollbars_SizeGetOnTab (int *pwidth, int *pheight, int iTabID)
{
	if (pEngine == NULL)
		return NOT_IMPLEMENTED;
	else
		return pEngine->Scrollbars_SizeGetOnTab(pwidth, pheight, iTabID);
}

//////////////////////////////////////////////
//											//
//		Application (Tab) Management		//
//											//
//////////////////////////////////////////////

/**
* \author	Darryn Campbell (DCC, JRQ768)
* \date		October 2009
*/
LRESULT PB_Tab_New (int iTabID, BOOL bBringToForeground, LPCTSTR tcIconURL, 
					BoolSettingValue bsvScrollbars, HTMLWndPROC_T hWndProc, 
					WNDPROC* ownerProc, 
					ReadEngineConfigParameter_T configFunction)
{
	//  This method has no effect if we are compiling for Single Instance
	//  engines.
	if (pEngine == NULL || !g_MultipleInstanceCompatible)
		return S_FALSE;
	else
	{
		return pEngine->Tab_New(iTabID, bBringToForeground, tcIconURL, 
								bsvScrollbars, hWndProc, ownerProc, configFunction);
	}
}

/**
* \author	Darryn Campbell (DCC, JRQ768)
* \date		October 2009
*/
LRESULT PB_Tab_Close (int iTabID)
{
	if (pEngine == NULL)
		return S_FALSE;
	else
		return pEngine->Tab_Close(iTabID);
}

/**
* \author	Darryn Campbell (DCC, JRQ768)
* \date		October 2009
*/
LRESULT PB_Tab_CloseCurrent ()
{
	if (pEngine == NULL)
		return S_FALSE;
	else
		return pEngine->Tab_CloseCurrent();
}

/**
* \author	Darryn Campbell (DCC, JRQ768)
* \date		October 2009
*/
LRESULT PB_Tab_Switch (int iTabID)
{
	if (pEngine == NULL)
		return S_FALSE;
	else
		return pEngine->Tab_Switch(iTabID);
}

/**
* \author	Darryn Campbell (DCC, JRQ768)
* \date		October 2009
*/
LRESULT PB_Tab_Resize (RECT rcNewSize, int iTabID)
{
	if (pEngine == NULL)
		return S_FALSE;
	else
		return pEngine->Tab_Resize(rcNewSize, iTabID);
}

/**
* \author	Darryn Campbell (DCC, JRQ768)
* \date		October 2009
*/
int PB_Tab_GetID ()
{
	if (pEngine == NULL)
		return -1;
	else
		return pEngine->Tab_GetID();
}

/**
* \author	Darryn Campbell (DCC, JRQ768)
* \date		October 2009
*/
UINT PB_Tab_Count ()
{
	if (pEngine == NULL)
		return -1;
	else
		return pEngine->Tab_Count();
}

/**
* \author	Darryn Campbell (DCC, JRQ768)
* \date		October 2009
*/
LRESULT PB_Tab_GetTitle (LPTSTR tcTitle, int iMaxLen)
{
	if (pEngine == NULL)
		return S_FALSE;
	else
		return pEngine->Tab_GetTitle(tcTitle, iMaxLen);
}

/**
* \author	Darryn Campbell (DCC, JRQ768)
* \date		October 2009
*/
LRESULT PB_Tab_GetTitleOnTab (LPTSTR tcTitle, int iMaxLen, int iTabID)
{
	if (pEngine == NULL)
		return S_FALSE;
	else
		return pEngine->Tab_GetTitleOnTab(tcTitle, iMaxLen, iTabID);
}

/**
* \author	Darryn Campbell (DCC, JRQ768)
* \date		October 2009
*/
LRESULT PB_Tab_GetIconOnTab (LPTSTR tcIconURL, int iMaxLen, int iTabID)
{
	if (pEngine == NULL)
		return S_FALSE;
	else
		return pEngine->Tab_GetIconOnTab(tcIconURL, iMaxLen, iTabID);
}

/**
* \author	Darryn Campbell (DCC, JRQ768)
* \date		October 2009
*/
LRESULT PB_Tab_ShowSwitcher (BOOL bShow)
{
	//  The application switcher is not available for single instance
	//  engines so test that we're multiple instance compatible here.
	if (pEngine == NULL || !g_MultipleInstanceCompatible)
		return S_FALSE;
	else
		return pEngine->Tab_ShowSwitcher(bShow);
}

/**
* \author	Darryn Campbell (DCC, JRQ768)
* \date		October 2009
*/
LRESULT PB_Tab_Lock (BOOL bLockDevice)
{
	//  The lock screen will only be available for multiple instance
	//  engines so test here we are multiple instance compatible.
	if (pEngine == NULL || !g_MultipleInstanceCompatible)
		return S_FALSE;
	else
		return pEngine->Tab_Lock(bLockDevice);
}

//////////////////////////////////////////////
//											//
//		JavaScript							//
//											//
//////////////////////////////////////////////

/**
* \author	Darryn Campbell (DCC, JRQ768)
* \date		October 2009
*/
LRESULT PB_JS_Invoke (LPCTSTR tcFunction)
{
	if (pEngine == NULL)
		return S_FALSE;
	else
		return pEngine->JS_Invoke(tcFunction);
}

/**
* \author	Darryn Campbell (DCC, JRQ768)
* \date		October 2009
*/
LRESULT PB_JS_InvokeOnTab (LPCTSTR tcFunction, int iTabID)
{
	if (pEngine == NULL)
		return S_FALSE;
	else
		return pEngine->JS_InvokeOnTab(tcFunction, iTabID);
}

/**
* \author	Darryn Campbell (DCC, JRQ768)
* \date		October 2009
*/
LRESULT PB_JS_Exists (LPCTSTR tcFunction)
{
	if (pEngine == NULL)
		return S_FALSE;
	else
		return pEngine->JS_Exists(tcFunction);
}

/**
* \author	Darryn Campbell (DCC, JRQ768)
* \date		October 2009
*/
LRESULT PB_JS_ExistsOnTab (LPCTSTR tcFunction, int iTabID)
{
	if (pEngine == NULL)
		return S_FALSE;
	else
		return pEngine->JS_ExistsOnTab(tcFunction, iTabID);
}


//////////////////////////////////////////////
//											//
//		History								//
//											//
//////////////////////////////////////////////

/**
* \author	Darryn Campbell (DCC, JRQ768)
* \date		October 2009
*/
LRESULT PB_History_GoForward (UINT iNumPages)
{
	if (pEngine == NULL)
		return S_FALSE;
	else
		return pEngine->History_GoForward(iNumPages);
}

/**
* \author	Darryn Campbell (DCC, JRQ768)
* \date		October 2009
*/
LRESULT PB_History_GoForwardOnTab (UINT iNumPages, int iTabID)
{
	if (pEngine == NULL)
		return S_FALSE;
	else
		return pEngine->History_GoForwardOnTab(iNumPages, iTabID);
}

/**
* \author	Darryn Campbell (DCC, JRQ768)
* \date		October 2009
*/
LRESULT PB_History_GoBack (UINT iNumPages)
{
	if (pEngine == NULL)
		return S_FALSE;
	else
		return pEngine->History_GoBack(iNumPages);
}

/**
* \author	Darryn Campbell (DCC, JRQ768)
* \date		October 2009
*/
LRESULT PB_History_GoBackOnTab (UINT iNumPages, int iTabID)
{
	if (pEngine == NULL)
		return S_FALSE;
	else
		return pEngine->History_GoBackOnTab(iNumPages, iTabID);
}


//////////////////////////////////////////////
//											//
//		Accessors for Properties			//
//											//
//////////////////////////////////////////////

/**
* \author	Darryn Campbell (DCC, JRQ768)
* \date		October 2009
*/
BoolSettingValue PB_SetFitToScreen(BoolSettingValue dwBoolSettingValue)	
{ 
	if (pEngine == NULL)
		return NOT_IMPLEMENTED;
	else
		return pEngine->SetFitToScreen(dwBoolSettingValue);
}

/**
* \author	Darryn Campbell (DCC, JRQ768)
* \date		October 2009
*/
BoolSettingValue PB_GetFitToScreen()
{ 
	if (pEngine == NULL)
		return NOT_IMPLEMENTED;
	else
		return pEngine->GetFitToScreen();
}

/**
* \author	Darryn Campbell (DCC, JRQ768)
* \date		October 2009
*/
BoolSettingValue PB_SetClearType(BoolSettingValue dwBoolSettingValue)
{ 
	if (pEngine == NULL)
		return NOT_IMPLEMENTED;
	else
		return pEngine->SetClearType(dwBoolSettingValue);
}

/**
* \author	Darryn Campbell (DCC, JRQ768)
* \date		October 2009
*/
BoolSettingValue PB_GetClearType()
{ 
	if (pEngine == NULL)
		return NOT_IMPLEMENTED;
	else
		return pEngine->GetClearType();
}

/**
* \author	Darryn Campbell (DCC, JRQ768)
* \date		October 2009
*/
BoolSettingValue PB_SetJavaScript(BoolSettingValue dwBoolSettingValue)
{ 
	if (pEngine == NULL)
		return NOT_IMPLEMENTED;
	else
		return pEngine->SetJavaScript(dwBoolSettingValue);
}

/**
* \author	Darryn Campbell (DCC, JRQ768)
* \date		October 2009
*/
BoolSettingValue PB_GetJavaScript()
{ 
	if (pEngine == NULL)
		return NOT_IMPLEMENTED;
	else
		return pEngine->GetJavaScript();
}

/**
* \author	Darryn Campbell (DCC, JRQ768)
* \date		October 2009
*/
BoolSettingValue PB_SetImages(BoolSettingValue dwBoolSettingValue)
{ 
	if (pEngine == NULL)
		return NOT_IMPLEMENTED;
	else
		return pEngine->SetImages(dwBoolSettingValue);
}

/**
* \author	Darryn Campbell (DCC, JRQ768)
* \date		October 2009
*/
BoolSettingValue PB_GetImages()
{ 
	if (pEngine == NULL)
		return NOT_IMPLEMENTED;
	else
		return pEngine->GetImages();
}

/**
* \author	Darryn Campbell (DCC, JRQ768)
* \date		October 2009
*/
BoolSettingValue PB_SetSounds(BoolSettingValue dwBoolSettingValue)
{ 
	if (pEngine == NULL)
		return NOT_IMPLEMENTED;
	else
		return pEngine->SetSounds(dwBoolSettingValue);
}

/**
* \author	Darryn Campbell (DCC, JRQ768)
* \date		October 2009
*/
BoolSettingValue PB_GetSounds()
{ 
	if (pEngine == NULL)
		return NOT_IMPLEMENTED;
	else
		return pEngine->GetSounds();
}

/**
* \author	Darryn Campbell (DCC, JRQ768)
* \date		October 2009
*/
BoolSettingValue PB_SetActiveX(BoolSettingValue dwBoolSettingValue)
{ 
	if (pEngine == NULL)
		return NOT_IMPLEMENTED;
	else
		return pEngine->SetActiveX(dwBoolSettingValue);
}

/**
* \author	Darryn Campbell (DCC, JRQ768)
* \date		October 2009
*/
BoolSettingValue PB_GetActiveX()
{ 
	if (pEngine == NULL)
		return NOT_IMPLEMENTED;
	else
		return pEngine->GetActiveX();
}

/**
* \author	Darryn Campbell (DCC, JRQ768)
* \date		October 2009
*/
BoolSettingValue PB_SetBrowserGesturing (BoolSettingValue dwBoolSettingValue)
{
	if (pEngine == NULL)
		return NOT_IMPLEMENTED;
	else
		return pEngine->SetBrowserGesturing(dwBoolSettingValue);
}

/**
* \author	Darryn Campbell (DCC, JRQ768)
* \date		October 2009
*/
BoolSettingValue PB_GetBrowserGesturing ()
{
	if (pEngine == NULL)
		return NOT_IMPLEMENTED;
	else
		return pEngine->GetBrowserGesturing();
}

/**
* \author	Darryn Campbell (DCC, JRQ768)
* \date		October 2009
*/
LRESULT PB_SetNavigationTimeout(DWORD dwTimeout)
{ 
	if (pEngine == NULL)
		return S_FALSE;
	else
		return pEngine->SetNavigationTimeout(dwTimeout);
}

/**
* \author	Darryn Campbell (DCC, JRQ768)
* \date		October 2009
*/
DWORD PB_GetNavigationTimeout()
{ 
	if (pEngine == NULL)
		return -1;
	else
		return pEngine->GetNavigationTimeout();
}

/**
* \author	Darryn Campbell (DCC, JRQ768)
* \date		October 2009
*/
AcceleratorValue PB_SetAcceleratorMode(AcceleratorValue dwAcceleratorValue)
{ 
	if (pEngine == NULL)
		return ACCELERATE_NOT_IMPLEMENTED;
	else
		return pEngine->SetAcceleratorMode(dwAcceleratorValue);
}

/**
* \author	Darryn Campbell (DCC, JRQ768)
* \date		October 2009
*/
AcceleratorValue PB_GetAcceleratorMode()
{ 
	if (pEngine == NULL)
		return ACCELERATE_NOT_IMPLEMENTED;
	else
		return pEngine->GetAcceleratorMode();
}

/**
 * Ekioh GeoLocation integration
 */


LRESULT PB_SetLocationInterface(EngineLocationInterface* locationInterface)
{
	if (pEngine == NULL)
		return S_FALSE;
	else
		return pEngine->SetLocationInterface(locationInterface);
}

LRESULT PB_UpdateLocation(EngineLocation* location)
{
	if (pEngine == NULL)
		return S_FALSE;
	else
		return pEngine->UpdateLocation(location);
}