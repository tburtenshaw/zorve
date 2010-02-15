#include <windows.h>
#include <windowsx.h>

int NavWindowRegisterWndClass(HINSTANCE hInst);
LRESULT CALLBACK ChildWndNavProc(HWND hwnd,UINT msg,WPARAM wParam,LPARAM lParam);
