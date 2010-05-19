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
	char decimalbyteHour;	//Binary-coded decimal hour and minute
	char decimalbyteMinute;

	char assocMpeg[256];	//relative to the zinwell's filesystem
	char assocNav[256];		//
	char assocJpeg[256];	//

	char localMpeg[256];	//the path and filename of the files
	char localNav[256];		//relative to what it is in Windows
	char localJpeg[256];	//

	int loadedMpeg;			//1 if success, 0 if haven't tried
	int loadedNav;			//-1 if failed
	int loadedJpeg;

	//PIDs
	unsigned short PID[8];			//Arbitary - I don't think any of ours have eight PIDs

	//All these things happen twice - prefix dup_
	long dup_unixtime;				//From the unix time (loaded, not calculated)
	unsigned long dup_duration;		//the duration in seconds as loaded
	unsigned long dup_filesize;		//the file size as loaded

	int dup_modjulianday;
	char dup_decimalbyteHour;
	char dup_decimalbyteMinute;

	//I don't like doing this, but this contains hwnds etc. (re-reading this, i don't know why i did it this way!)
	//I'd prefer to have the file stuff as subset - but too lazy to fix
	INFOFILEWINDOW_INFO windowInfo;
};

int InfoWindowRegisterWndClass(HINSTANCE hInst);
HWND InfoWindowCreateOrShow(HWND hwnd, HWND hwndChild, HINSTANCE hInst);
HWND InfoWindowCreate(HWND hwndMDIClient, HINSTANCE hInst);
int  InfoWindowLoadFile(HWND hwnd, char *filename, int stopReloads);

int SaveInfoChanges(HWND hwnd, INFOFILE_INFO *info);


void PaintInfoWindow(HWND hwnd);




LRESULT CALLBACK ChildWndInfoProc(HWND hwnd,UINT msg,WPARAM wParam,LPARAM lParam);

