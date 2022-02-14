/*******************************************************************************
 *    TIP.CPP                  TROUBLE IN PARADISE                  05/22/98   *
 ******************************************************************************/

#include <limits.h>
#include <string.h>
#include <stdio.h>

#include "tip.h"

#define TITLE_TOP 11
#define BODY_TOP 54
#define BODY_LEFT 156
#define BODY_RIGHT 445
#define BODY_BOTTOM 280

#define BODY_WIDTH (BODY_RIGHT - BODY_LEFT)
#define BODY_HEIGHT (BODY_BOTTOM - BODY_TOP)

#define LOGO_1_LEFT 157
#define LOGO_1_TOP 57

/*******************************************************************************
 * WinMain
 *
 * Startup the Windows program.
 *******************************************************************************/
void WinMain(uint8_t *DrivesSkipped) {
    CurrentDevice = 0;
    // test for an Iomega device
    EnumerateIomegaDevices(DrivesSkipped);
    if (DriveCount == 1) {
        // we have only one, so select it for the user
        CurrentDevice = DriveArray[0].scsi_id;
    }
    if (DriveCount == 0) {
        // disable testing button when no drives present (added by mlt)
        EnableWindow(hTestButton, false);
        SetRichEditText(szASPITrouble);
    }
    // now startup the timer for real-time features
    StartApplicationTimer();
}

/*******************************************************************************
 * PAINT 3D HEADLINE
 *******************************************************************************/
void Paint3DHeadline(const char *pszText, int Xleft, int Ytop) {
    TextSize(24);
    TextFace(bold);
    TextFont(helvetica);
    SetColor(WHITE_COLOR);
    TextOut(Xleft, Ytop, pszText);
    SetColor(BLACK_COLOR);
    TextOut(Xleft+1, Ytop+1, pszText);
    TextFont(applFont);
    TextFace(0);
    TextSize(10);
}

/*******************************************************************************
 * WndProc
 *
 * This is the system's main window procedure
 *******************************************************************************/
void WndProc(long iMessage, uint16_t wParam) {
    // -------------------------------------------------------------------------
    // WM_PAINT
    // -------------------------------------------------------------------------
    if (iMessage == WM_PAINT) {
        // Draw the Lower Horz Button Divider

        GetDC(hMainWnd);
        SetColor(GRAY_COLOR);
        MoveTo(15, 289);
        LineTo(446, 289);
        SetColor(WHITE_COLOR);
        LineTo(446, 290);
        LineTo(14, 290);
        ReleaseDC(hMainWnd);

        // Draw the Gibson 'G' Logo
        if(CurrentPage == INTRO_PAGE) {
            GetDC(hIntroWnd);
            SetColor(GRAY_COLOR,   GRAY_COLOR);
            MoveTo(LOGO_1_LEFT+1,  LOGO_1_TOP+29);
            LineTo(LOGO_1_LEFT+14, LOGO_1_TOP+29);
            LineTo(LOGO_1_LEFT+14, LOGO_1_TOP+0);
            SetColor(WHITE_COLOR,  LTGRAY_COLOR);
            LineTo(LOGO_1_LEFT+12, LOGO_1_TOP+0);
            LineTo(LOGO_1_LEFT+0,  LOGO_1_TOP+12);
            LineTo(LOGO_1_LEFT+0,  LOGO_1_TOP+30);

            SetColor(GRAY_COLOR,   GRAY_COLOR);
            MoveTo(LOGO_1_LEFT+18, LOGO_1_TOP+14);
            LineTo(LOGO_1_LEFT+46, LOGO_1_TOP+14);
            LineTo(LOGO_1_LEFT+46, LOGO_1_TOP+12);
            LineTo(LOGO_1_LEFT+34, LOGO_1_TOP+0);
            SetColor(WHITE_COLOR,  LTGRAY_COLOR);
            LineTo(LOGO_1_LEFT+17, LOGO_1_TOP+0);
            LineTo(LOGO_1_LEFT+17, LOGO_1_TOP+15);

            SetColor(GRAY_COLOR,   GRAY_COLOR);
            MoveTo(LOGO_1_LEFT+33, LOGO_1_TOP+46);
            LineTo(LOGO_1_LEFT+46, LOGO_1_TOP+46);
            LineTo(LOGO_1_LEFT+46, LOGO_1_TOP+29);
            LineTo(LOGO_1_LEFT+34, LOGO_1_TOP+17);
            SetColor(WHITE_COLOR,  LTGRAY_COLOR);
            LineTo(LOGO_1_LEFT+32, LOGO_1_TOP+17);
            LineTo(LOGO_1_LEFT+32, LOGO_1_TOP+47);

            SetColor(GRAY_COLOR,   GRAY_COLOR);
            MoveTo(LOGO_1_LEFT+1,  LOGO_1_TOP+35);
            LineTo(LOGO_1_LEFT+12, LOGO_1_TOP+46);
            LineTo(LOGO_1_LEFT+29, LOGO_1_TOP+46);
            LineTo(LOGO_1_LEFT+29, LOGO_1_TOP+32);
            SetColor(WHITE_COLOR,  LTGRAY_COLOR);
            LineTo(LOGO_1_LEFT+0,  LOGO_1_TOP+32);
            LineTo(LOGO_1_LEFT+0,  LOGO_1_TOP+35);

            // paint the 3D program title
            Paint3DHeadline(szIntroTitle, BODY_LEFT, TITLE_TOP);

            // now the rest of the stuff ...
            SetColor(BLACK_COLOR);
            #define WH_RECT(L,T,W,H) L, T, L + W, T + H

            Rect rect;
            SetRect(&rect, WH_RECT(221, BODY_TOP, 230, 60));
            TETextBox(szIntroSubTitle, strlen(szIntroSubTitle), &rect, teFlushDefault);

            SetRect(&rect, WH_RECT(BODY_LEFT, BODY_TOP+64, BODY_WIDTH, 115));
            TETextBox(szIntroText, strlen(szIntroText), &rect, teFlushDefault);

            // show the current logo bitmap
            SplashTheBitmap();

            ReleaseDC(hIntroWnd);
        }

        // Paint the Copyright Notice
        GetDC(hMainWnd);
        SetColor(GRAY_COLOR);
        TextOut(15, 298, szCopyright_1);
        TextOut(15, 311, szCopyright_2);
        ReleaseDC(hMainWnd);
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
                        printf("\nTesting the disk\n");
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
                        EjectIomegaCartridge(CurrentDevice);
                        break;
                    case DISK_LOW_SPARES:
                        CartridgeStatus = DISK_AT_SPEED;
                        SetRichEditText(szNotRunning);
                        SetWindowText(hTestButton, szPressToStart);
                        PrepareToBeginTesting();
                        InvalidateRect(hTestMonitor);
                        break;
                }
                break;
            default:
                break;
        }
    }
}

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
    if (DriveCount) {
        esi = CartStatStrings[eax];
    } else {
        esi = szNoIomegaDrives;
    }
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
        if(BarValue > 0 && BarValue <= 100) {
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
char *FindErrorString(long error) {
    char *errStr = 0;
    for (int i = 0; errorTypeList[i].str; i++) {
        errStr = errorTypeList[i].str;
        if (errorTypeList[i].code == error) break;
    }
    return errStr;
}

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
    PaintCenteredString(76, 172, 126, 14, FindErrorString(LastError), Active);

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

    // paint the little speaker icon
    Rect theRect;
    SetRect(&theRect, 232, 191, 232+16, 191+16);
    PlotIconID(&theRect, atTopLeft, 0, 129);
}

/*******************************************************************************
 * APPLICATION TIMER PROC
 *******************************************************************************/
void ApplicationTimerProc() {
    // only if we have at least ONE Iomega drive
    if(DriveCount) {
        HandleDriveChanging();
    }
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
 * PREVENT PROGRAM EXIT
 *******************************************************************************/
void PreventProgramExit() {
    EnableWindow(hExitButton, false);
}

/*******************************************************************************
 * ALLOW PROGRAM EXIT
 *******************************************************************************/
void AllowProgramExit() {
    EnableWindow(hExitButton, true);
}

/*******************************************************************************
 * ERROR SOUND
 *******************************************************************************/

void ErrorSound() {
    if(SendMessage(hSoundCheckbox, BM_GETCHECK) == BST_CHECKED) {
        SysBeep(10);
    }
}
