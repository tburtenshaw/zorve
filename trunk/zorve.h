#include <windows.h>
#include <windowsx.h>


#define Int32x32To64(a, b) ((LONGLONG)((LONG)(a)) * (LONGLONG)((LONG)(b)))

void UnixTimeToFileTime(long unixtime, LPFILETIME pft);
void BytesDisplayNice(long bytes, char *formatString, int thresholdratio, char *outputString);
void DurationShortFormatDHMS(long duration, char *outputString);

HWND ZorveGetHwndInfo(void);
void ZorveSetHwndInfo(HWND hwnd);


#define RGB_ZINNY_DARKBLUE RGB(0x00, 0x00, 0x66)
#define RGB_ZINNY_MIDPURPLE RGB(0xBE, 0xBE, 0xDC)
#define RGB_ZINNY_HIGHPURPLE RGB(0xCC, 0xCC, 0xED)

#define RGB_ZINNY_WHITE RGB(0xFF, 0xFF, 0xFF)
#define RGB_ZINNY_BRIGHTBLUE RGB(0x54, 0x9C, 0xF6)
//RGB(0x57, 0xA2, 0xFF)

