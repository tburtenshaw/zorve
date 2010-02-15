#include <windows.h>
#include <windowsx.h>
#include <commctrl.h>
#include <string.h>
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

HWND hwndList;
HWND hwndInfo;
HWND hwndMpeg;
HWND hwndNav;

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
	ListWindowRegisterWndClass(hInstProgram);
	MpegWindowRegisterWndClass(hInstProgram);
	NavWindowRegisterWndClass(hInstProgram);

	return 1;
}

HWND CreatezorveWndClassWnd(void)
{
	return CreateWindow("zorveWndClass","zorve",
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
{	char buffer[MAX_PATH];

	switch(id) {
		case IDM_ABOUT:
			DialogBox(hInstProgram,MAKEINTRESOURCE(IDD_ABOUT),
				hwndMain,AboutDlg);
			break;

		case IDM_OPEN:
			GetFileName(buffer,sizeof(buffer));
			hwndInfo = InfoWindowCreateOrShow(hwndInfo, hwndMDIClient, hInstProgram);
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
			return DefFrameProc(hwnd,hwndMDIClient,msg,wParam,lParam);
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
	tbb[0].fsState = TBSTATE_ENABLED;
	tbb[0].fsStyle = TBSTYLE_BUTTON;

	// Button "Properties"
	tbb[1].iBitmap = STD_PROPERTIES;
	tbb[1].idCommand = IDM_PROPERTIES;
	tbb[1].fsState = TBSTATE_ENABLED;
	tbb[1].fsStyle = TBSTYLE_BUTTON;

	// Button "Undo"
	tbb[2].iBitmap = STD_UNDO;
	tbb[2].idCommand = IDM_EDITUNDO;
	tbb[2].fsState = TBSTATE_ENABLED;
	tbb[2].fsStyle = TBSTYLE_BUTTON;

	// Button "Redo"
	tbb[3].iBitmap = STD_REDOW;
	tbb[3].idCommand = IDM_EDITREDO;
	tbb[3].fsState = TBSTATE_ENABLED;
	tbb[3].fsStyle = TBSTYLE_BUTTON;

	// Button "Open"
	tbb[4].iBitmap = STD_FILEOPEN;
	tbb[4].idCommand = IDM_OPEN;
	tbb[4].fsState = TBSTATE_ENABLED;
	tbb[4].fsStyle = TBSTYLE_BUTTON;


	// Button "Delete"
	tbb[5].iBitmap = STD_DELETE;
	tbb[5].idCommand = IDM_EDITDELETE;
	tbb[5].fsState = TBSTATE_ENABLED;
	tbb[5].fsStyle = TBSTYLE_BUTTON;

	// Button "Cut"
	tbb[6].iBitmap = STD_CUT;
	tbb[6].idCommand = IDM_EDITCUT;
	tbb[6].fsState = TBSTATE_ENABLED;
	tbb[6].fsStyle = TBSTYLE_BUTTON;

	// Button "Copy"
	tbb[7].iBitmap = STD_COPY;
	tbb[7].idCommand = IDM_EDITCOPY;
	tbb[7].fsState = TBSTATE_ENABLED;
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

	hwndList = ListWindowCreate(hwndMDIClient, hInstProgram);


	while (GetMessage(&msg,NULL,0,0)) {
		if (!TranslateMDISysAccel(hwndMDIClient, &msg))
			if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg)) {
				TranslateMessage(&msg);  // Translates virtual key codes
				DispatchMessage(&msg);   // Dispatches message to window
			}
	}
	return msg.wParam;
}
