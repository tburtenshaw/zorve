#include <windows.h>
#include <windowsx.h>

typedef struct sMpegWindowInfo MPEGWINDOW_INFO;
typedef struct sMpegFileInfo MPEGFILE_INFO;

typedef struct sAdaptationField ADAPTATION_FIELD;

char TS_raw_header[32];

struct sAdaptationField	{
	unsigned char adaptationfieldlength;
	BOOL	discontinuity;
	BOOL	randomaccess;
	BOOL	EsPriority;
	BOOL	PCRFlag;
	BOOL	OPCRFlag;
	BOOL	splicingpointflag;
	BOOL	transportprivatedataflag;
	BOOL	adaptationfieldextensionflag;
	ULONGLONG	PcrBase;
	unsigned int	PcrExt;
	ULONGLONG	OpcrBase;
	unsigned int	OpcrExt;
	signed char	splicecountdown;
	unsigned char transportprivatedatalength;


};

struct sTsHeader	{
	BYTE syncbyte;
	BOOL transporterror;
	BOOL payloadstart;
	BOOL transportpriority;
	unsigned int pid;
	BYTE scrambling;
	BYTE adaptation;
	BYTE continuitycounter;

	ADAPTATION_FIELD adaptationfield;
};

struct sMpegFileInfo	{
	HANDLE	hMpegFile;
	unsigned long filesize;
	char filename[MAX_PATH];
};

struct sMpegWindowInfo	{
	MPEGFILE_INFO fileInfo;
};

int MpegWindowRegisterWndClass(HINSTANCE hInst);
LRESULT CALLBACK ChildWndMpegProc(HWND hwnd,UINT msg,WPARAM wParam,LPARAM lParam);

HWND MpegWindowCreateOrShow(HWND hwnd, HWND hwndChild, HINSTANCE hInst);
HWND MpegWindowCreate(HWND hwndMDIClient, HINSTANCE hInst);

int MpegWindowLoadFile(HWND hwnd, char * mpegFile);
int MpegTSFindSyncByte(HANDLE hFile, long * syncbyteOffset);
