#include <windows.h>
#include <windowsx.h>

typedef struct sDirectoryList DIRECTORY_LIST;
typedef struct sDirectoryInfo DIRECTORY_INFO;
typedef struct sListWindowInfo LISTWINDOW_INFO;

struct sDirectoryInfo
{
	char directoryName[MAX_PATH];
	char directoryFilter[255];
	long numberOfFiles;
	DIRECTORY_LIST *first;
	DIRECTORY_LIST *last;

	LISTWINDOW_INFO	 *parentListWindowInfo;
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

	RECT boundingRect;
};


//This structure is malloc'd and the pointer is used as the windowlong
//It contains:
// - the info about the loaded directory
// - child hwnds
// - vars used for keeping tract of things - e.g. selected item, colours
struct sListWindowInfo
{
	DIRECTORY_INFO directoryInfo;

	//The HWND children of out main list display window
	//Separated into the folder display selector
	//and the scrollable and selectable list of files.
	HWND hwndFolder;
	HWND hwndFiles;

	int heightFolderSelector;	//The height of the folder selector

	int firstLine;				//First line displayed
	int fullyDisplayedLines;	//The number of fully displayed lines (i.e. not partially displayed)

	int	selectedLine;
	int caretedLine;

	RECT oldBoundingRect;

};


//Initiation/backbone stuff
int ListWindowRegisterWndClasses(HINSTANCE hInst);	//Registers the class "ListWndClass" and its children
HWND ListWindowCreateOrShow(HWND listHwnd, HWND hwndMDIClient, HINSTANCE hInst);
LRESULT CALLBACK ChildWndListProc(HWND hwnd,UINT msg,WPARAM wParam,LPARAM lParam);
LRESULT CALLBACK ListChildFolderProc(HWND hwnd,UINT msg,WPARAM wParam,LPARAM lParam);
LRESULT CALLBACK ListChildFileProc(HWND hwnd,UINT msg,WPARAM wParam,LPARAM lParam);

//Display stuff
int PaintListFileWindow(HWND hwnd, LISTWINDOW_INFO *lpListWindowInfo);
int PaintListFolderWindow(HWND hwnd, LISTWINDOW_INFO *lpListWindowInfo);
int ListScrollUpdate(HWND hwnd, LISTWINDOW_INFO *lpListWindowInfo);
int ListWindowHandleVScroll(HWND hwnd, WPARAM wparam, LPARAM lparam);
int ListWindowHandleKeydown(HWND hwnd, WPARAM wparam, LPARAM lparam);
long ListWindowOnMouseWheel(HWND hwnd, short nDelta);

//File stuff
int SetListDirectory(DIRECTORY_INFO *lpDirectoryInfo, char *directorypath);
int UpdateDirectoryList(DIRECTORY_INFO *lpDirectoryInfo);
int CheckAndAddFileToList(DIRECTORY_INFO *lpDirectoryInfo, WIN32_FIND_DATA *fileToAdd);
int ListWindowReadFileDetails(DIRECTORY_LIST* directoryItem, char* filename);
int RefreshAndOrSelectEntry(LISTWINDOW_INFO* windowInfo, DIRECTORY_LIST* entry, BOOL bRefresh, BOOL bSelect);

//Linked list functions
DIRECTORY_LIST* AddEntryCopyToList(DIRECTORY_INFO *lpDirectory, DIRECTORY_LIST *dirToAdd);
void	DeleteDirectoryList(DIRECTORY_INFO *lpDirectoryInfo);
DIRECTORY_LIST* ListWindowGetEntryFromFilename(LISTWINDOW_INFO* windowInfo, char* filename);
