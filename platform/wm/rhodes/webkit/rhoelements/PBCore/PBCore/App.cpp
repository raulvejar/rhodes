#include "StdAfx.h"
#include "App.h"
#include "Resource.h"
#include "Config.h"
#include "Message.h"


#define MENU_ID 5000

extern BOOL		Log			(LogTypeInterface logSeverity,LPCTSTR pLogComment, 
						LPCTSTR pFunctionName, DWORD dwLineNumber,LPCTSTR pCallingModule);

extern LRESULT			onMeta		(EngineEventID eeID, LPARAM value, int tabIndex);
extern BOOL				PageNavigate(int iInstID,LPCTSTR pURI);
extern LPCTSTR			g_pRegExPath;
extern CWebKitEngine    *g_pEngine;
extern PPBCORESTRUCT	g_pPBCore;
extern CConfig			*g_pConfig;
extern LRESULT			CALLBACK TagProc( LPPBNAMEVALUE pNameVal);
extern CApp				*g_pTargetApp;
extern BOOL				PreloadModule(PPBSTRUCT pPBStructure,LPCTSTR pTargetMod);



CApp::CApp()
{
	
	m_pEMML					= NULL;
	m_pHomePage				= NULL;
	m_pbStruct.hWnd			= NULL;
	m_pAppName				= NULL;
	m_bScrollBarsEnabled	= SETTING_OFF;
	m_bBadlink				= FALSE;
	m_dwNavTimeoutMs		= -1;
	memset(&m_szBadLinkURI,0,MAX_URL*sizeof(TCHAR));

	// GD
	// Initialse current URL to empty string
	m_szCurrentURL [0] = L'\0';
}

CApp::~CApp(void)
{
	delete [] m_pAppName;
	delete [] m_pHomePage;
	delete m_pEMML;
}

BOOL CApp::Init(HINSTANCE hInstance,int iInstID,LPCTSTR szAppName,LPCTSTR szStartPage)
{
	
	if(m_pAppName){
		return TRUE;//already initialised
	}
	
	int iLenAppName,iLenHomePage;
	iLenAppName		= wcslen(szAppName);
	iLenHomePage	= wcslen(szStartPage); 
	
	//set the application name
	m_pAppName = new WCHAR[iLenAppName + 1];
	if(!m_pAppName){
		Log(PB_LOG_ERROR,PB_MEM_ERR_STR,L"CApp::Init",__LINE__,L"Core");
		return FALSE;
	}
	wcsncpy(m_pAppName,szAppName,iLenAppName);
	m_pAppName[iLenAppName] = NULL;
	
	//set the home page
	m_pHomePage = new WCHAR[iLenHomePage + 1];
	if(!m_pHomePage){
		Log(PB_LOG_ERROR,PB_MEM_ERR_STR,L"CApp::Init",__LINE__,L"Core");
		return FALSE;
	}
	wcsncpy(m_pHomePage,szStartPage,iLenHomePage);
	m_pHomePage[iLenHomePage] =NULL;


	//create the EMML parser object
	m_pEMML = new CEMML();
	if(!m_pEMML){
		Log(PB_LOG_ERROR,PB_MEM_ERR_STR,L"CApp::Init",__LINE__,L"Core");
		return FALSE;
	}
	
	if(!LoadRegExFile()){
		Log(PB_LOG_ERROR,L"Could not load the RegEX file",L"CApp::Init",__LINE__,L"Core");
		return FALSE;	
	}
	LPCTSTR pScroll;
	pScroll = g_pConfig->GetAppSettingPtr(iInstID,L"ScrollBars\\ScrollBarsEnabled",L"Value");	
	if(pScroll){
		m_bScrollBarsEnabled = *pScroll==L'0'?SETTING_OFF:SETTING_ON;
	}
	m_pbStruct.iTabID		= iInstID;
	m_pbStruct.hInstance	= hInstance;
	//m_bScrollBarsEnabled	= bScrollBarsEnabled;
	return TRUE;
}

BOOL CApp::DoPreloads()
{
	int iPreLoadIndex;
	LPCTSTR pPreload;
	BOOL bRet;

	//do the preloads
	for(iPreLoadIndex = 0;;iPreLoadIndex++)
	{
		pPreload = g_pConfig->GetPreLoadPtr(m_pbStruct.iTabID,iPreLoadIndex);
		if(!pPreload){
			return TRUE;
		}
		m_pbStruct.bInvoked = FALSE;
		bRet = PreloadModule(&m_pbStruct,pPreload);
		if(!bRet){
			return FALSE;
		}
	}
	
		
}


BOOL CApp::LoadRegExFile()
{
	if(!m_pEMML){
		return FALSE;
	}
	
	CXMLReader* XML = new CXMLReader();
	//Tag refrences returned by the XMLREADER class
	XML_TAG xRoot,xEquivs,xContents,xExpression;
	
	XML_ATT xMatchingPattern,xReplaceString;

	int iLoop;
	LPCTSTR pRegEx = g_pRegExPath;
	if(!_memicmp(pRegEx,L"FILE://",14)) pRegEx+=7;
	if(!XML->ParseFile(pRegEx)==SUCCESS){
		return FALSE;
	}
	
	xRoot = XML->GetElement(L"RegularEx",0);
	//load all of the equiv regular expressions
	if(!xRoot){
		return FALSE;
	}
	xEquivs = XML->GetChildElement(xRoot,L"Equivs",0);
	if(xEquivs){
		
		for(iLoop = 0;;iLoop++)
		{
			xExpression = XML->GetChildElement(xEquivs,L"Expression",iLoop);
			if(!xExpression){ 
				break;//no more Equiv expressions
			}
			
			
			xMatchingPattern	= XML->GetAttrByName(xExpression,L"PatternEx");
			xReplaceString		= XML->GetAttrByName(xExpression,L"replaceEX");
						
			if(!xMatchingPattern || !xReplaceString){
				Log(PB_LOG_ERROR,L"Illegal XML tag",L"CApp::LoadRegExFile",__LINE__,L"Core");
				return FALSE;
			}
			if(!m_pEMML->AddRegExPair_HE(xMatchingPattern->lpValue,xReplaceString->lpValue)){
				Log(PB_LOG_ERROR,L"Could not add regEx to EMML",L"CApp::LoadRegExFile",__LINE__,L"Core");
				return FALSE;
			}
			//Log(PB_LOG_INFO,xMatchingPattern->lpValue,xReplaceString->lpValue,__LINE__,L"LoadRegExFile");
		}
	}
	
	xContents = XML->GetChildElement(xRoot,L"Contents",0);
	//load all of the content regular expressions
	if(xContents){
	
		for(iLoop = 0;;iLoop++)
		{
			xExpression = XML->GetChildElement(xContents,L"Expression",iLoop);
			if(!xExpression){ 
				break;//no more Equiv expressions
			}
			
			
			xMatchingPattern	= XML->GetAttrByName(xExpression,L"PatternEx");
			xReplaceString		= XML->GetAttrByName(xExpression,L"replaceEX"); 

			if(!xMatchingPattern || !xReplaceString){
				Log(PB_LOG_ERROR,L"Illegal XML tag",L"CApp::LoadRegExFile",__LINE__,L"Core");
				return FALSE;
			}
			if(!m_pEMML->AddRegExPair_CT(xMatchingPattern->lpValue,xReplaceString->lpValue)){
				Log(PB_LOG_ERROR,L"Could not add regEx to EMML",L"CApp::LoadRegExFile",__LINE__,L"Core");
				return FALSE;
			}
			//Log(PB_LOG_INFO,xMatchingPattern->lpValue,xReplaceString->lpValue,__LINE__,L"LoadRegExFile");
		}
	}
	delete XML;
	return TRUE;
}

BOOL CApp::Home()
{
	return PageNavigate(m_pbStruct.iTabID,m_pHomePage);
}


PPBSTRUCT CApp::GetAppPBStruct()
{
	return &m_pbStruct;
}

BOOL CApp::CallPlugMethod(LPCTSTR pTargetMod,LPCTSTR pMethod)
{
	if(g_pPBCore->pCallPlgMethod){
		return g_pPBCore->pCallPlgMethod(&m_pbStruct,pTargetMod,pMethod,m_pAppName);
	}
	return FALSE;
}

BOOL CApp::SetPlugProperty(LPCTSTR pTargetMod,LPCTSTR pParam,LPCTSTR pValue)
{
	if(g_pPBCore->pSetPlugProp){
		return g_pPBCore->pSetPlugProp(&m_pbStruct,pTargetMod,pParam,pValue,m_pAppName);
	}
	return FALSE;
}



BOOL CApp::Activate(BOOL bShow)///< places the window created in this object on the top
{
	if(g_pEngine){
		g_pEngine->SwitchTabs(m_pbStruct.iTabID);
		//before loading the script make sure that all plugins have processed the appfocus event
		//access the global table
		//if this slows the loading of the page then the plugin needs to be addressed
		/*
		if(m_bFirstLoad){
			m_bFirstLoad = FALSE;
			return PageNavigate(m_pbStruct.iTabID,m_pHomePage);
		}
		*/
	}
	return FALSE;
	
}



void CApp::DeActivate()
{
	//SetEvent(m_pGlobalStruct->pEventStructure->PBAppLostFocusEvent);
	g_pPBCore->pKillApplFunc(&m_pbStruct);
}



void CApp::RunDefaultMetaTags()
{
	LPCTSTR pTagStr,pTagStart;
	int iLoop;
	WCHAR szModName[MAXMODLEN + 1];
	WCHAR szValue[MAXLEN_ATTRIBUTE_VALUE + 1];
	
	for(iLoop = 0;;iLoop++)
	{
		pTagStr = g_pConfig->GetDefaultMetaTagsPtr(m_pbStruct.iTabID,iLoop);
		if(!pTagStr){
			break;
		}
		//get the module name and value
		int iIndex = 0;
		
		for(pTagStart = pTagStr;;pTagStr++)
		{
			if(*pTagStr == L' ') continue;
			if(*pTagStr == L'~'){
				if(!iIndex){
					//Log warning invalid tag
					return;
				}
				wcsncpy(szModName,pTagStart,iIndex);
				szModName[iIndex] = NULL;
				pTagStr++;
				wcscpy(szValue,pTagStr);
				m_pEMML->ParseEMMLTag(szModName,szValue,TagProc);
				break;
			}
			iIndex++;
		}

		
	}

}

/**
* \author	Darryn Campbell (DCC, JRQ768)
* \date		February (DCC: Initial Creation)
*/
BOOL CApp::ShowDebugButtons()
{
	//  Debug buttons are shown for each application.
	BOOL retVal = FALSE;
	//  Read the value of DebugButtons from the Configuration File
	LPCTSTR tcDebugButtons;
	tcDebugButtons = 
		g_pConfig->GetSettingPtr(L"Configuration\\DebugButtons\\DebugButtonsEnabled",L"Value");
	if (tcDebugButtons)
	{	
		if (*tcDebugButtons != L'0')
		{
			g_pTargetApp = this;
			//  Show the Debug Buttons
			//  Debugging with Motorola Webkit browser attempts to load each button library
			//  individually which takes a long time, load the library before the 
			//  debug buttons and unload it afterwards.
			HMODULE hImageLibrary = NULL;
			hImageLibrary = LoadLibrary(L"imaging.dll");

			SetPlugProperty(L"BackButton",L"Visibility",L"Visible");
			SetPlugProperty(L"GoButton",L"Visibility",L"Visible");
			SetPlugProperty(L"StopButton",L"Visibility",L"Visible");
			SetPlugProperty(L"ForwardButton",L"Visibility",L"Visible");
			SetPlugProperty(L"AddressBar",L"Visibility",L"Visible");
			SetPlugProperty(L"ReloadButton",L"Visibility",L"Visible");
			SetPlugProperty(L"QuitButton",L"Visibility",L"Visible");
			SetPlugProperty(L"ZoomTextButton",L"Visibility",L"Visible");
			SetPlugProperty(L"SIPButton",L"Visibility",L"Visible");

			if (hImageLibrary)
				FreeLibrary (hImageLibrary);
			hImageLibrary = NULL;

		}
	}
	return TRUE;
}

/**
* \author	Geoff Day (GRD, XFH386)
* \date		April 2010 (GRD: Initial Creation)
*/
BOOL CApp::ShowScrollBars()
{
	BOOL retVal = FALSE;
	LPCTSTR tcScrollBars;
	tcScrollBars = 
		g_pConfig->GetSettingPtr(L"Configuration\\Applications\\Application\\ScrollBars\\ScrollBarsEnabled",L"Value");
	if (tcScrollBars)
	{	
		if (*tcScrollBars != L'0')
		{
			g_pTargetApp = this;
			SetPlugProperty(L"Scroll",L"Visibility",L"Visible");
		}
	}
	return TRUE;
}

BOOL CApp::LoadBadlink()
{
	LPCTSTR pVal;
	//get the url if it is not NULL
	pVal = g_pConfig->GetAppSettingPtr(m_pbStruct.iTabID,L"Navigation\\BadLinkURI",L"Value");
	if(pVal){
		int iLen = _tcslen(pVal);
		if(iLen==0) return TRUE;//not an error badlink not set in config
		if(iLen > 5 && iLen<= MAX_URL){
			_tcscpy(m_szBadLinkURI,pVal);
			pVal = g_pConfig->GetSettingPtr(L"Configuration\\Navigation\\NavTimeout",L"Value");
			if(pVal){
				m_dwNavTimeoutMs = _ttol(pVal);
				if(m_dwNavTimeoutMs >= 0&& m_dwNavTimeoutMs < 45001){
					m_bBadlink = TRUE;
					return TRUE;
				}
			}
		}
	}
	return FALSE;
}

LPCTSTR CApp::GetBadlink(LPCTSTR pCurrURL, bool bNavigationStopped)
{
	memset(m_szBadLinkPlusURL, 0, sizeof(TCHAR)*MAX_URL);
	int iStoppedLen = wcslen(L"&Stopped=false");
	if(m_szBadLinkURI[0])
	{
		_tcscpy(m_szBadLinkPlusURL, m_szBadLinkURI);
		if((_tcslen(pCurrURL) + _tcslen(m_szBadLinkURI) + 10 + iStoppedLen) > MAX_URL)
		{
			//URL too long to display, see if there's enough space to set a couple of flags
			if((_tcslen(pCurrURL) + 10 + iStoppedLen) <= MAX_URL){
				//display just the badlink=1 var
				_tcscat(m_szBadLinkPlusURL, L"?BadLink=1");
				if (bNavigationStopped)
					_tcscat(m_szBadLinkPlusURL, L"&Stopped=true");
				else
					_tcscat(m_szBadLinkPlusURL, L"&Stopped=false");
			}
		}
		else
		{
			//  URL is not too long to display
			_tcscat(m_szBadLinkPlusURL,L"?BadLink=");
			_tcscat(m_szBadLinkPlusURL,pCurrURL);
			if (bNavigationStopped)
				_tcscat(m_szBadLinkPlusURL, L"&Stopped=true");
			else
				_tcscat(m_szBadLinkPlusURL, L"&Stopped=false");
		}
	}
	return m_szBadLinkPlusURL;
}











