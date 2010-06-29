#include <windows.h>
#include <windowsx.h>

typedef struct sMpegWindowInfo MPEGWINDOW_INFO;
typedef struct sMpegFileInfo MPEGFILE_INFO;
typedef struct sTsPacket TS_PACKET;
typedef struct sAdaptationField ADAPTATION_FIELD;
typedef struct sHexViewInfo HEXVIEW_INFO;

struct sHexViewInfo	{
	int	selStart;
	int	selEnd;
	int cursorPos;
};

struct sAdaptationField	{
	BYTE adaptationfieldlength;
	BOOL	discontinuity;
	BOOL	randomaccess;
	BOOL	EsPriority;
	BOOL	PCRFlag;
	BOOL	OPCRFlag;
	BOOL	splicingpointflag;
	BOOL	transportprivatedataflag;
	BOOL	adaptationfieldextensionflag;
	ULONGLONG	PcrBase;
	unsigned int	PcrExt;
	ULONGLONG	OpcrBase;
	unsigned int	OpcrExt;
	signed char	splicecountdown;
	BYTE	transportprivatedatalength;
	BYTE	privatedata[182];	//I guess in theory this is the largest possible
	BYTE	adaptationfieldextensionlength;
	BOOL	ltwflag;
	BOOL	piecewiserateflag;
	BOOL	seamlessspliceflag;
	BOOL	ltwvalidflag;
	unsigned int	ltwoffset;
	unsigned long	piecewiserate;
	BYTE	splicetype;
	ULONGLONG	DTSnextAU;	//annoyingly they make a 33-bit value

};

struct sTsPacket	{
	BYTE TS_raw_packet[188];

	BYTE sync_byte;
	BOOL transport_error_indicator;
	BOOL payload_unit_start_indicator;
	BOOL transport_priority;
	unsigned int pid;
	BYTE transport_scrambling_control;
	BYTE adaptation_field_control;
	BYTE continuity_counter;

	ADAPTATION_FIELD adaptation_field;
};

struct sMpegFileInfo	{
	HANDLE	hMpegFile;
	MPEGWINDOW_INFO* windowInfo;

	LONGLONG filesize;
	char filename[MAX_PATH];

	LONGLONG firstSyncByte;
	LONGLONG offset;
	LONGLONG displayOffset;

	HANDLE hBackgroundThread;
	HANDLE hFileAccessMutex;
	BOOL	stopThreadNow;
	BOOL	loadingInBackground;

	unsigned int seenPid[255];
	unsigned long countPid[255];
	unsigned int typePid[255];

	unsigned long countPackets;
	unsigned long countPES;

};

struct sMpegWindowInfo	{
	HWND	zorveHwnd;
	MPEGFILE_INFO fileInfo;
	TS_PACKET	displayedPacket;

	HWND	hwndFileInfo;
	HWND	hwndFileDetail;
	HWND	hwndBlockInfo;
	HWND	hwndBlockDetail;
	HWND	hwndBlockSlideSelector;
	HWND	hwndHexView;

	//In the block info section
	HWND	hwndNextButton;
	HWND	hwndPrevButton;
	HWND	hwndPositionEditbox;
	HWND	hwndSeekButton;
	HWND	hwndLockPID;		//checkboxes. For PID
	HWND	hwndPIDCombobox;
	HWND	hwndLockPayload;
	HWND	hwndLockNavPointer;
	HWND	hwndLockIframe;

	//specific parts
	HEXVIEW_INFO hexview;

	char recordingname[120];
};

int MpegWindowRegisterWndClass(HINSTANCE hInst);
LRESULT CALLBACK ChildWndMpegProc(HWND hwnd,UINT msg,WPARAM wParam,LPARAM lParam);
LRESULT CALLBACK MpegFileInfoProc(HWND hwnd,UINT msg,WPARAM wParam,LPARAM lParam);
LRESULT CALLBACK MpegFileDetailProc(HWND hwnd,UINT msg,WPARAM wParam,LPARAM lParam);
LRESULT CALLBACK MpegPacketInfoProc(HWND hwnd,UINT msg,WPARAM wParam,LPARAM lParam);
LRESULT CALLBACK MpegPacketDetailProc(HWND hwnd,UINT msg,WPARAM wParam,LPARAM lParam);

HWND MpegWindowCreateOrShow(HWND hwnd, HWND hwndChild, HINSTANCE hInst);
HWND MpegWindowCreate(HWND hwndMDIClient, HINSTANCE hInst);

int MpegWindowPaint(HWND hwnd);
int MpegFileInfoPaint(HWND hwnd);
int MpegFileDetailPaint(HWND hwnd);
int MpegPacketInfoPaint(HWND hwnd);
int MpegHexView(HWND hwnd);

BOOL MpegChangePacket(MPEGWINDOW_INFO * mpegWindowInfo, LPARAM buttonPressed); //returns ?redraw

HANDLE MpegWindowLoadFile(HWND hwnd, char * mpegFile);
int MpegTSFindSyncByte(HANDLE hFile, LONGLONG * syncbyteOffset);
int MpegReadPacket(MPEGFILE_INFO *mpegFileInfo, TS_PACKET *packet);
int MpegPrepareForClose(HWND hwnd);

DWORD WINAPI MpegReadFileStats(MPEGFILE_INFO *mpegFileInfo);
