/**
 *  \file Engine.h
 *  \brief Definition of base engine class
 */

#pragma once

#include ".\pbengine_defs.h"

/**
 *  Parent class for all derived Browser Engines.
 *  This class defines the methods and attributes common between all 
 *  PocketBrowser engines, e.g. Internet Explorer Engine, WebKit Engine.
 */
class CEngine
{
public:
	CEngine();
	~CEngine(void);

	//////////////////////////////////////////////
	//											//
	//		Setup								//
	//											//
	//////////////////////////////////////////////

	/**
	*  This virtual function is designed to be overridden by a child engine 
	*  class, see CIEEngine::InitEngine for an example.
	*  \param configFunction Pointer to the Cores configuration reading function.
	*  \author	Darryn Campbell (DCC, JRQ768)
	*  \date		October 2009
	*/
	virtual LRESULT InitEngine(HINSTANCE hInstance, HWND hwndParent, int iTabID, 
		LPCTSTR tcIconURL, BoolSettingValue bsvScrollbars, 
		ReadEngineConfigParameter_T configFunction) 
		{return S_FALSE;};

	/**
	*  This virtual function is designed to be overridden by a child engine
	*  class, see CIEEngine::DeinitEngine for an example.
	*  \author	Darryn Campbell (DCC, JRQ768)
	*  \date		October 2009
	*/
	virtual LRESULT DeinitEngine() 
		{return S_FALSE;};

	/**
	*  This virtual function is designed to be overridden by a child engine 
	*  class, see CIEEngine::CreateEngine for an example.
	*  \param configFunction Pointer to the Cores configuration reading function.
	*  \author	Darryn Campbell (DCC, JRQ768)
	*  \date		October 2009
	*/
	virtual LRESULT CreateEngine(ReadEngineConfigParameter_T configFunction) 
		{return S_FALSE;};

	/**
	*  This virtual function is designed to be overridden by a child engine 
	*  class, see CIEEngine::RegisterForEvent for an example.
	*  \author	Darryn Campbell (DCC, JRQ768)
	*  \date		October 2009
	*/
	virtual LRESULT RegisterForEvent(EngineEventID eeidEventID, ENGINEEVENTPROC pEventFunc) 
		{ return S_FALSE; };

	/**
	*  This virtual function is designed to be overridden by a child engine 
	*  class, see CIEEngine::PreprocessMessage for an example.
	*  \author	Darryn Campbell (DCC, JRQ768)
	*  \date		December 2009
	*/
	virtual LRESULT PreprocessMessage(MSG msg) {return S_FALSE;};


	//////////////////////////////////////////////
	//											//
	//		Navigation							//
	//											//
	//////////////////////////////////////////////

	/**
	*  This virtual function is designed to be overridden by a child engine 
	*  class, see CIEEngine::Navigate for an example.
	*  \author	Darryn Campbell (DCC, JRQ768)
	*  \date		October 2009
	*/
	virtual LRESULT Navigate(LPCTSTR tcAddress) 
		{return S_FALSE;};

	/**
	*  This virtual function is designed to be overridden by a child engine 
	*  class, see CIEEngine::NavigateOnTab for an example.
	*  \author	Darryn Campbell (DCC, JRQ768)
	*  \date		October 2009
	*/
	virtual LRESULT NavigateOnTab (LPCTSTR tcAddress, int iTabID) 
		{return S_FALSE;};

	/**
	*  This virtual function is designed to be overridden by a child engine 
	*  class, see CIEEngine::Stop for an example.
	*  \author	Darryn Campbell (DCC, JRQ768)
	*  \date		October 2009
	*/
	virtual LRESULT Stop(void) 
		{return S_FALSE;};

	/**
	*  This virtual function is designed to be overridden by a child engine 
	*  class, see CIEEngine::StopOnTab for an example.
	*  \author	Darryn Campbell (DCC, JRQ768)
	*  \date		October 2009
	*/
	virtual LRESULT StopOnTab (int iTabID) 
		{return S_FALSE;};

	/**
	*  This virtual function is designed to be overridden by a child engine 
	*  class, see CIEEngine::Reload for an example.
	*  \author	Darryn Campbell (DCC, JRQ768)
	*  \date		October 2009
	*/
	virtual LRESULT Reload(BOOL bFromCache) 
		{return S_FALSE;};

	/**
	*  This virtual function is designed to be overridden by a child engine 
	*  class, see CIEEngine::ReloadOnTab for an example.
	*  \author	Darryn Campbell (DCC, JRQ768)
	*  \date		October 2009
	*/
	virtual LRESULT ReloadOnTab (BOOL bFromCache, int iTabID) 
		{return S_FALSE;};

	/**
	*  This virtual function is designed to be overridden by a child engine 
	*  class, see CWebKitEngine::Zoom for an example.
	*  \author	Darryn Campbell (DCC, JRQ768)
	*  \date		October 2009
	*/
	virtual LRESULT Zoom(double dFactor) 
		{return S_FALSE;};

	/**
	*  This virtual function is designed to be overridden by a child engine 
	*  class, see CWebKitEngine::ZoomOnTab for an example.
	*  \author	Darryn Campbell (DCC, JRQ768)
	*  \date		October 2009
	*/
	virtual LRESULT ZoomOnTab (double dFactor, int iTabID) 
		{return S_FALSE;};

	/**
	*  This virtual function is designed to be overridden by a child engine 
	*  class, see CWebKitEngine::GetZoomOnTab for an example.
	*  \author	Darryn Campbell (DCC, JRQ768)
	*  \date		October 2009
	*/
	virtual LRESULT GetZoomOnTab (double *dFactor, int iTabID) 
		{return S_FALSE;};

	/**
	*  This virtual function is designed to be overridden by a child engine 
	*  class, see CIEEngine::ZoomText for an example.
	*  \author	Darryn Campbell (DCC, JRQ768)
	*  \date		October 2009
	*/
	virtual LRESULT ZoomText(TextZoomValue dwZoomLevel) 
		{return S_FALSE;};

	/**
	*  This virtual function is designed to be overridden by a child engine 
	*  class, see CIEEngine::ZoomTextOnTab for an example.
	*  \author	Darryn Campbell (DCC, JRQ768)
	*  \date		October 2009
	*/
	virtual LRESULT ZoomTextOnTab (TextZoomValue dwZoomLevel, int iTabID) 
		{return S_FALSE;};

	/**
	*  This virtual function is designed to be overridden by a child engine 
	*  class, see CIEEngine::GetZoomTextOnTab for an example.
	*  \author	Darryn Campbell (DCC, JRQ768)
	*  \date		October 2009
	*/
	virtual LRESULT GetZoomTextOnTab (TextZoomValue *dwZoomLevel, int iTabID) 
		{return S_FALSE;};

	//////////////////////////////////////////////
	//											//
	//		HWND Accessors						//
	//											//
	//////////////////////////////////////////////


	/**
	*  This virtual function is designed to be overridden by a child engine 
	*  class, see CIEEngine::GetHTMLHWND for an example.
	*  \author	Darryn Campbell (DCC, JRQ768)
	*  \date		October 2009
	*/
	virtual HWND GetHTMLHWND() 
		{return NULL;};

	/**
	*  This virtual function is designed to be overridden by a child engine
	*  class, see CIEEngine::GetHTMLHWNDOnTab for an example.
	*  \author	Darryn Campbell (DCC, JRQ768)
	*  \date		October 2009
	*/
	virtual HWND GetHTMLHWNDOnTab(int iTabID) 
		{return NULL;};

	/**
	*  This virtual function is designed to be overridden by a child engine 
	*  class, see CIEEngine::GetParentHWND for an example.
	*  \author	Darryn Campbell (DCC, JRQ768)
	*  \date		October 2009
	*/
	virtual HWND GetParentHWND() 
		{return NULL;};


	//////////////////////////////////////////////
	//											//
	//		Scrollbars							//
	//											//
	//////////////////////////////////////////////

	/**
	*  This virtual function is designed to be overridden by a child engine 
	*  class, see CIEEngine::SetScrollBars for an example.
	*  \author	Darryn Campbell (DCC, JRQ768)
	*  \date		October 2009
	*/
	virtual BoolSettingValue SetScrollBars(BoolSettingValue dwBoolSettingValue)	
		{ return NOT_IMPLEMENTED; };

	/**
	*  This virtual function is designed to be overridden by a child engine 
	*  class, see CIEEngine::SetScrollBarsOnTab for an example.
	*  \author	Darryn Campbell (DCC, JRQ768)
	*  \date		October 2009
	*/
	virtual BoolSettingValue SetScrollBarsOnTab(BoolSettingValue dwBoolSettingValue, int iTabID)	
		{ return NOT_IMPLEMENTED; };

	/**
	*  This virtual function is designed to be overridden by a child engine 
	*  class, see CIEEngine::GetScrollBars for an example.
	*  \author	Darryn Campbell (DCC, JRQ768)
	*  \date		October 2009
	*/
	virtual BoolSettingValue GetScrollBars()										
		{ return NOT_IMPLEMENTED; };

	/**
	*  This virtual function is designed to be overridden by a child engine 
	*  class, see CIEEngine::GetScrollBarsOnTab for an example.
	*  \author	Darryn Campbell (DCC, JRQ768)
	*  \date		October 2009
	*/
	virtual BoolSettingValue GetScrollBarsOnTab(int iTabID)									
		{ return NOT_IMPLEMENTED; };

	/**
	*  This virtual function is designed to be overridden by a child engine 
	*  class, see CIEEngine::Scrollbars_HPosSet for an example.
	*  \author	Darryn Campbell (DCC, JRQ768)
	*  \date		October 2009
	*/
	virtual LONG Scrollbars_HPosSet (LONG lPos) 
		{return -1;};

	/**
	*  This virtual function is designed to be overridden by a child engine 
	*  class, see CIEEngine::Scrollbars_HPosSetOnTab for an example.
	*  \author	Darryn Campbell (DCC, JRQ768)
	*  \date		October 2009
	*/
	virtual LONG Scrollbars_HPosSetOnTab (LONG lPos, int iTabID) 
		{return -1;};

	/**
	*  This virtual function is designed to be overridden by a child engine 
	*  class, see CIEEngine::Scrollbars_HPosGet for an example.
	*  \author	Darryn Campbell (DCC, JRQ768)
	*  \date		October 2009
	*/
	virtual LONG Scrollbars_HPosGet () 
		{return -1;};

	/**
	*  This virtual function is designed to be overridden by a child engine 
	*  class, see CIEEngine::Scrollbars_HPosGetOnTab for an example.
	*  \author	Darryn Campbell (DCC, JRQ768)
	*  \date		October 2009
	*/
	virtual LONG Scrollbars_HPosGetOnTab (int iTabID) 
		{return -1;};

	/**
	*  This virtual function is designed to be overridden by a child engine 
	*  class, see CIEEngine::Scrollbars_VPosSet for an example.
	*  \author	Darryn Campbell (DCC, JRQ768)
	*  \date		October 2009
	*/
	virtual LONG Scrollbars_VPosSet (LONG lPos)											
		{return -1;};

	/**
	*  This virtual function is designed to be overridden by a child engine 
	*  class, see CIEEngine::Scrollbars_VPosSetOnTab for an example.
	*  \author	Darryn Campbell (DCC, JRQ768)
	*  \date		October 2009
	*/
	virtual LONG Scrollbars_VPosSetOnTab (LONG lPos, int iTabID)						
		{return -1;};

	/**
	*  This virtual function is designed to be overridden by a child engine 
	*  class, see CIEEngine::Scrollbars_VPosGet for an example.
	*  \author	Darryn Campbell (DCC, JRQ768)
	*  \date		October 2009
	*/
	virtual LONG Scrollbars_VPosGet ()													
		{return -1;};

	/**
	*  This virtual function is designed to be overridden by a child engine 
	*  class, see CIEEngine::Scrollbars_VPosGetOnTab for an example.
	*  \author	Darryn Campbell (DCC, JRQ768)
	*  \date		October 2009
	*/
	virtual LONG Scrollbars_VPosGetOnTab (int iTabID)									
		{return -1;};


	//////////////////////////////////////////////
	//											//
	//		Application (Tab) Management		//
	//											//
	//////////////////////////////////////////////

	/**
	*  This virtual function is designed to be overridden by a child engine 
	*  class, see CIEEngine::Tab_New for an example.
	*  \author	Darryn Campbell (DCC, JRQ768)
	*  \date		October 2009
	*/
	virtual LRESULT Tab_New (int iTabID, BOOL bBringToForeground, LPCTSTR tcIconURL,
		BoolSettingValue bsvScrollbars)	
		{return S_FALSE;};

	/**
	*  This virtual function is designed to be overridden by a child engine 
	*  class, see CIEEngine::Tab_Close for an example.
	*  \author	Darryn Campbell (DCC, JRQ768)
	*  \date		October 2009
	*/
	virtual LRESULT Tab_Close (int iTabID)												
		{return S_FALSE;};

	/**
	*  This virtual function is designed to be overridden by a child engine 
	*  class, see CIEEngine::Tab_CloseCurrent for an example.
	*  \author	Darryn Campbell (DCC, JRQ768)
	*  \date		October 2009
	*/
	virtual LRESULT Tab_CloseCurrent ()													
		{return S_FALSE;};

	/**
	*  This virtual function is designed to be overridden by a child engine 
	*  class, see CIEEngine::Tab_Switch for an example.
	*  \author	Darryn Campbell (DCC, JRQ768)
	*  \date		October 2009
	*/
	virtual LRESULT Tab_Switch (int index)												
		{return S_FALSE;};

	/**
	*  This virtual function is designed to be overridden by a child engine
	*  class, see CIEEngine::Tab_Resize for an example.
	*  \author	Darryn Campbell (DCC, JRQ768)
	*  \date		October 2009
	*/
	virtual LRESULT Tab_Resize(RECT rcNewSize, int iTabID)								
		{return S_FALSE;};

	/**
	*  This virtual function is designed to be overridden by a child engine 
	*  class, see CIEEngine::Tab_GetID for an example.
	*  \author	Darryn Campbell (DCC, JRQ768)
	*  \date		October 2009
	*/
	virtual int  Tab_GetID ()															
		{return S_FALSE;};

	/**
	*  This virtual function is designed to be overridden by a child engine 
	*  class, see CIEEngine::Tab_Count for an example.
	*  \author	Darryn Campbell (DCC, JRQ768)
	*  \date		October 2009
	*/
	virtual UINT  Tab_Count ()															
		{return S_FALSE;};

	/**
	*  This virtual function is designed to be overridden by a child engine 
	*  class, see CIEEngine::Tab_GetTitle for an example.
	*  \author	Darryn Campbell (DCC, JRQ768)
	*  \date		October 2009
	*/
	virtual LRESULT Tab_GetTitle (LPTSTR title, int iMaxLen)							
		{return S_FALSE;};

	/**
	*  This virtual function is designed to be overridden by a child engine 
	*  class, see CIEEngine::Tab_GetTitleOnTab for an example.
	*  \author	Darryn Campbell (DCC, JRQ768)
	*  \date		October 2009
	*/
	virtual LRESULT Tab_GetTitleOnTab (LPTSTR tcTitle, int iMaxLen, int iTabID)			
		{return S_FALSE;};

	/**
	*  This virtual function is designed to be overridden by a child engine 
	*  class, see CIEEngine::Tab_GetIconOnTab for an example.
	*  \author	Darryn Campbell (DCC, JRQ768)
	*  \date		October 2009
	*/
	virtual LRESULT Tab_GetIconOnTab (LPTSTR tcIconURL, int iMaxLen, int iTabID)		
		{return S_FALSE;};

	/**
	*  This virtual function is designed to be overridden by a child engine 
	*  class, see CIEEngine::Tab_ShowSwitcher for an example.
	*  \author	Darryn Campbell (DCC, JRQ768)
	*  \date		October 2009
	*/
	virtual LRESULT Tab_ShowSwitcher (BOOL bShow)										
		{return S_FALSE;};

	/**
	*  This virtual function is designed to be overridden by a child engine
	*  class, see CIEEngine::Tab_Lock for an example.
	*  \author	Darryn Campbell (DCC, JRQ768)
	*  \date		October 2009
	*/
	virtual LRESULT Tab_Lock (BOOL bLockDevice)											
		{return S_FALSE;};


	//////////////////////////////////////////////
	//											//
	//		JavaScript							//
	//											//
	//////////////////////////////////////////////

	/**
	*  This virtual function is designed to be overridden by a child engine 
	*  class, see CIEEngine::JS_Invoke for an example.
	*  \author	Darryn Campbell (DCC, JRQ768)
	*  \date		October 2009
	*/
	virtual LRESULT JS_Invoke (LPCTSTR tcFunction)										
		{return S_FALSE;};

	/**
	*  This virtual function is designed to be overridden by a child engine 
	*  class, see CIEEngine::JS_InvokeOnTab for an example.
	*  \author	Darryn Campbell (DCC, JRQ768)
	*  \date		October 2009
	*/
	virtual LRESULT JS_InvokeOnTab (LPCTSTR tcFunction, int iTabID)						
		{return S_FALSE;};

	/**
	*  This virtual function is designed to be overridden by a child engine 
	*  class, see CIEEngine::JS_Exists for an example.
	*  \author	Darryn Campbell (DCC, JRQ768)
	*  \date		October 2009
	*/
	virtual LRESULT JS_Exists (LPCTSTR tcFunction)										
		{return S_FALSE;};

	/**
	*  This virtual function is designed to be overridden by a child engine 
	*  class, see CIEEngine::JS_ExistsOnTab for an example.
	*  \author	Darryn Campbell (DCC, JRQ768)
	*  \date		October 2009
	*/
	virtual LRESULT JS_ExistsOnTab (LPCTSTR tcFunction, int iTabID)						
		{return S_FALSE;};


	//////////////////////////////////////////////
	//											//
	//		History								//
	//											//
	//////////////////////////////////////////////

	/**
	*  This virtual function is designed to be overridden by a child engine 
	*  class, see CIEEngine::History_GoForward for an example.
	*  \author	Darryn Campbell (DCC, JRQ768)
	*  \date		October 2009
	*/
	virtual LRESULT History_GoForward	(UINT iNumPages)								
		{return S_FALSE;};

	/**
	*  This virtual function is designed to be overridden by a child engine 
	*  class, see CIEEngine::History_GoForwardOnTab for an example.
	*  \author	Darryn Campbell (DCC, JRQ768)
	*  \date		October 2009
	*/
	virtual LRESULT History_GoForwardOnTab (UINT iNumPages, int iTabID)					
		{return S_FALSE;};

	/**
	*  This virtual function is designed to be overridden by a child engine 
	*  class, see CIEEngine::History_GoBack for an example.
	*  \author	Darryn Campbell (DCC, JRQ768)
	*  \date		October 2009
	*/
	virtual LRESULT History_GoBack		(UINT iNumPages)								
		{return S_FALSE;};

	/**
	*  This virtual function is designed to be overridden by a child engine 
	*  class, see CIEEngine::History_GoBackOnTab for an example.
	*  \author	Darryn Campbell (DCC, JRQ768)
	*  \date		October 2009
	*/
	virtual LRESULT History_GoBackOnTab (UINT iNumPages, int iTabID)					
		{return S_FALSE;};


	//////////////////////////////////////////////
	//											//
	//		Accessors for Properties			//
	//											//
	//////////////////////////////////////////////

	/**
	*  This virtual function is designed to be overridden by a child engine 
	*  class, see CIEEngine::SetFitToScreen for an example.
	*  \author	Darryn Campbell (DCC, JRQ768)
	*  \date		October 2009
	*/
	virtual BoolSettingValue SetFitToScreen(BoolSettingValue dwBoolSettingValue)		
		{ return NOT_IMPLEMENTED; };

	/**
	*  This virtual function is designed to be overridden by a child engine 
	*  class, see CIEEngine::GetFitToScreen for an example.
	*  \author	Darryn Campbell (DCC, JRQ768)
	*  \date		October 2009
	*/
	virtual BoolSettingValue GetFitToScreen()											
		{ return NOT_IMPLEMENTED; };

	/**
	*  This virtual function is designed to be overridden by a child engine 
	*  class, see CIEEngine::SetClearType for an example.
	*  \author	Darryn Campbell (DCC, JRQ768)
	*  \date		October 2009
	*/
	virtual BoolSettingValue SetClearType(BoolSettingValue dwBoolSettingValue)			
		{ return NOT_IMPLEMENTED; };

	/**
	*  This virtual function is designed to be overridden by a child engine 
	*  class, see CIEEngine::GetClearType for an example.
	*  \author	Darryn Campbell (DCC, JRQ768)
	*  \date		October 2009
	*/
	virtual BoolSettingValue GetClearType()												
		{ return NOT_IMPLEMENTED; };

	/**
	*  This virtual function is designed to be overridden by a child engine 
	*  class, see CIEEngine::SetJavaScript for an example.
	*  \author	Darryn Campbell (DCC, JRQ768)
	*  \date		October 2009
	*/
	virtual BoolSettingValue SetJavaScript(BoolSettingValue dwBoolSettingValue)			
		{ return NOT_IMPLEMENTED; };

	/**
	*  This virtual function is designed to be overridden by a child engine 
	*  class, see CIEEngine::GetJavaScript for an example.
	*  \author	Darryn Campbell (DCC, JRQ768)
	*  \date		October 2009
	*/
	virtual BoolSettingValue GetJavaScript()											
		{ return NOT_IMPLEMENTED; };

	/**
	*  This virtual function is designed to be overridden by a child engine 
	*  class, see CIEEngine::SetImages for an example.
	*  \author	Darryn Campbell (DCC, JRQ768)
	*  \date		October 2009
	*/
	virtual BoolSettingValue SetImages(BoolSettingValue dwBoolSettingValue)				
		{ return NOT_IMPLEMENTED; };

	/**
	*  This virtual function is designed to be overridden by a child engine 
	*  class, see CIEEngine::GetImages for an example.
	*  \author	Darryn Campbell (DCC, JRQ768)
	*  \date		October 2009
	*/
	virtual BoolSettingValue GetImages()												
		{ return NOT_IMPLEMENTED; };

	/**
	*  This virtual function is designed to be overridden by a child engine 
	*  class, see CIEEngine::SetSounds for an example.
	*  \author	Darryn Campbell (DCC, JRQ768)
	*  \date		October 2009
	*/
	virtual BoolSettingValue SetSounds(BoolSettingValue dwBoolSettingValue)				
		{ return NOT_IMPLEMENTED; };

	/**
	*  This virtual function is designed to be overridden by a child engine 
	*  class, see CIEEngine::GetSounds for an example.
	*  \author	Darryn Campbell (DCC, JRQ768)
	*  \date		October 2009
	*/
	virtual BoolSettingValue GetSounds()												
		{ return NOT_IMPLEMENTED; };

	/**
	*  This virtual function is designed to be overridden by a child engine 
	*  class, see CIEEngine::SetActiveX for an example.
	*  \author	Darryn Campbell (DCC, JRQ768)
	*  \date		October 2009
	*/
	virtual BoolSettingValue SetActiveX(BoolSettingValue dwBoolSettingValue)			
		{ return NOT_IMPLEMENTED; };

	/**
	*  This virtual function is designed to be overridden by a child engine 
	*  class, see CIEEngine::GetActiveX for an example.
	*  \author	Darryn Campbell (DCC, JRQ768)
	*  \date		October 2009
	*/
	virtual BoolSettingValue GetActiveX()												
		{ return NOT_IMPLEMENTED; };

	/**
	*  This virtual function is designed to be overridden by a child engine 
	*  class, see CIEEngine::SetBrowserGesturing for an example.
	*  \author	Darryn Campbell (DCC, JRQ768)
	*  \date		October 2009
	*/
	virtual BoolSettingValue SetBrowserGesturing (BoolSettingValue dwBoolSettingValue)	
		{return NOT_IMPLEMENTED; };

	/**
	*  This virtual function is designed to be overridden by a child engine 
	*  class, see CIEEngine::GetBrowserGesturing for an example.
	*  \author	Darryn Campbell (DCC, JRQ768)
	*  \date		October 2009
	*/
	virtual BoolSettingValue GetBrowserGesturing ()										
		{return NOT_IMPLEMENTED; };

	/**
	*  This virtual function is designed to be overridden by a child engine 
	*  class, see CIEEngine::SetNavigationTimeout for an example.
	*  \author	Darryn Campbell (DCC, JRQ768)
	*  \date		October 2009
	*/
	virtual LRESULT SetNavigationTimeout(DWORD dwTimeout);

	/**
	*  This virtual function is designed to be overridden by a child engine 
	*  class, see CIEEngine::GetNavigationTimeout for an example.
	*  \author	Darryn Campbell (DCC, JRQ768)
	*  \date		October 2009
	*/
	virtual DWORD GetNavigationTimeout();

	/**
	*  This virtual function is designed to be overridden by a child engine 
	*  class, see CIEEngine::SetAcceleratorMode for an example.
	*  \author	Darryn Campbell (DCC, JRQ768)
	*  \date		October 2009
	*/
	virtual AcceleratorValue SetAcceleratorMode(AcceleratorValue dwAcceleratorValue)	
		{ return ACCELERATE_NORM; };

	/**
	*  This virtual function is designed to be overridden by a child engine 
	*  class, see CIEEngine::GetAcceleratorMode for an example.
	*  \author	Darryn Campbell (DCC, JRQ768)
	*  \date		October 2009
	*/
	virtual AcceleratorValue GetAcceleratorMode()										
		{ return ACCELERATE_NORM; };

	/**
	 * Ekioh GeoLocation integration
	 */
	virtual LRESULT SetLocationInterface(EngineLocationInterface* locationInterface)
                { return S_FALSE; }

        /**
        *  Update the current location when the engine has requested periodic updates.
        *
        *  \param location updated location
        *
        *  This must be called on the main thread.
        */
   virtual LRESULT UpdateLocation(EngineLocation* location)
                { return S_FALSE; }
	
protected:
	HINSTANCE	m_hparentInst;			///< The Parent application's HINSTANCE.
	HWND		m_parentHWND;			///< The Parent application's HWND
	DWORD		m_dwNavigationTimeout;	///< The Engine's navigation timeout, children can override this attribute should they wish to.
	BoolSettingValue	m_dwClearTypeEnabled;	///< Whether or not ClearType is enabled on the Engine
	BoolSettingValue	m_dwJavaScriptEnabled;	///< Whether or not JavaScript is enabled on the Engine
	RECT		m_rcViewSize;			///< The Parent application's geometry.
	
};
