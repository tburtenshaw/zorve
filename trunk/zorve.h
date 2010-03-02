#include <windows.h>
#include <windowsx.h>

#define ZFT_UNKNOWN 0
#define ZFT_INFO 1
#define ZFT_MPEGTS 2
#define ZFT_NAV 3

#define Int32x32To64(a, b) ((LONGLONG)((LONG)(a)) * (LONGLONG)((LONG)(b)))
#define MAX(a,b) (a > b ? a : b)
#define MIN(a,b) (a < b ? a : b)

int IndentifyFileType(char *filename);

void UnixTimeToFileTime(long unixtime, LPFILETIME pft);
void ModJulianTimeToFileTime(int juliantime, LPFILETIME pft);
void BytesDisplayNice(long bytes, char *formatString, int thresholdratio, char *outputString);
void DurationShortFormatDHMS(long duration, char *outputString);
WORD swap_endian_word(WORD i);
char * ReturnChannelNameFromPID(int pid);

//These functions are used the subprograms to find out the location of each other
HWND ZorveGetHwndInfo(void);
void ZorveSetHwndInfo(HWND hwnd);
HWND ZorveGetHwndList(void);
void ZorveSetHwndList(HWND hwnd);

#define RGB_ZINNY_DARKBLUE RGB(0x00, 0x00, 0x66)
#define RGB_ZINNY_MIDPURPLE RGB(0xBE, 0xBE, 0xDC)
#define RGB_ZINNY_HIGHPURPLE RGB(0xCC, 0xCC, 0xED)

#define RGB_ZINNY_BLACK RGB(0x00, 0x00, 0x00)
#define RGB_ZINNY_WHITE RGB(0xFF, 0xFF, 0xFF)
#define RGB_ZINNY_BRIGHTBLUE RGB(0x54, 0x9C, 0xF6)

#define RGB_ZINNY_REDALERT RGB(0xA2, 0x0E, 0x06)
#define RGB_ZINNY_ORANGEALERT RGB(0xDE,0x88,0x0D)
#define RGB_ZINNY_GREENALERT RGB(0x00,0x6F,0x10)
