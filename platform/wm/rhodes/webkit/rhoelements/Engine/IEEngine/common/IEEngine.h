/**
 *  \file IEEngine.h
 *  \brief Extends the CEngine class to implement a CE Internet Explorer 
 *  rendering engine and tab controller.
 */ 

#pragma once
#include "../../common/engine.h"

//  Forward declaration of Engine Tab, this allows us to return IE Engine Tabs
//  from methods of CEngine
class CIEEngine;

/**
*  Type Encapsulating an Internet Explorer Tab, an element of the doubly linked
*  tab list.
*/
struct IETab
{
	CIEEngine	 *pEngine;	///<  Associated Engine
	int	iTabID;				///<  Unique Identifier for this Tab
	BOOL bVisible;			///<  Whether the Tab is visible or not
	IETab* nextTab;			///<  Linked List of IE Tabs
	IETab* prevTab;			///<  Linked List of IE Tabs
};

/**
*  Structure used to hold information about all the tabs running.  
*  Differentiates between the types of running tabs (User Applications vs. 
*  reserved applications such as the application switcher or lock screen).
*/
struct TabCountInfo
{
	UINT totalApplications;				///< reserved + user
	UINT reservedApplications;			///< Reserved apps running, e.g. Lock Screen, App Switcher
	UINT pocketBrowserUserApplications; ///< User Applications, written by PocketBrowser customers
};

/**
 *  Main CE Internet Explorer Engine Class.
 *  Create an instance of this class to create an IE based window.  Use this 
 *  class to manage the engine attributes including the creation of Tabs.  
 *  This class inherits from CEngine.
 *  All calls into this class and its children should be made from the same 
 *  thread that created the class, this is due to a limitation of the 
 *  Microsoft HTML rendering component, see 
 *  http://msdn.microsoft.com/en-us/library/ms854241.aspx
 */
class CIEEngine :
	public CEngine
{
public:

	//////////////////////////////////////////////
	//											//
	//		Setup (Public)						//
	//											//
	//////////////////////////////////////////////

	/**
	*  Initialise the Engine.  This function should be called first in order 
	*  to create an Internet Explorer browser component, creates the first 
	*  tab and displays a blank page.  You will need to call CIEENgine::Navigate
	*  to subsequently update the location of the page.
	*  \param hInstance The Parent Window HINSTANCE
	*  \param hwndParent The Parent Window HWND
	*  \param iTabID Unique PocketBrowser identifying number for the initial tab, 
	*  used to identify the source of Callbacks and direct instructions such 
	*  as navigate to a specific engine.  
	*  \param tcIconURL URL of the Icon representing the application running in 
	*  the initial tab, leave blank if there is no associated icon.
	*  \param bsvScrollbars Whether or not Scrollbars are visible in the initial
	*  tab created in the engine
	*  \param hWndProc The WNDPROC to be associated with the HTML window which 
	*  receives all the windows messages.  This is not the HTML HWND, that 
	*  window has several child windows which are not created until some time 
	*  after initialisation, the window will be subclassed to this WNDPROC as 
	*  soon as it has been completely initialised, after which the caller 
	*  will start receiving the windows messages.
	*  \param ownerProc [out] Pointer to the HTML message window's previous 
	*  WNDPROC prior to subclassing, to enable CallWindowProc.
	*  \param configFunction Pointer to the Cores configuration reading function.
	*  \return whether or not Engine initialisation succeeded.
	*/
	LRESULT InitEngine(HINSTANCE hInstance, HWND hwndParent, int iTabID, 
		LPCTSTR tcIconURL, BoolSettingValue bsvScrollbars, 
		HTMLWndPROC_T hWndProc, WNDPROC* ownerProc, 
		ReadEngineConfigParameter_T configFunction);

	/**
	*  Destroys all tabs associated with this class and frees their memory back
	*  to the system.  Any web pages currently displayed in the tabs will be 
	*  lost.  The Engine object must have InitEngine called on it in order for 
	*  this function to succeed and InitEngine must be called again to continue
	*  running PocketBrowser applications.
	*  \return Whether or not Engine deinitialisation succeeded.
	*/
	LRESULT DeinitEngine();

	/**
	*  Specify a user defined function which will be called when an engine
	*  event occurs (such as progress indicator / meta tag parsed / document
	*  complete etc).  The same function can be specified to receive all engine
	*  callbacks or separate functions can be specified as desired.  Note 
	*  the same function will receive callbacks of the specified type from ALL
	*  tabs.  If users do not register for a EEID_TOPMOSTHWNDAVAILABLE the 
	*  RegisterWndProcThread will never exit.  Registration for events must take
	*  place after the engine has been initialised.
	*  \param eeidEventID The engine event being registered for.
	*  \param pEventFunc The user specified function which will receive the 
	*  engine callbacks, note this user function should return S_OK if it 
	*  successfully deals with the engine event.
	*  \return Whether or not the engine event was successfully registered for 
	*  ALL running tabs.
	*/
	LRESULT RegisterForEvent(EngineEventID eeidEventID, ENGINEEVENTPROC pEventFunc);

	/**
	*  Instruct the Engine of the visible tab to process the message.  
	*  Designed to be called as part of the core message pump to process 
	*  accelerator keys.
	*  \param msg The message to be translated and potentially other processing
	*  \return Whether or not the key should continue to be processed by 
	*  the message pump
	*/
	LRESULT PreprocessMessage(MSG msg);

	//////////////////////////////////////////////
	//											//
	//		Navigation (Public)					//
	//											//
	//////////////////////////////////////////////

	/**
	*  Instruct the currently visible tab to navigate to the specified web page 
	*  \param tcAddress URL to navigate to.  This parameter can be either a URL, a 
	*  JavaScript function or 'history:back' which invokes the history class
	*  to maintain backwards compatibility with PB2.x
	*  \return Whether or not the navigation succeeded.
	*/
	LRESULT Navigate(LPCTSTR tcAddress);

	/**
	*  Perform a \ref Navigate on the specified tab
	*  \param tcAddress See Navigate
	*  \param iTabID the Tab on which to perform the navigation
	*  \return see Navigate
	*/
	LRESULT NavigateOnTab (LPCTSTR tcAddress, int iTabID);

	/**
	*  Stop any navigation currently in progress in the visible tab.
	*  \return Whether or not the current navigation was successfully stopped.
	*/
	LRESULT Stop(void);

	/**
	*  Perform a \ref Stop on the specified tab
	*  \param iTabID the Tab to stop navigating on
	*  \return see Stop
	*/
	LRESULT StopOnTab (int iTabID);

	/**
	*  Reload the page in the currently visible tab.
	*  \param bFromCache Indicates whether or not to reload the web page from 
	*  the Internet Explorer cache, set this value to false to reload the page
	*  from the server.
	*  \return Whether or not the page was successfully reloaded.
	*/
	LRESULT Reload(BOOL bFromCache);

	/**
	*  Perform a \ref Reload on the specified tab
	*  \param bFromCache See Reload
	*  \param iTab the Tab to reload
	*  \return see Reload
	*/
	LRESULT ReloadOnTab (BOOL bFromCache, int iTabID);

	/**
	*  Zoom the text in the currently visible tab to the specified level.  Note
	*  this value will persist for newly created tabs but if changed in one tab 
	*  will not affect the others, i.e. 
	*  Engine creates tab A and tab B, both will be created with the last 
	*  used textSize.  Engine then increases the size of tab B's text to 
	*  'LARGEST', tab A will still have the default text and tab B will have 
	*  very large text.  Engine creates tab C, this tab is now created with 
	*  very large text, if the engine reduces the size of tab C's text to 'SMALLER'
	*  the tab will have small text but tab B will still have very large text.
	*  \param dwZoomLevel the level to zoom the text to, these map to the 
	*  Internet Explorer values.
	*  \return whether or not the text size was successfully set.
	*/
	LRESULT ZoomText(TextZoomValue dwZoomLevel);

	/**
	*  Perform a \ref ZoomText on the specified tab
	*  \param dwZoomLevel See ZoomText
	*  \param iTab the tab to zoom the text on
	*  \return See ZoomText
	*/
	LRESULT ZoomTextOnTab (TextZoomValue dwZoomLevel, int iTabID);

	/**
	*  Retrieve the Current level of Text zoom in the specified Tab
	*  \param *dwZoomLevel [out] Current level of page zoom.
	*  \param iTab the tab to retrieve the text zoom on
	*  \return Whether or not the text size was successfully retrieved
	*/
	LRESULT GetZoomTextOnTab (TextZoomValue *dwZoomLevel, int iTabID);

	//////////////////////////////////////////////
	//											//
	//		HWND Accessors (Public)				//
	//											//
	//////////////////////////////////////////////

	/**
	*  Obtain the HWND of the currently visible HTML window.
	*  \return HWND of currently visible HTML window or NULL on error.
	*/
	HWND GetHTMLHWND();

	/**
	*  Obtain the HWND of the specified tab's HTML window.
	*  \return HWND of specified tab's HTML window or NULL on error.
	*/
	HWND GetHTMLHWNDOnTab(int iTabID);

	/**
	*  Obtain the Parent HWND for the engine class, this will be identical 
	*  for all engine tabs (as they all have the same parent)
	*  \return Parent HWND or NULL on error
	*/
	HWND GetParentHWND();


	//////////////////////////////////////////////
	//											//
	//		Scrollbars (Public)					//
	//											//
	//////////////////////////////////////////////

	/**
	*  Sets the visibility of scrollbars on the current page, note scrollbars
	*  may be disabled if the page HTML does not render outside the confines
	*  of the screen.  The scrollbars will appear after the next navigate request.
	*  This method is not applicable to Windows Mobile
	*  \param dwBoolSettingValue The value to set the Scrollbar visibility to,
	*  SETTING_ON shows the scrollbars if needed, 
	*  SETTING_OFF never shows the scrollbars.
	*  \return The current visibility setting of the Scrollbars (Vertical & Horizontal)
	*/
	BoolSettingValue SetScrollBars (BoolSettingValue dwBoolSettingValue);

	/**
	*  Performs a \ref SetScrollBars on the 
	*  specified tab.
	*  This method is not applicable to Windows Mobile
	*  \param dwBoolSettingValue See SetScrollBars
	*  \param iTabID Tab whose scrollbars are being shown / hidden
	*  \return See SetScrollBars
	*/
	BoolSettingValue SetScrollBarsOnTab (BoolSettingValue dwBoolSettingValue, 
											int iTabID);

	/**
	*  Obtain whether the scrollbars will be shown if the pages contents extends 
	*  outside of the visible window.
	*  \return TRUE if the scrollbars are shown when needed, FALSE if they are never 
	*  shown.
	*/
	BoolSettingValue GetScrollBars ();

	/**
	*  Perform a \ref GetScrollBars on the 
	*  specified tab.
	*  \param iTabID Tab whose scrollbar visibility is being obtained 
	*  \return See GetScrollBars
	*/
	BoolSettingValue GetScrollBarsOnTab (int iTabID);

	/**
	*  Set the position of the Horizontal scrollbar in the currently displayed tab.
	*  \param lPos The position to set the scrollbar to, expressed in terms
	*  of pixels
	*  \return the actual position of the scrollbar after scrolling, or -1 on error
	*/
	LONG Scrollbars_HPosSet (LONG lPos);

	/**
	*  Perform a \ref Scrollbars_HPosSet
	*  on the specified tab
	*  \param lPos See Scrollbars_HPosSet
	*  \param iTab Tab whose scrollbar is being set
	*  \return See Scrollbars_HPosSet
	*/
	LONG Scrollbars_HPosSetOnTab (LONG lPos, int iTabID);

	/**
	*  Get the position of the horizontal scrollbar in the currently displayed tab.
	*  \return current horizontal scrollbar position in pixels.  -1 on Error.
	*/
	LONG Scrollbars_HPosGet ();

	/**
	*  Perform a \ref Scrollbars_HPosGet 
	*  on the specified tab
	*  \param iTab Tab whose horizontal scrollbar position is being obtained
	*  \return See Scrollbars_HPosGetOnTab
	*/
	LONG Scrollbars_HPosGetOnTab (int iTabID);

	/**
	*  Set the position of the Vertical scrollbar in teh currently displayed tab.
	*  \param lPos The position to set the scrollbar to, expressed in terms
	*  of pixels
	*  \return the actual position of the scrollbar after scrolling, or -1 on error
	*/
	LONG Scrollbars_VPosSet (LONG lPos);

	/**
	*  Perform a \ref Scrollbars_VPosSet 
	*  on the specified tab
	*  \param lPos See Scrollbars_VPosSet
	*  \param iTab Tab whose vertical scrollbar position is being obtained
	*  \return See Scrollbars_VPosSet
	*/
	LONG Scrollbars_VPosSetOnTab (LONG lPos, int iTabID);

	/**
	*  Get the position of the vertial scrollbar in the currently displayed tab.
	*  \return current vertical scrollbar position in pixels.  -1 on Error
	*/
	LONG Scrollbars_VPosGet ();

	/**
	*  Perform a \ref Scrollbars_VPosGet 
	*  on the specified tab.
	*  \param iTab the tab whose vertical scrollbar position is being obtained
	*  \return See Scrollbars_VPosGet
	*/
	LONG Scrollbars_VPosGetOnTab (int iTabID);

	virtual BOOL Scrollbars_SizeGet (int *pwidth, int *pheight);
	virtual BOOL Scrollbars_SizeGetOnTab (int *pwidth, int *pheight, int iTabID);

	//////////////////////////////////////////////////
	//												//
	//		Application (Tab) Management (Public)	//
	//												//
	//////////////////////////////////////////////////

	/**
	*  Opens a new browser tab and set this tab to the current tab.
	*  The text
	*  zoom of the newly created tab will be equal to the last value set or
	*  the current tab's settings (if no value was previously set).
	*  \param iTabID PocketBrowser Identifier for the Tab
	*  \param bBringToForeground Whether or not the newly created tab should
	*  be put at the top of the Z order.
	*  \param tcIconURL URL of the icon to be associated with the application 
	*  running in the new tab, leave blank if there is no associated icon.
	*  \param bsvScrollbars Whether or not Scrollbars are visible in the newly 
	*  created tab.
	*  \param hWndProc The WNDPROC to be associated with the HTML window which 
	*  receives all the windows messages.  This is not the HTML HWND, that 
	*  window has several child windows which are not created until some time 
	*  after initialisation, the window will be subclassed to this WNDPROC as 
	*  soon as it has been completely initialised, after which the caller 
	*  will start receiving the windows messages.
	*  \param ownerProc [out] Pointer to the HTML message window's previous 
	*  WNDPROC prior to subclassing, to enable CallWindowProc.
	*  \param configFunction Pointer to the Cores configuration reading function.
	*  \return Whether the tab was successfully opened or not.
	*/
	LRESULT Tab_New (int iTabID, BOOL bBringToForeground, LPCTSTR tcIconURL, 
		BoolSettingValue bsvScrollbars, HTMLWndPROC_T hWndProc, 
		WNDPROC* ownerProc, ReadEngineConfigParameter_T configFunction);

	/**
	*  Close the specified tab.  This function will fail if this is 
	*  the last user tab and there is no APP selector application or if this is
	*  the last tab.
	*  \param iTabID Unique identifier of the tab to close.  
	*  Requesting to close a non existant tab will have no effect.
	*  \return Whether the tab was successfully closed or not.
	*/
	LRESULT Tab_Close (int iTabID);

	/**
	*  Close the currently displayed tab.  This function will fail if this is 
	*  the last user tab and there is no APP selector application or if this is
	*  the last tab.
	*  \return Whether the tab was successfully closed or not.
	*/
	LRESULT Tab_CloseCurrent ();

	/**
	*  Change the visible tab
	*  \param iTabID Unique identifier of the tab to show, unique identifiers 
	*  are assigned to the tabs at initialisation.  
	*  Requesting to show a non existant tab will
	*  have no effect.
	*  \return Whether the tab was successfully shown or not.
	*/
	LRESULT Tab_Switch (int iTabID);

	/**
	*  Resizes the HTML window of the specified Tab
	*  \param rcNewSize RECT specifyin the new size for the specified tab's 
	*  window.
	*  \return Whether or not the resize was successful
	*/
	LRESULT Tab_Resize(RECT rcNewSize, int iTabID);

	/**
	*  Retrieve the ID of the currently displayed tab.
	*  \return currently displayed tab's unique identifier, unique identifiers
	*  are assigned to the tabs at initialisation.  
	*  \return -1 on error.
	*/
	int Tab_GetID ();

	/**
	*  Calculate the number of open tabs.  Each open tab on the device is consuming
	*  resources.
	*  \return Number of open tabs or -1 on error.
	*/
	UINT Tab_Count ();

	/**
	*  Retrieve the currently displayed tab's page title, the text between the 
	*  \<title\>\<\\title\> tags.
	*  \param tcTitle [out] string which will be set to the page's title.
	*  \param iMaxLen The maximum number of characters to copy into tcTitle.
	*  \return Whether or not the string was obtained successfully
	*/
	LRESULT Tab_GetTitle (LPTSTR tcTitle, int iMaxLen);

	/**
	*  Retrieve the currently displayed tab's page title, the text between the 
	*  \<title\>\<\\title\> tags.
	*  \param tcTitle [out] string which will be set to the page's title.
	*  \param iMaxLen The maximum number of characters to copy into tcTitle.
	*  \param iTabID The unique identifier of the Tab whose title to obtain.
	*  \return Whether or not the string was obtained successfully
	*/
	LRESULT Tab_GetTitleOnTab (LPTSTR tcTitle, int iMaxLen, int iTabID);

	/**
	*  Retrieve the specified tab's application's icon URL, that is the icon 
	*  which is to be shown on the application switcher.
	*  \param tcTitle [out] string which will be set to the application's Icon URL.
	*  \param iMaxLen The maximum number of characters to copy into tcTitle.
	*  \param iTabID The unique identifier of the Tab whose icon to obtain.
	*  \return Whether or not the string was obtained successfully
	*/
	LRESULT Tab_GetIconOnTab (LPTSTR tcIconURL, int iMaxLen, int iTabID);

	/**
	*  Show the PocketBrowser application switcher, always defined as the 
	*  application instance with Identifier APP_ID_APP_SWITCHER.
	*  \param bShow Set to TRUE to show the application switcher, set to FALSE
	*  to hide the application switcher.
	*  \return Whether or not it was possible to display the Application 
	*  switcher.
	*/
	LRESULT Tab_ShowSwitcher (BOOL bShow);

	/**
	*  Locks or unlocks the device and displays / hides the lock screen.
	*  Note this function will only succeed if a lock application has been 
	*  created.
	*  \param bLockDevice Set to TRUE to lock the device, set to FALSE to 
	*  unlock the device.
	*  \return Whether or not the device was successfully locked
	*/
	LRESULT Tab_Lock (BOOL bLockDevice);


	//////////////////////////////////////////////
	//											//
	//		JavaScript (Public)					//
	//											//
	//////////////////////////////////////////////

	/**
	*  Execute the specified JavaScript function in the context of the current
	*  page.  If the specified function does not exist or the page is not fully 
	*  loaded then this function will have no effect.
	*  e.g. 
	*  \code JS_Invoke(L"alert('hello world')");
	*  \endcode
	*  will output a JavaScript alert
	*  \param tcFunction name of the function to execute, this can be an inbuilt 
	*  function such as 'alert' or a function defined on the page between the 
	*  \<script\>\<\\script\> tags or in a .js file.
	*  \todo Whether it will be possible to obtain the JavaScript Function return value???
	*  \return Whether the function exists or not
	*/
	LRESULT JS_Invoke (LPCTSTR tcFunction);

	/**
	*  Perform a \ref JS_Invoke on the specified tab
	*  \param tcFunction See JS_Invoke
	*  \param iTab the tab where the JavaScript function should be executed
	*  \return See JS_Invoke
	*/
	LRESULT JS_InvokeOnTab (LPCTSTR tcFunction, int iTabID);

	/**
	*  Determine whether the specified function exists in the context of the 
	*  current page.
	*  \param tcFunction Name of the function to search for on the current page.
	*  \return Whether the function exists or not.
	*/
	LRESULT JS_Exists (LPCTSTR tcFunction);

	/**
	*  Perform a \ref JS_Exists on the specified tab
	*  \param tcFunction See JS_Exists
	*  \param iTab The tab in which the existance of the specified JavaScript 
	*              function should be tested
	*  \return See JS_Exists
	*/
	LRESULT JS_ExistsOnTab (LPCTSTR tcFunction, int iTabID);


	//////////////////////////////////////////////
	//											//
	//		History (Public)					//
	//											//
	//////////////////////////////////////////////

	/**
	*  If possible navigate forward through the browser's history in the current 
	*  tab.  This will only 
	*  be possible if the user had previously performed a History_GoBack, 
	*  just as in any modern browser.
	*  If it is not possible to navigate forward then this function fails silently.
	*  \param iNumPages The number of pages to navigate forward
	*  \return Whether or not the History command was successful
	*/
	LRESULT History_GoForward (UINT iNumPages);

	/**
	*  Perform a \ref History_GoForward on
	*  the specified tab.
	*  \param iNumPages The number of pages to navigate forward
	*  \param iTab The tab in which to navigate forward.
	*  \return Whether or not the History command was successful
	*/
	LRESULT History_GoForwardOnTab (UINT iNumPages, int iTabID);

	/**
	*  Navigate to the previously visited page in the current tab, analogous to 
	*  pressing the 'back' button in any modern browser.
	*  If it is not possible to navigate back, i.e. this is the first page loaded
	*  in this tab, this function fails silently.
	*  \param iNumPages The number of pages to navigate back
	*  \return Whether or not the History command was successful
	*/
	LRESULT History_GoBack (UINT iNumPages);

	/**
	*  Perform a \ref History_GoBack on the 
	*  specified tab.
	*  \param iNumPages The number of pages to navigate back.
	*  \param iTab The tab in which to navigate back.
	*  \return Whether or not the History command was successful
	*/
	LRESULT History_GoBackOnTab (UINT iNumPages, int iTabID);


	//////////////////////////////////////////////
	//											//
	//		Accessors for Properties (Public)	//
	//											//
	//////////////////////////////////////////////

	/**
	*  SetFitToScreen is not currently implemented for the Internet Explorer
	*  Engine.
	*  \param dwBoolSettingValue Whether or not to enable this property, 
	*  SETTING_ON enables this property whereas SETTING_OFF disables it.
	*  \return NOT_IMPLEMENTED
	*/
	BoolSettingValue SetFitToScreen (BoolSettingValue dwBoolSettingValue);

	/**
	*  GetFitToScreen is not currently implemented for the Internet Explorer
	*  Engine.
	*  \return NOT_IMPLEMENTED
	*/
	BoolSettingValue GetFitToScreen();

	/**
	*  SetClearType enables or disables ClearType&reg; for HTML text rendering
	*  on all tabs.
	*  \param dwBoolSettingValue Whether or not to enable this property, 
	*  SETTING_ON enables this property whereas SETTING_OFF disables it.
	*  \return Whether or not ClearType is Enabled
	*/
	BoolSettingValue SetClearType(BoolSettingValue dwBoolSettingValue);

	/**
	*  GetClearType retrieves whether or not ClearType&reg; for HTML text
	*  rendering is enabled on all tabs.
	*  \return Whether or not ClearType is enabled.
	*/
	BoolSettingValue GetClearType();

	/**
	*  SetJavaScript Enables or Disables JavaScript on the loaded
	*  page.  Note this method has no effect on pages that were loaded before 
	*  it was called, it only affects pages loaded after the call
	*  \param dwBoolSettingValue Whether or not to enable this property, 
	*  SETTING_ON enables this property whereas SETTING_OFF disables it.
	*  \return Whether or not JavaScript is enabled
	*/
	BoolSettingValue SetJavaScript(BoolSettingValue dwBoolSettingValue);

	/**
	*  GetJavaScript retrieves whether or not JavaScript is enabled on the 
	*  loaded page.
	*  \return Whether or not JavaScript is enabled.
	*/
	BoolSettingValue GetJavaScript();

	/**
	*  SetImages is not currently implemented for the Internet Explorer
	*  Engine.
	*  \param dwBoolSettingValue Whether or not to enable this property, 
	*  SETTING_ON enables this property whereas SETTING_OFF disables it.
	*  \return NOT_IMPLEMENTED
	*/
	BoolSettingValue SetImages(BoolSettingValue dwBoolSettingValue);

	/**
	*  GetImages is not currently implemented for the Internet Explorer
	*  Engine.
	*  \return NOT_IMPLEMENTED
	*/
	BoolSettingValue GetImages();

	/**
	*  SetSounds is not currently implemented for the Internet Explorer
	*  Engine.
	*  \param dwBoolSettingValue Whether or not to enable this property, 
	*  SETTING_ON enables this property whereas SETTING_OFF disables it.
	*  \return NOT_IMPLEMENTED
	*/
	BoolSettingValue SetSounds(BoolSettingValue dwBoolSettingValue);

	/**
	*  GetSounds is not currently implemented for the Internet Explorer
	*  Engine.
	*  \return NOT_IMPLEMENTED
	*/
	BoolSettingValue GetSounds();

	/**
	*  SetActiveX is not currently implemented for the Internet Explorer
	*  Engine.  It is set as constantly enabled.
	*  \param dwBoolSettingValue Whether or not to enable this property, 
	*  SETTING_ON enables this property whereas SETTING_OFF disables it.
	*  \return SETTING_ON
	*/
	BoolSettingValue SetActiveX(BoolSettingValue dwBoolSettingValue);

	/**
	*  GetActiveX is not currently implemented for the Internet Explorer
	*  Engine.  It is set asconstantly enabled
	*  \return SETTING_ON
	*/
	BoolSettingValue GetActiveX();

	/**
	*  Set the Torch Mobile Widget User Interface Gesturing on or off, such as 
	*  swiping left moves the view window to the right.
	*  \param dwBoolSettingValue SETTING_ON turns gesturing on, SETTING_OFF turns 
	*  gesturing off.
	*/
	BoolSettingValue SetBrowserGesturing (BoolSettingValue dwBoolSettingValue);

	/**
	*  Obtain whether the Torch Mobile Widget User Interface gesturing is turned
	*  on or off.
	*  \return SETTING_ON indicates swiping across the screen will navigate around
	*  the currently diaplayed page.  SETTING_OFF indicates swiping across the 
	*  screen will have no effect.
	*/
	BoolSettingValue GetBrowserGesturing ();

	/**
	*  Set the time given for the server to respond to a Navigation request 
	*  before the engine tab times out, gives up and raises an 'EEID_NAVIGATIONTIMEOUT'
	*  engine event.  Note the navigtion timeout is common across all engine 
	*  tabs.
	*  \param dwTimeout The time, in milliseconds, allowed for the server to
	*  respond to navigation requests.
	*  \return Whether or not the navigation timeout was successfully set for 
	*  ALL tabs.
	*/
	LRESULT SetNavigationTimeout(DWORD dwTimeout);

	/**
	*  Retrieve the current navigation timeout, i.e. the time after which 
	*  a non response from a server after a navigation request will trigger
	*  an 'EEID_NAVIGATIONTIMEOUT' engine event.
	*  \return the current value of the navigation timeout, in milliseconds.
	*/
	DWORD GetNavigationTimeout();

	/**
	*  SetAcceleratorMode is not currently implemented for the Internet Explorer
	*  Engine.
	*  \param dwAcceleratorValue Used to specify which Accelerator Key profile
	*  to use.  ACCELERATE_OFF disables all accelerate keys, ACCELERATE_NORM 
	*  implements 'normal' PocketBrowser behaviour, ACCELERATE_ON enables
	*  all accelerate keys.  See the help file for more information.
	*  \return ACCELERATE_NOT_IMPLEMENTED
	*/
	AcceleratorValue SetAcceleratorMode(AcceleratorValue dwAcceleratorValue);

	/**
	*  GetAcceleratorMode is not currently implemented for the Internet Explorer
	*  Engine.  It is set as constantly enabled
	*  \return ACCELERATE_NOT_IMPLEMENTED
	*/
	AcceleratorValue GetAcceleratorMode();
	
protected:

	/**
	*  This virtual function is designed to be overridden by an Internet Explorer
	*  tab class, see CIEEngineTab::GetEngineEvents for an example.
	*  \author	Darryn Campbell (DCC, JRQ768)
	*  \date		October 2009
	*/
	virtual ENGINEEVENTPROC* GetEngineEvents() {return NULL;};
	
	/**
	*  This virtual function is designed to be overridden by an Internet Explorer
	*  tab class, see CIEEngineTab::GetTabID for an example.
	*  \author	Darryn Campbell (DCC, JRQ768)
	*  \date		November 2009
	*/
	virtual int GetTabID() {return 0;};
	
	/**
	*  This virtual function is designed to be overridden by an Internet Explorer
	*  tab class, see CIEEngineTab::Tab_Resize for an example.
	*  \author	Darryn Campbell (DCC, JRQ768)
	*  \date		October 2009
	*/
	virtual LRESULT Tab_Resize(RECT rcNewSize) {return S_FALSE;};

	/**
	*  This virtual function is designed to be overridden by an Internet Explorer
	*  tab class, see CIEEngineTab::Tab_GetIcon for an example.
	*  \author	Darryn Campbell (DCC, JRQ768)
	*  \date		October 2009
	*/
	virtual LRESULT Tab_GetIcon (LPTSTR tcIconURL, int iMaxLen) {return S_FALSE;};

	/**
	*  This virtual function is designed to be overridden by an Internet Explorer
	*  tab class, see CIEEngineTab::GetZoomText for an example.
	*  \author	Darryn Campbell (DCC, JRQ768)
	*  \date	October 2009
	*/
	virtual LRESULT GetZoomText(TextZoomValue *dwZoomLevel) {return S_FALSE;};


	//////////////////////////////////////////////
	//											//
	//		Protected Functions					//
	//											//
	//////////////////////////////////////////////

	/**
	*  Return a pointer to the tab whose HTML window is currently visible.
	*  \return Pointer to currently visible tab
	*  \return NULL on error
	*/
	CIEEngine* GetVisibleTabEngine();
	/**
	*  Return a pointer to the tab with the specified ID (assigned during 
	*  initialisation.
	*  \return Pointer to the specified tab.
	*  \return NULL if the specified tab does not exist.
	*/
	CIEEngine* GetSpecificTabEngine(int tabID);

	/**
	*  Function to return a pointer to the IETab structure representing the 
	*  specified application identifier.  The list of running applications will 
	*  be iterated over until the appropriate ID is found.
	*  \param iTabID Unique identifier of the application to search for
	*  \return Pointer to the IETab structure representing the specified 
	*  application.
	*/
	IETab* GetSpecificTab(int iTabID);

	/**
	*  Return the tab ID of the currently visible tab.
	*  \return ID of the currently visible tab.
	*/
	int GetVisibleTabID();

	/**
	*  Return the tab ID of the tab with the specified HWND.
	*  \param hwndTab HWND of the tab whose ID to return.
	*  \return Unique Identifier of the tab with the specified HWND, or 0 on 
	*  error.
	*/
	static IETab* GetSpecificTab(HWND hwndTab);

	/**
	*  Used to determine the number of running tabs, i.e. tabs who have been 
	*  initialised and are available to run a PocketBrowser application.
	*  \return Number of running tabs
	*/
	void GetRunningTabs(TabCountInfo*);

	/**
	*  Used to determine whether or not the specified tab exists, iterates over 
	*  the tab list searching for the ID
	*  \return True if the tab exists, false if it does not.
	*/
	bool TabExists(int tabID);

	/**
	*  Returns whether the specified Tab ID is reserved or not.  Reserved IDs
	*  are special in that they will not be displayed unless specifically 
	*  selected, e.g. the Lock Screen
	*  \param iTabID The Tab Identifier to be compared to a Reserved ID
	*  \return whether or not iTabID is a reserved Identifier
	*/
	BOOL Tab_IsReservedID (int iTabID);

	/**
	*  Hide the specified Tab and bring another tab to the foreground.
	*  In order for this function to succeed the specified tab must exist and 
	*  be the currently visible tab and there must be more than one tab running.
	*  \param iTabID unique identifier of the tab to hide.
	*  \return Whether or not the specified tab was successfully hidden.
	*/
	LRESULT Tab_Hide (int iTabID);

	/**
	*  Function to determine the next tab to show if the selected tab is hidden.
	*  If there are multiple PocketBrowser applications running the next tab to
	*  show will be the next tab in the tab list, if this is the last tab
	*  then the application switcher will be shown.  If there are no PB 
	*  applications running and the app switcher is instructed to be hidden 
	*  this function will return FALSE.
	*  \param iTabIDBeingHidden Identifier of the Tab which is to be hidden
	*  \param iTabIDToShow [out] This parameter will be populated with the 
	*  ID of the tab to show once the iTabIDBeingHidden has been hidden.
	*  \return Whether or not the iTabIDToShow value should be trusted, i.e. 
	*  whether or not there is a valid tab id to show after the specified tab
	*  ID has been hidden.
	*/
	BOOL Tab_NextTabToShow(int iTabIDBeingHidden, int* iTabIDToShow);

	/**
	*  The HTML Window created by the Internet Explorer component (or 
	*  window on Windows Mobile) is actually a collection of child windows 
	*  the lowest of which receives all the windows messages.  When creating 
	*  the HTML component we are only given the HWND of the topmost IE window
	*  (m_hwndHTML).  This function is responsible for finding the child window
	*  which receives windows messages and subclassing it with the function
	*  specified by the calling application.  This needs to be performed in a 
	*  separate thread as the child windows are not created immediately and 
	*  their creation does not block the calling application, therefore this 
	*  thread continuously searches the window tree for the desired window 
	*  and when found that window is subclassed.  Once subclassed this function 
	*  exits.
	*  \param lpParameter Pointer to the engine whose HTML message window 
	*  is to be subclassed.
	*  \return 0 on exit.
	*/
	static DWORD WINAPI RegisterWndProcThread( LPVOID lpParameter );

protected:	//  Attributes
	/**
	*  Doubly Linked list of the currently running tabs.  Declared as static 
	*  so can be accessed by IE Mobile tabs who need to resolve messages 
	*  from the engine based on their associated HWND.  There will only ever 
	*  be one list of tabs.
	*/
	static IETab*		m_ieTabList;	

private:	//  Attributes
	BOOL		m_bDeviceLocked;				///< Whether or not the device is Locked
	int			m_tabIDOfTopUserApplication;	///< Even if the App Switcher or Lock Screen is shown this will hold the identifier of the last requested user application
	HTMLWndPROC_T	m_hWndProc;					///< The WndProc to be associated with the Lowest HTML Window  (The window that receives all the messages and is subclassed)
	WNDPROC*		m_ownerProc;				///< Pointer to the WndProc of the Lowest HTML Window prior to Subclassing.  This pointer is used by the calling application and set during subclassing.

	// GD
protected:
	HWND m_hwndApp;		// The window handle which the core passes to the plugins
	// GD


public:
	static BOOL StrContains (LPCWSTR source, LPCWSTR target);
};