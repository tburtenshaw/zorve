#include "zorve.h"
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
	mcs.x       = 450;
	mcs.y       = 0;
	mcs.cx      = 490;    // width
	mcs.cy      = 340;    // height
	mcs.style   = WS_CLIPCHILDREN|WS_CHILD;                // window style
	mcs.lParam  = 0;                // lparam

	hwndChild = (HWND) SendMessage(hwndMDIClient, WM_MDICREATE, 0, (LPARAM)(LPMDICREATESTRUCT) &mcs);

	if (hwndChild != NULL)
		ShowWindow(hwndChild, SW_SHOW);

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

	//Magic number, then recording name
	ReadFile(infoFile, &infoFileStruct->magicLong, 4, &n, NULL);	//if this isn't 00080500 then should fail
	ReadFile(infoFile, &infoFileStruct->unknownWord, 2, &n, NULL);
	ReadFile(infoFile,  &infoFileStruct->recordingname, 120, &n, NULL);

	//Description
	SetFilePointer(infoFile, 0x07e, NULL, FILE_BEGIN);
	ReadFile(infoFile, &infoFileStruct->description, 512, &n, NULL);
	infoFileStruct->description[511]=0;	//terminate the string, just in case it isn't

	//time, size and duration (first copy)
	SetFilePointer(infoFile, 0x0294, NULL, FILE_BEGIN);
	ReadFile(infoFile, &infoFileStruct->unixtime, 4, &n, NULL);

	SetFilePointer(infoFile, 0x02a0, NULL, FILE_BEGIN);
	ReadFile(infoFile, &infoFileStruct->filesize, 4, &n, NULL);

	SetFilePointer(infoFile, 0x02a8, NULL, FILE_BEGIN);
	ReadFile(infoFile, &infoFileStruct->duration, 4, &n, NULL);



	CloseHandle(infoFile);

	SendMessage(infoFileStruct->windowInfo.editHwndRecording, WM_SETTEXT, 0, (LPARAM)&infoFileStruct->recordingname);
	SendMessage(infoFileStruct->windowInfo.editHwndDescription, WM_SETTEXT, 0, (LPARAM)&infoFileStruct->description);


	InvalidateRect(hwnd, NULL, FALSE);

	return 0;
}

int SaveInfoChanges(HWND hwnd, INFOFILE_INFO *info)
{
	HANDLE infoFile;
	long n;

	SendMessage(info->windowInfo.editHwndRecording, WM_GETTEXT, 120, (LPARAM)&info->recordingname);
	SendMessage(info->windowInfo.editHwndDescription, WM_GETTEXT, 512, (LPARAM)&info->description);


	infoFile = CreateFile(info->filename, GENERIC_WRITE, 0x0000, NULL,
				OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL | FILE_FLAG_RANDOM_ACCESS, NULL);

	SetFilePointer(infoFile, 0x0006, NULL, FILE_BEGIN);
	WriteFile(infoFile, &info->recordingname, strlen(info->recordingname)+1, &n, NULL);

	SetFilePointer(infoFile, 0x007e, NULL, FILE_BEGIN);
	WriteFile(infoFile, &info->description, strlen(info->description)+1, &n, NULL);


	CloseHandle(infoFile);

	return 0;
}



//This is the main handler for this window
LRESULT CALLBACK ChildWndInfoProc(HWND hwnd,UINT msg,WPARAM wparam,LPARAM lparam)
{
	HINSTANCE hInst;

	INFOFILE_INFO *infoFile;

	switch(msg) {
		case WM_CREATE:
			hInst=((LPCREATESTRUCT)lparam)->hInstance;

			infoFile=malloc(sizeof(INFOFILE_INFO));
			SetWindowLong(hwnd, GWL_USERDATA, (long)infoFile);	//need to set this to pointer to new structure

			infoFile->windowInfo.editHwndRecording =	CreateWindow("EDIT", "Recording name",
				    					WS_CHILD|WS_VISIBLE|ES_LEFT|WS_TABSTOP|WS_BORDER|ES_AUTOHSCROLL,
									    100,20,0,0, hwnd, NULL, hInst, NULL);
			SendMessage(infoFile->windowInfo.editHwndRecording, EM_LIMITTEXT, 50, 0);	//limit text to 50 chars

			infoFile->windowInfo.editHwndDescription =	CreateWindow("EDIT", "Description",
				    					WS_CHILD|WS_VISIBLE|ES_LEFT|WS_TABSTOP|WS_BORDER|ES_AUTOVSCROLL|ES_MULTILINE,
									    100,40,250,60, hwnd, NULL, hInst, NULL);
			SendMessage(infoFile->windowInfo.editHwndDescription, EM_LIMITTEXT, 200, 0);	//limit text to 200 chars

			infoFile->windowInfo.buttonHwndSaveAll =	CreateWindow("BUTTON", "Save changes",
				    					WS_CHILD|WS_VISIBLE|WS_TABSTOP|WS_BORDER|BS_PUSHBUTTON,
									    20,250,120,24, hwnd, NULL, hInst, NULL);
			infoFile->windowInfo.buttonHwndRevert =	CreateWindow("BUTTON", "Revert",
				    					WS_CHILD|WS_VISIBLE|WS_TABSTOP|WS_BORDER|BS_PUSHBUTTON,
									    160,250,120,24, hwnd, NULL, hInst, NULL);

			break;
		case WM_COMMAND:
			infoFile = (INFOFILE_INFO *)GetWindowLong(hwnd, GWL_USERDATA);
			if (lparam==(LPARAM)infoFile->windowInfo.buttonHwndSaveAll)
				SaveInfoChanges(hwnd, infoFile);
			break;
		case WM_SIZE:
			InvalidateRect(hwnd, NULL, FALSE);
			break;
		case WM_PAINT:
			PaintInfoWindow(hwnd);
			break;
		case WM_ERASEBKGND:
			return 1;
			break;

	}
	return DefMDIChildProc(hwnd, msg, wparam, lparam);
}


void PaintInfoWindow(HWND hwnd)
{
	INFOFILE_INFO *infoFile;

	HDC hdc;
	PAINTSTRUCT psPaint;

	RECT clientRect;
	RECT textRect;

	TEXTMETRIC textMetric;
	SIZE sizeTextExtent;
	int xEditColumn;
	int y;
	int height;
	int margin=5;


	infoFile=(INFOFILE_INFO *)GetWindowLong(hwnd, GWL_USERDATA);

	GetClientRect(hwnd, &clientRect);

	hdc = BeginPaint(hwnd, &psPaint);
	SetBkColor(hdc, RGB_ZINNY_MIDPURPLE);

	//Blank our canvas
	ExtTextOut(hdc, 0,0,ETO_OPAQUE, &clientRect, infoFile->filename,strlen(infoFile->filename), NULL);


	xEditColumn=180;
	y=24;
	height=20;

	GetTextMetrics(hdc, &textMetric);
	height=textMetric.tmHeight;
	GetTextExtentPoint32(hdc, "Description:",12,&sizeTextExtent);
	xEditColumn=sizeTextExtent.cx+margin;

	textRect.left=margin; textRect.right=xEditColumn;
	textRect.top=y; textRect.bottom=y+height;

	ExtTextOut(hdc, margin, y, ETO_OPAQUE, &textRect, "Recording:", 10, NULL);
	MoveWindow(infoFile->windowInfo.editHwndRecording, xEditColumn+margin, y, (clientRect.right-clientRect.left)-xEditColumn-margin-margin, height+4, TRUE);

	y+=height+4+margin;

	textRect.left=margin; textRect.right=xEditColumn;
	textRect.top=y; textRect.bottom=y+height;

	ExtTextOut(hdc, margin, y, ETO_OPAQUE, &textRect, "Description:", 12, NULL);
	MoveWindow(infoFile->windowInfo.editHwndDescription, xEditColumn+margin, y, (clientRect.right-clientRect.left)-xEditColumn-margin-margin,height*4+4, TRUE);

	EndPaint(hwnd, &psPaint);

	return;
}


