#include <windows.h>
#include <windowsx.h>

typedef struct sInfoFileInformation INFOFILE_INFO;

struct sInfoFileInformation
{
	char filename[MAX_PATH];

	//These are identical to the list vars
	char recordingname[120];
	long unixtime;				//From the unix time (loaded, not calculated)
	unsigned long duration;		//the duration in seconds as loaded
	unsigned long filesize;		//the file size as loaded

	char description[512];		//Very comfortable number of bytes

	int decimalbyteHour;
	int decimalbyteMinute;

	char assocMpeg[256];
	char assocNav[256];
	char assocJpeg[256];

	//PIDs
	int PID[8];			//Arbitary - I don't think any of ours have eight PIDs

	//All these things happen twice - prefix dup
	long dup_unixtime;				//From the unix time (loaded, not calculated)
	unsigned long dup_duration;		//the duration in seconds as loaded
	unsigned long dup_filesize;		//the file size as loaded

	int dup_decimalbyteHour;
	int dup_decimalbyteMinute;


};

int InfoWindowRegisterWndClass(HINSTANCE hInst);
HWND InfoWindowCreateOrShow(HWND hwnd, HWND hwndChild, HINSTANCE hInst);
HWND InfoWindowCreate(HWND hwndMDIClient, HINSTANCE hInst);
int  InfoWindowLoadFile(HWND hwnd, char *filename);






LRESULT CALLBACK ChildWndInfoProc(HWND hwnd,UINT msg,WPARAM wParam,LPARAM lParam);

