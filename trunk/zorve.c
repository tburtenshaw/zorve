#include <windows.h>
#include <windowsx.h>
#include <commctrl.h>
#include <stdio.h>
#include "zorve.h"
#include "zorveres.h"
#include "list.h"	//This contains the code that handles the file list
#include "info.h"	//Handles the 4096 byte file
#include "mpegts.h"	//Handles the transport stream window
#include "navfile.h"//Handles the .nav file associated with mpg

/*<---------------------------------------------------------------------->*/
HINSTANCE hInstProgram;		// Instance handle
HWND hwndMain;			//Main window handle
HWND hwndMDIClient;		//Mdi client window handle
HWND hwndToolBar;
HWND  hWndStatusbar;

HWND hwndList = NULL;
HWND hwndInfo = NULL;
HWND hwndMpeg = NULL;
HWND hwndNav  = NULL;

LRESULT CALLBACK MainWndProc(HWND hwnd,UINT msg,WPARAM wParam,LPARAM lParam);

void UpdateStatusBar(LPSTR lpszStatusString, WORD partNumber, WORD displayFlags)
{
    SendMessage(hWndStatusbar,
                SB_SETTEXT,
                partNumber | displayFlags,
                (LPARAM)lpszStatusString);
}


LRESULT MsgMenuSelect(HWND hwnd, UINT uMessage, WPARAM wparam, LPARAM lparam)
{
    static char szBuffer[256];
    UINT   nStringID = 0;
    UINT   fuFlags = GET_WM_MENUSELECT_FLAGS(wparam, lparam) & 0xffff;
    UINT   uCmd    = GET_WM_MENUSELECT_CMD(wparam, lparam);
    HMENU  hMenu   = GET_WM_MENUSELECT_HMENU(wparam, lparam);

    szBuffer[0] = 0;                            // First reset the buffer
    if (fuFlags == 0xffff && hMenu == NULL)     // Menu has been closed
        nStringID = 0;

    else if (fuFlags & MFT_SEPARATOR)           // Ignore separators
        nStringID = 0;

    else if (fuFlags & MF_POPUP)                // Popup menu
    {
        if (fuFlags & MF_SYSMENU)               // System menu
            nStringID = IDS_SYSMENU;
        else
            // Get string ID for popup menu from idPopup array.
            nStringID = 0;
    }  // for MF_POPUP
    else                                        // Must be a command item
        nStringID = uCmd;                       // String ID == Command ID

    // Load the string if we have an ID
    if (0 != nStringID)
        LoadString(hInstProgram, nStringID, szBuffer, sizeof(szBuffer));
    // Finally... send the string to the status bar
    UpdateStatusBar(szBuffer, 0, 0);
    return 0;
}


void InitializeStatusBar(HWND hwndParent,int nrOfParts)
{
    const int cSpaceInBetween = 8;
    int   ptArray[40];   // Array defining the number of parts/sections
    RECT  rect;
    HDC   hDC;

    hDC = GetDC(hwndParent);
    GetClientRect(hwndParent, &rect);

    ptArray[nrOfParts-1] = rect.right;

    ReleaseDC(hwndParent, hDC);
    SendMessage(hWndStatusbar,
                SB_SETPARTS,
                nrOfParts,
                (LPARAM)(LPINT)ptArray);

    UpdateStatusBar("Ready", 0, 0);
}

static BOOL CreateSBar(HWND hwndParent,char *initialText,int nrOfParts)
{
    hWndStatusbar = CreateStatusWindow(WS_CHILD | WS_VISIBLE | WS_BORDER|SBARS_SIZEGRIP,
                                       initialText,
                                       hwndParent,
                                       IDM_STATUSBAR);
    if(hWndStatusbar)
    {
        InitializeStatusBar(hwndParent,nrOfParts);
        return TRUE;
    }

    return FALSE;
}

int GetFileName(char *buffer,int buflen)
{

	OPENFILENAME ofn;

	memset(&ofn,0,sizeof(ofn));
	ofn.lStructSize = sizeof(ofn);
	ofn.hInstance = GetModuleHandle(NULL);
	ofn.hwndOwner = GetActiveWindow();
	ofn.lpstrFile = buffer;
	ofn.nMaxFile = buflen;
	ofn.lpstrTitle = "Open";
	ofn.nFilterIndex = 2;
	ofn.lpstrDefExt = "";
	strcpy(buffer,"????Z*");

	ofn.Flags = 539650;
	ofn.lpstrFilter = "All files\0*.*\0Information file (????Z*)\0????Z*\0Navigation file (*.nav)\0*.nav\0MPEG file (*.mpg)\0*.mpg\0All recorded file types\0????Z*;*.nav;*.mpg\0";
	return GetOpenFileName(&ofn);

}

static BOOL InitApplication(void)
{
	WNDCLASS wc;

	memset(&wc,0,sizeof(WNDCLASS));
	wc.style = CS_HREDRAW|CS_VREDRAW |CS_DBLCLKS ;
	wc.lpfnWndProc = (WNDPROC)MainWndProc;
	wc.hInstance = hInstProgram;
	wc.hbrBackground = (HBRUSH)(COLOR_WINDOW+1);
	wc.lpszClassName = "zorveWndClass";
	wc.lpszMenuName = MAKEINTRESOURCE(IDMAINMENU);
	wc.hCursor = LoadCursor(NULL,IDC_ARROW);
	wc.hIcon = LoadIcon(hInstProgram, MAKEINTRESOURCE(IDI_FIRST));
	if (!RegisterClass(&wc))
		return 0;

	InfoWindowRegisterWndClass(hInstProgram);
	ListWindowRegisterWndClasses(hInstProgram);
	MpegWindowRegisterWndClass(hInstProgram);
	NavWindowRegisterWndClass(hInstProgram);

	return 1;
}

HWND CreatezorveWndClassWnd(void)
{
	return CreateWindow("zorveWndClass","Zorve",
		WS_MINIMIZEBOX|WS_VISIBLE|WS_CLIPSIBLINGS|WS_CLIPCHILDREN|WS_MAXIMIZEBOX|WS_CAPTION|WS_BORDER|WS_SYSMENU|WS_THICKFRAME,
		CW_USEDEFAULT,0,CW_USEDEFAULT,0,
		NULL,
		NULL,
		hInstProgram,
		NULL);
}

BOOL _stdcall AboutDlg(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch(msg) {
		case WM_CLOSE:
			EndDialog(hwnd,0);
			return 1;
		case WM_COMMAND:
			switch (LOWORD(wParam)) {
				case IDOK:
					EndDialog(hwnd,1);
					return 1;
			}
			break;
	}
	return 0;
}


void MainWndProc_OnCommand(HWND hwnd, int id, HWND hwndCtl, UINT codeNotify)
{
	LISTWINDOW_INFO *lpListWindowInfo;
	DIRECTORY_INFO *lpDirectoryInfo;

	char openfilename[MAX_PATH];
	char *p;

	int result;


	switch(id) {
		case IDM_ABOUT:
			DialogBox(hInstProgram,MAKEINTRESOURCE(IDD_ABOUT),
				hwndMain,AboutDlg);
			break;

		case IDM_OPEN:
			result = GetFileName(openfilename,sizeof(openfilename));
			if (!result)
				return;

			result = IndentifyFileType(openfilename);
			if (result==ZFT_INFO)	{
				//This loads the infofile after popping up (or making) the infowindow
				hwndInfo = InfoWindowCreateOrShow(hwndInfo, hwndMDIClient, hInstProgram);
				InfoWindowLoadFile(hwndInfo, &openfilename[0]);

				//Change folder
				if (!(IsWindow(hwndList)))	{
					hwndList = ListWindowCreateOrShow(hwndList, hwndMDIClient, hInstProgram);
				}

				lpListWindowInfo=(LISTWINDOW_INFO *)GetWindowLong(hwndList, GWL_USERDATA);
				lpDirectoryInfo=&lpListWindowInfo->directoryInfo;

				p=strrchr(openfilename,92);
				if (p)	{		//this temporarily hides the slash, truncating the file to a path- need to look for both backslash and slash though
					*p=0;
					SetListDirectory(lpDirectoryInfo, openfilename);
					*p=92;
				}
				//Now we need to select this loaded item
				RefreshAndOrSelectEntry(lpListWindowInfo, ListWindowGetEntryFromFilename(lpListWindowInfo, openfilename), FALSE, TRUE);

				InvalidateRect(lpListWindowInfo->hwndFolder, NULL, FALSE);
				InvalidateRect(lpListWindowInfo->hwndFiles, NULL, FALSE);
				return;
			}
			if (result==ZFT_MPEGTS)	{
				hwndMpeg= MpegWindowCreateOrShow(hwndMpeg, hwndMDIClient, hInstProgram);
				MpegWindowLoadFile(hwndMpeg, openfilename);
				return;
			}
			if (result==ZFT_NAV)	{
				hwndNav= NavWindowCreateOrShow(hwndNav, hwndMDIClient, hInstProgram);
				NavWindowLoadFile(hwndNav, openfilename);
				return;
			}

			MessageBox(hwnd, "Zorve was unable to detect the type of file.", "Open file", MB_ICONEXCLAMATION|MB_OK);

			break;
		case IDM_WINDOWTILE:
			SendMessage(hwndMDIClient,WM_MDITILE,0,0);
			break;
		case IDM_WINDOWCASCADE:
			SendMessage(hwndMDIClient,WM_MDICASCADE,0,0);
			break;
		case IDM_WINDOWICONS:
			SendMessage(hwndMDIClient,WM_MDIICONARRANGE,0,0);
			break;
		case IDM_EXIT:
			PostMessage(hwnd,WM_CLOSE,0,0);
			break;
	}
}


static HWND CreateMdiClient(HWND hwndparent)
{
    CLIENTCREATESTRUCT ccs = {0};
    HWND hwndMDIClient;
    int icount = GetMenuItemCount(GetMenu(hwndparent));

    // Find window menu where children will be listed
    ccs.hWindowMenu  = GetSubMenu(GetMenu(hwndparent), icount-2);
    ccs.idFirstChild = IDM_WINDOWCHILD;

    // Create the MDI client filling the client area
    hwndMDIClient = CreateWindow("mdiclient", NULL,
                                 WS_CHILD | WS_CLIPCHILDREN | WS_VSCROLL | WS_HSCROLL,
                                 0, 0, 0, 0,
                                 hwndparent, (HMENU)0xCAC, hInstProgram, (LPVOID)&ccs);

    ShowWindow(hwndMDIClient, SW_SHOW);

    return hwndMDIClient;
}

LRESULT CALLBACK MainWndProc(HWND hwnd,UINT msg,WPARAM wParam,LPARAM lParam)
{
	switch (msg) {

	case WM_CREATE:
		hwndMDIClient = CreateMdiClient(hwnd);
		break;
	case WM_SIZE:
		SendMessage(hWndStatusbar,msg,wParam,lParam);
		SendMessage(hwndToolBar,msg,wParam,lParam);
		InitializeStatusBar(hWndStatusbar,3);

		// Position the MDI client window between the tool and status bars
		if (wParam != SIZE_MINIMIZED) {
			RECT rc, rcClient;

			GetClientRect(hwnd, &rcClient);
			GetWindowRect(hWndStatusbar, &rc);
			ScreenToClient(hwnd, (LPPOINT)&rc.left);
			rcClient.bottom = rc.top;
			GetWindowRect(hwndToolBar,&rc);
			rcClient.top = rc.bottom - rc.top;
			MoveWindow(hwndMDIClient,rcClient.left,rcClient.top-1,rcClient.right-rcClient.left, rcClient.bottom-rcClient.top+1, TRUE);
		}

		case WM_MENUSELECT:
			return MsgMenuSelect(hwnd,msg,wParam,lParam);
		case WM_COMMAND:
			HANDLE_WM_COMMAND(hwnd,wParam,lParam,MainWndProc_OnCommand);
			break;
		case WM_DESTROY:
			PostQuitMessage(0);
			break;
		default:
			return DefFrameProc(hwnd, hwndMDIClient, msg, wParam, lParam);
	}
	return DefFrameProc(hwnd,hwndMDIClient,msg,wParam,lParam);
}



#define NUM_TOOLBAR_BUTTONS		8
HWND CreateAToolBar(HWND hwndParent)
{
	HWND hwndTB;
	TBADDBITMAP tbab;
	TBBUTTON tbb[NUM_TOOLBAR_BUTTONS];
	int index;

	// Ensure that the common control DLL is loaded.
	InitCommonControls();

	// Create a toolbar that the user can customize and that has a
	// tooltip associated with it.
	hwndTB = CreateWindowEx(0, TOOLBARCLASSNAME, (LPSTR) NULL,
	    WS_CHILD|WS_BORDER,
	    0, 0, 0, 0, hwndParent, (HMENU) ID_TOOLBAR, hInstProgram, NULL);

	// Send the TB_BUTTONSTRUCTSIZE message, which is required for
	// backward compatibility.
	SendMessage(hwndTB, TB_BUTTONSTRUCTSIZE,
	    (WPARAM) sizeof(TBBUTTON), 0);

	// Add the bitmap containing button images to the toolbar.
	tbab.hInst = HINST_COMMCTRL;
	tbab.nID   = IDB_STD_SMALL_COLOR;
	SendMessage(hwndTB, TB_ADDBITMAP, (WPARAM) NUM_TOOLBAR_BUTTONS,(LPARAM) &tbab);

	// clean memory before using it
	memset(tbb,0,sizeof tbb);

	// Button "Paste"
	tbb[0].iBitmap = STD_PASTE;
	tbb[0].idCommand = IDM_EDITPASTE;
	tbb[0].fsState = 0;//TBSTATE_ENABLED;
	tbb[0].fsStyle = TBSTYLE_BUTTON;

	// Button "Properties"
	tbb[1].iBitmap = STD_PROPERTIES;
	tbb[1].idCommand = IDM_PROPERTIES;
	tbb[1].fsState = 0;//TBSTATE_ENABLED;
	tbb[1].fsStyle = TBSTYLE_BUTTON;

	// Button "Undo"
	tbb[2].iBitmap = STD_UNDO;
	tbb[2].idCommand = IDM_EDITUNDO;
	tbb[2].fsState = 0;//TBSTATE_ENABLED;
	tbb[2].fsStyle = TBSTYLE_BUTTON;

	// Button "Redo"
	tbb[3].iBitmap = STD_REDOW;
	tbb[3].idCommand = IDM_EDITREDO;
	tbb[3].fsState = 0;//TBSTATE_ENABLED;
	tbb[3].fsStyle = TBSTYLE_BUTTON;

	// Button "Open"
	tbb[4].iBitmap = STD_FILEOPEN;
	tbb[4].idCommand = IDM_OPEN;
	tbb[4].fsState = TBSTATE_ENABLED;
	tbb[4].fsStyle = TBSTYLE_BUTTON;


	// Button "Delete"
	tbb[5].iBitmap = STD_DELETE;
	tbb[5].idCommand = IDM_EDITDELETE;
	tbb[5].fsState = 0;//TBSTATE_ENABLED;
	tbb[5].fsStyle = TBSTYLE_BUTTON;

	// Button "Cut"
	tbb[6].iBitmap = STD_CUT;
	tbb[6].idCommand = IDM_EDITCUT;
	tbb[6].fsState = 0;//TBSTATE_ENABLED;
	tbb[6].fsStyle = TBSTYLE_BUTTON;

	// Button "Copy"
	tbb[7].iBitmap = STD_COPY;
	tbb[7].idCommand = IDM_EDITCOPY;
	tbb[7].fsState = 0;//TBSTATE_ENABLED;
	tbb[7].fsStyle = TBSTYLE_BUTTON;

	SendMessage(hwndTB,TB_ADDBUTTONS,8,(LPARAM)&tbb);
	ShowWindow(hwndTB,SW_SHOW);
	return hwndTB;
}

/*<---------------------------------------------------------------------->*/
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, INT nCmdShow)
{
	MSG msg;
	HANDLE hAccelTable;

	hInstProgram = hInstance;
	if (!InitApplication())
		return 0;
	hAccelTable = LoadAccelerators(hInstProgram,MAKEINTRESOURCE(IDACCEL));
	if ((hwndMain = CreatezorveWndClassWnd()) == (HWND)0)
		return 0;
	CreateSBar(hwndMain,"Ready",3);
	hwndToolBar = CreateAToolBar(hwndMain);
	ShowWindow(hwndMain,SW_SHOW);

	hwndList = ListWindowCreateOrShow(hwndList, hwndMDIClient, hInstProgram);


	while (GetMessage(&msg,NULL,0,0)) {
		if (!TranslateMDISysAccel(hwndMDIClient, &msg))
			if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg)) {
				TranslateMessage(&msg);  // Translates virtual key codes
				DispatchMessage(&msg);   // Dispatches message to window
			}
	}
	return msg.wParam;
}


void UnixTimeToFileTime(long unixtime, LPFILETIME pft)
{
	// Note that LONGLONG is a 64-bit value
	LONGLONG ll;

	ll = Int32x32To64(unixtime, 10000000) + 116444736000000000;
	pft->dwLowDateTime = (DWORD)ll;
	pft->dwHighDateTime = ll >> 32;

	return;
}

void ModJulianTimeToFileTime(int juliantime, LPFILETIME pft)
{
	#define FILETIMEJULIANDIFF 94187	//the difference of 1 Jan 1601 and 17 Nov 1858

	LONGLONG ll;
	ll=(LONGLONG)((LONG)(juliantime +FILETIMEJULIANDIFF)) * (LONGLONG)864000000000;	//multiply by number of 100 ns in one day

	pft->dwLowDateTime = (DWORD)ll;
	pft->dwHighDateTime = ll >> 32;

	return;
}

void BytesDisplayNice(long bytes, char *formatString, int thresholdratio, char *outputString)
{
	float sizetempfloat;
	char sizeunits[6];

	sizetempfloat=(float)bytes;

	if (bytes>(1073741824*thresholdratio))	{
		sizetempfloat=sizetempfloat/1073741824;
		sizeunits[0]='G'; 			sizeunits[1]='B';
		sizeunits[2]=0;
	}
	else if (bytes>1048576*thresholdratio)	{
		sizetempfloat=sizetempfloat/1048576;
		sizeunits[0]='M'; 			sizeunits[1]='B';
		sizeunits[2]=0;
	}
	else if (bytes>1024*thresholdratio)	{
		sizetempfloat=sizetempfloat/1024;
		sizeunits[0]='K'; 			sizeunits[1]='B';
		sizeunits[2]=0;
	}
	else	{
		sizeunits[0]='b'; 			sizeunits[1]='y';
		sizeunits[2]='t'; 			sizeunits[3]='e';
		sizeunits[4]='s'; 			sizeunits[5]=0;
	}

	sprintf(outputString, formatString, sizetempfloat, sizeunits);

	return;
}

void DurationShortFormatDHMS(long duration, char *outputString)
{
	int seconds=0;
	int minutes=0;
	int hours=0;
	int days=0;

	seconds=duration%60;

	if (duration>=60)
		minutes=(duration/60)%60;
	if (duration>=3600)
		hours=(duration/3600)%24;
	if (duration>=86400)
		days=(duration/86400);

	outputString[0]=0;
	if (duration>=86400)
		sprintf(outputString, "%id ", days);
	if (duration>=3600)
    	sprintf(outputString, "%s%ih ", outputString, hours);
	if (duration>=60)
		sprintf(outputString, "%s%im ", outputString, minutes);
	sprintf(outputString, "%s%is", outputString, seconds);

	return;
}

HWND ZorveGetHwndInfo()
{

	return hwndInfo;
}

void ZorveSetHwndInfo(HWND hwnd)
{
	hwndInfo=hwnd;

	return;
}

HWND ZorveGetHwndList()
{

	return hwndList;
}

void ZorveSetHwndList(HWND hwnd)
{
	hwndList=hwnd;

	return;
}

WORD swap_endian_word(WORD i)
{
    return ((i>>8)&0xff)+((i << 8)&0xff00);
}


char * ReturnChannelNameFromPID(int pid)
{

	switch (pid)	{
		case 0x00FA:
	   		return "TV1";
		case 0x00FB:
			return "TV2";
		case 0x00FC:
		 	return "TVNZ6";
		case 0x00FD:
		 	return "TVNZ7";
		case 0x01C2:
		 	return "TV3";
		case 0x01C3:
		 	return "C4";
		case 0x01C4:
		 	return "TV3+1";
		case 0x0226:
		 	return "Maori TV";
		case 0x0227:
		 	return "Parliament TV";
		case 0x0229:
		 	return "Chinese TV";
		case 0x022A:
		 	return "Prime";
		case 0x022C:
		 	return "Freeview HD";
	}


	return NULL;

}


int IndentifyFileType(char *filename)
{

	HANDLE hFile;
	LARGE_INTEGER filesize;
	int result;
	long n;

	long magic=0;
	unsigned int navEight_int=0;
	long navZero_long=0;
	long mpegSyncByte=0;

	hFile=CreateFile(filename, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL|FILE_FLAG_RANDOM_ACCESS, NULL);

	//If we could load the file, then we can't tell what it is
	if (!hFile) return 0;

	GetFileSizeEx(hFile, &filesize);

	//if it's 4096 bytes, then check if it's an info file
	if ((filesize.LowPart==4096) && (filesize.HighPart==0))	{
		ReadFile(hFile, &magic, 4, &n, NULL);
		if (magic==0x50800)	{
			CloseHandle(hFile);
			return ZFT_INFO;
		}
	}


	//Return if file too small to be nav (or mpeg)
	if ((filesize.HighPart==0) && (filesize.LowPart<64))	{	//we're too small to be even a single line nav
		CloseHandle(hFile);
		return ZFT_UNKNOWN;
	}

	//Try to detect if it's a navigation file
	if (!(filesize.LowPart % 0x20))	{//if we accept it, it needs to be divisible by 64
		SetFilePointer(hFile, 0x06, NULL, FILE_BEGIN);
		result = ReadFile(hFile, &navEight_int, 2, &n, NULL);
		result = ReadFile(hFile, &navZero_long, 4, &n, NULL);
		if ((navEight_int==8)&&(navZero_long==0))	{
			CloseHandle(hFile);
			return ZFT_NAV;
		}
	}

	//Return if too small to be an MPEG (we detect by reading 5 packets of 188).
	if ((filesize.HighPart==0) && (filesize.LowPart<940))	{	//we're too small to be even a single line nav
		CloseHandle(hFile);
		return ZFT_UNKNOWN;
	}

	if (MpegTSFindSyncByte(hFile, &mpegSyncByte))	{
		CloseHandle(hFile);
		return ZFT_MPEGTS;
	}



	CloseHandle(hFile);

	return ZFT_UNKNOWN;	//if returns 0, then we don't know
}
