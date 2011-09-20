/**
 *  \file IEEngineTab.h
 *  \brief Extends the CIEEngine class to implement a CE Internet Explorer 
 *  rendering engine for a single tab.
 */ 

#pragma once
#include "../common/IEEngine.h"
#include "exdisp.h"
#include "exdispid.h"
#include "mshtmhst.h"
#include "mshtml.h"
#include "../common/History.h"
#include "../../../Common/Private/RelativeURLs.h"

class CScrollNotify;
/**
 *  Class to define a single tab in the rendering engine component.  You should 
 *  not create instances of this object directly, tabs should be managed
 *  via CIEEngine.  This engine is designed to work on CE devices.
 */
class CIEEngineTab :
	public CIEEngine,
	public IOleContainer,
	public IOleClientSite,
	public IOleInPlaceSite,
	public IDocHostUIHandler2,
	public DWebBrowserEvents2,
	public IDocHostShowUI
{
public:

	//////////////////////////////////////////////
	//											//
	//		Setup (Public)						//
	//											//
	//////////////////////////////////////////////

	/**
	 *  Constructor for the CIEEngineTab.  This will initialise the member 
	 *  variables, viewport and COM interface.
	 *  \param hInstance	Parent window's HINSTANCE
	 *  \param hwndParent	Parent window's Window Handle
	 *  \param tabID	Identifier for this engine which can be used to identify
	 *					the source of callbacks.
	 *  \param bsvScrollbars Whether or not the Scrollbars are visible in the 
	 *					newly created tab.
	 *  \param tcIconURL	URL of the icon for the application loaded in this tab
	 */
	CIEEngineTab(HINSTANCE hInstance, HWND hwndParent, int tabID, 
				LPCTSTR tcIconURL, BoolSettingValue bsvScrollbars);

	/**
	*  Engine Destructor, this will destroy the HTML window and
	*  uninitialise the COM components.
	*/
	~CIEEngineTab(void);

	/**
	*  Creates the Internet Explorer Browser COM component and performs all
	*  necessary initialisation.  After creating the Engine Tab object you will 
	*  need to call this function to initialise the browser correctly.
	*  \param configFunction Pointer to the Cores configuration reading function, 
	*                        not used in IE Engine CE.
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
	*  \return Whether or not the key should continue to be processed by 
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
	ENGINEEVENTPROC* CIEEngineTab::GetEngineEvents();


	//////////////////////////////////////////////
	//											//
	//		Navigation (Public)					//
	//											//
	//////////////////////////////////////////////

	/**
	*  Instruct the IE engine to navigate to the specified URL.
	*  \param tcURL URI to Navigate to, can be http://, ftp://
	*				or JavaScript://
	*  \return the result of instructing the Browser Component to navigate
	*/
	LRESULT Navigate(LPCTSTR tcURL);

	/**
	*  Halts any navigation currently in progress immediately causing a 
	*  callback to indicate Document Complete.  If there is no navigation in 
	*  progress this function will have no effect.
	*  \return Whether or not the browser successfully stopped navigating
	*/
	LRESULT Stop();

	/**
	*  Reloads the page currently displayed in the browser window.  Note that
	*  reloading the page will not cause the META tags to be reparsed as
	*  the PocketBrowser application is not designed to be modified at runtime.
	*  \param bFromCache Whether to reload the page from cache or not
	*  \bug The IE browser component can sometimes be unreliable in its handling
	*  of the bFromCache parameter.
	*  \return Whether or not the page was successfully reloaded.
	*/
	LRESULT Reload(BOOL bFromCache);

	/**
	*  Zoom the text for this tab to the specified zoom level.  This level will 
	*  persist across newly created tabs also.
	*  \param dwZoomLevel the level to zoom the text to.
	*  \return S_OK if the zoom was successful, else S_FALSE
	*/
	LRESULT ZoomText(TextZoomValue dwZoomLevel);

	/**
	*  Retrives the current level of the Text zoom on this tab.
	*  \param *dwZoomLevel [out] the current level of text zoom.
	*  \return S_OK if the retrieval was successful, else S_FALSE.
	*/
	LRESULT GetZoomText(TextZoomValue *dwZoomLevel);


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
	*  Set the visibility of the scrollbars.  Note scrollbars will not be 
	*  enabled unless the webpage is larger than the size of the screen.
	*  \param dwBoolSettingValue The visibility of the Scrollbars, both 
	*							horizontal and vertical.
	*  \return Whether the scrollbar visibility was successfully set
	*/
	BoolSettingValue SetScrollBars(BoolSettingValue dwBoolSettingValue);

	/**
	*  Retrieve whether or not Scrollbars are currently visibile in the browser
	*  component.
	*  \return The visibility of the scrollbars, both horizontal and vertical.
	*/
	BoolSettingValue GetScrollBars();

	/**
	*  Set the position of the Horizontal scrollbar.
	*  \param lPos The position to set the scrollbar to, expressed in terms
	*  of pixels
	*  \return the actual position of the scrollbar after scrolling, or -1 on error
	*/
	LONG Scrollbars_HPosSet (LONG lPos);

	/**
	*  Retrieve the position of the horizontal scrollbar.
	*  \return current horizontal scrollbar position in pixels.  -1 on Error.
	*/
	LONG Scrollbars_HPosGet ();

	/**
	*  Set the position of the Vertical scrollbar.
	*  \param lPos The position to set the scrollbar to, expressed in terms
	*  of pixels
	*  \return the actual position of the scrollbar after scrolling, or -1 on error
	*/
	LONG Scrollbars_VPosSet (LONG lPos);

	/**
	*  Get the position of the vertial scrollbar.
	*  \return current vertical scrollbar position in pixels.  -1 on Error
	*/
	LONG Scrollbars_VPosGet ();

	/**
	*  Retrieve the current rendered size of the page in pixels
	*  \param pwidth Pointer to int to receive the width
	*  \param pheight Pointer to int to receive the height
	*  \return TRUE if size retrieved successfully
	*/
	BOOL Scrollbars_SizeGet (int *pwidth, int *pheight);

	//////////////////////////////////////////////////
	//												//
	//		Application (Tab) Management (Public)	//
	//												//
	//////////////////////////////////////////////////

	/**
	*  Updates the location of the HTML window within the Browser component.
	*  \param rcNewSize New Position location for the HTML window
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
	*  Invoke a JavaScript Function on the current page.  The function will 
	*  only be invoked if it is present on the page.
	*  \param tcFunction JavaScript Function, example Format 
	*  \code
	*	doScan('12345678', 0x35, 'Decode');  
	*  \endcode
	*					You can escape
	*					the quotation mark within strings using \'.
	*  \todo Unable to implement this function for Arrays and non string 
	*		types, further investigation is needed as to whether or not this is
	*		possible, at the moment implemented as a navigation to a JavaScript
	*		function.
	*  \return Whether or not the function was successfully invoked, note this 
	*			does not correspond to the return value of the invoked 
	*			JavaScript function.
	*/
	LRESULT JS_Invoke (LPCTSTR tcfunction);

	/**
	*  Determines whether or not the function exists in the context of the 
	*  current page.
	*  \param tcFunction JavaScript function whose existance is being 
	*					determined, example Format
	*  \code
	*	doScan('12345678', 0x35, 'Decode');
	*	doScan();
	*  \endcode
	*					You can escape the quotation mark within strings 
	*					using \'.
	*  \return Whether or not the function exists in the context of the current
	*			page
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
	*  SetAcceleratorMode sets the Accelerator Mode
	*  \param dwAcceleratorValue Used to specify which Accelerator Key profile
	*  to use.  ACCELERATE_OFF disables all accelerate keys, ACCELERATE_NORM 
	*  implements 'normal' PocketBrowser behaviour, ACCELERATE_ON enables
	*  all accelerate keys.  See the help file for more information.
	*  \return ACCELERATE_NOT_IMPLEMENTED
	*/
	AcceleratorValue SetAcceleratorMode(AcceleratorValue dwAcceleratorValue);

	/**
	*  GetAcceleratorMode retrieves the accelerator mode.
	*  \return ACCELERATE_NOT_IMPLEMENTED
	*/
	AcceleratorValue GetAcceleratorMode();
private:
	
	//////////////////////////////////////////////
	//											//
	//		Private Functions					//
	//		Non COM Object						//
	//											//
	//////////////////////////////////////////////

	/**
	*  Thread used to determine whether or not the current page load has 
	*  exceeded the navigation timeout period, as specified by 
	*  SetNavigationTimeout
	*  \param lpParameter Pointer to the CIEEngineTab object
	*  \return 0 on completion of the thread
	*/
	static DWORD WINAPI NavigationTimeoutThread( LPVOID lpParameter );


	/**
	*  Parses META tags from the currently loaded page and invokes the 
	*  EEID_METATAG Engine event to notify the calling parent application.
	*  EEID_METATAG is invoked once for each Meta Tag encountered.  
	*  A Meta tag is expected in the form
	*  \code
	*  <HTTP-EQUIV="the http-equiv" Content=the content"> 
	*  \endcode
	*  \return S_OK The tags were successfully parsed.
	*/
	HRESULT ParseTags();

	/**
	*  Retrieve the current position of the specified scrollbar in pixels
	*  \param eAxis Either the horizontal or the Veritcal Scrollbar
	*  \param lNewValue The New Value for the specified scrollbar.  Set this 
	*  value to -1 if you do not wish to move the scrollbar.
	*  \return The position of the specified Scrollbar, or -1 on error.
	*/
	LONG AdjustCurrentScroll (ScrollbarOrientation eAxis, LONG lNewValue);

	/**
	*  Retrieve the current rendered size of the page in pixels
	*  \param pwidth Pointer to int to receive the width
	*  \param pheight Pointer to int to receive the height
	*  \return TRUE if size retrieved successfully
	*/
	BOOL GetPageSize (int *pwidth, int *pheight);

	/**
	*  Accessor for m_tabID
	*  \return m_tabID
	*/
	int GetTabID();

	//////////////////////////////////////////////
	//											//
	//		Private Functions					//
	//		COM Object Maintenance				//
	//											//
	//////////////////////////////////////////////

	/**
	*  Initializes the events so that we get notified via the "Invoke" function.
	*  This function is called during CreateEngine.
	*  \return S_OK if the function succeeded
	*  \return S_FALSE the function faled
	*/
	HRESULT InitEvents();
	
	/**
	*  Gets the user interface (UI) capabilities of the application that is hosting MSHTML.
	*  \param pInfo [in, out] A pointer to a DOCHOSTUIINFO structure that receives the host's UI capabilities.
	*  \return S_OK if successful, or an error value otherwise.
	*/
	HRESULT GetHostInfo( DOCHOSTUIINFO* pInfo );
	
	/**
	*  Overrides the HTML component's TranslateAccelerator which processes 
	*  keystrokes received by the HTML component.
	*  Returning S_OK will prevent the key being processed further by the 
	*  application.
	*  \param lpMsg [in] A pointer to a MSG structure that specifies the message to be translated.
	*  \param pguidCmdGroup [in] A pointer to a GUID for the command group identifier. 
	*  \param nCmdID [in] A DWORD that specifies a command identifier.
	*  \return S_OK if the function succeeded or and error value otherwise
	*/
	HRESULT TranslateAccelerator( LPMSG lpMsg, const GUID __RPC_FAR *pguidCmdGroup, DWORD nCmdID);

	/**
	*  Retrieves pointers to the supported interfaces on an object.
	*  This method calls AddRef.
	*  \param riid [in] The identifier of the interface being requested.
	*  \param ppvObject [out] The address of a pointer variable that receives the interface pointer requested in the riid parameter. Upon successful return, *ppvObject contains the requested interface pointer to the object. If the object does not support the interface, *ppvObject is set to NULL.
	*  \return S_OK if the interface is supported
	*  \return E_NOINTERFACE if the interface is not supported
	*  \return E_POINTER if ppvObject is NULL
	*/
	HRESULT QueryInterface (REFIID riid, LPVOID * ppv);
	
	/**
	*  Increments the reference count 
	*  \return reference count after the add.
	*/
	ULONG AddRef(void);

	/**
	*  Decrements the reference count and deletes this object if there are no refences remaining 
	*  \return reference count after the release.
	*/
	ULONG Release(void);

	HRESULT Scroll (SIZE scrollExtent);
	
	/**
	*  Returns the html window handle, used by the automation object
	*  \return value of m_hwndHTML.
	*/
	HRESULT GetWindow(HWND *phwnd);

	/**
	*  This method provides access to properties and methods exposed by an object.
	*  \param dispIdMember [in] Identifies the member. Use IDispatch::GetIDsOfNames or the object's documentation to obtain the dispatch identifier.
	*  \param riid [in] Reserved for future use; set to IID_NULL.
	*  \param lcid [in] Locale context in which to interpret parameters. The lcid is used by the GetIDsOfNames function, and is also passed to Invoke to allow the objectto interpret its parameters specific to a locale. 
	*			Applications that do not support multiple national languages can ignore this parameter. 
	*  \param wFlags [in] Flags describing the context of the Invoke call. It is one of the following values.
	*			DISPATCH_METHOD The member is invoked as a method. If a property has the same name, both this and the DISPATCH_PROPERTYGET flag may be set.
	*			DISPATCH_PROPERTYGET The member is retrieved as a property or data member.
	*			DISPATCH_PROPERTYPUT The member is changed as a property or data member.
	*			DISPATCH_PROPERTYPUTREF The member is changed by a reference assignment, rather than a value assignment. This flag is valid only when the property accepts a reference to an object.
	*  \param pDispParams [in, out] Pointer to a structure that contains an array of parameters, an array of parameter DISPIDs for named parameters, and counts for the number of elements in the arrays. See the Remarks section that follows for a description of the DISPPARAMS structure.
	*  \param pVarResult [out] Pointer to the location where the result is to be stored, or NULL if the caller expects no result. This parameter is ignored if DISPATCH_PROPERTYPUT or DISPATCH_PROPERTYPUTREF is specified.
	*  \param pExcepInfo [out] Pointer to a structure that contains exception information. This structure should be filled in if DISP_E_EXCEPTION is returned. Can be NULL.
	*  \param puArgErr [out] The index within rgvarg of the first parameter that has an error. Arguments are stored in pDispParams->rgvarg in reverse order, so the first parameter is the one with the highest index in the array. This parameter is returned only when the resulting return value is DISP_E_TYPEMISMATCH or DISP_E_PARAMNOTFOUND. 
	*  \return S_OK Success.
	*  \return DISP_E_BADPARAMCOUNT The number of elements provided to DISPPARAMS is different from the number of parameters accepted by the method or property.
	*  \return DISP_E_BADVARTYPE One of the parameters in rgvarg is not a valid variant type. 
	*  \return DISP_E_EXCEPTION The application needs to raise an exception. In this case, the structure passed in pExcepInfo should be filled in.
	*  \return DISP_E_MEMBERNOTFOUND The requested member does not exist, or the call to Invoke tried to set the value of a read-only property.
	*  \return DISP_E_NONAMEDARGS This implementation of IDispatch does not support named parameters.
	*  \return DISP_E_OVERFLOW One of the parameters in rgvarg could not be coerced to the specified type.
	*  \return DISP_E_PARAMNOTFOUND One of the parameter DISPIDs does not correspond to a parameter on the method. In this case, puArgErr should be set to the first parameter that contains the error.
	*  \return DISP_E_TYPEMISMATCH One or more of the parameters could not be coerced. The index within rgvarg of the first parameter with the incorrect type is returned in the puArgErr parameter.
	*  \return DISP_E_UNKNOWNINTERFACE The interface identifier passed in riid is not IID_NULL.
	*  \return DISP_E_UNKNOWNLCID The member being invoked interprets string parameters according to the LCID, and the LCID is not recognized. If the LCID is not needed to interpret parameters, this error should not be returned.
	*  \return DISP_E_PARAMNOTOPTIONAL A required parameter was omitted.
 	*/
	HRESULT Invoke(DISPID dispidMember,REFIID riid,LCID lcid,WORD wFlags,DISPPARAMS FAR* pdparams, VARIANT FAR* pvarResult,EXCEPINFO FAR* pexcepinfo,UINT FAR* puArgErr);
	
	/**
	*  Notifies the container that one of its objects is being activated in place.
	*  \return S_OK The method successfully notified the container.
	*  \return E_UNEXPECTED The call to the method unexpectedly failed.
	*/
	HRESULT OnInPlaceActivate(void);
	
	/**
	*  Enables an in-place object to retrieve window interfaces that form at the window object hierarchy, and the position in the parent window to locate the object's in-place activation window. 
	*  \param ppFrame [out] Address of IOleInPlaceFrame* pointer variable that receives the interface pointer to the frame. If an error occurs, the implementation must set *ppFrame to NULL. 
	*  \param ppDoc [out] Address of IOleInPlaceUIWindow* pointer variable that receives the interface pointer to the document window. If the document window is the same as the frame window, *ppDoc is set to NULL. In this case, the object can only use *ppFrame or border negotiation. If an error is returned, the implementation must set *ppDoc to NULL. 
	*  \param lprcPosRect [out] Pointer to the rectangle containing the position of the in-place object in the client coordinates of its parent window. If an error is returned, this parameter must be set to NULL. 
	*  \param lprcClipRect [out] Pointer to the outer rectangle containing the in-place object's position rectangle (PosRect). This rectangle is relative to the client area of the object's parent window. If an error is returned, this parameter must be set to NULL. 
	*  \param lpFrameInfo [out] Pointer to an OLEINPLACEFRAMEINFO structure the container is to fill in with appropriate data. If an error is returned, this parameter must be set to NULL. 
	*  \return S_OK The method completed successfully. 
	*  \return E_INVALIDARG The call to the method contains an invalid argument.
	*  \return E_UNEXPECTED The call to the method unexpectedly failed.
	*/
	HRESULT GetWindowContext(LPOLEINPLACEFRAME FAR * lplpFrame,LPOLEINPLACEUIWINDOW FAR * lplpDoc, LPRECT lprcPosRect, LPRECT lprcClipRect, LPOLEINPLACEFRAMEINFO lpFrameInfo);

	/**
	*  Notifies the container that the object is no longer active in place. 
	*  \return S_OK The method successfully notified the container. 
	*  \return E_UNEXPECTED The call to the method unexpectedly failed.
	*/
	HRESULT OnInPlaceDeactivate(void);

	/**
	*  Overides the ShowMessage method of IDocHostShowUI to customize the 
	*  appearance of Javascript alert boxes.
	*  \param hwnd The HWND of the owner window.
	*  \param lpstrText [in] pointer to a string containing the text for the message box.
	*  \param lpstrCaption [in] pointer to a string containing the caption for the message box.
	*  \param dwType [in] A DWORD containing the flag type (taken from the MessageBox MB_xxxx constants).
	*  \param lpstrHelpFile [in] pointer to a string containing the Help file name.
	*  \param dwHelpContext [in] A DWORD containing the Help context identifier.
	*  \param plResult [out] A pointer to an LRESULT that indicates what button the user clicked (taken from the MessageBox IDxxx constants).
	*  \return S_OK to indicate that we have shown the message box.
	*/
	HRESULT ShowMessage(HWND hwnd, 
						LPOLESTR lpstrText, 
						LPOLESTR lpstrCaption, 
						DWORD dwType,
						LPOLESTR lpstrHelpFile, 
						DWORD dwHelpContext, 
						LRESULT *plResult);


private:	//  Attributes
	TCHAR	m_tcNavigatedURL[MAX_URL];	///< The current URL loaded or being navigated to
	ULONG	m_ulRefs;					///< COM reference counter
	IWebBrowser2		*m_pBrowser;	///< Pointer to Microsoft IE Browser component
	IConnectionPoint	*m_pCP;			///< Web Browser Connection Point
	IOleObject			*m_pObject;		///< IOleObject reference 
	BOOL	m_bInPlaceActive;			///< Whether or not one of the container's objects is activated in place
	BoolSettingValue m_bsvScrollBars;	///<  Whether scrollbars or visible or not
	ENGINEEVENTPROC	m_EngineEvents[EEID_MAXEVENTID];	///< Array of pointers to functions to call when Engine Events occur
	TCHAR	m_tcCurrentPageTitle[MAX_URL];///< The title of the currently loaded page
	TCHAR	m_tcIconURL[MAX_URL];		///< The path to the icon for the application loaded in this tab.
	HANDLE	m_hNavigated;				///< Event handle set on document complete or on navigation error, used to stop the navigation timeout thread.
	int		m_tabID;					///< The unique PocketBrowser reference for this tab (PocketBrowser Application)
	HWND	m_hwndTabHTML;				///< HTML Window Handle
	CHistory*	m_BrowserHistory;		///< This tab's browser history
	AcceleratorValue m_AcceleratorMode;	///< The Tab's Accelerator Mode
	BOOL	bRunningOnWM;				///< Whether or not the COM engine is running on Windows Mobile.
	int		iLoadPageCount;				///< Number of times the Load Page has been loaded
	BOOL	m_bTextSelectionEnabled;
	BOOL	bDeviceCausesDoubleBackspace;	///< For EMBPD00023872, some devices will cause a double backspace when the backspace key is pressed, this variable is used to determine the result of PreProcessMessage.


#pragma region not_implemented_virtual_functions
	//virtual functions not implemented
	STDMETHOD(GetOverrideKeyPath)(LPOLESTR __RPC_FAR *pchKey, DWORD dw) { return E_NOTIMPL; }
	STDMETHOD(ShowContextMenu)(DWORD dwID,POINT __RPC_FAR *ppt,IUnknown __RPC_FAR *pcmdtReserved,IDispatch __RPC_FAR *pdispReserved) { return S_OK; }
	STDMETHOD(ShowUI)(DWORD dwID,IOleInPlaceActiveObject __RPC_FAR *pActiveObject,IOleCommandTarget __RPC_FAR *pCommandTarget,IOleInPlaceFrame __RPC_FAR *pFrame,IOleInPlaceUIWindow __RPC_FAR *pDoc) { return E_NOTIMPL; }
    STDMETHOD(HideUI)(void) { return E_NOTIMPL; }
	STDMETHOD(UpdateUI)(void) { return E_NOTIMPL; }
	STDMETHOD(EnableModeless)(BOOL fEnable) { return E_NOTIMPL ; }
	STDMETHOD(OnDocWindowActivate)(BOOL fActivate) { return E_NOTIMPL; }
	STDMETHOD(OnFrameWindowActivate)(BOOL fActivate) { return E_NOTIMPL; }
	STDMETHOD(ResizeBorder)(LPCRECT prcBorder,IOleInPlaceUIWindow __RPC_FAR *pUIWindow,BOOL fRameWindow) { return E_NOTIMPL; }
	STDMETHOD(GetOptionKeyPath)(LPOLESTR __RPC_FAR *pchKey,DWORD dw) { return E_NOTIMPL; }
	STDMETHOD(GetDropTarget)(IDropTarget __RPC_FAR *pDropTarget,IDropTarget __RPC_FAR *__RPC_FAR *ppDropTarget) { return E_NOTIMPL; }
	STDMETHOD(GetExternal)(IDispatch __RPC_FAR *__RPC_FAR *ppDispatch) { return E_NOTIMPL; }
	STDMETHOD(TranslateUrl)(DWORD dwTranslate,OLECHAR __RPC_FAR *pchURLIn,OLECHAR __RPC_FAR *__RPC_FAR *ppchURLOut) { return E_NOTIMPL; }
	STDMETHOD(FilterDataObject)(IDataObject __RPC_FAR *pDO,IDataObject __RPC_FAR *__RPC_FAR *ppDORet) { return E_NOTIMPL; }
	STDMETHOD(ParseDisplayName)(IBindCtx *, LPOLESTR, ULONG *, IMoniker **) { return E_NOTIMPL; }
	STDMETHOD(EnumObjects)(DWORD, IEnumUnknown **) { return E_NOTIMPL; }
	STDMETHOD(LockContainer)(BOOL) { return S_OK; }
	STDMETHOD(SaveObject)(void) { return E_NOTIMPL; }
	STDMETHOD(GetMoniker)(DWORD dwAssign, DWORD dwWhichMoniker, LPMONIKER * ppmk) { return E_NOTIMPL; }
	STDMETHOD(GetContainer)(LPOLECONTAINER * ppContainer) { return E_NOINTERFACE; }
	STDMETHOD(ShowObject)(void) { return E_NOTIMPL; }
	STDMETHOD(OnShowWindow)(BOOL fShow) { return E_NOTIMPL; }
	STDMETHOD(RequestNewObjectLayout)(void) { return E_NOTIMPL; }
	STDMETHOD(ContextSensitiveHelp)(BOOL fEnterMode) { return E_NOTIMPL; }
	STDMETHOD(CanInPlaceActivate)(void) { return S_OK; }
	STDMETHOD(OnUIActivate)(void){ return E_NOTIMPL; }

	STDMETHOD(OnUIDeactivate)(BOOL fUndoable){ return E_NOTIMPL; }
	STDMETHOD(DiscardUndoState)(void){ return E_NOTIMPL; }
	STDMETHOD(DeactivateAndUndo)(void){ return E_NOTIMPL; }
	STDMETHOD(OnPosRectChange)(LPCRECT lprcPosRect){ return E_NOTIMPL; }
	STDMETHOD(GetTypeInfoCount)(UINT FAR* pctinfo){ return E_NOTIMPL; }
	STDMETHOD(GetTypeInfo)(UINT itinfo,LCID lcid,ITypeInfo FAR* FAR* pptinfo){ return E_NOTIMPL; }
	STDMETHOD(GetIDsOfNames)(REFIID riid,OLECHAR FAR* FAR* rgszNames,UINT cNames,LCID lcid, DISPID FAR* rgdispid){ return E_NOTIMPL; }
	STDMETHOD(ShowHelp)( HWND hwnd, LPOLESTR pszHelpFile, UINT uCommand,DWORD dwData, POINT ptMouse, IDispatch *pDispatchObjectHit){ return E_NOTIMPL; }
#pragma endregion

private:
	CScrollNotify *pScrollNotify;
};
