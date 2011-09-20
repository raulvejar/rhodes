// pbengine.cpp : Defines the entry point for the DLL application.
//

#include "IEEngine.h"

//  An IE Engine can either have IEEngineTabs (CE) or IEMobileEngineTabs (WM)
#ifdef PB_ENGINE_IE_CE
	#include "../IEEngineCE/IEEngineTab.h"
	#define PB_ENGINE_TAB_FLAVOUR CIEEngineTab
#elif defined (PB_ENGINE_IE_MOBILE)
	#include "../IEEngineMobile/IEMobileEngineTab.h"
	#define PB_ENGINE_TAB_FLAVOUR CIEMobileEngineTab
#elif defined (PB_ENGINE_IE_WM65)
	#include "../IEEngineWM65AKU/IEEngineWM65AKUTab.h"
	#define PB_ENGINE_TAB_FLAVOUR CIEWM65EngineTab
#endif

//  Initialise Static Variables
IETab* CIEEngine::m_ieTabList = NULL;

#include <windows.h>
#include <commctrl.h>

//////////////////////////////////////////////
//											//
//		Setup (Public)						//
//											//
//////////////////////////////////////////////

/**
* \author	Darryn Campbell (DCC, JRQ768)
* \date		October 2009 (DCC: Initial Creation)
* \date		November 2009 (DCC: Added Subclassing of HTML Message Window)
* \date		February 2010 (DCC: Added configuration function so engine could read the XML config)
*/
LRESULT CIEEngine::InitEngine(HINSTANCE hInstance, HWND hwndParent, int iTabID, 
							  LPCTSTR tcIconURL, BoolSettingValue bsvScrollbars,
							  HTMLWndPROC_T hWndProc, WNDPROC* ownerProc, 
							  ReadEngineConfigParameter_T configFunction)
{
	//  These parent variables are used each time we create a Tab
	m_hparentInst	= hInstance;
	m_parentHWND	= hwndParent;
	m_bDeviceLocked	= FALSE;
	m_ieTabList		= NULL;
	m_tabIDOfTopUserApplication	= 0;
	m_ownerProc		= NULL;

	//  Set up the Attributes of the first Tab
	CIEEngine::m_ieTabList = new IETab();
	m_ieTabList->iTabID = iTabID;
	m_ieTabList->bVisible = TRUE;
	m_ieTabList->nextTab = NULL;
	m_ieTabList->prevTab = NULL;
	m_ieTabList->pEngine = 
		new PB_ENGINE_TAB_FLAVOUR(m_hparentInst, m_parentHWND, iTabID, 
									tcIconURL, bsvScrollbars);
	
	if (m_ieTabList->pEngine == NULL)
		return S_FALSE;
	if (m_ieTabList->pEngine->CreateEngine(configFunction) != S_OK)
		return S_FALSE;

	//  Subclass the HTML window which receives the windows messages.  This is 
	//  done in a separate thread as it will not succeed until the window 
	//  is fully initialised.
	m_ieTabList->pEngine->m_hWndProc = hWndProc;
	m_ieTabList->pEngine->m_ownerProc = ownerProc;
	CloseHandle (CreateThread(NULL, 0, 
			&CIEEngine::RegisterWndProcThread, 
			(LPVOID)m_ieTabList->pEngine, 0, NULL));

	return S_OK;
}

/**
* \author	Darryn Campbell (DCC, JRQ768)
* \date		October 2009
*/
LRESULT CIEEngine::DeinitEngine()
{
	IETab* nextTab = NULL;
	IETab* deletingTab = m_ieTabList;
	while (deletingTab != NULL)
	{
		nextTab = deletingTab->nextTab;
		delete (PB_ENGINE_TAB_FLAVOUR*)deletingTab->pEngine;
		delete deletingTab;
		deletingTab = nextTab;
	}

	return S_OK;
}

/**
* \author	Darryn Campbell (DCC, JRQ768)
* \date		October 2009
*/
LRESULT CIEEngine::RegisterForEvent(EngineEventID eeidEventID, ENGINEEVENTPROC pEventFunc) 
{ 
	//  Loop through all the tabs and call Register for Event on each
	//  one 
	LRESULT returnValue = S_OK;
	IETab* tempTab = m_ieTabList;
	while (tempTab != NULL)
	{
		if (tempTab->pEngine != NULL)
		{
			if (S_OK != tempTab->pEngine->RegisterForEvent(eeidEventID, pEventFunc))
				returnValue = S_FALSE;
		}
		tempTab = tempTab->nextTab;
	}

	return returnValue;
}

/**
* \author	Darryn Campbell (DCC, JRQ768)
* \date		December 2009
*/
LRESULT CIEEngine::PreprocessMessage(MSG msg)
{
	//if the engine has not been initialized, fail
	CIEEngine* pEngine = GetVisibleTabEngine();
	if(!pEngine)	
		return S_FALSE;
	else
		return pEngine->PreprocessMessage(msg);
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
LRESULT CIEEngine::Navigate(LPCTSTR szURL)
{
	//if the engine has not been initialized, fail
	CIEEngine* pEngine = GetVisibleTabEngine();

	if(!pEngine)	
		return S_FALSE;
	else
		return pEngine->Navigate(szURL);
}

/**
* \author	Darryn Campbell (DCC, JRQ768)
* \date		October 2009
*/
LRESULT CIEEngine::NavigateOnTab(LPCTSTR tcAddress, int iTabID)
{
	CIEEngine* pEngine = GetSpecificTabEngine(iTabID);

	if (!pEngine)
		return S_FALSE;
	else
		return pEngine->Navigate(tcAddress);
}

/**
* \author	Darryn Campbell (DCC, JRQ768)
* \date		October 2009
*/
LRESULT CIEEngine::Stop(void)
{
	//if the engine has not been initialized, fail
	CIEEngine* pEngine = GetVisibleTabEngine();
	if(!pEngine)	
		return S_FALSE;
	else
		return pEngine->Stop();
}

/**
* \author	Darryn Campbell (DCC, JRQ768)
* \date		October 2009
*/
LRESULT CIEEngine::StopOnTab (int iTabID)
{
	CIEEngine* pEngine = GetSpecificTabEngine(iTabID);
	if (!pEngine)
		return S_FALSE;
	else
		return pEngine->Stop();
}

/**
* \author	Darryn Campbell (DCC, JRQ768)
* \date		October 2009
*/
LRESULT CIEEngine::Reload(BOOL bFromCache)
{
	//if the engine has not been initialized, fail
	CIEEngine* pEngine = GetVisibleTabEngine();
	if(!pEngine)	
		return S_FALSE;
	else
		return pEngine->Reload(bFromCache);
}

/**
* \author	Darryn Campbell (DCC, JRQ768)
* \date		October 2009
*/
LRESULT CIEEngine::ReloadOnTab (BOOL bFromCache, int iTabID)
{
	CIEEngine* pEngine = GetSpecificTabEngine(iTabID);
	if (!pEngine)
		return S_FALSE;
	else
		return pEngine->Reload(bFromCache);
}

/**
* \author	Darryn Campbell (DCC, JRQ768)
* \date		October 2009
*/
LRESULT CIEEngine::ZoomText(TextZoomValue dwZoomLevel)
{
	//  Find the visible tab and zoom the text on it
	CIEEngine* pEngine = GetVisibleTabEngine();
	if(!pEngine)	
		return S_FALSE;
	else
		return pEngine->ZoomText(dwZoomLevel);
}

/**
* \author	Darryn Campbell (DCC, JRQ768)
* \date		October 2009
*/
LRESULT CIEEngine::ZoomTextOnTab (TextZoomValue dwZoomLevel, int iTabID)
{
	CIEEngine* pEngine = GetSpecificTabEngine(iTabID);
	if (!pEngine)
		return S_FALSE;
	else
		return pEngine->ZoomText(dwZoomLevel);
}

/**
* \author	Darryn Campbell (DCC, JRQ768)
* \date		October 2009
*/
LRESULT CIEEngine::GetZoomTextOnTab (TextZoomValue *dwZoomLevel, int iTabID)
{
	CIEEngine* pEngine = GetSpecificTabEngine(iTabID);
	if (!pEngine)
		return S_FALSE;
	else
		return pEngine->GetZoomText(dwZoomLevel);
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
HWND CIEEngine::GetHTMLHWND() 
{
	//  Return the Visible HTML HWND
	//if the engine has not been initialized, fail
	CIEEngine* pEngine = GetVisibleTabEngine();
	if(!pEngine)	
		return NULL;
	else
		return pEngine->GetHTMLHWND();
}

/**
* \author	Darryn Campbell (DCC, JRQ768)
* \date		October 2009
*/
HWND CIEEngine::GetHTMLHWNDOnTab(int iTabID)
{
	//  Return the specified HTML HWND
	//  If the engine has not been initialised, fail
	CIEEngine* pEngine = GetSpecificTabEngine(iTabID);
	if (!pEngine)	
		return NULL;
	else
		return pEngine->GetHTMLHWND();
}

/**
* \author	Darryn Campbell (DCC, JRQ768)
* \date		October 2009
*/
HWND CIEEngine::GetParentHWND() 
{
	return m_parentHWND;
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
BoolSettingValue CIEEngine::SetScrollBars(BoolSettingValue dwBoolSettingValue)
{ 
	//if the engine has not been initialized, fail
	CIEEngine* pEngine = GetVisibleTabEngine();
	if (!pEngine)	
		return NOT_IMPLEMENTED;
	else
		return pEngine->SetScrollBars(dwBoolSettingValue);
}

/**
* \author	Darryn Campbell (DCC, JRQ768)
* \date		October 2009
*/
BoolSettingValue CIEEngine::SetScrollBarsOnTab (BoolSettingValue dwBoolSettingValue, 
										int iTabID)
{
	CIEEngine* pEngine = GetSpecificTabEngine(iTabID);
	if (!pEngine)
		return NOT_IMPLEMENTED;
	else
		return pEngine->SetScrollBars(dwBoolSettingValue);
}

/**
* \author	Darryn Campbell (DCC, JRQ768)
* \date		October 2009
*/
BoolSettingValue CIEEngine::GetScrollBars()
{ 
	//  Obtain the visibility of the Scrollbars
	CIEEngine* pEngine = GetVisibleTabEngine();
	if(!pEngine)	
		return NOT_IMPLEMENTED;
	else
		return pEngine->GetScrollBars();
}

/**
* \author	Darryn Campbell (DCC, JRQ768)
* \date		October 2009
*/
BoolSettingValue CIEEngine::GetScrollBarsOnTab (int iTabID)
{
	CIEEngine* pEngine = GetSpecificTabEngine(iTabID);
	if (!pEngine)
		return NOT_IMPLEMENTED;
	else
		return pEngine->GetScrollBars();
}

/**
* \author	Darryn Campbell (DCC, JRQ768)
* \date		October 2009
*/
LONG CIEEngine::Scrollbars_HPosSet (LONG lPos)
{
	CIEEngine* pEngine = GetVisibleTabEngine();
	if (!pEngine)
		return NOT_IMPLEMENTED;
	else
		return pEngine->Scrollbars_HPosSet(lPos);
}

/**
* \author	Darryn Campbell (DCC, JRQ768)
* \date		October 2009
*/
LONG CIEEngine::Scrollbars_HPosSetOnTab (LONG lPos, int iTabID)
{
	CIEEngine* pEngine = GetSpecificTabEngine(iTabID);
	if (!pEngine)
		return NOT_IMPLEMENTED;
	else
		return pEngine->Scrollbars_HPosSet(lPos);
}

/**
* \author	Darryn Campbell (DCC, JRQ768)
* \date		October 2009
*/
LONG CIEEngine::Scrollbars_HPosGet ()
{
	CIEEngine* pEngine = GetVisibleTabEngine();
	if (!pEngine)
		return NOT_IMPLEMENTED;
	else
		return pEngine->Scrollbars_HPosGet();
}

/**
* \author	Darryn Campbell (DCC, JRQ768)
* \date		October 2009
*/
LONG CIEEngine::Scrollbars_HPosGetOnTab (int iTabID)
{
	CIEEngine* pEngine = GetSpecificTabEngine(iTabID);
	if (!pEngine)
		return NOT_IMPLEMENTED;
	else
		return pEngine->Scrollbars_HPosGet();
}

/**
* \author	Darryn Campbell (DCC, JRQ768)
* \date		October 2009
*/
LONG CIEEngine::Scrollbars_VPosSet (LONG lPos)
{
	CIEEngine* pEngine = GetVisibleTabEngine();
	if (!pEngine)
		return NOT_IMPLEMENTED;
	else
		return pEngine->Scrollbars_VPosSet(lPos);
}

/**
* \author	Darryn Campbell (DCC, JRQ768)
* \date		October 2009
*/
LONG CIEEngine::Scrollbars_VPosSetOnTab (LONG lPos, int iTabID)
{
	CIEEngine* pEngine = GetSpecificTabEngine(iTabID);
	if (!pEngine)
		return NOT_IMPLEMENTED;
	else
		return pEngine->Scrollbars_VPosSet(lPos);
}

/**
* \author	Darryn Campbell (DCC, JRQ768)
* \date		October 2009
*/
LONG CIEEngine::Scrollbars_VPosGet ()
{
	CIEEngine* pEngine = GetVisibleTabEngine();
	if (!pEngine)
		return NOT_IMPLEMENTED;
	else
		return pEngine->Scrollbars_VPosGet();
}

/**
* \author	Darryn Campbell (DCC, JRQ768)
* \date		October 2009
*/
LONG CIEEngine::Scrollbars_VPosGetOnTab (int iTabID)
{
	CIEEngine* pEngine = GetSpecificTabEngine(iTabID);
	if (!pEngine)
		return NOT_IMPLEMENTED;
	else
		return pEngine->Scrollbars_VPosGet();
}

/**
* \author	Geoff Day (GRD, XFH386)
* \date		March 2010
*/
BOOL CIEEngine::Scrollbars_SizeGet (int *pwidth, int *pheight)
{
	CIEEngine* pEngine = GetVisibleTabEngine();
	if (!pEngine)
		return NOT_IMPLEMENTED;
	else
		return pEngine->Scrollbars_SizeGet (pwidth, pheight);
}

/**
* \author	Geoff Day (GRD, XFH386)
* \date		March 2010
*/
BOOL CIEEngine::Scrollbars_SizeGetOnTab (int *pwidth, int *pheight, int iTabID)
{
	CIEEngine* pEngine = GetSpecificTabEngine(iTabID);
	if (!pEngine)
		return NOT_IMPLEMENTED;
	else
		return pEngine->Scrollbars_SizeGet (pwidth, pheight);
}

//////////////////////////////////////////////////
//												//
//		Application (Tab) Management (Public)	//
//												//
//////////////////////////////////////////////////

/**
* \author	Darryn Campbell (DCC, JRQ768)
* \date		October 2009 (DCC: Initial Creation)
* \date		November 2009 (DCC: Added Subclassing of HTML Message Window)
*/
LRESULT CIEEngine::Tab_New (int iTabID, BOOL bBringToForeground, 
							LPCTSTR tcIconURL, BoolSettingValue bsvScrollbars,
							HTMLWndPROC_T hWndProc, WNDPROC* ownerProc, 
							ReadEngineConfigParameter_T configFunction)
{
	//  Don't allow duplication of the Tab ID
	if (TabExists(iTabID))
		return S_FALSE;

	//  Create the New Tab
	IETab* newIETab = new IETab();
	newIETab->iTabID = iTabID;
	newIETab->nextTab = NULL;
	newIETab->prevTab = NULL;
	newIETab->bVisible = bBringToForeground;
	newIETab->pEngine = 
		new PB_ENGINE_TAB_FLAVOUR(m_hparentInst, m_parentHWND, iTabID, 
									tcIconURL, bsvScrollbars);
	if (newIETab->pEngine == NULL)
		return S_FALSE;
	newIETab->pEngine->CreateEngine(configFunction);

	//  Subclass the HTML window which receives the windows messages.  This is 
	//  done in a separate thread as it will not succeed until the window 
	//  is fully initialised.
	newIETab->pEngine->m_hWndProc = hWndProc;
	newIETab->pEngine->m_ownerProc = ownerProc;
	CloseHandle (CreateThread(NULL, 0, 
			&CIEEngine::RegisterWndProcThread, 
			(LPVOID)newIETab->pEngine, 0, NULL));

	//  Add this tab to the end of the tab attributes list and set all
	//  other tabs to be invisible.
	IETab* tempTab = m_ieTabList;
	while (tempTab != NULL)
	{
		//  If we want the new tab to be in the foreground disable all other 
		//  HTML windows.  Note we'll only be changing the foreground app if 
		//  the device is not locked
		if (bBringToForeground && !m_bDeviceLocked)
		{
			tempTab->bVisible = FALSE;
			//  Development note, temptation here is to EnableWindow(HWND, FALSE)
			//  to ensure background tabs do not respond to keyboard input but
			//  this also stops them navigating.
			ShowWindow(tempTab->pEngine->GetHTMLHWND(), SW_HIDE);
		}
		if (tempTab->nextTab == NULL)
		{
			//  We have reached the end of the existing tabs list, add the 
			//  new tab to the end of the tab attributes list
			tempTab->nextTab = newIETab;
			newIETab->prevTab = tempTab;
			//  and quit the loop
			break;
		}
		tempTab = tempTab->nextTab;
	}

	//  Give the new Tab the same registered events as the previous tab
	ENGINEEVENTPROC* engineProcs = tempTab->pEngine->GetEngineEvents();
	if (engineProcs != NULL)
	{
		for(int ev = 0; ev < EEID_MAXEVENTID; ev++)
		{
			newIETab->pEngine->RegisterForEvent((EngineEventID)ev, *engineProcs);
			engineProcs++;
		}
	}
	newIETab->pEngine->SetClearType(m_dwClearTypeEnabled);
	newIETab->pEngine->SetJavaScript(m_dwJavaScriptEnabled);
	newIETab->pEngine->SetNavigationTimeout(m_dwNavigationTimeout);

	//  Show the Tab
	if (bBringToForeground)
	{
		//  Set the last visible user tab (or Home App)
		if (!Tab_IsReservedID(newIETab->iTabID) || 
			newIETab->iTabID == APP_ID_APP_HOME)
			m_tabIDOfTopUserApplication = newIETab->iTabID;

		if (!m_bDeviceLocked)
		{
			//  If the device is not locked bring the tab to the foreground.
			SetWindowPos(newIETab->pEngine->GetHTMLHWND(), 
						HWND_TOP, 
						0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_SHOWWINDOW);
			ShowWindow(newIETab->pEngine->GetHTMLHWND(), SW_SHOW);
		}
	}
	else
	{
		//  Calling Application requested we not show the Tab, 
		//  ensure it is not displayed by default
		ShowWindow(newIETab->pEngine->GetHTMLHWND(), SW_HIDE);
	}

	//  When first created hide the Lock Screen via the Windows API to 
	//  prevent it appearing behind the Application switcher.
	if (iTabID == APP_ID_LOCK_SCREEN)
		ShowWindow(GetHTMLHWNDOnTab(APP_ID_LOCK_SCREEN), SW_HIDE);

	return S_OK;
}

/**
* \author	Darryn Campbell (DCC, JRQ768)
* \date		October 2009
*/
LRESULT CIEEngine::Tab_Close (int iTabID)
{
	//  Ensure this isn't the last Tab
	if (!TabExists(iTabID))
		return S_FALSE;

	//  Find the Tab Index we're being asked to close
	IETab* tabToDelete = m_ieTabList;
	IETab* tabBeforeTabToDelete = NULL;
	while (tabToDelete != NULL)
	{
		if (tabToDelete->iTabID == iTabID)
		{
			//  Found the Tab ID we're being asked to close
			//  If the tab being deleted is currently being shown then
			//  show another tab.  There _should_ always be another tab to show
			//  even if it's just the application switcher.
			if (tabToDelete->bVisible)
			{
				//  Find out which tab to show in preference to this Tab
				int tabIDToShow;
				BOOL CanShowAlternativeTab = Tab_NextTabToShow(iTabID, &tabIDToShow);

				//  Show the suggested tab, else default to trying to show the App Switcher
				if (CanShowAlternativeTab)
					Tab_Switch(tabIDToShow);
				else
				{
					//  Unable to Display another Tab if we were to 
					//  delete this tab, do not continue.
					return S_FALSE;
				}


			}
			//  shut down the engine
			delete tabToDelete->pEngine;
			tabToDelete->pEngine = NULL;
			
			//  Delete the Tab from the list of Tabs
			//  If this is tab 0 set the beginning of the tab list to be the 
			//  second element
			if (tabToDelete == m_ieTabList)
			{
				m_ieTabList = m_ieTabList->nextTab;
				m_ieTabList->prevTab = NULL;
			}
			else
			{
				//  The tab being closed isn't the first tab in the 
				//  tab list
				tabBeforeTabToDelete->nextTab = tabToDelete->nextTab;
				if (tabToDelete->nextTab != NULL)
					tabToDelete->nextTab->prevTab = tabBeforeTabToDelete;
			}
			delete tabToDelete;
			tabToDelete = NULL;
			return S_OK;
		}
		tabToDelete = tabToDelete->nextTab;
		if (tabBeforeTabToDelete == NULL)
			tabBeforeTabToDelete = m_ieTabList;
		else
			tabBeforeTabToDelete = tabBeforeTabToDelete->nextTab;
	}
	//  Unable to find Tab To Delete
	return S_FALSE;	
}

/**
* \author	Darryn Campbell (DCC, JRQ768)
* \date		October 2009
*/
LRESULT CIEEngine::Tab_CloseCurrent ()
{
	//  Find the ID of the tab to close
	IETab* tempTab = m_ieTabList;
	while (tempTab != NULL)
	{
		if (tempTab->bVisible)
			return Tab_Close(tempTab->iTabID);
		tempTab = tempTab->nextTab;
	}
	//  Unable to find a visible tab
	return S_FALSE;
}

/**
* \author	Darryn Campbell (DCC, JRQ768)
* \date		October 2009
*/
LRESULT CIEEngine::Tab_Switch (int iTabID)
{
	if (!TabExists(iTabID))
		return S_FALSE;

	//  Set the last requested applicationt to focus.
	if (!Tab_IsReservedID(iTabID) || 
		iTabID == APP_ID_APP_HOME)
		m_tabIDOfTopUserApplication = iTabID;

	//  Check we're not currently locked
	if (m_bDeviceLocked)
	{
		//  The device is locked, we will switch tabs once the device is
		//  unlocked so return true early.
		return S_OK;
	}

	IETab* tempTab = m_ieTabList;
	while (tempTab != NULL)
	{
		if (tempTab->iTabID == iTabID)
		{
			tempTab->bVisible = TRUE;
			SetWindowPos(tempTab->pEngine->GetHTMLHWND(), 
				HWND_TOP, 
				0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_SHOWWINDOW);
			ShowWindow(tempTab->pEngine->GetHTMLHWND(), SW_SHOW);
		}
		else if (iTabID != APP_ID_APP_SWITCHER)
		{
			//  Only hide the tabs if we're showing a tab other than the App
			//  Switcher
			tempTab->bVisible = FALSE;
			//  Development note, temptation here is to EnableWindow(HWND, FALSE)
			//  to ensure background tabs do not respond to keyboard input but
			//  this also stops them navigating.
			ShowWindow(tempTab->pEngine->GetHTMLHWND(), SW_HIDE);
		}
		tempTab = tempTab->nextTab;
	}
	return S_OK;
}

/**
* \author	Darryn Campbell (DCC, JRQ768)
* \date		October 2009
*/
LRESULT CIEEngine::Tab_Resize(RECT rcNewSize, int iTabID)
{
	//  If the engine has not been initialised, fail.
	CIEEngine* pEngine = GetSpecificTabEngine(iTabID);
	if (!pEngine)
		return S_FALSE;
	else
		return pEngine->Tab_Resize(rcNewSize);
}

/**
* \author	Darryn Campbell (DCC, JRQ768)
* \date		October 2009
*/
int CIEEngine::Tab_GetID ()
{
	return GetVisibleTabID();
}

/**
* \author	Darryn Campbell (DCC, JRQ768)
* \date		October 2009
*/
UINT CIEEngine::Tab_Count ()
{
	TabCountInfo tci;
	GetRunningTabs(&tci);
	return tci.totalApplications;
}

/**
* \author	Darryn Campbell (DCC, JRQ768)
* \date		October 2009
*/
LRESULT CIEEngine::Tab_GetTitle (LPTSTR tcTitle, int iMaxLen)
{
	CIEEngine* pEngine = GetVisibleTabEngine();
	if (!pEngine)
		return S_FALSE;
	else
		return pEngine->Tab_GetTitle(tcTitle, iMaxLen);
}

/**
* \author	Darryn Campbell (DCC, JRQ768)
* \date		October 2009
*/
LRESULT CIEEngine::Tab_GetTitleOnTab (LPTSTR tcTitle, int iMaxLen, int iTabID)
{
	CIEEngine* pEngine = GetSpecificTabEngine(iTabID);
	if (!pEngine)
		return S_FALSE;
	else
		return pEngine->Tab_GetTitle(tcTitle, iMaxLen);
}

/**
* \author	Darryn Campbell (DCC, JRQ768)
* \date		October 2009
*/
LRESULT CIEEngine::Tab_GetIconOnTab (LPTSTR tcIconURL, int iMaxLen, int iTabID)
{
	CIEEngine* pEngine = GetSpecificTabEngine(iTabID);
	if (!pEngine)
		return S_FALSE;
	else
		return pEngine->Tab_GetIcon(tcIconURL, iMaxLen);
}

/**
* \author	Darryn Campbell (DCC, JRQ768)
* \date		October 2009
*/
LRESULT CIEEngine::Tab_ShowSwitcher (BOOL bShow)
{
	if (bShow)
	{
		//  Show the App Switcher
		return Tab_Switch(APP_ID_APP_SWITCHER);
	}
	else
	{
		//  Hide the App Switcher
		return Tab_Hide(APP_ID_APP_SWITCHER);
	}
}

/**
* \author	Darryn Campbell (DCC, JRQ768)
* \date		October 2009
*/
LRESULT CIEEngine::Tab_Lock (BOOL bLockDevice)
{
	if (!TabExists(APP_ID_LOCK_SCREEN) || bLockDevice == m_bDeviceLocked)
	{
		//  If there is no lock screen we are unable to lock the device
		//  or the device is already in the lock state being requested.
		return S_FALSE;
	}
	
	//  The Lock screen is unique in that it should not be shown behind
	//  the application switcher, therefore also call ShowWindow on the 
	//  handle here.
	if (bLockDevice)
	{
		//  Lock the Device
		//  store the current top tab (including the app switcher if shown)
		m_tabIDOfTopUserApplication = GetVisibleTabID();
		LRESULT lReturnValue = Tab_Switch(APP_ID_LOCK_SCREEN);
		ShowWindow(GetHTMLHWNDOnTab(APP_ID_LOCK_SCREEN), SW_SHOW);
		m_bDeviceLocked = true;
		return lReturnValue;
	}
	else
	{
		//  Unlock the Device
		m_bDeviceLocked = false;
		ShowWindow(GetHTMLHWNDOnTab(APP_ID_LOCK_SCREEN), SW_HIDE);
		return Tab_Hide(APP_ID_LOCK_SCREEN);
	}

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
LRESULT CIEEngine::JS_Invoke (LPCTSTR tcFunction)
{
	CIEEngine* pEngine = GetVisibleTabEngine();
	if (!pEngine)
		return S_FALSE;
	else
		return pEngine->JS_Invoke(tcFunction);
}

/**
* \author	Darryn Campbell (DCC, JRQ768)
* \date		October 2009
*/
LRESULT CIEEngine::JS_InvokeOnTab (LPCTSTR tcFunction, int iTabID)
{
	CIEEngine* pEngine = GetSpecificTabEngine(iTabID);
	if (!pEngine)
		return S_FALSE;
	else
		return pEngine->JS_Invoke(tcFunction);
}

/**
* \author	Darryn Campbell (DCC, JRQ768)
* \date		October 2009
*/
LRESULT CIEEngine::JS_Exists (LPCTSTR tcFunction)
{
	CIEEngine* pEngine = GetVisibleTabEngine();
	if (!pEngine)
		return S_FALSE;
	else
		return pEngine->JS_Exists(tcFunction);
}

/**
* \author	Darryn Campbell (DCC, JRQ768)
* \date		October 2009
*/
LRESULT CIEEngine::JS_ExistsOnTab (LPCTSTR tcFunction, int iTabID)
{
	CIEEngine* pEngine = GetSpecificTabEngine(iTabID);
	if (!pEngine)
		return S_FALSE;
	else
		return pEngine->JS_Exists(tcFunction);
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
LRESULT CIEEngine::History_GoForward (UINT iNumPages)
{
	CIEEngine* pEngine = GetVisibleTabEngine();
	if (!pEngine)
		return S_FALSE;
	else
		return pEngine->History_GoForward(iNumPages);
}

/**
* \author	Darryn Campbell (DCC, JRQ768)
* \date		October 2009
*/
LRESULT CIEEngine::History_GoForwardOnTab (UINT iNumPages, int iTabID)
{
	CIEEngine* pEngine = GetSpecificTabEngine(iTabID);
	if (!pEngine)
		return S_FALSE;
	else
		return pEngine->History_GoForward(iNumPages);
}

/**
* \author	Darryn Campbell (DCC, JRQ768)
* \date		October 2009
*/
LRESULT CIEEngine::History_GoBack (UINT iNumPages)
{
	CIEEngine* pEngine = GetVisibleTabEngine();
	if (!pEngine)
		return S_FALSE;
	else
		return pEngine->History_GoBack(iNumPages);
}

/**
* \author	Darryn Campbell (DCC, JRQ768)
* \date		October 2009
*/
LRESULT CIEEngine::History_GoBackOnTab (UINT iNumPages, int iTabID)
{
	CIEEngine* pEngine = GetSpecificTabEngine(iTabID);
	if (!pEngine)
		return S_FALSE;
	else
		return pEngine->History_GoBack(iNumPages);
}


//////////////////////////////////////////////
//											//
//		Accessors for Properties (Public)	//
//											//
//////////////////////////////////////////////

/**
* \author	Darryn Campbell (DCC, JRQ768)
* \date		October 2009
* \todo		Implement This
*/
BoolSettingValue CIEEngine::SetFitToScreen(BoolSettingValue dwBoolSettingValue)	
{ 
	//  Fit to Screen is common across all tabs
	return NOT_IMPLEMENTED;
}

/**
* \author	Darryn Campbell (DCC, JRQ768)
* \date		October 2009
* \todo		Implement This
*/
BoolSettingValue CIEEngine::GetFitToScreen()
{ 
	//  Fit to Screen is common across all Tabs
	return NOT_IMPLEMENTED;
}

/**
* \author	Darryn Campbell (DCC, JRQ768)
* \date		October 2009
*/
BoolSettingValue CIEEngine::SetClearType(BoolSettingValue dwBoolSettingValue)
{ 
	m_dwClearTypeEnabled = dwBoolSettingValue;
	LRESULT retVal = S_OK;
	IETab* tempTab = m_ieTabList;
	while (tempTab != NULL)
	{
		if (NOT_IMPLEMENTED == tempTab->pEngine->SetClearType(dwBoolSettingValue))
			return NOT_IMPLEMENTED;
		tempTab = tempTab->nextTab;
	}
	return m_dwClearTypeEnabled;
}

/**
* \author	Darryn Campbell (DCC, JRQ768)
* \date		October 2009
*/
BoolSettingValue CIEEngine::GetClearType()
{ 
	//  ClearType is Common across all Tabs
	return m_dwClearTypeEnabled;
}

/**
* \author	Darryn Campbell (DCC, JRQ768)
* \date		October 2009
*/
BoolSettingValue CIEEngine::SetJavaScript(BoolSettingValue dwBoolSettingValue)
{ 
	m_dwJavaScriptEnabled = dwBoolSettingValue;
	LRESULT retVal = S_OK;
	IETab* tempTab = m_ieTabList;
	while (tempTab != NULL)
	{
		if (NOT_IMPLEMENTED == tempTab->pEngine->SetJavaScript(dwBoolSettingValue))
			return NOT_IMPLEMENTED;
		tempTab = tempTab->nextTab;
	}
	return m_dwJavaScriptEnabled;
}

/**
* \author	Darryn Campbell (DCC, JRQ768)
* \date		October 2009
*/
BoolSettingValue CIEEngine::GetJavaScript()
{ 
	//  JavaScript is Common across all Tabs
	return m_dwJavaScriptEnabled;
}

/**
* \author	Darryn Campbell (DCC, JRQ768)
* \date		October 2009
* \todo		Implement This
*/
BoolSettingValue CIEEngine::SetImages(BoolSettingValue dwBoolSettingValue)
{ 
	//  Images are Tab Specific
	return NOT_IMPLEMENTED;
}

/**
* \author	Darryn Campbell (DCC, JRQ768)
* \date		October 2009
* \todo		Implement This
*/
BoolSettingValue CIEEngine::GetImages()
{ 
	//  Images are Tab Specific
	return NOT_IMPLEMENTED;
}

/**
* \author	Darryn Campbell (DCC, JRQ768)
* \date		October 2009
* \todo		Implement This
*/
BoolSettingValue CIEEngine::SetSounds(BoolSettingValue dwBoolSettingValue)
{ 
	//  Sounds are Tab Specific
	return NOT_IMPLEMENTED;
}

/**
* \author	Darryn Campbell (DCC, JRQ768)
* \date		October 2009
* \todo		Implement This
*/
BoolSettingValue CIEEngine::GetSounds()
{ 
	//  Sounds are Tab Specific
	return NOT_IMPLEMENTED;
}

/**
* \author	Darryn Campbell (DCC, JRQ768)
* \date		October 2009
* \todo		Implement This
*/
BoolSettingValue CIEEngine::SetActiveX(BoolSettingValue dwBoolSettingValue)
{ 
	//  It is not possible to Disable ActiveX for the Internet Explorer 
	//  Engine... is it?
	return SETTING_ON;
}

/**
* \author	Darryn Campbell (DCC, JRQ768)
* \date		October 2009
* \todo		Implement This
*/
BoolSettingValue CIEEngine::GetActiveX()
{ 
	//  It is not possible to Disable ActiveX for the Internet Explorer Engine
	return SETTING_ON;
}

/**
* \author	Darryn Campbell (DCC, JRQ768)
* \date		October 2009
* \todo		Implement This
*/
BoolSettingValue CIEEngine::SetBrowserGesturing (BoolSettingValue dwBoolSettingValue)
{
	//  Internet Explorer Does not Support Browser Gesturing
	return NOT_IMPLEMENTED;
}

/**
* \author	Darryn Campbell (DCC, JRQ768)
* \date		October 2009
* \todo		Implement This
*/
BoolSettingValue CIEEngine::GetBrowserGesturing ()
{
	//  Internet Explorer Does not Support Browser Gesturing
	return NOT_IMPLEMENTED;
}

/**
* \author	Darryn Campbell (DCC, JRQ768)
* \date		October 2009
*/
LRESULT CIEEngine::SetNavigationTimeout(DWORD dwTimeout)
{
	LRESULT retVal = S_OK;
	if (dwTimeout < MINIMUM_NAVIGATION_TIMEOUT)
	{
		retVal = S_FALSE;
	}
	else
	{
		m_dwNavigationTimeout = dwTimeout;
		IETab* tempTab = m_ieTabList;
		while (tempTab != NULL)
		{
			if (S_OK != tempTab->pEngine->SetNavigationTimeout(dwTimeout))
				retVal = S_FALSE;
			tempTab = tempTab->nextTab;
		}
	}
	return retVal;
}

/**
* \author	Darryn Campbell (DCC, JRQ768)
* \date		October 2009
*/
DWORD CIEEngine::GetNavigationTimeout()
{
	//  This value is global across all tabs
	return m_dwNavigationTimeout;
}

/**
* \author	Darryn Campbell (DCC, JRQ768)
* \date		October 2009
* \todo		Implement This
*/
AcceleratorValue CIEEngine::SetAcceleratorMode(AcceleratorValue dwAcceleratorValue)
{ 
	//  Accelerate Mode is tab specific
	LRESULT retVal = S_OK;
	IETab* tempTab = m_ieTabList;
	while (tempTab != NULL)
	{
		if (ACCELERATE_NOT_IMPLEMENTED != tempTab->pEngine->SetAcceleratorMode(dwAcceleratorValue))
			retVal = S_FALSE;
		tempTab = tempTab->nextTab;
	}
	return ACCELERATE_NOT_IMPLEMENTED;
}

/**
* \author	Darryn Campbell (DCC, JRQ768)
* \date		October 2009
* \todo		Implement This
*/
AcceleratorValue CIEEngine::GetAcceleratorMode()
{ 
	//  Accelerate Mode is tab specific
	return ACCELERATE_NORM;
}

//////////////////////////////////////////////
//											//
//		Protected Functions					//
//											//
//////////////////////////////////////////////

/**
* \author	Darryn Campbell (DCC, JRQ768)
* \date		October 2009
*/
CIEEngine* CIEEngine::GetVisibleTabEngine()
{
	//  Returns the Currently Visible Engine
	IETab* tempTab = m_ieTabList;
	while (tempTab != NULL)
	{
		if (tempTab->bVisible)
			return tempTab->pEngine;
		tempTab = tempTab->nextTab;
	}
	return NULL;
}

/**
* \author	Darryn Campbell (DCC, JRQ768)
* \date		October 2009
*/
CIEEngine* CIEEngine::GetSpecificTabEngine(int tabID)
{
	//  Returns the Engine with the Specified Tab ID
	IETab* tempTab = m_ieTabList;
	while (tempTab != NULL)
	{
		if (tempTab->iTabID == tabID)
			return tempTab->pEngine;
		tempTab = tempTab->nextTab;
	}
	return NULL;	
}

/**
* \author	Darryn Campbell (DCC, JRQ768)
* \date		October 2009
*/
IETab* CIEEngine::GetSpecificTab(int iTabID)
{
	//  Returns the Tab with the specified ID
	IETab* tempTab = m_ieTabList;
	while (tempTab != NULL)
	{
		if (tempTab->iTabID == iTabID)
			return tempTab;
		tempTab = tempTab->nextTab;
	}
	return NULL;	
}

/**
* \author	Darryn Campbell (DCC, JRQ768)
* \date		October 2009
*/
IETab* CIEEngine::GetSpecificTab(HWND hwndTab)
{
	//  Returns the Tab with the specified ID
	IETab* tempTab = m_ieTabList;
	while (tempTab != NULL)
	{
		if (tempTab->pEngine->GetHTMLHWND() == hwndTab)
			return tempTab;
		tempTab = tempTab->nextTab;
	}
	return 0;	
}

/**
* \author	Darryn Campbell (DCC, JRQ768)
* \date		October 2009
*/
int CIEEngine::GetVisibleTabID()
{
	IETab* tempTab = m_ieTabList;
	while (tempTab != NULL)
	{
		if (tempTab->bVisible)
			return tempTab->iTabID;
		tempTab = tempTab->nextTab;
	}
	return NULL;
}

/**
* \author	Darryn Campbell (DCC, JRQ768)
* \date		October 2009
*/
void CIEEngine::GetRunningTabs(TabCountInfo* tci)
{
	//  Returns the number of Tabs
	tci->totalApplications = 0;
	tci->pocketBrowserUserApplications = 0;
	tci->reservedApplications = 0;

	IETab* tempTab = m_ieTabList;
	while (tempTab != NULL)
	{
		tci->totalApplications++;
		if (Tab_IsReservedID(tempTab->iTabID))
			tci->reservedApplications++;
		else
			tci->pocketBrowserUserApplications++;

		tempTab = tempTab->nextTab;
	}
}

/**
* \author	Darryn Campbell (DCC, JRQ768)
* \date		October 2009
*/
bool CIEEngine::TabExists(int tabID)
{
	//  Returns whether the Specified tab exists or not
	bool retVal = false;
	IETab* tempTab = m_ieTabList;
	while (tempTab != NULL)
	{
		if (tempTab->iTabID == tabID)
			return true;
		tempTab = tempTab->nextTab;
	}
	return retVal;
}

/**
* \author	Darryn Campbell (DCC, JRQ768)
* \date		October 2009
*/
BOOL CIEEngine::Tab_IsReservedID(int iTabID)
{
	//  Returns whether the specified Tab ID is reserved (shouldn't be treated as 
	//  a normal tab, e.g. the app switcher, lock screen)
	return iTabID < 0;
}

/**
* \author	Darryn Campbell (DCC, JRQ768)
* \date		October 2009
*/
LRESULT CIEEngine::Tab_Hide (int iTabID)
{
	//  Check the Tab Exists
	if (!TabExists(iTabID))
		return S_FALSE;

	//  Check this is the Visible Tab
	if (GetVisibleTabID() != iTabID)
		return S_FALSE;

	//  Find out which tab to show in preference to this Tab
	int tabIDToShow;
	BOOL CanShowAlternativeTab = Tab_NextTabToShow(iTabID, &tabIDToShow);
	if (!CanShowAlternativeTab)
		return S_FALSE;
	else
		return Tab_Switch(tabIDToShow); 
}

/**
* \author	Darryn Campbell (DCC, JRQ768)
* \date		October 2009
*/
BOOL CIEEngine::Tab_NextTabToShow(int iTabIDBeingHidden, int* iTabIDToShow)
{
	//  Populates tabIDToShow with the identifier of the tab to show next if 
	//  iTabIDBeingHidden is to be hidden.  Returns FALSE if there is no sensible tab 
	//  to show next

	//  Obtain information about the currently running tabs.
	TabCountInfo tabCount;
	GetRunningTabs(&tabCount);

	//  Ensure the tab being hidden exists
	if (!TabExists(iTabIDBeingHidden))
	{
		*iTabIDToShow = iTabIDBeingHidden;
		return FALSE;
	}
	//  If the tab being hidden is either the application Switcher or the lock screen
	else if (Tab_IsReservedID(iTabIDBeingHidden))
	{
		//		* ensure there is at least one pocketBrowser application to show 
		//		  and show it.
		//      * OR the requested application is the app swicher (special case)
		if (tabCount.pocketBrowserUserApplications >= 1 || 
			Tab_IsReservedID(m_tabIDOfTopUserApplication ))
		{
			//  There is at least one PocketBrowser Application running
			//  Show the last tab which was requested to be shown.
			if (TabExists(m_tabIDOfTopUserApplication))
			{
				*iTabIDToShow = m_tabIDOfTopUserApplication;
				return TRUE;
			}
			else
			{
				//  The last tab being shown must have been deleted, set the 
				//  first tab in the tab list to be visible
				IETab* tabToShow = m_ieTabList;
				while (tabToShow != NULL)
				{
					if (!Tab_IsReservedID(tabToShow->iTabID))
					{
						//  Found an application tab
						*iTabIDToShow = tabToShow->iTabID;
						return TRUE;
					}
					tabToShow = tabToShow->nextTab;
				}
				//  If we reach here we haven't found a visible user application
				//  Therefore can not suggest an alternative tab to show.
				//  This shouldn't happen as there should be at least one user 
				//  application
				*iTabIDToShow = iTabIDBeingHidden;
				return FALSE;
			}

		}
		else
		{
			//		* If there are no PocketBrowser applications running keep showing 
			//        the hidden screen (Return False)
			*iTabIDToShow = iTabIDBeingHidden;
			return FALSE;
		}
	}
	//  If there is only one PocketBrowser application (which must be the tab being
	//  closed) show the application switcher next if it exsits.
	else if (tabCount.pocketBrowserUserApplications == 1)
	{
		if (TabExists(APP_ID_APP_HOME))
		{
			*iTabIDToShow = APP_ID_APP_HOME;
			return TRUE;
		}
		else
		{
			//  Application Switcher does not exist, unable to suggest
			//  the next visible tab
			*iTabIDToShow = APP_ID_APP_HOME;
			return FALSE;
		}
	}

	//  If there are more than one PocketBrowser applications running search 
	//  forward (then back) of the current position looking for the next tab
	//  to show.
	else if (tabCount.pocketBrowserUserApplications > 1)
	{
		IETab* tabToShow = GetSpecificTab(iTabIDBeingHidden)->nextTab;
		while (tabToShow != NULL)
		{
			if (!Tab_IsReservedID(tabToShow->iTabID))
			{
				//  Found an application tab
				*iTabIDToShow = tabToShow->iTabID;
				return TRUE;
			}
			tabToShow = tabToShow->nextTab;
		}
		//  We have not found a tab forward of the tab being hidden, look 
		//  back
		tabToShow = GetSpecificTab(iTabIDBeingHidden)->prevTab;
		while (tabToShow != NULL)
		{
			if (!Tab_IsReservedID(tabToShow->iTabID))
			{
				//  Found an application tab
				*iTabIDToShow = tabToShow->iTabID;
				return TRUE;
			}
			tabToShow = tabToShow->prevTab;
		}
		//  Should never reach here, it means we have not found more than one 
		//  user application
		*iTabIDToShow = iTabIDBeingHidden;
		return FALSE;
	}

	//  Should never reach here
	*iTabIDToShow = iTabIDBeingHidden;
	return FALSE;
}

//////////////////////////////////////////////
//											//
//		Protected Threaded Functions		//
//											//
//////////////////////////////////////////////


/**
* \author	Darryn Campbell (DCC, JRQ768)
* \date		November 2009 (DCC: Initial Creation)
*/
DWORD WINAPI CIEEngine::RegisterWndProcThread( LPVOID lpParameter )
{
	//  We are passed a pointer to the engine we are interested in.
	CIEEngine* pEngine = (CIEEngine*)lpParameter;

	//  The window tree appears as follows on CE:
	//  +--m_htmlHWND
	//     |
	//     +--child of m_htmlHWND
	//        |
	//        +--child which receives Windows Messages

	//  The window tree appears as follows on WM:
	//  +--m_htmlHWND
	//     |
	//     +--child of m_htmlHWND which receives Windows Messages

	//  Obtain the created HTML HWND
	HWND hwndHTML = NULL;
	HWND hwndHTMLChild1 = NULL;
	HWND hwndHTMLMessageWindow = NULL;
	while (hwndHTMLMessageWindow == NULL)
	{
		hwndHTML = pEngine->GetHTMLHWND();
		if (hwndHTML != NULL)
		{
#ifdef PB_ENGINE_IE_CE
			hwndHTMLChild1 = GetWindow(hwndHTML, GW_CHILD);
			if (hwndHTMLChild1 != NULL)
			{
				hwndHTMLMessageWindow = GetWindow(hwndHTMLChild1, GW_CHILD);
			}
#elif defined (PB_ENGINE_IE_MOBILE)
			hwndHTMLMessageWindow = GetWindow(hwndHTML, GW_CHILD);
#elif defined (PB_ENGINE_IE_WM65)
			hwndHTMLChild1 = GetWindow(hwndHTML, GW_CHILD);
			if (hwndHTMLChild1 != NULL)
			{
				hwndHTMLMessageWindow = GetWindow(hwndHTMLChild1, GW_CHILD);
			}
#endif
		}
		//  Failed to find the desired child window, take a short nap before 
		//  trying again as the window is still creating.
		Sleep(100);
	}

	// GD
	// Remember the window handle which the core will pass to the plugins
	pEngine->m_hwndApp = hwndHTMLMessageWindow;
	// GD

	//  The Tab's HTML message window has been successfully set up, Subclass it.
	//  Note pEngine->m_ownerProc and pEngine->m_hWndProc have been set by 
	//  the calling application to point to values it knows.
	*pEngine->m_ownerProc = (WNDPROC)SetWindowLong(hwndHTMLMessageWindow, 
										GWL_WNDPROC, 
										(LONG)pEngine->m_hWndProc);
	
	ENGINEEVENTPROC* eEngineEvents = pEngine->GetEngineEvents();

	//  If the user has registered to receive the EEID_TOPMOSTHWNDAVAILABLE 
	//  event send the topmost window HWND in the callback.  If they haven't
	//  registered one wait until they do
	while (*eEngineEvents[EEID_TOPMOSTHWNDAVAILABLE] == NULL)
	{
		Sleep(100);
	}
	eEngineEvents[EEID_TOPMOSTHWNDAVAILABLE](EEID_TOPMOSTHWNDAVAILABLE, 
		(int)hwndHTMLMessageWindow, pEngine->GetTabID());
	return 0;
}

BOOL CIEEngine::StrContains (LPCWSTR source, LPCWSTR target)
{
	LPWSTR psource_lower, ptarget_lower;
	BOOL result;

	psource_lower = wcsdup (source);
	_wcslwr (psource_lower);

	ptarget_lower = wcsdup (target);
	_wcslwr (ptarget_lower);

	result = (wcsstr (psource_lower, ptarget_lower) != 0);

	free (psource_lower);
	free (ptarget_lower);

	return result;
}