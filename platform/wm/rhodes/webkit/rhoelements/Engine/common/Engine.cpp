#include "Engine.h"

CEngine::CEngine()
	: m_dwNavigationTimeout(DEFAULT_NAV_TIMEOUT)
	, m_dwClearTypeEnabled(SETTING_OFF)
	, m_dwJavaScriptEnabled(SETTING_ON)
	, m_hparentInst(0)
{
}

CEngine::~CEngine(void)
{
}

/**
* \author	Darryn Campbell (DCC, JRQ768)
* \date		October 2009
*/
LRESULT CEngine::SetNavigationTimeout(DWORD dwTimeout)		
{ 
	m_dwNavigationTimeout = dwTimeout;
	return S_OK;
};

/**
* \author	Darryn Campbell (DCC, JRQ768)
* \date		October 2009
*/
DWORD CEngine::GetNavigationTimeout()
{ 
	return m_dwNavigationTimeout; 
};


