#include "zorveres.h"
#include "navfile.h"

//This registers the Window Class for the list files
int NavWindowRegisterWndClass(HINSTANCE hInst)
{
	WNDCLASS wc;

	//Wipe the wndclass for editing.
	memset(&wc,0,sizeof(WNDCLASS));

	wc.style         = 0;
	wc.lpfnWndProc   = (WNDPROC)ChildWndNavProc;
	wc.cbClsExtra    = 0;
	wc.cbWndExtra    = 20;
	wc.hInstance     = hInst;                      // Owner of this class
	wc.hIcon         = LoadIcon(hInst, MAKEINTRESOURCE(IDI_NAV));
	wc.hCursor       = LoadCursor(NULL, IDC_ARROW);
	wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1); // Default color
	wc.lpszMenuName  = NULL;
	wc.lpszClassName = "NavWndClass";

	if (!RegisterClass((LPWNDCLASS)&wc))
		return 0;
	return 1;
}

//This is the main handler for this window
LRESULT CALLBACK ChildWndNavProc(HWND hwnd,UINT msg,WPARAM wparam,LPARAM lparam)
{
	switch(msg) {
	}
	return DefMDIChildProc(hwnd, msg, wparam, lparam);
}


