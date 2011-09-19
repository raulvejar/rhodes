#include "stdafx.h"

#include "RhoWKBrowserEngine.h"
#include "webkit/PBCore/Eng.h"
#include "MainWindow.h"

namespace rho
{
WNDPROC CRhoWKBrowserEngine::m_WebKitOwnerProc;

CRhoWKBrowserEngine::CRhoWKBrowserEngine(HWND hParentWnd, HINSTANCE hInstance) : m_wkengine(NULL)
{
    m_wkengine = new CWebKitEngine(hParentWnd, hInstance);
    if(m_wkengine->Init(L"PBEngine_WK.dll")) 
    {
        m_wkengine->InitEngine(0, &WK_HTMLWndProc, &m_WebKitOwnerProc, SETTING_OFF, &WK_GetEngineConfig);
    }
}

CRhoWKBrowserEngine::~CRhoWKBrowserEngine(void)
{
    m_wkengine->DeInitEngine();
    //TODO: delete engine - now it crash when delete
    //delete m_wkengine;
}

BOOL CRhoWKBrowserEngine::Navigate(LPCTSTR szURL)
{
    return m_wkengine->Navigate(szURL);
}

HWND CRhoWKBrowserEngine::GetHTMLWND()
{
    return m_wkengine->GetHTMLWND();
}

BOOL CRhoWKBrowserEngine::ResizeOnTab(int iInstID,RECT rcNewSize)
{
    return m_wkengine->ResizeOnTab(iInstID,rcNewSize);
}

BOOL CRhoWKBrowserEngine::BackOnTab(int iInstID,int iPagesBack/* = 1*/)
{
    return m_wkengine->BackOnTab(iInstID,iPagesBack);
}

BOOL CRhoWKBrowserEngine::ForwardOnTab(int iInstID)
{
    return m_wkengine->ForwardOnTab(iInstID);
}

BOOL CRhoWKBrowserEngine::Reload(bool bFromCache)
{
    return m_wkengine->Reload(bFromCache);
}

BOOL CRhoWKBrowserEngine::NavigateToHtml(LPCTSTR szHtml)
{
    //return m_wkengine->NavigateToHtml(szHtml);
    //TODO: NavigateToHtml

    return FALSE;
}

LRESULT CALLBACK CRhoWKBrowserEngine::WK_HTMLWndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	LRESULT result;
	// Pass message to original window procedure
	result = CallWindowProc (m_WebKitOwnerProc, hwnd, message, wParam, lParam);
	return result;

    //return DefWindowProc(hwnd, message, wParam, lParam);
    //return (message == 0xF) ? 1 : 0;
}

LRESULT CALLBACK CRhoWKBrowserEngine::WK_GetEngineConfig(int iInstID, LPCTSTR tcSetting, TCHAR* tcValue)
{
    //LPCTSTR tcValueRead;
    //tcValueRead = g_pConfig->GetAppSettingPtr(iInstID, tcSetting, L"Value");
    //if (tcValueRead != NULL)
    //  wcscpy(tcValue, tcValueRead);
    //else
    tcValue = NULL;
    return S_OK;
} 

LRESULT CRhoWKBrowserEngine::OnWebKitMessages(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	int iTabID;
	LPTSTR pStr;

    bHandled = TRUE;

    switch (uMsg) 
    {
		case PB_DEFTAGS:
/*			iTabID = (int)wParam;
			g_pTargetApp = g_pAppManager->GetApp(iTabID);
			if(g_pTargetApp){
				g_pTargetApp->RunDefaultMetaTags();
			}*/
			break;
		
		case PB_ONMETA:
/*			iTabID = (int)wParam;
			g_pTargetApp = g_pAppManager->GetApp(iTabID);
			if(g_pTargetApp){
				metaTag = (EngineMETATag*)lParam;
				g_pTargetApp->m_pbStruct.bInvoked = FALSE;
				g_pTargetApp->m_pEMML->ParseEMMLTag(metaTag->tcHTTPEquiv,metaTag->tcContents,TagProc);
				return S_OK; 
			}
			return S_FALSE; 
*/			

		case PB_NAVIGATETAB:
			
			pStr = (LPTSTR)lParam;
			iTabID = (int)wParam;
			
			//  Remove spaces after 'JavaScript:'
			if (_memicmp(pStr, L"JavaScript: ", 12 * sizeof(TCHAR)) == 0 ||
				_memicmp(pStr, L"JavaScript:\t", 12 * sizeof(TCHAR)) == 0)
			{
				//  There are spaces to remove
				int iJSLen = _tcslen(L"JavaScript:");
				while(*(pStr + iJSLen) == L' ' || *(pStr + iJSLen) == L'\t')
				{
					//  +1 for space, +1 for NULL terminator
					wmemmove(pStr + iJSLen, pStr + iJSLen + 1, wcslen(pStr + iJSLen + 1) + 1);
				}
			}

			if(_memicmp(pStr,L"JavaScript:",11*2) == 0){
				if(m_wkengine->JavaScript_Exist(iTabID,pStr)){
					return m_wkengine->JavaScriptInvoke(iTabID,pStr);
				}
			}
			else{
                   return m_wkengine->NavigateOnTab(pStr,iTabID);
			}
		
			break;
		
		case PB_GEN_ACTIVEX:
			// GD - don't attempt to call plugins while shutting down
/*			if (g_bShuttingDown)
				break;

			pEquiv = (LPWSTR) wParam;
			pContent = (LPWSTR)lParam;
			
			pApp = g_pAppManager->GetApp(0);
			if(pApp){
				pApp->m_pbStruct.bInvoked = TRUE;
				pApp->m_pEMML->ParseEMMLTag(pEquiv,pContent,TagProc);
			}*/
			
			break;
		case PB_GEN_QUIT://currently quits all instances of PB
			//DeInitialise();
			break;
		case PB_SETPLG_PROPERTY:
			
			break;
		case PB_CALLPLG_METHOD:

			break;
		case PB_ONTOPMOSTWINDOW:
        {
			//LRESULT rtRes = g_pAppManager->ProcessOnTopMostWnd((HWND)lParam,(int)wParam); 
			//if(rtRes == S_FALSE){
			//	PostMessage(PB_GEN_QUIT,0,0);//we have not successfully processed the topMostWindow so shutdown
			//}
			//return rtRes;
            break;
        }		
		case PB_WINDOW_RESTORE:
			//  The window has been restored 
			//BrowserRestore(0, NULL);
	        ShowWindow(m_wkengine->GetHTMLWND(), SW_RESTORE);
	        //  Hide the Start Bar
	        //PBScreenMode(g_bFullScreen, FALSE);
	        SetForegroundWindow(m_wkengine->GetHTMLWND());
	        EnableWindow(m_wkengine->GetHTMLWND(), TRUE);

			break;
		case PB_SCREEN_ORIENTATION_CHANGED:
			//g_bLeftRotated = wParam == TYPEHERE?TRUE:FALSE;//update the global
			//use a thread so that we don't hold up the message pump
			//CloseHandle(CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE) WindowChangedThread,(LPVOID) lParam, 0, 0));
			break;
		
		case PB_NEWGPSDATA:
			//Notifies the web engine when there are new location data available
			//g_pEngine->SendLocationDatoToEngine();
			break;
    }

    return 0;
}

void CRhoWKBrowserEngine::RunMessageLoop(CMainWindow& mainWnd)
{
	tPB_PreprocessMessage lpPreprocessMessage = m_wkengine->GetlpPreprocessMessage();

	LRESULT preProcRes;
    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0))
    {
	    preProcRes = ACCELERATE_KEY_HANDLE;
//		    if (!g_bShuttingDown)
		    preProcRes = lpPreprocessMessage(msg);
//		    else
//			    continue;
	    //  Allow the browser to process the accelerator keys (most notably Tab)
	    //  Do not preProcess the message if we are shutting down as the 
	    //  Engine DLL will have been deinitialised
	    if (//CLicense::GetLicenseScreenVisible() || 
		    ACCELERATE_KEY_DONT_HANDLE != preProcRes)
	    {
		    if (preProcRes != ACCELERATE_KEY_DONT_TRANSLATE //|| 
			    //CLicense::GetLicenseScreenVisible()
                )
		    {
			    //if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg)) 
                if (!mainWnd.TranslateAccelerator(&msg))
			    {
				    TranslateMessage(&msg);
				    DispatchMessage(&msg);
			    }
		    }
		    else
		    {
			    DispatchMessage(&msg);
		    }
	    }

    }
}

}