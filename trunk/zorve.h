#define MAXLEN_RECORDINGTITLE 120;

#include <windows.h>
#include <windowsx.h>

#define ZFT_UNKNOWN 0
#define ZFT_INFO 1
#define ZFT_MPEGTS 2
#define ZFT_NAV 3

#define ZORVEWINDOWMARGIN 5

#define Int32x32To64(a, b) ((LONGLONG)((LONG)(a)) * (LONGLONG)((LONG)(b)))
#define MAX(a,b) (a > b ? a : b)
#define MIN(a,b) (a < b ? a : b)

int IdentifyFileType(char *filename);

void UnixTimeToFileTime(long unixtime, LPFILETIME pft);
void ModJulianTimeToFileTime(int juliantime, LPFILETIME pft);
void BytesDisplayNice(long bytes, char *formatString, int thresholdratio, char *outputString);
void DurationShortFormatDHMS(long duration, char *outputString);
WORD swap_endian_word(WORD i);
int UnsignedLongLongToString(ULONGLONG ull, char *s);
char * ReturnChannelNameFromPID(int pid);
int FilenameMpegOrNavToInfo(char *buffer, char *filename);	//returns number of chars before .

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

#define RGB_ZINNY_COLOUR_SYNC		RGB(47,	188,47)
#define RGB_ZINNY_COLOUR_ERROR		RGB(245,131,0)
#define RGB_ZINNY_COLOUR_PAYLOAD	RGB(69,168,	255)
#define RGB_ZINNY_COLOUR_PRIORITY	RGB(255,0,	141)

#define RGB_ZINNY_COLOUR_SYNC_FADE		RGB((47+0xBE)/2,	(188+0xBE)/2,	(47+0xBE)/2)
#define RGB_ZINNY_COLOUR_ERROR_FADE		RGB((245+0xBE)/2,(131+0xBE)/2,(0+0xBE)/2)
#define RGB_ZINNY_COLOUR_PAYLOAD_FADE	RGB((69+0xBE)/2,(168+0xBE)/2,	(255+0xBE)/2)
#define RGB_ZINNY_COLOUR_PRIORITY_FADE	RGB((255+0xBE)/2,(0+0xBE)/2,	(141+0xBE)/2)


//Define the my custom window messages
//These should all be initially handled by main zorve.c (but may be transfered to a child window)
//i.e. SendMessage to the main HWND to get this done. (Potentially can send to HWND in same source file.)
#define ZM_OPENNAV_DLG		(WM_USER+0)	//will bring up dialog to choose a NAV by default
#define ZM_OPENMPEG_DLG 	(WM_USER+1) //will bring up dialog to choose an MPEG (i.e. list of mpgs by default)
#define ZM_OPENFILENAV		(WM_USER+2)	//Open up particular nav file, wParam is the file
#define ZM_OPENFILEMPEG		(WM_USER+3)	//Open up a specified mpeg file
#define ZM_OPENFILEINFO		(WM_USER+4)
#define ZM_LIST_SELECTFROMFILEANDREFRESH	(WM_USER+10)	//Select the file as specified
#define ZM_LIST_SELECTFROMFILE	(WM_USER+11)
#define ZM_MPEG_SKIPTOOFFSET (WM_USER+20)	//Skip to a specified point
#define ZM_REQUEST_RECORDINGNAME (WM_USER+30)	//Send this using lParam as receive window, wParam as filename
#define ZM_REPLY_RECORDINGNAME (WM_USER+31)	//Send this back to window with wParam as text

#define ZM_INFO_CHANGEMPEGSTATUS (WM_USER+40)	//Lparam either -1, 0, 1
#define ZM_INFO_CHANGENAVSTATUS (WM_USER+41)


//Define timers
#define IDT_MPEG_SCANDISPLAY 1
