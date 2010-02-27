#include <windows.h>
#include <windowsx.h>

int NavWindowRegisterWndClass(HINSTANCE hInst);
LRESULT CALLBACK ChildWndNavProc(HWND hwnd,UINT msg,WPARAM wParam,LPARAM lParam);
HWND NavWindowCreateOrShow(HWND listHwnd, HWND hwndMDIClient, HINSTANCE hInst);
HWND NavWindowCreate(HWND hwndMDIClient, HINSTANCE hInst);
