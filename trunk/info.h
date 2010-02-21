#include <windows.h>
#include <windowsx.h>

typedef struct sInfoFileInformation INFOFILE_INFO;
typedef struct sIFWI INFOFILEWINDOW_INFO;

struct sIFWI
{
	HWND editHwndRecording;
	HWND editHwndDescription;

	HWND buttonHwndSaveAll;
	HWND buttonHwndRevert;
};

struct sInfoFileInformation
{
	long magicLong;
	int unknownWord;

	char filename[MAX_PATH];

	//These are identical to the list vars
	char recordingname[120];
	long unixtime;				//From the unix time (loaded, not calculated)
	unsigned long duration;		//the duration in seconds as loaded
	unsigned long filesize;		//the file size as loaded

	char description[512];		//Very comfortable number of bytes

	int modjulianday;		//Modified Julian date of recording
	char decimalbyteHour;
	char decimalbyteMinute;

	char assocMpeg[256];
	char assocNav[256];
	char assocJpeg[256];

	//PIDs
	unsigned short PID[8];			//Arbitary - I don't think any of ours have eight PIDs

	//All these things happen twice - prefix dup_
	long dup_unixtime;				//From the unix time (loaded, not calculated)
	unsigned long dup_duration;		//the duration in seconds as loaded
	unsigned long dup_filesize;		//the file size as loaded

	int dup_modjulianday;
	char dup_decimalbyteHour;
	char dup_decimalbyteMinute;

	//I don't like doing this, but this contains hwnds etc.
	//I'd prefer to have the file stuff as subset - but too lazy to fix
	INFOFILEWINDOW_INFO windowInfo;
};

int InfoWindowRegisterWndClass(HINSTANCE hInst);
HWND InfoWindowCreateOrShow(HWND hwnd, HWND hwndChild, HINSTANCE hInst);
HWND InfoWindowCreate(HWND hwndMDIClient, HINSTANCE hInst);
int  InfoWindowLoadFile(HWND hwnd, char *filename);

int SaveInfoChanges(HWND hwnd, INFOFILE_INFO *info);


void PaintInfoWindow(HWND hwnd);




LRESULT CALLBACK ChildWndInfoProc(HWND hwnd,UINT msg,WPARAM wParam,LPARAM lParam);

