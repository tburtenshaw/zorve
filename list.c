#include <stdio.h>
#include "zorve.h"
#include "zorveres.h"
#include "list.h"

//This registers the Window Class for the list files
int ListWindowRegisterWndClass(HINSTANCE hInst)
{
	WNDCLASS wc;

	//Wipe the wndclass for editing.
	memset(&wc,0,sizeof(WNDCLASS));

	wc.style         = 0;
	wc.lpfnWndProc   = (WNDPROC)ChildWndListProc;
	wc.cbClsExtra    = 0;
	wc.cbWndExtra    = 20;
	wc.hInstance     = hInst;                      // Owner of this class
	wc.hIcon         = LoadIcon(hInst, MAKEINTRESOURCE(IDI_LIST));
	wc.hCursor       = LoadCursor(NULL, IDC_ARROW);
	wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1); // Default color
	wc.lpszMenuName  = NULL;
	wc.lpszClassName = "ListWndClass";

	if (!RegisterClass((LPWNDCLASS)&wc))
		return 0;
	return 1;
}

//Creates the list window
HWND ListWindowCreate(HWND hwndMDIClient, HINSTANCE hInst)
{
	HWND  hwndChild;
	MDICREATESTRUCT mcs;

	mcs.szClass = "ListWndClass";      // window class name
	mcs.szTitle = "Recorded List";             // window title
	mcs.hOwner  = hInst;            // owner
	mcs.x       = 0;
	mcs.y       = 0;
	mcs.cx      = 300;    // width
	mcs.cy      = 450;    // height
	mcs.style   = 0;                // window style
	mcs.lParam  = 0;                // lparam

	hwndChild = (HWND) SendMessage(hwndMDIClient, WM_MDICREATE, 0, (LPARAM)(LPMDICREATESTRUCT) &mcs);

	if (hwndChild != NULL)
		ShowWindow(hwndChild, SW_SHOW);

	return hwndChild;
}

//This is the main handler for this window
LRESULT CALLBACK ChildWndListProc(HWND hwnd,UINT msg,WPARAM wparam,LPARAM lparam)
{
	DIRECTORY_INFO *lpDirectoryInfo;

	switch(msg) {
		case WM_CREATE:		//When we create the file we do certain things
			lpDirectoryInfo=malloc(sizeof(DIRECTORY_INFO));	//make the structure that holds browsed directory
			SetWindowLong(hwnd, GWL_USERDATA, (long)lpDirectoryInfo); //set the custom long of the window to remember it
			memset(lpDirectoryInfo, 0, sizeof(DIRECTORY_INFO));	//make sure everything set to zero

			//SetListDirectory(lpDirectoryInfo, NULL);	//Set the directory to the default, this will call another function that fills the linked list
			SetListDirectory(lpDirectoryInfo, "L:\\Record_Video\\");	//Set the directory to the default, this will call another function that fills the linked list

			break;
		case WM_PAINT:
			PaintListWindow(hwnd);
			break;
	}
	return DefMDIChildProc(hwnd, msg, wparam, lparam);
}


int PaintListWindow(HWND hwnd)
{
	DIRECTORY_INFO *lpDirectoryInfo;
	DIRECTORY_LIST *llist;

	HDC hdc;
	PAINTSTRUCT psPaint;
	char textoutbuffer[255];

	char filesizeString[255];
	char durationString[255];

	FILETIME filetime;
	SYSTEMTIME systemtime;

	int y=0;

	lpDirectoryInfo=(DIRECTORY_INFO *)GetWindowLong(hwnd, GWL_USERDATA);

	hdc = BeginPaint(hwnd, &psPaint);
	ExtTextOut(hdc, 0,y, ETO_OPAQUE, NULL, lpDirectoryInfo->directoryName, strlen(lpDirectoryInfo->directoryName), NULL);

	y=40;

	llist=lpDirectoryInfo->first;
	while (llist)	{
		ExtTextOut(hdc, 0,y, ETO_OPAQUE, NULL, llist->shortfilename, strlen(llist->shortfilename), NULL);
		y+=20;

		ExtTextOut(hdc, 0,y, ETO_OPAQUE, NULL, llist->recordingname, strlen(llist->recordingname), NULL);
		y+=20;


		//This helps to get a pretty date
		UnixTimeToFileTime(llist->unixtime, &filetime);
		FileTimeToSystemTime(&filetime, &systemtime);

		BytesDisplayNice(llist->filesize, "%.2f %s", 1, filesizeString);
		DurationShortFormatDHMS(llist->duration, durationString);

		sprintf(textoutbuffer, "%02d/%02d/%04d, Size:%s , %s", systemtime.wDay,systemtime.wMonth,systemtime.wYear , filesizeString, durationString);
		ExtTextOut(hdc, 0,y, ETO_OPAQUE, NULL, textoutbuffer, strlen(textoutbuffer), NULL);
		y+=20;

		llist=llist->next;
	}

	//SHBrowseForFolder

	EndPaint (hwnd, &psPaint);
	return 0;
}

int SetListDirectory(DIRECTORY_INFO *lpDirectoryInfo, char *directorypath)
{
	if (directorypath==NULL)
			GetCurrentDirectory(MAX_PATH, &(lpDirectoryInfo->directoryName[0]));	//start in the current directory
	else
		strcpy(&(lpDirectoryInfo->directoryName[0]), directorypath);

	strcpy(lpDirectoryInfo->directoryFilter, "????Z*");
	UpdateDirectoryList(lpDirectoryInfo);
	return 0;
}

int UpdateDirectoryList(DIRECTORY_INFO *lpDirectoryInfo)
{
	WIN32_FIND_DATA win32DirectoryResult;
	HANDLE currentFile;

	char fullFilter[MAX_PATH+1];
	int lenDN;

	lenDN=strlen(lpDirectoryInfo->directoryName);

	if (lenDN>0)	{	//trim any ending slash
		if ((lpDirectoryInfo->directoryName[lenDN-1] == '\\') || (lpDirectoryInfo->directoryName[lenDN-1] == '\\'))
			lpDirectoryInfo->directoryName[lenDN-1]=0;	//replace the slash with a null if it exists
	}

	fullFilter[0]=0;	//make sure this string is blank

	strcat(fullFilter, lpDirectoryInfo->directoryName);
	strcat(fullFilter, "\\");
	strcat(fullFilter, lpDirectoryInfo->directoryFilter);

	//First delete everything in the linked list so far
	DeleteDirectoryList(lpDirectoryInfo);

	currentFile = FindFirstFile(fullFilter, &win32DirectoryResult);

	if (currentFile == INVALID_HANDLE_VALUE)	{	//we have no files found
		lpDirectoryInfo->numberOfFiles=0;
		lpDirectoryInfo->first=NULL;
		lpDirectoryInfo->last=NULL;
		return 0;
	}

	//Add the first file to the list (if appropriate)
	CheckAndAddFileToList(lpDirectoryInfo, &win32DirectoryResult);

	while (FindNextFile(currentFile, &win32DirectoryResult))	{
		CheckAndAddFileToList(lpDirectoryInfo, &win32DirectoryResult);
	}

	return 0;
}

int CheckAndAddFileToList(DIRECTORY_INFO *lpDirectoryInfo, WIN32_FIND_DATA *fileToAdd)
{
	HANDLE infoFile;
	char fullFilePathAndName[MAX_PATH];
	char *stringPointer;

	DIRECTORY_LIST newEntry;

	//needed for filemanagement part
	long n;
	long magic;
	int dummyint;

	//First some simple tests to exclude them
	if (!(fileToAdd->nFileSizeLow==4096))
		return 0;
	if (strchr(fileToAdd->cFileName, '.'))
		return 0;
	if (!(strlen(fileToAdd->cFileName)==16))
		return 0;

	fullFilePathAndName[0]=0;

	strcat(fullFilePathAndName, lpDirectoryInfo->directoryName);
	strcat(fullFilePathAndName, "\\");
	strcat(fullFilePathAndName, fileToAdd->cFileName);

	//Now open up the file and  read some information
	//(I wonder if this FILE_FLAG_OPEN_NO_RECALL is useful if network gets set up)
	infoFile = CreateFile(fullFilePathAndName, GENERIC_READ, FILE_SHARE_READ, NULL,
				OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL | FILE_FLAG_RANDOM_ACCESS, NULL);

	ReadFile(infoFile, &magic, 4, &n, NULL);	//if this isn't 00080500 then should fail
	ReadFile(infoFile, &dummyint, 2, &n, NULL);
	ReadFile(infoFile, newEntry.recordingname, 120, &n, NULL);

	SetFilePointer(infoFile, 0x0294, NULL, FILE_BEGIN);
	ReadFile(infoFile, &newEntry.unixtime, 4, &n, NULL);

	SetFilePointer(infoFile, 0x02a0, NULL, FILE_BEGIN);
	ReadFile(infoFile, &newEntry.filesize, 4, &n, NULL);

	SetFilePointer(infoFile, 0x02a8, NULL, FILE_BEGIN);
	ReadFile(infoFile, &newEntry.duration, 4, &n, NULL);


	CloseHandle(infoFile);


	strcpy(newEntry.filename, fullFilePathAndName);

	stringPointer=strrchr(fullFilePathAndName, 92);
	if (stringPointer)	strncpy(newEntry.shortfilename, stringPointer+1, 16);
	else	strncpy(newEntry.shortfilename, fullFilePathAndName, 16);
	newEntry.shortfilename[16]=0;

	AddEntryCopyToList(lpDirectoryInfo, &newEntry);

	return 1;
}


//This mallocs a new entry, then adds it to the linked list
DIRECTORY_LIST* AddEntryCopyToList(DIRECTORY_INFO *lpDirectory, DIRECTORY_LIST *dirToAdd)
{
	DIRECTORY_LIST*	newEntry;

	newEntry=malloc(sizeof(DIRECTORY_LIST));
	memcpy(newEntry, dirToAdd, sizeof(DIRECTORY_LIST));	//copy the entry completely, we need to adjust next, prev


	if (lpDirectory->numberOfFiles==0)	{	//if the list is currently empty
		lpDirectory->first=newEntry;

		newEntry->prev=NULL;
	}
	else	{
		lpDirectory->last->next=newEntry;
		newEntry->prev=lpDirectory->last;
	}

	//This function adds to the end of the list, so these always be true.
	lpDirectory->last=newEntry;
	newEntry->index=lpDirectory->numberOfFiles;
	lpDirectory->numberOfFiles++;
	newEntry->next=NULL;

	return newEntry;
}


// This deletes and frees the whole linked list
void DeleteDirectoryList(DIRECTORY_INFO *lpDirectoryInfo)
{
	DIRECTORY_LIST*	activeEntry;
	DIRECTORY_LIST*	nextEntry;

	activeEntry=lpDirectoryInfo->last;

	while (activeEntry)	{
		nextEntry=activeEntry->prev;
		free(activeEntry);
		activeEntry=nextEntry;
	}

	lpDirectoryInfo->first=NULL;
	lpDirectoryInfo->last=NULL;
	lpDirectoryInfo->numberOfFiles=0;

	return;
}
