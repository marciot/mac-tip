#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <ctype.h>
#include <SIOUX.h>

#include <Windows.h>
#include <Quickdraw.h>

#include "pstring.h"
#include "LaunchLib.h"
#include "mac_vol.h"
#include "tip.h"

WindowPtr tipWindow;

static int gDone;
static bool AllowColor;
static bool timerEnabled = true;
static ControlHandle testBtn = 0;
static ControlHandle exitBtn = 0;

void NewTipWindow();
void DestroyTipWindow();
void DoEvent(EventRecord &event, RgnHandle *cursorRgn);
void DoUpdate(WindowPtr window);
void DoMouseDown(EventRecord &event);
void DoMouseMove(EventRecord &event, RgnHandle *cursorRegion);
void DoDiskEvent(EventRecord &event);

void run_tip(int id) {
    WinMain(id);
    RgnHandle cursorRgn = NewRgn();

    SetRichEditText(szInstructions);
    NewTipWindow();
    EnableWindow(hTestButton, false);

    gDone = false;
    do {
        EventRecord event;
        if (WaitNextEvent(everyEvent, &event, GetCaretTime(), cursorRgn)) {
            DoEvent(event, &cursorRgn);
        }
        if(timerEnabled) {
            ApplicationTimerProc();
        }
    } while (!gDone);

    DestroyTipWindow();
    DisposeRgn(cursorRgn);
}

void NewTipWindow() {
    OSErr       error;
    SysEnvRec   theWorld;

    error = SysEnvirons(1, &theWorld);
    AllowColor = theWorld.hasColorQD;

    Rect rect = qd.screenBits.bounds;
    rect.left = 8;
    rect.top  = GetMBarHeight() + rect.left + 16;
    rect.bottom = rect.top + 336 - 35;
    rect.right = rect.left + 467;

    Str255 title;
    StrToPascal(title, szWindowTitle);
    tipWindow = AllowColor ?
        NewCWindow(NULL,&rect, title, true, 0, (WindowPtr)-1, true, 0) :
        NewWindow(NULL,&rect,  title, true, 0, (WindowPtr)-1, true, 0);

    GetDC(hMainWnd);

    if (AllowColor) {
        SetColor(BACK_COLOR);
        RGBColor bgColor;
        GetForeColor(&bgColor);
        RGBBackColor(&bgColor);
    }
    TextSize(10);

    for(int i = 0; tipBtns[i].name; i++) {
        if (!tipBtns[i].visible) continue;
        SetRect(&rect,
            tipBtns[i].x,
            tipBtns[i].y,
            tipBtns[i].x + tipBtns[i].w,
            tipBtns[i].y + tipBtns[i].h
        );

        StrToPascal(title, tipBtns[i].name);
        ControlHandle h = NewControl(tipWindow, &rect, title, true, 0, 0, 0, 0, tipBtns[i].id);
        if(tipBtns[i].id == IDB_TEST) {
            testBtn = h;
        }
        if(tipBtns[i].id == IDB_QUIT) {
            exitBtn = h;
        }
    }

    ReleaseDC(hMainWnd);
}

void DestroyTipWindow() {
    DisposeWindow(tipWindow);
}

void DoEvent(EventRecord &event, RgnHandle *cursorRgn) {
    if(!SIOUXHandleOneEvent(&event)) {
        // If SIOUX didn't handle the event, then handle it ourselves
        switch(event.what) {
            case mouseDown: DoMouseDown(event); break;
            case updateEvt: DoUpdate((WindowPtr)event.message); break;
            case diskEvt:   DoDiskEvent(event); break;
            case osEvt:     DoMouseMove(event, cursorRgn); break;
        }
    }

}

void DoUpdate(WindowPtr window) {
    BeginUpdate(window);
    SetPort(window);
    EraseRect(&window->portRect);

    GetDC(hMainWnd);
    WndProc(WM_PAINT, 0);
    DrawControls(window);
    ReleaseDC(hMainWnd);

    GetDC(hTestMonitor);
    TestMonitorWndProc();
    ReleaseDC(hTestMonitor);

    EndUpdate(window);
}

void DoMouseDown(EventRecord &event) {
    WindowPtr thisWindow;
    ControlHandle thisControl;
    int part = FindWindow(event.where, &thisWindow);
    switch(part) {
        case inSysWindow:
            SystemClick(&event, thisWindow);
            break;
        case inContent:
            if(thisWindow != FrontWindow()) {
                SelectWindow(thisWindow);
            }
            else if(thisWindow == tipWindow) {
                long id = 0;
                Point mouse = event.where;
                GetDC(hMainWnd);
                GlobalToLocal(&mouse);
                if(FindControl(mouse, thisWindow, &thisControl) == inButton) {
                    if(TrackControl(thisControl, mouse, 0) == inButton) {
                        id = GetControlReference(thisControl);
                    }
                }
                ReleaseDC(hMainWnd);
                if(id) {
                    WndProc(WM_COMMAND, id);
                }
            }
            break;
        case inDrag:
            DragWindow(thisWindow, event.where, &(*GetGrayRgn())->rgnBBox);
            break;
        case inGrow:
            //DoGrowWindow(thisWindow, event);
            break;
        case inGoAway:
            if (TrackGoAway(thisWindow, event.where)) {
                gDone = true;
            }
            break;
    }
}

void DoMouseMove(EventRecord &event, RgnHandle *cursorRgn) {
    WindowPtr thisWindow;
    int part = FindWindow(event.where, &thisWindow);
    if (thisWindow == tipWindow) {
        InitCursor();
        // Set the cursorRegion to everything inside our window
        if(cursorRgn) {
            CopyRgn(((WindowPeek)tipWindow)->contRgn, *cursorRgn);
        }
    }
}

void DoDiskEvent(EventRecord &event) {
    OSErr mountErr = HiWord(event.message);
    short driveNum = LoWord(event.message);

    if (mountErr == noErr) {
        // Get the volume name of recently mounted drive
        Str255 volumes;
        mac_get_drive_volumes(driveNum, volumes);

        // Ask the user whether they want to unmount the disk
        ParamText(volumes, "\p", "\p", "\p");
        if (CautionAlert(128, NULL) == 1) {
            // The user wishes to unmount the disk
            OSErr err = mac_unmount_drive(driveNum);
            if(err != noErr) {
                if(err == fBsyErr) {
                    ShowAlert(ERR_DLG, "Failed to unmount. One or more files are open.");
                } else {
                    ShowAlert(ERR_DLG, "Failed to unmount. Error Code: %d", err);
                }
            }
        }
    }
}

void StrToPascal(Str255 pStr, const char *str) {
    size_t len = strlen(str);
    pStr[0] = (len > 255) ? 255 : len;
    strncpy((char*)pStr + 1, str, 255);
}

/*******************************************************************************
 * SHOW ALERT BOX
 *******************************************************************************/

int ShowAlert(AlertTypes type, const char* format, ...) {
    Str255 pStr;
    va_list argptr;
    va_start(argptr, format);
    vsprintf((char*)pStr + 1, format, argptr);
    va_end(argptr);
    pStr[0] = strlen((char*)pStr + 1);
    ParamText(pStr, "\p", "\p", "\p");
    switch(type) {
        case ERR_DLG: return StopAlert(129, NULL);
        case YN_DLG: return NoteAlert(130, NULL);
    }
    return 0;
}

/*******************************************************************************
 * SET RICH EDIT TEXT
 *
 * This routine will open one of the TIP explanation files using SimpleText
 *
 * This uses code from Thomas Tempelmann's C libraries
 *
 * http://www.tempel.org/macdev/index.html
 *******************************************************************************/

void SetRichEditText(const char *name) {
    static const char *lastName = 0;
    if(name == lastName) return;
    lastName = name;

    if(ShowAlert(YN_DLG, "Would you like to read the document \"%s\" now?", name) == 2) {
        return;
    }
    Str255 docName;
    StrToPascal(docName, name);

    Str255 pathName;
    pstrcpy(pathName, "\p:tip-doc:");
    pstrcat(pathName, docName);

    FSSpec docSpec;
    FSSpec appSpec;
    OSErr err = FSMakeFSSpec(0, 0, pathName, &docSpec);
    if(err) {
        ShowAlert(ERR_DLG, "Can't find the \"%s\" file. Make sure it is inside the \"tip-doc\" folder.", name);
        return;
    }

    err = FindApplicationFromDocument(&docSpec, &appSpec);
    if(err) {
        ShowAlert(ERR_DLG, "Can't find an application to open \"%s\". Is \"SimpleText\" installed?", name);
        return;
    }
    err = LaunchApplicationWithDocument(&appSpec, &docSpec, false);
    if(err) {
        ShowAlert(ERR_DLG, "Can't open \"%s\". If \"%#s\" is already running, please close it.", name, appSpec.name);
        return;
    }
}

/*******************************************************************************
 * SET COLOR
 *******************************************************************************/

void SetColor(long color) {
    if (AllowColor) {
        if(color == BACK_COLOR) color = LTGRAY_COLOR;
        // Use colors when available
        RGBColor rgbColor;
        rgbColor.red   = (color & 0xFF0000) >> 8;
        rgbColor.green = (color & 0x00FF00) >> 0;
        rgbColor.blue  = (color & 0x0000FF) << 8;
        RGBForeColor(&rgbColor);
     } else {
        // Use patterns for B&W Macs
        TextMode(srcCopy);
        switch(color) {
            case BACK_COLOR:
            case WHITE_COLOR:
                PenPat(&qd.white);
                TextMode(srcBic);
                break;
            case BLACK_COLOR:
            case GRAY_COLOR:
                PenPat(&qd.black);
                break;
            case RED_COLOR:
            case BLUE_COLOR:
                PenPat(&qd.ltGray);
                break;
            case LTGRAY_COLOR:
            case GREEN_COLOR:
                PenPat(&qd.gray);
                break;
        }
     }
}

void SetColor(long color, long monoColor) {
    if (AllowColor) {
        SetColor(color);
    } else {
        SetColor(monoColor);
    }
}

/*******************************************************************************
 * DRAW LED
 *******************************************************************************/

void DrawLed(int x, int y, long color) {
    Rect ledRect;
    SetRect(&ledRect, x, y, x + 12, y + 12);
    // Draw the LED
    SetColor(color);
    PaintOval(&ledRect);
    if (AllowColor) {
        // Draw a recessed outline
        SetColor(BLACK_COLOR);
        FrameOval(&ledRect);
        SetColor(WHITE_COLOR);
        FrameArc(&ledRect,45,180);
    } else {
        // Draw a non-recessed outline
        SetColor(BLACK_COLOR);
        FrameOval(&ledRect);
        SetColor(WHITE_COLOR);
    }
    // Draw the reflection
    if(color != BACK_COLOR) {
        ledRect.left += 3;
        ledRect.top += 3;
        ledRect.right -= 7;
        ledRect.bottom -= 7;
        OffsetRect(&ledRect,0,1);
        PaintRect(&ledRect);
        OffsetRect(&ledRect,1,-1);
        PaintRect(&ledRect);
    }
}

/*******************************************************************************
 * DRAW EDGE
 *******************************************************************************/

void DrawEdge(Rect *qrc, int edge, int grfFlags) {
    if(edge == BDR_SUNKENOUTER && AllowColor) {
        // Draw a sunken rectangle
        SetColor(GRAY_COLOR);
        MoveTo(qrc->left,qrc->bottom-1);
        LineTo(qrc->left,qrc->top);
        LineTo(qrc->right-1,qrc->top);
        SetColor(WHITE_COLOR);
        MoveTo(qrc->left,qrc->bottom-1);
        LineTo(qrc->right-1,qrc->bottom-1);
        LineTo(qrc->right-1,qrc->top);
    } else {
        // Draw a non-recessed rectangle
        SetColor(LTGRAY_COLOR);
        FrameRect(qrc);
    }
}

/*******************************************************************************
 * RECTANGLE
 *******************************************************************************/

void Rectangle(int left, int top, int right, int bottom) {
    Rect rect;
    SetRect(&rect, left, top, right, bottom);
    PaintRect(&rect);
}

/*******************************************************************************
 * TEXT OUT
 *******************************************************************************/

void TextOut(int x, int y, Str255 pStr) {
    FontInfo info;
    GetFontInfo(&info);
    MoveTo(x, y + info.ascent + info.descent);
    DrawString(pStr);
}

void TextOut(int x, int y, const char *str) {
    Str255 pStr;
    StrToPascal(pStr, str);
    TextOut(x, y, pStr);
}

void TextOutCentered(int x, int y, int w, int h, const char *str) {
    // convert to Pascal string
    Str255 pStr;
    StrToPascal(pStr, str);
    // now place the floating string in the middle
    const int width = TextWidth(pStr, 0, strlen(str));
    if(width < w) {
        TextOut(x + w/2 - width/2, y, pStr);
    }
}

/*******************************************************************************
 * GET TEXT EXTENT
 *******************************************************************************/
long GetTextExtent(const char *str, unsigned long len) {
    // convert to Pascal string
    Str255 pStr;
    StrToPascal(pStr, str);
    // now place the floating string in the middle
    return TextWidth(pStr, 0, len);
}

/*******************************************************************************
 * GET SYSTEM TIME
 *******************************************************************************/
unsigned long GetSystemTime() {
    unsigned long time;
    GetDateTime (&time);
    return time;
}

/*******************************************************************************
 * SET WINDOW TEXT
 *******************************************************************************/
void SetWindowText(int id, const char *str) {
    Str255 pStr;
    StrToPascal(pStr, str);
    if(testBtn && id == hTestButton) {
        GetDC(hMainWnd);
        SetCTitle(testBtn, pStr);
        ReleaseDC(hMainWnd);
    }
}

/*******************************************************************************
 * ENABLE WINDOW
 *******************************************************************************/
void EnableWindow(int id, bool enabled) {
    ControlHandle hCntl = 0;
    if(id == hTestButton) hCntl = testBtn;
    if(id == hExitButton) hCntl = exitBtn;
    if(hCntl) {
        GetDC(hMainWnd);
        HiliteControl(hCntl, enabled ? 0 : 255);
        ReleaseDC(hMainWnd);
    }
}

/*******************************************************************************
 * INVALIDATE RECT
 *******************************************************************************/
void InvalidateRect(int id) {
    GetDC(id);
    InvalRect(&qd.thePort->portRect);
    ReleaseDC(id);
}

/*******************************************************************************
 * POST QUIT MESSAGE
 *******************************************************************************/
void PostQuitMessage() {
    gDone = true;
}

/*******************************************************************************
 * START APPLICATION TIMER
 *******************************************************************************/
void StartApplicationTimer() {
    timerEnabled = true;
}

/*******************************************************************************
 * STOP APPLICATION TIMER
 *******************************************************************************/
void StopApplicationTimer() {
    timerEnabled = false;
}

/*******************************************************************************
 * PROCESS PENDING MESSAGES
 *******************************************************************************/
void ProcessPendingMessages() {
    EventRecord event;
    if (GetNextEvent(everyEvent, &event)) {
        DoEvent(event,NULL);
    }
    SystemTask();
}
