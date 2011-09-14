/**
 *  \file pbengine_defs.h
 *  \brief Definitions common between all engines used in RhoElements
 */

#pragma once
#include "PB_Defines.h"  //  For MAX_URL

//////////////////////////////
//							//
//		Enumerated Types	//
//							//
//////////////////////////////

/**
*  Enumeration value used to specify the value of attributes.  For example 
*  Scrollbar visibility; whether JavaScript is enabled in the browser; 
*  whether images are enabled e.t.c.
*/
enum BoolSettingValue
{ 
	NOT_IMPLEMENTED = -1,	///< The Setting is Not Implemented
	SETTING_OFF = 1,		///< Setting is off, or FALSE
	SETTING_ON = 0			///< Setting is on, or TRUE
};


/**
*  Possible Scrollbar orientations.
*/
enum ScrollbarOrientation
{
	SCROLLBAR_HORIZONTAL	= 0,	///< The Horizontal Scrollbar
	SCROLLBAR_VERTICAL		= 1		///< The Vertical Scrollbar
};

/**
*  Possible values for the browser's text zoom.  These map to The IWebBrowser2
*  zoom factor.
*/
enum TextZoomValue
{
	TEXT_ZOOM_SMALLEST = 0,	///< Smallest possible text size
	TEXT_ZOOM_SMALLER = 1,	///< Slightly smaller than normal text size
	TEXT_ZOOM_NORMAL = 2,	///< Normal Text size
	TEXT_ZOOM_BIGGER = 3,	///< Slightly larger than normal text size
	TEXT_ZOOM_BIGGEST = 4	///< Largest possible text size
};

/**
*  Events fired by the engine which can be registered for via 
*  CWebEngine::RegisterForEvent.
*  Note the values given to this enumeration should be sequential and start from 
*  zero, this is to enable registered functions to be iterated over using 
*  pointer arithmetic.
*  \todo determine if my comments for EEID_PROGRESSCHANGE are accurate.
*/
enum EngineEventID
{
	EEID_BEFORENAVIGATE = 0,///< Fired Before a new page is navigated to, if the calling application instructs a CWebEngine::Navigate or the user clicks on a link.
	EEID_NAVIGATECOMPLETE,	///< Fired once the server responds to the client's navigation request, the page now starts loading.
	EEID_DOCUMENTCOMPLETE,  ///< Fired when the page is 100% loaded included all embedded images.
	EEID_NAVIGATIONERROR,	///< Fired if there is an error navigating to the page, for example attempting to navigate to http://www.motorola.com if the device does not have a network connection.
	EEID_METATAG,			///< Fired when a Meta Tag is parsed on the loaded page.  This event is fired once for each meta tag, so a page with 3 meta tags will invoke this event 3 times.
	/**
	* Fires when the title of a document in the object becomes available or changes.  
	* Because the title might change while an HTML page is downloading, the URL of 
	* the document is set as the title. If the HTML page specifies a title, it is parsed, 
	* and the title is changed to reflect the actual title.
	*/
	EEID_TITLECHANGE,		
	EEID_NAVIGATIONTIMEOUT, ///< Fired when the page load takes longer than the value specified by CWebEngine::SetNavigationTimeout
	EEID_STATUSTEXTCHANGE,	///< Fired when the status bar text on the currently loaded page changes.
	EEID_PROGRESSCHANGE,	///< Fired when the progress of a download operation is updated.  LOWORD(value) contains the maximum progress value.  HIWORD(value) contains the current progress value, or -1 if the download is complete.
	EEID_TOPMOSTHWNDAVAILABLE,	///< Fired when the TopMost Window handle is available.   Value contains the HWND cast to an integer.  TabID refers to the Tab Identifier to which the HWND refers.
	EEID_SETSIPSTATE,			///<  Fired when an editable field gains or loses focus
	EEID_CONSOLEMESSAGE,		///<  Fired when a console message is available from WebKit (could be a JS error or window.console.log)
	//this event ID MUST remain at the END of the list
	EEID_MAXEVENTID			///< Maximum event Identifier.  This does not represent an event but can be used in sizing arrays etc. 
};

/**
*  Meta Tag structure transfered between the Engine component and the parent 
*  application.  This meta tag structure differs from that used internally by 
*  RhoElements in that it contains the http-equiv and makes no attempt to 
*  separate the contents into parameters and values.
*/
typedef struct engineMETATagType
{
    LPWSTR tcHTTPEquiv;	///< Contents of the Meta tag's HTTP-Equiv String
    LPWSTR tcContents;	///< Contents of the Meta tag's Contents string
} EngineMETATag;

/**
 * Location structure transferred from core to engine.
 */
typedef struct engineLocationType
{
    SYSTEMTIME  timestamp;  ///< milliseconds since 1st Jan 1970
    double      latitude;   ///< degrees from equator (north +ve)
    double      longitude;  ///< degrees from Greenwich (east +ve)
    double      accuracy;   ///< accuracy in metres
    double      altitude;   ///< altitude in metres, from WGS84
    double      altitudeAccuracy; ///< altitude accuracy in metres
    double      heading;    ///< degrees clockwise from north
    double      speed;      ///< metres per second
    BOOL        hasAltitude;///<< true if altitude field valid
    BOOL        hasAltitudeAccuracy; ///<< altitude accuracy field valid
    BOOL        hasHeading; ///<< true if heading field valid
    BOOL        hasSpeed; ///<< true if speed field valid
} EngineLocation;

/**
 * Interface provided to MotoWebKit by the Core
 */
typedef struct engineLocationInterfaceType EngineLocationInterface;
/**
 * Format of the callback function specified by the core for the engine to use
 * to query the last cached location.
 *
 * \param location EngineLocation structure to contain results
 *
 * \return S_OK if cached location available
 */
typedef LRESULT (CALLBACK* GetCachedLocation)(EngineLocationInterface* locationInterface,
                                              EngineLocation* location);

/**
 * Format of the callback function specified by the core for the engine to use
 * to request periodic location updates.
 *
 * \param highAccuracy whether high accuracy results are required or not
 *
 * \return S_OK for success
 */
typedef LRESULT (CALLBACK* StartLocationUpdates)(EngineLocationInterface* locationInterface,
                                                 BOOL highAccuracy);

/**
 * Format of the callback function specified by the core for the engine to use
 * to stop periodic location updates.
 *
 * \return S_OK for success
 */
typedef LRESULT (CALLBACK* StopLocationUpdates)(EngineLocationInterface* locationInterface);
struct engineLocationInterfaceType
{
    GetCachedLocation getCachedLocation; ///< callback to get cached location
    StartLocationUpdates startLocationUpdates; ///< callback to start location updates
    StopLocationUpdates stopLocationUpdates; ///< callback to stop location updates
    void* coreContext; ///< private pointer for use by the core
};


//////////////////////////////////////
//									//
//		Reserved Application IDs	//
//									//
//////////////////////////////////////

/**
*  Reserved Identifier for RhoElements application representing the 
*  lock screen.
*/
const int APP_ID_LOCK_SCREEN = -1;

/**
*  Reserved Identifier for RhoElements application representing the 
*  application switcher.
*/
const int APP_ID_APP_SWITCHER = -2;

/**
*  Reserved Identifier for RhoElements application representing the 
*  application launcher.
*/
const int APP_ID_APP_HOME = -3;

//////////////////////////////
//							//
//		Other Defines		//
//							//
//////////////////////////////

/**
*  The default value if the parent application does not specify a navigation
*  timeout via CWebEngine::SetNavigationTimeout
*/
const int DEFAULT_NAV_TIMEOUT = 45000;

/**
*  The WebKit zoom factor when pages are first loaded.
*/
const double DEFAULT_SCALE = 1.5;

/**
*  The maximum number of entries in the history.  Valid values are greater than 2.
*/
const int MAX_HISTORY = 30;

/**
*  The minimum legal navigation timeout value.
*/
const int MINIMUM_NAVIGATION_TIMEOUT = 500;

/**
*  Format of the callback function specified by the parent application to 
*  receive Engine Events from the Rendering Engine.
*  \param eeID Engine event being invoked.
*  \param value Data associated with the engine event, e.g. OnBeforeNavigate will provide the URL of the page being navigated.
*  \param tabIndex Unique RhoElements ID for the engine instance, as specified in CWebEngine::Tab_New
*  \return S_OK to indicate the callback has been successfully handled.
*/
typedef LRESULT (CALLBACK* ENGINEEVENTPROC)(EngineEventID eeID,	LPARAM value, int tabIndex);

/**
*  Format of the callback function specified by the parent application to 
*  act as the WNDPROC for the HTML window which receives windows messages.
*  \param hwnd Window handle receiving message
*  \param message Message being received
*  \param wParam WPARAM associated with the message
*  \param lParam LPARAM associated with the message
*  \param LRESULT Whether or not the message was swallowed.
*/
typedef LRESULT (CALLBACK* HTMLWndPROC_T)  (HWND hwnd, UINT message,
											WPARAM wParam, LPARAM lParam);

/**
*  Format of the callback function specified by the core for the engine to 
*  use to retrieve configuration settings.
*  \param iInstID The RhoElements application (Engine Tab) whose configuration
*                 setting is to be retrieved.
*  \param tcSetting The configuration setting to retrieve, in the format 
*                   Group\\Tag, e.g. HTMLStyles\\FitToScreenEnabled
*  \param tcValue [out] The value read from the configuration.  This will be 
*                 NULL if the value was not read successfully.
*  \return S_OK.
*         
*/
typedef LRESULT (CALLBACK* ReadEngineConfigParameter_T)	(int iInstID, 
														 LPCTSTR tcSetting, 
														 TCHAR* tcValue);

//////////////////////////////////
//								//
//		Windows Mobile Only		//
//		HTML Engine	Component	//
//								//
//////////////////////////////////

#ifdef PB_ENGINE_IE_MOBILE

//  Codes returned from Pocket IE Engine component to indicate events
#define NM_PIE_TITLE            (WM_USER + 104)  ///< Message ID indicating the associated message defines the page title.
#define NM_PIE_META             (WM_USER + 105)  ///< Message ID indicating the associated message defines a Meta Tag (HTTP-Equiv only for WM).
#define NM_PIE_BEFORENAVIGATE   (WM_USER + 109)  ///< Message ID indicating the associated message indicates a BeforeNavigate Event.
#define NM_PIE_DOCUMENTCOMPLETE	(WM_USER + 110)  ///< Message ID indicating the associated message defines a DocumentComplete Event.
#define NM_PIE_NAVIGATECOMPLETE (WM_USER + 111)  ///< Message ID indicating the associated message defines a NavigateComplete Event.
#define NM_PIE_KEYSTATE			(WM_USER + 112)  ///< Message ID indicating the associated message notifies that the key state has changed.
#define NM_PIE_ALPHAKEYSTATE	(WM_USER + 113)  ///< Message ID indicating the associated message notifies that the alpha key state has changed.

#define HTML_CONTAINER_NAME		TEXT("HTMLContainer")  ///< Name of the Window which is parent to all HTML components and handles notifications from the components.
#endif

////////////////////////////////////////////////////////////////////////////////
//																			  //
//			TypeDefs of functions exported from the DLL						  //
//  just remove the 't' from the start to determine which function			  //
//  is referenced															  //
//																			  //
////////////////////////////////////////////////////////////////////////////////

typedef LRESULT			(CALLBACK *tPB_CreateEngine)			();
typedef LRESULT			(CALLBACK *tPB_InitEngine)				(HINSTANCE, HWND, int, LPCTSTR, BoolSettingValue, HTMLWndPROC_T, WNDPROC*, ReadEngineConfigParameter_T);
typedef LRESULT			(CALLBACK *tPB_DeinitEngine)			();
typedef HWND			(CALLBACK *tPB_GetHTMLHWND)				();
typedef HWND			(CALLBACK *tPB_GetHTMLHWNDOnTab)		(int);
typedef HWND			(CALLBACK *tPB_GetParentHWND)			();
typedef LRESULT			(CALLBACK *tPB_RegisterForEvent)		(EngineEventID, ENGINEEVENTPROC);
typedef LRESULT			(CALLBACK *tPB_PreprocessMessage)		(MSG msg);
typedef LRESULT			(CALLBACK *tPB_Navigate)				(LPCTSTR);
typedef LRESULT			(CALLBACK *tPB_NavigateOnTab)			(LPCTSTR, int);
typedef LRESULT			(CALLBACK *tPB_Stop)					(void);
typedef LRESULT			(CALLBACK *tPB_StopOnTab)				(int);
typedef LRESULT			(CALLBACK *tPB_Reload)					(bool);
typedef LRESULT			(CALLBACK *tPB_ReloadOnTab)				(bool, int);
typedef LRESULT			(CALLBACK *tPB_Zoom)					(double);
typedef LRESULT			(CALLBACK *tPB_ZoomOnTab)				(double, int);
typedef LRESULT			(CALLBACK *tPB_GetZoomOnTab)			(double*, int);
typedef LRESULT			(CALLBACK *tPB_ZoomText)				(TextZoomValue);
typedef LRESULT			(CALLBACK *tPB_ZoomTextOnTab)			(TextZoomValue, int);
typedef LRESULT			(CALLBACK *tPB_GetZoomTextOnTab)		(TextZoomValue*, int);
typedef LONG			(CALLBACK *tPB_Scrollbars_HPosSet)		(LONG);
typedef LONG			(CALLBACK *tPB_Scrollbars_HPosSetOnTab)	(LONG, int);
typedef LONG			(CALLBACK *tPB_Scrollbars_HPosGet)		();
typedef LONG			(CALLBACK *tPB_Scrollbars_HPosGetOnTab)	(int);
typedef LONG			(CALLBACK *tPB_Scrollbars_VPosSet)		(LONG);
typedef LONG			(CALLBACK *tPB_Scrollbars_VPosSetOnTab)	(LONG, int);
typedef LONG			(CALLBACK *tPB_Scrollbars_VPosGet)		();
typedef LONG			(CALLBACK *tPB_Scrollbars_VPosGetOnTab)	(int);
typedef BoolSettingValue(CALLBACK *tPB_SetScrollbars)			(BoolSettingValue);
typedef BoolSettingValue(CALLBACK *tPB_SetScrollbarsOnTab)		(BoolSettingValue, int);
typedef BoolSettingValue(CALLBACK *tPB_GetScrollbars)			();
typedef BoolSettingValue(CALLBACK *tPB_GetScrollbarsOnTab)		(int);
typedef LRESULT			(CALLBACK *tPB_Tab_New)					(int, BOOL, LPCTSTR, BoolSettingValue, HTMLWndPROC_T, WNDPROC*, ReadEngineConfigParameter_T);
typedef LRESULT			(CALLBACK *tPB_Tab_Close)				(int);
typedef LRESULT			(CALLBACK *tPB_Tab_CloseCurrent)		();
typedef LRESULT			(CALLBACK *tPB_Tab_Switch)				(int);
typedef LRESULT			(CALLBACK *tPB_Tab_Resize)				(RECT, int);
typedef int				(CALLBACK *tPB_Tab_GetID)				();
typedef UINT			(CALLBACK *tPB_Tab_Count)				();
typedef LRESULT			(CALLBACK *tPB_Tab_GetTitle)			(LPTSTR, int);
typedef LRESULT			(CALLBACK *tPB_Tab_GetTitleOnTab)		(LPTSTR, int, int);
typedef LRESULT			(CALLBACK *tPB_Tab_GetIconOnTab)		(LPTSTR, int, int);
typedef LRESULT			(CALLBACK *tPB_Tab_ShowSwitcher)		(BOOL);
typedef LRESULT			(CALLBACK *tPB_Tab_Lock)				(BOOL);
typedef LRESULT			(CALLBACK *tPB_JS_Invoke)				(LPCTSTR);
typedef LRESULT			(CALLBACK *tPB_JS_InvokeOnTab)			(LPCTSTR, int);
typedef LRESULT			(CALLBACK *tPB_JS_Exists)				(LPCTSTR);
typedef LRESULT			(CALLBACK *tPB_JS_ExistsOnTab)			(LPCTSTR, int);
typedef LRESULT			(CALLBACK *tPB_History_GoForward)		(UINT);
typedef LRESULT			(CALLBACK *tPB_History_GoForwardOnTab)	(UINT, int);
typedef LRESULT			(CALLBACK *tPB_History_GoBack)			(UINT);
typedef LRESULT			(CALLBACK *tPB_History_GoBackOnTab)		(UINT, int);
typedef BoolSettingValue(CALLBACK *tPB_SetFitToScreen)			(BoolSettingValue);
typedef BoolSettingValue(CALLBACK *tPB_GetFitToScreen)			();
typedef BoolSettingValue(CALLBACK *tPB_SetClearType)			(BoolSettingValue);
typedef BoolSettingValue(CALLBACK *tPB_GetClearType)			();
typedef BoolSettingValue(CALLBACK *tPB_SetJavaScript)			(BoolSettingValue);
typedef BoolSettingValue(CALLBACK *tPB_GetJavaScript)			();
typedef BoolSettingValue(CALLBACK *tPB_SetImages)				(BoolSettingValue);
typedef BoolSettingValue(CALLBACK *tPB_GetImages)				();
typedef BoolSettingValue(CALLBACK *tPB_SetSounds)				(BoolSettingValue);
typedef BoolSettingValue(CALLBACK *tPB_GetSounds)				();
typedef BoolSettingValue(CALLBACK *tPB_SetActiveX)				(BoolSettingValue);
typedef BoolSettingValue(CALLBACK *tPB_GetActiveX)				();
typedef BoolSettingValue(CALLBACK *tPB_SetAcceleratorMode)		(AcceleratorValue);
typedef AcceleratorValue(CALLBACK *tPB_GetAcceleratorMode)		();
typedef BoolSettingValue(CALLBACK *tPB_SetBrowserGesturing)		(BoolSettingValue);
typedef BoolSettingValue(CALLBACK *tPB_GetBrowserGesturing)		();
typedef LRESULT			(CALLBACK *tPB_SetNavigationTimeout)	(DWORD);
typedef DWORD			(CALLBACK *tPB_GetNavigationTimeout)	();
typedef BOOL			(CALLBACK *tPB_GetSize)					(int*, int*);
typedef BOOL			(CALLBACK *tPB_GetSizeOnTab)			(int*, int*, int);
typedef LRESULT         (CALLBACK *tPB_SetLocationInterface)    (EngineLocationInterface*);
typedef LRESULT         (CALLBACK *tPB_UpdateLocation)          (EngineLocation*);
