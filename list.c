#include <stdio.h>
#include "zorve.h"
#include "zorveres.h"
#include "list.h"
#include "info.h"

//This registers the Window Class for the list files
int ListWindowRegisterWndClasses(HINSTANCE hInst)
{
	WNDCLASS wc;

	//First make the main list window class, this will contain two main children (for folder and files)
	//Wipe the wndclass for editing.
	memset(&wc,0,sizeof(WNDCLASS));

	wc.style         = 0;
	wc.lpfnWndProc   = (WNDPROC)ChildWndListProc;
	wc.cbClsExtra    = 0;
	wc.cbWndExtra    = 20;
	wc.hInstance     = hInst;                      // Owner of this class
	wc.hIcon         = LoadIcon(hInst, MAKEINTRESOURCE(IDI_LIST));
	wc.hCursor       = LoadCursor(NULL, IDC_ARROW);
	wc.hbrBackground = NULL;		//we draw the background, otherwise we get bad flicker
	wc.lpszMenuName  = NULL;
	wc.lpszClassName = "ListWndClass";

	if (!RegisterClass((LPWNDCLASS)&wc))
		return 0;


	wc.style         = 0;
	wc.lpfnWndProc   = (WNDPROC)ListChildFolderProc;
	wc.cbClsExtra    = 0;
	wc.cbWndExtra    = 20;
	wc.hInstance     = hInst;                      // Owner of this class
	wc.hIcon         = NULL;
	wc.hCursor       = LoadCursor(NULL, IDC_ARROW);
	wc.hbrBackground = NULL;		//we draw the background, otherwise we get bad flicker
	wc.lpszMenuName  = NULL;
	wc.lpszClassName = "ListChildFolderSelector";

	if (!RegisterClass((LPWNDCLASS)&wc))
		return 0;

	wc.style         = CS_DBLCLKS;
	wc.lpfnWndProc   = (WNDPROC)ListChildFileProc;
	wc.cbClsExtra    = 0;
	wc.cbWndExtra    = 20;
	wc.hInstance     = hInst;                      // Owner of this class
	wc.hIcon         = NULL;
	wc.hCursor       = LoadCursor(NULL, IDC_ARROW);
	wc.hbrBackground = NULL;		//we draw the background, otherwise we get bad flicker
	wc.lpszMenuName  = NULL;
	wc.lpszClassName = "ListChildFileSelector";

	if (!RegisterClass((LPWNDCLASS)&wc))
		return 0;


	return 1;
}

//Creates the list window
HWND ListWindowCreateOrShow(HWND currentListWindowHwnd, HWND hwndMDIClient, HINSTANCE hInst)
{
	HWND  hwndChild;
	MDICREATESTRUCT mcs;

	LISTWINDOW_INFO *lpListWindowInfo;

	if (IsWindow(currentListWindowHwnd))	{
		ShowWindow(currentListWindowHwnd, SW_SHOW);
		return currentListWindowHwnd;
	}


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
	LISTWINDOW_INFO *lpListWindowInfo;
	DIRECTORY_INFO *lpDirectoryInfo;
	HINSTANCE	hInst;

	HWND  childWndButton;
	HBITMAP folderIcon;
	LRESULT lResult;

	RECT clientRect;

	switch(msg) {
		case WM_CREATE:
			hInst=((LPCREATESTRUCT)lparam)->hInstance;

			//Still have my doubts about malloc'ing in a message - will need to check before using from other functions
			lpListWindowInfo=malloc(sizeof(LISTWINDOW_INFO));	//make the structure that holds child windows and folderlist
			SetWindowLong(hwnd, GWL_USERDATA, (long)lpListWindowInfo); //set the custom long of the window to remember it
			memset(lpListWindowInfo, 0, sizeof(LISTWINDOW_INFO));	//make sure everything set to zero

			lpListWindowInfo=(LISTWINDOW_INFO *)GetWindowLong(hwnd, GWL_USERDATA);

			lpDirectoryInfo=&lpListWindowInfo->directoryInfo;	//the directory pointer is set to the location in the windowinfostruct
			lpDirectoryInfo->parentListWindowInfo = lpListWindowInfo;
			SetListDirectory(lpDirectoryInfo, NULL);	//Set the directory to the default, this will call another function that fills the linked list
			//SetListDirectory(lpDirectoryInfo, "L:\\Record_Video\\");	//Set the directory to the default, this will call another function that fills the linked list

			lpListWindowInfo->heightFolderSelector=50;

			lpListWindowInfo->hwndFolder = CreateWindow("ListChildFolderSelector", "folderselect", WS_CHILD|WS_VISIBLE, 0,0,50,lpListWindowInfo->heightFolderSelector,hwnd, NULL, hInst, NULL);
			lpListWindowInfo->hwndFiles = CreateWindow("ListChildFileSelector", "fileselect", WS_CHILD|WS_VISIBLE|WS_VSCROLL, 0,lpListWindowInfo->heightFolderSelector,50,150,hwnd, NULL, hInst, NULL);
			break;

		case WM_SIZE:
			lpListWindowInfo=(LISTWINDOW_INFO *)GetWindowLong(hwnd, GWL_USERDATA);

			GetClientRect(hwnd, &clientRect);

			MoveWindow(lpListWindowInfo->hwndFolder, 0, 0, clientRect.right-clientRect.left,lpListWindowInfo->heightFolderSelector, TRUE);
			MoveWindow(lpListWindowInfo->hwndFiles,
							0, lpListWindowInfo->heightFolderSelector,
							clientRect.right-clientRect.left, clientRect.bottom-clientRect.top-lpListWindowInfo->heightFolderSelector, TRUE);
			break;
		case WM_DESTROY:
			lpListWindowInfo=(LISTWINDOW_INFO *)GetWindowLong(hwnd, GWL_USERDATA);
			DeleteDirectoryList(&lpListWindowInfo->directoryInfo);
			free(lpListWindowInfo);
			ZorveSetHwndList(NULL);

			break;

	}
	return DefMDIChildProc(hwnd, msg, wparam, lparam);
}

LRESULT CALLBACK ListChildFolderProc(HWND hwnd,UINT msg,WPARAM wparam,LPARAM lparam)
{
	LISTWINDOW_INFO *lpListWindowInfo;

	switch(msg) {
		case WM_PAINT:
			lpListWindowInfo=(LISTWINDOW_INFO *)GetWindowLong(GetParent(hwnd), GWL_USERDATA);
			PaintListFolderWindow(hwnd, lpListWindowInfo);
			break;
		case WM_ERASEBKGND:
			return 1;
			break;
	}
	return DefMDIChildProc(hwnd, msg, wparam, lparam);
}

LRESULT CALLBACK ListChildFileProc(HWND hwnd,UINT msg,WPARAM wparam,LPARAM lparam)
{
	LISTWINDOW_INFO *lpListWindowInfo;
	DIRECTORY_INFO *lpDirectoryInfo;
	DIRECTORY_LIST *llist;

	HWND hwndInfo;

	POINT point;

	switch(msg) {
		case WM_PAINT:
			lpListWindowInfo=(LISTWINDOW_INFO *)GetWindowLong(GetParent(hwnd), GWL_USERDATA);
			PaintListFileWindow(hwnd, lpListWindowInfo);
			break;
		case WM_ERASEBKGND:
			return 1;
			break;
		case WM_SIZE:
			InvalidateRect(hwnd, NULL, FALSE);
			break;
		case WM_KEYDOWN:
			if (wparam==VK_DOWN)	{
				lpListWindowInfo=(LISTWINDOW_INFO *)GetWindowLong(GetParent(hwnd), GWL_USERDATA);
				lpListWindowInfo->firstLine++;
				InvalidateRect(hwnd, NULL, FALSE);
			}
			break;
		case WM_LBUTTONDBLCLK:
			return 0;
			break;
		case WM_MOUSEWHEEL:
			return ListWindowOnMouseWheel(hwnd, (short)HIWORD(wparam));
		case WM_VSCROLL:
			ListWindowHandleVScroll(hwnd, wparam, lparam);
			break;
		case WM_LBUTTONDOWN:
		//case WM_LBUTTONDBLCLK:
			//xPos = GET_X_LPARAM(lparam);
			//yPos = GET_Y_LPARAM(lparam);
			point.x=GET_X_LPARAM(lparam);
			point.y=GET_Y_LPARAM(lparam);
			lpListWindowInfo=(LISTWINDOW_INFO *)GetWindowLong(GetParent(hwnd), GWL_USERDATA);
			lpDirectoryInfo=&lpListWindowInfo->directoryInfo;

			llist=lpDirectoryInfo->first;
			while (llist)	{
				if (PtInRect(&llist->boundingRect, point))	{
					hwndInfo = (HWND)ZorveGetHwndInfo();
					hwndInfo = InfoWindowCreateOrShow(hwndInfo, GetParent(GetParent(hwnd)), (HINSTANCE)GetWindowLong(hwnd, GWL_HINSTANCE));
					ZorveSetHwndInfo(hwndInfo);
					lpDirectoryInfo->parentListWindowInfo->selectedLine=llist->index;
					InvalidateRect(hwnd, &lpDirectoryInfo->parentListWindowInfo->oldBoundingRect, FALSE);
					InvalidateRect(hwnd, &llist->boundingRect, FALSE);


					InfoWindowLoadFile(hwndInfo, llist->filename);
				}

					//MessageBox(0,llist->filename,"3",0);

				llist=llist->next;
			}

			break;

	}
	return DefMDIChildProc(hwnd, msg, wparam, lparam);
}


int PaintListFolderWindow(HWND hwnd, LISTWINDOW_INFO *lpListWindowInfo)
{
	DIRECTORY_INFO *lpDirectoryInfo;
	char textbuffer[255];

	HDC hdc;
	PAINTSTRUCT psPaint;

	RECT clientRect;
	RECT textRect;

	HFONT hLargeFont;
	HFONT hSmallFont;

	lpDirectoryInfo=&lpListWindowInfo->directoryInfo;	//get the pointer to the directory info block

	hdc = BeginPaint(hwnd, &psPaint);
	GetClientRect(hwnd, &clientRect);

	SetBkColor(hdc, RGB_ZINNY_DARKBLUE);
	SetTextColor(hdc, RGB(255, 255, 255));
	hLargeFont = CreateFont(
				MulDiv(14, GetDeviceCaps(hdc, LOGPIXELSY), 72),
				0,0,0,FW_BOLD,
				FALSE,FALSE,FALSE,
				DEFAULT_CHARSET,OUT_OUTLINE_PRECIS,
                CLIP_DEFAULT_PRECIS,ANTIALIASED_QUALITY, VARIABLE_PITCH|FF_SWISS,"Arial");

	hSmallFont = CreateFont(
				MulDiv(8, GetDeviceCaps(hdc, LOGPIXELSY), 72),
				0,0,0,FW_BOLD,
				FALSE,FALSE,FALSE,
				DEFAULT_CHARSET,OUT_OUTLINE_PRECIS,
                CLIP_DEFAULT_PRECIS,NONANTIALIASED_QUALITY, VARIABLE_PITCH|FF_SWISS,"Arial");

	SelectObject(hdc,hLargeFont);


	textRect.top=0; textRect.bottom=30;
	textRect.left=0; textRect.right=clientRect.right;
	ExtTextOut(hdc, 5,5, ETO_OPAQUE, &textRect, lpDirectoryInfo->directoryName, strlen(lpDirectoryInfo->directoryName), NULL);

	SelectObject(hdc,hSmallFont);
	sprintf(textbuffer, "(%i file%s found)", lpDirectoryInfo->numberOfFiles, (lpDirectoryInfo->numberOfFiles==1)?"":"s");
	textRect.top=30; textRect.bottom=clientRect.bottom;
	textRect.left=0; textRect.right=clientRect.right;
	ExtTextOut(hdc, 5,30, ETO_OPAQUE, &textRect, textbuffer, strlen(textbuffer), NULL);

	DeleteObject(hSmallFont);
	DeleteObject(hLargeFont);

	EndPaint (hwnd, &psPaint);

	return 0;
}


int PaintListFileWindow(HWND hwnd, LISTWINDOW_INFO *lpListWindowInfo)
{
	DIRECTORY_INFO *lpDirectoryInfo;
	DIRECTORY_LIST *llist;

	RECT clientRect;
	RECT textRect;

	TEXTMETRIC textMetric;
	HFONT hLargeFont;
	HFONT hSmallFont;
	SIZE sizeTextExtent;

	int	heightLargeFont;
	int heightSmallFont;
	int	heightLine;

	HDC hdc;
	PAINTSTRUCT psPaint;
	char textoutbuffer[255];

	char filesizeString[255];
	char durationString[255];

	FILETIME filetime;
	SYSTEMTIME systemtime;

	SCROLLINFO scrollInfo;

	int smallcolumnwidth=80;
	int dividerlinewidth=2;
	int y=0;


	lpDirectoryInfo=&lpListWindowInfo->directoryInfo;

	hdc = BeginPaint(hwnd, &psPaint);
	GetClientRect(hwnd, &clientRect);

	//Load the fonts
	hLargeFont = CreateFont(
				MulDiv(13, GetDeviceCaps(hdc, LOGPIXELSY), 72),
				0,0,0,FW_BOLD,
				FALSE,FALSE,FALSE,
				DEFAULT_CHARSET,OUT_OUTLINE_PRECIS,
                CLIP_DEFAULT_PRECIS,ANTIALIASED_QUALITY, VARIABLE_PITCH|FF_SWISS,"Arial");

	hSmallFont = CreateFont(
				MulDiv(8, GetDeviceCaps(hdc, LOGPIXELSY), 72),
				0,0,0,FW_BOLD,
				FALSE,FALSE,FALSE,
				DEFAULT_CHARSET,OUT_OUTLINE_PRECIS,
                CLIP_DEFAULT_PRECIS,NONANTIALIASED_QUALITY, VARIABLE_PITCH|FF_SWISS,"Arial");


	SelectObject(hdc,hLargeFont);
	GetTextMetrics(hdc, &textMetric);
	heightLargeFont= textMetric.tmHeight;

	SelectObject(hdc,hSmallFont);
	GetTextMetrics(hdc, &textMetric);
	heightSmallFont= textMetric.tmHeight;

	if ((heightLargeFont)>(2*heightSmallFont))	//if the large font is much larger, then it controls line height
		heightLine=heightLargeFont/2;
	else
		heightLine=heightSmallFont;

	GetTextExtentPoint32(hdc, "WW/WW/WW", 8, &sizeTextExtent);
	smallcolumnwidth = sizeTextExtent.cx;


	//Start going through linked list, and display all the files
	llist=lpDirectoryInfo->first;

	//Skip up until the first thing to display
	while ((llist) && (llist->index<lpListWindowInfo->firstLine) )	{
		if ((llist->index < lpListWindowInfo->firstLine))
			llist->boundingRect.top=-1;llist->boundingRect.bottom=-1;
			llist->boundingRect.left=-1;llist->boundingRect.right=-1;
			llist=llist->next;
	}

	while ((llist)&&(y<clientRect.bottom))	{

		if (llist->index==lpListWindowInfo->selectedLine)	{
			SetTextColor(hdc, RGB_ZINNY_WHITE);
			SetBkColor(hdc, RGB_ZINNY_BRIGHTBLUE);
		} else	{
			SetTextColor(hdc, RGB_ZINNY_DARKBLUE);
			SetBkColor(hdc, RGB_ZINNY_MIDPURPLE);
		}


		//Draw the Recording Name, then the filename (on the left)
		SetTextAlign(hdc, TA_LEFT);
		SelectObject(hdc,hLargeFont);

		textRect.left=clientRect.left; textRect.right=clientRect.right-smallcolumnwidth;
		textRect.top=y+0;textRect.bottom=y+heightLargeFont;
		ExtTextOut(hdc, 5,textRect.top, ETO_OPAQUE, &textRect, llist->recordingname, strlen(llist->recordingname), NULL);

		SelectObject(hdc,hSmallFont);

		//textRect.top=y+2*heightLine;textRect.bottom=y+3*heightLine;
		textRect.top=y+heightLargeFont;textRect.bottom=y+3*heightLine;
		ExtTextOut(hdc, 5,textRect.top, ETO_OPAQUE, &textRect, llist->shortfilename, strlen(llist->shortfilename), NULL);


		SetTextAlign(hdc, TA_RIGHT);

		//This helps to get a pretty date
		UnixTimeToFileTime(llist->unixtime, &filetime);
		FileTimeToSystemTime(&filetime, &systemtime);
		textRect.top=y+0;textRect.bottom=y+1*heightLine;
		textRect.left=clientRect.right-smallcolumnwidth; textRect.right=clientRect.right;
		sprintf(textoutbuffer, "%02d/%02d/%04d",systemtime.wDay,systemtime.wMonth,systemtime.wYear);
		ExtTextOut(hdc, textRect.right-5,textRect.top, ETO_OPAQUE, &textRect, textoutbuffer, strlen(textoutbuffer), NULL);

		textRect.top=y+1*heightLine;textRect.bottom=y+2*heightLine;
		//24 hour time
		sprintf(textoutbuffer, "%02d:%02d",systemtime.wHour,systemtime.wMinute);
		//12 hour time
		sprintf(textoutbuffer, "%d:%02d%s",systemtime.wHour==0?12:(systemtime.wHour>12?systemtime.wHour-12:systemtime.wHour),systemtime.wMinute, systemtime.wHour>=12?"p":"a");

		ExtTextOut(hdc, textRect.right-5,textRect.top, ETO_OPAQUE, &textRect, textoutbuffer, strlen(textoutbuffer), NULL);


		DurationShortFormatDHMS(llist->duration, durationString);
	    sprintf(textoutbuffer, "%s", durationString);
		textRect.top=y+2*heightLine;textRect.bottom=y+3*heightLine;
		ExtTextOut(hdc, textRect.right-5,textRect.top, ETO_OPAQUE, &textRect, textoutbuffer, strlen(textoutbuffer), NULL);


		//We won't display the size, as if often not done correctly
		//BytesDisplayNice(llist->filesize, "%.2f %s", 1, filesizeString);


		SetBkColor(hdc, RGB_ZINNY_HIGHPURPLE);
		textRect.left=clientRect.left; textRect.right=clientRect.right;
		textRect.top=y+3*heightLine;textRect.bottom=y+3*heightLine+dividerlinewidth;
		ExtTextOut(hdc, 0,0, ETO_OPAQUE, &textRect, "", 0, NULL);	//this is the separator

		llist->boundingRect.left=clientRect.left;
		llist->boundingRect.right=clientRect.right;
		llist->boundingRect.top=y;
		llist->boundingRect.bottom=y+3*heightLine+dividerlinewidth;

		if (llist->index==lpListWindowInfo->selectedLine)	{	//if this is selected, remember its bounding box
			lpListWindowInfo->oldBoundingRect.left=llist->boundingRect.left;
			lpListWindowInfo->oldBoundingRect.right=llist->boundingRect.right;
			lpListWindowInfo->oldBoundingRect.top=llist->boundingRect.top;
			lpListWindowInfo->oldBoundingRect.bottom=llist->boundingRect.bottom;
		}



		y+=3*heightLine+dividerlinewidth;

		if (y<=clientRect.bottom)	//if this is true, then we have displayed a full line
			lpListWindowInfo->fullyDisplayedLines=llist->index - lpListWindowInfo->firstLine;


		llist=llist->next;
	}

	//Because we don't display these, we need to ensure their bounding rects are gone
	while (llist)	{
		llist->boundingRect.left=-1; llist->boundingRect.right=-1;
		llist->boundingRect.top=-1; llist->boundingRect.bottom=-1;
		llist=llist->next;
	}

//	if (y<clientRect.bottom)	//if we don't get to the end, work it out by dividing
		lpListWindowInfo->fullyDisplayedLines=(clientRect.bottom-clientRect.top) / (3*heightLine+dividerlinewidth);

	SetBkColor(hdc, RGB_ZINNY_MIDPURPLE);
	textRect.left=clientRect.left; textRect.right=clientRect.right;
	textRect.top=y; textRect.bottom=clientRect.bottom;
	ExtTextOut(hdc, 0,0, ETO_OPAQUE, &textRect, "", 0, NULL);	//this is the separator

	//SHBrowseForFolder

	DeleteObject(hSmallFont);
	DeleteObject(hLargeFont);
	EndPaint (hwnd, &psPaint);

	//Scrollbars
	scrollInfo.cbSize = sizeof(SCROLLINFO);
	scrollInfo.fMask = SIF_PAGE|SIF_POS|SIF_RANGE;
	scrollInfo.nPage=lpListWindowInfo->fullyDisplayedLines;
	scrollInfo.nMin=0;
	scrollInfo.nMax=lpDirectoryInfo->numberOfFiles-1;
	scrollInfo.nPos=lpListWindowInfo->firstLine;

	SetScrollInfo(hwnd, SB_VERT, &scrollInfo, TRUE);

	return 0;
}

int ListWindowHandleVScroll(HWND hwnd, WPARAM wparam, LPARAM lparam)
{
	LISTWINDOW_INFO *lpListWindowInfo;

	lpListWindowInfo=(LISTWINDOW_INFO *)GetWindowLong(GetParent(hwnd), GWL_USERDATA);

	short nSBCode;

	nSBCode=LOWORD(wparam);

	switch (nSBCode)	{
		case SB_TOP:
			lpListWindowInfo->firstLine=0;
			InvalidateRect(hwnd, NULL, FALSE);
			break;
		case SB_BOTTOM:
			lpListWindowInfo->firstLine = (lpListWindowInfo->directoryInfo.numberOfFiles - lpListWindowInfo->fullyDisplayedLines);
			InvalidateRect(hwnd, NULL, FALSE);
			break;
		case SB_LINEDOWN:
			if (lpListWindowInfo->firstLine < (lpListWindowInfo->directoryInfo.numberOfFiles - lpListWindowInfo->fullyDisplayedLines))
			lpListWindowInfo->firstLine++;
			InvalidateRect(hwnd, NULL, FALSE);
			break;
		case SB_LINEUP:
			if (lpListWindowInfo->firstLine>0)	{
				lpListWindowInfo->firstLine--;
				InvalidateRect(hwnd, NULL, FALSE);
			}
			break;
		case SB_PAGEDOWN:
			lpListWindowInfo->firstLine+=lpListWindowInfo->fullyDisplayedLines;
			if (lpListWindowInfo->firstLine > (lpListWindowInfo->directoryInfo.numberOfFiles - lpListWindowInfo->fullyDisplayedLines))
				lpListWindowInfo->firstLine = (lpListWindowInfo->directoryInfo.numberOfFiles - lpListWindowInfo->fullyDisplayedLines);
			InvalidateRect(hwnd, NULL, FALSE);
			break;
		case SB_PAGEUP:
			lpListWindowInfo->firstLine-=lpListWindowInfo->fullyDisplayedLines;
			if (lpListWindowInfo->firstLine <0)
				lpListWindowInfo->firstLine = 0;
			InvalidateRect(hwnd, NULL, FALSE);
			break;
		case SB_THUMBPOSITION:
		case SB_THUMBTRACK:
			lpListWindowInfo->firstLine = HIWORD(wparam);
			InvalidateRect(hwnd, NULL, FALSE);
			break;
	}

	return 0;
}

long ListWindowOnMouseWheel(HWND hwnd, short nDelta)
{
	LISTWINDOW_INFO *lpListWindowInfo;
	lpListWindowInfo=(LISTWINDOW_INFO *)GetWindowLong(GetParent(hwnd), GWL_USERDATA);

	int oldFirstLine;

	MessageBox(hwnd,"t","t",0);

	oldFirstLine = lpListWindowInfo->firstLine;

	if (nDelta<0) {
		if (lpListWindowInfo->firstLine < (lpListWindowInfo->directoryInfo.numberOfFiles - lpListWindowInfo->fullyDisplayedLines))	{
			lpListWindowInfo->firstLine++;
		}
	}
	if (nDelta>0) {
		if (lpListWindowInfo->firstLine>0)	{
			lpListWindowInfo->firstLine--;
		}
	}

	if (oldFirstLine!=lpListWindowInfo->firstLine)
		InvalidateRect(hwnd, NULL, FALSE);

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

	lpDirectoryInfo->parentListWindowInfo->firstLine=0;
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
	ListWindowReadFileDetails(&newEntry, fullFilePathAndName);

	strcpy(newEntry.filename, fullFilePathAndName);

	stringPointer=strrchr(fullFilePathAndName, 92);
	if (stringPointer)	strncpy(newEntry.shortfilename, stringPointer+1, 16);
	else	strncpy(newEntry.shortfilename, fullFilePathAndName, 16);
	newEntry.shortfilename[16]=0;

	AddEntryCopyToList(lpDirectoryInfo, &newEntry);

	return 1;
}

int ListWindowReadFileDetails(DIRECTORY_LIST* directoryItem, char* filename)
{
	HANDLE infoFile;

	long n;
	long magic;
	int dummyint;

	//(I wonder if this FILE_FLAG_OPEN_NO_RECALL is useful if network gets set up)
	infoFile = CreateFile(filename, GENERIC_READ, FILE_SHARE_READ, NULL,
				OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL | FILE_FLAG_RANDOM_ACCESS, NULL);

	ReadFile(infoFile, &magic, 4, &n, NULL);	//if this isn't 00080500 then should fail
	ReadFile(infoFile, &dummyint, 2, &n, NULL);
	ReadFile(infoFile, directoryItem->recordingname, 120, &n, NULL);

	SetFilePointer(infoFile, 0x0294, NULL, FILE_BEGIN);
	ReadFile(infoFile, &directoryItem->unixtime, 4, &n, NULL);

	SetFilePointer(infoFile, 0x02a0, NULL, FILE_BEGIN);
	ReadFile(infoFile, &directoryItem->filesize, 4, &n, NULL);

	SetFilePointer(infoFile, 0x02a8, NULL, FILE_BEGIN);
	ReadFile(infoFile, &directoryItem->duration, 4, &n, NULL);


	CloseHandle(infoFile);

	return 0;

}

DIRECTORY_LIST* ListWindowGetEntryFromFilename(LISTWINDOW_INFO* windowInfo, char* filename)
{
	DIRECTORY_LIST* llist;

	//Go throught the linked list
	llist=windowInfo->directoryInfo.first;

	while (llist)	{
		if (!strcmp(llist->filename, filename))
			return llist;
		llist=llist->next;
	}


	return NULL;
}

int RefreshAndOrSelectEntry(LISTWINDOW_INFO* windowInfo, DIRECTORY_LIST* entry, BOOL bRefresh, BOOL bSelect)
{
	if (bRefresh)
		ListWindowReadFileDetails(entry, entry->filename);
	if (bSelect)	{
		windowInfo->selectedLine=entry->index;
		if (entry->index > windowInfo->firstLine+windowInfo->fullyDisplayedLines-1)
			windowInfo->firstLine+=entry->index-(windowInfo->firstLine+windowInfo->fullyDisplayedLines)+1;
		if (entry->index < windowInfo->firstLine)
			windowInfo->firstLine=entry->index;

	}

	return 0;
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
