#include "zorveres.h"
#include "mpegts.h"

//This registers the Window Class for the list files
int MpegWindowRegisterWndClass(HINSTANCE hInst)
{
	WNDCLASS wc;

	//Wipe the wndclass for editing.
	memset(&wc,0,sizeof(WNDCLASS));

	wc.style         = 0;
	wc.lpfnWndProc   = (WNDPROC)ChildWndMpegProc;
	wc.cbClsExtra    = 0;
	wc.cbWndExtra    = 20;
	wc.hInstance     = hInst;                      // Owner of this class
	wc.hIcon         = LoadIcon(hInst, MAKEINTRESOURCE(IDI_MPEGTS));
	wc.hCursor       = LoadCursor(NULL, IDC_ARROW);
	wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1); // Default color
	wc.lpszMenuName  = NULL;
	wc.lpszClassName = "MpegWndClass";

	if (!RegisterClass((LPWNDCLASS)&wc))
		return 0;
	return 1;
}

LRESULT CALLBACK ChildWndMpegProc(HWND hwnd,UINT msg,WPARAM wparam,LPARAM lparam)
{
	switch(msg) {
	}
	return DefMDIChildProc(hwnd, msg, wparam, lparam);
}

