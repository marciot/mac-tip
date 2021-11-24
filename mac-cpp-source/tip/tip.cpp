/*******************************************************************************
 *    TIP.CPP                  TROUBLE IN PARADISE                  05/22/98   *
 ******************************************************************************/

#include <limits.h>
#include <string.h>
#include <stdio.h>

#include "tip.h"

// ----------------------- Test Monitor Panel Definitions   -------------------
#define SET_RECT(LEFT, TOP, RIGHT, BOTTOM) {TOP, LEFT, BOTTOM, RIGHT}

Rect CS_Stat = SET_RECT(114,  8,  242,  28);
Rect TP_Perc = SET_RECT( 12,  56, 409,  72);
Rect SS_Jaz  = SET_RECT( 12,  94, 409, 126);
Rect SS_Sid0 = SET_RECT( 12,  94, 409, 110);
Rect TL_Sect = SET_RECT( 75, 154, 203, 170);
Rect ES_Read = SET_RECT(346, 154, 409, 170);
Rect SE_Rect = SET_RECT(222, 154, 255, 221);

/*******************************************************************************
 * WndProc
 *
 * This is the system's main window procedure
 */
void WndProc(long iMessage, long wParam) {
    // -------------------------------------------------------------------------
    // WM_PAINT
    // -------------------------------------------------------------------------
    if (iMessage == WM_PAINT) {
        // Draw the Lower Horz Button Divider

        SetColor(GRAY_COLOR);
        MoveTo(15, 289);
        LineTo(446, 289);
        SetColor(WHITE_COLOR);
        LineTo(446, 290);
        LineTo(14, 290);

        // Paint the Copyright Notice
        SetColor(GRAY_COLOR);
        TextOut(15, 298, szCopyright_1);
        TextOut(15, 311, szCopyright_2);
    }
    // -------------------------------------------------------------------------
    // WM_COMMAND : a button was pressed
    // -------------------------------------------------------------------------
    else if (iMessage == WM_COMMAND) {
        switch(wParam) {
            case IDB_QUIT:
                if (TestingPhase < TESTING_STARTUP) {
                    PostQuitMessage();
                }
                break;
            case IDB_TEST:
                switch(CartridgeStatus) {
                    case DISK_SPUN_DOWN:
                        SpinUpIomegaCartridge(CurrentDevice);
                        break;
                    case DISK_AT_SPEED:
                        printf("Testing the disk\n");
                        if(TestingPhase != READY_TO_TEST) {
                            PrepareToBeginTesting();
                        }
                        TestTheDisk();
                        printf("Test finished\n");
                        break;
                    case DISK_TEST_UNDERWAY:
                        UserInterrupt = 1;
                        break;
                    case DISK_Z_TRACK_FAILURE:
                    case DISK_TEST_FAILURE:
                    case DISK_PROTECTED:
                        //EjectIomegaCartridge();
                        break;
                    case DISK_LOW_SPARES:
                        PrepareToBeginTesting();
                        break;
                }
                break;
            default:
                break;
        }
    }
}

BtnList tipBtns[] = {
    {IDB_TEST, szPressToStart, 200, 301, 160, 24, true}, // Added by MLT
    {IDB_BACK, szBack, 185-28, 301, 80, 24, false},
    {IDB_NEXT, szNext, 264-28, 301, 80, 24, false},
    {IDB_QUIT, szQuit, 367+35, 301, 45, 24, true},
    {0, 0, 0, 0, 0, 0, 0}
};

/*******************************************************************************
 * SUNKEN FIELDS
 *******************************************************************************/
void SunkenFields(Rect *pFirstRect, long count, long yspacing) {
    Rect drawRect;
    drawRect = *pFirstRect; // make a local copy
    do {
        DrawEdge(&drawRect, BDR_SUNKENOUTER, BF_RECT);
        drawRect.top += yspacing;
        drawRect.bottom += yspacing;
    } while(--count);
}

/*******************************************************************************
 * PAINT TEXT ARRAY
 *******************************************************************************/
void PaintTextArray(TextList *list, long color) {
    SetColor(color);
    for(int i = 0; list[i].str; i++) {
        TextOut(list[i].x, list[i].y, list[i].str);
    }
}

/*******************************************************************************
 * PAINT TEST PHASE
 *******************************************************************************/
void PaintTestPhase() {
    DrawLed(320, 5,  TestingPhase == READING_DATA ? GREEN_COLOR : BACK_COLOR);
    DrawLed(336, 5,  TestingPhase == WRITING_PATT ? RED_COLOR   : BACK_COLOR);
    DrawLed(336, 21, TestingPhase == READING_PATT ? GREEN_COLOR : BACK_COLOR);
    DrawLed(320, 21, TestingPhase == WRITING_DATA ? RED_COLOR   : BACK_COLOR);
}

/*******************************************************************************
 * PAINT CART STATUS
 *
 * Paints the textual cartridge status window
 *******************************************************************************/
void PaintCartStatus() {
    // blank out any previous status display
    SetColor(BACK_COLOR);
    Rectangle(115, 9, 241, 27);
    // display the new cartridge status
    long eax = CartridgeStatus - 1;
    if(eax < 0 || eax >= LAST_CART_STATUS) {
        eax = 0;
    }
    // pickup the pointer to the string
    const char *esi;
    //if (DriveCount) {
    esi = CartStatStrings[eax];
    //} else {
    //  esi = szNoIomegaDrives;
    //}
    SetColor(BLACK_COLOR);
    TextOutCentered(115, 9, 241 - 115, 27 - 9, esi);
}

/*******************************************************************************
 * PAINT BAR GRAPH
 *******************************************************************************/
void PaintBarGraph(int Xleft, int Ytop, int XWidth, int YHeight, long BarColor, long BarValue, char *pRightText, bool Active) {
    // fill the entire rectangle with background gray
    SetColor(BACK_COLOR);
    Rectangle(Xleft, Ytop, Xleft + XWidth, Ytop + YHeight);
    if (Active) { // now fleshout the bar ONLY IF we're active
        // if RightText string is non-null, paint it in darker gray
        if(pRightText) {
            SetColor(GRAY_COLOR);
            TextOutCentered(Xleft, Ytop, XWidth, YHeight, pRightText);
        }
        // now paint the active portion
        const long AbsoluteBarWidth = XWidth * BarValue / 100;
        SetColor(BarColor, BLACK_COLOR);
        Rectangle(Xleft, Ytop, Xleft + AbsoluteBarWidth, Ytop + YHeight);
        // now place the floating percentage into the middle (if it fits there)
        if(BarValue) {
            char PercentString[8];
            sprintf(PercentString, szBarChartPercent, BarValue);
            SetColor(WHITE_COLOR);
            TextOutCentered(Xleft, Ytop, AbsoluteBarWidth, YHeight, PercentString);

        }
    }
}

/*******************************************************************************
 * PAINT THE BAR GRAPHS
 *
 * This paints the two or three bar graphs on the test monitor window.
 *******************************************************************************/
void PaintTheBarGraphs(bool Active) {
    long ebx, ecx;
    PaintBarGraph(13, 57, 395, 14, BLUE_COLOR, PercentComplete, NULL, Active);
    if(JazDrive) {
        ebx = Side_0_SparesCount;
        ecx = MAXIMUM_JAZ_SPARES;
        if(ebx > ecx) { // if Spares > MAXIMUM
            ebx = ecx; // clip Spares down to MAX
        }
        PaintBarGraph(13, 95, 395, 30, RED_COLOR, CvrtIntoPrcnt(ebx, ecx), NULL, Active);
    } else {
        ebx = Side_0_SparesCount;
        ecx = MAXIMUM_ZIP_SPARES;
        PaintBarGraph(13,  95, 395, 14, RED_COLOR, CvrtIntoPrcnt(ebx, ecx), "Side 0", Active);
        ebx = Side_1_SparesCount;
        ecx = MAXIMUM_ZIP_SPARES;
        PaintBarGraph(13, 111, 395, 14, RED_COLOR, CvrtIntoPrcnt(ebx, ecx), "Side 1", Active);
    }
}

int CvrtIntoPrcnt(long val, long max) {
    return 100 - val*100/max;
}

/*******************************************************************************
 * PAINT CENTERED STRING
 *
 * Paubts a string int a rectangular region
 *******************************************************************************/
void PaintCenteredString(int Xleft, int Ytop, int XWidth, int YHeight, const char *pText, bool Active) {
    // fill the entire rectangle with background gray
    SetColor(BACK_COLOR);
    Rectangle(Xleft, Ytop, Xleft + XWidth, Ytop + YHeight);
    // now fleshout the bitmap ONLY IF we're active
    if(Active) {
        SetColor(BLACK_COLOR);
        TextOutCentered(Xleft, Ytop, XWidth, YHeight, pText);
    }
}

/*******************************************************************************
 * PAINT CENTERED VALUE
 *
 * Paints a decimal-value into a rectangular region
 *******************************************************************************/
void PaintCenteredValue(int Xleft, int Ytop, int XWidth, int YHeight, long value, bool Active) {
    char szString[40];
    // convert our value into a string
    sprintf(szString, szCenteredDecimal, value);
    // and paint it's value
    PaintCenteredString(Xleft, Ytop, XWidth, YHeight, szString, Active);
}

/*******************************************************************************
 * PAINT TEST STATISTICS
 *
 * This paints the two columns of testing statistics on the test minitor window.
 *******************************************************************************/
void PaintTestStatistics(bool Active) {
    char szString[40];
    // assemble and paint the sector testing range
    long eax = SingleTransferLBA;
    if (eax) {
        sprintf(szString, szCenteredDecimal, eax);
    } else {
        eax = FirstLBASector;
        if (!TestingPhase) {
            eax = 0;
        }
        sprintf(szString, szCenteredDecimal, eax);
        strcat(szString, szSpaceDashSpace);
        eax = FirstLBASector + NumberOfLBAs - 1;
        if (TestingPhase == READY_TO_TEST) {
            eax = LastLBAOnCartridge;
        }
        sprintf(szString + strlen(szString), szCenteredDecimal, eax);
    }
    PaintCenteredString(76, 155, 126, 14, szString, Active);

    // show the LastError
    char *errStr = 0;
    for (int i = 0; errorTypeList[i].str; i++) {
        errStr = errorTypeList[i].str;
        if (errorTypeList[i].code == LastError) break;
    }
    PaintCenteredString(76, 172, 126, 14, errStr, Active);

    // show the elapsed time
    CvrtSecondsToHMSstring(szString, SecondsElapsed);
    PaintCenteredString(76, 189, 126, 14, szString, Active);

    // see if it's time for us to estimate...
    if(SecondsElapsed - ElapsedTimeOfLastEstimate > 15 && PercentComplete > 0) {
        // assemble the remaining time
        ElapsedTimeOfLastEstimate = SecondsElapsed;
        CurrentTotalTimeEstimate = SecondsElapsed * 100 / PercentComplete;
    }

    // given the current estimate time, show the remaining time!
    eax = CurrentTotalTimeEstimate;
    if(eax) {
        eax -= SecondsElapsed;
        if(eax < 0) eax = 0;
        CvrtSecondsToHMSstring(szString, eax);
    } else {
        strcpy(szString, szEstimating);
    }
    PaintCenteredString(76, 206, 126, 14, szString, Active);

    // now show the error accumulations...
    long TotalErrors = SoftErrors + FirmErrors + HardErrors;
    PaintCenteredValue(347, 155, 61, 14, SoftErrors, Active);
    PaintCenteredValue(347, 172, 61, 14, FirmErrors, Active);
    PaintCenteredValue(347, 189, 61, 14, HardErrors, Active);
    PaintCenteredValue(347, 206, 61, 14, TotalErrors, Active);
}

void CvrtSecondsToHMSstring(char *szString, long seconds) {
    const long h = (seconds / 3600);
    const long m = (seconds / 60) % 60;
    const long s = (seconds % 60);
    sprintf(szString, szHoursMinsSecs, h, m, s);
}

/*******************************************************************************
 * TEST MONITOR WND PROC
 *******************************************************************************/
void TestMonitorWndProc() {
    long eax;
    SetColor(BLACK_COLOR);
    TextOut(12, 9, szCartStatus);
    DrawEdge(&CS_Stat, BDR_SUNKENOUTER, BF_RECT);
    PaintCartStatus();

    // draw the sunken rectangles
    DrawEdge(&TP_Perc, BDR_SUNKENOUTER, BF_RECT);
    DrawEdge(&SE_Rect, BDR_SUNKENOUTER, BF_RECT);
    SunkenFields(&TL_Sect, 4, 17);
    SunkenFields(&ES_Read, 4, 17);
    if(JazDrive) { // draw a single LARGE rectangle
        DrawEdge(&SS_Jaz, BDR_SUNKENOUTER, BF_RECT);
    } else { // draw a pair of smaller rectangles
        SunkenFields(&SS_Sid0, 2, 16);
    }
    if((CartridgeStatus == DISK_AT_SPEED) || (CartridgeStatus == DISK_SPUN_DOWN) || (CartridgeStatus >= DISK_LOW_SPARES)) {
        PaintTheBarGraphs(true);
        PaintTestStatistics(true);
        eax = BLACK_COLOR;
    } else {
        PaintTheBarGraphs(false);
        PaintTestStatistics(false);
        eax = GRAY_COLOR;
    }
    PaintTextArray(TestBlackText, eax);
    PaintTextArray(TestGrayText, GRAY_COLOR);
    PaintTestPhase();

    // TODO: paint the little speaker icon
}

/*******************************************************************************
 * UPDATE RUN TIME DISPLAY
 *******************************************************************************/

void UpdateRunTimeDisplay() {
    GetDC(hTestMonitor);
    PaintTestPhase();
    PaintTheBarGraphs(true);
    PaintTestStatistics(true);
    PaintCartStatus();// Added by MLT
    ReleaseDC(hTestMonitor);
}

/*******************************************************************************
 * UPDATE CURRENT SECTOR
 *******************************************************************************/

void UpdateCurrentSector() {
    GetDC(hTestMonitor);
    char szString[40];
    sprintf(szString, szCenteredDecimal, SingleTransferLBA);
    PaintCenteredString(76, 155, 126, 14, szString, true);
    ReleaseDC(hTestMonitor);
}

/*******************************************************************************
 * UPDATE RUN PHASE DISPLAY
 *******************************************************************************/

void UpdateRunPhaseDisplay() {
    GetDC(hTestMonitor);
    PaintTestPhase();
    ReleaseDC(hTestMonitor);
}

/*******************************************************************************
 * ERROR SOUND
 *******************************************************************************/

void ErrorSound() {
    SysBeep(10);
}