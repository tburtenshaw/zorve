#include <windows.h>
#include <windowsx.h>

typedef struct sNavWindowInfo NAVWINDOW_INFO;
typedef struct sNavFileInfo NAVFILE_INFO;
typedef struct sNavRecord NAV_RECORD;

struct sNavRecord {
   unsigned short s0;    //Sawtooth pattern, with same increments as L6
                         //Drops everytime lowbyte of var2 increases.

   unsigned short twobytes2; //LOWBYTE
                             //The low byte increments by 1 every few records
                             //Its increase is coincidental with the sawtooth drop of s1
                             //It resets itself at the same time as l12 does.
                             //It usually stays ten or under (usually in the single digits)

                             //HIBYTE
                             //The high byte seems to be of value either 1, 2 or 3

   unsigned short twobytes4;    //Two bytes. The high byte increments by 1 each record.
                         //The lowbyte is always 30.
                         //Sawtooth pattern
                         //Drops the record after s12
                         //Period between ~23 and ~71, usually an odd number

   unsigned short s6;    //Always 8

   unsigned long offsethi;    //The high-order long of the offset (l6)

   unsigned long offsetlo;    //Near linear, but varying increments averaging 12700 (max 69000, min 1800)
                        //Larger change of values generally coincide with sawtooth drop of s3
                        //Strongly suspect this refers to a byte in the associated mpg file...
                        //It seems to coincide with a payload indicator of the video PID.

   unsigned long timer;    //VERY high number, slowly increases linearly, with superimposed wave of period 7
                        //(+900, -4500, +900, -2700, +900, +2700, +900, +8100)
                        //Could this be Presentation or Decoding Time Stamp?
                        //Looks to be 50% of program_clock_reference_base (this is a 33bit value in the MPEG-TS)
                        //Actually, it seems to be the 32 high bits of the 33 bit PTS.
   unsigned short s14;

   unsigned short s16;    //Usually zero, occasionally 1 or 2

   unsigned long milliseconds;        //Is directly proportional to L7. This is the number of milliseconds

   unsigned long l1Czero;    //Zero

   unsigned long l20;    //Sawtooth pattern.
                         //Goes up by exactly same as l6 (about 9000-20000) each record.
                         //Period exactly same as s3, except fall happens one record prior.
                         //Maximum not limited by type of int

   unsigned short s24;   //Always 30

   unsigned short s26zero;   //All these below are typically (?always) zero
   unsigned long l28zero;    //
   unsigned long l2Czero;    //
   unsigned long l30zero;    //
   unsigned long l34zero;    //
   unsigned long l38zero;    //
   unsigned long l3Czero;    //
};


struct sNavFileInfo	{
	HANDLE	hNavFile;
	unsigned long filesize;
	char filename[MAX_PATH];

	long numRecords;
	double estTimeSpanSeconds;
};

struct sNavWindowInfo	{
	NAVFILE_INFO fileInfo;

	HWND hwndRecordList;
	HWND hwndRecordListHeader;

	int headerHeight;
	int listviewheaderHeight;

	long firstDisplayedRecord;
	long indexRecordBuffer;
	int numDisplayedLines;

	NAV_RECORD displayRecord[1024];	//I want to load 1024, starting record nearest 512

	int	mousePushed;
	int mouseDragging;
	int movingColumn;		//the column adjuster highlighted

	int widthColumn[24];	//0 is the key, 1 is s1
	char columnTitle[24][64];

};

int NavWindowRegisterWndClass(HINSTANCE hInst);
LRESULT CALLBACK ChildWndNavProc(HWND hwnd,UINT msg,WPARAM wParam,LPARAM lParam);
LRESULT CALLBACK NavRecordListViewFileProc(HWND hwnd,UINT msg,WPARAM wParam,LPARAM lParam);
LRESULT CALLBACK NavRecordListViewHeaderProc(HWND hwnd,UINT msg,WPARAM wParam,LPARAM lParam);


HWND NavWindowCreateOrShow(HWND listHwnd, HWND hwndMDIClient, HINSTANCE hInst);
HWND NavWindowCreate(HWND hwndMDIClient, HINSTANCE hInst);

int	NavRecordListHeaderPaint(HWND hwnd);
int NavRecordListViewPaint(HWND hwnd);
int NavRecordHeaderPaint(HWND hwnd);
int NavRecordListHeaderCheckAdjustBar(NAVWINDOW_INFO * navWindowInfo, int x, int y);
int NavWindowListHeaderHandleContextMenu(HWND hwnd, WPARAM wparam, LPARAM lparam);

void NavInitiateColumnWidths(NAVWINDOW_INFO * navWindowInfo);

int NavHandleVScroll(HWND hwnd, WPARAM wparam, LPARAM lparam);
int NavScrollUpdate(HWND hwnd, NAVWINDOW_INFO * navWindowInfo);
long NavWindowOnMouseWheel(HWND hwnd, short nDelta);

int ReadRecordsFromNavFile(NAVWINDOW_INFO *navWindowInfo, unsigned long offset, NAV_RECORD* ptrRecord, long numRecords);
HANDLE NavWindowLoadFile(HWND hwnd, char *openfilename);
int NavWindowUnloadFile(HWND hwnd);	//closes the file handle, and frees the memory
