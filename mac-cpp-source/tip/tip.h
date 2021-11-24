
typedef Boolean bool;

extern WindowPtr tipWindow;

void run_tip(int id);

#define MINIMUM_JAZ_SPARES 500
#define MAXIMUM_JAZ_SPARES 2557
#define MINIMUM_ZIP_SPARES 50
#define MAXIMUM_ZIP_SPARES 126

extern long CurrentDevice;
extern bool JazDrive; // true if the current drive
extern long CartridgeStatus;
extern long LastLBAOnCartridge;
extern unsigned long StartingInstant;
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

// ----------------------- Macintosh Compatibility -----------------------

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

void SetColor(long color);
void SetColor(long color, long monoColor);
void DrawLed(int x, int y, long color);
void StrToPascal(Str255 pStr, const char *str);
long GetTextExtent(const char *str, unsigned long len);
void TextOut(int x, int y, Str255 str);
void TextOut(int x, int y, const char *str);
void TextOutCentered(int x, int y, int w, int h, const char *str);
void SetButtonText(const char *str);
void Rectangle(int left, int top, int right, int bottom);
void DrawEdge(Rect *qrc, int edge, int grfFlags);
void PostQuitMessage();
unsigned long GetSystemTime();

#define hTestMonitor -20, -10
#define hMainWnd       0,  40

#define GetDC(h)     {GrafPtr oldPort; \
                     GetPort(&oldPort); \
                     SetPort(tipWindow); \
                     SetOrigin(h);

#define ReleaseDC(h) SetOrigin(0,0); \
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

typedef struct {long id; const char *name; int x; int y; int w; int h; bool visible;} BtnList;
extern BtnList tipBtns[];

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
void ErrorSound();
void ProcessPendingMessages();
void WndProc(long iMessage, long wParam);
void TestMonitorWndProc();
void TestButtonClicked();

void GetCommandDetails(char command, char &cmd_flags, char &cmd_length);
long SCSICommand(short Device, char *lpCmdBlk, void *lpIoBuf, short IoBufLen);
long GetModePage(short Device, short PageToGet, void *pBuffer, short BufLen);
long SetModePage(short Device, void *pBuffer);
void ModifyModePage(char *PageBuff, char eec, char retries);
void SetErrorRecovery(bool Retries, bool ECC, bool Testing);
long GetNonSenseData(short Device, short DataPage, void *Buffer, short BufLen);
long LockCurrentDrive();
long UnlockCurrentDrive();
long SpinUpIomegaCartridge(short Device);
void GetSpareSectorCounts(bool);
long PerformRegionTransfer(short XferCmd, void *pBuffer);
long TestTheDisk();
long GetElapsedTimeInSeconds();
void PrepareToBeginTesting();
void BumpErrorCounts(long ErrorCode);