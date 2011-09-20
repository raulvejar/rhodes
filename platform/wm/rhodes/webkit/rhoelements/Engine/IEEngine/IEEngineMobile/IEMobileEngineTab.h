/**
 *  \file IEMobileEngineTab.h
 *  \brief Extends the CIEEngine class to implement a Windows Mobile 
 *  Internet Explorer rendering engine for a single tab.
 */ 

#pragma once
#include "../../IEEngine/common/IEEngine.h"
#include "../common/History.h"
#include "../../../Common/Private/RelativeURLs.h"


/**
 *  Class to define a single tab in the rendering engine component.  You should 
 *  not create instances of this object directly, tabs should be managed
 *  via CIEEngine.  This engine is designed to work on Windows Mobile devices.
 */
class CIEMobileEngineTab : 
	public CIEEngine
{
public:
	//////////////////////////////////////////////
	//											//
	//		Setup (Public)						//
	//											//
	//////////////////////////////////////////////

	/**
	*  Constructor for the CIEMobileEngineTab.  This will initialise the member 
	*  variables.
	*  \param hInstance	Parent window's HINSTANCE
	*  \param hwndParent	Parent window's Window Handle
	*  \param tabID	Identifier for this engine which can be used to identify
	*					the source of callbacks.
	*  \param tcIconURL	URL of the icon for the application loaded in this tab
	*  \param bsvScrollbars Whether or not Scrollbars are visible on the Tab
	*/
	CIEMobileEngineTab(HINSTANCE hInstance, HWND hwndParent, int tabID, 
		LPCTSTR tcIconURL, BoolSettingValue bsvScrollbars);

	/**
	*  Engine Destructor, this will destroy the HTML windows and free all memory.
	*/
	~CIEMobileEngineTab(void);

	/**
	*  Creates the HTML Container Window and ensures it has a parent to 
	*  receive messages from the container.  After creating the Engine Tab object 
	*  you will need to call this function to initialise the browser correctly.
	*  \param configFunction Pointer to the Cores configuration reading function.
	*  \return Whether or not the engine was successfully created.
	*/
	LRESULT CreateEngine(ReadEngineConfigParameter_T configFunction);

	/**
	*  Register to receive an event from the Engine.
	*  \param eeidEventID The identifier of the event being registered for.
	*  \param pEventFunc Pointer to a function which will receive notification 
	*					of the event.
	*  \return Whether or not the event was successfully registered for.
	*/
	LRESULT RegisterForEvent(EngineEventID eeidEventID, ENGINEEVENTPROC pEventFunc);

	/**
	*  Instruct the Engine to process the message.  Designed to be called as part 
	*  of the core message pump to process accelerator keys.
	*  \param msg The message to be translated and potentially other processing
	*  \preturn Whether or not the key should continue to be processed by 
	*  the message pump
	*/
	LRESULT PreprocessMessage(MSG msg);

	/**
	*  Returns a pointer to the callback functions which are called in 
	*  response to engine events eminating from this engine.  This function is 
	*  used when creating a new tab and enables the new tab to be created with
	*  the same registered events in its engine.
	*  \return Pointer to Callback functions invoked in response to this 
	*  tab's Engine Events.
	*/
	ENGINEEVENTPROC* GetEngineEvents();


	//////////////////////////////////////////////
	//											//
	//		Navigation (Public)					//
	//											//
	//////////////////////////////////////////////

	/**
	*  Instruct the WM IE engine to navigate to the specified URL.
	*  \param tcURL URI to Navigate to, can be http://, ftp://
	*				or JavaScript://
	*  \return the result of instructing the Browser Component to navigate
	*/
	LRESULT Navigate(LPCTSTR tcURL);

	/**
	*  Halts any navigation currently in progress immediately causing a 
	*  callback to indicate Document Complete.  If there is no navigation in 
	*  progress this function will have no effect.
	*  \return Whether or not a DTM_STOP message was successfully posted to the 
	*  HTML component, a return value of S_OK does not therefore necessarily 
	*  mean the HTML component successfully responded to the request.
	*/
	LRESULT Stop();

	/**
	*  Reloads the page currently displayed in the browser window.
	*  \param bFromCache Whether to reload the page from cache or not
	*  \return Whether or not the request to reload the page was successfully 
	*  sent.
	*/
	LRESULT Reload(BOOL bFromCache);

	/**
	*  Zoom the text for this tab to the specified zoom level.  
	*  \param dwZoomLevel the level to zoom the text to.
	*  \return S_OK if the zoom was successful, else S_FALSE
	*/
	LRESULT ZoomText(TextZoomValue dwZoomLevel);

	/**
	*  Retrieve the current level of Text zoom.  
	*  \param *dwZoomLevel [out] Will be set to the current level of zoom.
	*  \return S_OK if the zoom was successful, else S_FALSE
	*/
	LRESULT GetZoomText(TextZoomValue* dwZoomLevel);

	//////////////////////////////////////////////
	//											//
	//		HWND Accessors (Public)				//
	//											//
	//////////////////////////////////////////////

	/**
	*  Returns the HTML window handle of the current tab
	*  \return The HTML window handle
	*/
	HWND GetHTMLHWND();

	//////////////////////////////////////////////
	//											//
	//		Scrollbars (Public)					//
	//											//
	//////////////////////////////////////////////

	/**
	*  This method returns not implemented in the PocketBrowser IE Engine
	*  for Windows Mobile.
	*  \param dwBoolSettingValue The visibility of the Scrollbars, both 
	*							horizontal and vertical.
	*  \return NOT_IMPLEMENTED
	*/
	BoolSettingValue SetScrollBars(BoolSettingValue dwBoolSettingValue);

	/**
	*  Retrieve whether or not Scrollbars are currently visibile in the browser
	*  component.
	*  \return The visibility of the scrollbars, both horizontal and vertical.
	*/
	BoolSettingValue GetScrollBars();

	/**
	*  This method returns not implemented in the PocketBrowser IE Engine
	*  for Windows Mobile.
	*  \param lPos The position to set the scrollbar to, expressed in terms
	*  of pixels
	*  \return -1
	*/
	LONG Scrollbars_HPosSet (LONG lPos);

	/**
	*  This method returns not implemented in the PocketBrowser IE Engine
	*  for Windows Mobile.
	*  \return -1
	*/
	LONG Scrollbars_HPosGet ();

	/**
	*  This method returns not implemented in the PocketBrowser IE Engine
	*  for Windows Mobile.
	*  \param lPos The position to set the scrollbar to, expressed in terms
	*  of pixels
	*  \return -1
	*/
	LONG Scrollbars_VPosSet (LONG lPos);

	/**
	*  This method returns not implemented in the PocketBrowser IE Engine
	*  for Windows Mobile.
	*  \return -1
	*/
	LONG Scrollbars_VPosGet ();

	//////////////////////////////////////////////////
	//												//
	//		Application (Tab) Management (Public)	//
	//												//
	//////////////////////////////////////////////////

	/**
	*  Updates the location of the HTML window within the Browser component and 
	*  the HTML window's parent.
	*  \param rcNewSize New Position location for the HTML windows
	*  \return Whether or not the tab was successfully resized.
	*/
	LRESULT Tab_Resize(RECT rcNewSize);

	/**
	*  Returns the title of the page currently loaded in the engine, i.e. 
	*  the text between the &lt;TITLE&gt; and &lt;\TITLE&gt; tags or the 
	*  address of the page if no TITLE tag is specified.
	*  \param title [out] User allocated string in which to place the page title
	*  \param iMaxLen Maximum length of the string to copy into the title
	*			parameter, note the calling function is responsible for 
	*			ensuring there is sufficient buffer space available in title.
	*  \return Whether or not the string was obtained successfully
	*/
	LRESULT Tab_GetTitle (LPTSTR title, int iMaxLen);

	/**
	*  Returns the icon for the application loaded in this tab, as set during 
	*  tab creation.
	*  \param tcIconURL [out] User allocated string in which to place the icon
	*  URL
	*  \param iMaxLen Maximum length of the string to copy into the tcIconURL
	*  parameter, note the calling function is repsonsible for ensuring there 
	*  is sufficient buffer space available in the Icon URL.
	*  \return Whether or not the string was obtained successfully
	*/
	LRESULT Tab_GetIcon (LPTSTR tcIconURL, int iMaxLen);

	//////////////////////////////////////////////
	//											//
	//		JavaScript (Public)					//
	//											//
	//////////////////////////////////////////////

	/**
	*  Invoke a JavaScript Function on the current page.  The browser 
	*  component will be instructed to navigate to JavaScript://<tcFunction>
	*  \return The result of instructing the Browser Component to navigate
	*/
	LRESULT JS_Invoke (LPCTSTR tcfunction);

	/**
	*  This function is not implemented for the Windows Mobile browser engine.
	*  \return 
	*/
	LRESULT JS_Exists (LPCTSTR tcFunction);

	//////////////////////////////////////////////
	//											//
	//		History (Public)					//
	//											//
	//////////////////////////////////////////////

	/**
	*  Navigate back to a previous page in this tab's history.  If the number of 
	*  pages requested to go back exceeds the size of the history stack this
	*  function will have no effect.
	*  \param iNumPages Number of pages to navigate back.
	*  \return Whether or not the history command was successful
	*/
	LRESULT History_GoBack (UINT iNumPages);

	/**
	*  Navigate forward in this tab's history.  If the number of pages 
	*  requested to go forward exceeds the size of the history stack this 
	*  function will have no effect.  It is only possible to navigate 
	*  forward if there has previously been a call to History_GoBack.
	*  \param iNumPages The number of pages to navigate forward.
	*  \return Whether or not the history command was successful
	*/
	LRESULT History_GoForward (UINT iNumPages);


	//////////////////////////////////////////////
	//											//
	//		Accessors for Properties (Public)	//
	//											//
	//////////////////////////////////////////////

	/**
	*  SetClearType enables or disables ClearType&reg; for HTML text rendering
	*  on this tab.
	*  \param dwBoolSettingValue Whether or not to enable this property, 
	*  SETTING_ON enables this property whereas SETTING_OFF disables it.
	*  \return Whether or not ClearType is Enabled
	*/
	BoolSettingValue SetClearType(BoolSettingValue dwBoolSettingValue);

	/**
	*  GetClearType retrieves whether or not ClearType&reg; for HTML text
	*  rendering is enabled on this tab.
	*  \return Whether or not ClearType is enabled.
	*/
	BoolSettingValue GetClearType();

	/**
	*  SetJavaScript enables or disables JavaScript on this tab.
	*  \param dwBoolSettingValue Whether or not to enable this property, 
	*  SETTING_ON enables this property whereas SETTING_OFF disables it.
	*  \return Whether or not JavaScript is Enabled
	*/
	BoolSettingValue SetJavaScript(BoolSettingValue dwBoolSettingValue);

	/**
	*  GetJavaScript retrieves whether or not JavaScript is enabled on this tab.
	*  \return Whether or not JavaScript is enabled.
	*/
	BoolSettingValue GetJavaScript();

	/**
	*  Set the value of the Navigation timeout, the time given for the server 
	*  to respond to a navigation request before raising the engine event 
	*  EEID_NAVIGATIONTIMEOUT
	*  \param dwTimeout The timeout in milliseconds.
	*  \return The result of setting the timeout value.
	*/
	LRESULT SetNavigationTimeout(DWORD dwTimeout);

	/**
	*  Retrieve the navigation timeout.
	*  \return The navigation timeout value, in milliseconds or -1 on error
	*/
	DWORD GetNavigationTimeout();

	/**
	*  SetAcceleratorMode is not implemented in the Windows Mobile Tab and 
	*  will return as such
	*  \param dwAcceleratorValue Unused.
	*  \return ACCELERATE_NOT_IMPLEMENTED
	*/
	AcceleratorValue SetAcceleratorMode(AcceleratorValue dwAcceleratorValue);

	/**
	*  GetAcceleratorMode is not implemented in the Windows Mobile Tab and 
	*  will return as such
	*  \return ACCELERATE_NOT_IMPLEMENTED
	*/
	AcceleratorValue GetAcceleratorMode();
	

private:  //  Functions

	/**
	*  Thread used to determine whether or not the current page load has 
	*  exceeded the navigation timeout period, as specified by 
	*  SetNavigationTimeout
	*  \param lpParameter Pointer to the CIEEngineTab object
	*  \return 0 on completion of the thread
	*/
	static DWORD WINAPI NavigationTimeoutThread( LPVOID lpParameter );

	/**
	*  Registers the window class for the HTML container, the parent of all 
	*  HTML Controls (tabs) which receives notifications from those controls.
	*  \param hInstance HINSTANCE of the parent application
	*  \param appWndProc Message Handler for the Window
	*  \return Whether or not the class was successfully registered
	*/
	HRESULT RegisterWindowClass(HINSTANCE hInstance, WNDPROC appWndProc);

	/**
	*  Called in response to receiving a META tag from the engine.  Calls the 
	*  event handler registered by the core.
	*  \param tcHttpEquiv The HTTP-Equiv part of the Meta Tag
	*  \param tcContent the Content part of the Meta Tag
	*/
	void	InvokeEngineEventMetaTag(LPTSTR tcHttpEquiv, LPTSTR tcContent);

	/**
	*  Called in response to receiving a title change event from the engine.  
	*  Calls the event handler registered by the core.
	*  \param tcTitle The new page title.
	*/
	void	InvokeEngineEventTitleChange(LPTSTR tcTitle);

	/**
	*  Called in response to receiving a navigation / document load status, 
	*  i.e. BeforeNavigate, NavigateComplete, DocumentComplete.
	*  \param tcURL The URL being Navigated to.
	*  \param eeEventID Which event has occured.
	*/
	void	InvokeEngineEventLoad(LPTSTR tcURL, EngineEventID eeEventID);

	/**
	*  Accessor method for m_tabID
	*/
	int GetTabID();

	/**
	*  WndProc to handle messages for the HTML Container window, the window
	*  established to receive messages from the HTML components (tabs).  This 
	*  WndProc only processes messages of type WM_NOTIFY eminating from the 
	*  HTML container.  Data associated with messages is extracted and
	*  forwarded to the relevant tab.
	*  \param hwnd Window Handle
	*  \param uMsg Message Received
	*  \param wParam wParam of the received message
	*  \param lParam lParam of the received message
	*  \return S_OK
	*/
	static LRESULT WndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

private:  //  Attributes
	TCHAR	m_tcNavigatedURL[MAX_URL];		///< The current URL loaded or being navigated to
	TCHAR	m_tcCurrentPageTitle[MAX_URL];	///< The title of the currently loaded page
	TCHAR	m_tcIconURL[MAX_URL];			///< The path to the icon for the application loaded in this tab.
	HANDLE	m_hNavigated;					///< Event handle set on document complete or on navigation error, used to stop the navigation timeout thread.
	int		m_tabID;						///< The unique PocketBrowser reference for this tab (PocketBrowser Application)
	HWND	m_hwndTabHTML;					///< HTML Window Handle for this Tab's HTML Component
	BoolSettingValue m_bsvScrollBars;		///<  Whether scrollbars or visible or not
	static	HWND	m_hwndTabHTMLContainer;	///< HTML Window for this Tab's HTML Component's Parent
	ENGINEEVENTPROC	m_EngineEvents[EEID_MAXEVENTID];	///< Array of pointers to functions to call when Engine Events occur
	CHistory*	m_BrowserHistory;			///< This tab's browser history
	TextZoomValue m_dwCurrentTextZoomLevel;	///< The current level of text zoomin the tab
	BOOL	m_bPageLoaded;					///< Indication that the document complete has been received for the current page
};


	