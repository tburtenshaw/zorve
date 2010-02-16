#include <windows.h>
#include <windowsx.h>

typedef struct sDirectoryList DIRECTORY_LIST;
typedef struct sDirectoryInfo DIRECTORY_INFO;

struct sDirectoryInfo
{
	char directoryName[MAX_PATH];
	char directoryFilter[255];
	long numberOfFiles;
	DIRECTORY_LIST *first;
	DIRECTORY_LIST *last;
};

struct sDirectoryList
{
	char filename[MAX_PATH];	//includes path
	char shortfilename[17];		//path is cropped off, max 16 chars

	char recordingname[120];	//only 120 can be squeezed in file, observed 40 without crashing machine
	long unixtime;				//time&date of recording, obviously breaks at year 2038.
	unsigned long duration;		//the duration in seconds
	unsigned long filesize;		//the file size in bytes (you'd think it would need to be more than a 'long')

	int uptodate;				//boolean, if 1 then no need to read nam/duration/time again


	int index;					//The number of linked list this is.
	DIRECTORY_LIST *next;		//Next in linked list or NULL if at end of list
	DIRECTORY_LIST *prev;		//Prev in linked list (or NULL if list empty)
};

int ListWindowRegisterWndClass(HINSTANCE hInst);	//Registers the class "ListWndClass"
HWND ListWindowCreate(HWND hwndMDIClient, HINSTANCE hInst);
LRESULT CALLBACK ChildWndListProc(HWND hwnd,UINT msg,WPARAM wParam,LPARAM lParam);
int PaintListWindow(HWND hwnd);
int SetListDirectory(DIRECTORY_INFO *lpDirectoryInfo, char *directorypath);
int UpdateDirectoryList(DIRECTORY_INFO *lpDirectoryInfo);
int CheckAndAddFileToList(DIRECTORY_INFO *lpDirectoryInfo, WIN32_FIND_DATA *fileToAdd);

//Linked list functions
DIRECTORY_LIST* AddEntryCopyToList(DIRECTORY_INFO *lpDirectory, DIRECTORY_LIST *dirToAdd);
void	DeleteDirectoryList(DIRECTORY_INFO *lpDirectoryInfo);
