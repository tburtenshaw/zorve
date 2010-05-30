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

	wc.style         = 0;
	wc.lpfnWndProc   = (WNDPROC)MpegPacketDetailProc;
	wc.cbClsExtra    = 0;
	wc.cbWndExtra    = 20;
	wc.hInstance     = hInst;                      // Owner of this class
	wc.hIcon         = NULL;
	wc.hCursor       = LoadCursor(NULL, IDC_ARROW);
	wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1); // Default color
	wc.lpszMenuName  = NULL;
	wc.lpszClassName = "MpegPacketDetailClass";

	if (!RegisterClass((LPWNDCLASS)&wc))
		return 0;

	return 1;
}

LRESULT CALLBACK ChildWndMpegProc(HWND hwnd,UINT msg,WPARAM wparam,LPARAM lparam)
{
	HINSTANCE hInst;
	MPEGWINDOW_INFO *mpegWindowInfo;

	switch(msg) {
		case WM_CREATE:
			hInst=((LPCREATESTRUCT)lparam)->hInstance;	//get the hinstance for use when creating the child window

			mpegWindowInfo=malloc(sizeof(MPEGWINDOW_INFO));
			SetWindowLong(hwnd, GWL_USERDATA, (long)mpegWindowInfo); //set the custom long of the window to remember it
			memset(mpegWindowInfo, 0, sizeof(MPEGWINDOW_INFO));	//make sure everything set to zero

			mpegWindowInfo->zorveHwnd = GetParent(GetParent(hwnd));

			mpegWindowInfo->hwndFileInfo = CreateWindow("MpegFileInfoClass", "fileinfo",
										WS_CHILD|WS_VISIBLE,
										0,0,300,100, hwnd, NULL, hInst,NULL);
			mpegWindowInfo->hwndFileDetail = CreateWindow("MpegFileDetailClass", "filedetail",
										WS_CHILD|WS_VISIBLE,
										300,0,200,100, hwnd, NULL, hInst,NULL);
			mpegWindowInfo->hwndBlockInfo = CreateWindow("MpegPacketInfoClass", "blockinfo",
										WS_CHILD|WS_VISIBLE,
										0,100,250,200, hwnd, NULL, hInst,NULL);
			mpegWindowInfo->hwndBlockDetail = CreateWindow("MpegPacketDetailClass", "blockinfo",
										WS_CHILD|WS_VISIBLE,
										200,100,250,200, hwnd, NULL, hInst,NULL);

			//for now, the hexview is the blockdetail
			mpegWindowInfo->hwndHexView	= mpegWindowInfo->hwndBlockDetail;

			break;
		case ZM_REPLY_RECORDINGNAME:
			mpegWindowInfo=(MPEGWINDOW_INFO *)GetWindowLong(hwnd, GWL_USERDATA);	//get the point to window info
			sprintf(mpegWindowInfo->recordingname, "%s", (char *)wparam);
			break;
		case WM_ERASEBKGND:
			return 1;
			break;
		case WM_SIZE:
			RECT clientRect;
			int heightTopRow=100;
			int minWidthFirstCell=250;
			int minWidthBottomLeftCell=300;
			mpegWindowInfo=(MPEGWINDOW_INFO *)GetWindowLong(hwnd, GWL_USERDATA);	//get the point to window info

			GetClientRect(hwnd, &clientRect);
			MoveWindow(mpegWindowInfo->hwndFileInfo,0,0,minWidthFirstCell, heightTopRow, TRUE);
			MoveWindow(mpegWindowInfo->hwndFileDetail,minWidthFirstCell,0,clientRect.right-minWidthFirstCell,heightTopRow,TRUE);
			MoveWindow(mpegWindowInfo->hwndBlockInfo,0,heightTopRow,minWidthBottomLeftCell,clientRect.bottom-heightTopRow,TRUE);
			MoveWindow(mpegWindowInfo->hwndBlockDetail,minWidthBottomLeftCell,heightTopRow,clientRect.right-minWidthBottomLeftCell,clientRect.bottom-heightTopRow,TRUE);
			break;
		case WM_DESTROY:
			MpegPrepareForClose(hwnd);
			break;
	}
	return DefMDIChildProc(hwnd, msg, wparam, lparam);
}

HANDLE MpegWindowLoadFile(HWND hwnd, char * mpegFile)
{
	MPEGWINDOW_INFO *mpegWindowInfo;
	unsigned long tempFilesizeHigh;

	mpegWindowInfo=(MPEGWINDOW_INFO *)GetWindowLong(hwnd, GWL_USERDATA);

	//Check if there is already a file open and unload it
	if (mpegWindowInfo->fileInfo.hMpegFile)	{
		CloseHandle(mpegWindowInfo->fileInfo.hMpegFile);
		//reset everything that should be reset (we can't memset windowinfo to zero, as we need to retain hwnds
		memset(&mpegWindowInfo->fileInfo, 0, sizeof(MPEGFILE_INFO));
		memset(&mpegWindowInfo->displayedPacket, 0, sizeof(TS_PACKET));
	}


	lstrcpy(mpegWindowInfo->fileInfo.filename, mpegFile);

	SendMessage(mpegWindowInfo->zorveHwnd, ZM_REQUEST_RECORDINGNAME, (WPARAM)mpegWindowInfo->fileInfo.filename, (LPARAM)hwnd);

	//Load as 'sequential' for caching.
	mpegWindowInfo->fileInfo.hMpegFile = CreateFile(mpegFile, GENERIC_READ, FILE_SHARE_READ, NULL,
				OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL | FILE_FLAG_SEQUENTIAL_SCAN, NULL);

	if (mpegWindowInfo->fileInfo.hMpegFile == INVALID_HANDLE_VALUE)
		return INVALID_HANDLE_VALUE;

	mpegWindowInfo->fileInfo.filesize=GetFileSize(mpegWindowInfo->fileInfo.hMpegFile, &tempFilesizeHigh);
	mpegWindowInfo->fileInfo.filesize+=tempFilesizeHigh * (0x100000000);

	mpegWindowInfo->fileInfo.firstSyncByte = 0;	//set the start seek position to zero
	MpegTSFindSyncByte(mpegWindowInfo->fileInfo.hMpegFile, &mpegWindowInfo->fileInfo.firstSyncByte);
	mpegWindowInfo->fileInfo.offset=mpegWindowInfo->fileInfo.firstSyncByte;

	//Now start a thread that gradually loads in statistical information about the mpeg.
	mpegWindowInfo->fileInfo.hFileAccessMutex = CreateMutex(NULL, FALSE, "fileaccessmutex");	//mutex for accessing the file, so can jump to offsets while scanning
	mpegWindowInfo->fileInfo.hBackgroundThread = CreateThread(NULL, (SIZE_T)0, (LPTHREAD_START_ROUTINE)MpegReadFileStats, &mpegWindowInfo->fileInfo, 0, NULL);

	SetTimer(mpegWindowInfo->hwndFileDetail, IDT_MPEG_SCANDISPLAY, 300, NULL);

	//MpegReadFileStats(&mpegWindowInfo->fileInfo);	//override the background thread for debugging

	return mpegWindowInfo->fileInfo.hMpegFile;
}

DWORD WINAPI MpegReadFileStats(MPEGFILE_INFO *mpegFileInfo)
{
	TS_PACKET packet;
	ULONGLONG offset;
	int pTry;
	int stay;

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

	mpegFileInfo->loadingInBackground = FALSE;
	return 0;
}

int MpegTSFindSyncByte(HANDLE hFile, ULONGLONG * syncbyteOffset)
{
	BYTE testbyte;
	long n;

	BOOL readResult;
	int occurence;
	LARGE_INTEGER offset;
	int justskipped;	//if we have just found one and skipped some bytes, we'll go back

	LARGE_INTEGER largeint_filesize;
	long highZero=0;	//used as a high order rather than null

	GetFileSizeEx(hFile, &largeint_filesize);

	offset.QuadPart=*syncbyteOffset;	//the syncbyte offset will be our starting position
	occurence = 0;
	justskipped=0;

	SetFilePointer(hFile, offset.LowPart, &offset.HighPart, FILE_BEGIN);

	readResult = ReadFile(hFile, &testbyte, 1, &n, NULL);
	while ((readResult) && (offset.QuadPart<largeint_filesize.QuadPart))	{
		if (testbyte == 0x47)	{
			occurence++;
			if (occurence==5)	{
				offset.QuadPart -= 752;
				*syncbyteOffset = offset.QuadPart;
				SetFilePointer(hFile, offset.LowPart, &offset.HighPart, FILE_BEGIN);
				return 1;
			}
			offset.QuadPart+=188;
			justskipped=1;
			readResult = SetFilePointer(hFile, 187, &highZero, FILE_CURRENT);
			if (readResult==INVALID_SET_FILE_POINTER)
				return 0;
			readResult = ReadFile(hFile, &testbyte, 1, &n, NULL);
		}
		else	{
			if (justskipped)	{
				offset.QuadPart-=188;
				readResult = SetFilePointer(hFile, -188, &highZero, FILE_CURRENT);
				if (readResult==INVALID_SET_FILE_POINTER)
					return 0;
				justskipped=0;
			}
			occurence=0;
			offset.QuadPart++;

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
	packet->transporterror =	(packet->TS_raw_packet[1] & 0x80) >> 7;
	packet->payloadstart =		(packet->TS_raw_packet[1] & 0x40) >> 6;
	packet->transportpriority =	(packet->TS_raw_packet[1] & 0x20) >> 5;
	packet->pid	= ((packet->TS_raw_packet[1] & 0b11111) << 8) | packet->TS_raw_packet[2];
	packet->scrambling	=		(packet->TS_raw_packet[3] & 0b11000000) >> 6;
	packet->adaptation	= 		(packet->TS_raw_packet[3] & 0b00110000) >> 4;
	packet->continuitycounter= 	(packet->TS_raw_packet[3] & 0b00001111);

	if (packet->adaptation & 0b10)	{
		packet->adaptationfield.adaptationfieldlength = packet->TS_raw_packet[4];
		packet->adaptationfield.discontinuity =		(packet->TS_raw_packet[5] & 0b10000000) >> 7;
		packet->adaptationfield.randomaccess =		(packet->TS_raw_packet[5] & 0b01000000) >> 6;
		packet->adaptationfield.EsPriority =		(packet->TS_raw_packet[5] & 0b00100000) >> 5;
		packet->adaptationfield.PCRFlag =			(packet->TS_raw_packet[5] & 0b00010000) >> 4;
		packet->adaptationfield.OPCRFlag =			(packet->TS_raw_packet[5] & 0b00001000) >> 3;
		packet->adaptationfield.splicingpointflag =	(packet->TS_raw_packet[5] & 0b00000100) >> 2;
		packet->adaptationfield.transportprivatedataflag =		(packet->TS_raw_packet[5] & 0b00000010) >> 1;
		packet->adaptationfield.adaptationfieldextensionflag =	(packet->TS_raw_packet[5] & 0b00000001);

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
			//this was causing crash, transportprivatedatalength was longer than it should be! (needs fix)
			//memcpy(&packet->adaptationfield.privatedata, &packet->TS_raw_packet[b+1], packet->adaptationfield.transportprivatedatalength);
			b+=packet->adaptationfield.transportprivatedatalength+1;
		}
		if (packet->adaptationfield.adaptationfieldextensionflag)	{
			packet->adaptationfield.adaptationfieldextensionlength = packet->TS_raw_packet[b];
			b++;
			offsetAtAFExt=b;
			packet->adaptationfield.ltwflag	= (packet->TS_raw_packet[b] & 0b10000000) >> 7;
			packet->adaptationfield.piecewiserateflag	= (packet->TS_raw_packet[b] & 0b01000000) >> 6;
			packet->adaptationfield.seamlessspliceflag	= (packet->TS_raw_packet[b] & 0b00100000) >> 5;
			b++;
			if (packet->adaptationfield.ltwflag)	{
				packet->adaptationfield.ltwflag	= (packet->TS_raw_packet[b] & 0b10000000) >> 7;
				packet->adaptationfield.ltwoffset = (LONG)(packet->TS_raw_packet[b]&0b01111111) << 8;
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
				packet->adaptationfield.splicetype=	(packet->TS_raw_packet[b] & 0b11110000) >> 4;
				packet->adaptationfield.DTSnextAU=	(packet->TS_raw_packet[b] & 0b00001110);
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
	MPEGWINDOW_INFO *mpegWindowInfo;

	switch(msg) {
		case WM_TIMER:
			if (wParam == IDT_MPEG_SCANDISPLAY)	{
				//Every so often (depending on what the timer is set to) we update the display details
				//We must also redraw the window, so we're not redrawing only half new things
				mpegWindowInfo=(MPEGWINDOW_INFO *)GetWindowLong(GetParent(hwnd), GWL_USERDATA);	//get the point to window info
				InvalidateRect(hwnd, NULL, FALSE);
				if (!mpegWindowInfo->fileInfo.loadingInBackground)	{	//if we're no longer loading
					KillTimer(hwnd, IDT_MPEG_SCANDISPLAY); //then kill the timer
					InvalidateRect(mpegWindowInfo->hwndFileInfo, NULL, FALSE);	//and redraw the fileinfo part
				}

			}
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
	HFONT	hSmallFont;

	switch(msg) {
		case WM_CREATE:
			hInst=((LPCREATESTRUCT)lParam)->hInstance;	//get the hinstance for use when creating the child window

			mpegWindowInfo=(MPEGWINDOW_INFO *)GetWindowLong(GetParent(hwnd), GWL_USERDATA);	//get the point to window info
			mpegWindowInfo->hwndNextButton =	CreateWindow("BUTTON", "Next",
					   					WS_CHILD|WS_VISIBLE|WS_TABSTOP|WS_BORDER|BS_PUSHBUTTON,
									    100,90,60,23, hwnd, NULL, hInst, NULL);
			mpegWindowInfo->hwndPrevButton =	CreateWindow("BUTTON", "Prev",
					   					WS_CHILD|WS_VISIBLE|WS_TABSTOP|WS_BORDER|BS_PUSHBUTTON,
									    20,90,60,23, hwnd, NULL, hInst, NULL);

			mpegWindowInfo->hwndPositionEditbox =	CreateWindow("EDIT", "Position",
				    					WS_CHILD|WS_VISIBLE|ES_LEFT|WS_TABSTOP|WS_BORDER,
									    30,60,80,18, hwnd, NULL, hInst, NULL);
			SendMessage(mpegWindowInfo->hwndPositionEditbox, EM_LIMITTEXT, 20, 0);	//shouldn't be any offset longer than 20 digits

			mpegWindowInfo->hwndSeekButton =	CreateWindow("BUTTON", "Seek",
					   					WS_CHILD|WS_VISIBLE|WS_TABSTOP|WS_BORDER|BS_PUSHBUTTON,
									    120,60,60,23, hwnd, NULL, hInst, NULL);




			hSmallFont = CreateFont(
				MulDiv(10, GetDeviceCaps(GetDC(hwnd), LOGPIXELSY), 72),
				0,0,0,FW_NORMAL,
				FALSE,FALSE,FALSE,
				DEFAULT_CHARSET,OUT_OUTLINE_PRECIS,
                CLIP_DEFAULT_PRECIS,NONANTIALIASED_QUALITY, VARIABLE_PITCH|FF_SWISS,"MS Shell Dlg");
			//NEED TO DESTROY THIS FONT ONCE WINDOW CLOSES (consider moving it to top mpegts window, and sharing it)


			SendMessage(mpegWindowInfo->hwndNextButton, WM_SETFONT, (WPARAM)hSmallFont, 0);
			SendMessage(mpegWindowInfo->hwndPrevButton, WM_SETFONT, (WPARAM)hSmallFont, 0);
			SendMessage(mpegWindowInfo->hwndPositionEditbox, WM_SETFONT, (WPARAM)hSmallFont, 0);
			SendMessage(mpegWindowInfo->hwndSeekButton, WM_SETFONT, (WPARAM)hSmallFont, 0);

			break;

		case WM_PAINT:
			MpegPacketInfoPaint(hwnd);
			break;
		case WM_ERASEBKGND:
			return 1;
			break;
		case WM_COMMAND:
			mpegWindowInfo=(MPEGWINDOW_INFO *)GetWindowLong(GetParent(hwnd), GWL_USERDATA);	//get the point to window info
			if (MpegChangePacket(mpegWindowInfo, lParam))	{
				InvalidateRect(hwnd, NULL, FALSE);
				InvalidateRect(mpegWindowInfo->hwndHexView, NULL, FALSE);
			}
			break;

	}

	return DefMDIChildProc(hwnd, msg, wParam, lParam);
}

LRESULT CALLBACK MpegPacketDetailProc(HWND hwnd,UINT msg,WPARAM wParam,LPARAM lParam)
{

	switch(msg) {
//		case WM_ERASEBKGND:
//			return 1;
//			break;
		case WM_PAINT:
			MpegHexView(hwnd);
			break;
	}

	return DefMDIChildProc(hwnd, msg, wParam, lParam);
}

int MpegHexView(HWND hwnd)
{
	MPEGWINDOW_INFO *mpegWindowInfo;

	HDC hdc;
	PAINTSTRUCT ps;
	RECT clientRect;
	RECT outputRect;

	TS_PACKET * packet;

	HFONT hFixedFont;

	int i;
	int row,col;

	int x,y;

	char hex[3];

	mpegWindowInfo=(MPEGWINDOW_INFO *)GetWindowLong(GetParent(hwnd), GWL_USERDATA);	//get the point to window info
	packet=&mpegWindowInfo->displayedPacket;

	GetClientRect(hwnd, &clientRect);
	hdc=BeginPaint(hwnd, &ps);

	hFixedFont = CreateFont(
				MulDiv(10, GetDeviceCaps(hdc, LOGPIXELSY), 72),
				0,0,0,FW_NORMAL,
				FALSE,FALSE,FALSE,
				DEFAULT_CHARSET,OUT_OUTLINE_PRECIS,
                CLIP_DEFAULT_PRECIS,NONANTIALIASED_QUALITY, FIXED_PITCH|FF_DONTCARE,"Courier New");
	SelectObject(hdc,hFixedFont);


	x=0;y=0;
	for (i=0;i<188;i++)	{
		sprintf(hex, "%02X", packet->TS_raw_packet[i]);
		outputRect.left=x;
		outputRect.top=y;
		outputRect.right=x+24;
		outputRect.bottom=y+16;
		ExtTextOut(hdc, x,y,ETO_OPAQUE, &outputRect, hex, strlen(hex), NULL);
		x+=24;
		if ((i % 16) == 15)	{
			y+=16;
			x=0;
		}
	}

	DeleteObject(hFixedFont);
	EndPaint(hwnd, &ps);

	return 0;

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
	int len;

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

	//Recording name if exists
	if (mpegWindowInfo->recordingname[0])	{
		outputRect.top=y;
		outputRect.bottom=y+heightMidFont;
		sprintf(buffer, "%s",  mpegWindowInfo->recordingname);
		ExtTextOut(hdc, ZORVEWINDOWMARGIN,y,ETO_OPAQUE, &outputRect, buffer, strlen(buffer), NULL);
		y+=heightMidFont;
	}

	//Size
	outputRect.top=y;
	outputRect.bottom=y+heightMidFont;
	sprintf(buffer, "Size: "); //len=6
	len = UnsignedLongLongToString(mpegWindowInfo->fileInfo.filesize, buffer+6);
	sprintf(buffer+6+len+1, " bytes");
	ExtTextOut(hdc, ZORVEWINDOWMARGIN,y,ETO_OPAQUE, &outputRect, buffer, strlen(buffer), NULL);
	y+=heightMidFont;

	//Blocks
	outputRect.top=y;
	outputRect.bottom=y+heightMidFont;
	if (mpegWindowInfo->fileInfo.loadingInBackground)	{
		sprintf(buffer, "Packets: ");	//len=9
		len = UnsignedLongLongToString(mpegWindowInfo->fileInfo.filesize/188, buffer+9);
		sprintf(buffer+9+len, " (est.)");
	}

	else
		sprintf(buffer, "Packets: %u",  mpegWindowInfo->fileInfo.countPackets);

	ExtTextOut(hdc, ZORVEWINDOWMARGIN,y,ETO_OPAQUE, &outputRect, buffer, strlen(buffer), NULL);
	y+=heightMidFont;



	//Fill to the bottom of this window
	outputRect.top=y;
	outputRect.bottom=clientRect.bottom;
	ExtTextOut(hdc, 0,0,ETO_OPAQUE, &outputRect, NULL, 0, NULL);

	DeleteObject(hMidFont);
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
	int i;

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
	if (((mpegWindowInfo->fileInfo.countPackets>0) && (!mpegWindowInfo->fileInfo.loadingInBackground)) || (mpegWindowInfo->fileInfo.filesize<189))
		sprintf(buffer, "Scanned");
	else
		sprintf(buffer, "Scanning: %i%%", (int)(mpegWindowInfo->fileInfo.countPackets*100/(mpegWindowInfo->fileInfo.filesize/188)));

	ExtTextOut(hdc, ZORVEWINDOWMARGIN,y,ETO_OPAQUE, &outputRect, buffer, strlen(buffer), NULL);
	y+=heightMidFont;

	i=0;
	while (mpegWindowInfo->fileInfo.seenPid[i])	{
		outputRect.top=y;
		outputRect.bottom=y+heightMidFont;
		sprintf(buffer, "PID $%03x: %u instances (%3.2f%%)", mpegWindowInfo->fileInfo.seenPid[i], mpegWindowInfo->fileInfo.countPid[i], (float)mpegWindowInfo->fileInfo.countPid[i]*100/(float)mpegWindowInfo->fileInfo.countPackets);
		ExtTextOut(hdc, ZORVEWINDOWMARGIN,y,ETO_OPAQUE, &outputRect, buffer, strlen(buffer), NULL);
		y+=heightMidFont;
		i++;
	}


	//Fill to the bottom of this window
	outputRect.top=y;
	outputRect.bottom=clientRect.bottom;
	ExtTextOut(hdc, 0,0,ETO_OPAQUE, &outputRect, NULL, 0, NULL);

	DeleteFont(hMidFont);
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

	HFONT hLargeFont;
	HFONT hMidFont;
	HFONT hSmallFont;
	TEXTMETRIC textMetric;

	SIZE textSize;

	char buffer[255];
	int y=ZORVEWINDOWMARGIN;
	int x;
	int heightSmallFont=12;
	int heightMidFont=16;

	mpegWindowInfo=(MPEGWINDOW_INFO *)GetWindowLong(GetParent(hwnd), GWL_USERDATA);	//get the point to window info

	GetClientRect(hwnd, &clientRect);
	hdc=BeginPaint(hwnd, &ps);

	SetBkColor(hdc, RGB_ZINNY_MIDPURPLE);
	SetTextColor(hdc, RGB_ZINNY_BLACK);

	hSmallFont = CreateFont(
				MulDiv(8, GetDeviceCaps(hdc, LOGPIXELSY), 72),
				0,0,0,FW_NORMAL,
				FALSE,FALSE,FALSE,
				DEFAULT_CHARSET,OUT_OUTLINE_PRECIS,
                CLIP_DEFAULT_PRECIS,NONANTIALIASED_QUALITY, VARIABLE_PITCH|FF_SWISS,"Arial");

	hMidFont = CreateFont(
				MulDiv(10, GetDeviceCaps(hdc, LOGPIXELSY), 72),
				0,0,0,FW_NORMAL,
				FALSE,FALSE,FALSE,
				DEFAULT_CHARSET,OUT_OUTLINE_PRECIS,
                CLIP_DEFAULT_PRECIS,NONANTIALIASED_QUALITY, VARIABLE_PITCH|FF_SWISS,"MS Shell Dlg");
	hLargeFont = CreateFont(
				MulDiv(12, GetDeviceCaps(hdc, LOGPIXELSY), 72),
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

	//Packet number
	sprintf(buffer, "Packet: %u (", (long)(mpegWindowInfo->fileInfo.displayOffset/188));
	UnsignedLongLongToString((mpegWindowInfo->fileInfo.displayOffset)-188, buffer+strlen(buffer));
	ExtTextOut(hdc, ZORVEWINDOWMARGIN, y, ETO_OPAQUE, &outputRect, buffer, strlen(buffer), NULL);
	y+=heightMidFont;

	//Initial bytes/bits
	SelectObject(hdc,hSmallFont);
	GetTextMetrics(hdc, &textMetric);	//get height
	heightSmallFont= textMetric.tmHeight;

	outputRect.top=y;
	outputRect.bottom=y+heightMidFont;

	GetTextExtentPoint32(hdc, "SYNC", 4, &textSize);

	outputRect.left=clientRect.left;
	outputRect.right=outputRect.left+textSize.cx;

	x=ZORVEWINDOWMARGIN;
	if (mpegWindowInfo->displayedPacket.syncbyte ==0x47)
		SetTextColor(hdc, RGB_ZINNY_COLOUR_SYNC);
	else
		SetTextColor(hdc, RGB_ZINNY_COLOUR_SYNC_FADE);

	ExtTextOut(hdc, x, y, ETO_OPAQUE, &outputRect, "SYNC", 4, NULL);
	x+=textSize.cx;

	GetTextExtentPoint32(hdc, "ERROR", 5, &textSize);
	outputRect.left=x;	outputRect.right=x+textSize.cx;

	if (mpegWindowInfo->displayedPacket.transporterror)
		SetTextColor(hdc, RGB_ZINNY_COLOUR_ERROR);
	else
		SetTextColor(hdc, RGB_ZINNY_COLOUR_ERROR_FADE);

	ExtTextOut(hdc, x, y, ETO_OPAQUE, &outputRect, "ERROR", 5, NULL);
	x+=textSize.cx;

	GetTextExtentPoint32(hdc, "PAYLOAD", 7, &textSize);
	outputRect.left=x;	outputRect.right=x+textSize.cx;
	if (mpegWindowInfo->displayedPacket.payloadstart)
		SetTextColor(hdc, RGB_ZINNY_COLOUR_PAYLOAD);
	else
		SetTextColor(hdc, RGB_ZINNY_COLOUR_PAYLOAD_FADE);

	ExtTextOut(hdc, x, y, ETO_OPAQUE, &outputRect, "PAYLOAD", 7, NULL);
	x+=textSize.cx;

	outputRect.left=x;	outputRect.right=clientRect.right;
	if (mpegWindowInfo->displayedPacket.transportpriority)
		SetTextColor(hdc, RGB_ZINNY_COLOUR_PRIORITY);
	else
		SetTextColor(hdc, RGB_ZINNY_COLOUR_PRIORITY_FADE);

	ExtTextOut(hdc, x, y, ETO_OPAQUE, &outputRect, "PRIORITY", 8, NULL);


	y+=heightSmallFont;

	//PID
	SetTextColor(hdc, RGB_ZINNY_BLACK);
	SelectObject(hdc,hMidFont);
	outputRect.left=clientRect.left;
	outputRect.right=clientRect.right;
	outputRect.top=y;
	outputRect.bottom=y+heightMidFont;
	sprintf(buffer, "PID: $%03x", mpegWindowInfo->displayedPacket.pid);
	ExtTextOut(hdc, ZORVEWINDOWMARGIN,y,ETO_OPAQUE, &outputRect, buffer, strlen(buffer), NULL);
	y+=heightMidFont;

	//Cont
	outputRect.top=y;
	outputRect.bottom=y+heightMidFont;
	sprintf(buffer, "Cont: %u", mpegWindowInfo->displayedPacket.continuitycounter);
	ExtTextOut(hdc, ZORVEWINDOWMARGIN,y,ETO_OPAQUE, &outputRect, buffer, strlen(buffer), NULL);
	y+=heightMidFont;


	//Fill to the bottom of this window
	outputRect.top=y;
	outputRect.bottom=clientRect.bottom;
	ExtTextOut(hdc, 0,0,ETO_OPAQUE, &outputRect, NULL, 0, NULL);

	DeleteObject(hSmallFont);
	DeleteObject(hMidFont);
	DeleteObject(hLargeFont);
	EndPaint(hwnd, &ps);

	return 0;
}

BOOL MpegChangePacket(MPEGWINDOW_INFO *mpegWindowInfo, LPARAM lParam)
{
	int	readPacket=0;
	LARGE_INTEGER liOffset;	//we use this to split up longlong for setfilepointer
	LONGLONG tempOffset;
	char positionString[24];


	if (lParam == (LPARAM)mpegWindowInfo->hwndPrevButton)	{
		if (mpegWindowInfo->fileInfo.displayOffset>=188*2)	{
			mpegWindowInfo->fileInfo.displayOffset-=188*2;
			readPacket=1;
		}
		else
			readPacket=0;
	}
	if (lParam == (LPARAM)mpegWindowInfo->hwndNextButton)	{

		readPacket=1;
	}

	if (lParam == (LPARAM)mpegWindowInfo->hwndSeekButton)	{

		SendMessage(mpegWindowInfo->hwndPositionEditbox, WM_GETTEXT, 20, (LPARAM)&positionString);

		//Give the opportunity for 0x to be entered as first two digits and regard as hexadecimal
		if ((positionString[0] == '0') && ((positionString[1] == 'x') || (positionString[1] == 'X')))
			mpegWindowInfo->fileInfo.displayOffset = strtoll(positionString+2, NULL ,16);
		else	//otherwise default to decimal
			mpegWindowInfo->fileInfo.displayOffset = strtoll(positionString, NULL ,10);

		MpegTSFindSyncByte(mpegWindowInfo->fileInfo.hMpegFile, &mpegWindowInfo->fileInfo.displayOffset);

		UnsignedLongLongToString(mpegWindowInfo->fileInfo.displayOffset, positionString);

		readPacket = 1;
	}


	if (readPacket)	{
		if (mpegWindowInfo->fileInfo.loadingInBackground)	{
			WaitForSingleObject(mpegWindowInfo->fileInfo.hFileAccessMutex,INFINITE);

			tempOffset=mpegWindowInfo->fileInfo.offset;	//remember the offset

			mpegWindowInfo->fileInfo.offset=mpegWindowInfo->fileInfo.displayOffset; //move to the display offset
			liOffset.QuadPart = mpegWindowInfo->fileInfo.displayOffset;	//move this to something to separate parts for SFP
			SetFilePointer(mpegWindowInfo->fileInfo.hMpegFile, liOffset.LowPart, &liOffset.HighPart, FILE_BEGIN);
			MpegReadPacket(&mpegWindowInfo->fileInfo, &mpegWindowInfo->displayedPacket);//readit
			mpegWindowInfo->fileInfo.displayOffset=mpegWindowInfo->fileInfo.offset;
			mpegWindowInfo->fileInfo.offset=tempOffset;	//set the offset back to what it was

			liOffset.QuadPart = tempOffset;
			SetFilePointer(mpegWindowInfo->fileInfo.hMpegFile, liOffset.LowPart, &liOffset.HighPart, FILE_BEGIN);

			ReleaseMutex(mpegWindowInfo->fileInfo.hFileAccessMutex);
		}
		else	{
			mpegWindowInfo->fileInfo.offset=mpegWindowInfo->fileInfo.displayOffset;
			liOffset.QuadPart = mpegWindowInfo->fileInfo.displayOffset;	//move this to largeint to separate parts for SFP

			SetFilePointer(mpegWindowInfo->fileInfo.hMpegFile, liOffset.LowPart, &liOffset.HighPart, FILE_BEGIN);
			MpegReadPacket(&mpegWindowInfo->fileInfo, &mpegWindowInfo->displayedPacket);
			mpegWindowInfo->fileInfo.displayOffset=mpegWindowInfo->fileInfo.offset;
		}

			return TRUE;	//redraw
	}

	return FALSE;
}
