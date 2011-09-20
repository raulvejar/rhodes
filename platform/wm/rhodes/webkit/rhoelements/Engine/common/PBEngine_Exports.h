/**
 *  \file PBEngine_Exports.h
 *  \brief Definition of functions exported from the PBEngine DLL.  This file 
 *  is designed to be compatible with both the IE engine DLL and WebKit Engine 
 *  DLLs and is shared between the projects.  Future Engines should also use
 *  this file to minimise maintenance.
 */

#pragma once

#include ".\pbengine_defs.h"

//////////////////////////////////////////////
//											//
//		Set up								//
//											//
//////////////////////////////////////////////

/**
*	Initialise the Engine.  For an Example see 
*  \link CIEEngine::InitEngine \endlink
*/
LRESULT PB_InitEngine(HINSTANCE hInstance, HWND hwndParent, int iTabID, 
					  LPCTSTR tcIconURL, BoolSettingValue bsvScrollbars);

/**
*  Shut down the engine and free associated memory.  For an example see 
*  \link CIEEngine::DeinitEngine \endlink
*/
LRESULT PB_DeinitEngine();

/**
*  Create an instance of the engine object.  Note this is non functional
*  for multi-tabbed IE as IE tabs are implemented in a separate class.
*  See CWebKitEngine::CreateEngine for an example.
*/
LRESULT PB_CreateEngine();

/**
*  Register for an engine event.
*  See \link CIEEngine::RegisterForEvent \endlink for an example.
*/
LRESULT PB_RegisterForEvent(EngineEventID eeidEventID, ENGINEEVENTPROC pEventFunc);

/**
*  Instruct the Engine to process the message.  Designed to be called as part 
*  of the core message pump to process accelerator keys.
*  See \link CIEEngine::PreprocessMessage \endlink for an example.
*/
LRESULT PB_PreprocessMessage(MSG msg);

//////////////////////////////////////////////
//											//
//		Navigation							//
//											//
//////////////////////////////////////////////

/**
*  Navigate to the specified URL.  See \link CIEEngine::Navigate \endlink 
*  for an example.
*/
LRESULT PB_Navigate(LPCTSTR szURL);

/**
*  Navigate to the specified address on the specified tab.
*  See \link CIEEngine::NavigateOnTab \endlink for an example.
*/
LRESULT PB_NavigateOnTab (LPCTSTR tcAddress, int iTabID);

/**
*  Stop Navigating to the specified URL.  See \link CIEEngine::Stop \endlink 
*  for an example.
*/
LRESULT PB_Stop(void);

/**
*  Stop any navigation currently in progress in the specified tab.
*  See \link CIEEngine::StopOnTab \endlink for an example.
*/
LRESULT PB_StopOnTab (int iTabID);

/**
*  Reload the currently displayed web page.  See \link CIEEngine::Reload \endlink
*  for an example.
*/
LRESULT PB_Reload(bool bFromCache);

/**
*  Reload the page displayed in the specified tab.
*  See \link CIEEngine::ReloadOnTab \endlink for an example.
*/
LRESULT PB_ReloadOnTab (bool bFromCache, int iTabID);

/**
*  Zoom all elements on the current tab's web page in or out.  
*  See CWebKitEngine::Zoom for an example.
*/
LRESULT PB_Zoom (double dFactor);

/**
*  Zoom all elements on the specified tab's web page in or out.
*  See CWebKitEngine::ZoomOnTab for an example.
*/
LRESULT PB_ZoomOnTab (double dFactor, int iTabID);

/**
*  Zoom all elements on the specified tab's web page in or out.
*  See CWebKitEngine::GetZoomOnTab for an example.
*/
LRESULT PB_GetZoomOnTab (double *dFactor, int iTabID);

/**
*  Zoom the text in the currently displayed HTML window.  See
*  \link CIEEngine::ZoomText \endlink for an example.
*/
LRESULT PB_ZoomText(TextZoomValue dwZoomLevel);

/**
*  Zoom the text in the specified tab's HTML window.
*  See \link CIEEngine::ZoomTextOnTab \endlink for an example.
*/
LRESULT PB_ZoomTextOnTab (TextZoomValue dwZoomLevel, int iTabID);

/**
*  Zoom the text in the specified tab's HTML window.
*  See \link CIEEngine::GetZoomTextOnTab \endlink for an example.
*/
LRESULT PB_GetZoomTextOnTab (TextZoomValue *dwZoomLevel, int iTabID);

//////////////////////////////////////////////
//											//
//		HWND Accessors						//
//											//
//////////////////////////////////////////////

/** 
*  Retrieve the window handle for the HTML window in the currently visible 
*  web page.  See \link CIEEngine::GetHTMLHWND \endlink for an example.
*/
HWND PB_GetHTMLHWND();

/**
*  Retrieve the window handle for the HTML window in the specified tab's
*  web page.  See \link CIEEngine::GetHTMLHWNDOnTab \endlink for an example.
*/
HWND PB_GetHTMLHWNDOnTab(int iTabID);

/**
*  Get the window handle for the parent application.  See 
*  \link CIEEngine::GetParentHWND \endlink for an example.
*/
HWND PB_GetParentHWND();

//////////////////////////////////////////////
//											//
//		Scrollbars							//
//											//
//////////////////////////////////////////////

/**
*  Hides or shows the scrollbars.  Scrollbars can not have their visibility 
*  adjusted individually, but will only be enabled on the device if the page's 
*  contents extends outside of the visible area.
*  See \link CIEEngine::SetScrollbars \endlink for an example.
*/
BoolSettingValue PB_SetScrollBars (BoolSettingValue dwBoolSettingValue);

/**
*  Set the Scrollbar visibility on the specified tab.
*  See \link CIEEngine::SetScrollbarsOnTab \endlink for an example.
*/
BoolSettingValue PB_SetScrollBarsOnTab (BoolSettingValue dwBoolSettingValue, 
										int iTabID);

/**
*  Retrieve the visibility of the Scrollbars on the currently visible page.
*  See \link CIEEngine::GetScrollBars \endlink for an example.
*/
BoolSettingValue PB_GetScrollBars ();

/**
*  Retrive the visibility of the Scrollbars on the specified tab.
*  See \link CIEENgine::GetScrollBarsOnTab \endlink for an example.
*/
BoolSettingValue PB_GetScrollBarsOnTab (int iTabID);

/**
*  Set the position of the Horizontal scrollbar in the currently displayed tab.
*  See \link CIEEngine::Scrollbars_HPosSet \endlink for an example.
*/
LONG PB_Scrollbars_HPosSet (LONG lPos);

/**
*  Set the position of the Horizontal scrollbar in the specified Tab.
*  See \link CIEEngine::Scrollbars_HPosSetOnTab \endlink for an example.
*/
LONG PB_Scrollbars_HPosSetOnTab (LONG lPos, int iTabID);

/**
*  Get the position of the horizontal scrollbar in the currently displayed tab.
*  See \link CIEEngine::Scrollbars_HPosGet \endlink for an example.
*/
LONG PB_Scrollbars_HPosGet ();

/**
*  Perform a Horizonal Scrollbar Get on the specified tab.
*  See \link CIEEngine::Scrollbars_HPosGetOnTab \endlink for an example.
*/
LONG PB_Scrollbars_HPosGetOnTab (int iTabID);

/**
*  Set the position of the Vertical scrollbar in the currently displayed tab.
*  See \link CIEEngine::Scrollbars_VPosSet \endlink for an example.
*/
LONG PB_Scrollbars_VPosSet (LONG lPos);

/**
*  Perform a Set Position on the Vertical Scrollbar on the specified tab
*  see \link CIEEngine::Scrollbars_VPosSetOnTab \endlink for an example.
*/
LONG PB_Scrollbars_VPosSetOnTab (LONG lPos, int iTabID);

/**
*  Get the position of the Vertial scrollbar in the currently displayed tab.
*  See \link CIEEngine::Scrollbars_VPosGet \endlink for an example.
*/
LONG PB_Scrollbars_VPosGet ();

/**
*  Retrieve the position of the vertial scrollbar on the specified tab.
*  See \link CIEEngine::Scrollbars_VPosGetOnTab \endlink for an example.
*/
LONG PB_Scrollbars_VPosGetOnTab (int iTabID);

LONG PB_Scrollbars_SizeGet (int *pwidth, int *pheight);
LONG PB_Scrollbars_SizeGetOnTab (int *pwidth, int *pheight, int iTabID);

//////////////////////////////////////////////
//											//
//		Application (Tab) Management		//
//											//
//////////////////////////////////////////////

/**
*  Opens a new browser tab with the specified ID and set this tab to the visible tab.
*  See \link CIEEngine::Tab_New \endlink for an example.
*/
LRESULT PB_Tab_New (int iTabID, BOOL bBringToForeground, LPCTSTR tcIconURL,
					BoolSettingValue bsvScrollbars);

/**
*  Close the tab with the specified ID.  This method has no effect if the 
*  specified tab is the last open tab.
*  See \link CIEEngine::Tab_Close \endlink for an example.
*/
LRESULT PB_Tab_Close (int tabID);

/**
*  Close the currently displayed tab.  This method has no effect if the current
*  tab is the last open tab.
*  See \link CIEEngine::Tab_CloseCurrent \endlink for an example.
*/
LRESULT PB_Tab_CloseCurrent ();

/**
*  Change the visible tab to be the specified Tab.
*  See \link CIEEngine::Tab_Switch \endlink for an example.
*/
LRESULT PB_Tab_Switch (int iTabID);

/**
*  Resize the HTML window of the specified tab.  See
*  \link CIEEngine::Tab_Resize \endlink for an example.
*/
LRESULT PB_Tab_Resize (RECT rcNewSize, int iTabID);

/**
*  Retrieve the unique identifier of the currently visible tab.
*  See \link CIEEngine::Tab_GetID \endlink for an example.
*/
int PB_Tab_GetID ();

/**
*  Calculate the number of open tabs.  
*  See \link CIEEngine::Tab_Count \endlink for an example.
*/
UINT PB_Tab_Count ();

/**
*  Retrieve the currently displayed tab's page title, the text between the 
*  \<title\>\<\\title\> tags.
*  See \link CIEEngine::Tab_GetTitle \endlink for an example.
*/
LRESULT PB_Tab_GetTitle (LPTSTR tcTitle, int iMaxLen);

/**
*  Retrieve the currently displayed tab's page title, the text between the 
*  \<title\>\<\\title\> tags.
*  See \link CIEEngine::Tab_GetTitleOnTab \endlink for an example.
*/
LRESULT PB_Tab_GetTitleOnTab (LPTSTR tcTitle, int iMaxLen, int iTabID);

/**
*  Retrieve the currently displayed tab's application's icon, as defined
*  during creation of the Tab.
*  See \link CIEEngine::Tab_GetIconOnTab \endlink for an example.
*/
LRESULT PB_Tab_GetIconOnTab (LPTSTR tcIconURL, int iMaxLen, int iTabID);

/**
*  Show the PocketBrowser Application Switcher, See 
*  \link CIEEngine::Tab_ShowSwitcher \endlink for an example.
*/
LRESULT PB_Tab_ShowSwitcher (BOOL bShow);

/**
*  Locks or unlocks the device and displays / hides the lock screen, See
*  \link CIEEngine::Lock \endlink for an example.
*  Note this function will only succeed if a lock application has been 
*  created.
*/
LRESULT PB_Tab_Lock (BOOL bLockDevice);


//////////////////////////////////////////////
//											//
//		JavaScript							//
//											//
//////////////////////////////////////////////

/**
*  Execute the specified JavaScript function in the context of the current
*  page.  If the specified function does not exist or the page is not fully 
*  loaded then this function will have no effect.
*  See \link CIEEngine::JS_Invoke \endlink for an example.
*/
LRESULT PB_JS_Invoke (LPCTSTR tcFunction);

/**
*  Execute the specified JavaScript function in the context of the specified
*  tab.  
*  See \link CIEEngine::JS_InvokeOnTab \endlink for an example.
*/
LRESULT PB_JS_InvokeOnTab (LPCTSTR tcFunction, int iTabID);

/**
*  Determine whether the specified function exists in the context of the 
*  current page.
*  See \link CIEEngine::JS_Exists \endlink for an example
*/
LRESULT PB_JS_Exists (LPCTSTR tcFunction);

/**
*  Determine whether the specified function exists in the context of the
*  specified tab's page.
*  See \link CIEEngine::JS_ExistsONTab \endlink for an example.
*/
LRESULT PB_JS_ExistsOnTab (LPCTSTR tcFunction, int iTabID);


//////////////////////////////////////////////
//											//
//		History								//
//											//
//////////////////////////////////////////////

/**
*  Navigate forward through the browser's history in the current tab.
*  See \link CIEEngine::History_GoForward \endlink for an example.
*/
LRESULT PB_History_GoForward (UINT iNumPages);

/**
*  Navigate forward in the specified tab's history if possible.
*  See \link CIEEngine::History_GoForwardOnTab \endlink for an example.
*/
LRESULT PB_History_GoForwardOnTab (UINT iNumPages, int iTabID);

/**
*  Navigate to a previously visited page in the current tab.
*  See \link CIEEngine::History_GoBack \endlink for an example.
*/
LRESULT PB_History_GoBack (UINT iNumPages);

/**
*  Navigate to a previously visited page in the specified tab.
*  See \link CIEEngine::History_GoBackOnTab \endlink
*/
LRESULT PB_History_GoBackOnTab (UINT iNumPages, int iTabID);


//////////////////////////////////////////////
//											//
//		Accessors for Properties			//
//											//
//////////////////////////////////////////////

/** 
*  Set the Fit to Screen Attribute.
*  See \link CIEEngine::SetFitToScreen \endlink for an example.
*/
BoolSettingValue PB_SetFitToScreen(BoolSettingValue dwBoolSettingValue);

/**
*  Obtain the value of the Fit to Screen Attribute.
*  See \link CIEEngine::GetFitToScreen \endlink for an example.
*/
BoolSettingValue PB_GetFitToScreen();

/** 
*  Set the ClearType Attribute.
*  See \link CIEEngine::SetClearType \endlink for an example.
*/
BoolSettingValue PB_SetClearType(BoolSettingValue dwBoolSettingValue);

/**
*  Obtain the value of the ClearType Attribute.
*  See \link CIEEngine::GetClearType \endlink for an example.
*/
BoolSettingValue PB_GetClearType();

/** 
*  Set the JavaScript Attribute.
*  See \link CIEEngine::SetJavaScript \endlink for an example.
*/
BoolSettingValue PB_SetJavaScript(BoolSettingValue dwBoolSettingValue);
/**
*  Obtain the value of the JavaScript Attribute.
*  See \link CIEEngine::GetJavaScript \endlink for an example.
*/
BoolSettingValue PB_GetJavaScript();

/** 
*  Set the Images Attribute.
*  See \link CIEEngine::SetImages \endlink for an example.
*/
BoolSettingValue PB_SetImages(BoolSettingValue dwBoolSettingValue);

/**
*  Obtain the value of the Images Attribute.
*  See \link CIEEngine::GetImages \endlink for an example.
*/
BoolSettingValue PB_GetImages();

/** 
*  Set the Sounds Attribute.
*  See \link CIEEngine::SetSounds \endlink for an example.
*/
BoolSettingValue PB_SetSounds(BoolSettingValue dwBoolSettingValue);

/**
*  Obtain the value of the Sounds Attribute.
*  See \link CIEEngine::GetSounds \endlink for an example.
*/
BoolSettingValue PB_GetSounds();

/** 
*  Set the ActiveX Attribute.
*  See \link CIEEngine::ActiveX \endlink for an example.
*/
BoolSettingValue PB_SetActiveX(BoolSettingValue dwBoolSettingValue);

/**
*  Obtain the value of the ActiveX Attribute.
*  See \link CIEEngine::GetActiveX \endlink for an example.
*/
BoolSettingValue PB_GetActiveX();

/**
*  Enable or Disable any gesturing inherent in the underlying browser.
*  See CWebKitEngine::SetBrowserGesturing for an example.
*/
BoolSettingValue PB_SetBrowserGesturing (BoolSettingValue dwBoolSettingValue);

/**
*  Retrieve whether any gesturing inherent in the underlying browser is enabled.
*  See CWebKitEngine::GetBrowserGesturing for an example.
*/
BoolSettingValue PB_GetBrowserGesturing ();

/** 
*  Set the Navigation Timeout Attribute.
*  See \link CIEEngine::SetNavigationTimeout \endlink for an example.
*/
LRESULT PB_SetNavigationTimeout(DWORD dwTimeout);

/**
*  Obtain the value of the Navigation Timeout Attribute.
*  See \link CIEEngine::GetNavigationTimeout \endlink for an example.
*/
DWORD PB_GetNavigationTimeout();

/** 
*  Set the Accelerator Mode Attribute.
*  See \link CIEEngine::SetAcceleratorMode \endlink for an example.
*/
AcceleratorValue PB_SetAcceleratorMode(AcceleratorValue dwAcceleratorValue);

/**
*  Obtain the value of the Accelerator Mode Attribute.
*  See \link CIEEngine::GetAcceleratorMode \endlink for an example.
*/
AcceleratorValue PB_GetAcceleratorMode();

/**
 * Ekioh GeoLocation integration
 */

/**
*  Set the EngineLocationInterface used for location tracking.
*
*  \param locationInterface location interface
*
*  The pointer must remain valid for the lifetime of the engine, or until this
*  function is called again with a different value, or 0.
*/
LRESULT PB_SetLocationInterface(EngineLocationInterface* locationInterface);
 
/**
*  Update the current location when the engine has requested periodic updates.
*
*  \param location updated location
*
*  This must be called on the main thread.
*/
LRESULT PB_UpdateLocation(EngineLocation* location);


