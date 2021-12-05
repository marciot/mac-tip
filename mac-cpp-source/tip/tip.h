#include "ctype.h"

extern WindowPtr tipWindow;

void run_tip();

#define MINIMUM_JAZ_SPARES 500
#define MAXIMUM_JAZ_SPARES 2557
#define MINIMUM_ZIP_SPARES 50
#define MAXIMUM_ZIP_SPARES 126

extern long CurrentDevice;
extern long DriveCount;
extern long JazDrive; // true if the current drive
extern long CartridgeStatus;
extern long LastLBAOnCartridge;
extern uint32_t StartingInstant;
extern long NumberOfLBAs;
extern long Side_0_SparesCount; // JAZ has only one count
extern long Side_1_SparesCount; // ZIP has counts for both sides
extern long Initial_Side_0_Spares;
extern long Initial_Side_1_Spares;
extern long TestingPhase; // 0 = not testing, no data ...
extern long PercentComplete;
extern long FirstLBASector;
extern long NumberOfLBAs;
extern long SecondsElapsed;
extern long SoftErrors;
extern long FirmErrors;
extern long HardErrors;
extern long ElapsedTimeOfLastEstimate;
extern long CurrentTotalTimeEstimate;
extern bool UserInterrupt;
extern long LastError;
extern long SingleTransferLBA;

//-------------------------- Drive Array Status Flags ---------------------------

#define JAZ_DRIVE             0x01
#define MEDIA_CHANGED         0x02
#define DISK_EJECTING         0x04 // we've asked for eject and waiting ...
#define ODD_BYTE_COMPENSATION 0x08 // special handling for ODD length PSWD
#define MOUNTED_DRIVE         0x10 // drive was mounted at startup, ignore it
#define MAX_DRIVE_COUNT       8    // we can handle up to 8 Zip/Jaz drives

struct DriveEntry {
    uint8_t flags;
    uint8_t scsi_id;
};

extern DriveEntry DriveArray[MAX_DRIVE_COUNT];

// ----------------------- Macintosh Compatibility -----------------------

enum AlertTypes {
    ERR_DLG,
    YN_DLG
};

enum {
    BACK_COLOR   = -1,
    BLACK_COLOR  = 0x000000,
    LTGRAY_COLOR = 0xc0c0c0,
    GRAY_COLOR   = 0x808080,
    WHITE_COLOR  = 0xffffff,
    BLUE_COLOR   = 0x0000ff,
    RED_COLOR    = 0xff0000,
    GREEN_COLOR  = 0x00ff00,
};

#define BDR_SUNKENOUTER 1
#define BF_RECT 1
#define WM_PAINT 1
#define WM_COMMAND 2
#define SW_SHOW 1
#define SW_HIDE 2
#define BM_GETCHECK 1
#define BST_CHECKED 1

void SetRGBColor(long color, RGBColor *rgbColor);
void SetColor(long color);
void SetColor(long color, long monoColor);
void SetBackColor(long color);
void DrawLed(int x, int y, long color);
void StrToPascal(Str255 pStr, const char *str);
int ShowAlert(AlertTypes type, const char* format, ...);
void SetRichEditText(const char *name);
long GetTextExtent(const char *str, unsigned long len);
void TextOut(int x, int y, Str255 str);
void TextOut(int x, int y, const char *str);
void TextOutCentered(int x, int y, int w, int h, const char *str);
void SetWindowText(int id, const char *str);
void EnableWindow(int id, bool enabled);
void ShowWindow(ControlHandle hCntl, int state);
void ShowWindow(int id, int state);
long SendMessage(int id, int msg);
void InvalidateRect(int id);
void Rectangle(int left, int top, int right, int bottom);
void DrawEdge(Rect *qrc, int edge, int grfFlags);
void StartApplicationTimer();
void StopApplicationTimer();
void PostQuitMessage();
unsigned long GetSystemTime();
bool PrepareDC(int which);

#define GetDC(h)     {GrafPtr oldPort; \
                     GetPort(&oldPort); \
                     if(PrepareDC(h)) {

#define ReleaseDC(h) } SetOrigin(0,0); \
                     SetPort(oldPort);}


// ------------------------------   Cartridge Status -------------------------------

enum {
    DISK_STATUS_UNKNOWN  = 1,
    DISK_AT_SPEED        = 2,
    DISK_SPINNING_UP     = 3,
    DISK_NOT_PRESENT     = 4,
    DISK_SPUN_DOWN       = 5,
    DISK_STALLED         = 6,
    DISK_Z_TRACK_FAILURE = 7,
    DISK_PROTECTED       = 8,
    DISK_LOW_SPARES      = 9,
    DISK_TEST_UNDERWAY   = 10,
    DISK_TEST_FAILURE    = 11,

    LAST_CART_STATUS     = 11
};

// ---------------------------- Testing Phase Status -----------------------------

enum {
    UNTESTED              = 0,
    READY_TO_TEST         = 1,
    TESTING_STARTUP       = 2,
    READING_DATA          = 3,
    WRITING_PATT          = 4,
    READING_PATT          = 5,
    WRITING_DATA          = 6
};

/*******************************************************************************
 * STRINGS
 *******************************************************************************/

extern const char *szWindowTitle;
extern const char *szCopyright_1;
extern const char *szCopyright_2;
extern const char *szIomega;
extern const char *szZip;
extern const char *szJaz;
extern const char *szSide0;
extern const char *szSide1;
extern const char *szSpaceDashSpace;
extern const char *szBarChartPercent;
extern const char *szCenteredDecimal;
extern const char *szCenteredHex;
extern const char *szHoursMinsSecs;
extern const char *szCartStatus;
extern const char *szEstimating;
extern const char *szOneMoment;
extern const char *szPressToStart;
extern const char *szPressToStop;
extern const char *szPressToSpin;
extern const char *szPressToEject;
extern const char *szPressToProceed;

/************* Filenames *************/

extern const char *szInstructions;
extern const char *szNoASPI;
extern const char *szASPITrouble;
extern const char *szPPAVersion;
extern const char *szDefectList;
extern const char *szLocked;
extern const char *szNoSpares;
extern const char *szOutOfSpares;
extern const char *szFewSpares;
extern const char *szNotRunning;
extern const char *szRunning;
extern const char *szInterrupted;
extern const char *szPerfectResult;
extern const char *szExplainResult;
extern const char *szBadResult;
extern const char *szIomegaQuote;

/************* Cartridge Status Text *************/

typedef struct {long code; char *str;} ErrorTypeList;
typedef struct {int x, y;  char *str;} TextList;

extern const char *szUnknownStat;
extern const char *szAtSpeedStat;
extern const char *szSpinningUp;
extern const char *szNotPresent;
extern const char *szSpunDown;
extern const char *szStalledStat;
extern const char *szZtrackFailure;
extern const char *szDiskLocked;
extern const char *szLowSpares;
extern const char *szTestUnderway;
extern const char *DriveUnderTest;
extern const char *szTestFailure;
extern const char *szNoIomegaDrives;

extern const char *CartStatStrings[];

extern ErrorTypeList errorTypeList[];
extern TextList TestBlackText[];
extern TextList TestGrayText[];

/************* Window Creation Data **************/

extern const char *szBack;
extern const char *szNext;
extern const char *szQuit;

#define IDB_BACK 0xFF00
#define IDB_NEXT 0xFF01
#define IDB_QUIT 0xFF02
#define IDB_TEST 0xFF03
#define IDB_EXPL 0xFF04
#define IDB_OKAY 0xFF05
#define IDB_READ 0xFF06
#define IDB_BEEP 0xFF07

enum {
    hDefault,
    hMainWnd,
    hTestMonitor,
    hTestButton = IDB_TEST,
    hExitButton = IDB_QUIT,
    hSoundCheckbox,
    // Extras added by MLT
    hExplainWnd = IDB_EXPL
};

typedef struct {
    int id;
    const char *name;
    int x;
    int y;
    int w;
    int h;
    int type;
    ControlHandle hndl;
} BtnList;
extern BtnList tipBtns[];

extern Rect CS_Stat, TP_Perc, SS_Jaz, SS_Sid0, TL_Sect, ES_Read, SE_Rect;

/*******************************************************************************
 * FUNCTION PROTOTYPES
 *******************************************************************************/

void PaintCenteredString(int Xleft, int Ytop, int XWidth, int YHeight, const char *pText, bool Active);
void PaintCenteredValue(int Xleft, int Ytop, int XWidth, int YHeight, long value, bool Active);
void SunkenFields(Rect *pFirstRect, long count, long yspacing);
void PaintCartStatus();
void PaintTextArray(TextList *list, long color);
void PaintBarGraph(int Xleft, int Ytop, int XWidth, int YHeight, long BarColor, long BarValue, char *pRightText, bool Active);
int CvrtIntoPrcnt(long val, long max);
void PaintTestPhase();
void PaintTheBarGraphs(bool Active);
void PaintTestStatistics(bool Active);
void CvrtSecondsToHMSstring(char *szString, long seconds);

void UpdateCurrentSector();
void UpdateRunTimeDisplay();
void UpdateRunPhaseDisplay();
void PreventProgramExit();
void AllowProgramExit();
void ErrorSound();
void ProcessPendingMessages();
void WinMain(uint8_t *DrivesSkipped);
void WndProc(long iMessage, uint16_t wParam);
void TestMonitorWndProc();
void ApplicationTimerProc();
void TestButtonClicked();

int GetDriveEntryOffset(short Device);
void GetCommandDetails(char command, char &cmd_flags, char &cmd_length);
long SCSICommand(short Device, char *lpCmdBlk, void *lpIoBuf, short IoBufLen);
long EnumerateIomegaDevices(uint8_t *DrivesSkipped);
long GetModePage(short Device, short PageToGet, void *pBuffer, short BufLen);
long SetModePage(short Device, void *pBuffer);
void ModifyModePage(char *PageBuff, char eec, char retries);
void SetErrorRecovery(bool Retries, bool ECC, bool Testing);
long GetNonSenseData(short Device, short DataPage, void *Buffer, short BufLen);
long LockCurrentDrive();
long UnlockCurrentDrive();
void UnlockAllMedia();
long SpinUpIomegaCartridge(short Device);
void EjectAllMedia();
long GetSpareSectorCounts(char);
uint8_t GetCartridgeStatus(long Device);
void HandleDriveChanging();
void SetCartridgeStatusToEAX(long eax);
void EjectIomegaCartridge(int Device);
long PerformRegionTransfer(short XferCmd, void *pBuffer);
void TestTheDisk();
long GetElapsedTimeInSeconds();
void PrepareToBeginTesting();
void BumpErrorCounts(long ErrorCode);
void EjectIomegaCartridge();
