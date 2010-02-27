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

int MpegWindowLoadFile(HWND hwnd, char * mpegFile)
{

	return 0;
}

int MpegTSFindSyncByte(HANDLE hFile, long * syncbyteOffset)
{
	BYTE testbyte;
	long n;

	BOOL readResult;
	int occurence;
	long offset;
	int justskipped;	//if we have just found one and skipped some bytes, we'll go back

	LARGE_INTEGER largeint_filesize;
	long filesize;

	GetFileSizeEx(hFile, &largeint_filesize);
	filesize=largeint_filesize.LowPart;


	SetFilePointer(hFile, 0x0, NULL, FILE_BEGIN);
	offset=0;
	occurence = 0;
	justskipped=0;



	readResult = ReadFile(hFile, &testbyte, 1, &n, NULL);
	while ((readResult) && (offset<filesize))	{
		if (testbyte == 0x47)	{
			occurence++;
			if (occurence==5)	{
				*syncbyteOffset=offset-940;
				return 1;
			}
			offset+=188;
			justskipped=1;
			readResult = SetFilePointer(hFile, 187, NULL, FILE_CURRENT);
			if (readResult==INVALID_SET_FILE_POINTER)
				return 0;
			readResult = ReadFile(hFile, &testbyte, 1, &n, NULL);
		}
		else	{
			if (justskipped)	{
				offset-=188;
				readResult = SetFilePointer(hFile, -188, NULL, FILE_CURRENT);
				if (readResult==INVALID_SET_FILE_POINTER)
					return 0;
				justskipped=0;
			}
			occurence=0;
			offset++;

			readResult = ReadFile(hFile, &testbyte, 1, &n, NULL);

		}
	}

	return 0;
}

HWND MpegWindowCreateOrShow(HWND hwnd, HWND hwndChild, HINSTANCE hInst)
{
	if (IsWindow(hwnd))	{
		ShowWindow(hwnd, SW_SHOW);
		return hwnd;
	}

	return MpegWindowCreate(hwndChild, hInst);
}

HWND MpegWindowCreate(HWND hwndMDIClient, HINSTANCE hInst)
{
	HWND  hwndChild;
	MDICREATESTRUCT mcs;

	mcs.szClass = "MpegWndClass";      // window class name
	mcs.szTitle = "MPEG-TS File";             // window title
	mcs.hOwner  = hInst;            // owner
	mcs.x       = 0;
	mcs.y       = 400;
	mcs.cx      = 490;    // width
	mcs.cy      = 340;    // height
	mcs.style   = WS_CLIPCHILDREN|WS_CHILD;                // window style
	mcs.lParam  = 0;                // lparam

	hwndChild = (HWND) SendMessage(hwndMDIClient, WM_MDICREATE, 0, (LPARAM)(LPMDICREATESTRUCT) &mcs);

	if (hwndChild != NULL)
		ShowWindow(hwndChild, SW_SHOW);

	return hwndChild;
}

