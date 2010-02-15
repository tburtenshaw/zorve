#include <windows.h>
#include <windowsx.h>

int ListWindowRegisterWndClass(HINSTANCE hInst);	//Registers the class "ListWndClass"
HWND ListWindowCreate(HWND hwndMDIClient, HINSTANCE hInst);
LRESULT CALLBACK ChildWndListProc(HWND hwnd,UINT msg,WPARAM wParam,LPARAM lParam);

