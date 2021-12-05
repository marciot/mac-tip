#include "tip.h"

const char *szWindowTitle = "TIP 2.1b -- Zip & Jaz Drive and Cartridge Testing System";
const char *szCopyright_1 = "Copyright (c) 2006 by";
const char *szCopyright_2 = "Gibson Research Corp.";

const char *szIomega = "IOMEGA";
const char *szZip = "ZIP";
const char *szJaz = "JAZ";

const char *szSide0 = "Side 0";
const char *szSide1 = "Side 1";
const char *szSpaceDashSpace = " - ";

const char *szBarChartPercent = " %ld%% ";
const char *szCenteredDecimal = "%ld";
const char *szCenteredHex     = "ErrorCode: %06lX";
const char *szHoursMinsSecs   = "%ld:%02ld:%02ld";

const char *szCartStatus = "Cartridge Status:";
const char *szEstimating = "Estimating ...";
const char *szOneMoment  = "---  ---  ---";
const char *szPressToStart = "Press to Begin";
const char *szPressToStop = "Press to Stop";
const char *szPressToSpin = "Press to Spin Up";
const char *szPressToEject = "Press to Eject";
const char *szPressToProceed = "Press to Proceed";

// Filenames

const char *szInstructions = "Instructions";
const char *szNoASPI = "No ASPI";
const char *szASPITrouble = "No Drives";
const char *szPPAVersion = "Parallel Port";
const char *szDefectList = "Z-Track Failure";
const char *szLocked = "Cartridge Locked";
const char *szNoSpares = "No Spares";
const char *szOutOfSpares = "Out of Spares";
const char *szFewSpares = "Few Spares";
const char *szNotRunning = "Not Running";
const char *szRunning = "Test Running";
const char *szInterrupted = "Test Interrupted";
const char *szPerfectResult = "Perfect Result";
const char *szExplainResult = "Good Result";
const char *szBadResult = "Bad Result";
const char *szIomegaQuote = "Iomega Quote";

/************* Cartridge Status Text *************/

const char *szUnknownStat    = "Ejecting Cartridge";
const char *szAtSpeedStat    = "Ready to Test";
const char *szSpinningUp     = "Spinning Up";
const char *szNotPresent     = "Awaiting Cartridge";
const char *szSpunDown       = "Not Spinning";
const char *szStalledStat    = "Stalled Error";
const char *szZtrackFailure  = "Z-Tracks Failure !!";
const char *szDiskLocked     = "Disk Protected";
const char *szLowSpares      = "Low Spares Count";
const char *szTestUnderway   = "Testing Drive ";
const char *DriveUnderTest   = "X: ...";
const char *szTestFailure    = "Testing Failed";
const char *szNoIomegaDrives = "No Iomega Drives";

const char *CartStatStrings[] = {
    szUnknownStat, szAtSpeedStat, szSpinningUp,
    szNotPresent, szSpunDown, szStalledStat,
    szZtrackFailure, szDiskLocked, szLowSpares,
    szTestUnderway, szTestFailure
};

ErrorTypeList errorTypeList[] = {
    0x00000000, "--  None So Far  --",

    0x001C0000, "Missing Defect List",

    0x000C0101, "Wrote with Realloc",
    0x000C8001, "Wrote with Retries",
    0x000C8101, "Wrote with Off Track",
    0x000C8201, "Wrote w/o SectorMark",
    0x000C8301, "Wrote with ID skip",
    0x00170101, "Read with Retries",
    0x00170601, "Retried & Realloc",
    0x00180001, "Data Corrected",
    0x00180101, "ECC & Retries",
    0x00180201, "ECC & Realloc'd",
    0x001C8F01, "Defect List Recvr'd",

    0x00040002, "Drive Not Ready",
    0x00040102, "Drive Going Ready",
    0x00040202, "Drive Not Ready #2",
    0x00040302, "Drive Needs Help",
    0x00040402, "Not Rdy - Formating",
    0x00300002, "Incompatible Media",
    0x003A0002, "Media Not Present",

    0x00010003, "Missing Sector Mark",
    0x00030003, "Off Track Write",
    0x00100003, "Bad ID Checksum",
    0x00110003, "Unrecovered Read",
    0x00118003, "Unrecovered Read",
    0x00120003, "MissingID Sync",
    0x00130003, "Missing Addr Mark",
    0x00140003, "Sector Not Found",
    0x00160003, "Sync Mark Missing",
    0x001C0003, "Defect List Error",
    0x00310003, "Corrupt Media Format",
    0x00310103, "Format Command Fail",
//  0x0031xx03, " -- failures --"
    0x00320003, "No Spare Sectors", // abort on this
    0x00328F03, "No Spare Tracks", // abort on this

    0x00018104, "Missing Sector Pulse",
    0x00090004, "Track Follow Error",
    0x00150004, "Head Seek Failure",
    0x00220004, "Cartridge Sense Fail",
//  0x0040xx04, "Self Test Failed",
    0x00470004, "Data Parity Error",
//  0x00xx0004, "Vendor Specific Error",

    0x00290006, "I/O Bus Reset Error",

    0x0088010B, "Reassigned Blk Err",
    0x0088020B, "Side Switch Error",
    0x00FFFFE6, "Buffer Too Big",

    0xFFFFFFFF, "-- Unknown Error --",

    0, 0
};

/********** Main Window Controls Text ************/

const char *szIntroTitle = "Trouble in Paradise";
const char *szIntroSubTitle = "FREEWARE by Steve Gibson\r" \
                              "Gibson Research Corporation\r" \
                              "http://grc.com                  ( v 2.1b )\r" \
                              "http://github.com/marciot/mac-tip";
const char *szIntroText =
    "A Macintosh port of \"TIP\" for Windows by Marcio Teixeira, made possible by a "
    "generous code donation by Steve Gibson.\r\r"
    "This freeware utility determines whether an Iomega Zip or Jaz drive is prone "
    "to developing the dreaded \"Click of Death\" syndrome. Gibson's "
    "research into the maintenance, repair and data recovery of Iomega's removable "
    "media mass storage products led to this capability.";

/****************** Control Text *****************/

const char *szBack = "< Back";
const char *szNext = "Next >";
const char *szQuit = "Exit";

/*******************************************************************************
 * PERFORM_TEST_PAGE
 *******************************************************************************/

TextList TestBlackText[] = {
    {250,   2, "Data Read"},
    {350,   2, "Patt Write"},
    {250,  18, "Data Write"},
    {350,  18, "Patt Read"},
    {11,   39, "0%"},
    {377,  39, "100%"},
    { 11,  77, "0"},
    {377,  77, "100%"},
    { 22, 154, "Sectors"},
    { 10, 171, "Last Error"},
    { 17, 188, "Elapsed"},
    { 13, 205, "Time Left"},
    {278, 154, "Soft Errors"},
    {275, 171, "Firm Errors"},
    {272, 188, "Hard Errors"},
    {271, 205, "Total Errors"},
    {0,0,0}
};

TextList TestGrayText[] = {
    {155,  39, "Testing Progress"},
    {129,  77, "Spare Sectors Consumed"},
    {61,  135, "Testing Location"},
    {219, 135, "Sound"},
    {297, 135, "Error Summary"},
    {0,0,0}
};


// ----------------------- Test Monitor Panel Definitions   -------------------

#define SET_RECT(LEFT, TOP, RIGHT, BOTTOM) {TOP, LEFT, BOTTOM, RIGHT}

Rect CS_Stat = SET_RECT(114,  8,  242,  28);
Rect TP_Perc = SET_RECT( 12,  56, 409,  72);
Rect SS_Jaz  = SET_RECT( 12,  94, 409, 126);
Rect SS_Sid0 = SET_RECT( 12,  94, 409, 110);
Rect TL_Sect = SET_RECT( 75, 154, 203, 170);
Rect ES_Read = SET_RECT(346, 154, 409, 170);
Rect SE_Rect = SET_RECT(222, 154, 255, 221);

BtnList tipBtns[] = {
    {IDB_BACK, szBack, 157, 301, 80, 24},
    {IDB_NEXT, szNext, 236, 301, 80, 24},
    {IDB_QUIT, szQuit, 402, 301, 45, 24},
    {IDB_BEEP, "", 252, 211, 22, 22, checkBoxProc},
     // For Mac TIP only
    {IDB_TEST, szPressToStart, 157, 301, 150, 24},
    {IDB_EXPL, "Explain", 330, 301, 60, 24},
    {IDB_OKAY, "Okay",    380, 301, 65, 24},
    {IDB_READ, "Open in SimpleText...", 210, 301, 160, 24},
    {0, 0, 0, 0, 0, 0}
};