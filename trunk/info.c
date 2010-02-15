#include "zorveres.h"
#include "info.h"

//This registers the Window Class for the list files
int InfoWindowRegisterWndClass(HINSTANCE hInst)
{
	WNDCLASS wc;

	//Wipe the wndclass for editing.
	memset(&wc,0,sizeof(WNDCLASS));

	wc.style         = 0;
	wc.lpfnWndProc   = (WNDPROC)ChildWndInfoProc;
	wc.cbClsExtra    = 0;
	wc.cbWndExtra    = 20;
	wc.hInstance     = hInst;                      // Owner of this class
	wc.hIcon         = LoadIcon(hInst, MAKEINTRESOURCE(IDI_INFO));
	wc.hCursor       = LoadCursor(NULL, IDC_ARROW);
	wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1); // Default color
	wc.lpszMenuName  = NULL;
	wc.lpszClassName = "InfoWndClass";

	if (!RegisterClass((LPWNDCLASS)&wc))
		return 0;
	return 1;
}

HWND InfoWindowCreateOrShow(HWND hwnd, HWND hwndChild, HINSTANCE hInst)
{
	if (IsWindow(hwnd))	{
		ShowWindow(hwnd, SW_SHOW);
		return hwnd;
	}

	return InfoWindowCreate(hwndChild, hInst);
}


//Creates the list window
HWND InfoWindowCreate(HWND hwndMDIClient, HINSTANCE hInst)
{
	HWND  hwndChild;
	MDICREATESTRUCT mcs;

	mcs.szClass = "InfoWndClass";      // window class name
	mcs.szTitle = "File Information";             // window title
	mcs.hOwner  = hInst;            // owner
	mcs.x       = 500;
	mcs.y       = 0;
	mcs.cx      = 450;    // width
	mcs.cy      = 320;    // height
	mcs.style   = 0;                // window style
	mcs.lParam  = 0;                // lparam

	hwndChild = (HWND) SendMessage(hwndMDIClient, WM_MDICREATE, 0, (LPARAM)(LPMDICREATESTRUCT) &mcs);

	if (hwndChild != NULL)
		ShowWindow(hwndChild, SW_SHOW);

	return hwndChild;
}


//This is the main handler for this window
LRESULT CALLBACK ChildWndInfoProc(HWND hwnd,UINT msg,WPARAM wparam,LPARAM lparam)
{
	switch(msg) {
	}
	return DefMDIChildProc(hwnd, msg, wparam, lparam);
}

