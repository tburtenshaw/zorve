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

			mpegWindowInfo->fileInfo.windowInfo=mpegWindowInfo;

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
		case ZM_MPEG_SKIPTOOFFSET: 		//I'll handle this sloppily for now
			char buffer[24];
			LARGE_INTEGER	liOffset;
			mpegWindowInfo=(MPEGWINDOW_INFO *)GetWindowLong(hwnd, GWL_USERDATA);	//get the point to window info
			liOffset.LowPart=wparam;
			liOffset.HighPart=lparam;
			UnsignedLongLongToString(liOffset.QuadPart, buffer);
			SendMessage(mpegWindowInfo->hwndPositionEditbox, WM_SETTEXT, 0, (LPARAM)buffer);	//fill the editbox with offset
			SendMessage(mpegWindowInfo->hwndBlockInfo, WM_COMMAND, 0, (LPARAM)mpegWindowInfo->hwndSeekButton); //simulate seek press
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
		SendMessage(mpegWindowInfo->hwndPIDCombobox,CB_RESETCONTENT,0, 0);	//reset the combobox listing pids
	}

	mpegWindowInfo->fileInfo.windowInfo=mpegWindowInfo;
	lstrcpy(mpegWindowInfo->fileInfo.filename, mpegFile);

	SendMessage(mpegWindowInfo->zorveHwnd, ZM_REQUEST_RECORDINGNAME, (WPARAM)mpegWindowInfo->fileInfo.filename, (LPARAM)hwnd);
	InvalidateRect(mpegWindowInfo->hwndFileInfo, NULL, FALSE);

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
	LONGLONG offset;
	int pTry;
	int stay;
	char buffer[16];

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
				buffer[0]='0'; buffer[1]='x';
				ComboBox_AddString(mpegFileInfo->windowInfo->hwndPIDCombobox, ltoa(packet.pid, buffer+2, 16)-2);
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

int MpegTSFindSyncByte(HANDLE hFile, LONGLONG * syncbyteOffset)
{
	LARGE_INTEGER largeint_filesize;
	BYTE testbyte;
	long n;

	BOOL readResult;
	int occurence;
	LARGE_INTEGER offset;
	int justskipped;	//if we have just found one and skipped some bytes, we'll go back
	long highZero=0;	//used as a high order rather than null

	GetFileSizeEx(hFile, &largeint_filesize);

	offset.QuadPart=*syncbyteOffset;	//the syncbyte offset will be our starting position
	occurence = 0;
	justskipped=0;

	readResult = SetFilePointer(hFile, offset.LowPart, &offset.HighPart, FILE_BEGIN);
	if (readResult==INVALID_SET_FILE_POINTER)
		return 0;


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
				offset.QuadPart-=187;
				readResult = SetFilePointer(hFile, offset.LowPart, &offset.HighPart, FILE_BEGIN);
				offset.QuadPart--;	//need to do this, because we end up adding later.
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

	memset(packet, 0, sizeof(TS_PACKET));
	ReadFile(mpegFileInfo->hMpegFile, packet->TS_raw_packet, 188, &n, NULL);
	mpegFileInfo->offset+=188;

	packet->sync_byte = packet->TS_raw_packet[0];
	packet->transport_error_indicator =	(packet->TS_raw_packet[1] & 0x80) >> 7;
	packet->payload_unit_start_indicator =		(packet->TS_raw_packet[1] & 0x40) >> 6;
	packet->transport_priority =	(packet->TS_raw_packet[1] & 0x20) >> 5;
	packet->pid	= ((packet->TS_raw_packet[1] & 0b11111) << 8) | packet->TS_raw_packet[2];
	packet->transport_scrambling_control	=		(packet->TS_raw_packet[3] & 0b11000000) >> 6;
	packet->adaptation_field_control	= 		(packet->TS_raw_packet[3] & 0b00110000) >> 4;
	packet->continuity_counter= 	(packet->TS_raw_packet[3] & 0b00001111);

	if (packet->adaptation_field_control & 0b10)	{
		packet->adaptation_field.adaptationfieldlength = packet->TS_raw_packet[4];
		packet->adaptation_field.discontinuity =		(packet->TS_raw_packet[5] & 0b10000000) >> 7;
		packet->adaptation_field.randomaccess =		(packet->TS_raw_packet[5] & 0b01000000) >> 6;
		packet->adaptation_field.EsPriority =		(packet->TS_raw_packet[5] & 0b00100000) >> 5;
		packet->adaptation_field.PCRFlag =			(packet->TS_raw_packet[5] & 0b00010000) >> 4;
		packet->adaptation_field.OPCRFlag =			(packet->TS_raw_packet[5] & 0b00001000) >> 3;
		packet->adaptation_field.splicingpointflag =	(packet->TS_raw_packet[5] & 0b00000100) >> 2;
		packet->adaptation_field.transportprivatedataflag =		(packet->TS_raw_packet[5] & 0b00000010) >> 1;
		packet->adaptation_field.adaptationfieldextensionflag =	(packet->TS_raw_packet[5] & 0b00000001);

		b=6;
		if (packet->adaptation_field.PCRFlag)	{	//nothing is more annoying than a 33bit value
			packet->adaptation_field.PcrBase	=  packet->TS_raw_packet[6]*0x2000000;
			packet->adaptation_field.PcrBase	|= packet->TS_raw_packet[7]*0x20000;
			packet->adaptation_field.PcrBase	|= packet->TS_raw_packet[8]*0x200;
			packet->adaptation_field.PcrBase	|= packet->TS_raw_packet[9]*0x2;
			packet->adaptation_field.PcrBase	|= packet->TS_raw_packet[10]>>7; //keeps just the most sig bit

			packet->adaptation_field.PcrExt	=(packet->TS_raw_packet[10] & 1)<<8;
			packet->adaptation_field.PcrExt	|=	packet->TS_raw_packet[11];

			b=12;
		}
		if (packet->adaptation_field.OPCRFlag)	{	//nothing is more annoying than a 33bit value
			packet->adaptation_field.OpcrBase	=  packet->TS_raw_packet[b+0]*0x2000000;
			packet->adaptation_field.OpcrBase	|= packet->TS_raw_packet[b+1]*0x20000;
			packet->adaptation_field.OpcrBase	|= packet->TS_raw_packet[b+2]*0x200;
			packet->adaptation_field.OpcrBase	|= packet->TS_raw_packet[b+3]*0x2;
			packet->adaptation_field.OpcrBase	|= packet->TS_raw_packet[b+4]>>7; //keeps just the most sig bit

			packet->adaptation_field.OpcrExt	=(packet->TS_raw_packet[b+4] & 1)<<8;
			packet->adaptation_field.OpcrExt	|=	packet->TS_raw_packet[b+5];

			b+=6;
		}
		if (packet->adaptation_field.splicingpointflag)	{
			packet->adaptation_field.splicecountdown = packet->TS_raw_packet[b];
			b++;
		}
		if (packet->adaptation_field.transportprivatedataflag)	{
			packet->adaptation_field.transportprivatedatalength = packet->TS_raw_packet[b];
			if (packet->adaptation_field.transportprivatedatalength>187-b)
				packet->adaptation_field.transportprivatedatalength=0;
			memcpy(&packet->adaptation_field.privatedata, &packet->TS_raw_packet[b+1], packet->adaptation_field.transportprivatedatalength);
			b+=packet->adaptation_field.transportprivatedatalength+1;
		}
		if (packet->adaptation_field.adaptationfieldextensionflag)	{
			packet->adaptation_field.adaptationfieldextensionlength = packet->TS_raw_packet[b];
			b++;
			offsetAtAFExt=b;
			packet->adaptation_field.ltwflag	= (packet->TS_raw_packet[b] & 0b10000000) >> 7;
			packet->adaptation_field.piecewiserateflag	= (packet->TS_raw_packet[b] & 0b01000000) >> 6;
			packet->adaptation_field.seamlessspliceflag	= (packet->TS_raw_packet[b] & 0b00100000) >> 5;
			b++;
			if (packet->adaptation_field.ltwflag)	{
				packet->adaptation_field.ltwflag	= (packet->TS_raw_packet[b] & 0b10000000) >> 7;
				packet->adaptation_field.ltwoffset = (LONG)(packet->TS_raw_packet[b]&0b01111111) << 8;
				b++;
				packet->adaptation_field.ltwoffset |=packet->TS_raw_packet[b];
				b++;
			}
			if (packet->adaptation_field.piecewiserateflag)	{
				packet->adaptation_field.piecewiserate  = (LONG)(packet->TS_raw_packet[b] & 0b00111111) <<16;
				packet->adaptation_field.piecewiserate |= (LONG)packet->TS_raw_packet[b+1] <<8;
				packet->adaptation_field.piecewiserate |= packet->TS_raw_packet[b+2];
				b+=3;

			}
			if (packet->adaptation_field.seamlessspliceflag)	{
				packet->adaptation_field.splicetype=	(packet->TS_raw_packet[b] & 0b11110000) >> 4;
				packet->adaptation_field.DTSnextAU=	(packet->TS_raw_packet[b] & 0b00001110);
				packet->adaptation_field.DTSnextAU <<=29;
				//packet->adaptation_field.DTSnextAU|=(ULONGLONG)(packet->TS_raw_packet[b+1] & 0b1111111111111110)<<14;
				//NEED TO FIX
				b+=5;
			}
			//We could skip this, but it'll be skipped when we use the AF length
			//b=offsetAtAFExt+packet->adaptationfield.adaptationfieldextensionlength;
		}
		//Skip the padding. The AF length is at byte 4, this ensures we end after this.
		b=4+packet->adaptation_field.adaptationfieldlength;

	}

	return b;
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

			mpegWindowInfo->hwndPositionEditbox =	CreateWindow("EDIT", "0",
				    					WS_CHILD|WS_VISIBLE|ES_LEFT|WS_TABSTOP|WS_BORDER,
									    30,70,80,18, hwnd, NULL, hInst, NULL);
			SendMessage(mpegWindowInfo->hwndPositionEditbox, EM_LIMITTEXT, 20, 0);	//shouldn't be any offset longer than 20 digits

			mpegWindowInfo->hwndSeekButton =	CreateWindow("BUTTON", "Seek",
					   					WS_CHILD|WS_VISIBLE|WS_TABSTOP|WS_BORDER|BS_PUSHBUTTON,
									    120,60,60,23, hwnd, NULL, hInst, NULL);

			mpegWindowInfo->hwndLockPID	= CreateWindow("BUTTON", "PID",
					   					WS_CHILD|WS_VISIBLE|WS_TABSTOP|BS_AUTOCHECKBOX,
									    20,120,60,23, hwnd, NULL, hInst, NULL);

			mpegWindowInfo->hwndPIDCombobox = CreateWindow("COMBOBOX", "PID",
					   					WS_CHILD|WS_VISIBLE|WS_TABSTOP|	CBS_DROPDOWNLIST,
									    90,120,60,123, hwnd, NULL, hInst, NULL);

			mpegWindowInfo->hwndLockPayload = CreateWindow("BUTTON", "Payload",
					   					WS_CHILD|WS_VISIBLE|WS_TABSTOP|BS_AUTOCHECKBOX,
									    160,120,60,23, hwnd, NULL, hInst, NULL);

			mpegWindowInfo->hwndLockNavPointer = CreateWindow("BUTTON", "Nav files",
					   					WS_CHILD|WS_VISIBLE|WS_TABSTOP|BS_CHECKBOX|WS_DISABLED,
									    160,150,60,23, hwnd, NULL, hInst, NULL);
//hwndLockIframe

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
			SendMessage(mpegWindowInfo->hwndLockPID, WM_SETFONT, (WPARAM)hSmallFont, 0);
			SendMessage(mpegWindowInfo->hwndPIDCombobox, WM_SETFONT, (WPARAM)hSmallFont, 0);
			SendMessage(mpegWindowInfo->hwndLockPayload, WM_SETFONT, (WPARAM)hSmallFont, 0);
			SendMessage(mpegWindowInfo->hwndLockNavPointer, WM_SETFONT, (WPARAM)hSmallFont, 0);

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


		if ((i>=mpegWindowInfo->hexview.selStart) && (i<=mpegWindowInfo->hexview.selEnd))	{
			SetBkColor(hdc, RGB_ZINNY_DARKBLUE);
			SetTextColor(hdc, RGB_ZINNY_WHITE);
		} else	{
			SetBkColor(hdc, RGB_ZINNY_WHITE);
			SetTextColor(hdc, RGB_ZINNY_BLACK);
		}



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
	sprintf(buffer+6+len, " bytes");
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
		sprintf(buffer, "PID 0x%03x: %u instances (%3.2f%%)", mpegWindowInfo->fileInfo.seenPid[i], mpegWindowInfo->fileInfo.countPid[i], (float)mpegWindowInfo->fileInfo.countPid[i]*100/(float)mpegWindowInfo->fileInfo.countPackets);
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

	int len;

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
	len = strlen(buffer);
	len+=UnsignedLongLongToString((mpegWindowInfo->fileInfo.displayOffset)-188, buffer+strlen(buffer));
	sprintf(buffer+len, ")");

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
	if (mpegWindowInfo->displayedPacket.sync_byte ==0x47)
		SetTextColor(hdc, RGB_ZINNY_COLOUR_SYNC);
	else
		SetTextColor(hdc, RGB_ZINNY_COLOUR_SYNC_FADE);

	ExtTextOut(hdc, x, y, ETO_OPAQUE, &outputRect, "SYNC", 4, NULL);
	x+=textSize.cx;

	GetTextExtentPoint32(hdc, "ERROR", 5, &textSize);
	outputRect.left=x;	outputRect.right=x+textSize.cx;

	if (mpegWindowInfo->displayedPacket.transport_error_indicator)
		SetTextColor(hdc, RGB_ZINNY_COLOUR_ERROR);
	else
		SetTextColor(hdc, RGB_ZINNY_COLOUR_ERROR_FADE);

	ExtTextOut(hdc, x, y, ETO_OPAQUE, &outputRect, "ERROR", 5, NULL);
	x+=textSize.cx;

	GetTextExtentPoint32(hdc, "PAYLOAD", 7, &textSize);
	outputRect.left=x;	outputRect.right=x+textSize.cx;
	if (mpegWindowInfo->displayedPacket.payload_unit_start_indicator)
		SetTextColor(hdc, RGB_ZINNY_COLOUR_PAYLOAD);
	else
		SetTextColor(hdc, RGB_ZINNY_COLOUR_PAYLOAD_FADE);

	ExtTextOut(hdc, x, y, ETO_OPAQUE, &outputRect, "PAYLOAD", 7, NULL);
	x+=textSize.cx;

	outputRect.left=x;	outputRect.right=clientRect.right;
	if (mpegWindowInfo->displayedPacket.transport_priority)
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
	sprintf(buffer, "PID: 0x%03x", mpegWindowInfo->displayedPacket.pid);
	ExtTextOut(hdc, ZORVEWINDOWMARGIN,y,ETO_OPAQUE, &outputRect, buffer, strlen(buffer), NULL);
	y+=heightMidFont;

	//Cont
	outputRect.top=y;
	outputRect.bottom=y+heightMidFont;
	sprintf(buffer, "Cont: %u", mpegWindowInfo->displayedPacket.continuity_counter);
	ExtTextOut(hdc, ZORVEWINDOWMARGIN,y,ETO_OPAQUE, &outputRect, buffer, strlen(buffer), NULL);
	y+=heightMidFont;


//now, a temporary place to dump packet data
	if (mpegWindowInfo->displayedPacket.adaptation_field_control & 0b10)	{
		outputRect.top=y;
		outputRect.bottom=y+heightMidFont;
		sprintf(buffer, "AFL: %u, PCRext: %u", mpegWindowInfo->displayedPacket.adaptation_field.adaptationfieldlength,
			mpegWindowInfo->displayedPacket.adaptation_field.PcrExt);
		ExtTextOut(hdc, ZORVEWINDOWMARGIN,y,ETO_OPAQUE, &outputRect, buffer, strlen(buffer), NULL);
		y+=heightMidFont;

	}




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

	LONGLONG maxOffset;
	LONGLONG minOffset;

	//Various vars regarding
	int pidToLock;
	BOOL payloadLock;
	BOOL pidLock;
	BOOL lockTest;


	SendMessage(mpegWindowInfo->hwndPIDCombobox, WM_GETTEXT, 20, (LPARAM)&positionString);

	if ((positionString[0] == '0') && ((positionString[1] == 'x') || (positionString[1] == 'X')))
		pidToLock = strtol(positionString+2,NULL, 16);
	else if (positionString[0] == '$')
		pidToLock = strtol(positionString+1,NULL, 16);
	else
		pidToLock = strtol(positionString,NULL, 10);

	if	(pidLock)	{
		//check that it exists
	}

	payloadLock = SendMessage(mpegWindowInfo->hwndLockPayload, BM_GETSTATE, 0, 0) == BST_CHECKED;
	pidLock = SendMessage(mpegWindowInfo->hwndLockPID, BM_GETSTATE, 0, 0) == BST_CHECKED;

	//If we're locking to something
	if (payloadLock || pidLock)	{
		//If searching forwards or backwards
		if ((lParam == (LPARAM)mpegWindowInfo->hwndNextButton)||(lParam == (LPARAM)mpegWindowInfo->hwndPrevButton))	{
			WaitForSingleObject(mpegWindowInfo->fileInfo.hFileAccessMutex,INFINITE);

			tempOffset=mpegWindowInfo->fileInfo.offset;	//remember the offset

			mpegWindowInfo->fileInfo.offset=mpegWindowInfo->fileInfo.displayOffset; //move to the display offset
			liOffset.QuadPart = mpegWindowInfo->fileInfo.displayOffset;	//move this to something to separate parts for SFP
			SetFilePointer(mpegWindowInfo->fileInfo.hMpegFile, liOffset.LowPart, &liOffset.HighPart, FILE_BEGIN);

			if (lParam == (LPARAM)mpegWindowInfo->hwndPrevButton)	{
				maxOffset=mpegWindowInfo->fileInfo.filesize;
				minOffset=188*2;
			} else {
				maxOffset=mpegWindowInfo->fileInfo.filesize-188;
				minOffset=0;
			}
			mpegWindowInfo->displayedPacket.payload_unit_start_indicator= FALSE;

			//The main search loop
			while ((lockTest)
			    && (mpegWindowInfo->fileInfo.offset <= maxOffset)
			    && (mpegWindowInfo->fileInfo.offset >= minOffset))	{
				if (lParam == (LPARAM)mpegWindowInfo->hwndPrevButton)	{
					mpegWindowInfo->fileInfo.offset-=188*2;	//move back over the block just read, to the begining of the previous
					liOffset.QuadPart = mpegWindowInfo->fileInfo.offset;
					SetFilePointer(mpegWindowInfo->fileInfo.hMpegFile, liOffset.LowPart, &liOffset.HighPart, FILE_BEGIN);

				}
				MpegReadPacket(&mpegWindowInfo->fileInfo, &mpegWindowInfo->displayedPacket);//read it

				//What are we testing for?
				if (payloadLock && pidLock)
					lockTest=(mpegWindowInfo->displayedPacket.payload_unit_start_indicator==FALSE) || (mpegWindowInfo->displayedPacket.pid != pidToLock);
				else if (payloadLock)
					lockTest = (mpegWindowInfo->displayedPacket.payload_unit_start_indicator==FALSE);
				else if (pidLock)
					lockTest = (mpegWindowInfo->displayedPacket.pid != pidToLock);
				else
					lockTest = TRUE;
			}



			mpegWindowInfo->fileInfo.displayOffset=mpegWindowInfo->fileInfo.offset;
			mpegWindowInfo->fileInfo.offset=tempOffset;	//set the offset back to what it was

			liOffset.QuadPart = tempOffset;



			ReleaseMutex(mpegWindowInfo->fileInfo.hFileAccessMutex);

			return TRUE;
		}
	}

	//Remember that displayOffset is primed to read the next
	if (lParam == (LPARAM)mpegWindowInfo->hwndPrevButton)	{
		if (mpegWindowInfo->fileInfo.displayOffset>=188*2)	{
			mpegWindowInfo->fileInfo.displayOffset-=188*2;
			readPacket=1;
		}
		else
			readPacket=0;
	}
	if (lParam == (LPARAM)mpegWindowInfo->hwndNextButton)	{
		if (mpegWindowInfo->fileInfo.displayOffset < mpegWindowInfo->fileInfo.filesize-188)
			readPacket=1;
	}

	//Seek to a particular offset
	if (lParam == (LPARAM)mpegWindowInfo->hwndSeekButton)	{

		SendMessage(mpegWindowInfo->hwndPositionEditbox, WM_GETTEXT, 20, (LPARAM)&positionString);

		//Give the opportunity for 0x to be entered as first two digits and regard as hexadecimal
		if ((positionString[0] == '0') && ((positionString[1] == 'x') || (positionString[1] == 'X')))
			mpegWindowInfo->fileInfo.displayOffset = strtoll(positionString+2, NULL ,16);
		else if (positionString[0] == '$')
			mpegWindowInfo->fileInfo.displayOffset = strtoll(positionString+1,NULL, 16);
		else if ((positionString[0] == 'P') || (positionString[0] == 'p'))	//let P as a prefix seek to packet number
			mpegWindowInfo->fileInfo.displayOffset = strtoll(positionString+1, NULL ,10)*188-188;
		else	//otherwise default to decimal
			mpegWindowInfo->fileInfo.displayOffset = strtoll(positionString, NULL ,10);

		mpegWindowInfo->hexview.selStart=mpegWindowInfo->fileInfo.displayOffset;	//correct to 188 later

		//Since we want to find the packet with the offset in it, we move back 187
		if (mpegWindowInfo->fileInfo.displayOffset>187)
			mpegWindowInfo->fileInfo.displayOffset-=187;
		else
			mpegWindowInfo->fileInfo.displayOffset=0;

		//Sanity checking
		if (mpegWindowInfo->fileInfo.displayOffset<0)	//too low
			mpegWindowInfo->fileInfo.displayOffset=0;

		if (mpegWindowInfo->fileInfo.displayOffset>mpegWindowInfo->fileInfo.filesize-188)	//too high
			mpegWindowInfo->fileInfo.displayOffset=mpegWindowInfo->fileInfo.filesize-188;
			//note: as sync needs to find 5 consecutive syncbytes, we should ensure the starting offset
		    //is not too close to the EOF. If it is, an amount should be subtracted, sync found, then re-add
			//the amount.

		//Wait until the file isn't being read in the background
		WaitForSingleObject(mpegWindowInfo->fileInfo.hFileAccessMutex,INFINITE);

		tempOffset=mpegWindowInfo->fileInfo.offset;	//remember the offset
		MpegTSFindSyncByte(mpegWindowInfo->fileInfo.hMpegFile, &mpegWindowInfo->fileInfo.displayOffset);

		mpegWindowInfo->hexview.selStart-=mpegWindowInfo->fileInfo.displayOffset;	//adjust to 188
		mpegWindowInfo->hexview.selEnd=mpegWindowInfo->hexview.selStart;

		MpegReadPacket(&mpegWindowInfo->fileInfo, &mpegWindowInfo->displayedPacket);
		mpegWindowInfo->fileInfo.displayOffset+=188;

		if (!mpegWindowInfo->fileInfo.loadingInBackground)
			mpegWindowInfo->fileInfo.offset=mpegWindowInfo->fileInfo.displayOffset;
		else	{
			liOffset.QuadPart = tempOffset;
			SetFilePointer(mpegWindowInfo->fileInfo.hMpegFile, liOffset.LowPart, &liOffset.HighPart, FILE_BEGIN);
		}


		ReleaseMutex(mpegWindowInfo->fileInfo.hFileAccessMutex);

		UnsignedLongLongToString(mpegWindowInfo->fileInfo.displayOffset, positionString);
		return TRUE;
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
