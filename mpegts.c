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

			mpegWindowInfo->hwndNextButton =	CreateWindow("BUTTON", "Next record",
				    					WS_CHILD|WS_VISIBLE|WS_TABSTOP|WS_BORDER|BS_PUSHBUTTON,
									    20,250,130,30, hwnd, NULL, hInst, NULL);


			break;
		case WM_PAINT:
			MpegWindowPaint(hwnd);
			break;
		case WM_COMMAND:
			mpegWindowInfo=(MPEGWINDOW_INFO *)GetWindowLong(hwnd, GWL_USERDATA);	//get the point to window info
			if (lparam==(LPARAM)mpegWindowInfo->hwndNextButton)	{
				MpegReadPacket(&mpegWindowInfo->fileInfo, &mpegWindowInfo->displayedPacket);
				InvalidateRect(hwnd, NULL, FALSE);
			}
			break;
		case WM_ERASEBKGND:
			return 1;
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

	mpegWindowInfo=(MPEGWINDOW_INFO *)GetWindowLong(hwnd, GWL_USERDATA);	//get the point to window info

	GetClientRect(hwnd, &clientRect);
	hdc=BeginPaint(hwnd, &ps);

	SetBkColor(hdc, RGB_ZINNY_DARKBLUE);
	SetTextColor(hdc, RGB_ZINNY_WHITE);

	outputRect.left=clientRect.left;
	outputRect.right=clientRect.right;
	outputRect.top=clientRect.top;
	outputRect.bottom=clientRect.bottom;

	sprintf(buffer, "File size: %u. Offset:0x%x, pid:$%x, cont:%u", mpegWindowInfo->fileInfo.filesize, mpegWindowInfo->fileInfo.offset-188, mpegWindowInfo->displayedPacket.pid, mpegWindowInfo->displayedPacket.continuitycounter);
	ExtTextOut(hdc, 0,0,ETO_OPAQUE, &outputRect, buffer, strlen(buffer), NULL);


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
	mpegWindowInfo->fileInfo.hBackgroundThread=CreateThread(NULL, (SIZE_T)0, (LPTHREAD_START_ROUTINE)MpegReadFileStats, &mpegWindowInfo->fileInfo, 0, NULL);
	//MpegReadFileStats(&mpegWindowInfo->fileInfo);

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
		MpegReadPacket(mpegFileInfo, &packet);
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
	long offset;
	int justskipped;	//if we have just found one and skipped some bytes, we'll go back

	LARGE_INTEGER largeint_filesize;
	long filesize;

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

