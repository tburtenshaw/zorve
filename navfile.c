#include <stdio.h>
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

	wc.style         = CS_DBLCLKS;
	wc.lpfnWndProc   = (WNDPROC)NavRecordListViewHeaderProc;
	wc.cbClsExtra    = 0;
	wc.cbWndExtra    = 20;
	wc.hInstance     = hInst;                      // Owner of this class
	wc.hIcon         = NULL;
	wc.hCursor       = LoadCursor(NULL, IDC_ARROW);
	wc.hbrBackground = NULL;		//we draw the background, otherwise we get bad flicker
	wc.lpszMenuName  = NULL;
	wc.lpszClassName = "NavRecordListViewHeader";

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

			navWindowInfo->headerHeight=40;
			navWindowInfo->listviewheaderHeight=20;
			navWindowInfo->hwndRecordList = CreateWindow("NavRecordListView", "recordview", WS_CHILD|WS_VISIBLE|WS_VSCROLL, 5,navWindowInfo->headerHeight,340,250,hwnd, NULL, hInst, NULL);
			navWindowInfo->hwndRecordListHeader = CreateWindow("NavRecordListViewHeader", "recordviewheader", WS_CHILD|WS_VISIBLE, 5,navWindowInfo->headerHeight,0,0,hwnd, NULL, hInst, NULL);

			NavInitiateColumnWidths(navWindowInfo);
			break;
		case WM_SIZE:
			navWindowInfo=(NAVWINDOW_INFO *)GetWindowLong(hwnd, GWL_USERDATA);
			GetClientRect(hwnd, &clientRect);
			MoveWindow(navWindowInfo->hwndRecordList, 5, navWindowInfo->headerHeight+navWindowInfo->listviewheaderHeight+5, clientRect.right-clientRect.left-10,clientRect.bottom-10-navWindowInfo->headerHeight-navWindowInfo->listviewheaderHeight, TRUE);
			MoveWindow(navWindowInfo->hwndRecordListHeader, 5, navWindowInfo->headerHeight+5, clientRect.right-clientRect.left-10,navWindowInfo->listviewheaderHeight, TRUE);
			InvalidateRect(hwnd, NULL, FALSE);
			break;
		case WM_PAINT:
			NavRecordHeaderPaint(hwnd);
			DefMDIChildProc(hwnd, msg, wparam, lparam);
			break;
		case WM_ERASEBKGND:
			return 1;
			break;
		case WM_MOUSEWHEEL:
			return NavWindowOnMouseWheel(hwnd, (short)HIWORD(wparam));
			break;
		case WM_DESTROY:
			NavWindowUnloadFile(hwnd);
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
	NAVWINDOW_INFO *navWindowInfo;

	switch(msg) {
		case WM_PAINT:
			NavRecordListViewPaint(hwnd);
			navWindowInfo=(NAVWINDOW_INFO *)GetWindowLong(GetParent(hwnd), GWL_USERDATA);	//get the point to window info
			NavScrollUpdate(hwnd, navWindowInfo);
			break;
		case WM_ERASEBKGND:
			return 1;
			break;
		case WM_VSCROLL:
			NavHandleVScroll(hwnd, wparam, lparam);
			break;
		case WM_SIZE:
			navWindowInfo=(NAVWINDOW_INFO *)GetWindowLong(GetParent(hwnd), GWL_USERDATA);	//get the point to window info
			NavScrollUpdate(hwnd, navWindowInfo);
			if (navWindowInfo->firstDisplayedRecord + navWindowInfo->numDisplayedLines > navWindowInfo->fileInfo.numRecords)	{
				navWindowInfo->firstDisplayedRecord=navWindowInfo->fileInfo.numRecords - navWindowInfo->numDisplayedLines;
				NavScrollUpdate(hwnd, navWindowInfo);
				InvalidateRect(hwnd, NULL, FALSE);
			}
		break;

		default:
			return DefMDIChildProc(hwnd, msg, wparam, lparam);
	}

	return 0;
}

//This displays the header, and adjusts the size of the listview header
LRESULT CALLBACK NavRecordListViewHeaderProc(HWND hwnd,UINT msg, WPARAM wparam,LPARAM lparam)
{
	NAVWINDOW_INFO *navWindowInfo;

	int newSelectedColumn;
	int c;
	int x;

	switch(msg) {
		case WM_PAINT:
			NavRecordListHeaderPaint(hwnd);
			break;
		case WM_ERASEBKGND:
			return 1;
			break;
		case WM_MOUSEMOVE:
			navWindowInfo=(NAVWINDOW_INFO *)GetWindowLong(GetParent(hwnd), GWL_USERDATA);

			if (navWindowInfo->mousePushed)
				navWindowInfo->mouseDragging=1;

			if	(!navWindowInfo->mouseDragging)	{
				newSelectedColumn = NavRecordListHeaderCheckAdjustBar(navWindowInfo, GET_X_LPARAM(lparam), GET_Y_LPARAM(lparam));

				if (newSelectedColumn!=navWindowInfo->movingColumn){
					navWindowInfo->movingColumn=newSelectedColumn;
				}
					if	(newSelectedColumn>-1)
						SetCursor(LoadCursor(NULL, IDC_SIZEWE));
					else
						SetCursor(LoadCursor(NULL, IDC_ARROW));
			}	else	{	//if dragging
				x=0;
				for (c=0; c<navWindowInfo->movingColumn; c++)	{	//get the cumulative position
					x+=navWindowInfo->widthColumn[c];
				}
					navWindowInfo->widthColumn[navWindowInfo->movingColumn]=MAX(GET_X_LPARAM(lparam)-x, 0);
					InvalidateRect(navWindowInfo->hwndRecordList, NULL, FALSE);
					InvalidateRect(navWindowInfo->hwndRecordListHeader, NULL, FALSE);
			}

			break;
		case WM_LBUTTONDOWN:
			navWindowInfo=(NAVWINDOW_INFO *)GetWindowLong(GetParent(hwnd), GWL_USERDATA);
			navWindowInfo->movingColumn=NavRecordListHeaderCheckAdjustBar(navWindowInfo, GET_X_LPARAM(lparam), GET_Y_LPARAM(lparam));
			if (navWindowInfo->movingColumn>-1)	{
				SetCapture(hwnd);
				navWindowInfo->mousePushed=1;
				SetCursor(LoadCursor(NULL, IDC_SIZEWE));
			}

			break;
		case WM_LBUTTONUP:
			navWindowInfo=(NAVWINDOW_INFO *)GetWindowLong(GetParent(hwnd), GWL_USERDATA);
			if (navWindowInfo->mousePushed)	{
				ReleaseCapture();
				navWindowInfo->mousePushed=0;
				navWindowInfo->mouseDragging=0;
			}
			break;
		case WM_CONTEXTMENU:
			NavWindowListHeaderHandleContextMenu(hwnd, wparam, lparam);
			break;
		case WM_COMMAND:
			c= LOWORD(wparam);

			if (c==IDM_LISTVIEWHEADERDEFAULT)	{
				navWindowInfo=(NAVWINDOW_INFO *)GetWindowLong(GetParent(hwnd), GWL_USERDATA);
				NavInitiateColumnWidths(navWindowInfo);
				InvalidateRect(navWindowInfo->hwndRecordList, NULL, FALSE);
				InvalidateRect(navWindowInfo->hwndRecordListHeader, NULL, FALSE);
			}

			if ((c>=100) && (c<121))	{
				navWindowInfo=(NAVWINDOW_INFO *)GetWindowLong(GetParent(hwnd), GWL_USERDATA);
				c-=100;

				if (navWindowInfo->widthColumn[c])
					navWindowInfo->widthColumn[c]=0;
				else
					navWindowInfo->widthColumn[c]=80;

				InvalidateRect(navWindowInfo->hwndRecordList, NULL, FALSE);
				InvalidateRect(navWindowInfo->hwndRecordListHeader, NULL, FALSE);
			}

		default:
			return DefMDIChildProc(hwnd, msg, wparam, lparam);
	}

	return 0;
}


int NavRecordListHeaderCheckAdjustBar(NAVWINDOW_INFO * navWindowInfo, int x, int y)
{
	int c;

	int columnx=0;

	for (c=0;c<21;c++)	{
		columnx+=navWindowInfo->widthColumn[c];
		if ((x>columnx-4) && (x<columnx+4))
			return c;

	}


//Returns the row to adjust, or -1 if none
	return -1;
}

int	NavRecordListHeaderPaint(HWND hwnd)
{
	NAVWINDOW_INFO *navWindowInfo;

	HDC	hdc;
	PAINTSTRUCT ps;
	RECT clientRect;
	RECT outputRect;
	RECT controlRect;

	HFONT hSmallFont;
	TEXTMETRIC textMetric;

	int c;

	int	x,y;
	char buffer[255];

	navWindowInfo=(NAVWINDOW_INFO *)GetWindowLong(GetParent(hwnd), GWL_USERDATA);	//get the point to window info

	GetClientRect(hwnd, &clientRect);
	hdc=BeginPaint(hwnd, &ps);

	SetBkColor(hdc, RGB_ZINNY_DARKBLUE);
	SetTextColor(hdc, RGB_ZINNY_WHITE);

	hSmallFont = CreateFont(
				MulDiv(10, GetDeviceCaps(hdc, LOGPIXELSY), 72),
				0,0,0,FW_NORMAL,
				FALSE,FALSE,FALSE,
				DEFAULT_CHARSET,OUT_OUTLINE_PRECIS,
                CLIP_DEFAULT_PRECIS,NONANTIALIASED_QUALITY, VARIABLE_PITCH|FF_SWISS,"MS Shell Dlg");
	SelectObject(hdc,hSmallFont);


	GetTextMetrics(hdc, &textMetric);
	//heightFont= textMetric.tmHeight;


	y=0;
	x=clientRect.left;

	outputRect.top=clientRect.top;
	outputRect.bottom=clientRect.bottom;
	outputRect.left=clientRect.left;
	outputRect.right=clientRect.left + navWindowInfo->widthColumn[0];

	for (c=0; c<21; c++)	{

		if (navWindowInfo->widthColumn[c]>0)	{
			//margin of two for control
			ExtTextOut(hdc, x+2,y,ETO_OPAQUE, &outputRect, navWindowInfo->columnTitle[c], strlen(navWindowInfo->columnTitle[c]), NULL);
			x+=navWindowInfo->widthColumn[c];
		}
			outputRect.left=x+1;
			outputRect.right=x+navWindowInfo->widthColumn[c+1];

			SetBkColor(hdc, RGB_ZINNY_MIDPURPLE);
			controlRect.left=x;
			controlRect.right=x+1;
			controlRect.top=clientRect.top;
			controlRect.bottom=clientRect.bottom;


			ExtTextOut(hdc,0,0,ETO_OPAQUE, &controlRect, NULL, 0,NULL);
			SetBkColor(hdc, RGB_ZINNY_DARKBLUE);
	}

	//Fill up to right margin
	outputRect.right=clientRect.right;

	ExtTextOut(hdc, x,y,ETO_OPAQUE, &outputRect, NULL, 0, NULL);

	DeleteObject(hSmallFont);
	EndPaint(hwnd, &ps);

	return 0;
}

int NavRecordListViewPaint(HWND hwnd)
{
	NAVWINDOW_INFO *navWindowInfo;

	HDC	hdc;
	PAINTSTRUCT ps;
	RECT clientRect;
	RECT outputRect;

	HFONT hSmallFont;
	TEXTMETRIC textMetric;
	int heightFont;

	int numberOfLinesToDraw;
	char buffer[255];

	int y;
	int i;	//row counter
	int c;	//column counter

	int columnStart=0;

	navWindowInfo=(NAVWINDOW_INFO *)GetWindowLong(GetParent(hwnd), GWL_USERDATA);	//get the point to window info

	GetClientRect(hwnd, &clientRect);
	hdc=BeginPaint(hwnd, &ps);

	hSmallFont = CreateFont(
				MulDiv(10, GetDeviceCaps(hdc, LOGPIXELSY), 72),
				0,0,0,FW_NORMAL,
				FALSE,FALSE,FALSE,
				DEFAULT_CHARSET,OUT_OUTLINE_PRECIS,
                CLIP_DEFAULT_PRECIS,NONANTIALIASED_QUALITY, VARIABLE_PITCH|FF_SWISS,"MS Shell Dlg");
	SelectObject(hdc,hSmallFont);


	GetTextMetrics(hdc, &textMetric);
	heightFont= textMetric.tmHeight;

	y=0;

	navWindowInfo->numDisplayedLines=(clientRect.bottom-clientRect.top)/(heightFont+1);

	numberOfLinesToDraw=MIN(navWindowInfo->numDisplayedLines+1, navWindowInfo->fileInfo.numRecords - navWindowInfo->firstDisplayedRecord);

	SetTextColor(hdc, RGB_ZINNY_DARKBLUE);
	for (i=0;i<numberOfLinesToDraw;i++)	{

		SetBkColor(hdc, RGB_ZINNY_MIDPURPLE);
		outputRect.top=y;
		outputRect.bottom=y+heightFont;
		columnStart=0;

		for (c=0;c<21;c++)	{
			outputRect.left=columnStart;
			outputRect.right=columnStart+navWindowInfo->widthColumn[c];

			if	(navWindowInfo->widthColumn[c] > 0)	{
				switch (c)	{
					case 0:
						sprintf(buffer, "%i (0x%x)",navWindowInfo->firstDisplayedRecord+i, (navWindowInfo->firstDisplayedRecord+i)*sizeof(NAV_RECORD));
						break;
					case 1:
						sprintf(buffer, "%u", navWindowInfo->displayRecord[navWindowInfo->firstDisplayedRecord-navWindowInfo->indexRecordBuffer+i].s0);
						break;
					case 2:
						sprintf(buffer, "%u", navWindowInfo->displayRecord[navWindowInfo->firstDisplayedRecord-navWindowInfo->indexRecordBuffer+i].twobytes2 & 0xFF);
						break;
					case 3:
						sprintf(buffer, "%u", navWindowInfo->displayRecord[navWindowInfo->firstDisplayedRecord-navWindowInfo->indexRecordBuffer+i].twobytes2 >> 8);
						break;
					case 4:
						sprintf(buffer, "%u", navWindowInfo->displayRecord[navWindowInfo->firstDisplayedRecord-navWindowInfo->indexRecordBuffer+i].twobytes4 & 0xFF);
						break;
					case 5:
						sprintf(buffer, "%u", navWindowInfo->displayRecord[navWindowInfo->firstDisplayedRecord-navWindowInfo->indexRecordBuffer+i].twobytes4 >> 8);
						break;
					case 6:
						sprintf(buffer, "%u", navWindowInfo->displayRecord[navWindowInfo->firstDisplayedRecord-navWindowInfo->indexRecordBuffer+i].s6);
						break;
					case 7:
						UnsignedLongLongToString((ULONGLONG)navWindowInfo->displayRecord[navWindowInfo->firstDisplayedRecord-navWindowInfo->indexRecordBuffer+i].offsetlo +
					   							   (ULONGLONG)navWindowInfo->displayRecord[navWindowInfo->firstDisplayedRecord-navWindowInfo->indexRecordBuffer+i].offsethi * (ULONGLONG)0x100000000,
												  	 buffer);
						//sprintf(buffer, "%u %u", navWindowInfo->displayRecord[navWindowInfo->firstDisplayedRecord-navWindowInfo->indexRecordBuffer+i].offsetlo,navWindowInfo->displayRecord[navWindowInfo->firstDisplayedRecord-navWindowInfo->indexRecordBuffer+i].offsethi);
						break;
					case 8:
						//The time stamp is the 300x the number of 1/27000000ths, Olevia stores this as a 32 bit value (half the original)
						sprintf(buffer, "%u", navWindowInfo->displayRecord[navWindowInfo->firstDisplayedRecord-navWindowInfo->indexRecordBuffer+i].timer);
						break;
					case 9:
						sprintf(buffer, "%u", navWindowInfo->displayRecord[navWindowInfo->firstDisplayedRecord-navWindowInfo->indexRecordBuffer+i].s14);
						break;
					case 10:
						sprintf(buffer, "%u", navWindowInfo->displayRecord[navWindowInfo->firstDisplayedRecord-navWindowInfo->indexRecordBuffer+i].s16);
						break;
					case 11:
						sprintf(buffer, "%u", navWindowInfo->displayRecord[navWindowInfo->firstDisplayedRecord-navWindowInfo->indexRecordBuffer+i].milliseconds);
						break;
					case 12:
						sprintf(buffer, "%u", navWindowInfo->displayRecord[navWindowInfo->firstDisplayedRecord-navWindowInfo->indexRecordBuffer+i].l1Czero);
						break;
					case 13:
						sprintf(buffer, "%u", navWindowInfo->displayRecord[navWindowInfo->firstDisplayedRecord-navWindowInfo->indexRecordBuffer+i].l20);
						break;
					case 14:
						sprintf(buffer, "%u", navWindowInfo->displayRecord[navWindowInfo->firstDisplayedRecord-navWindowInfo->indexRecordBuffer+i].s24);
						break;
					case 15:
						sprintf(buffer, "%u", navWindowInfo->displayRecord[navWindowInfo->firstDisplayedRecord-navWindowInfo->indexRecordBuffer+i].s26zero);
						break;
					case 16:
						sprintf(buffer, "%u", navWindowInfo->displayRecord[navWindowInfo->firstDisplayedRecord-navWindowInfo->indexRecordBuffer+i].l28zero);
						break;
					case 17:
						sprintf(buffer, "%u", navWindowInfo->displayRecord[navWindowInfo->firstDisplayedRecord-navWindowInfo->indexRecordBuffer+i].l2Czero);
						break;
					case 18:
						sprintf(buffer, "%u", navWindowInfo->displayRecord[navWindowInfo->firstDisplayedRecord-navWindowInfo->indexRecordBuffer+i].l30zero);
						break;
					case 19:
						sprintf(buffer, "%u", navWindowInfo->displayRecord[navWindowInfo->firstDisplayedRecord-navWindowInfo->indexRecordBuffer+i].l34zero);
						break;
					case 20:
						sprintf(buffer, "%u", navWindowInfo->displayRecord[navWindowInfo->firstDisplayedRecord-navWindowInfo->indexRecordBuffer+i].l38zero);
						break;
					case 21:
						sprintf(buffer, "%u", navWindowInfo->displayRecord[navWindowInfo->firstDisplayedRecord-navWindowInfo->indexRecordBuffer+i].l3Czero);
						break;

					default:
						buffer[0]=0;
						break;
				}


				ExtTextOut(hdc, columnStart,y,ETO_OPAQUE, &outputRect, buffer, strlen(buffer), NULL);
				columnStart+=navWindowInfo->widthColumn[c];
			}
		}
		outputRect.left=columnStart;
		outputRect.right=clientRect.right;

		ExtTextOut(hdc, columnStart,y,ETO_OPAQUE, &outputRect, NULL, 0, NULL);



	y+=heightFont;


	outputRect.left=clientRect.left;
	outputRect.right=clientRect.right;
	outputRect.top=y;
	outputRect.bottom=y+1;
	SetBkColor(hdc, RGB_ZINNY_HIGHPURPLE);
	ExtTextOut(hdc, 0,y,ETO_OPAQUE, &outputRect, NULL, 0, NULL);
	y++;
	}

	if (y<clientRect.bottom)	{		//if we still have some space left at end of list
		SetBkColor(hdc, RGB_ZINNY_MIDPURPLE);
		outputRect.left=clientRect.left;
		outputRect.right=clientRect.right;
		outputRect.top=y;
		outputRect.bottom=clientRect.bottom;
		ExtTextOut(hdc, 0,y,ETO_OPAQUE, &outputRect, NULL, 0, NULL);
	}

	DeleteObject(hSmallFont);
	EndPaint(hwnd, &ps);

	return 0;
}

void NavInitiateColumnWidths(NAVWINDOW_INFO * navWindowInfo)
{

	navWindowInfo->widthColumn[0]=110;
	navWindowInfo->widthColumn[1]=80;
	navWindowInfo->widthColumn[2]=40;
	navWindowInfo->widthColumn[3]=40;
	navWindowInfo->widthColumn[4]=40;
	navWindowInfo->widthColumn[5]=40;
	navWindowInfo->widthColumn[6]=0;
	navWindowInfo->widthColumn[7]=80;
	navWindowInfo->widthColumn[8]=80;
	navWindowInfo->widthColumn[9]=70;
	navWindowInfo->widthColumn[10]=40;
	navWindowInfo->widthColumn[11]=80;
	navWindowInfo->widthColumn[12]=0;
	navWindowInfo->widthColumn[13]=70;
	navWindowInfo->widthColumn[14]=40;
	navWindowInfo->widthColumn[15]=0;
	navWindowInfo->widthColumn[16]=0;
	navWindowInfo->widthColumn[17]=0;
	navWindowInfo->widthColumn[18]=0;
	navWindowInfo->widthColumn[19]=0;
	navWindowInfo->widthColumn[20]=0;
	navWindowInfo->widthColumn[21]=0;

	sprintf(navWindowInfo->columnTitle[0], "Number");
	sprintf(navWindowInfo->columnTitle[1], "Short0");
	sprintf(navWindowInfo->columnTitle[2], "Byte2");
	sprintf(navWindowInfo->columnTitle[3], "Byte3");
	sprintf(navWindowInfo->columnTitle[4], "Byte4");
	sprintf(navWindowInfo->columnTitle[5], "Byte5");
	sprintf(navWindowInfo->columnTitle[6], "Short6");
	sprintf(navWindowInfo->columnTitle[7], "Offset");
	sprintf(navWindowInfo->columnTitle[8], "Timer");
	sprintf(navWindowInfo->columnTitle[9], "Short14");
	sprintf(navWindowInfo->columnTitle[10], "Short16");
	sprintf(navWindowInfo->columnTitle[11], "Milliseconds");
	sprintf(navWindowInfo->columnTitle[12], "Long1Cz");
	sprintf(navWindowInfo->columnTitle[13], "Long20");
	sprintf(navWindowInfo->columnTitle[14], "Short24");
	sprintf(navWindowInfo->columnTitle[15], "Short26");
	sprintf(navWindowInfo->columnTitle[16], "Long28z");
	sprintf(navWindowInfo->columnTitle[17], "Long2Cz");
	sprintf(navWindowInfo->columnTitle[18], "Long30z");
	sprintf(navWindowInfo->columnTitle[19], "Long34z");
	sprintf(navWindowInfo->columnTitle[20], "Long38z");
	sprintf(navWindowInfo->columnTitle[21], "Long3Cz");
	sprintf(navWindowInfo->columnTitle[22], "empty");


	return;
}

int NavRecordHeaderPaint(HWND hwnd)
{
	HDC	hdc;
	PAINTSTRUCT ps;
	RECT clientRect;
	RECT outputRect;

	NAVWINDOW_INFO *navWindowInfo;
	char buffer[255];
	HFONT hSmallFont;

	TEXTMETRIC textMetric;
	int heightFont;
	int y;

	navWindowInfo=(NAVWINDOW_INFO *)GetWindowLong(hwnd, GWL_USERDATA);


	GetClientRect(hwnd, &clientRect);
	hdc=BeginPaint(hwnd, &ps);

	hSmallFont = CreateFont(
				MulDiv(10, GetDeviceCaps(hdc, LOGPIXELSY), 72),
				0,0,0,FW_NORMAL,
				FALSE,FALSE,FALSE,
				DEFAULT_CHARSET,OUT_OUTLINE_PRECIS,
                CLIP_DEFAULT_PRECIS,NONANTIALIASED_QUALITY, VARIABLE_PITCH|FF_SWISS,"MS Shell Dlg");
	SelectObject(hdc,hSmallFont);

	GetTextMetrics(hdc, &textMetric);
	heightFont= textMetric.tmHeight;

	SetTextColor(hdc, RGB_ZINNY_BLACK);
	SetBkColor(hdc, RGB_ZINNY_MIDPURPLE);


	y=0;

	outputRect.top=y; outputRect.bottom=y+heightFont;
	outputRect.left=clientRect.left; outputRect.right=clientRect.right;
	ExtTextOut(hdc,	0,y,ETO_OPAQUE, &outputRect, navWindowInfo->fileInfo.filename,strlen(navWindowInfo->fileInfo.filename), NULL);
	y+=heightFont;

	outputRect.top=y; outputRect.bottom=y+heightFont;
	sprintf(buffer, "Size: %u bytes", navWindowInfo->fileInfo.filesize);
	ExtTextOut(hdc,	0,y,ETO_OPAQUE, &outputRect, buffer, strlen(buffer), NULL);
	y+=heightFont;

	outputRect.top=y; outputRect.bottom=clientRect.bottom;
	outputRect.left=clientRect.left; outputRect.right=(clientRect.left+clientRect.right)/2;
	sprintf(buffer, "Records: %u", navWindowInfo->fileInfo.numRecords);
	ExtTextOut(hdc,	0,y,ETO_OPAQUE, &outputRect, buffer, strlen(buffer), NULL);

	outputRect.left=(clientRect.left+clientRect.right)/2;		outputRect.right=clientRect.right;
	sprintf(buffer, "Length: %9.2fs", navWindowInfo->fileInfo.estTimeSpanSeconds);
	ExtTextOut(hdc,	(clientRect.left+clientRect.right)/2,y,ETO_OPAQUE, &outputRect, buffer, strlen(buffer), NULL);
	y+=heightFont;

	DeleteObject(hSmallFont);
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


long NavWindowOnMouseWheel(HWND hwnd, short nDelta)
{
	NAVWINDOW_INFO *navWindowInfo;
	navWindowInfo=(NAVWINDOW_INFO *)GetWindowLong(hwnd, GWL_USERDATA);

	int oldFirstLine;

	oldFirstLine = navWindowInfo->firstDisplayedRecord;

	if (nDelta<0) {
		if (navWindowInfo->firstDisplayedRecord < (navWindowInfo->fileInfo.numRecords - navWindowInfo->numDisplayedLines))	{
			navWindowInfo->firstDisplayedRecord++;
		}
	}
	if (nDelta>0) {
		if (navWindowInfo->firstDisplayedRecord>0)	{
			navWindowInfo->firstDisplayedRecord--;
		}
	}

	if (oldFirstLine!=navWindowInfo->firstDisplayedRecord)	{
		UpdateBuffer(navWindowInfo);
		InvalidateRect(navWindowInfo->hwndRecordList, NULL, FALSE);
	}

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

HANDLE NavWindowLoadFile(HWND hwnd, char *openfilename)
{
	NAVWINDOW_INFO *navWindowInfo;
	NAV_RECORD lastRecord;

	navWindowInfo=(NAVWINDOW_INFO *)GetWindowLong(hwnd, GWL_USERDATA);

	//Check if there is already a file open and unload it
	if (navWindowInfo->fileInfo.hNavFile)	{
		CloseHandle(navWindowInfo->fileInfo.hNavFile);

		navWindowInfo->firstDisplayedRecord=0;
		navWindowInfo->indexRecordBuffer=-1;
		InvalidateRect(hwnd, NULL, FALSE);
		InvalidateRect(navWindowInfo->hwndRecordList, NULL, FALSE);
		InvalidateRect(navWindowInfo->hwndRecordListHeader, NULL, FALSE);
	}

	lstrcpy(navWindowInfo->fileInfo.filename, openfilename);
	navWindowInfo->fileInfo.hNavFile = CreateFile(openfilename, GENERIC_READ, FILE_SHARE_READ, NULL,
				OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL | FILE_FLAG_RANDOM_ACCESS, NULL);

	if (navWindowInfo->fileInfo.hNavFile == INVALID_HANDLE_VALUE)
		return INVALID_HANDLE_VALUE;

	navWindowInfo->fileInfo.filesize=GetFileSize(navWindowInfo->fileInfo.hNavFile,NULL);
	navWindowInfo->fileInfo.numRecords=navWindowInfo->fileInfo.filesize/sizeof(NAV_RECORD);

	ReadRecordsFromNavFile(navWindowInfo, navWindowInfo->fileInfo.numRecords-1, &lastRecord, 1);

	navWindowInfo->fileInfo.estTimeSpanSeconds=(double)(lastRecord.milliseconds)/1000;

	//These should be set up before loading.
	UpdateBuffer(navWindowInfo);
	return navWindowInfo->fileInfo.hNavFile;

}

int NavWindowUnloadFile(HWND hwnd)
{
	NAVWINDOW_INFO *navWindowInfo;
	navWindowInfo=(NAVWINDOW_INFO *)GetWindowLong(hwnd, GWL_USERDATA);

	CloseHandle(navWindowInfo->fileInfo.hNavFile);

	free(navWindowInfo);


	return 0;
}


int NavWindowListHeaderHandleContextMenu(HWND hwnd, WPARAM wparam, LPARAM lparam)
{
	NAVWINDOW_INFO *navWindowInfo;
	HMENU hMenu;
	MENUITEMINFO menuItemInfo;
	int index;

	int x,y;
	int c;

	navWindowInfo=(NAVWINDOW_INFO *)GetWindowLong(GetParent(hwnd), GWL_USERDATA);

	x=GET_X_LPARAM(lparam);
	y=GET_Y_LPARAM(lparam);

	hMenu = CreatePopupMenu();


	memset(&menuItemInfo, 0, sizeof(MENUITEMINFO));

	menuItemInfo.cbSize=sizeof(MENUITEMINFO);
	menuItemInfo.fMask = MIIM_STRING|MIIM_STATE|MIIM_ID;
	menuItemInfo.fType = MFT_STRING;

	index=0;

	//Default
	menuItemInfo.dwTypeData="&Default";
	menuItemInfo.cch = 8;
	menuItemInfo.wID = IDM_LISTVIEWHEADERDEFAULT;
	InsertMenuItem(hMenu, index, TRUE, &menuItemInfo);
	index++;

	//Separator
	menuItemInfo.fMask = MIIM_STATE;
	menuItemInfo.fType = MFT_SEPARATOR;
	InsertMenuItem(hMenu, index, TRUE, &menuItemInfo);
	index++;

	menuItemInfo.fMask = MIIM_STRING|MIIM_STATE|MIIM_ID;
	menuItemInfo.fType = MFT_STRING;

	for (c=0; c<21; c++)	{

		menuItemInfo.dwTypeData=navWindowInfo->columnTitle[c];
		menuItemInfo.cch = strlen(navWindowInfo->columnTitle[c]);
		menuItemInfo.wID = IDM_LISTVIEWHEADERCOLUMNS + c;

		if (navWindowInfo->widthColumn[c])
			menuItemInfo.fState = MF_CHECKED;
		else
			menuItemInfo.fState = MF_UNCHECKED;

		InsertMenuItem(hMenu, index, TRUE, &menuItemInfo);
		index++;
	}




	TrackPopupMenuEx(hMenu, TPM_LEFTALIGN| TPM_TOPALIGN, x, y, hwnd, NULL);
	DestroyMenu(hMenu);

	return 0;
}
