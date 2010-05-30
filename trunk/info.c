#include <stdio.h>
#include <wchar.h>
#include "zorve.h"
#include "zorveres.h"
#include "info.h"


//This registers the Window Class for the list files
int InfoWindowRegisterWndClass(HINSTANCE hInst)
{
	WNDCLASS wc;

	//Wipe the wndclass for editing.
	memset(&wc,0,sizeof(WNDCLASS));

	wc.style         = 0;
	wc.lpfnWndProc   = (WNDPROC)ChildWndInfoProc;
	wc.cbClsExtra    = 0;
	wc.cbWndExtra    = 20;
	wc.hInstance     = hInst;                      // Owner of this class
	wc.hIcon         = LoadIcon(hInst, MAKEINTRESOURCE(IDI_INFO));
	wc.hCursor       = LoadCursor(NULL, IDC_ARROW);
	wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1); // Default color
	wc.lpszMenuName  = NULL;
	wc.lpszClassName = "InfoWndClass";

	if (!RegisterClass((LPWNDCLASS)&wc))
		return 0;
	return 1;
}

HWND InfoWindowCreateOrShow(HWND hwnd, HWND hwndChild, HINSTANCE hInst)
{
	if (IsWindow(hwnd))	{
		ShowWindow(hwnd, SW_SHOW);
		return hwnd;
	}

	return InfoWindowCreate(hwndChild, hInst);
}


//Creates the list window
HWND InfoWindowCreate(HWND hwndMDIClient, HINSTANCE hInst)
{
	HWND  hwndChild;
	MDICREATESTRUCT mcs;

	mcs.szClass = "InfoWndClass";      // window class name
	mcs.szTitle = "File Information";             // window title
	mcs.hOwner  = hInst;            // owner
	mcs.x       = 450;
	mcs.y       = 0;
	mcs.cx      = 490;    // width
	mcs.cy      = 340;    // height
	mcs.style   = WS_CLIPCHILDREN|WS_CHILD;                // window style
	mcs.lParam  = 0;                // lparam

	hwndChild = (HWND) SendMessage(hwndMDIClient, WM_MDICREATE, 0, (LPARAM)(LPMDICREATESTRUCT) &mcs);

	if (hwndChild != NULL)
		ShowWindow(hwndChild, SW_SHOW);

	return hwndChild;
}

int  InfoWindowLoadFile(HWND hwnd, char *filename, int checkForReloads)
{
	//returns 1 if already open and nothing is done
	//0 if opens file again

	HANDLE infoFile;
	INFOFILE_INFO *infoFileStruct;

	long n;

	infoFileStruct = (INFOFILE_INFO *)GetWindowLong(hwnd, GWL_USERDATA);

	if ((checkForReloads) && (strcmp(infoFileStruct->filename, filename)==0))
		return 1;

	//MessageBox(hwnd, infoFileStruct->filename, filename, 0);

	//copy the filename first
	strcpy(infoFileStruct->filename, filename);
	//the zero some things (unfortunately, the hwnds are stored in this property, so can't efficiently memset to zero.
	infoFileStruct->loadedMpeg=0;
	infoFileStruct->loadedNav=0;
	infoFileStruct->loadedJpeg=0;


	infoFile = CreateFile(filename, GENERIC_READ, FILE_SHARE_READ, NULL,
				OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL | FILE_FLAG_RANDOM_ACCESS, NULL);

	//Magic number, then recording name
	ReadFile(infoFile, &infoFileStruct->magicLong, 4, &n, NULL);	//if this isn't 00080500 then should fail
	ReadFile(infoFile, &infoFileStruct->unknownWord, 2, &n, NULL);
	ReadFile(infoFile,  &infoFileStruct->recordingname, 120, &n, NULL);

	//Description
	SetFilePointer(infoFile, 0x07e, NULL, FILE_BEGIN);
	ReadFile(infoFile, &infoFileStruct->description, 512, &n, NULL);
	infoFileStruct->description[511]=0;	//terminate the string, just in case it isn't

	//Supplementary time and date
	SetFilePointer(infoFile, 0x0288, NULL, FILE_BEGIN);
	ReadFile(infoFile, &infoFileStruct->modjulianday, 2, &n, NULL);
	infoFileStruct->modjulianday=swap_endian_word(infoFileStruct->modjulianday);
	//then the hour and minute
	ReadFile(infoFile, &infoFileStruct->decimalbyteHour, 1, &n, NULL);
	ReadFile(infoFile, &infoFileStruct->decimalbyteMinute, 1, &n, NULL);
	infoFileStruct->decimalbyteHour=(infoFileStruct->decimalbyteHour & 0x0F) + ((infoFileStruct->decimalbyteHour & 0xF0)/16)*10;
	infoFileStruct->decimalbyteMinute=(infoFileStruct->decimalbyteMinute & 0x0F) + ((infoFileStruct->decimalbyteMinute & 0xF0)/16)*10;


	//time, size and duration (first copy)
	SetFilePointer(infoFile, 0x0294, NULL, FILE_BEGIN);
	ReadFile(infoFile, &infoFileStruct->unixtime, 4, &n, NULL);

	SetFilePointer(infoFile, 0x02a0, NULL, FILE_BEGIN);
	ReadFile(infoFile, &infoFileStruct->filesize, 4, &n, NULL);

	SetFilePointer(infoFile, 0x02a8, NULL, FILE_BEGIN);
	ReadFile(infoFile, &infoFileStruct->duration, 4, &n, NULL);

	//Read the duplicates at 0x0a00
	//DUP Supplementary time and date
	SetFilePointer(infoFile, 0x0a88, NULL, FILE_BEGIN);
	ReadFile(infoFile, &infoFileStruct->dup_modjulianday, 2, &n, NULL);
	infoFileStruct->dup_modjulianday=swap_endian_word(infoFileStruct->dup_modjulianday);
	//DUP then the hour and minute
	ReadFile(infoFile, &infoFileStruct->dup_decimalbyteHour, 1, &n, NULL);
	ReadFile(infoFile, &infoFileStruct->dup_decimalbyteMinute, 1, &n, NULL);
	infoFileStruct->dup_decimalbyteHour=(infoFileStruct->dup_decimalbyteHour & 0x0F) + ((infoFileStruct->dup_decimalbyteHour & 0xF0)/16)*10;
	infoFileStruct->dup_decimalbyteMinute=(infoFileStruct->dup_decimalbyteMinute & 0x0F) + ((infoFileStruct->dup_decimalbyteMinute & 0xF0)/16)*10;
	//DUP time, size and duration (second copy)
	SetFilePointer(infoFile, 0x0a94, NULL, FILE_BEGIN);
	ReadFile(infoFile, &infoFileStruct->dup_unixtime, 4, &n, NULL);
	SetFilePointer(infoFile, 0x0aa0, NULL, FILE_BEGIN);
	ReadFile(infoFile, &infoFileStruct->dup_filesize, 4, &n, NULL);
	SetFilePointer(infoFile, 0x0aa8, NULL, FILE_BEGIN);
	ReadFile(infoFile, &infoFileStruct->dup_duration, 4, &n, NULL);




	//Read the associated MPEG
	SetFilePointer(infoFile, 0x0c04, NULL, FILE_BEGIN);
	ReadFile(infoFile, &infoFileStruct->assocMpeg, 256, &n, NULL);

	//Read the associated Nav file
	SetFilePointer(infoFile, 0x0d04, NULL, FILE_BEGIN);
	ReadFile(infoFile, &infoFileStruct->assocNav, 256, &n, NULL);

	//read the pids
	SetFilePointer(infoFile, 0x0e04, NULL, FILE_BEGIN);	//read the first one
	ReadFile(infoFile, &infoFileStruct->PID[0], 2, &n, NULL);

	//Read the associated JPEG
	SetFilePointer(infoFile, 0x0e60, NULL, FILE_BEGIN);
	ReadFile(infoFile, &infoFileStruct->assocJpeg, 256, &n, NULL);


	CloseHandle(infoFile);

	SendMessage(infoFileStruct->windowInfo.editHwndRecording, WM_SETTEXT, 0, (LPARAM)&infoFileStruct->recordingname);
	SendMessage(infoFileStruct->windowInfo.editHwndDescription, WM_SETTEXT, 0, (LPARAM)&infoFileStruct->description);


	InvalidateRect(hwnd, NULL, FALSE);

	return 0;
}

int SaveInfoChanges(HWND hwnd, INFOFILE_INFO *info)
{
	HANDLE infoFile;
	long n;

	SendMessage(info->windowInfo.editHwndRecording, WM_GETTEXT, 120, (LPARAM)&info->recordingname);
	SendMessage(info->windowInfo.editHwndDescription, WM_GETTEXT, 512, (LPARAM)&info->description);


	infoFile = CreateFile(info->filename, GENERIC_WRITE, 0x0000, NULL,
				OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL | FILE_FLAG_RANDOM_ACCESS, NULL);

	SetFilePointer(infoFile, 0x0006, NULL, FILE_BEGIN);
	WriteFile(infoFile, &info->recordingname, strlen(info->recordingname)+1, &n, NULL);

	SetFilePointer(infoFile, 0x007e, NULL, FILE_BEGIN);
	WriteFile(infoFile, &info->description, strlen(info->description)+1, &n, NULL);


	CloseHandle(infoFile);

	return 0;
}



//This is the main handler for this window
LRESULT CALLBACK ChildWndInfoProc(HWND hwnd,UINT msg,WPARAM wparam,LPARAM lparam)
{
	HINSTANCE hInst;

	INFOFILE_INFO *infoFile;
	HFONT hSmallFont;

	switch(msg) {
		case WM_CREATE:
			hInst=((LPCREATESTRUCT)lparam)->hInstance;

			infoFile=malloc(sizeof(INFOFILE_INFO));
			SetWindowLong(hwnd, GWL_USERDATA, (long)infoFile);	//need to set this to pointer to new structure
			memset(infoFile, 0, sizeof(INFOFILE_INFO));

			infoFile->windowInfo.editHwndRecording =	CreateWindow("EDIT", "Recording name",
				    					WS_CHILD|WS_VISIBLE|ES_LEFT|WS_GROUP|WS_TABSTOP|WS_BORDER|ES_AUTOHSCROLL,
									    100,20,0,0, hwnd, NULL, hInst, NULL);
			SendMessage(infoFile->windowInfo.editHwndRecording, EM_LIMITTEXT, 50, 0);	//limit text to 50 chars

			infoFile->windowInfo.editHwndDescription =	CreateWindow("EDIT", "Description",
				    					WS_CHILD|WS_VISIBLE|ES_LEFT|WS_TABSTOP|WS_BORDER|ES_AUTOVSCROLL|ES_MULTILINE,
									    100,40,0,0, hwnd, NULL, hInst, NULL);
			SendMessage(infoFile->windowInfo.editHwndDescription, EM_LIMITTEXT, 200, 0);	//limit text to 200 chars

			infoFile->windowInfo.buttonHwndSaveAll =	CreateWindow("BUTTON", "Save changes",
				    					WS_CHILD|WS_VISIBLE|WS_TABSTOP|WS_BORDER|BS_PUSHBUTTON,
									    20,250,0,0, hwnd, NULL, hInst, NULL);
			infoFile->windowInfo.buttonHwndRevert =	CreateWindow("BUTTON", "Revert",
				    					WS_CHILD|WS_VISIBLE|WS_TABSTOP|WS_BORDER|BS_PUSHBUTTON,
									    160,250,0,0, hwnd, NULL, hInst, NULL);
			hSmallFont = CreateFont(
				MulDiv(10, GetDeviceCaps(GetDC(hwnd), LOGPIXELSY), 72),
				0,0,0,FW_NORMAL,
				FALSE,FALSE,FALSE,
				DEFAULT_CHARSET,OUT_OUTLINE_PRECIS,
                CLIP_DEFAULT_PRECIS,NONANTIALIASED_QUALITY, VARIABLE_PITCH|FF_SWISS,"MS Shell Dlg");
				//needs to be deleted somewhere


			SendMessage(infoFile->windowInfo.editHwndRecording, WM_SETFONT, (WPARAM)hSmallFont, 0);
			SendMessage(infoFile->windowInfo.editHwndDescription, WM_SETFONT, (WPARAM)hSmallFont, 0);
			SendMessage(infoFile->windowInfo.buttonHwndSaveAll, WM_SETFONT, (WPARAM)hSmallFont, 0);
			SendMessage(infoFile->windowInfo.buttonHwndRevert, WM_SETFONT, (WPARAM)hSmallFont, 0);

			break;
		case WM_COMMAND:
			infoFile = (INFOFILE_INFO *)GetWindowLong(hwnd, GWL_USERDATA);
			if (lparam==(LPARAM)infoFile->windowInfo.buttonHwndSaveAll)	{
				SaveInfoChanges(hwnd, infoFile);
				SendMessage(GetParent(GetParent(hwnd)), ZM_LIST_SELECTFROMFILEANDREFRESH, (WPARAM)infoFile->filename, (LPARAM)0);
			}
			if (lparam==(LPARAM)infoFile->windowInfo.buttonHwndRevert)	{
				InfoWindowLoadFile(hwnd, infoFile->filename, FALSE);
			}

			break;
		case WM_SIZE:
			InvalidateRect(hwnd, NULL, FALSE);
			break;
		case WM_PAINT:
			PaintInfoWindow(hwnd);
			break;
		case WM_ERASEBKGND:
			return 1;
			break;
		case ZM_REQUEST_RECORDINGNAME:
			infoFile = (INFOFILE_INFO *)GetWindowLong(hwnd, GWL_USERDATA);
			if (!strcmp((char*) wparam, infoFile->filename))
				SendMessage((HWND)lparam, ZM_REPLY_RECORDINGNAME, (WPARAM)infoFile->recordingname,0);
			return 1;
			break;
		case ZM_INFO_CHANGEMPEGSTATUS:
			infoFile = (INFOFILE_INFO *)GetWindowLong(hwnd, GWL_USERDATA);
			infoFile->loadedMpeg=lparam;
			//MessageBox(hwnd, "Unable to load MPEG-TS file.", "Zorve", MB_ICONEXCLAMATION|MB_RETRYCANCEL); (we want to avoid modals)
			InvalidateRect(hwnd, NULL, FALSE);
			break;
		case ZM_INFO_CHANGENAVSTATUS:
			infoFile = (INFOFILE_INFO *)GetWindowLong(hwnd, GWL_USERDATA);
			infoFile->loadedNav=lparam;

//			MessageBox(hwnd, "Unable to load NAV file.", "Zorve", MB_ICONEXCLAMATION|MB_RETRYCANCEL);
			InvalidateRect(hwnd, NULL, FALSE);
			break;
		case WM_DESTROY:
			infoFile = (INFOFILE_INFO *)GetWindowLong(hwnd, GWL_USERDATA);
			free(infoFile);
			ZorveSetHwndInfo(NULL);
			break;
	}
	return DefMDIChildProc(hwnd, msg, wparam, lparam);
}


void PaintInfoWindow(HWND hwnd)
{
	INFOFILE_INFO *infoFile;

	HINSTANCE hInst;
	HDC hdc;
	PAINTSTRUCT psPaint;

	RECT clientRect;
	RECT textRect;

	HFONT hSmallFont;
	TEXTMETRIC textMetric;
	SIZE sizeTextExtent;
	char outputText[255];

	TIME_ZONE_INFORMATION tzi;
	char timezonename[32];
	FILETIME filetime;
	FILETIME localfiletime;
	SYSTEMTIME systemtime;

	int xEditColumn;
	int y;
	int height;
	int margin=5;

	int xbuttonpos;
	int buttonwidth;
	int buttonheight;

	HICON mpegIcon;
	HICON navIcon;
	HBRUSH hBrush;	//background colour for icons



	infoFile=(INFOFILE_INFO *)GetWindowLong(hwnd, GWL_USERDATA);

	GetClientRect(hwnd, &clientRect);

	hdc = BeginPaint(hwnd, &psPaint);

	hSmallFont = CreateFont(
				MulDiv(10, GetDeviceCaps(hdc, LOGPIXELSY), 72),
				0,0,0,FW_NORMAL,
				FALSE,FALSE,FALSE,
				DEFAULT_CHARSET,OUT_OUTLINE_PRECIS,
                CLIP_DEFAULT_PRECIS,NONANTIALIASED_QUALITY, VARIABLE_PITCH|FF_SWISS,"MS Shell Dlg");
	SelectObject(hdc,hSmallFont);




	SetBkColor(hdc, RGB_ZINNY_MIDPURPLE);


	y=0;
	//Filename
	textRect.left=0; textRect.right=clientRect.right;
	textRect.top=0; textRect.bottom=24;
	ExtTextOut(hdc, margin, y, ETO_OPAQUE, &textRect, infoFile->filename,strlen(infoFile->filename), NULL);

	y=24;

	GetTextMetrics(hdc, &textMetric);
	height=textMetric.tmHeight;
	GetTextExtentPoint32(hdc, "Description:",12,&sizeTextExtent);
	xEditColumn=sizeTextExtent.cx+margin;

	textRect.left=0; textRect.right=xEditColumn+margin;
	textRect.top=y; textRect.bottom=y+height+4;

	ExtTextOut(hdc, margin, y, ETO_OPAQUE, &textRect, "Recording:", 10, NULL);
//	SendMessage(infoFile->windowInfo.editHwndRecording, WM_SETFONT, (WPARAM)hSmallFont, 0);
	MoveWindow(infoFile->windowInfo.editHwndRecording, xEditColumn+margin, y, (clientRect.right-clientRect.left)-xEditColumn-margin-margin, height+4, TRUE);
	textRect.left=clientRect.right-margin; textRect.right=clientRect.right;
	ExtTextOut(hdc, margin, y, ETO_OPAQUE, &textRect, NULL, 0, NULL);	//rightmarginfill
	textRect.top=y+height+4; textRect.bottom=y+height+4+margin;
	textRect.left=clientRect.left; textRect.right=clientRect.right;
	ExtTextOut(hdc, margin, y, ETO_OPAQUE, &textRect, NULL, 0, NULL);	//lowermarginfill
	y+=height+4+margin;

	textRect.left=0; textRect.right=xEditColumn+margin;
	textRect.top=y; textRect.bottom=y+height*4+4;

	ExtTextOut(hdc, margin, y, ETO_OPAQUE, &textRect, "Description:", 12, NULL);
//	SendMessage(infoFile->windowInfo.editHwndDescription, WM_SETFONT, (WPARAM)hSmallFont, 0);
	MoveWindow(infoFile->windowInfo.editHwndDescription, xEditColumn+margin, y, (clientRect.right-clientRect.left)-xEditColumn-margin-margin,height*4+4, TRUE);
	textRect.left=clientRect.right-margin; textRect.right=clientRect.right;
	ExtTextOut(hdc, margin, y, ETO_OPAQUE, &textRect, NULL, 0, NULL);	//rightmarginfill
	textRect.top=y+height+4; textRect.bottom=y+height*4+4+margin;
	textRect.left=clientRect.left; textRect.right=clientRect.right;
	ExtTextOut(hdc, margin, y, ETO_OPAQUE, &textRect, NULL, 0, NULL);	//lowermarginfill
	y+=height*4+4 +margin;

	textRect.left=0; textRect.right=clientRect.right;
	textRect.top=y; textRect.bottom=y+height+margin;
	ModJulianTimeToFileTime(infoFile->modjulianday, &filetime);
	FileTimeToSystemTime(&filetime, &systemtime);
	sprintf(outputText, "Original Date/Time: %i/%i/%i, %02i:%02i", systemtime.wDay, systemtime.wMonth, systemtime.wYear, infoFile->decimalbyteHour, infoFile->decimalbyteMinute);
	ExtTextOut(hdc, margin, y, ETO_OPAQUE, &textRect, outputText, strlen(outputText), NULL);
	y+=height+margin;

	textRect.left=0; textRect.right=clientRect.right;
	textRect.top=y; textRect.bottom=y+height+margin;
	UnixTimeToFileTime(infoFile->unixtime, &filetime);		//convert the file's unix timestamp to a FILETIME
	FileTimeToLocalFileTime(&filetime, &localfiletime);		//Then correct for timezone
	FileTimeToSystemTime(&localfiletime, &systemtime);		//Then convert it to be easily printable
	GetTimeZoneInformation(&tzi);

	WideCharToMultiByte(CP_ACP, 0, &tzi.StandardName, -1, &timezonename[0], 32, NULL, NULL);
	sprintf(outputText, "Display Date/Time: %i/%i/%i, %02i:%02i:%02i (%s)", systemtime.wDay, systemtime.wMonth, systemtime.wYear, systemtime.wHour, systemtime.wMinute, systemtime.wSecond, timezonename);
	ExtTextOut(hdc, margin, y, ETO_OPAQUE, &textRect, outputText, strlen(outputText), NULL);
	y+=height+margin;

	textRect.left=0; textRect.right=clientRect.right;
	textRect.top=y; textRect.bottom=y+height+margin;
	sprintf(outputText, "First PID: 0x%03x (%s) ", infoFile->PID[0], ReturnChannelNameFromPID(infoFile->PID[0]));
	ExtTextOut(hdc, margin, y, ETO_OPAQUE, &textRect, outputText, strlen(outputText), NULL);
	y+=height+margin;


	hInst=(HINSTANCE)GetWindowLong(hwnd, GWL_HINSTANCE);
	hBrush=CreateSolidBrush(RGB_ZINNY_MIDPURPLE);

	mpegIcon = LoadIcon(hInst, MAKEINTRESOURCE(IDI_MPEGTS));
	DrawIconEx(hdc, margin, y, mpegIcon, 16,16, 0, hBrush, DI_NORMAL);
	textRect.left=margin+16; textRect.right=clientRect.right;
	textRect.top=y; textRect.bottom=y+MAX(height,16)+margin;
	sprintf(outputText, " %s ", infoFile->assocMpeg);
	if (infoFile->loadedMpeg<0)
		SetTextColor(hdc, RGB_ZINNY_REDALERT);
	else if (infoFile->loadedMpeg==0)
		SetTextColor(hdc, RGB_ZINNY_BLACK);
	else
		SetTextColor(hdc, RGB_ZINNY_GREENALERT);
	ExtTextOut(hdc, margin+16+margin, y, ETO_OPAQUE, &textRect, outputText, strlen(outputText), NULL);

	textRect.left=0; textRect.right=margin;
	textRect.top=y; textRect.bottom=y+MAX(height,16)+margin;
	ExtTextOut(hdc, margin, y, ETO_OPAQUE, &textRect, NULL, 0, NULL);	//leftmarginfill
	textRect.left=margin; textRect.right=margin+16;
	textRect.top=y+16; textRect.bottom=y+MAX(height,16)+margin;
	ExtTextOut(hdc, margin, y, ETO_OPAQUE, &textRect, NULL, 0, NULL);	//bottommarginfill

	y+=MAX(height,16)+margin;


	navIcon = LoadIcon(hInst, MAKEINTRESOURCE(IDI_NAV));
	DrawIconEx(hdc, margin, y, navIcon, 16,16, 0, hBrush, DI_NORMAL);
	textRect.left=margin+16; textRect.right=clientRect.right;
	textRect.top=y; textRect.bottom=y+MAX(height,16)+margin;
	sprintf(outputText, " %s ", infoFile->assocNav);
	if (infoFile->loadedNav<0)
		SetTextColor(hdc, RGB_ZINNY_REDALERT);
	else if (infoFile->loadedNav==0)
		SetTextColor(hdc, RGB_ZINNY_BLACK);
	else
		SetTextColor(hdc, RGB_ZINNY_GREENALERT);
	ExtTextOut(hdc, margin+16+margin, y, ETO_OPAQUE, &textRect, outputText, strlen(outputText), NULL);
	textRect.left=0; textRect.right=margin;
	textRect.top=y; textRect.bottom=y+MAX(height,16)+margin;
	ExtTextOut(hdc, margin, y, ETO_OPAQUE, &textRect, NULL, 0, NULL);	//leftmarginfill
	textRect.left=margin; textRect.right=margin+16;
	textRect.top=y+16; textRect.bottom=y+MAX(height,16)+margin;
	ExtTextOut(hdc, margin, y, ETO_OPAQUE, &textRect, NULL, 0, NULL);	//bottommarginfill
	y+=MAX(height,16)+margin;


	DeleteObject(hBrush);



	//Adjust the position of buttons
	buttonheight=MAX(height+4+4, 23);
	buttonwidth=100;
	xbuttonpos=clientRect.right-margin-buttonwidth;

	//SendMessage(infoFile->windowInfo.buttonHwndRevert, WM_SETFONT, (WPARAM)hSmallFont, 0);
	MoveWindow(infoFile->windowInfo.buttonHwndRevert, xbuttonpos, clientRect.bottom-margin-buttonheight, buttonwidth, buttonheight, TRUE);
	InvalidateRect(infoFile->windowInfo.buttonHwndRevert, NULL, TRUE);

	xbuttonpos=clientRect.right-margin-buttonwidth-margin-buttonwidth;
	//SendMessage(infoFile->windowInfo.buttonHwndSaveAll, WM_SETFONT, (WPARAM)hSmallFont, 0);
	MoveWindow(infoFile->windowInfo.buttonHwndSaveAll, xbuttonpos, clientRect.bottom-margin-buttonheight, buttonwidth, buttonheight, TRUE);
	InvalidateRect(infoFile->windowInfo.buttonHwndSaveAll, NULL, TRUE);

	//Fill the bottom of the window, after the last line
	textRect.top=y; textRect.bottom=clientRect.bottom;
	textRect.left=clientRect.left; textRect.right=clientRect.right;
	ExtTextOut(hdc, margin, y, ETO_OPAQUE, &textRect, NULL, 0, NULL);	//lowermarginfill


	DeleteObject(hSmallFont);

	EndPaint(hwnd, &psPaint);

	return;
}


