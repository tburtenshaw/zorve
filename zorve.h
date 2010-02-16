#include <windows.h>
#include <windowsx.h>


#define Int32x32To64(a, b) ((LONGLONG)((LONG)(a)) * (LONGLONG)((LONG)(b)))

void UnixTimeToFileTime(long unixtime, LPFILETIME pft);
void BytesDisplayNice(long bytes, char *formatString, int thresholdratio, char *outputString);
void DurationShortFormatDHMS(long duration, char *outputString);
