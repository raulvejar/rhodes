//-----------------------------------------------------------------------------
// FILENAME:		kbdapi.h
// MODULE NAME:		N/A
//
// Copyright(c) 2004 Symbol Technologies Inc. All rights reserved.
//
// DESCRIPTION:		Provide protypes for the Symbol 
//					Matix Keyboard API
//
// NOTES:			None
//
// 
//-----------------------------------------------------------------------------
#ifndef KBDAPI_H
#define KBDAPI_H

#ifdef __cplusplus
extern "C"
{
#endif

//--------------------------------------------------------------------
// Nested Includes
//--------------------------------------------------------------------
//#include "kbddef.h"

//
// Keyboard driver name
//
#define KEYBOARD_DLL TEXT("keybddr.dll")

//
// API's exported for applications to call.
//

//-------------------------------------------------------------------
// FUNCTION:   BOOL RegisterKeyStateNotification (HWND hWnd, uint Msg);
// PARAMETERS:	HWND hWnd - specifies the window to be notified.
//				   uint Msg  - specifies the message to be sent (WM_APP+x). 
// RETURNS:		TRUE on successful completion, FALSE if error.
// NOTES:		
// Applications call the RegisterKeyStateNotification routine to receive notification
// when there's changes in key states. The hWnd parameter specifies the window to be
// notified and the Msg parameter specifies the message to be sent (WM_APP+x). 
// When a key state change occurs, the keyboard driver will notify registered
// callers by posting a message.
//-------------------------------------------------------------------
#define REGKEYNOTIFY          TEXT("RegisterKeyStateNotification")
typedef BOOL (WINAPI* LPFNREGKEYNOTIFY)(HWND hWnd, uint Msg);

//-------------------------------------------------------------------
// FUNCTION:      BOOL UnregisterKeyStateNotification (HWND hWnd);
// PARAMETERS:	   HWND hWnd - specifies the window to be removed.
// DESCRIPTION:	This function requests that the handle no longer
//				      receive keyboard state notification
// RETURNS:		   TRUE on successful completion, FALSE if error.
//-------------------------------------------------------------------
#define UNREGKEYNOTIFY	      TEXT("UnregisterKeyStateNotification")
typedef BOOL (WINAPI* LPFNUNREGKEYNOTIFY) (HWND hWnd);

//-------------------------------------------------------------------
// FUNCTION:   BOOL RegisterKeyboardNotification (HWND hWnd, uint Msg);
// PARAMETERS:	HWND hWnd - specifies the window to be notified.
//				   uint Msg  - specifies the message to be sent (WM_APP+x). 
// RETURNS:		TRUE on successful completion, FALSE if error.
// NOTES: 
// Applications call the RegisterKeyboardNotification routine to receive notification
// when there's changes in keyboard layout. For example, changed from Alpha layout to 
// unshifted layout.  The hWnd parameter specifies the window to be
// notified and the Msg parameter specifies the message to be sent (WM_APP+x). 
// When a keyboard layout change occurs, the keyboard driver will notify registered
// callers by posting a message.
//-------------------------------------------------------------------
#define REGKEYBOARDNOTIFY	   TEXT("RegisterKeyboardNotification")
typedef BOOL (WINAPI* LPFNREGKEYBOARDNOTIFY)(HWND hWnd, uint Msg);

//-------------------------------------------------------------------
// FUNCTION:      BOOL UnregisterKeyboardNotification (HWND hWnd);
// PARAMETERS:	   HWND hWnd - specifies the window to be removed.
// DESCRIPTION:	This function requests that the handle no longer
//				      receive keyboard layout notification
// RETURNS:		   TRUE on successful completion, FALSE if error.
//-------------------------------------------------------------------
#define UNREGKEYBOARDNOTIFY	TEXT("UnregisterKeyboardNotification")
typedef BOOL (WINAPI* LPFNUNREGKEYBOARDNOTIFY) (HWND hWnd);

//-------------------------------------------------------------------
// FUNCTION:      BOOL SetKeyState(DWORD dwState, DWORD dwActiveModifer, 
//                               BOOL bUpdateRegistry);
// PARAMETERS:	   DWORD dwState - Keybord state bit field(s)
//				      DWORD dwActiveModifier - The remapping table selector.	
//				      BOOL bUpdateRegistry - Flag requesting the registry to be updated.
// DESCRIPTION:	Re-initialize the keyboard state flags to match the requested state.
//				      Read CE System to determine what it thinks it's state is
// RETURNS:		   TRUE on successful completion, FALSE if error.
//
// NOTES:  
//				dwState uses the same bitfield definitions as wParam in notification calls.
//			   dwActiveModifer uses the same bitfield definitions as wParam in notification calls.
//-------------------------------------------------------------------
#define SETKEYSTATE		      TEXT("SetKeyState")
typedef BOOL (WINAPI* LPFNSETKEYSTATE)(DWORD dwState, DWORD dwActiveModifer, BOOL bUpdateRegistry);	

//-------------------------------------------------------------------
// FUNCTION:   BOOL SetMappingTable (LPCTSTR pszFileName, BOOL bUpdateRegistry)
// PARAMETERS:	LPCTSTR	pszFileName - 
//						pointer to the NULL terminated filename string
//				   BOOL	bUpdateRegistry - 
//						Boolean flag requesting the registry to be updated.
//						TRUE: update registry, FALSE do not update registry
// DESCRIPTION:	Load the keyboard remapping table using the file specified.  
//				      Reinitialize the key-mapping structure with the one in the file.
// RETURNS:		TRUE on successful completion, FALSE if error.
//-------------------------------------------------------------------
#define SETMAPTABLE		      TEXT("SetMappingTable")
typedef BOOL (WINAPI* LPFNSETMAPTABLE)(LPCTSTR pszFileName, BOOL bUpdateRegistry);

//-------------------------------------------------------------------
// FUNCTION:   BOOL SetMultiMappingTable (LPCTSTR pszFileName, BOOL bUpdateRegistry)
// PARAMETERS:	LPCTSTR	pszFileName - 
//						pointer to the NULL terminated filename string
//				   BOOL	bUpdateRegistry - 
//						Boolean flag requesting the registry to be updated.
//						TRUE: update registry, FALSE do not update registry
// DESCRIPTION:	Load the keyboard multi remapping table using the file specified.  
//				      Reinitialize the key-mapping structure with the one in the file.
// RETURNS:		TRUE on successful completion, FALSE if error.
//-------------------------------------------------------------------
#define SETMULTIMAPTABLE	   TEXT("SetMultiMappingTable")
typedef BOOL (WINAPI* LPFNSETMULTIMAPTABLE)(LPCTSTR pszFileName, BOOL bUpdateRegistry);

//-------------------------------------------------------------------
// FUNCTION:      BOOL SetAlphaMode (DWORD dwKeyMode);
// PARAMETERS:	   None
// DESCRIPTION:	This function set the keyboard state to Alpha mode
// RETURNS:		   TRUE on successful completion, FALSE if error.
//-------------------------------------------------------------------
#define SETALPHAMODE			   TEXT("SetAlphaMode")
typedef BOOL (WINAPI* LPFNSETALPHAMODE)(DWORD dwKeyMode);

//-------------------------------------------------------------------
// FUNCTION:      DWORD GetAlphaMode (void);
// PARAMETERS:	   None
// DESCRIPTION:	This function get the current alpha key state 
// RETURNS:		   return 1 if in alpha mode; 0 otherwise.
//-------------------------------------------------------------------
#define GETALPHAMODE			   TEXT("GetAlphaMode")
typedef DWORD (WINAPI* LPFNGETALPHAMODE)(void);

#ifdef __cplusplus
}
#endif

#endif // KBDAPI_H
