#include <windows.h>
#include <windowsx.h>

typedef struct sMpegWindowInfo MPEGWINDOW_INFO;
typedef struct sMpegFileInfo MPEGFILE_INFO;

struct sMpegWindowInfo	{
	MPEGFILE_INFO fileInfo;
};

struct sMpegFileInfo	{
	HANDLE	hMpegFile;
	unsigned long filesize;
	char filename[MAX_PATH];
};

int MpegWindowRegisterWndClass(HINSTANCE hInst);
LRESULT CALLBACK ChildWndMpegProc(HWND hwnd,UINT msg,WPARAM wParam,LPARAM lParam);

HWND MpegWindowCreateOrShow(HWND hwnd, HWND hwndChild, HINSTANCE hInst);
HWND MpegWindowCreate(HWND hwndMDIClient, HINSTANCE hInst);

int MpegWindowLoadFile(HWND hwnd, char * mpegFile);
int MpegTSFindSyncByte(HANDLE hFile, long * syncbyteOffset);
