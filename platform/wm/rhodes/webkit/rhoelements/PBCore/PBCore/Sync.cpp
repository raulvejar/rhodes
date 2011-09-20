#include "StdAfx.h"
#include "Sync.h"

extern BOOL Log			(LogTypeInterface logSeverity,LPCTSTR pLogComment, 
						LPCTSTR pFunctionName, DWORD dwLineNumber,LPCTSTR pCallingModule);


CSync::CSync(void)
{
	m_pHeadEventRec = NULL;
	m_pHeadCallback	= NULL;
	//m_hQuitEvent = CreateEvent(NULL,TRUE,FALSE,NULL);
	memset(m_ArrEventRec,0,sizeof(m_ArrEventRec));
	
}

//initialise any callback functions
void CSync::InitCB(PPBCORESTRUCT pbCoreStruct)
{
	m_pbCoreStruct = pbCoreStruct;
}

CSync::~CSync(void)
{
	
	int iIndex;
	//SetEvent(m_hQuitEvent);
	//EventRecord *pEventRec = NULL; 
	for(iIndex = 0;iIndex < SYNCARRAYSIZE;iIndex++)
	{
		//pEventRec = 
		if(m_ArrEventRec[iIndex].hEvTrigger){
			//DeleteCriticalSection(&m_ArrEventRec[iIndex]->m_CriticalSection);
			CloseHandle(m_ArrEventRec[iIndex].hEvTrigger);
			m_ArrEventRec[iIndex].hEvTrigger = NULL;
			//delete m_ArrEventRec[iIndex];
		}
	}
	//CloseHandle(m_hQuitEvent);
	if(m_pHeadCallback)
		m_pHeadCallback = deletemem(m_pHeadCallback);
	
}

plgCallback *CSync::deletemem(plgCallback *pCallbackRec)
{
	if(pCallbackRec->pNext)
		return deletemem(pCallbackRec->pNext);
	delete pCallbackRec;
	return NULL;
}


BOOL CSync::RegisterCallback(PBModule *pObj,PB_SyncEvent dwEvent,LPCTSTR pCallingModule)
{
	plgCallback *pRec = NULL;
	if(pObj == NULL)
		return FALSE;
	if(dwEvent < 0 )
		return FALSE;
	if(m_ArrEventRec[dwEvent].hEvTrigger == NULL)//don't register a callback if the event has not been created
		return FALSE;

	
	if(m_pHeadCallback == NULL){//check for first record
		m_pHeadCallback = new plgCallback;
		if(m_pHeadCallback == NULL){
			//mem error
			return FALSE;
		}
		pRec = m_pHeadCallback;
		memset(pRec,0,sizeof(plgCallback));
	}
	else{//else check to see if already registered
		for(pRec = m_pHeadCallback;pRec;pRec = pRec->pNext)
		{
			if(pObj == pRec->pObj)
				break;//break if we find a match
						
			else if(pRec->pNext == NULL){
				pRec->pNext = new plgCallback;
				if(pRec->pNext == NULL){
					Log(PB_LOG_ERROR,L"Could not create a callback record",L"CSync::RegisterCallback",__LINE__,L"Core");//mem error
					return FALSE;
				}
				pRec = pRec->pNext;
				memset(pRec,0,sizeof(plgCallback));
				break;
			}
		}
		
	}
	
	pRec->pObj = pObj;
	pRec->pModName = pCallingModule;
	pRec->dwRegister = pRec->dwRegister | (1<<(dwEvent-1));
	Log(PB_LOG_INFO,L"Registered Callback",L"CSync::RegisterCallback",__LINE__,L"Core");
	return TRUE;
	
	
	
}

// GD
BOOL CSync::UnRegisterCallback (PBModule *pmodule, DWORD event_id)
{
	// Check there's something in the list
	if (!m_pHeadCallback)
		return FALSE;

	// Find matching record in callback list for this module object
	// Remove event from registered bit field
	// If there are no more events registered then delete entry

	plgCallback *pprevious, *pentry;

	// Find matching record in list, keeping track of previous list entry if any
	pprevious = NULL;

	for (pentry = m_pHeadCallback; pentry; pentry = pentry->pNext)
	{
		if (pentry->pObj == pmodule)
			break;

		pprevious = pentry;
	}

	// Check we found it
	if (!pentry)
		return FALSE;

	// Remove this event from registered bit field
	DWORD mask = 1 << (event_id - 1);
	pentry->dwRegister &= ~mask;

	// Any callbacks still registered for module?
	if (pentry->dwRegister == 0)
	{
		// No, remove the record from the list
		if (pprevious)
			pprevious->pNext = pentry->pNext;
		else
		{
			if (pentry == m_pHeadCallback)
				m_pHeadCallback = pentry->pNext;
		}

		// Delete the record itself
		delete pentry;
	}

	return TRUE;
}

/*
BOOL CSync::UnRegisterCallback(PBModule *pObj,DWORD dwEvent)
{
	
	if(m_pHeadCallback){//check for first record
		//plgCallback  = NULL;
		plgCallback *pRec,*pTempRec,*oldRec = NULL;
		DWORD dwMask = dwEvent ^ 0xFFFF;
		pRec = pTempRec = oldRec = NULL;
		
		//
		if(m_pHeadCallback->pObj == pObj){
			if(m_pHeadCallback->dwRegister = dwMask & m_pHeadCallback->dwRegister){
				Log(PB_LOG_INFO,L"DeRegistered Callback",L"CSync::UnRegisterCallback",__LINE__,L"Core");
				return TRUE;
			}
			//we have no more events registered on this record
			//so delete it
			pTempRec = m_pHeadCallback->pNext;
			delete m_pHeadCallback;
			m_pHeadCallback = pTempRec;
			return TRUE;
		}
		else{
			for(pRec = m_pHeadCallback;pRec->pNext;pRec = pRec->pNext)
			{
				if(pRec->pNext->pObj == pObj){
					if(pRec->pNext->dwRegister = dwMask & m_pHeadCallback->dwRegister){
						Log(PB_LOG_INFO,L"DeRegistered Callback",L"CSync::UnRegisterCallback",__LINE__,L"Core");
						return TRUE;
					}
					pTempRec = pRec->pNext->pNext;
					delete pRec->pNext;
					pRec->pNext = pTempRec;
					Log(PB_LOG_INFO,L"DeRegistered Callback",L"CSync::UnRegisterCallback",__LINE__,L"Core");
					return TRUE;
				
				}
					
				return FALSE;//not found
			}
		}
		
		//no more registrations so we should delete the entry from the callback linked list
		

		
	}

	return FALSE;
}
*/

//returns the event that will signal when all modules functions have returned
HANDLE CSync::CreateSyncEvent(int iEvent)
{
	EventRecord *pEventRec = &m_ArrEventRec[iEvent];

	if(pEventRec->hEvTrigger == NULL){
		pEventRec->dwEventType = 1<<(iEvent-1);
		
		//InitializeCriticalSection(&pEventRec->m_CriticalSection);
		pEventRec->hEvTrigger = CreateEvent(NULL,TRUE,FALSE,NULL);
		//Log(PB_LOG_INFO,L"Event created",	L"CSync::CreateSyncEvent",iEvent,L"Core");
		return pEventRec->hEvTrigger;
	}
		
	return NULL; 

}

//run synchronously
BOOL CSync::RunSync(int iEvent,LPARAM lParam1,LPARAM lParam2)
{
	EventRecord *pRec = &m_ArrEventRec[iEvent];
	plgCallback *pCBRec;
	
	if(pRec->hEvTrigger == NULL){
		Log(PB_LOG_ERROR,L"Event not set",L"CSync::RunSync",__LINE__,L"Core");
		return FALSE;//no event has been set 
	}		
	
	if(m_pHeadCallback){
		
		ResetEvent(pRec->hEvTrigger);//set the event to non signalled

		//go through each registered callback record 
		for(pCBRec = m_pHeadCallback;pCBRec;pCBRec = pCBRec->pNext)
		{
			
			switch(iEvent)
			{
			case PB_APPFOCUSEVENT:
				//check for a match against each record
				if(pCBRec->dwRegister & pRec->dwEventType){//if this event is to be handled by this callback 
					pCBRec->pObj->onAppFocus((int)lParam1 ,(int)lParam2);
					
				}
				break;
			
			case PB_BROWSER_BEFORE_NAV_EVENT:
				//check for a match against each record
				if(pCBRec->dwRegister & pRec->dwEventType){
					pCBRec->pObj->onBeforeNavigate((int)lParam1);
				}
				break;
			case PB_BROWSER_NAV_COMPLETE_EVENT:
				//check for a match against each record
				if(pCBRec->dwRegister & pRec->dwEventType){
					pCBRec->pObj->onNavigateComplete((int)lParam1);
				}
				break;
			case PB_BROWSER_DOC_COMPLETE_EVENT:
				//check for a match against each record
				if(pCBRec->dwRegister & pRec->dwEventType){
					pCBRec->pObj->onDocumentComplete((int)lParam1);
				}
				break;
			case PB_BROWSER_NAV_ERROR_EVENT:
				//check for a match against each record
				if(pCBRec->dwRegister & pRec->dwEventType){
					pCBRec->pObj->onNavigateError((int)lParam1);
				}
				break;
			case PB_WINDOWCHANGED_EVENT:
				//check for a match against each record
				if(pCBRec->dwRegister & pRec->dwEventType){
					pCBRec->pObj->onWindowChanged((int)lParam1);
				}
				break;
			
			
			}
			
		}
		SetEvent(pRec->hEvTrigger);
		return TRUE;//success


	}
	return FALSE;//error

}


BOOL CSync::Run(int iEvent,LPARAM lParam1,LPARAM lParam2)
{
	
	EventRecord *pRec = &m_ArrEventRec[iEvent];	
	
	Log(PB_LOG_INFO,L"Running Event: ",	L"CSync::Run",iEvent,L"Core");
	if(pRec->hEvTrigger == NULL){
		Log(PB_LOG_ERROR,L"Event not set",L"CSync::Run",__LINE__,L"Core");
		return FALSE;//no event has been set 
	}
			
	
	if(m_pHeadCallback){
		ResetEvent(pRec->hEvTrigger);//set the event to non signalled
		syncThreadParam *pSyncParam = new syncThreadParam;
		if(pSyncParam == NULL){
			Log(PB_LOG_ERROR,L"Mem Alloc error",L"CSync::Run",__LINE__,L"Core");
			SetEvent(pRec->hEvTrigger);//set the event to signalled so's not to hold up any threads
			return FALSE;
		}
		pSyncParam->pEventRec = pRec;
		
		pSyncParam->pHeadCBLst = m_pHeadCallback;
		pSyncParam->lParam1 = lParam1;
		pSyncParam->lParam2 = lParam2;
		pSyncParam->iType = iEvent;//pRec->dwEventType;
		Log(PB_LOG_INFO,L"Creating Thread for: ",	L"CSync::Run",pRec->dwEventType,L"Core");
		pRec->hThread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE) RunThread,(LPVOID) pSyncParam, 0, 0);	
		
		if(pRec->hThread){
			return TRUE;	
		}
			
		Log(PB_LOG_ERROR,L"Thread Handle NULL",	L"CSync::Run",__LINE__,L"Core");

	}
	else{
		Log(PB_LOG_INFO,L"No Registrations yet",	L"CSync::Run",__LINE__,L"Core");
	}
	SetEvent(pRec->hEvTrigger);//set the event to signalled so's not to hold up any threads
	return FALSE;
}

DWORD WINAPI CSync::RunThread(LPVOID lparam)
{
	int iRet = -1;
	syncThreadParam *pParam = (syncThreadParam *)lparam;
	
	if(pParam){
		plgCallback *pCBRec = pParam->pHeadCBLst;
		//go through each registered callback record 
		for(;pCBRec;pCBRec = pCBRec->pNext)
		{
			
			switch(pParam->iType)
			{
			case PB_APPFOCUSEVENT:
				//check for a match against each record
				if(pCBRec->dwRegister & pParam->pEventRec->dwEventType){//if this event is to be handled by this callback 
					pCBRec->pObj->onAppFocus((int)pParam->lParam1 ,(int)pParam->lParam2);
					
				}
				break;
			
			case PB_BROWSER_BEFORE_NAV_EVENT:
				//check for a match against each record
				if(pCBRec->dwRegister & pParam->pEventRec->dwEventType){
					pCBRec->pObj->onBeforeNavigate((int)pParam->lParam1);
				}
				break;
			case PB_BROWSER_NAV_COMPLETE_EVENT:
				//check for a match against each record
				if(pCBRec->dwRegister & pParam->pEventRec->dwEventType){
					pCBRec->pObj->onNavigateComplete((int)pParam->lParam1);
				}
				break;
			case PB_BROWSER_DOC_COMPLETE_EVENT:
				//check for a match against each record
				if(pCBRec->dwRegister & pParam->pEventRec->dwEventType){
					pCBRec->pObj->onDocumentComplete((int)pParam->lParam1);
				}
				break;
			case PB_BROWSER_NAV_ERROR_EVENT:
				//check for a match against each record
				if(pCBRec->dwRegister & pParam->pEventRec->dwEventType){
					pCBRec->pObj->onNavigateError((int)pParam->lParam1);
				}
				break;
			
			case PB_WINDOWCHANGED_EVENT:
				//check for a match against each record
				if(pCBRec->dwRegister & pParam->pEventRec->dwEventType){
					pCBRec->pObj->onWindowChanged((int)pParam->lParam1);
				}
				break;
			
			}
			
		}
		//Any registered plug-in have handled their callbacks so set the event to signalled 
		SetEvent(pParam->pEventRec->hEvTrigger);
		iRet = 0;
	
	}
	CloseHandle(pParam->pEventRec->hThread);
	delete (syncThreadParam *)pParam;
	return iRet;//error code
}

/*
BOOL CSync::Log(LogTypeInterface logSeverity, LPCTSTR pLogComment, 
		   LPCTSTR pFunctionName, DWORD dwLineNumber,int iErrCode)
{
	BOOL bOK = FALSE;
	if(iErrCode){//then add the error code to the end of the comment string 
		int iLen = _tcslen(pLogComment);
		LPTSTR pSzErr = new TCHAR[iLen + 30];
		if(pSzErr == NULL) 
			return FALSE;
		_tcscpy(pSzErr,pLogComment);
		_tcscat(pSzErr,_T(" - Error Code: "));
		_itot(iErrCode,&pSzErr[iLen + 15],10);
		bOK =  m_pbCoreStruct->pLoggingFunc(logSeverity,pSzErr,pFunctionName,dwLineNumber,L"AppManager");
		delete [] pSzErr;
		return bOK;
			
	}
	if(m_pbCoreStruct){
		return m_pbCoreStruct->pLoggingFunc(logSeverity,pLogComment,pFunctionName,dwLineNumber,L"AppManager");
	}
	return TRUE;
}

*/



