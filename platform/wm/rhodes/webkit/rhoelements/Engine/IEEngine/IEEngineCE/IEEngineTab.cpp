#include "IEEngineTab.h"
#include "atlbase.h"

//#define SCROLL_NOTIFY

#ifdef SCROLL_NOTIFY
#include "../common/ScrollNotify.h"
#endif

//////////////////////////////////////////////
//											//
//		Setup (Public)						//
//											//
//////////////////////////////////////////////

/**
* \author	Darryn Campbell (DCC, JRQ768)
* \author	James Morley-Smith (JMS, JNP837)
* \date		June 2009 (First Created, JMS)
* \date		October 2009 (Modifications for Multiple Instances, DCC)
*/
CIEEngineTab::CIEEngineTab(HINSTANCE hInstance, HWND hwndParent, int tabID, 
						   LPCTSTR tcIconURL, BoolSettingValue bsvScrollbars)
	: m_ulRefs(0)
	, m_bInPlaceActive(true)
	, m_pBrowser(NULL)
	, m_hNavigated(NULL)
	, m_BrowserHistory(NULL)
	, m_AcceleratorMode(ACCELERATE_NORM)
	, m_hwndTabHTML(NULL)
	, bRunningOnWM(FALSE)
	, iLoadPageCount(0)
	, bDeviceCausesDoubleBackspace(FALSE)
{

	pScrollNotify = NULL;

	m_bsvScrollBars = bsvScrollbars;
	m_parentHWND = hwndParent;
	m_hparentInst = hInstance;
	m_tabID = tabID;
	memset(m_tcCurrentPageTitle, NULL, sizeof(TCHAR) * MAX_URL);
	memset(m_tcIconURL, NULL, sizeof(TCHAR) * MAX_URL);
	wcsncpy(m_tcIconURL, tcIconURL, MAX_URL);
	memset(m_tcNavigatedURL, 0, sizeof(TCHAR) * MAX_URL);

	for(int i=0; i < EEID_MAXEVENTID; i++)
		m_EngineEvents[i] = NULL;

	//get the parents window size for the default size of the HTML
	//container.
	GetWindowRect(hwndParent, &m_rcViewSize);

	//initialize the Component Object Model (COM)
	CoInitializeEx(NULL, COINIT_MULTITHREADED);

}

/**
* \author	Darryn Campbell (DCC, JRQ768)
* \author	James Morley-Smith (JMS, JNP837)
* \date		June 2009 (First Created, JMS)
* \date		October 2009 (Modifications for Browser History, DCC)
*/
CIEEngineTab::~CIEEngineTab(void)
{
	if (pScrollNotify)
		delete pScrollNotify;

	//  Remove the browser history object from memory
	delete m_BrowserHistory;
	m_BrowserHistory = NULL;

	//destroy the browser window
	DestroyWindow(m_hwndTabHTML);
	m_hwndTabHTML = NULL;
	//uninitialize the Component Object Model (COM)
	//CoUninitialize();
}

/**
* \author	Darryn Campbell (DCC, JRQ768)
* \author	James Morley-Smith (JMS, JNP837)
* \date		June 2009 (First Created, JMS)
* \date		October 2009 (Modifications for Browser History, DCC)
* \date		February 2010 (Added configFunction parameter in line with Mobile
*                          engine, non functional for CE)
*/
LRESULT CIEEngineTab::CreateEngine(ReadEngineConfigParameter_T configFunction)
{
	IUnknown		*pUnk = NULL,		/// pointer to an IUknown interface object
					*pUnk2 = NULL;		/// pointer to an IUknown interface object

	IOleObject		*pObject = NULL;	/// IOleObject object pointer
	DWORD			dwFlags;			/// shdocvw client flags
	IClassFactory	*ppv;				/// pointer variable to hold WebBrowser interface pointer
	TCHAR			tcConfigSetting[MAX_URL];

	//we only want to do this once, so check that the m_pBrowser is null
	if (!m_pBrowser)
	{
		// See if text selection is enabled, so we can pass the correct value through the GetHostInfo() interface
		configFunction(m_tabID, L"HTMLStyles\\TextSelectionEnabled", tcConfigSetting);
		m_bTextSelectionEnabled = (tcConfigSetting [0] == L'1');

		// Create an instance of a web browser object (from Shdocvw.dll).
		
		//get the interface pointer
		ppv = NULL;
		if (S_OK != CoGetClassObject(CLSID_WebBrowser, CLSCTX_INPROC_SERVER | CLSCTX_INPROC_HANDLER, NULL,IID_IClassFactory, (LPVOID *)(&ppv))) 
			return S_FALSE;

		//create an uninitialized object of a WebBrowser 
		if (S_OK != ppv->CreateInstance(NULL, IID_IUnknown, (LPVOID*)&pUnk))
			return S_FALSE;
		
		// Retrieve an IOleObject interface to set up OLE activation.
		if (S_OK != pUnk->QueryInterface(IID_IOleObject, (LPVOID *)(&pObject))) 
			goto Cleanup;

		// Check if Shdocvw requires a client site to be created first.
		if (S_OK != pObject->GetMiscStatus(DVASPECT_CONTENT, &dwFlags)) 
			goto Cleanup;

		if (dwFlags & OLEMISC_SETCLIENTSITEFIRST)
		{
			// Get an IOleClientSite instance from the browser and pass it to the browser.
			IOleClientSite *pClientSite;
			if (S_OK != QueryInterface(IID_IOleClientSite, (LPVOID *)(&pClientSite))) 
				goto Cleanup;

			if (S_OK != pObject->SetClientSite(pClientSite)) 
				goto Cleanup;

			pClientSite->Release();
		}

		// Activate the object. 
		// Store off the IOleObject reference.
		m_pObject = pObject;
		pObject->AddRef();

		// Retrieve the IWebBrowser2 interface from the IOleObject.
		if (S_OK != pObject->QueryInterface(IID_IWebBrowser2, (void **)&m_pBrowser)) 
			goto Cleanup;
	
		
		if (pObject)
			pObject->Release();


		InitEvents();

		//  If this engine is being run on Windows Mobile set a flag used
		//  later to determine the accelerator key behaviour.
		OSVERSIONINFO osvi;
		memset(&osvi, 0, sizeof(OSVERSIONINFO));
		osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
		GetVersionEx(&osvi);
		bRunningOnWM = (osvi.dwMajorVersion == 5 && osvi.dwMinorVersion == 2) ||
		     (osvi.dwMajorVersion == 5 && osvi.dwMinorVersion == 1);

		//  Find out the current operating system version, this
		//  will be used later to determine whether this device is subject
		//  to EMBPD00023872 which causes two back spaces when pressing the 
		//  delete key.
		//  Operating system is stored in osvi

		//  Devices which cause a Double Backspace:
		//  1. MC3190 CE 6 build 3122
		if (osvi.dwMajorVersion == 6 && 
			osvi.dwMinorVersion == 0 &&	osvi.dwBuildNumber >= 3122)
		{
			bDeviceCausesDoubleBackspace = TRUE;
		}

		//  Create an associated History Object
		m_BrowserHistory = new CHistory(this);
	}

Cleanup:
	if (pUnk)
		pUnk->Release();
	if (pObject)
		pObject->Release();

	IOleWindow *pWnd = NULL;

	if (m_pBrowser)
	{
		if (S_OK !=  m_pBrowser->QueryInterface(IID_IOleWindow, (LPVOID *)(&pWnd)))
			return S_FALSE;
	}

	if (pWnd)
	{
		pWnd->GetWindow(&m_hwndTabHTML);
		pWnd->Release();
	}

	if (pUnk2)
		pUnk2->Release();

	//make the html window a child of the container window (hwndParent)
	//must unset the popup flag and set the child flag
	//then call SetParent
	LONG lStyle = GetWindowLong(m_hwndTabHTML, GWL_STYLE);
	lStyle ^= WS_POPUP;
	lStyle |= WS_CHILD;
	SetWindowLong(m_hwndTabHTML, GWL_STYLE, lStyle);
	SetParent(m_hwndTabHTML, m_parentHWND);

	ShowWindow(m_hwndTabHTML, SW_SHOW);
	SetForegroundWindow(m_hwndTabHTML);
	MoveWindow(m_hwndTabHTML, m_rcViewSize.left, m_rcViewSize.top, (m_rcViewSize.right-m_rcViewSize.left), (m_rcViewSize.bottom-m_rcViewSize.top), TRUE);

	return S_OK;		
};

/**
* \author	Darryn Campbell (DCC, JRQ768)
* \date		October 2009 
*/
LRESULT CIEEngineTab::RegisterForEvent(EngineEventID eeidEventID, ENGINEEVENTPROC pEventFunc)
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
LRESULT CIEEngineTab::PreprocessMessage(MSG msg)
{
	//  Call Translate Accelerator on the message to enable the browser to have
	//  first stab at translating the accelerator keys.
	IDispatch* pDisp;
	m_pBrowser->get_Document(&pDisp);
	if (pDisp == NULL)
	{
		return S_FALSE;
	}
	else if(msg.message == WM_KEYDOWN && GetAcceleratorMode() == ACCELERATE_OFF)
	{
		//  Accelerate OFF
		switch (msg.wParam)
		{
			case VK_TAB:
			case VK_RETURN:
			case VK_BACK:
			case VK_LEFT:
			case VK_UP:
			case VK_RIGHT:
			case VK_DOWN:
				//  Don't Translate or process the key any further
				return S_OK;
		}
	}

	//  If the Key is back we do not want to translate it causing the browser
	//  to navigate back.  If Accelerate is ON we do want the browser to 
	//  navigate back.  Note we do not want this behaviour when running on 
	//  Windows Mobile as it causes backspace to delete the previous two 
	//  characters.
	else if (msg.message == WM_KEYDOWN && 
    		msg.wParam == (WPARAM)VK_BACK && 
			GetAcceleratorMode() == ACCELERATE_NORM &&
			bRunningOnWM == FALSE)
	{
		//  Special behaviour is observed on the MC3190 CE6 device (but not 
		//  the MC3100 CE6 device)
		if (bDeviceCausesDoubleBackspace)
			return ACCELERATE_KEY_DONT_TRANSLATE;
		else
			return S_FALSE;
	}
	//  If the key is Enter we do not wish to translate when ACCELERATE_NORM
	else if (msg.message == WM_KEYDOWN &&
			msg.wParam == (WPARAM)VK_RETURN &&
			GetAcceleratorMode() == ACCELERATE_NORM)
	{
		return S_OK;
	}
	else if (msg.message == WM_KEYDOWN)
	{
		//  Translate all remaining Accelerators
		IOleInPlaceActiveObject* pInPlaceObject;
		pDisp->QueryInterface( IID_IOleInPlaceActiveObject, (void**)&pInPlaceObject);
		HRESULT handleKey = pInPlaceObject->TranslateAccelerator(&msg);
		return handleKey;
	}
	return S_FALSE;
}

/**
* \author	Darryn Campbell (DCC, JRQ768)
* \date		October 2009
*/
ENGINEEVENTPROC* CIEEngineTab::GetEngineEvents()
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
LRESULT CIEEngineTab::Navigate(LPCTSTR tcURL)
{
	LRESULT retVal = S_FALSE;
	if (!tcURL || wcslen(tcURL) == 0)
		return S_FALSE;

	//  For backwards compatibility test whether or not the user is trying
	//  to navigate to 'history:back' and if they are invoke the history 
	//  object to do so
	if (wcslen(tcURL) >= wcslen(L"history:back") && 
		wcsicmp(tcURL, L"history:back") == 0)
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

		//  Test to see if the navigateion URL starts with a '/', if it does
		//  then convert that to a '\'.  Sometimes this happens with <a name> 
		//  tags
		if (tcDereferencedURL[0] == L'/')
		{
			if (wcslen(tcDereferencedURL) <= (MAX_URL - wcslen(L"file://") + 1))
			{
				TCHAR tcNewURL[MAX_URL + 1];
				wsprintf(tcNewURL, L"file://\\%s", tcDereferencedURL + 1);
				if (S_OK == m_pBrowser->Navigate(BSTR(tcNewURL), NULL, NULL, NULL, NULL))
					retVal = S_OK;
				else
					retVal = S_FALSE;
			}
		}
		else
		{

			if (S_OK == m_pBrowser->Navigate(BSTR(tcDereferencedURL), NULL, NULL, NULL, NULL))
				retVal = S_OK;
			else
				retVal = S_FALSE;
		}
	}
	return retVal;
}

/**
* \author	Darryn Campbell (DCC, JRQ768)
* \date		October 2009
*/
LRESULT CIEEngineTab::Stop()
{
	if(S_OK == m_pBrowser->Stop())
	{
		//  The IWebBrowser2 engine does not give a NavComplete or a DocComplete
		//  event when you call Stop() on it, therefore need to do this manually
		SetEvent(m_hNavigated);
		CloseHandle(m_hNavigated);
		m_hNavigated = NULL;
		if (m_EngineEvents[EEID_NAVIGATECOMPLETE])
			m_EngineEvents[EEID_NAVIGATECOMPLETE](EEID_NAVIGATECOMPLETE, 
												(LPARAM)L"NavigationStopped", 
												m_tabID);
		if (m_EngineEvents[EEID_DOCUMENTCOMPLETE])
			m_EngineEvents[EEID_DOCUMENTCOMPLETE](EEID_DOCUMENTCOMPLETE, 
												(LPARAM)L"NavigationStopped", 
												m_tabID);
		return S_OK;
	}
	return S_FALSE;
	
}

/**
* \author	Darryn Campbell (DCC, JRQ768)
* \date		October 2009
*/
LRESULT CIEEngineTab::Reload(BOOL bFromCache)
{	
	VARIANT var;
	if(bFromCache)
		var.intVal = REFRESH_NORMAL;
	else 
		var.intVal = REFRESH_COMPLETELY;

	if (S_OK == m_pBrowser->Refresh2( &var ))
	{	
		//  Do not reload meta tags when the browser is reloaded in line with 
		//  PocketBrowser 2.x
		//ParseTags(m_pBrowser);
		return S_OK;
	}
	else
		return S_FALSE;
}

/**
* \author	Darryn Campbell (DCC, JRQ768)
* \date		October 2009
*/
LRESULT CIEEngineTab::ZoomText(TextZoomValue dwZoomLevel)
{
	LPDISPATCH pDisp = NULL;
	LPOLECOMMANDTARGET pCmdTarg = NULL;
	if (S_OK != m_pBrowser->get_Document(&pDisp)) 
		return S_FALSE;
	if (pDisp == NULL)
		return S_FALSE;
	pDisp->QueryInterface(IID_IOleCommandTarget, (LPVOID*)&pCmdTarg);
	if (pCmdTarg == NULL)
		return S_FALSE;
	VARIANT vaZoomFactor;   // input arguments
	VariantInit(&vaZoomFactor);
	V_VT(&vaZoomFactor) = VT_I4;
	V_I4(&vaZoomFactor) = dwZoomLevel;
	pCmdTarg->Exec(NULL,
				OLECMDID_ZOOM,
				OLECMDEXECOPT_DONTPROMPTUSER,
				&vaZoomFactor,
				NULL);
	VariantClear(&vaZoomFactor);
	if (pCmdTarg)
	   pCmdTarg->Release(); // release document's command target
	if (pDisp)
	   pDisp->Release();    // release document's dispatch interface
	return S_OK;
}

/**
* \author	Darryn Campbell (DCC, JRQ768)
* \date		October 2009
*/
LRESULT CIEEngineTab::GetZoomText(TextZoomValue *dwZoomLevel)
{
	LRESULT retVal = S_FALSE;
	LPDISPATCH pDisp = NULL;
	LPOLECOMMANDTARGET pCmdTarg = NULL;
	if (S_OK != m_pBrowser->get_Document(&pDisp)) 
		return retVal;
	if (pDisp == NULL)
		return retVal;
	pDisp->QueryInterface(IID_IOleCommandTarget, (LPVOID*)&pCmdTarg);
	if (pCmdTarg == NULL)
		return retVal;
	VARIANT vaZoomFactor;   // input arguments
	VariantInit(&vaZoomFactor);
	pCmdTarg->Exec(NULL,
				OLECMDID_ZOOM,
				OLECMDEXECOPT_DONTPROMPTUSER,
				NULL,
				&vaZoomFactor);
	//  vaZoomFactor contains the current zoom level of the page
	retVal = S_OK;
	if (V_I4(&vaZoomFactor) == (DWORD)TEXT_ZOOM_SMALLEST)
		*dwZoomLevel = TEXT_ZOOM_SMALLEST;
	else if (V_I4(&vaZoomFactor) == (DWORD)TEXT_ZOOM_SMALLER)
		*dwZoomLevel = TEXT_ZOOM_SMALLER;
	else if (V_I4(&vaZoomFactor) == (DWORD)TEXT_ZOOM_NORMAL)
		*dwZoomLevel = TEXT_ZOOM_NORMAL;
	else if (V_I4(&vaZoomFactor) == (DWORD)TEXT_ZOOM_BIGGER)
		*dwZoomLevel = TEXT_ZOOM_BIGGER;
	else if (V_I4(&vaZoomFactor) == (DWORD)TEXT_ZOOM_BIGGEST)
		*dwZoomLevel = TEXT_ZOOM_BIGGEST;
	else
	{
		retVal = S_FALSE;
		dwZoomLevel = NULL;
	}
	VariantClear(&vaZoomFactor);
	if (pCmdTarg)
	   pCmdTarg->Release(); // release document's command target
	if (pDisp)
	   pDisp->Release();    // release document's dispatch interface
	return retVal;
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
HWND CIEEngineTab::GetHTMLHWND()
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
BoolSettingValue CIEEngineTab::SetScrollBars(BoolSettingValue dwBoolSettingValue)
{
	return m_bsvScrollBars = dwBoolSettingValue;
}

/**
* \author	Darryn Campbell (DCC, JRQ768)
* \date		October 2009
*/
BoolSettingValue CIEEngineTab::GetScrollBars()
{
	return m_bsvScrollBars;
}

/**
* \author	Darryn Campbell (DCC, JRQ768)
* \date		October 2009
*/
LONG CIEEngineTab::Scrollbars_HPosSet (LONG lPos)
{
	return AdjustCurrentScroll(SCROLLBAR_HORIZONTAL, lPos);
}

/**
* \author	Darryn Campbell (DCC, JRQ768)
* \date		October 2009
*/
LONG CIEEngineTab::Scrollbars_HPosGet ()
{
	return AdjustCurrentScroll(SCROLLBAR_HORIZONTAL, -1);
}

/**
* \author	Darryn Campbell (DCC, JRQ768)
* \date		October 2009
*/
LONG CIEEngineTab::Scrollbars_VPosSet (LONG lPos)
{
	return AdjustCurrentScroll(SCROLLBAR_VERTICAL, lPos);
}

/**
* \author	Darryn Campbell (DCC, JRQ768)
* \date		October 2009
*/
LONG CIEEngineTab::Scrollbars_VPosGet ()
{
	return AdjustCurrentScroll(SCROLLBAR_VERTICAL, -1);
}

/**
* \author	Geoff Day (GRD, XFH386)
* \date		March 2010
*/
BOOL CIEEngineTab::Scrollbars_SizeGet (int *pwidth, int *pheight)
{
	return GetPageSize (pwidth, pheight);
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
LRESULT CIEEngineTab::Tab_Resize(RECT rcNewSize)
{
	m_rcViewSize = rcNewSize;

	if (MoveWindow(m_hwndTabHTML, m_rcViewSize.left, 
									m_rcViewSize.top, 
									(m_rcViewSize.right-m_rcViewSize.left), 
									(m_rcViewSize.bottom-m_rcViewSize.top), 
									TRUE))
		return S_OK;
	else
		return S_FALSE;
}

/**
* \author	Darryn Campbell (DCC, JRQ768)
* \date		October 2009
*/
LRESULT CIEEngineTab::Tab_GetTitle (LPTSTR title, int iMaxLen)
{
	memset(title, NULL, sizeof(TCHAR) * iMaxLen);
	wcsncpy(title, m_tcCurrentPageTitle, iMaxLen);
	return S_OK;	//  wcsncpy has no return value to indicate error
}

/**
* \author	Darryn Campbell (DCC, JRQ768)
* \date		October 2009
*/
LRESULT CIEEngineTab::Tab_GetIcon (LPTSTR tcIconURL, int iMaxLen)
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
LRESULT CIEEngineTab::JS_Invoke (LPCTSTR tcFunction)
{
	//  Unable to invoke JavaScript directly for Arrays, only for basic strings
	//  therefore just call navigate
	if (S_OK != JS_Exists(tcFunction))
	{
		return S_FALSE;
	}
	else
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
}

/**
* \author	Darryn Campbell (DCC, JRQ768)
* \date		October 2009
*/
LRESULT CIEEngineTab::JS_Exists (LPCTSTR tcFunction)
{
	//  tcFunction is in the form doScan('12345678', 0x30, 00:00:00, 'Decode');
	//  or something like that, need to split this into function name and 
	//  parameters
	TCHAR functionName[MAX_URL];
	memset(functionName, 0, sizeof(functionName));
	//  This is made tricky because we ignore white space between characters
	TCHAR* functionNamePtr = functionName;
	TCHAR* pParen = wcschr(tcFunction, '(');
	int posOfFirstParen = pParen-tcFunction;
	int startPos = 0;
	//  If this function has been called with JavaScript: then increase the start
	//  position of the Function name
	if (!_memicmp(tcFunction, L"JavaScript:", 22))
		startPos = 11;
	for (int i=startPos; i < posOfFirstParen; i++)
	{
		//  Populate the function Name
		if (tcFunction[i] != ' ')
		{
			*functionNamePtr = tcFunction[i];
			functionNamePtr++;
		}
	}
	OLECHAR FAR* szJSMethodName = (OLECHAR *)functionName;
	//  Declare a pointer to an IDispatch object
	LPDISPATCH pDisp = (IDispatch FAR*)NULL;

	BOOL retVal = S_FALSE;

	//  Set the IDispatch object to the document loaded in the browser
	m_pBrowser->get_Document(&pDisp);

	if (pDisp != NULL)
	{
		//  Provided there was a document loaded in the browser component populate
		//  an IHTMLDocument2 object, this contains (amongst other things) a reference
		//  to the scripts in the loaded document.
		IHTMLDocument2* pHTMLDocument2;
		HRESULT hr;
		hr = pDisp->QueryInterface( IID_IHTMLDocument2, (void**)&pHTMLDocument2 );

		//  Set the IDispatch pointer to the scripts in the document
		hr = pHTMLDocument2->get_Script(&pDisp);
		//  dispid will hold a reference to the JavaScript function we want to call
		DISPID dispid;
		
		//  Obtain the id of the JavaScript method with the name 'szJSMethodName'
		hr = pDisp->GetIDsOfNames(IID_NULL, &szJSMethodName, 1, LOCALE_SYSTEM_DEFAULT, &dispid);
		if (hr != DISP_E_UNKNOWNNAME)
		{
			//  Recognised Function name
			retVal = S_OK;
		}
	}
	return retVal;
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
LRESULT CIEEngineTab::History_GoBack (UINT iNumPages)
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
LRESULT CIEEngineTab::History_GoForward (UINT iNumPages)
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
BoolSettingValue CIEEngineTab::SetClearType(BoolSettingValue dwBoolSettingValue)
{ 
	return NOT_IMPLEMENTED;
}

/**
* \author	Darryn Campbell (DCC, JRQ768)
* \date		October 2009
*/
BoolSettingValue CIEEngineTab::GetClearType()
{ 
	//  ClearType is Common across all Tabs
	return NOT_IMPLEMENTED;
}

/**
* \author	Darryn Campbell (DCC, JRQ768)
* \date		October 2009
*/
BoolSettingValue CIEEngineTab::SetJavaScript(BoolSettingValue dwBoolSettingValue)
{ 
	return NOT_IMPLEMENTED;
}

/**
* \author	Darryn Campbell (DCC, JRQ768)
* \date		October 2009
*/
BoolSettingValue CIEEngineTab::GetJavaScript()
{ 
	//  ClearType is Common across all Tabs
	return NOT_IMPLEMENTED;
}

/**
* \author	Darryn Campbell (DCC, JRQ768)
* \date		October 2009
*/
LRESULT CIEEngineTab::SetNavigationTimeout(DWORD dwTimeout)
{
	m_dwNavigationTimeout = dwTimeout;
	return S_OK;
}

/**
* \author	Darryn Campbell (DCC, JRQ768)
* \date		October 2009
*/
DWORD CIEEngineTab::GetNavigationTimeout()
{
	return m_dwNavigationTimeout; 
}

/**
* \author	Darryn Campbell (DCC, JRQ768)
* \date		December 2009
*/
AcceleratorValue CIEEngineTab::SetAcceleratorMode(AcceleratorValue dwAcceleratorValue)
{ 
	//  Accelerate Mode is tab specific
	m_AcceleratorMode = dwAcceleratorValue;
	return m_AcceleratorMode;
}

/**
* \author	Darryn Campbell (DCC, JRQ768)
* \date		October 2009
*/
AcceleratorValue CIEEngineTab::GetAcceleratorMode()
{ 
	//  Accelerate Mode is tab specific
	return m_AcceleratorMode;
}


//////////////////////////////////////////////
//											//
//		Private Functions					//
//		Non COM Object						//
//											//
//////////////////////////////////////////////

/**
* \author	Darryn Campbell (DCC, JRQ768)
* \date		October 2009
*/
HRESULT CIEEngineTab::ParseTags()
{
	IDispatch* pDisp;
	m_pBrowser->get_Document(&pDisp);
		
	if (pDisp != NULL )
	{
		IHTMLDocument2* pHTMLDocument2;
		HRESULT hr;
		
		hr = pDisp->QueryInterface( IID_IHTMLDocument2, (void**)&pHTMLDocument2 );
		// Finished with pDisp so release
		pDisp->Release();

		if (hr == S_OK)
		{
			IHTMLElementCollection* pColl;
			hr = pHTMLDocument2->get_all( &pColl );
			
			// Finished so release
			pHTMLDocument2->Release();

			if (hr == S_OK)
			{
				LONG celem;
				hr = pColl->get_length( &celem );

				if ( hr == S_OK )
				{
					for ( int i=0; i< celem; i++ )
					{
						VARIANT varIndex;
						varIndex.vt = VT_UINT;
						varIndex.lVal = i;
						VARIANT var2;
						VariantInit( &var2 );
						IDispatch* pDisp2; 

						hr = pColl->item( varIndex, var2, &pDisp2 );

						if ( hr == S_OK )
						{
							BSTR bstr;
							memset(&bstr, 0, sizeof(BSTR));
							IHTMLMetaElement* pMetaElem;
							hr = pDisp2->QueryInterface( IID_IHTMLMetaElement, 
								(void **)&pMetaElem );
							if ( hr == S_OK )
							{
								//  Invoke Meta Tag Event if somebody has registered
								if (m_EngineEvents[EEID_METATAG])
								{
									//  The engine uses its own definition of 
									//  meta tags, must HTTP Equiv and Contents
									EngineMETATag metaTag;
									memset(&metaTag, 0, sizeof(metaTag));
									TCHAR tcHttpEquiv[MAX_URL], tcContents[MAX_URL];
									memset(tcHttpEquiv, 0, MAX_URL);
									memset(tcContents, 0, MAX_URL);
									//  Obtain the HTTP Equiv from the IE 
									//  component, stored in bstr
									if (S_OK == pMetaElem->get_httpEquiv(&bstr)) 
									{
										if (bstr != 0) 
										{
											//  Copy the HTTP Equiv returned
											//  from the IE Component into our 
											//  Meta Tag Structure
											wcsncpy(tcHttpEquiv, bstr, MAX_URL);
											metaTag.tcHTTPEquiv = tcHttpEquiv;
											::SysFreeString(bstr);
										}
										else if	(S_OK == pMetaElem->get_name(&bstr))
										{
											//  Failed to get the HTTP Equiv, try and get the <META Name...>
											if (bstr != 0)
											{
												//  Copy the HTTP Equiv returned
												//  from the IE Component into our 
												//  Meta Tag Structure
												wcsncpy(tcHttpEquiv, bstr, MAX_URL);
												metaTag.tcHTTPEquiv = tcHttpEquiv;
												::SysFreeString(bstr);
											}
										}
									}
									if (metaTag.tcHTTPEquiv && S_OK == pMetaElem->get_content(&bstr)) 
									{
										if (bstr != 0) 
										{
											//  Copy the Contents returned
											//  from the IE component into our
											//  Meta Tag Structure
											wcsncpy(tcContents, bstr, MAX_URL);
											metaTag.tcContents = (TCHAR*) tcContents;
											::SysFreeString(bstr);
											//  Invoke the Meta Tag Callback.
											//  This blocks whilst the callback
											//  code is handled so metaTag does not
											//  go out of scope.
											m_EngineEvents[EEID_METATAG]
												(EEID_METATAG, 
												(LPARAM)&metaTag, 
												m_tabID);
										}
									}
								}
								pMetaElem->Release();
							}
							pDisp2->Release();
						}
					}
				}
				pColl->Release();
			}
		}
	}
	return S_OK;
}

/**
* \author	Geoff Day (GRD, XFH386)
* \date		March 2010
*/
BOOL CIEEngineTab::GetPageSize (int *pwidth, int *pheight)
{
	// Get size of rendered page

	IDispatch *pdisp_doc = NULL;
	IHTMLDocument2 *pdoc = NULL;
	IHTMLElement *pbody1 = NULL;
	IHTMLElement2 *pbody2 = NULL;
	IHTMLRect *prect;
	int left, top, right, bottom;
	HRESULT hr;

	hr = m_pBrowser->get_Document (&pdisp_doc);
	if (hr != S_OK)
		goto Exit;

	hr = pdisp_doc->QueryInterface (IID_IHTMLDocument2, (void**) &pdoc);
	if (hr != S_OK)
		goto Exit;

	hr = pdoc->get_body (&pbody1);
	if (hr != S_OK)
		goto Exit;

	hr = pbody1->QueryInterface (IID_IHTMLElement2, (void**) &pbody2);
	if (hr != S_OK)
		goto Exit;

	hr = pbody2->getBoundingClientRect (&prect);
	if (hr != S_OK)
		goto Exit;

	prect->get_left ((long*) &left);
	prect->get_top ((long*) &top);
	prect->get_right ((long*) &right);
	prect->get_bottom ((long*) &bottom);

	*pwidth = right - left;
	*pheight = bottom- top;

Exit:
	if (pbody2)
		pbody2->Release ();

	if (pbody1)
		pbody1->Release ();

	if (pdoc)
		pdoc->Release ();

	if (pdisp_doc)
		pdisp_doc->Release ();

	return hr == S_OK;
}

/**
* \author	Darryn Campbell (DCC, JRQ768)
* \date		October 2009
*/
LONG CIEEngineTab::AdjustCurrentScroll (ScrollbarOrientation eAxis, LONG lNewValue)
{
	LONG scroll; 
	//  Declare a pointer to an IDispatch object
	LPDISPATCH pDisp = (IDispatch FAR*)NULL;
	//  Set the IDispatch object to the document loaded in the browser
	m_pBrowser->get_Document(&pDisp);

	if (pDisp != NULL)
	{
		//  Provided there was a document loaded in the browser component populate
		//  an IHTMLDocument2 object, this contains (amongst other things) a reference
		//  to the scripts in the loaded document.
		IHTMLDocument2* pHTMLDocument2;
		HRESULT hr = pDisp->QueryInterface( IID_IHTMLDocument2, (void**)&pHTMLDocument2 );
		if (S_OK != hr)
			return -1;
		IHTMLElement *pBody = NULL;
		hr = pHTMLDocument2->get_body( &pBody );
		if (S_OK != hr)
			return -1;
	    // from body we can get element2 interface,
	    // which allows us to do scrolling
		IHTMLElement2 *pElement = NULL;
	    hr = pBody->QueryInterface(IID_IHTMLElement2,(void**)&pElement);

		//  Adjust the specified scrollbar if that's what the caller wanted to do.
		if (lNewValue >= 0)
		{
			if (eAxis == SCROLLBAR_VERTICAL)
				pElement->put_scrollTop(lNewValue);
			else
				pElement->put_scrollLeft(lNewValue);
		}
		if (eAxis == SCROLLBAR_VERTICAL)
			pElement->get_scrollTop( &scroll);
		else
			pElement->get_scrollLeft(&scroll);
		return scroll;
	}
	else
	{
		//  Unable to retrieve IDispatch*
		return -1;
	}

	//  Method successfully invoked
	return scroll;
}

/**
* \author	Darryn Campbell (DCC, JRQ768)
* \date		November 2009
*/
int CIEEngineTab::GetTabID()
{
	return m_tabID;
}

/**
* \author	Darryn Campbell (DCC, JRQ768)
* \date		October 2009
*/
DWORD WINAPI CIEEngineTab::NavigationTimeoutThread( LPVOID lpParameter )
{
	DEBUGMSG(1, (L"NavThread Started\n"));

	CIEEngineTab * pIEEng = (CIEEngineTab*) lpParameter;

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


//////////////////////////////////////////////
//											//
//		Private Functions					//
//		COM Object Maintenance				//
//											//
//////////////////////////////////////////////

/**
* \author	James Morley-Smith (JMS, JNP837)
* \date		June 2009 (First Created, JMS)
*/
HRESULT CIEEngineTab::InitEvents()
{
	HRESULT                     hr;
	IConnectionPointContainer  *pCPCont = NULL;
	DWebBrowserEvents          *pEvents = NULL;
	DWORD _dwEventCookie;

	if (!m_pBrowser)
		return S_FALSE;

	hr = m_pObject->DoVerb(OLEIVERB_UIACTIVATE, NULL, this, 0, m_parentHWND, &m_rcViewSize);
	if (hr == OLE_E_NOT_INPLACEACTIVE)
		return S_FALSE;

	// Get the connection point container for the browser object.
	hr = m_pBrowser->QueryInterface(IID_IConnectionPointContainer, (LPVOID *)&pCPCont);
	//DWORD dw = GetLastError();
	if (hr)
		return S_FALSE;

	// Look for DWebBrowserEvents2 connection point.
	hr = pCPCont->FindConnectionPoint(DIID_DWebBrowserEvents2, &m_pCP);
	if (hr)
	{
		m_pCP = NULL;
		goto Cleanup;
	}

	// Get a DWebBrowserEvents2 pointer from the browser.
	hr = QueryInterface(DIID_DWebBrowserEvents2, (LPVOID *)(&pEvents));
	if (hr)
		goto Cleanup;

	// Add your event sink to the connectionpoint.
	hr = m_pCP->Advise(pEvents, &(_dwEventCookie));
	if (hr)
		goto Cleanup;


	Cleanup:
	if (pCPCont)
		pCPCont->Release();
	if (pEvents)
		pEvents->Release();

	return S_OK;

}

/**
* \author	James Morley-Smith (JMS, JNP837)
* \date		June 2009 (First Created, JMS)
*/
HRESULT CIEEngineTab::GetHostInfo( DOCHOSTUIINFO* pInfo )
{
	pInfo->cbSize   = sizeof(DOCHOSTUIINFO);
	
	if (GetScrollBars() == SETTING_ON) {
		pInfo->dwFlags = DOCHOSTUIFLAG_NO3DBORDER;
	}
	else {
		pInfo->dwFlags = DOCHOSTUIFLAG_NO3DBORDER|DOCHOSTUIFLAG_SCROLL_NO ;
	}

	if (!m_bTextSelectionEnabled)
		pInfo->dwFlags |= DOCHOSTUIFLAG_DIALOG;

	pInfo->dwDoubleClick = DOCHOSTUIDBLCLK_DEFAULT;

	return S_OK;
}

/**
* \author	James Morley-Smith (JMS, JNP837)
* \date		June 2009 (First Created, JMS)
* \todo		Modify this function to implement AccelerateKey functionality
*/
HRESULT CIEEngineTab::TranslateAccelerator( 
			LPMSG lpMsg,
			const GUID __RPC_FAR *pguidCmdGroup,
			DWORD nCmdID)
{
//	if (lpMsg && (lpMsg->message == WM_KEYDOWN) && 
//		GetAcceleratorMode() == ACCELERATE_ON)
//	{
//		if (lpMsg->wParam == VK_LEFT ||
//					lpMsg->wParam == VK_RIGHT ||
//					lpMsg->wParam == VK_UP ||
//					lpMsg->wParam == VK_DOWN ||
//					//lpMsg->wParam == VK_BACK ||
//					lpMsg->wParam == VK_RETURN)
//		{
//			//  Prevent duplicate left, right, up and down keys
//			return S_OK;
//		}
//	}


	//  Under certain circumstances duplicate key messages are sent to the
	//  HTML control, this allows us to perform different actions depending 
	//  on whether the keyboard input is occuring within or outside of a 
	//  textbox.  For example we do not wish to action a backspace when in
	//  ACCLERATE_NORM and outside a text box as this would navigate back to 
	//  the previous page but we do wish to process it within a text box.
/*
	//  Only process Keydown messages
	if (lpMsg && (lpMsg->message == WM_KEYDOWN))
	{
		if (lpMsg->wParam == VK_BACK && GetAcceleratorMode() == ACCELERATE_NORM)
		{
			//  Don't navigate to previous page if ACCELERATE_NORM in effect and
			//  NO textbox has focus.
			return S_OK;
		}
//		else if (lpMsg->wParam == VK_LEFT ||
//					lpMsg->wParam == VK_RIGHT ||
//					lpMsg->wParam == VK_UP ||
//					lpMsg->wParam == VK_DOWN)
//		{
//			//  Prevent duplicate left, right, up, down and enter keys when inside 
//			//  textboxes.
//			return S_OK;  
//		}
		else
		{
			//  Default behaviour is to pass the keystroke onto the application
			//  for processing.
			return S_FALSE;
		}
	}
*/
	return S_FALSE;
}

/**
* \author	James Morley-Smith (JMS, JNP837)
* \date		June 2009 (First Created, JMS)
*/
HRESULT CIEEngineTab::QueryInterface (REFIID riid, LPVOID * ppv)
{
	if ((riid == IID_IOleContainer) || (riid == IID_IUnknown))
	{
		*ppv = (IOleContainer *) this;
	}
	else if (riid == IID_IOleClientSite)
	{
		*ppv = (IOleClientSite *)this;
	}
	else if (riid == IID_IOleInPlaceSite)
	{
		*ppv = (IOleInPlaceSite *)this;
	}
	else if (riid == DIID_DWebBrowserEvents2)
	{
		*ppv = (DWebBrowserEvents2 *)this;
	}
	else if (riid == IID_IDocHostShowUI)
	{
   		*ppv = (IDocHostShowUI *)this;
	}
	else if (riid == IID_IDocHostUIHandler)
	{
   		*ppv = (IDocHostUIHandler *)this;
	}
	else
	{
		*ppv = NULL;
		return E_NOINTERFACE;
	}
	
	AddRef();

	return S_OK;
}

/**
* \author	James Morley-Smith (JMS, JNP837)
* \date		June 2009 (First Created, JMS)
*/
ULONG CIEEngineTab::AddRef(void)
{
	//safely increment the reference counter
	InterlockedIncrement((LONG*)&m_ulRefs);
	return m_ulRefs;
}

/**
* \author	James Morley-Smith (JMS, JNP837)
* \date		June 2009 (First Created, JMS)
*/
ULONG CIEEngineTab::Release(void)
{
	//safely decrement the reference counter
	if (InterlockedDecrement((LONG*)&m_ulRefs) == 0)
	{
		delete this;
		return 0;
	}
	return m_ulRefs;
}

HRESULT CIEEngineTab::Scroll (SIZE scrollExtent)
{

	return S_OK;
}

/**
* \author	James Morley-Smith (JMS, JNP837)
* \date		June 2009 (First Created, JMS)
*/
HRESULT CIEEngineTab::GetWindow(HWND *phwnd)
{
	*phwnd = m_hwndTabHTML;
	return S_OK;
}

/**
* \author	James Morley-Smith (JMS, JNP837)
* \author	Darryn Campbell (DCC, JRQ768)
* \date		June 2009 (First Created, JMS)
* \date		October 2009 (Modifications for Multiple Instances, DCC)
* \todo		Investigate invoking JavaScript directly (Issue with Array Parameters).
*/
HRESULT CIEEngineTab::Invoke(DISPID dispidMember,REFIID riid,LCID lcid,WORD wFlags,
					  DISPPARAMS FAR* pdparams, VARIANT FAR* pvarResult,
					  EXCEPINFO FAR* pexcepinfo,UINT FAR* puArgErr)
{
	
	HRESULT retVal = DISP_E_MEMBERNOTFOUND;
	TCHAR *tcURL = new TCHAR[MAX_URL];
	memset(tcURL, NULL, sizeof(TCHAR) * MAX_URL);

	switch (dispidMember) 
	{
		case DISPID_NAVIGATEERROR:
		//Fires when an error occurs during navigation.
		/*
		This event fires before Windows Internet Explorer displays an error page due to 
		an error in navigation. An application has a chance to stop the display of the 
		error page by setting the Cancel parameter to True. However, if the server contacted 
		in the original navigation supplies its own substitute page navigation, 
		when you set Cancel to True, it has no effect, and the navigation to the server's 
		alternate page proceeds. For example, assume that a navigation 
		to http://www.www.wingtiptoys.com/BigSale.htm causes this event to fire because the 
		page does not exist. However, the server is set to redirect the navigation 
		to http://www.www.wingtiptoys.com/home.htm. In this case, when you set Cancel to True, 
		it has no effect, and navigation proceeds to http://www.www.wingtiptoys.com/home.htm. 
		*/

			SetEvent(m_hNavigated);
			CloseHandle(m_hNavigated);
			m_hNavigated = NULL;

			//Validate that there is an event handler
			if(m_EngineEvents[EEID_NAVIGATIONERROR])
			{
				//get the URL which failed
				if (pdparams && pdparams->rgvarg[0].vt == VT_BSTR)
					wcsncpy(tcURL, pdparams->rgvarg[3].pvarVal->bstrVal, MAX_URL-1);
				
				if(m_EngineEvents[EEID_NAVIGATIONERROR](EEID_NAVIGATIONERROR, (LPARAM)tcURL, m_tabID) == S_OK)
					*(pdparams->rgvarg[0].pboolVal) = VARIANT_TRUE;
				else
					*(pdparams->rgvarg[0].pboolVal) = VARIANT_FALSE;
				
				retVal = S_OK;
			}
			break;
		
		case DISPID_STATUSTEXTCHANGE:
		//Fires when the status bar text of the object has changed.

			//Validate that there is an event handler
			if(m_EngineEvents[EEID_STATUSTEXTCHANGE])
			{
				//get the URL which failed
				if (pdparams && pdparams->rgvarg[0].vt == VT_BSTR)
				{
					if(pdparams->rgvarg[0].bstrVal)
						wcsncpy(tcURL, pdparams->rgvarg[0].bstrVal, MAX_URL-1);
				}
				m_EngineEvents[EEID_STATUSTEXTCHANGE](EEID_STATUSTEXTCHANGE, (LPARAM)tcURL, m_tabID);
				
				retVal = S_OK;
			}
			break;

		case DISPID_PROGRESSCHANGE:
		//Fires when the progress of a download operation is updated on the object. 

		/*
		The container can use the information provided by this event 
		to display the number of bytes downloaded or to update a progress indicator. 
		To calculate the percent of progress to show in a progress indicator, 
		multiply the value of nProgress by 100, and divide by the value of 
		nProgressMax; unless nProgress is -1, in which case the container indicates 
		that the operation is finished or hides the progress indicator.
		*/

			//Validate that there is an event handler
			if(m_EngineEvents[EEID_PROGRESSCHANGE])
			{
				if (pdparams && pdparams->rgvarg[0].vt == VT_I4 && pdparams && pdparams->rgvarg[1].vt == VT_I4) 
				{
					LPARAM lParam = MAKELPARAM(
									pdparams->rgvarg[1].intVal,		//A Long that specifies the amount of total progress to show, or -1 when progress is complete.
									pdparams->rgvarg[0].intVal);	//A Long that specifies the maximum progress value.

					m_EngineEvents[EEID_PROGRESSCHANGE](EEID_PROGRESSCHANGE, lParam, m_tabID);
				}
				
				retVal = S_OK;
			}
			break;
		case DISPID_NAVIGATECOMPLETE2:
			
		//Fires after a navigation to a link is completed on a window element or a frameSet element. 

		/*
		The document might still be downloading (and in the case of HTML, images 
		might still be downloading), but at least part of the document has 
		been received from the server, and the viewer for the document has been created.
		*/

			SetEvent(m_hNavigated);
			CloseHandle(m_hNavigated);
			m_hNavigated = NULL;

			//Validate that there is an event handler
			if(m_EngineEvents[EEID_NAVIGATECOMPLETE])
			{
				if (pdparams && pdparams->rgvarg[0].vt == VT_BSTR) 
				{
					if(pdparams->rgvarg[0].bstrVal)
						wcsncpy(tcURL, pdparams->rgvarg[0].bstrVal, MAX_URL-1);
				}
				else if (pdparams && pdparams->rgvarg[0].vt == (VT_VARIANT|VT_BYREF)) 
				{
					if(pdparams->rgvarg[0].pvarVal->vt == VT_BSTR && pdparams->rgvarg[0].pvarVal->bstrVal)
						wcsncpy(tcURL, pdparams->rgvarg[0].pvarVal->bstrVal, MAX_URL-1);
				}

				m_EngineEvents[EEID_NAVIGATECOMPLETE](EEID_NAVIGATECOMPLETE, (LPARAM)tcURL, m_tabID);
				
				retVal = S_OK;
			}
			
			//  Add the URL to the History stack if we have an associated History
			if (m_BrowserHistory != NULL)
			{
				m_BrowserHistory->Add(tcURL);
			}
			break;
		case DISPID_TITLECHANGE:
		//Fires when the title of a document in the object becomes available or changes.
		/*
		Because the title might change while an HTML page is downloading, the URL of 
		the document is set as the title. If the HTML page specifies a title, it is parsed, 
		and the title is changed to reflect the actual title.
		*/

			if (pdparams && pdparams->rgvarg[0].vt == VT_BSTR) 
				wcsncpy(tcURL, pdparams->rgvarg[0].bstrVal, MAX_URL-1);
				wcscpy(m_tcCurrentPageTitle, tcURL);

			//Validate that there is an event handler
			if(m_EngineEvents[EEID_TITLECHANGE])
			{
				m_EngineEvents[EEID_TITLECHANGE](EEID_TITLECHANGE, (LPARAM)tcURL, m_tabID);
				retVal = S_OK;
			}
			break;

		case DISPID_DOCUMENTCOMPLETE:
		//Fires when a document is completely loaded and initialized
		/*
		The value of the URL parameter might not match the URL that was 
		originally given to the WebBrowser Control, because the URL might 
		be converted to a qualified form. For example, if an application 
		specified a URL of www.microsoft.com in a call to the Navigate method 
		or the Navigate2 method, then the URL passed into DocumentComplete 
		is http://www.microsoft.com/. If the server has redirected the browser 
		to a different URL, the redirected URL is passed into the URL parameter.
		*/
			
			//Validate that there is an event handler
			if(m_EngineEvents[EEID_DOCUMENTCOMPLETE])
			{
				if (pdparams && pdparams->rgvarg[0].vt == VT_BSTR) 
				{
					if(pdparams->rgvarg[0].bstrVal)
						wcsncpy(tcURL, pdparams->rgvarg[0].bstrVal, MAX_URL-1);
				}
				else if (pdparams && pdparams->rgvarg[0].vt == (VT_VARIANT|VT_BYREF)) 
				{
					if(pdparams->rgvarg[0].pvarVal->vt == VT_BSTR && pdparams->rgvarg[0].pvarVal->bstrVal)
						wcsncpy(tcURL, pdparams->rgvarg[0].pvarVal->bstrVal, MAX_URL-1);
				}

				m_EngineEvents[EEID_DOCUMENTCOMPLETE](EEID_DOCUMENTCOMPLETE, (LPARAM)tcURL, m_tabID);
				
				retVal = S_OK;
			}
			ParseTags();

#ifdef SCROLL_NOTIFY
			// Create a scroll notifier to watch for changes in the size and scroll position of this page
			// Delete any existing one first
			if (pScrollNotify)
				delete pScrollNotify;

			pScrollNotify = new CScrollNotify (m_pBrowser, m_hwndApp);
#endif
			//  This line allows iFrames to render correctly when running in
			//  non-compatibility mode, but as we have removed this mode this
			//  line can be taken away as it was never officially part of the build.
			//  (DCC)
			//  AdjustCurrentScroll(SCROLLBAR_VERTICAL, 0);
			break;

		
		case DISPID_BEFORENAVIGATE2:
		//Fires before navigation occurs in the given object (on either a window element or a frameset element).
		/*
		returning S_FALSE from the event handler will cause the navigation to be cancelled

		The parameters for this DISPID are as follows:
		[0]: Cancel flag               - VT_BYREF|VT_BOOL
		[1]: HTTP headers              - VT_BYREF|VT_VARIANT
		[2]: Address of HTTP POST data - VT_BYREF|VT_VARIANT
		[3]: Target frame name         - VT_BYREF|VT_VARIANT
		[4]: Option flags              - VT_BYREF|VT_VARIANT
		[5]: URL to navigate to        - VT_BYREF|VT_VARIANT
		[6]: An object that evaluates to the top-level or frame
		WebBrowser object corresponding to the event.
		[6]: type = 9 VT_DISPATCH

		*/



			//  Check we're not trying to navigate back to the Load Page for
			//  a second time
			if (pdparams && pdparams->rgvarg[5].pvarVal[0].vt == VT_BSTR)
			{
				if(pdparams->rgvarg[5].pvarVal[0].bstrVal)
				{
					if (StrContains(pdparams->rgvarg[5].pvarVal[0].bstrVal, 
						L"PocketBrowser\\HTML\\LoadPage.html"))
					{
						iLoadPageCount++;
						if (iLoadPageCount > 1)
						{
							Stop();
							return S_FALSE;
						}
					}
				}
			}

			if (pdparams && pdparams->rgvarg[5].pvarVal[0].vt == VT_BSTR) 
			{
				if(pdparams->rgvarg[5].pvarVal[0].bstrVal)
					wcsncpy(tcURL, pdparams->rgvarg[5].pvarVal[0].bstrVal, MAX_URL-1);
			}

			if(memcmp(tcURL, L"res://", 12) == 0)
			{
				*(pdparams->rgvarg[0].pboolVal) = VARIANT_TRUE;
				retVal = S_OK;
				break;
			}

			SetEvent(m_hNavigated);
			CloseHandle(m_hNavigated);
			m_hNavigated = NULL;

			//Validate that there is an event handler
			if(m_EngineEvents[EEID_BEFORENAVIGATE])
			{
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
					&CIEEngineTab::NavigationTimeoutThread, (LPVOID)this, 0, NULL));
				wcscpy(m_tcNavigatedURL, tcURL);


#ifdef SCROLL_NOTIFY
			// Stop any checking for scroll changes during navigation
			if (pScrollNotify)
			{
				delete pScrollNotify;
				pScrollNotify = NULL;
			}
#endif
				if(m_EngineEvents[EEID_BEFORENAVIGATE](EEID_BEFORENAVIGATE, (LPARAM)tcURL, m_tabID) == S_FALSE)
					*(pdparams->rgvarg[0].pboolVal) = VARIANT_TRUE;
				else
				{
					*(pdparams->rgvarg[0].pboolVal) = VARIANT_FALSE;
				}
				
				retVal = S_OK;
			}
			break;
	}

	delete[] tcURL;
	tcURL = NULL;

	return retVal;

}

/**
* \author	James Morley-Smith (JMS, JNP837)
* \date		June 2009 (First Created, JMS)
*/
HRESULT CIEEngineTab::OnInPlaceActivate(void)
{
	m_bInPlaceActive = TRUE;
	return S_OK;
}

/**
* \author	James Morley-Smith (JMS, JNP837)
* \date		June 2009 (First Created, JMS)
*/
HRESULT CIEEngineTab::GetWindowContext( LPOLEINPLACEFRAME FAR * lplpFrame, LPOLEINPLACEUIWINDOW FAR *  lplpDoc, LPRECT lprcPosRect, LPRECT lprcClipRect, LPOLEINPLACEFRAMEINFO lpFrameInfo)
{
	int nHeight = 0;
	GetClientRect(m_hwndTabHTML, lprcPosRect);
	GetClientRect(m_hwndTabHTML, lprcClipRect);

	RECT rc = {0,0,0,0};

	return S_OK;
}

/**
* \author	James Morley-Smith (JMS, JNP837)
* \date		June 2009 (First Created, JMS)
*/
HRESULT CIEEngineTab::OnInPlaceDeactivate(void)
{
	m_bInPlaceActive = FALSE;
	return S_OK;
}

/**
* \author	Darryn Campbell (DCC, JRQ768)
* \date		February 2010 (First Created, DCC)
*/
HRESULT CIEEngineTab::ShowMessage(HWND hwnd, 
						LPOLESTR lpstrText, 
						LPOLESTR lpstrCaption, 
						DWORD dwType,
						LPOLESTR lpstrHelpFile, 
						DWORD dwHelpContext, 
						LRESULT *plResult)
{
	//  If we have a current page title use it, else just default to 
	//  "PocketBrowser"
	if (wcsncmp(m_tcCurrentPageTitle, L"\\", 1) == 0 ||
		wcsncmp(m_tcCurrentPageTitle, L"file://", 7) == 0)
		lpstrCaption = L"PocketBrowser";
	else
		lpstrCaption = m_tcCurrentPageTitle;

    // Create your own message box and display it
    *plResult = MessageBox(hwnd, lpstrText, lpstrCaption, dwType);
	return S_OK;
}

