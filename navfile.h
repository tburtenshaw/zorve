#include <windows.h>
#include <windowsx.h>

typedef struct sNavWindowInfo NAVWINDOW_INFO;
typedef struct sNavFileInfo NAVFILE_INFO;
typedef struct sNavRecord NAV_RECORD;

struct sNavRecord {
   unsigned short s1;    //Sawtooth pattern, with same increments as L6
                         //Drops everytime lowbyte of var2 increases.

   unsigned short twobytes2; //I believe this is actually two bytes

                             //LOWBYTE
                             //The low byte increments by 1 every few records
                             //Its increase is coincidental with the sawtooth drop of s1
                             //It resets itself at the same time as l12 does.
                             //It usually stays ten or under (usually in the single digits)

                             //HIBYTE
                             //The high byte seems to be of value either 1, 2 or 3

   unsigned short s3;    //Increments by 256 each record (?may be misreading by a byte)
                         //Sawtooth pattern
                         //Drops the record after s12
                         //Period between ~23 and ~71, usually an odd number

   unsigned short s4;    //Always 8

   unsigned long l5zero;    //Always zero

   unsigned long l6;    //Near linear, but varying increments averaging 12700 (max 69000, min 1800)
                        //Larger change of values generally coincide with sawtooth drop of s3
                        //Strongly suspect this refers to a byte in the associated mpg file...
                        //It seems to coincide with a payload indicator of the PID $002A.

   unsigned long l7;    //VERY high number, slowly increases linearly, with superimposed wave of period 7
                        //(+900, -4500, +900, -2700, +900, +2700, +900, +8100)
                        //Could this be Presentation or Decoding Time Stamp?
   unsigned short s8;

   unsigned short s9zero;    //Zero

   unsigned long l10;

   unsigned long l11zero;    //Zero

   unsigned long l12;    //Sawtooth pattern.
                         //Goes up by exactly same as l6 (about 9000-20000) each record.
                         //Period exactly same as s3, except fall happens one record prior.
                         //Maximum not limited by type of int

   unsigned short s13;   //Always 30

   unsigned short s14zero;   //All these below are typically (?always) zero
   unsigned long l15zero;    //
   unsigned long l16zero;    //
   unsigned long l17zero;    //
   unsigned long l18zero;    //
   unsigned long l19zero;    //
   unsigned long l20zero;    //
};

struct sNavFileInfo	{
	HANDLE	hNavFile;
	unsigned long filesize;
	char filename[MAX_PATH];

	long numRecords;
};

struct sNavWindowInfo	{
	NAVFILE_INFO fileInfo;

	HWND hwndRecordList;
	int headerHeight;

	long firstDisplayedRecord;
	long indexRecordBuffer;
	int numDisplayedLines;

	NAV_RECORD displayRecord[1024];	//I want to load 1024, starting record nearest 512


	int widthColumn[24];	//0 is the key, 1 is s1

};

int NavWindowRegisterWndClass(HINSTANCE hInst);
LRESULT CALLBACK ChildWndNavProc(HWND hwnd,UINT msg,WPARAM wParam,LPARAM lParam);
LRESULT CALLBACK NavRecordListViewFileProc(HWND hwnd,UINT msg,WPARAM wParam,LPARAM lParam);

HWND NavWindowCreateOrShow(HWND listHwnd, HWND hwndMDIClient, HINSTANCE hInst);
HWND NavWindowCreate(HWND hwndMDIClient, HINSTANCE hInst);

int NavRecordListViewPaint(HWND hwnd);
int NavRecordHeaderPaint(HWND hwnd);
int NavHandleVScroll(HWND hwnd, WPARAM wparam, LPARAM lparam);
int NavScrollUpdate(HWND hwnd, NAVWINDOW_INFO * navWindowInfo);

int ReadRecordsFromNavFile(NAVWINDOW_INFO *navWindowInfo, unsigned long offset, NAV_RECORD* ptrRecord, long numRecords);
int NavWindowLoadFile(HWND hwnd, char *openfilename);
