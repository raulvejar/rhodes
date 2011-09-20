/**
 *  \file History.h
 *  \brief Interface to the CHistory Class, used for storing the history 
 *  information for an Internet Explorer tab.
 */ 

#pragma once

//  Forward Declaration of CIEEngineTab, this allows to declare a pointer 
//  to our owner window in the header file
class CIEEngine;

//  Include global constants relating to the History
#include "../../common/pbengine_defs.h"

/**
*  Stores a single URL with next and previous pointers.  This is the building
*  block of the history list which stores the history of a tab up to 
*  MAX_HISTORY elements.
*/
class CHistoryElement
{
public:
	LPTSTR tcURL;			///< URL of page in the history, this will never be a JavaScript function.
	CHistoryElement* pPrev;	///< Pointer to previous element in the list of history elements.
	CHistoryElement* pNext;	///< Pointer to next element in the list of history elements.
};

/**
* Stores a list of URLs in the order they were navigated 
* in order to allow the users to digress back through their
* navigation path
*/
class CHistory
{
public:
	/**
	*  Create the CHistory object, initialise variables and associate it with 
	*  a specific tab (instance of a PocketBrowser application).
	*  \param associatedEngine Engine tab associated with this history.  This
	*  allows us to communicate with the engine from within the history class, 
	*  for example to instruct the engine to navigate to a URL.
	*/
	CHistory(CIEEngine* associatedEngineTab);

	/**
	*  Free all memory associated with this history list
	*/
	virtual ~CHistory();

	/**
	*  Add a URL to the history list.  The URL will be added to the end 
	*  of the history list, if there are entries after the current pointer 
	*  (because the user has navigated back) all entries after the current 
	*  pointer are deleted.
	*  \param urlNew URL to add to the History List.
	*  \return Whether or not the URL was successfully added to the History
	*/
	BOOL Add(LPCTSTR urlNew);

	/**
	*  Instruct the engine to navigate back to a previously visited page.
	*  This function will have no effect if the tab can not be navigated back 
	*  the specified number of pages.
	*  \param iPagesPrevious The number of pages to go back in the history.
	*  \return Whether or not the tab was able to navigate back
	*/
	LRESULT Back(UINT iPagesPrevious);

	/**
	*  Instruct the engine to navigate forward.  This call will have no effect 
	*  if there have been no previous instructions to navigate back.
	*  \param iPagesForward The number of pages to go forward in the history.
	*  \return Whether or not the tab was able to navigate forward.
	*/
	LRESULT Forward(UINT iPagesForward);

private:
	/**
	*  Calculate the number of times the user can navigate back before they 
	*  run out of history.
	*  \return Number of pages in this back history.
	*/
	UINT BackListSize();

	/**
	*  Given a pointer to an element in the history list this function deletes
	*  all list entries forward of this position.
	*  \param fromThisElementOn Element in this history list after which all 
	*  elements should be deleted.
	*/
	void DeleteCascade(CHistoryElement* fromThisElementOn);

	CIEEngine* m_associatedEngineTab;		///< Engine whose history this object represents.
	CHistoryElement* m_urlList;				///< List of all URLs visited by the browser
	CHistoryElement* m_currentPage;			///< Pointer to current position in the History List
};


