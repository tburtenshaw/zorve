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

	HDC hdc;
	PAINTSTRUCT psPaint;
	char textoutbuffer[255];

	lpDirectoryInfo=(DIRECTORY_INFO *)GetWindowLong(hwnd, GWL_USERDATA);

	hdc = BeginPaint(hwnd, &psPaint);
	ExtTextOut(hdc, 0,0, ETO_OPAQUE, NULL, &lpDirectoryInfo->directoryName[0], strlen(&lpDirectoryInfo->directoryName[0]), NULL);

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
	//DeleteDirectoryList(DIRECTORY_INFO *lpDirectoryInfo);

	currentFile = FindFirstFile(fullFilter, &win32DirectoryResult);

	if (currentFile == INVALID_HANDLE_VALUE)	{	//we have no files found
		lpDirectoryInfo->numberOfFiles=0;
		lpDirectoryInfo->first=NULL;
		lpDirectoryInfo->last=NULL;
		return 0;
	}

	//MessageBox(0,win32DirectoryResult.cFileName, fullFilter ,0);
	CheckAndAddFileToList(lpDirectoryInfo, &win32DirectoryResult);

	while (FindNextFile(currentFile, &win32DirectoryResult))	{
		if (CheckAndAddFileToList(lpDirectoryInfo, &win32DirectoryResult))
			1;
			//MessageBox(0,win32DirectoryResult.cFileName,"File",0);
	}

	return 0;
}

int CheckAndAddFileToList(DIRECTORY_INFO *lpDirectoryInfo, WIN32_FIND_DATA *fileToAdd)
{
	HANDLE infoFile;
	char fullFilePathAndName[MAX_PATH];

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
	infoFile = CreateFile(fullFilePathAndName, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING,FILE_FLAG_RANDOM_ACCESS, NULL);

	MessageBox(0, fullFilePathAndName, "File",0);

	CloseHandle(infoFile);
	return 1;
}
