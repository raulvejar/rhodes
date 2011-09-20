/******************************************************************************/
#include <mshtml.h>
#include "ScrollNotify.h"
#include "..\..\..\Common\Public\PB_Defines.h"

/******************************************************************************/
CScrollNotify::CScrollNotify (IWebBrowser2 *pbrowser, HWND htarget)
{
	pBrowser = pbrowser;
	hTarget = htarget;

	hQuit = CreateEvent (NULL, TRUE, FALSE, NULL);
	hThread = CreateThread (NULL, 0, StaticNotifyProc, this, 0, NULL);
}

/******************************************************************************/
CScrollNotify::~CScrollNotify ()
{
	SetEvent (hQuit);
	if (WaitForSingleObject (hThread, 1000) == WAIT_TIMEOUT)
		TerminateThread (hThread, 0);

	CloseHandle (hThread);
	CloseHandle (hQuit);
}

/******************************************************************************/
void CScrollNotify::NotifyProc (void)
{
	IDispatch *pdisp_doc = NULL;
	IHTMLDocument2 *pdoc = NULL;
	IHTMLElement *pbody1 = NULL;
	IHTMLElement2 *pbody2 = NULL;
	HRESULT hr;
	int height, last_height, width, last_width;
	int vpos, last_vpos, hpos, last_hpos;
	WPARAM wparam;
	LPARAM lparam;

	// Get IHTMLElement2 interface on page body
	hr = pBrowser->get_Document (&pdisp_doc);
	if (hr != S_OK || !pdisp_doc)
		goto Exit;

	hr = pdisp_doc->QueryInterface (IID_IHTMLDocument2, (void**) &pdoc);
	if (hr != S_OK || !pdoc)
		goto Exit;

	hr = pdoc->get_body (&pbody1);
	if (hr != S_OK || !pbody1)
		goto Exit;

	hr = pbody1->QueryInterface (IID_IHTMLElement2, (void**) &pbody2);
	if (hr != S_OK || !pbody1)
		goto Exit;

	// Release unneeded interfaces
	pbody1->Release (); pbody1 = NULL;
	pdoc->Release (); pdoc = NULL;
	pdisp_doc->Release (); pdisp_doc = NULL;

	// Watch for changes in page size and scroll position
	last_height = -1;
	last_width = -1;
	last_vpos = -1;
	last_hpos = -1;

	while (1)
	{
		if (WaitForSingleObject (hQuit, 500) != WAIT_TIMEOUT)
			break;

		// Get rendered height and width
		pbody2->get_scrollHeight ((LONG*) &height);
		pbody2->get_scrollWidth ((LONG*) &width);

		// Ignore it if either width or height are zero, page hasn't finished rendering
		if (height == 0 || width == 0)
			continue;

		// Get current scroll positions
		pbody2->get_scrollTop ((LONG*) &vpos);
		pbody2->get_scrollLeft ((LONG*) &hpos);

		// Ignore it if there are no changes from last time
		if (height == last_height && width == last_width && vpos == last_vpos && hpos == last_hpos)
			continue;

		// Notify via message to containing window, which will be passed to any registered plugins
		// LOWORD (wparam) = vertical position, HIWORD (wparam) = height
		// LOWORD (lparam) = horizontal position, HIWORD (lparam) = width
		wparam = MAKEWPARAM (vpos, height);
		lparam = MAKELPARAM (hpos, width);
		PostMessage (hTarget, PBM_SCROLL_CHANGE, wparam, lparam);

		last_height = height;
		last_width = width;
		last_vpos = vpos;
		last_hpos = hpos;
	}

Exit:
	if (pbody2)
		pbody2->Release ();

	if (pbody1)
		pbody1->Release ();

	if (pdoc)
		pdoc->Release ();

	if (pdisp_doc)
		pdisp_doc->Release ();
}

/******************************************************************************/
DWORD CScrollNotify::StaticNotifyProc (LPVOID pparam)
{
	CScrollNotify *pthis = (CScrollNotify*) pparam;
	pthis->NotifyProc ();

	return 0;
}
