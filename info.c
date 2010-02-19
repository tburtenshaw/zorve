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

	INFOFILE_INFO *infoFile;

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

	infoFile=malloc(sizeof(INFOFILE_INFO));
	SetWindowLong(hwndChild, GWL_USERDATA, (long)infoFile);	//need to set this to pointer to new structure

	return hwndChild;
}

int  InfoWindowLoadFile(HWND hwnd, char *filename)
{

	HANDLE infoFile;
	INFOFILE_INFO *infoFileStruct;

	long n;

	infoFileStruct = (INFOFILE_INFO *)GetWindowLong(hwnd, GWL_USERDATA);
	strcpy(infoFileStruct->filename, filename);

	infoFile = CreateFile(filename, GENERIC_READ, FILE_SHARE_READ, NULL,
				OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL | FILE_FLAG_RANDOM_ACCESS, NULL);


	SetFilePointer(infoFile, 0x07e, NULL, FILE_BEGIN);
	ReadFile(infoFile, &infoFileStruct->description, 512, &n, NULL);


	CloseHandle(infoFile);

	InvalidateRect(hwnd, NULL, FALSE);

	return 0;
}


//This is the main handler for this window
LRESULT CALLBACK ChildWndInfoProc(HWND hwnd,UINT msg,WPARAM wparam,LPARAM lparam)
{
	HDC hdc;
	PAINTSTRUCT psPaint;

	RECT textRect;

	INFOFILE_INFO *infoFile;

	switch(msg) {
		case WM_PAINT:
			infoFile=(INFOFILE_INFO *)GetWindowLong(hwnd, GWL_USERDATA);

			hdc = BeginPaint(hwnd, &psPaint);
			ExtTextOut(hdc, 0,0,ETO_OPAQUE, NULL, infoFile->filename,strlen(infoFile->filename), NULL);


			textRect.left=0;			textRect.right=400;
			textRect.top=20;			textRect.bottom=300;
			DrawText(hdc, infoFile->description,strlen(infoFile->description), &textRect , DT_WORDBREAK);

			EndPaint(hwnd, &psPaint);
			break;

	}
	return DefMDIChildProc(hwnd, msg, wparam, lparam);
}

