#include <windows.h>
#include <windowsx.h>

int MpegWindowRegisterWndClass(HINSTANCE hInst);
LRESULT CALLBACK ChildWndMpegProc(HWND hwnd,UINT msg,WPARAM wParam,LPARAM lParam);
