#include "stdio.h"
#include "zorveres.h"
#include "zorve.h"
#include "navfile.h"

//This registers the Window Class for the list files
int NavWindowRegisterWndClass(HINSTANCE hInst)
{
	WNDCLASS wc;

	//Wipe the wndclass for editing.
	memset(&wc,0,sizeof(WNDCLASS));

	wc.style         = 0;
	wc.lpfnWndProc   = (WNDPROC)ChildWndNavProc;
	wc.cbClsExtra    = 0;
	wc.cbWndExtra    = 20;
	wc.hInstance     = hInst;                      // Owner of this class
	wc.hIcon         = LoadIcon(hInst, MAKEINTRESOURCE(IDI_NAV));
	wc.hCursor       = LoadCursor(NULL, IDC_ARROW);
	wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1); // Default color
	wc.lpszMenuName  = NULL;
	wc.lpszClassName = "NavWndClass";

	if (!RegisterClass((LPWNDCLASS)&wc))
		return 0;


	wc.style         = CS_DBLCLKS;
	wc.lpfnWndProc   = (WNDPROC)NavRecordListViewFileProc;
	wc.cbClsExtra    = 0;
	wc.cbWndExtra    = 20;
	wc.hInstance     = hInst;                      // Owner of this class
	wc.hIcon         = NULL;
	wc.hCursor       = LoadCursor(NULL, IDC_ARROW);
	wc.hbrBackground = NULL;		//we draw the background, otherwise we get bad flicker
	wc.lpszMenuName  = NULL;
	wc.lpszClassName = "NavRecordListView";

	if (!RegisterClass((LPWNDCLASS)&wc))
		return 0;


	return 1;
}

//This is the main handler for this window
LRESULT CALLBACK ChildWndNavProc(HWND hwnd,UINT msg,WPARAM wparam,LPARAM lparam)
{
	NAVWINDOW_INFO *navWindowInfo;
	HINSTANCE hInst;
	RECT clientRect;

	switch(msg) {
		case WM_CREATE:
			hInst=((LPCREATESTRUCT)lparam)->hInstance;	//get the hinstance for use when creating the child window

			navWindowInfo=malloc(sizeof(NAVWINDOW_INFO));
			SetWindowLong(hwnd, GWL_USERDATA, (long)navWindowInfo); //set the custom long of the window to remember it
			memset(navWindowInfo, 0, sizeof(NAVWINDOW_INFO));	//make sure everything set to zero

			navWindowInfo->firstDisplayedRecord=0;
			navWindowInfo->indexRecordBuffer=-1;

			navWindowInfo->headerHeight=60;
			navWindowInfo->hwndRecordList = CreateWindow("NavRecordListView", "recordview", WS_CHILD|WS_VISIBLE|WS_VSCROLL, 5,navWindowInfo->headerHeight,340,250,hwnd, NULL, hInst, NULL);
			break;
		case WM_SIZE:
			navWindowInfo=(NAVWINDOW_INFO *)GetWindowLong(hwnd, GWL_USERDATA);

			GetClientRect(hwnd, &clientRect);

			MoveWindow(navWindowInfo->hwndRecordList, 5, navWindowInfo->headerHeight+5, clientRect.right-clientRect.left-10,clientRect.bottom-10-navWindowInfo->headerHeight, TRUE);
			break;
		case WM_PAINT:
			NavRecordHeaderPaint(hwnd);
			DefMDIChildProc(hwnd, msg, wparam, lparam);
			break;
		default:
			return DefMDIChildProc(hwnd, msg, wparam, lparam);
	}
	return 0;
}

HWND NavWindowCreateOrShow(HWND hwnd, HWND hwndChild, HINSTANCE hInst)
{
	if (IsWindow(hwnd))	{
		ShowWindow(hwnd, SW_SHOW);
		return hwnd;
	}

	return NavWindowCreate(hwndChild, hInst);
}

HWND NavWindowCreate(HWND hwndMDIClient, HINSTANCE hInst)
{
	HWND  hwndChild;
	MDICREATESTRUCT mcs;

	mcs.szClass = "NavWndClass";      // window class name
	mcs.szTitle = "MPEG Navigation/Timing";             // window title
	mcs.hOwner  = hInst;            // owner
	mcs.x       = 500;
	mcs.y       = 350;
	mcs.cx      = 490;    // width
	mcs.cy      = 320;    // height
	mcs.style   = WS_CLIPCHILDREN|WS_CHILD;                // window style
	mcs.lParam  = 0;                // lparam

	hwndChild = (HWND) SendMessage(hwndMDIClient, WM_MDICREATE, 0, (LPARAM)(LPMDICREATESTRUCT) &mcs);

	if (hwndChild != NULL)
		ShowWindow(hwndChild, SW_SHOW);

	return hwndChild;
}

//This is the listview-type box displaying the records
//It handles most of the thinking.
LRESULT CALLBACK NavRecordListViewFileProc(HWND hwnd,UINT msg, WPARAM wparam,LPARAM lparam)
{
	switch(msg) {
		case WM_PAINT:
			NavRecordListViewPaint(hwnd);
			break;
		case WM_VSCROLL:
			NavHandleVScroll(hwnd, wparam, lparam);
			break;
		default:
			return DefMDIChildProc(hwnd, msg, wparam, lparam);
	}

	return 0;
}

int NavRecordListViewPaint(HWND hwnd)
{
	NAVWINDOW_INFO *navWindowInfo;

	HDC	hdc;
	PAINTSTRUCT ps;
	RECT clientRect;
	RECT outputRect;

	TEXTMETRIC textMetric;
	int heightFont;

	int numberOfLinesToDraw;
	char buffer[255];

	int y;
	int i;

	navWindowInfo=(NAVWINDOW_INFO *)GetWindowLong(GetParent(hwnd), GWL_USERDATA);	//get the point to window info


	GetClientRect(hwnd, &clientRect);
	hdc=BeginPaint(hwnd, &ps);

	GetTextMetrics(hdc, &textMetric);
	heightFont= textMetric.tmHeight;

	y=0;

	navWindowInfo->numDisplayedLines=(clientRect.bottom-clientRect.top)/(heightFont+1);

	numberOfLinesToDraw=MIN(navWindowInfo->numDisplayedLines+1, navWindowInfo->fileInfo.numRecords - navWindowInfo->firstDisplayedRecord);

	for (i=0;i<numberOfLinesToDraw;i++)	{
	sprintf(buffer, "%i (0x%x), s1:%u l6:%u l7:%u   ",navWindowInfo->firstDisplayedRecord+i, (navWindowInfo->firstDisplayedRecord+i)*sizeof(NAV_RECORD),
    	navWindowInfo->displayRecord[navWindowInfo->firstDisplayedRecord-navWindowInfo->indexRecordBuffer+i].s1,
    	navWindowInfo->displayRecord[navWindowInfo->firstDisplayedRecord-navWindowInfo->indexRecordBuffer+i].l6,
    	navWindowInfo->displayRecord[navWindowInfo->firstDisplayedRecord-navWindowInfo->indexRecordBuffer+i].l7
		);
	SetBkColor(hdc, RGB_ZINNY_WHITE);
	outputRect.left=clientRect.left;
	outputRect.right=clientRect.right;
	outputRect.top=y;
	outputRect.bottom=y+heightFont;
	ExtTextOut(hdc, 0,y,ETO_OPAQUE, &outputRect, buffer, strlen(buffer), NULL);
	y+=heightFont;


	outputRect.left=clientRect.left;
	outputRect.right=clientRect.right;
	outputRect.top=y;
	outputRect.bottom=y+1;
	SetBkColor(hdc, RGB_ZINNY_BLACK);
	ExtTextOut(hdc, 0,y,ETO_OPAQUE, &outputRect, NULL, 0, NULL);
	y++;
	}

	if (y<clientRect.bottom)	{		//if we still have some space left at end of list
		SetBkColor(hdc, RGB_ZINNY_WHITE);
		outputRect.left=clientRect.left;
		outputRect.right=clientRect.right;
		outputRect.top=y;
		outputRect.bottom=clientRect.bottom;
		ExtTextOut(hdc, 0,y,ETO_OPAQUE, &outputRect, NULL, 0, NULL);
	}


	EndPaint(hwnd, &ps);

	return 0;
}

int NavRecordHeaderPaint(HWND hwnd)
{
	HDC	hdc;
	PAINTSTRUCT ps;
	RECT clientRect;

	NAVWINDOW_INFO *navWindowInfo;
	char buffer[255];

	navWindowInfo=(NAVWINDOW_INFO *)GetWindowLong(hwnd, GWL_USERDATA);


	GetClientRect(hwnd, &clientRect);
	hdc=BeginPaint(hwnd, &ps);

	ExtTextOut(hdc,	0,0,ETO_OPAQUE, &clientRect, navWindowInfo->fileInfo.filename,strlen(navWindowInfo->fileInfo.filename), NULL);


	EndPaint(hwnd, &ps);


	return 0;
}


long UpdateBuffer(NAVWINDOW_INFO *navWindowInfo)
{
	long lineToCalcBuffer;
	long oldBufferOffset;
	long appropriateBuffer;

	//We want some overlap
	lineToCalcBuffer = navWindowInfo->firstDisplayedRecord;
	if (lineToCalcBuffer>=0)	lineToCalcBuffer-=0;

	oldBufferOffset=navWindowInfo->indexRecordBuffer;
	appropriateBuffer = (lineToCalcBuffer & 0xFFFFFE00);	//round to nearest 512

	if (oldBufferOffset==appropriateBuffer)	//we've got the right buffer
		return 0;

	if (appropriateBuffer==oldBufferOffset+512)	{//we need the buffer to be one 512block further down
		//copy the last 512 (of 1024) bytes to where they should be (i can't see how caching could be faster than this)
		memcpy(&navWindowInfo->displayRecord[0], &navWindowInfo->displayRecord[512], 512*sizeof(NAV_RECORD));
		//load the records for the last 512 from file
		ReadRecordsFromNavFile(navWindowInfo, appropriateBuffer+512, &navWindowInfo->displayRecord[512], 512);
		navWindowInfo->indexRecordBuffer=appropriateBuffer;
		return 0;
	}

	//If the oldBuffer is halfway too far down
	if (appropriateBuffer+512==oldBufferOffset)	{
		memcpy(&navWindowInfo->displayRecord[512], &navWindowInfo->displayRecord[0], 512*sizeof(NAV_RECORD));
		ReadRecordsFromNavFile(navWindowInfo, appropriateBuffer, &navWindowInfo->displayRecord[0], 512);
		navWindowInfo->indexRecordBuffer=appropriateBuffer;
		return 0;
	}

	//If the old buffer has nothing in common to the new one
	ReadRecordsFromNavFile(navWindowInfo, appropriateBuffer, &navWindowInfo->displayRecord[0], 1024);
	navWindowInfo->indexRecordBuffer=appropriateBuffer;

	return 0;
}

int ReadRecordsFromNavFile(NAVWINDOW_INFO *navWindowInfo, unsigned long offset, NAV_RECORD* ptrRecord, long numRecords)
{
	long n;

	//if ((offset+numRecords)*sizeof(NAV_RECORD) >= navWindowInfo->fileInfo.numRecords)
	//	return 0;

	SetFilePointer(navWindowInfo->fileInfo.hNavFile, offset*sizeof(NAV_RECORD), NULL, FILE_BEGIN);
	ReadFile(navWindowInfo->fileInfo.hNavFile, ptrRecord, sizeof(NAV_RECORD)*numRecords, &n, NULL);

	return 1;
}

int NavHandleVScroll(HWND hwnd, WPARAM wparam, LPARAM lparam)
{
	NAVWINDOW_INFO *navWindowInfo;
	short nSBCode;

	SCROLLINFO si;


	nSBCode=LOWORD(wparam);
	navWindowInfo=(NAVWINDOW_INFO *)GetWindowLong(GetParent(hwnd), GWL_USERDATA);

	switch (nSBCode)	{
		case SB_TOP:
			navWindowInfo->firstDisplayedRecord=0;
			break;
		case SB_BOTTOM:
		case SB_LINEDOWN:
 			navWindowInfo->firstDisplayedRecord++;
			break;
		case SB_LINEUP:
			navWindowInfo->firstDisplayedRecord--;
			break;
		case SB_PAGEDOWN:
 			navWindowInfo->firstDisplayedRecord+=navWindowInfo->numDisplayedLines;
			break;
		case SB_PAGEUP:
			navWindowInfo->firstDisplayedRecord-=navWindowInfo->numDisplayedLines;
			break;
		case SB_THUMBPOSITION:
		case SB_THUMBTRACK:

            ZeroMemory(&si, sizeof(si));
            si.cbSize = sizeof(si);
            si.fMask = SIF_TRACKPOS;
			GetScrollInfo(hwnd, SB_VERT, &si);

			//Need to do this differently for precision >65535
			navWindowInfo->firstDisplayedRecord = si.nTrackPos;
			break;
		}

	if (navWindowInfo->firstDisplayedRecord + navWindowInfo->numDisplayedLines > navWindowInfo->fileInfo.numRecords)
		navWindowInfo->firstDisplayedRecord=navWindowInfo->fileInfo.numRecords - navWindowInfo->numDisplayedLines;

	if (navWindowInfo->firstDisplayedRecord < 0)
		navWindowInfo->firstDisplayedRecord =0;


	UpdateBuffer(navWindowInfo);

	NavScrollUpdate(hwnd, navWindowInfo);


	InvalidateRect(hwnd, NULL, FALSE);

	return 0;
}

int NavScrollUpdate(HWND hwnd, NAVWINDOW_INFO * navWindowInfo)
{
	SCROLLINFO scrollInfo;

	scrollInfo.cbSize = sizeof(SCROLLINFO);
	scrollInfo.fMask = SIF_PAGE|SIF_POS|SIF_RANGE;
	scrollInfo.nPage=navWindowInfo->numDisplayedLines;
	scrollInfo.nMin=0;
	scrollInfo.nMax=navWindowInfo->fileInfo.numRecords-1;
	scrollInfo.nPos=navWindowInfo->firstDisplayedRecord;

	SetScrollInfo(hwnd, SB_VERT, &scrollInfo, TRUE);


	return 0;
}

int NavWindowLoadFile(HWND hwnd, char *openfilename)
{
	NAVWINDOW_INFO *navWindowInfo;

	navWindowInfo=(NAVWINDOW_INFO *)GetWindowLong(hwnd, GWL_USERDATA);

	strcpy(navWindowInfo->fileInfo.filename, openfilename);
	navWindowInfo->fileInfo.hNavFile = CreateFile(openfilename, GENERIC_READ, FILE_SHARE_READ, NULL,
				OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL | FILE_FLAG_RANDOM_ACCESS, NULL);


	navWindowInfo->fileInfo.filesize=GetFileSize(navWindowInfo->fileInfo.hNavFile,NULL);
	navWindowInfo->fileInfo.numRecords=navWindowInfo->fileInfo.filesize/sizeof(NAV_RECORD);

	//These should be set up before loading.
	UpdateBuffer(navWindowInfo);
	return 0;

}
