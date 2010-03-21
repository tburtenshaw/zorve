#include "stdio.h"
#include "zorveres.h"
#include "zorve.h"
#include "mpegts.h"

//This registers the Window Class for the list files
int MpegWindowRegisterWndClass(HINSTANCE hInst)
{
	WNDCLASS wc;

	//Wipe the wndclass for editing.
	memset(&wc,0,sizeof(WNDCLASS));

	wc.style         = 0;
	wc.lpfnWndProc   = (WNDPROC)ChildWndMpegProc;
	wc.cbClsExtra    = 0;
	wc.cbWndExtra    = 20;
	wc.hInstance     = hInst;                      // Owner of this class
	wc.hIcon         = LoadIcon(hInst, MAKEINTRESOURCE(IDI_MPEGTS));
	wc.hCursor       = LoadCursor(NULL, IDC_ARROW);
	wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1); // Default color
	wc.lpszMenuName  = NULL;
	wc.lpszClassName = "MpegWndClass";

	if (!RegisterClass((LPWNDCLASS)&wc))
		return 0;

	wc.style         = 0;
	wc.lpfnWndProc   = (WNDPROC)MpegFileInfoProc;
	wc.cbClsExtra    = 0;
	wc.cbWndExtra    = 20;
	wc.hInstance     = hInst;                      // Owner of this class
	wc.hIcon         = NULL;
	wc.hCursor       = LoadCursor(NULL, IDC_ARROW);
	wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1); // Default color
	wc.lpszMenuName  = NULL;
	wc.lpszClassName = "MpegFileInfoClass";

	if (!RegisterClass((LPWNDCLASS)&wc))
		return 0;

	wc.style         = 0;
	wc.lpfnWndProc   = (WNDPROC)MpegFileDetailProc;
	wc.cbClsExtra    = 0;
	wc.cbWndExtra    = 20;
	wc.hInstance     = hInst;                      // Owner of this class
	wc.hIcon         = NULL;
	wc.hCursor       = LoadCursor(NULL, IDC_ARROW);
	wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1); // Default color
	wc.lpszMenuName  = NULL;
	wc.lpszClassName = "MpegFileDetailClass";

	if (!RegisterClass((LPWNDCLASS)&wc))
		return 0;

	wc.style         = 0;
	wc.lpfnWndProc   = (WNDPROC)MpegPacketInfoProc;
	wc.cbClsExtra    = 0;
	wc.cbWndExtra    = 20;
	wc.hInstance     = hInst;                      // Owner of this class
	wc.hIcon         = NULL;
	wc.hCursor       = LoadCursor(NULL, IDC_ARROW);
	wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1); // Default color
	wc.lpszMenuName  = NULL;
	wc.lpszClassName = "MpegPacketInfoClass";

	if (!RegisterClass((LPWNDCLASS)&wc))
		return 0;

	return 1;
}

LRESULT CALLBACK ChildWndMpegProc(HWND hwnd,UINT msg,WPARAM wparam,LPARAM lparam)
{
	HINSTANCE hInst;
	MPEGWINDOW_INFO *mpegWindowInfo;

	unsigned long tempOffset;

	switch(msg) {
		case WM_CREATE:
			hInst=((LPCREATESTRUCT)lparam)->hInstance;	//get the hinstance for use when creating the child window

			mpegWindowInfo=malloc(sizeof(MPEGWINDOW_INFO));
			SetWindowLong(hwnd, GWL_USERDATA, (long)mpegWindowInfo); //set the custom long of the window to remember it
			memset(mpegWindowInfo, 0, sizeof(MPEGWINDOW_INFO));	//make sure everything set to zero

			mpegWindowInfo->hwndFileInfo = CreateWindow("MpegFileInfoClass", "fileinfo",
										WS_CHILD|WS_VISIBLE,
										0,0,300,100, hwnd, NULL, hInst,NULL);
			mpegWindowInfo->hwndFileDetail = CreateWindow("MpegFileDetailClass", "filedetail",
										WS_CHILD|WS_VISIBLE,
										300,0,200,100, hwnd, NULL, hInst,NULL);
			mpegWindowInfo->hwndBlockInfo = CreateWindow("MpegPacketInfoClass", "blockinfo",
										WS_CHILD|WS_VISIBLE,
										0,100,250,200, hwnd, NULL, hInst,NULL);
			break;
		case WM_PAINT:
			MpegWindowPaint(hwnd);
			break;
		case WM_ERASEBKGND:
			return 1;
			break;
		case WM_DESTROY:
			MpegPrepareForClose(hwnd);
			break;
	}
	return DefMDIChildProc(hwnd, msg, wparam, lparam);
}

int MpegWindowPaint(HWND hwnd)
{
	MPEGWINDOW_INFO *mpegWindowInfo;

	HDC	hdc;
	PAINTSTRUCT ps;
	RECT clientRect;
	RECT outputRect;

	HFONT hSmallFont;
	TEXTMETRIC textMetric;

	char buffer[255];
	int y=0;

	mpegWindowInfo=(MPEGWINDOW_INFO *)GetWindowLong(hwnd, GWL_USERDATA);	//get the point to window info

	GetClientRect(hwnd, &clientRect);
	hdc=BeginPaint(hwnd, &ps);

	SetBkColor(hdc, RGB_ZINNY_DARKBLUE);
	SetTextColor(hdc, RGB_ZINNY_WHITE);

	outputRect.left=clientRect.left;
	outputRect.right=clientRect.right;

	y=clientRect.top;

	outputRect.top=y;
	outputRect.bottom=y+16;

	sprintf(buffer, "%s",  mpegWindowInfo->fileInfo.filename);
	ExtTextOut(hdc, 0,y,ETO_OPAQUE, &outputRect, buffer, strlen(buffer), NULL);
	y+=16;

	outputRect.top=y;
	outputRect.bottom=clientRect.bottom;
	sprintf(buffer, "File size: %u. Offset:0x%x, pid:$%x, cont:%u", mpegWindowInfo->fileInfo.filesize, mpegWindowInfo->fileInfo.displayOffset, mpegWindowInfo->displayedPacket.pid, mpegWindowInfo->displayedPacket.continuitycounter);
	ExtTextOut(hdc, 0,y,ETO_OPAQUE, &outputRect, buffer, strlen(buffer), NULL);
	y+=16;



	outputRect.top=y;
	outputRect.bottom=clientRect.bottom;
	sprintf(buffer, "Percentage: %i", mpegWindowInfo->fileInfo.countPackets*100/(mpegWindowInfo->fileInfo.filesize/188));
	ExtTextOut(hdc, 0,y,ETO_OPAQUE, &outputRect, buffer, strlen(buffer), NULL);
	y+=16;



	EndPaint(hwnd, &ps);

	return 0;
}

int MpegWindowLoadFile(HWND hwnd, char * mpegFile)
{
	MPEGWINDOW_INFO *mpegWindowInfo;

	mpegWindowInfo=(MPEGWINDOW_INFO *)GetWindowLong(hwnd, GWL_USERDATA);

	//Check if there is already a file open and unload it
	if (mpegWindowInfo->fileInfo.hMpegFile)	{
		CloseHandle(mpegWindowInfo->fileInfo.hMpegFile);
		memset(mpegWindowInfo,0,sizeof(MPEGWINDOW_INFO));
	}


	lstrcpy(mpegWindowInfo->fileInfo.filename, mpegFile);
	//Load as 'sequential' for caching.
	mpegWindowInfo->fileInfo.hMpegFile = CreateFile(mpegFile, GENERIC_READ, FILE_SHARE_READ, NULL,
				OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL | FILE_FLAG_SEQUENTIAL_SCAN, NULL);

	mpegWindowInfo->fileInfo.filesize=GetFileSize(mpegWindowInfo->fileInfo.hMpegFile, NULL);

	MpegTSFindSyncByte(mpegWindowInfo->fileInfo.hMpegFile, &mpegWindowInfo->fileInfo.firstSyncByte);
	mpegWindowInfo->fileInfo.offset=mpegWindowInfo->fileInfo.firstSyncByte;

	//Now start a thread that gradually loads in statistical information about the mpeg.
	mpegWindowInfo->fileInfo.hFileAccessMutex = CreateMutex(NULL, FALSE, "fileaccessmutex");
	mpegWindowInfo->fileInfo.hBackgroundThread = CreateThread(NULL, (SIZE_T)0, (LPTHREAD_START_ROUTINE)MpegReadFileStats, &mpegWindowInfo->fileInfo, 0, NULL);
	//MpegReadFileStats(&mpegWindowInfo->fileInfo);	//override the background thread for debugging

	return 0;
}

DWORD WINAPI MpegReadFileStats(MPEGFILE_INFO *mpegFileInfo)
{
	TS_PACKET packet;
	long offset;
	int pTry;
	int stay;

	char tempBuffer[255];


	offset=0;
	mpegFileInfo->loadingInBackground = TRUE;

	while (offset < mpegFileInfo->filesize)	{
		if (mpegFileInfo->stopThreadNow)
			return 0;
		WaitForSingleObject(mpegFileInfo->hFileAccessMutex,INFINITE);
		MpegReadPacket(mpegFileInfo, &packet);
		mpegFileInfo->countPackets++;
		offset+=188;

		pTry=0;
		stay=1;
		while (stay)	{
			if (mpegFileInfo->seenPid[pTry]==packet.pid)	{
				mpegFileInfo->countPid[pTry]++;
				stay=0;
			}
			if ((stay==1) && (mpegFileInfo->seenPid[pTry]==0))	{
				mpegFileInfo->seenPid[pTry]=packet.pid;
				mpegFileInfo->countPid[pTry]=1;
				stay=0;
			}
			pTry++;
			if (pTry==255)
				stay=0;
		}
		ReleaseMutex(mpegFileInfo->hFileAccessMutex);

	}
	SetFilePointer(mpegFileInfo->hMpegFile, 0, 0, FILE_BEGIN);
	mpegFileInfo->offset=0;

	pTry=0;

	while (mpegFileInfo->seenPid[pTry] && pTry<255)	{
		sprintf(tempBuffer, "No: %i PID: %x",pTry, mpegFileInfo->seenPid[pTry]);
		MessageBox(0, tempBuffer,"Loaded Mpeg",0);
		pTry++;
	}


	mpegFileInfo->loadingInBackground = FALSE;
	return 0;
}

int MpegTSFindSyncByte(HANDLE hFile, long * syncbyteOffset)
{
	BYTE testbyte;
	long n;

	BOOL readResult;
	int occurence;
	unsigned long offset;
	int justskipped;	//if we have just found one and skipped some bytes, we'll go back

	LARGE_INTEGER largeint_filesize;
	unsigned long filesize;

	GetFileSizeEx(hFile, &largeint_filesize);
	filesize=largeint_filesize.LowPart;

	offset=0;
	occurence = 0;
	justskipped=0;

	SetFilePointer(hFile, offset, NULL, FILE_BEGIN);

	readResult = ReadFile(hFile, &testbyte, 1, &n, NULL);
	while ((readResult) && (offset<filesize))	{
		if (testbyte == 0x47)	{
			occurence++;
			if (occurence==5)	{
				*syncbyteOffset = offset-752;
				SetFilePointer(hFile, *syncbyteOffset, NULL, FILE_BEGIN);
				return 1;
			}
			offset+=188;
			justskipped=1;
			readResult = SetFilePointer(hFile, 187, NULL, FILE_CURRENT);
			if (readResult==INVALID_SET_FILE_POINTER)
				return 0;
			readResult = ReadFile(hFile, &testbyte, 1, &n, NULL);
		}
		else	{
			if (justskipped)	{
				offset-=188;
				readResult = SetFilePointer(hFile, -188, NULL, FILE_CURRENT);
				if (readResult==INVALID_SET_FILE_POINTER)
					return 0;
				justskipped=0;
			}
			occurence=0;
			offset++;

			readResult = ReadFile(hFile, &testbyte, 1, &n, NULL);

		}
	}

	return 0;
}

int MpegReadPacket(MPEGFILE_INFO *mpegFileInfo, TS_PACKET *packet)
{
	long n;	//dummy amount read

	int b;	//the byte we are up to
	int offsetAtAF;
	int offsetAtAFExt;

	ReadFile(mpegFileInfo->hMpegFile, packet->TS_raw_packet, 188, &n, NULL);
	mpegFileInfo->offset+=188;

	packet->syncbyte = packet->TS_raw_packet[0];
	packet->transporterror =	packet->TS_raw_packet[1] & 0x80 >> 7;
	packet->payloadstart =		packet->TS_raw_packet[1] & 0x40 >> 6;
	packet->transportpriority =	packet->TS_raw_packet[1] & 0x20 >> 5;
	packet->pid	= ((packet->TS_raw_packet[1] & 0b11111) << 8) | packet->TS_raw_packet[2];
	packet->scrambling	=		(packet->TS_raw_packet[3] & 0b11000000) >> 6;
	packet->adaptation	= 		(packet->TS_raw_packet[3] & 0b00110000) >> 4;
	packet->continuitycounter= 	(packet->TS_raw_packet[3] & 0b00001111);

	if (packet->adaptation & 0b10)	{
		packet->adaptationfield.adaptationfieldlength = packet->TS_raw_packet[4];
		packet->adaptationfield.discontinuity =		packet->TS_raw_packet[5] & 0b10000000 >> 7;
		packet->adaptationfield.randomaccess =		packet->TS_raw_packet[5] & 0b01000000 >> 6;
		packet->adaptationfield.EsPriority =		packet->TS_raw_packet[5] & 0b00100000 >> 5;
		packet->adaptationfield.PCRFlag =			packet->TS_raw_packet[5] & 0b00010000 >> 4;
		packet->adaptationfield.OPCRFlag =			packet->TS_raw_packet[5] & 0b00001000 >> 3;
		packet->adaptationfield.splicingpointflag =	packet->TS_raw_packet[5] & 0b00000100 >> 2;
		packet->adaptationfield.transportprivatedataflag =		packet->TS_raw_packet[5] & 0b00000010 >> 1;
		packet->adaptationfield.adaptationfieldextensionflag =	packet->TS_raw_packet[5] & 0b00000001;

		b=6;
		if (packet->adaptationfield.PCRFlag)	{	//nothing is more annoying than a 33bit value
			packet->adaptationfield.PcrBase	= packet->TS_raw_packet[6];
			packet->adaptationfield.PcrBase <<=1;
			packet->adaptationfield.PcrBase |=packet->TS_raw_packet[7]>>7;

			packet->adaptationfield.PcrExt	=	(packet->TS_raw_packet[7]&1)<<9;
			packet->adaptationfield.PcrExt	|=	packet->TS_raw_packet[8];
			b=9;
		}
		if (packet->adaptationfield.OPCRFlag)	{	//nothing is more annoying than a 33bit value
			packet->adaptationfield.OpcrBase	= packet->TS_raw_packet[b];
			packet->adaptationfield.OpcrBase <<=1;
			packet->adaptationfield.OpcrBase |=packet->TS_raw_packet[b+1]>>7;

			packet->adaptationfield.OpcrExt	=	(packet->TS_raw_packet[b+1]&1)<<9;
			packet->adaptationfield.OpcrExt	|=	packet->TS_raw_packet[b+2];
			b+=3;
		}
		if (packet->adaptationfield.splicingpointflag)	{
			packet->adaptationfield.splicecountdown = packet->TS_raw_packet[b];
			b++;
		}
		if (packet->adaptationfield.transportprivatedataflag)	{
			packet->adaptationfield.transportprivatedatalength = packet->TS_raw_packet[b];
			memcpy(&packet->adaptationfield.privatedata, &packet->TS_raw_packet[b+1], packet->adaptationfield.transportprivatedatalength);
			b+=packet->adaptationfield.transportprivatedatalength+1;
		}
		if (packet->adaptationfield.adaptationfieldextensionflag)	{
			packet->adaptationfield.adaptationfieldextensionlength = packet->TS_raw_packet[b];
			b++;
			offsetAtAFExt=b;
			packet->adaptationfield.ltwflag	= packet->TS_raw_packet[b] & 0b10000000>>7;
			packet->adaptationfield.piecewiserateflag	= packet->TS_raw_packet[b] & 0b01000000>>6;
			packet->adaptationfield.seamlessspliceflag	= packet->TS_raw_packet[b] & 0b00100000>>5;
			b++;
			if (packet->adaptationfield.ltwflag)	{
				packet->adaptationfield.ltwflag	= packet->TS_raw_packet[b] & 0b10000000>>7;
				packet->adaptationfield.ltwoffset = (LONG)(packet->TS_raw_packet[b]&0b01111111)<<8;
				b++;
				packet->adaptationfield.ltwoffset |=packet->TS_raw_packet[b];
				b++;
			}
			if (packet->adaptationfield.piecewiserateflag)	{
				packet->adaptationfield.piecewiserate  = (LONG)(packet->TS_raw_packet[b] & 0b00111111) <<16;
				packet->adaptationfield.piecewiserate |= (LONG)packet->TS_raw_packet[b+1] <<8;
				packet->adaptationfield.piecewiserate |= packet->TS_raw_packet[b+2];
				b+=3;

			}
			if (packet->adaptationfield.seamlessspliceflag)	{
				packet->adaptationfield.splicetype=packet->TS_raw_packet[b] &0b11110000 >> 4;
				packet->adaptationfield.DTSnextAU=packet->TS_raw_packet[b] & 0b00001110;
				packet->adaptationfield.DTSnextAU <<=29;
				//packet->adaptationfield.DTSnextAU|=(ULONGLONG)(packet->TS_raw_packet[b+1] & 0b1111111111111110)<<14;
				//NEED TO FIX
				b+=5;
			}
			//We could skip this, but it'll be skipped when we use the AF length
			//b=offsetAtAFExt+packet->adaptationfield.adaptationfieldextensionlength;
		}
		//Skip the padding. The AF length is at byte 4, this ensures we end after this.
		b=4+packet->adaptationfield.adaptationfieldlength;

	}

	return 0;
}


HWND MpegWindowCreateOrShow(HWND hwnd, HWND hwndChild, HINSTANCE hInst)
{
	if (IsWindow(hwnd))	{
		ShowWindow(hwnd, SW_SHOW);
		return hwnd;
	}

	return MpegWindowCreate(hwndChild, hInst);
}

HWND MpegWindowCreate(HWND hwndMDIClient, HINSTANCE hInst)
{
	HWND  hwndChild;
	MDICREATESTRUCT mcs;

	mcs.szClass = "MpegWndClass";      // window class name
	mcs.szTitle = "MPEG-TS File";             // window title
	mcs.hOwner  = hInst;            // owner
	mcs.x       = 0;
	mcs.y       = 400;
	mcs.cx      = 490;    // width
	mcs.cy      = 340;    // height
	mcs.style   = WS_CLIPCHILDREN|WS_CHILD;                // window style
	mcs.lParam  = 0;                // lparam

	hwndChild = (HWND) SendMessage(hwndMDIClient, WM_MDICREATE, 0, (LPARAM)(LPMDICREATESTRUCT) &mcs);

	if (hwndChild != NULL)
		ShowWindow(hwndChild, SW_SHOW);

	return hwndChild;
}

int MpegPrepareForClose(HWND hwnd)
{
	MPEGWINDOW_INFO *mpegWindowInfo;

	mpegWindowInfo=(MPEGWINDOW_INFO *)GetWindowLong(hwnd, GWL_USERDATA);	//get the point to window info

	//Signal to close the thread. Then wait.
	if (mpegWindowInfo->fileInfo.loadingInBackground)	{
		mpegWindowInfo->fileInfo.stopThreadNow=TRUE;
		WaitForSingleObject(mpegWindowInfo->fileInfo.hBackgroundThread, INFINITE);
	}

	//Close the file
	CloseHandle(mpegWindowInfo->fileInfo.hMpegFile);

	//Free the window structure
	free(mpegWindowInfo);

	return 0;
}

LRESULT CALLBACK MpegFileInfoProc(HWND hwnd,UINT msg,WPARAM wParam,LPARAM lParam)
{

	switch(msg) {
		case WM_PAINT:
			MpegFileInfoPaint(hwnd);
			break;
		case WM_ERASEBKGND:
			return 1;
			break;
	}

	return DefMDIChildProc(hwnd, msg, wParam, lParam);
}

LRESULT CALLBACK MpegFileDetailProc(HWND hwnd,UINT msg,WPARAM wParam,LPARAM lParam)
{

	switch(msg) {
		case WM_PAINT:
			MpegFileDetailPaint(hwnd);
			break;
		case WM_ERASEBKGND:
			return 1;
			break;
	}

	return DefMDIChildProc(hwnd, msg, wParam, lParam);
}

LRESULT CALLBACK MpegPacketInfoProc(HWND hwnd,UINT msg,WPARAM wParam,LPARAM lParam)
{
	HINSTANCE hInst;
	MPEGWINDOW_INFO *mpegWindowInfo;

	switch(msg) {
		case WM_CREATE:
			hInst=((LPCREATESTRUCT)lParam)->hInstance;	//get the hinstance for use when creating the child window

			mpegWindowInfo=(MPEGWINDOW_INFO *)GetWindowLong(GetParent(hwnd), GWL_USERDATA);	//get the point to window info
			mpegWindowInfo->hwndNextButton =	CreateWindow("BUTTON", "Next",
					   					WS_CHILD|WS_VISIBLE|WS_TABSTOP|WS_BORDER|BS_PUSHBUTTON,
									    20,50,60,30, hwnd, NULL, hInst, NULL);
			break;

		case WM_PAINT:
			MpegPacketInfoPaint(hwnd);
			break;
		case WM_ERASEBKGND:
			return 1;
			break;
		case WM_COMMAND:
			unsigned long tempOffset;
			mpegWindowInfo=(MPEGWINDOW_INFO *)GetWindowLong(GetParent(hwnd), GWL_USERDATA);	//get the point to window info
			if (lParam==(LPARAM)mpegWindowInfo->hwndNextButton)	{
				if (mpegWindowInfo->fileInfo.loadingInBackground)	{
					WaitForSingleObject(mpegWindowInfo->fileInfo.hFileAccessMutex,INFINITE);

					tempOffset=mpegWindowInfo->fileInfo.offset;	//remember the offset

					mpegWindowInfo->fileInfo.offset=mpegWindowInfo->fileInfo.displayOffset; //move to the display offset
					SetFilePointer(mpegWindowInfo->fileInfo.hMpegFile, mpegWindowInfo->fileInfo.displayOffset, 0, FILE_BEGIN);
					MpegReadPacket(&mpegWindowInfo->fileInfo, &mpegWindowInfo->displayedPacket);//readit
					mpegWindowInfo->fileInfo.displayOffset=mpegWindowInfo->fileInfo.offset;

					mpegWindowInfo->fileInfo.offset=tempOffset;	//set the offset back to what it was
					SetFilePointer(mpegWindowInfo->fileInfo.hMpegFile, tempOffset,0, FILE_BEGIN);

					ReleaseMutex(mpegWindowInfo->fileInfo.hFileAccessMutex);
				}
				else	{
					mpegWindowInfo->fileInfo.offset=mpegWindowInfo->fileInfo.displayOffset;
					MpegReadPacket(&mpegWindowInfo->fileInfo, &mpegWindowInfo->displayedPacket);
					mpegWindowInfo->fileInfo.displayOffset=mpegWindowInfo->fileInfo.offset;
				}

				InvalidateRect(hwnd, NULL, FALSE);
			}
			break;

	}

	return DefMDIChildProc(hwnd, msg, wParam, lParam);
}

int MpegFileInfoPaint(HWND hwnd)
{
	MPEGWINDOW_INFO *mpegWindowInfo;

	HDC	hdc;
	PAINTSTRUCT ps;
	RECT clientRect;
	RECT outputRect;

	HFONT hMidFont;
	TEXTMETRIC textMetric;

	char buffer[255];
	int y=ZORVEWINDOWMARGIN;
	int heightMidFont=16;

	mpegWindowInfo=(MPEGWINDOW_INFO *)GetWindowLong(GetParent(hwnd), GWL_USERDATA);	//get the point to window info

	GetClientRect(hwnd, &clientRect);
	hdc=BeginPaint(hwnd, &ps);

	SetBkColor(hdc, RGB_ZINNY_MIDPURPLE);
	SetTextColor(hdc, RGB_ZINNY_BLACK);

	hMidFont = CreateFont(
				MulDiv(10, GetDeviceCaps(hdc, LOGPIXELSY), 72),
				0,0,0,FW_NORMAL,
				FALSE,FALSE,FALSE,
				DEFAULT_CHARSET,OUT_OUTLINE_PRECIS,
                CLIP_DEFAULT_PRECIS,NONANTIALIASED_QUALITY, VARIABLE_PITCH|FF_SWISS,"MS Shell Dlg");
	SelectObject(hdc,hMidFont);


	GetTextMetrics(hdc, &textMetric);
	heightMidFont= textMetric.tmHeight;


	outputRect.left=clientRect.left;
	outputRect.right=clientRect.right;
	outputRect.top=clientRect.top;
	outputRect.bottom=clientRect.top+y+heightMidFont;

	//File name
	sprintf(buffer, "%s",  mpegWindowInfo->fileInfo.filename);
	ExtTextOut(hdc, ZORVEWINDOWMARGIN,y,ETO_OPAQUE, &outputRect, buffer, strlen(buffer), NULL);
	y+=heightMidFont;

	//Size
	outputRect.top=y;
	outputRect.bottom=y+heightMidFont;
	sprintf(buffer, "Size: %u bytes",  mpegWindowInfo->fileInfo.filesize);
	ExtTextOut(hdc, ZORVEWINDOWMARGIN,y,ETO_OPAQUE, &outputRect, buffer, strlen(buffer), NULL);
	y+=heightMidFont;

	//Blocks
	outputRect.top=y;
	outputRect.bottom=y+heightMidFont;
	if (mpegWindowInfo->fileInfo.loadingInBackground)
		sprintf(buffer, "Packets: %u (est.)",  mpegWindowInfo->fileInfo.filesize/188);
	else
		sprintf(buffer, "Packets: %u",  mpegWindowInfo->fileInfo.countPackets);

	ExtTextOut(hdc, ZORVEWINDOWMARGIN,y,ETO_OPAQUE, &outputRect, buffer, strlen(buffer), NULL);
	y+=heightMidFont;



	//Fill to the bottom of this window
	outputRect.top=y;
	outputRect.bottom=clientRect.bottom;
	ExtTextOut(hdc, 0,0,ETO_OPAQUE, &outputRect, NULL, 0, NULL);

	EndPaint(hwnd, &ps);

	return 0;
}

int MpegFileDetailPaint(HWND hwnd)
{
	MPEGWINDOW_INFO *mpegWindowInfo;

	HDC	hdc;
	PAINTSTRUCT ps;
	RECT clientRect;
	RECT outputRect;

	HFONT hMidFont;
	TEXTMETRIC textMetric;

	char buffer[255];
	int y=ZORVEWINDOWMARGIN;
	int heightMidFont=16;

	mpegWindowInfo=(MPEGWINDOW_INFO *)GetWindowLong(GetParent(hwnd), GWL_USERDATA);	//get the point to window info

	GetClientRect(hwnd, &clientRect);
	hdc=BeginPaint(hwnd, &ps);

	SetBkColor(hdc, RGB_ZINNY_MIDPURPLE);
	SetTextColor(hdc, RGB_ZINNY_BLACK);

	hMidFont = CreateFont(
				MulDiv(10, GetDeviceCaps(hdc, LOGPIXELSY), 72),
				0,0,0,FW_NORMAL,
				FALSE,FALSE,FALSE,
				DEFAULT_CHARSET,OUT_OUTLINE_PRECIS,
                CLIP_DEFAULT_PRECIS,NONANTIALIASED_QUALITY, VARIABLE_PITCH|FF_SWISS,"MS Shell Dlg");
	SelectObject(hdc,hMidFont);


	GetTextMetrics(hdc, &textMetric);
	heightMidFont= textMetric.tmHeight;


	outputRect.left=clientRect.left;
	outputRect.right=clientRect.right;
	outputRect.top=clientRect.top;
	outputRect.bottom=clientRect.top+y+heightMidFont;

	//Percent scanned
	sprintf(buffer, "Scanning: %i%%", mpegWindowInfo->fileInfo.countPackets*100/(mpegWindowInfo->fileInfo.filesize/188));
	ExtTextOut(hdc, ZORVEWINDOWMARGIN,y,ETO_OPAQUE, &outputRect, buffer, strlen(buffer), NULL);
	y+=heightMidFont;

	//Size
//	outputRect.top=y;
//	outputRect.bottom=y+heightMidFont;
//	sprintf(buffer, "Size: %u bytes",  mpegWindowInfo->fileInfo.filesize);
//	ExtTextOut(hdc, ZORVEWINDOWMARGIN,y,ETO_OPAQUE, &outputRect, buffer, strlen(buffer), NULL);
//	y+=heightMidFont;


	//Fill to the bottom of this window
	outputRect.top=y;
	outputRect.bottom=clientRect.bottom;
	ExtTextOut(hdc, 0,0,ETO_OPAQUE, &outputRect, NULL, 0, NULL);

	EndPaint(hwnd, &ps);

	return 0;
}

int MpegPacketInfoPaint(HWND hwnd)
{
	MPEGWINDOW_INFO *mpegWindowInfo;

	HDC	hdc;
	PAINTSTRUCT ps;
	RECT clientRect;
	RECT outputRect;

	HFONT hMidFont;
	TEXTMETRIC textMetric;

	char buffer[255];
	int y=ZORVEWINDOWMARGIN;
	int heightMidFont=16;

	mpegWindowInfo=(MPEGWINDOW_INFO *)GetWindowLong(GetParent(hwnd), GWL_USERDATA);	//get the point to window info

	GetClientRect(hwnd, &clientRect);
	hdc=BeginPaint(hwnd, &ps);

	SetBkColor(hdc, RGB_ZINNY_MIDPURPLE);
	SetTextColor(hdc, RGB_ZINNY_BLACK);

	hMidFont = CreateFont(
				MulDiv(10, GetDeviceCaps(hdc, LOGPIXELSY), 72),
				0,0,0,FW_NORMAL,
				FALSE,FALSE,FALSE,
				DEFAULT_CHARSET,OUT_OUTLINE_PRECIS,
                CLIP_DEFAULT_PRECIS,NONANTIALIASED_QUALITY, VARIABLE_PITCH|FF_SWISS,"MS Shell Dlg");
	SelectObject(hdc,hMidFont);


	GetTextMetrics(hdc, &textMetric);
	heightMidFont= textMetric.tmHeight;


	outputRect.left=clientRect.left;
	outputRect.right=clientRect.right;
	outputRect.top=clientRect.top;
	outputRect.bottom=clientRect.top+y+heightMidFont;

	//Block number
	sprintf(buffer, "Block: %u", mpegWindowInfo->fileInfo.displayOffset/188);
	ExtTextOut(hdc, ZORVEWINDOWMARGIN,y,ETO_OPAQUE, &outputRect, buffer, strlen(buffer), NULL);
	y+=heightMidFont;

	//Size
	outputRect.top=y;
	outputRect.bottom=y+heightMidFont;
	sprintf(buffer, "PID: $%03x", mpegWindowInfo->displayedPacket.pid);
	ExtTextOut(hdc, ZORVEWINDOWMARGIN,y,ETO_OPAQUE, &outputRect, buffer, strlen(buffer), NULL);
	y+=heightMidFont;


	//Fill to the bottom of this window
	outputRect.top=y;
	outputRect.bottom=clientRect.bottom;
	ExtTextOut(hdc, 0,0,ETO_OPAQUE, &outputRect, NULL, 0, NULL);

	EndPaint(hwnd, &ps);

	return 0;
}
