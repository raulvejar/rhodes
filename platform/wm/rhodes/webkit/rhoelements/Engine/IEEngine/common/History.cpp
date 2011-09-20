#include "History.h"
#include "IEEngine.h"


/**
* \author	Darryn Campbell (DCC, JRQ768)
* \date		October 2009 (Initial Creation, DCC)
*/
CHistory::CHistory(CIEEngine* associatedEngineTab)
{
	//  The engine associated with this history, so we can call functions
	//  on the engine such as Navigate
	m_associatedEngineTab = associatedEngineTab;

	//  There are currently no pages in thie history
	m_urlList = NULL;
	m_currentPage = NULL;
}

/**
* \author	Darryn Campbell (DCC, JRQ768)
* \date		October 2009 (Initial Creation, DCC)
*/
CHistory::~CHistory()
{
	//  ToDo - Test This Method
	m_associatedEngineTab = NULL;

	//  Delete the memory associated with the history List
	m_currentPage = NULL;
	DeleteCascade(m_urlList);
}

/**
* \author	Darryn Campbell (DCC, JRQ768)
* \date		October 2009 (Initial Creation, DCC)
*/
BOOL CHistory::Add(LPCTSTR urlNew)
{
	if (wcsstr(urlNew, L"PocketBrowser\\HTML\\LoadPage.html") ||
		wcslen(urlNew) == 0)
		return FALSE;

	CHistoryElement* newElement = new CHistoryElement();
	newElement->pNext = NULL;
	newElement->pPrev = NULL;
	newElement->tcURL = new TCHAR[wcslen(urlNew)+1];
	wcscpy(newElement->tcURL, urlNew);

	//  Base Case, there is no current URL History
	if (m_urlList == NULL)
	{
		//  Start the History List
		m_urlList = newElement;
		m_currentPage = newElement;
	}
	else
	{
		//  There is already at least one item in the history
		//  Check we're not trying to add the same item to the history
		//  twice (Reload)
		if(!wcscmp(m_currentPage->tcURL, urlNew))
			return FALSE;

		//  Check the History hasn't grown too large
		//  Assume the Maximum history size is sensible, suggest a value of 50
		if (BackListSize() >= MAX_HISTORY && MAX_HISTORY > 2)
		{
			//  History will be too large after the next element is added
			//  Remove the first element in the history list and free the memory
			CHistoryElement* firstElement = m_urlList;
			m_urlList = m_urlList->pNext;
			m_urlList->pPrev = NULL;
			delete[] firstElement->tcURL;
			delete firstElement;
		}

		//  Delete all history items FORWARD of the currentPage
		DeleteCascade(m_currentPage->pNext);

		//  Add the new history item to the List
		m_currentPage->pNext = newElement;
		newElement->pPrev = m_currentPage;
		m_currentPage = newElement;
	}
	return TRUE;
}

/**
* \author	Darryn Campbell (DCC, JRQ768)
* \date		October 2009 (Initial Creation, DCC)
*/
void CHistory::DeleteCascade(CHistoryElement* fromThisElementOn)
{
	CHistoryElement* deletingElement = fromThisElementOn;
	while (deletingElement != NULL)
	{
		delete[] deletingElement->tcURL;
		deletingElement->tcURL = NULL;
		CHistoryElement* nextElement = deletingElement->pNext;
		delete deletingElement;
		deletingElement = nextElement;
	}
}

/**
* \author	Darryn Campbell (DCC, JRQ768)
* \date		October 2009 (Initial Creation, DCC)
*/
LRESULT CHistory::Back(UINT iPagesPrevious)
{
	//  iPreviousPages must be greater than 0, otherwise we're not doing anything
	if (iPagesPrevious == 0)
	{
		m_associatedEngineTab->Stop();
		return m_associatedEngineTab->Navigate(m_currentPage->tcURL);
	}

	//  Check to see we can go back this many pages
	int iHistoryCounter = 0;
	CHistoryElement* tempHistoryElement = m_currentPage;
	while(tempHistoryElement != NULL && tempHistoryElement->pPrev != NULL)
	{
		//  We can go to the previous page
		iHistoryCounter++;
		//  Go back another item in the history
		tempHistoryElement = tempHistoryElement->pPrev;
		//  If we have gone back the specified number of times navigate to the 
		//  page
		if (iPagesPrevious == iHistoryCounter)
		{
			if (m_associatedEngineTab != NULL)
			{
				m_currentPage = tempHistoryElement;
				return m_associatedEngineTab->Navigate(m_currentPage->tcURL);
			}
		}
	}

	//  If we exit the While loop we were not able to go back the 
	//  specified number of places
	return S_FALSE;
}

/**
* \author	Darryn Campbell (DCC, JRQ768)
* \date		October 2009 (Initial Creation, DCC)
*/
LRESULT CHistory::Forward(UINT iPagesForward)
{
	//  iPreviousPages must be greater than 0, otherwise we're not doing anything
	if (iPagesForward == 0)
		return S_OK;

	//  Check to see we can go forward this many pages
	int iHistoryCounter = 0;
	CHistoryElement* tempHistoryElement = m_currentPage;
	while(tempHistoryElement != NULL && tempHistoryElement->pNext != NULL)
	{
		//  We can go to the previous page
		iHistoryCounter++;
		//  Go forward another item in the history
		tempHistoryElement = tempHistoryElement->pNext;
		//  If we have gone forward the specified number of times navigate to the 
		//  page
		if (iPagesForward == iHistoryCounter)
		{
			if (m_associatedEngineTab != NULL)
			{
				m_currentPage = tempHistoryElement;
				return m_associatedEngineTab->Navigate(m_currentPage->tcURL);
			}
		}
	}
	//  If we exit the While loop we were not able to go forward the 
	//  specified number of places
	return S_FALSE;
}

/**
* \author	Darryn Campbell (DCC, JRQ768)
* \date		October 2009 (Initial Creation, DCC)
*/
UINT CHistory::BackListSize()
{
	UINT iHistoryCounter = 1;
	CHistoryElement* tempHistoryElement = m_currentPage;
	while (tempHistoryElement != NULL && tempHistoryElement->pPrev != NULL)
	{
		//  We can go back
		iHistoryCounter++;
		tempHistoryElement = tempHistoryElement->pPrev;
	}
	return iHistoryCounter;
}


