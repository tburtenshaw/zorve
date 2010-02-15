#include <windows.h>
#include <windowsx.h>

int InfoWindowRegisterWndClass(HINSTANCE hInst);
HWND InfoWindowCreateOrShow(HWND hwnd, HWND hwndChild, HINSTANCE hInst);
HWND InfoWindowCreate(HWND hwndMDIClient, HINSTANCE hInst);
LRESULT CALLBACK ChildWndInfoProc(HWND hwnd,UINT msg,WPARAM wParam,LPARAM lParam);
