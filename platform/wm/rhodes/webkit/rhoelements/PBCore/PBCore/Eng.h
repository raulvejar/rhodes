#pragma once

#ifndef __ENGINE_NEON_H__
#define __ENGINE_NEON_H__

//#include "../../Common/Public/PB_Defines.h"
#include "../../Engine/common/pbengine_defs.h"
// #include "gpswrapper/gpswrapper.h"
// #include "gpswrapper/gpswrapperobserver.h"

class CWebKitEngine // : public IGpsWrapperObserver
{
public:
	CWebKitEngine(HWND hParentWnd, HINSTANCE hInstance);
	~CWebKitEngine(void);
	BOOL DeInitEngine(void);
	BOOL CloseTab(int iInstID);
	BOOL JavaScriptInvoke(int iInstID,LPCTSTR pfunctionJS);
	BOOL JavaScript_Exist(int iInstID,LPCTSTR pfunctionJS);
	BOOL Navigate(LPCTSTR szURL);
	BOOL NavigateOnTab (LPCTSTR szURL, UINT iTab);
	BOOL BackOnTab(int iInstID,int iPagesBack = 1);
	BOOL ForwardOnTab(int iInstID);
	
	BoolSettingValue SetFitToScreen(BoolSettingValue bsvOn);
	BoolSettingValue SetClearType(BoolSettingValue bsvOn);
	BoolSettingValue SetJavaScript(BoolSettingValue bsvOn);
	
	
	BOOL	Stop(void);
	BOOL	StopOnTab (UINT iTab);
	BOOL	Reload(bool bFromCache);
	BOOL	ReloadOnTab (BOOL bFromCache, UINT iTab);
	
	BOOL	ResizeOnTab(int iInstID,RECT rcNewSize);
//	BOOL	Zoom(double dFactor);
//	BOOL	ZoomOnTab (double dFactor, UINT iTab);
	
	BOOL	ZoomText(DWORD dwZoomLevel);

	BOOL	ZoomPageOnTab (double dFactor, UINT iTab);
	
	BOOL	ZoomTextOnTab (TextZoomValue ZoomLevel, UINT iTab);
	BOOL	GetTextZoomOnTab (TextZoomValue* pZoomLevel, UINT iTab);
	
	/**
	*  Wrapper for the engine's function to set the Accelerator Key Mode.
	*  \param eAcceleratorValue New value to set the Accelerator Key Mode to.
	*  \return TRUE if the value was successfully set.
	*/
	BOOL	SetAccelerator (AcceleratorValue eAcceleratorValue);
	BOOL	SetNavigationTimeout(DWORD dwMilliseconds);

	BOOL	GetSize (int *pwidth, int *pheight);
	BOOL	GetSizeOnTab (int *pwidth, int *pheight, int tabid);

	BOOL	SetVScrollOnTab (int scroll, int tab);
	BOOL	SetHScrollOnTab (int scroll, int tab);
	BOOL	GetTitleOnTab(LPTSTR szURL, UINT iMaxLen, UINT iTab);

	BOOL	NewTab(int iInstID,BOOL bForeground,LPCTSTR pHomeURI,BoolSettingValue bScrollBars,HTMLWndPROC_T pSubClassProc, WNDPROC* pOwnerProc);
	BOOL	SwitchTabs(int iNewTab);
	BOOL	RegisterForEvent(EngineEventID EventID,ENGINEEVENTPROC pCBFunc);
	HWND	GetHTMLWND() {return m_lpGetHTMLHWND();};
	HWND	GetHTMLWND(int iInstID) {return m_lpGetHTMLHWNDOnTab(iInstID);};
	HWND	GetParentHWND() {return m_hwndParent;};
	BOOL	LoadEngineDLL(LPCTSTR pDLLPathName);
	BOOL	Init(LPCTSTR pDLLPathName);
	BOOL	InitEngine(int iInstID,HTMLWndPROC_T pSubClassProc, WNDPROC* pOwnerProc,BoolSettingValue bScrollBars, ReadEngineConfigParameter_T configFunction);

	// LRESULT	InitGpsWrapper();
	// LRESULT	DeInitGpsWrapper();
	// LRESULT SendLocationDatoToEngine();

	/**
	*  Accessor method for m_lpPreprocessMessage.
	*  \return m_lpPreprocessMessage
	*/
	tPB_PreprocessMessage	GetlpPreprocessMessage();
	
	/**
	*  Unloads the currently loaded engine DLL from memory
	*  \return TRUE
	*/
	BOOL UnloadEngineDLL();

	//Position change notifier
	// void OnGpsPositionReceived(PGPS_POSITION pPosition){};
	// void OnGpsPositionReceived(EngineLocation* pPosition);
	//GPS state change notifier
	// void OnGpsStateReceived(PGPS_DEVICE pDevice);
private:

	HWND		m_hwndHTML;
	HWND		m_hwndParent;
	HMODULE 	m_hEngineLib;
	HINSTANCE	m_hInstance;
	RECT		m_rcViewSize;
	//  Instances of Functions Exported from the DLL 
	tPB_CreateEngine				m_lpCreateEngine;
	tPB_InitEngine					m_lpInitEngine;
	tPB_DeinitEngine				m_lpDeinitEngine;
	tPB_GetHTMLHWND					m_lpGetHTMLHWND;
	tPB_GetHTMLHWNDOnTab			m_lpGetHTMLHWNDOnTab;
	tPB_GetParentHWND				m_lpGetParentHWND;
	tPB_RegisterForEvent			m_lpRegisterForEvent;
	tPB_PreprocessMessage			m_lpPreprocessMessage;
	tPB_Navigate					m_lpNavigate;
	tPB_NavigateOnTab				m_lpNavigateOnTab;
	tPB_Stop						m_lpStop;
	tPB_StopOnTab					m_lpStopOnTab;
	tPB_Reload						m_lpReload;
	tPB_ReloadOnTab					m_lpReloadOnTab;
	tPB_Zoom						m_lpZoom;
	tPB_ZoomOnTab					m_lpZoomOnTab;
	tPB_ZoomText					m_lpZoomText;
	tPB_ZoomTextOnTab				m_lpZoomTextOnTab;

	tPB_GetZoomTextOnTab			m_lpGetZoomTextOnTab;

	tPB_Scrollbars_HPosSet			m_lpScrollbars_HPosSet;
	tPB_Scrollbars_HPosSetOnTab		m_lpScrollbars_HPosSetOnTab;
	tPB_Scrollbars_HPosGet			m_lpScrollbars_HPosGet;
	tPB_Scrollbars_HPosGetOnTab		m_lpScrollbars_HPosGetOnTab;
	tPB_Scrollbars_VPosSet			m_lpScrollbars_VPosSet;
	tPB_Scrollbars_VPosSetOnTab		m_lpScrollbars_VPosSetOnTab;
	tPB_Scrollbars_VPosGet			m_lpScrollbars_VPosGet;
	tPB_Scrollbars_VPosGetOnTab		m_lpScrollbars_VPosGetOnTab;
	tPB_SetScrollbars				m_lpSetScrollbars;
	tPB_SetScrollbarsOnTab			m_lpSetScrollbarsOnTab;
	tPB_GetScrollbars				m_lpGetScrollbars;
	tPB_GetScrollbarsOnTab			m_lpGetScrollbarsOnTab;
	tPB_Tab_New						m_lpTab_New;
	tPB_Tab_Close					m_lpTab_Close;
	tPB_Tab_CloseCurrent			m_lpTab_CloseCurrent;
	tPB_Tab_Switch					m_lpTab_Switch;
	tPB_GetSize						m_lpGetSize;
	tPB_GetSizeOnTab				m_lpGetSizeOnTab;


	tPB_Tab_Resize					m_lpTab_Resize;
	tPB_Tab_Lock					m_lpTab_Lock;
	tPB_Tab_GetID					m_lpTab_GetID;
	tPB_Tab_Count					m_lpTab_Count;
	tPB_Tab_GetTitle				m_lpTab_GetTitle;

	tPB_Tab_GetTitleOnTab			m_lpTab_GetTitleOnTab;
	tPB_Tab_GetIconOnTab			m_lpTab_GetIconOnTab;


	tPB_Tab_ShowSwitcher			m_lpTab_ShowSwitcher;
	tPB_JS_Invoke					m_lpJS_Invoke;
	tPB_JS_InvokeOnTab				m_lpJS_InvokeOnTab;
	tPB_JS_Exists					m_lpJS_Exists;
	tPB_JS_ExistsOnTab				m_lpJS_ExistsOnTab;
	tPB_History_GoForward			m_lpHistory_GoForward;
	tPB_History_GoForwardOnTab		m_lpHistory_GoForwardOnTab;
	tPB_History_GoBack				m_lpHistory_GoBack;
	tPB_History_GoBackOnTab			m_lpHistory_GoBackOnTab;
	tPB_SetFitToScreen				m_lpSetFitToScreen;
	tPB_GetFitToScreen				m_lpGetFitToScreen;
	tPB_SetClearType				m_lpSetClearType;
	tPB_GetClearType				m_lpGetClearType;
	tPB_SetJavaScript				m_lpSetJavaScript;
	tPB_GetJavaScript				m_lpGetJavaScript;
	tPB_SetImages					m_lpSetImages;
	tPB_GetImages					m_lpGetImages;
	tPB_SetSounds					m_lpSetSounds;
	tPB_GetSounds					m_lpGetSounds;
	tPB_SetActiveX					m_lpSetActiveX;
	tPB_GetActiveX					m_lpGetActiveX;
	tPB_SetAcceleratorMode			m_lpSetAcceleratorMode;
	tPB_GetAcceleratorMode			m_lpGetAcceleratorMode;
	tPB_SetBrowserGesturing			m_lpSetBrowserGesturing;
	tPB_GetBrowserGesturing			m_lpGetBrowserGesturing;
	tPB_SetNavigationTimeout		m_lpSetNavigationTimeout;
	tPB_GetNavigationTimeout		m_lpGetNavigationTimeout;
	
	tPB_SetLocationInterface		m_lpSetLocationInterface;
	tPB_UpdateLocation				m_lpUpdateLocation;

	// CGpsWrapper*					m_pGpsWrapper;
	// EngineLocationInterface*		m_pLocationInterface;
};

#endif