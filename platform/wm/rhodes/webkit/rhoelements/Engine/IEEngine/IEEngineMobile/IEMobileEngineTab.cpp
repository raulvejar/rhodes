// IEMobileEngine.cpp : Defines the entry point for the DLL application.
//

#include "IEMobileEngineTab.h"

#include "htmlctrl.h"
//#include "aygshell.h"

//  Default Static Variables
HWND CIEMobileEngineTab::m_hwndTabHTMLContainer = NULL;

//////////////////////////////////////////////
//											//
//		Setup (Public)						//
//											//
//////////////////////////////////////////////

/**
* \author	Darryn Campbell (DCC, JRQ768)
* \date		October 2009 
*/
CIEMobileEngineTab::CIEMobileEngineTab(HINSTANCE hInstance, 
									   HWND hwndParent, 
									   int tabID, 
									   LPCTSTR tcIconURL,
									   BoolSettingValue bsvScrollbars)
									   : m_hwndTabHTML(NULL)
									   , m_hNavigated(NULL)
									   , m_BrowserHistory(NULL)
									   , m_dwCurrentTextZoomLevel(TEXT_ZOOM_NORMAL)
									   , m_bPageLoaded(TRUE)
{ 

	m_parentHWND = hwndParent;
	m_hparentInst = hInstance;
	m_tabID = tabID;
	m_bsvScrollBars = bsvScrollbars;

	//get the parents window size for the default size of the HTML
	//container.
	GetWindowRect(hwndParent, &m_rcViewSize);

	for(int i=0; i < EEID_MAXEVENTID; i++)
		m_EngineEvents[i] = NULL;

	memset(m_tcIconURL, NULL, sizeof(TCHAR) * MAX_URL);
	wcsncpy(m_tcIconURL, tcIconURL, MAX_URL);
	memset(m_tcCurrentPageTitle, NULL, sizeof(TCHAR) * MAXURL);
}

/**
* \author	Darryn Campbell (DCC, JRQ768)
* \date		October 2009 
*/
CIEMobileEngineTab::~CIEMobileEngineTab(void)
{
	//  Remove the browser history object from memory
	delete m_BrowserHistory;
	m_BrowserHistory = NULL;

	//  Destroy the Browser Object
	DestroyWindow(m_hwndTabHTML);
	m_hwndTabHTML = NULL;

	//  Destroy the Browser Object's parent if it exists
	if (m_hwndTabHTMLContainer)
	{
		DestroyWindow(m_hwndTabHTMLContainer);
		m_hwndTabHTMLContainer = NULL;
	}

}

/**
* \author	Darryn Campbell (DCC, JRQ768)
* \date		October 2009 (DCC, Initial Creation)
* \date		February 2010 (DCC, Added reading configuration settings)
*/
LRESULT CIEMobileEngineTab::CreateEngine(ReadEngineConfigParameter_T configFunction)
{
	//  Create an HTML container to listen for Engine Events if one does not 
	//  already exist
	if (m_hwndTabHTMLContainer == NULL)
	{
		//register the main window
		if (S_OK != RegisterWindowClass(m_hparentInst, &CIEMobileEngineTab::WndProc))
			return S_FALSE;
			
		//create the main window
		m_hwndTabHTMLContainer = CreateWindowEx( 0, HTML_CONTAINER_NAME, NULL, 
										WS_POPUP | WS_VISIBLE, 
										CW_USEDEFAULT, CW_USEDEFAULT, 
										CW_USEDEFAULT, CW_USEDEFAULT, 
										m_parentHWND, NULL, m_hparentInst, 0);

		if(!m_hwndTabHTMLContainer)
			return S_FALSE;


		//initialize the HTML Control
		if(!InitHTMLControl(m_hparentInst))
			return S_FALSE;

	}
	HRESULT dwResult = 0;


	//create the HTML window
	DWORD scrollStyle;
	if (m_bsvScrollBars == SETTING_ON)
		scrollStyle = 0;
	else
		scrollStyle = HS_NOSCROLL;

	//  Read the FitToScreenEnabled Config Setting
	TCHAR tcConfigSetting[MAX_URL];
	memset(tcConfigSetting, 0, sizeof(TCHAR) * MAX_URL);
	//  By default the window is created with FitToScreen true, so if it is 
	//  false use the HS_NOFITTOWINDOW windowstyle
	DWORD dwFitToWindowStyle, dwTextSelectionStyle;
	dwFitToWindowStyle = 0;
	dwTextSelectionStyle = 0;
	if (configFunction != NULL)
	{
		configFunction(m_tabID, L"HTMLStyles\\FitToScreenEnabled", tcConfigSetting);
		if (tcConfigSetting)
		{
			if (*tcConfigSetting == L'0')
				dwFitToWindowStyle = HS_NOFITTOWINDOW;
		}

		configFunction(m_tabID, L"HTMLStyles\\TextSelectionEnabled", tcConfigSetting);
		if (tcConfigSetting)
		{
			if (*tcConfigSetting == L'0')
				dwTextSelectionStyle = HS_NOSELECTION;
		}
	}
	m_hwndTabHTML = CreateWindow(WC_HTML, NULL, 
					WS_POPUP | WS_VISIBLE | scrollStyle | dwFitToWindowStyle | dwTextSelectionStyle, 
					m_rcViewSize.left, m_rcViewSize.top, 
					(m_rcViewSize.right-m_rcViewSize.left), 
					(m_rcViewSize.bottom-m_rcViewSize.top), 
					m_hwndTabHTMLContainer, NULL, m_hparentInst, NULL);

	if (m_hwndTabHTML)
	{
		if (configFunction != NULL)
		{
			//  Read the JavascriptEnabled configuration setting
			memset(tcConfigSetting, 0, sizeof(TCHAR) * MAX_URL);
			configFunction(m_tabID, L"HTMLStyles\\JavascriptEnabled", tcConfigSetting);
			if (tcConfigSetting)
			{
				if (*tcConfigSetting == L'0')
					SetJavaScript(SETTING_OFF);
				else
					SetJavaScript(SETTING_ON);
			}
			//  Read the ClearType Configuration Setting
			memset(tcConfigSetting, 0, sizeof(TCHAR) * MAX_URL);
			configFunction(m_tabID, L"HTMLStyles\\ClearTypeEnabled", tcConfigSetting);
			if (tcConfigSetting)
			{
				if (*tcConfigSetting == L'0')
					SetClearType(SETTING_OFF);
				else
					SetClearType(SETTING_ON);
			}
		}
	}
	else
	{
		//  Failed to create Tab window
		return S_FALSE;
	}

	//  Create the associated History Object
	m_BrowserHistory = new CHistory(this);

	return S_OK;
}


/**
* \author	Darryn Campbell (DCC, JRQ768)
* \date		October 2009 
*/
LRESULT CIEMobileEngineTab::RegisterForEvent(EngineEventID eeidEventID, ENGINEEVENTPROC pEventFunc)
{
	if(eeidEventID < 0 || eeidEventID >= EEID_MAXEVENTID)
		return S_FALSE;

	m_EngineEvents[eeidEventID] = pEventFunc;

	return S_OK;
}

/**
* \author	Darryn Campbell (DCC, JRQ768)
* \date		December 2009
*/
LRESULT CIEMobileEngineTab::PreprocessMessage(MSG msg)
{
	//  Call Translate Accelerator on the message to enable the browser to have
	//  first stab at translating the accelerator keys.
	IDispatch* pDisp;
	IOleInPlaceActiveObject* pInPlaceObject;
	SendMessage(m_hwndTabHTML, DTM_BROWSERDISPATCH, 0, (LPARAM) &pDisp); // New HTMLVIEW message

	if (pDisp == NULL)
	{
		return S_FALSE;
	}
	//  If the Key is back we do not want to translate it causing the browser
	//  to navigate back.  If Accelerate is ON we do want the browser to 
	//  navigate back.
	else if (msg.message == WM_KEYDOWN && 
    		msg.wParam == (WPARAM)VK_BACK)
	{
		return S_FALSE;
	}
	else
	{
		pDisp->QueryInterface( IID_IOleInPlaceActiveObject, (void**)&pInPlaceObject );
		return pInPlaceObject->TranslateAccelerator(&msg);	
	}
}

/**
* \author	Darryn Campbell (DCC, JRQ768)
* \date		October 2009
*/
ENGINEEVENTPROC* CIEMobileEngineTab::GetEngineEvents()
{
	return m_EngineEvents;
}

//////////////////////////////////////////////
//											//
//		Navigation (Public)					//
//											//
//////////////////////////////////////////////

/**
* \author	Darryn Campbell (DCC, JRQ768)
* \date		October 2009
*/
LRESULT CIEMobileEngineTab::Navigate(LPCTSTR tcURL)
{
	//  On Windows Mobile devices it has been observed that attempting to 
	//  navigate to a Javascript function before the page is fully loaded can 
	//  crash PocketBrowser (specifically when using Reload).  This condition
	//  prevents that behaviour.
	if (!m_bPageLoaded && (wcsnicmp(tcURL, L"JavaScript:", wcslen(L"JavaScript:")) == 0))
	{
		DEBUGMSG(TRUE, (L"Failed to Navigate, Navigation in Progress\n"));
		return S_FALSE;
	}

	LRESULT retVal = S_FALSE;
	//  For backwards compatibility test whether or not the user is trying
	//  to navigate to 'history:back' and if they are invoke the history 
	//  object to do so
	if (wcsicmp(tcURL, L"history:back") == 0)
	{
		if (m_BrowserHistory != NULL)
		{
			retVal = m_BrowserHistory->Back(1);
		}
	}
	else
	{
		//  Engine component does not accept Navigate(page.html), it needs
		//  the absolute URL of the page, add that here (if the user puts a .\ before)
		TCHAR tcDereferencedURL[MAX_URL];
		memset(tcDereferencedURL, 0, MAX_URL * sizeof(TCHAR));
		if (IsRelativeURL(tcURL))
		{
			if (!DereferenceURL(tcURL, tcDereferencedURL, m_tcNavigatedURL, NULL))
				return S_FALSE;
		}
		else
			wcscpy(tcDereferencedURL, tcURL);

		//  Test to see if the navigation URL starts with a '\', if it does
		//  then prepend 'file://'
		if (tcDereferencedURL[0] == L'\\')
		{
			if (wcslen(tcDereferencedURL) <= (MAX_URL - wcslen(L"file://")))
			{
				TCHAR tcNewURL[MAX_URL + 1];
				wsprintf(tcNewURL, L"file://%s", tcDereferencedURL);
				retVal = SendMessage(m_hwndTabHTML, DTM_NAVIGATE, 0, (LPARAM) (LPCTSTR)tcNewURL);
			}
		}
		else if (wcslen(tcDereferencedURL) > wcslen(L"www") && wcsnicmp(tcURL, L"www", 3) == 0)
		{
			if (wcslen(tcDereferencedURL) <= (MAX_URL - wcslen(L"http://")))
			{
				TCHAR tcNewURL[MAX_URL + 1];
				wsprintf(tcNewURL, L"http://%s", tcDereferencedURL);
				retVal = SendMessage(m_hwndTabHTML, DTM_NAVIGATE, 0, (LPARAM) (LPCTSTR)tcNewURL);
			}
		}
		else
			retVal = SendMessage(m_hwndTabHTML, DTM_NAVIGATE, 0, (LPARAM) (LPCTSTR)tcDereferencedURL);
	}
	return retVal;
}

/**
* \author	Darryn Campbell (DCC, JRQ768)
* \date		October 2009
*/
LRESULT CIEMobileEngineTab::Stop()
{
	if (PostMessage(m_hwndTabHTML, DTM_STOP, 0, 0))
		return S_OK;
	else
		return S_FALSE;
}

/**
* \author	Darryn Campbell (DCC, JRQ768)
* \date		October 2009
*/
LRESULT CIEMobileEngineTab::Reload(BOOL bFromCache)
{	
	BOOL bRetVal = FALSE;
	if(bFromCache)
		Navigate(L"Javascript:location.reload(false);");
//		bRetVal = PostMessage(m_hwndTabHTML, DTM_NAVIGATE, 0, 
//						(LPARAM)(LPCTSTR)L"Javascript:location.reload(false);");
	else
		Navigate(L"Javascript:location.reload(true);");
		//		bRetVal = PostMessage(m_hwndTabHTML, DTM_NAVIGATE, 0, 
//						(LPARAM)(LPCTSTR)L"Javascript:location.reload(true);");

	if (bRetVal)
		return S_OK;
	else
		return S_FALSE;
}

/**
* \author	Darryn Campbell (DCC, JRQ768)
* \date		October 2009
*/
LRESULT CIEMobileEngineTab::ZoomText(TextZoomValue dwZoomLevel)
{
	BOOL bRetVal = PostMessage(m_hwndTabHTML, DTM_ZOOMLEVEL, 0, 
								(LPARAM)(DWORD) dwZoomLevel);

	if (bRetVal)
	{
		m_dwCurrentTextZoomLevel = dwZoomLevel;
		return S_OK;
	}
	else
		return S_FALSE;
}

/**
* \author	Darryn Campbell (DCC, JRQ768)
* \date		October 2009
*/
LRESULT CIEMobileEngineTab::GetZoomText(TextZoomValue* dwZoomLevel)
{
	*dwZoomLevel = m_dwCurrentTextZoomLevel;
	return S_OK;
}

//////////////////////////////////////////////
//											//
//		HWND Accessors (Public)				//
//											//
//////////////////////////////////////////////

/**
* \author	Darryn Campbell (DCC, JRQ768)
* \date		October 2009
*/
HWND CIEMobileEngineTab::GetHTMLHWND()
{
	return m_hwndTabHTML;
}


//////////////////////////////////////////////
//											//
//		Scrollbars (Public)					//
//											//
//////////////////////////////////////////////

/**
* \author	Darryn Campbell (DCC, JRQ768)
* \date		October 2009
*/
BoolSettingValue CIEMobileEngineTab::SetScrollBars(BoolSettingValue dwBoolSettingValue)
{ 
	return NOT_IMPLEMENTED;
}


/**
* \author	Darryn Campbell (DCC, JRQ768)
* \date		October 2009
*/
BoolSettingValue CIEMobileEngineTab::GetScrollBars()
{
	return m_bsvScrollBars;
}

/**
* \author	Darryn Campbell (DCC, JRQ768)
* \date		October 2009
*/
LONG CIEMobileEngineTab::Scrollbars_HPosSet (LONG lPos)
{
	return -1;
}

/**
* \author	Darryn Campbell (DCC, JRQ768)
* \date		October 2009
*/
LONG CIEMobileEngineTab::Scrollbars_HPosGet ()
{
	return -1;
}

/**
* \author	Darryn Campbell (DCC, JRQ768)
* \date		October 2009
*/
LONG CIEMobileEngineTab::Scrollbars_VPosSet (LONG lPos)
{
	return -1;
}

/**
* \author	Darryn Campbell (DCC, JRQ768)
* \date		October 2009
*/
LONG CIEMobileEngineTab::Scrollbars_VPosGet ()
{
	return -1;
}


//////////////////////////////////////////////////
//												//
//		Application (Tab) Management (Public)	//
//												//
//////////////////////////////////////////////////

/**
* \author	Darryn Campbell (DCC, JRQ768)
* \date		October 2009
*/
LRESULT CIEMobileEngineTab::Tab_Resize(RECT rcNewSize)
{
	m_rcViewSize = rcNewSize;

	//  During testing needed to move both the HTML Component and its
	//  parent (there's only one parent shared by all HTML components)
	//  to prevent painting over the test menu.
	//  Keep this functionality as it seems safest.
	if (MoveWindow(m_hwndTabHTML, m_rcViewSize.left, 
					m_rcViewSize.top, 
					(m_rcViewSize.right-m_rcViewSize.left), 
					(m_rcViewSize.bottom-m_rcViewSize.top), 
					TRUE)
		&&
		MoveWindow(m_hwndTabHTMLContainer, m_rcViewSize.left, 
					m_rcViewSize.top, 
					(m_rcViewSize.right-m_rcViewSize.left), 
					(m_rcViewSize.bottom-m_rcViewSize.top), 
					TRUE))
	{
		return S_OK;
	}
	else
	{
		return S_FALSE;
	}
}

/**
* \author	Darryn Campbell (DCC, JRQ768)
* \date		October 2009
*/
LRESULT CIEMobileEngineTab::Tab_GetTitle (LPTSTR title, int iMaxLen)
{
	memset(title, NULL, sizeof(TCHAR) * iMaxLen);
	wcsncpy(title, m_tcCurrentPageTitle, iMaxLen);
	return S_OK;	//  wcsncpy has no return value to indicate error
}

/**
* \author	Darryn Campbell (DCC, JRQ768)
* \date		October 2009
*/
LRESULT CIEMobileEngineTab::Tab_GetIcon (LPTSTR tcIconURL, int iMaxLen)
{
	memset(tcIconURL, NULL, sizeof(TCHAR) * iMaxLen);
	wcsncpy(tcIconURL, m_tcIconURL, iMaxLen);
	return S_OK;	//  wcsncpy has no return value to indicate error
}

//////////////////////////////////////////////
//											//
//		JavaScript (Public)					//
//											//
//////////////////////////////////////////////


/**
* \author	Darryn Campbell (DCC, JRQ768)
* \date		October 2009
*/
LRESULT CIEMobileEngineTab::JS_Invoke (LPCTSTR tcFunction)
{
	//  Test to see if the passed function starts with "JavaScript:" and 
	//  if it does not then prepend it.
	if (_memicmp(tcFunction, L"JavaScript:", 22))
	{
		//  Function does not start with JavaScript:
		TCHAR* tcURI = new TCHAR[MAX_URL];
		wsprintf(tcURI, L"JavaScript:%s", tcFunction);
		LRESULT retVal;
		retVal = Navigate(tcURI);
		delete[] tcURI;
		return retVal;
	}
	else
	{
		return Navigate(tcFunction);
	}
}

/**
* \author	Darryn Campbell (DCC, JRQ768)
* \date		October 2009
*/
LRESULT CIEMobileEngineTab::JS_Exists (LPCTSTR tcFunction)
{
	//  This function is not implemented in Windows Mobile
	//return E_NOTIMPL;
	return S_OK;
}



//////////////////////////////////////////////
//											//
//		History (Public)					//
//											//
//////////////////////////////////////////////

/**
* \author	Darryn Campbell (DCC, JRQ768)
* \date		October 2009
*/
LRESULT CIEMobileEngineTab::History_GoBack (UINT iNumPages)
{
	if (m_BrowserHistory != NULL)
	{
		return m_BrowserHistory->Back(iNumPages);
	}
	else
		return S_FALSE;
}

/**
* \author	Darryn Campbell (DCC, JRQ768)
* \date		October 2009
*/
LRESULT CIEMobileEngineTab::History_GoForward (UINT iNumPages)
{
	if (m_BrowserHistory != NULL)
	{
		return m_BrowserHistory->Forward(iNumPages);
	}
	else
		return S_FALSE;
}


//////////////////////////////////////////////
//											//
//		Accessors for Properties (Public)	//
//											//
//////////////////////////////////////////////

/**
* \author	Darryn Campbell (DCC, JRQ768)
* \date		October 2009
*/
BoolSettingValue CIEMobileEngineTab::SetClearType(BoolSettingValue dwBoolSettingValue)
{ 
	m_dwClearTypeEnabled = dwBoolSettingValue;
	if (dwBoolSettingValue == SETTING_ON)
		PostMessage(m_hwndTabHTML, DTM_ENABLECLEARTYPE, 0, (LPARAM)TRUE);
	else if (dwBoolSettingValue == SETTING_OFF)
		PostMessage(m_hwndTabHTML, DTM_ENABLECLEARTYPE, 0, (LPARAM)FALSE);

	return m_dwClearTypeEnabled;
}

/**
* \author	Darryn Campbell (DCC, JRQ768)
* \date		October 2009
*/
BoolSettingValue CIEMobileEngineTab::GetClearType()
{ 
	//  ClearType is Common across all Tabs
	return m_dwClearTypeEnabled;
}

/**
* \author	Darryn Campbell (DCC, JRQ768)
* \date		October 2009
*/
BoolSettingValue CIEMobileEngineTab::SetJavaScript(BoolSettingValue dwBoolSettingValue)
{ 
	m_dwJavaScriptEnabled = dwBoolSettingValue;
	if (dwBoolSettingValue == SETTING_ON)
		PostMessage(m_hwndTabHTML, DTM_ENABLESCRIPTING, 0, (LPARAM)TRUE);
	else if (dwBoolSettingValue == SETTING_OFF)
		PostMessage(m_hwndTabHTML, DTM_ENABLESCRIPTING, 0, (LPARAM)FALSE);

	return m_dwJavaScriptEnabled;
}

/**
* \author	Darryn Campbell (DCC, JRQ768)
* \date		October 2009
*/
BoolSettingValue CIEMobileEngineTab::GetJavaScript()
{ 
	//  The JavaScript setting is Common across all Tabs
	return m_dwJavaScriptEnabled;
}

/**
* \author	Darryn Campbell (DCC, JRQ768)
* \date		October 2009
*/
LRESULT CIEMobileEngineTab::SetNavigationTimeout(DWORD dwTimeout)
{
	m_dwNavigationTimeout = dwTimeout;
	return S_OK;
}

/**
* \author	Darryn Campbell (DCC, JRQ768)
* \date		October 2009
*/
DWORD CIEMobileEngineTab::GetNavigationTimeout()
{
	return m_dwNavigationTimeout; 
}

/**
* \author	Darryn Campbell (DCC, JRQ768)
* \date		January 2010
*/
AcceleratorValue CIEMobileEngineTab::SetAcceleratorMode(AcceleratorValue dwAcceleratorValue)
{ 
	//  Accelerate Mode is not implemented on Windows Mobile
	return ACCELERATE_NOT_IMPLEMENTED;
}

/**
* \author	Darryn Campbell (DCC, JRQ768)
* \date		January 2010
*/
AcceleratorValue CIEMobileEngineTab::GetAcceleratorMode()
{ 
	//  Accelerate Mode is not implemented in Windows Mobile
	return ACCELERATE_NOT_IMPLEMENTED;
}

//////////////////////////////////////////////
//											//
//		Private Functions					//
//		Member Variables					//
//											//
//////////////////////////////////////////////

/**
* \author	Darryn Campbell (DCC, JRQ768)
* \date		October 2009
*/
void CIEMobileEngineTab::InvokeEngineEventMetaTag(LPTSTR tcHttpEquiv, LPTSTR tcContent)
{
	if (m_EngineEvents[EEID_METATAG])
	{
		EngineMETATag metaTag;
		memset(&metaTag, 0, sizeof(metaTag));
		metaTag.tcHTTPEquiv = tcHttpEquiv;
		metaTag.tcContents = tcContent;
		m_EngineEvents[EEID_METATAG]
			(EEID_METATAG, 
			(LPARAM)&metaTag,
			m_tabID);
	}	
}

/**
* \author	Darryn Campbell (DCC, JRQ768)
* \date		October 2009
*/
void CIEMobileEngineTab::InvokeEngineEventTitleChange(LPTSTR tcTitle)
{
	//  Notify the core the page title has changed, if not specified between 
	//  <TITLE> </TITLE> tags this should be set to the page URL.
	if(m_EngineEvents[EEID_TITLECHANGE])
	{
		TCHAR tcURL[MAX_URL];
		memset(tcURL, 0, sizeof(MAX_URL) * sizeof(TCHAR));
		wcsncpy(tcURL, tcTitle, MAX_URL);
		m_EngineEvents[EEID_TITLECHANGE](EEID_TITLECHANGE, 
										(LPARAM)tcURL, 
										m_tabID);
		wcscpy(m_tcCurrentPageTitle, tcURL);
	}
}

/**
* \author	Darryn Campbell (DCC, JRQ768)
* \date		October 2009
*/
void CIEMobileEngineTab::InvokeEngineEventLoad(LPTSTR tcURL, EngineEventID eeEventID)
{
	//  Engine component has indicated a load event, this should be 
	//  one of BeforeNavigate, NavigateComplete or DocumentComplete.
	wcscpy(m_tcNavigatedURL, tcURL);
	switch (eeEventID)
	{
		case EEID_BEFORENAVIGATE:
			m_bPageLoaded = FALSE;
			SetEvent(m_hNavigated);
			CloseHandle(m_hNavigated);
			m_hNavigated = NULL;

			//  Do not start the Navigation Timeout Timer if the 
			//  navigation request is a script call.
			if((!_memicmp(tcURL, L"javascript:", 11 * sizeof(TCHAR)))
				|| (!_memicmp(tcURL, L"jscript:", 8 * sizeof(TCHAR)))
				|| (!_memicmp(tcURL, L"vbscript:", 9 * sizeof(TCHAR)))
				|| (!_memicmp(tcURL, L"res://\\Windows\\shdoclc.dll/navcancl.htm", 35 * sizeof(TCHAR))))
			{
					break;
			}

			//  Test if the user has attempted to navigate back in the history
			if (wcsicmp(tcURL, L"history:back") == 0)
			{
				if (m_BrowserHistory != NULL)
				{
					m_BrowserHistory->Back(1);
					break;
				}
			}
			

			CloseHandle (CreateThread(NULL, 0, 
									&CIEMobileEngineTab::NavigationTimeoutThread, 
									(LPVOID)this, 0, NULL));

			if(m_EngineEvents[EEID_BEFORENAVIGATE])
			{
				m_EngineEvents[EEID_BEFORENAVIGATE](EEID_BEFORENAVIGATE, 
													(LPARAM)tcURL, m_tabID);
			}
			break;
		case EEID_DOCUMENTCOMPLETE:
			m_bPageLoaded = TRUE;
			if(m_EngineEvents[EEID_DOCUMENTCOMPLETE])
			{
				m_EngineEvents[EEID_DOCUMENTCOMPLETE](EEID_DOCUMENTCOMPLETE, 
														(LPARAM)tcURL, m_tabID);
			}
			break;
		case EEID_NAVIGATECOMPLETE:
			SetEvent(m_hNavigated);
			CloseHandle(m_hNavigated);
			m_hNavigated = NULL;

			//Validate that there is an event handler
			if(m_EngineEvents[EEID_NAVIGATECOMPLETE])
			{
				m_EngineEvents[EEID_NAVIGATECOMPLETE](EEID_NAVIGATECOMPLETE, 
														(LPARAM)tcURL, m_tabID);
			}
			
			//  Add the URL to the History stack if we have an associated History
			if (m_BrowserHistory != NULL)
			{
				m_BrowserHistory->Add(tcURL);
			}
			break;
	}
}


/**
* \author	Darryn Campbell (DCC, JRQ768)
* \date		November 2009
*/
int CIEMobileEngineTab::GetTabID()
{
	return m_tabID;
}


/**
* \author	James Morley-Smith (JMS, JNP837)
* \date		November 2004
*/
HRESULT CIEMobileEngineTab::RegisterWindowClass(HINSTANCE hInstance, WNDPROC appWndProc) 
{
    WNDCLASS    wc = { 0 };
    HRESULT     hrResult = 0;

    if (!GetClassInfo(hInstance, HTML_CONTAINER_NAME, &wc))
    {
        wc.style            = CS_HREDRAW | CS_VREDRAW;
        wc.lpfnWndProc      = appWndProc;
        wc.hInstance        = hInstance;
		wc.hIcon			= NULL;
        wc.lpszClassName    = HTML_CONTAINER_NAME;
        wc.hCursor          = LoadCursor(NULL, IDC_ARROW);
        wc.hbrBackground    = (HBRUSH)(COLOR_WINDOW + 1);

        hrResult = (RegisterClass(&wc) ? S_OK : E_FAIL);
    }
    else
        hrResult = S_OK;

    return hrResult;
}

//void CIEMobileEngineTab::setTabHTMLControl(HWND controlHWND)
//{
//	m_hwndTabHTMLControl = controlHWND;
//}

//////////////////////////////////////////////
//											//
//		Private Functions					//
//			Static							//
//											//
//////////////////////////////////////////////

/**
* \author	Darryn Campbell (DCC, JRQ768)
* \date		October 2009
*/
DWORD WINAPI CIEMobileEngineTab::NavigationTimeoutThread( LPVOID lpParameter )
{
	DEBUGMSG(1, (L"Mobile NavThread Started\n"));

	CIEMobileEngineTab * pIEEng = (CIEMobileEngineTab*) lpParameter;

	if(pIEEng->m_hNavigated==NULL)
		pIEEng->m_hNavigated = CreateEvent(NULL, TRUE, FALSE, L"PB_IEENGINE_NAVIGATION_IN_PROGRESS"/*NULL*/);

	if(WaitForSingleObject(pIEEng->m_hNavigated, pIEEng->m_dwNavigationTimeout) != WAIT_OBJECT_0)
	{
		//no point in doing anything as there is no event handler
		pIEEng->Stop();
		CloseHandle(pIEEng->m_hNavigated);
		pIEEng->m_hNavigated = NULL;
	
		if(pIEEng->m_EngineEvents[EEID_NAVIGATIONTIMEOUT])
			pIEEng->m_EngineEvents[EEID_NAVIGATIONTIMEOUT](EEID_NAVIGATIONTIMEOUT, (LPARAM)pIEEng->m_tcNavigatedURL, pIEEng->m_tabID);
	}

	DEBUGMSG(1, (L"NavThread Ended\n"));

	return 0;
}

/**
* \author	Darryn Campbell (DCC, JRQ768)
* \date		October 2009
*/
LRESULT CIEMobileEngineTab::WndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	LRESULT lResult = S_OK;
	switch (uMsg) 
	{
		case WM_NOTIFY:
		{
			//  Received a message from the Pocket Internet Explorer component
			//  to indicate something has happened (BeforeNavigate / DocumentComplete etc).
			//  The lParam contains an NM_HTMLVIEWW containing information about
			//  the event, parse it.
			NM_HTMLVIEWW * pnmHTML;
			LPNMHDR pnmh;
			pnmHTML = (NM_HTMLVIEWW *) lParam;
			pnmh = (LPNMHDR) &(pnmHTML->hdr);
			
			//  Check the message originated from an expected window, i.e. 
			//  it came from a PIE component we had created whose HWND is identical
			//  to one of the tabs
			HWND hwndOriginatingPIEHTML = pnmh->hwndFrom;
			//  When we created the HTML tab it created a child window which 
			//  originates these messages, therefore find the parent of the 
			//  originator and ensure it has the correct class name
			HWND hwndParentOfOriginator = GetParent(hwndOriginatingPIEHTML);
			TCHAR* tcOriginatingClassName = new TCHAR[wcslen(WC_HTML)+1];
			GetClassName(hwndParentOfOriginator, tcOriginatingClassName, wcslen(WC_HTML)+1);
			if (wcscmp(tcOriginatingClassName, WC_HTML) != 0)
			{
				delete[] tcOriginatingClassName;
				break;
			}
			delete[] tcOriginatingClassName;

			//  The message has originated from one of the tabs we created, determine
			//  the tab ID from the hwnd
			IETab* tab = GetSpecificTab(hwndParentOfOriginator);
			CIEMobileEngineTab* mobileTab = (CIEMobileEngineTab*)tab->pEngine;

			//  Invoke the appropriate tab with the event.  The data will vary
			//  depending on which event has been received
			TCHAR tcTarget[MAX_URL + 1];
			memset (tcTarget, 0, sizeof(TCHAR) * MAX_URL + 1);
			switch (pnmh->code)
			{
			case NM_PIE_TITLE:
				//  The Page Title has been received from the Page, convert 
				//  the content to a wide string
				if (pnmHTML->szTarget)
					mbstowcs(tcTarget, (LPSTR)pnmHTML->szTarget, MAX_URL);
				if (tcTarget)
					mobileTab->InvokeEngineEventTitleChange(tcTarget);
				break;
			case NM_PIE_META:
				//  A Meta Tag has been received from the Page, convert the content
				//  and data to wide strings.
				if (pnmHTML->szTarget)
					mbstowcs(tcTarget, (LPSTR)pnmHTML->szTarget, MAX_URL);
				TCHAR tcData[MAX_URL+1];
				memset(tcData, 0, sizeof(TCHAR) * MAX_URL + 1);
				if (pnmHTML->szData)
					mbstowcs(tcData, (LPSTR)pnmHTML->szData, MAX_URL);
				//  If there is both an HTTP Equiv and some Content to the Meta
				//  tag then invoke it
				if (tcTarget && tcData)
					mobileTab->InvokeEngineEventMetaTag(tcTarget, tcData);			
				break;
			case NM_PIE_BEFORENAVIGATE:
				if (pnmHTML->szTarget)
					mbstowcs(tcTarget, (LPSTR)pnmHTML->szTarget, MAX_URL);

				// GD - stop navigation if target starts with file:// and ends with '\'.
				// This is the generated target when using <a href=""> from a local page.
				// If we don't stop it then the File Explorer is launched.
				if (wcslen (tcTarget) >= 8)
					if (!wcsnicmp (tcTarget, L"file://", 7))
						if (tcTarget [wcslen (tcTarget) - 1] == '\\')
						{
							DEBUGMSG(TRUE,(L"Navigation to file folder aborted\n"));
							return S_FALSE;
						}
				if (tcTarget)
					mobileTab->InvokeEngineEventLoad(tcTarget, EEID_BEFORENAVIGATE);
				break;
			case NM_PIE_DOCUMENTCOMPLETE:
				if (pnmHTML->szTarget)
					mbstowcs(tcTarget, (LPSTR)pnmHTML->szTarget, MAX_URL);
				//  If the network is available but the server being reached
				//  is inaccessible the browser component appears to immediately
				//  give a document complete with the current page URL (not the
				//  page being navigated to) which is hiding the hourglass, 
				//  stop this behaviour.
				if (tcTarget && !wcsicmp(tcTarget, mobileTab->m_tcNavigatedURL))
					mobileTab->InvokeEngineEventLoad(tcTarget, EEID_DOCUMENTCOMPLETE);
				break;
			case NM_PIE_NAVIGATECOMPLETE:
				if (pnmHTML->szTarget)
					mbstowcs(tcTarget, (LPSTR)pnmHTML->szTarget, MAX_URL);
				if (tcTarget)
					mobileTab->InvokeEngineEventLoad(tcTarget, EEID_NAVIGATECOMPLETE);
				break;
			case NM_PIE_KEYSTATE:
			case NM_PIE_ALPHAKEYSTATE:
				//  Not Used
				break;

			}
		}	
//		lResult = DefWindowProc(hwnd, uMsg, wParam, lParam);
	}
	return lResult;

}