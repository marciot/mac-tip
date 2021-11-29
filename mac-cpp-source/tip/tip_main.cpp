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
#include "text_box.h"
#include "tip.h"

enum TipPage {
    kTestingPage,
    kExplainPage,
} page;

static int           gDone;
static bool          allowColor;
static bool          inited = false;
static bool          timerEnabled = false;
static WindowPtr     tipWindow;
static TBHandle      richText;
static const char   *textFileName;

void NewTipWindow();
void DisposeTipWindow();
void DoEvent(EventRecord &event, RgnHandle *cursorRgn);
void DoUpdate(WindowPtr window);
void DoMouseDown(EventRecord &event);
void DoMouseMove(EventRecord &event, RgnHandle *cursorRegion);
void DoDiskEvent(EventRecord &event);
void SetPage(TipPage page);
ControlHandle FindControl(int id);
OSErr GetExplanationFSSpec(const char *name, FSSpec *docSpec);
void OpenExplanationInSimpleText();

#define SET_POINT(x,y) {y,x};

const Point mainWndOrigin = SET_POINT(0, 40);

void run_tip(int id) {
    RgnHandle cursorRgn = NewRgn();

    NewTipWindow();
    EnableWindow(hTestButton, false);
    SetRichEditText(szInstructions);

    gDone = false;
    do {
        EventRecord event;
        if (WaitNextEvent(everyEvent, &event, GetCaretTime(), cursorRgn)) {
            DoEvent(event, &cursorRgn);
            if(!inited && page == kTestingPage) {
                printf("Starting tip\n");
                // Start TIP as soon as the user dismisses the intro screen
                inited = true;
                WinMain(id);
            }
        }
        if(timerEnabled) {
            ApplicationTimerProc();
        }
    } while (!gDone);

    DisposeTipWindow();
    DisposeRgn(cursorRgn);
}

void NewTipWindow() {
    OSErr       error;
    SysEnvRec   theWorld;

    error = SysEnvirons(1, &theWorld);
    allowColor = theWorld.hasColorQD;

    const int wndHeight = 336 - 35;
    const int wndWidth  = 467;

    Rect rect = qd.screenBits.bounds;
    rect.left = 8;
    rect.top  = GetMBarHeight() + rect.left + 16;
    rect.bottom = rect.top + wndHeight;
    rect.right  = rect.left + wndWidth;

    Str255 title;
    StrToPascal(title, szWindowTitle);
    tipWindow = allowColor ?
        NewCWindow(NULL,&rect, title, true, 0, (WindowPtr)-1, true, 0) :
        NewWindow(NULL,&rect,  title, true, 0, (WindowPtr)-1, true, 0);

    GetDC(hMainWnd);

    TextSize(10);

    // Create the controls

    for(int i = 0; tipBtns[i].name; i++) {
        SetRect(&rect,
            tipBtns[i].x,
            tipBtns[i].y - mainWndOrigin.v,
            tipBtns[i].x + tipBtns[i].w,
            tipBtns[i].y + tipBtns[i].h - mainWndOrigin.v
        );
        StrToPascal(title, tipBtns[i].name);
        tipBtns[i].hndl = NewControl(tipWindow, &rect, title, true, 0, 0, 0, 0, tipBtns[i].id);
    }
    ReleaseDC(hMainWnd);

    page = kExplainPage;
    GetDC(hExplainWnd);

    // Create the text edit widget
    SetRect(&rect, 15, 15, 447, 250);
    richText = TBNew(tipWindow, &rect);

    ReleaseDC(hExplainWnd);

    SetPage(kTestingPage);
}

void DisposeTipWindow() {
    TBDispose(richText);
    DisposeWindow(tipWindow);
}

ControlHandle FindControl(int id) {
    for(int i = 0; tipBtns[i].name; i++) {
        if (tipBtns[i].id == id)
            return tipBtns[i].hndl;
    }
    return 0;
}

bool PrepareDC(int which) {
    SetPort(tipWindow);
    switch(which) {
        case hExplainWnd:
            if(page != kExplainPage) return false;
            break;
        case hTestMonitor:
            if(page != kTestingPage) return false;
            SetOrigin(-20, -10);
            break;
        case hMainWnd:
            SetOrigin(mainWndOrigin.h,  mainWndOrigin.v);
            break;
    }
    return true;
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
    SetColor(BACK_COLOR);
    PaintRect(&window->portRect);

    GetDC(hMainWnd);
    WndProc(WM_PAINT, 0);
    ReleaseDC(hMainWnd);

    GetDC(hTestMonitor);
    TestMonitorWndProc();
    ReleaseDC(hTestMonitor);

    GetDC(hExplainWnd);
    SetColor(BLACK_COLOR);
    EraseRect(&(*richText)->frame);
    TBUpdate(richText);
    DrawEdge(&(*richText)->frame, BDR_SUNKENOUTER, BF_RECT);
    ReleaseDC(hExplainWnd);

    UpdateControls(window, window->visRgn);

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
                Point mouse = event.where;
                GrafPtr oldPort;
                GetPort(&oldPort);
                SetPort(thisWindow);
                GlobalToLocal(&mouse);
                const bool hitButton = (!TBMouseDown(richText, mouse, thisWindow)) &&
                                       (FindControl(mouse, thisWindow, &thisControl) == inButton) &&
                                       (TrackControl(thisControl, mouse, 0) == inButton);
                SetPort(oldPort);
                if(hitButton) {
                    int id = GetControlReference(thisControl);
                    switch(id) {
                        case IDB_OKAY:
                            SetPage(kTestingPage);
                            break;
                        case IDB_EXPL:
                            SetPage(kExplainPage);
                            break;
                        case IDB_READ:
                            OpenExplanationInSimpleText();
                            break;
                        default:
                            WndProc(WM_COMMAND, id);
                            break;
                    }
                }
            }
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

void SetPage(TipPage newPage) {
    if(page == newPage) return;
    page = newPage;
    switch(page) {
        case kTestingPage:
            ShowWindow(IDB_TEST, SW_SHOW);
            ShowWindow(IDB_BACK, SW_HIDE);
            ShowWindow(IDB_NEXT, SW_HIDE);
            ShowWindow(IDB_EXPL, SW_SHOW);
            ShowWindow(IDB_OKAY, SW_HIDE);
            ShowWindow(IDB_QUIT, SW_SHOW);
            ShowWindow(IDB_READ, SW_HIDE);
            HideControl((*richText)->scroll);
            break;
        case kExplainPage:
            ShowWindow(IDB_TEST, SW_HIDE);
            ShowWindow(IDB_BACK, SW_HIDE);
            ShowWindow(IDB_NEXT, SW_HIDE);
            ShowWindow(IDB_EXPL, SW_HIDE);
            ShowWindow(IDB_OKAY, SW_SHOW);
            ShowWindow(IDB_QUIT, SW_HIDE);
            ShowWindow(IDB_READ, SW_SHOW);
            ShowControl((*richText)->scroll);
            TBSetScroll(richText, 0);
            break;
    }
    InvalidateRect(hMainWnd);
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
 * GET EXPLANATION FSSPEC
 *
 * Returns the FSSpec for an explanation file
 *******************************************************************************/
OSErr GetExplanationFSSpec(const char *name, FSSpec *docSpec) {
    Str255 docName;
    StrToPascal(docName, name);

    Str255 pathName;
    pstrcpy(pathName, "\p:tip-doc:");
    pstrcat(pathName, docName);

    OSErr err = FSMakeFSSpec(0, 0, pathName, docSpec);
    if(err) {
        ShowAlert(ERR_DLG, "Can't find the \"%s\" file. Make sure it is inside the \"tip-doc\" folder.", name);
    }
    return err;
}

/*******************************************************************************
 * OPEN EXPLANATION IN SIMPLE TEXT
 *
 * Opens the currently selected TIP explanation file using SimpleText
 *
 * This uses code from Thomas Tempelmann's C libraries
 *
 * http://www.tempel.org/macdev/index.html
 *******************************************************************************/
void OpenExplanationInSimpleText() {
    FSSpec docSpec;
    FSSpec appSpec;

    OSErr err = GetExplanationFSSpec(textFileName, &docSpec);
    if(err != noErr) return;

    err = FindApplicationFromDocument(&docSpec, &appSpec);
    if(err) {
        ShowAlert(ERR_DLG, "Can't find an application to open \"%s\". Is \"SimpleText\" installed?", textFileName);
        return;
    }

    err = LaunchApplicationWithDocument(&appSpec, &docSpec, false);
    if(err) {
        ShowAlert(ERR_DLG, "Can't open \"%s\". If \"%#s\" is already running, please close it.", textFileName, appSpec.name);
        return;
    }
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
    short fRefNum = 0;

    // Don't reload a file that is already loaded

    if(textFileName == name) return;
    textFileName = name;

    printf("Loading explanation file \"%s\"\n", name);

    // Get the specification for the explanation file

    FSSpec docSpec;
    OSErr err = GetExplanationFSSpec(textFileName, &docSpec);
    if(err != noErr) return;

    // Load the text from the data fork

    if (name != szRunning && name != szNotRunning) {
        SetPage(kExplainPage);
    }

    TBReadSimpleText(richText, &docSpec);
}

/*******************************************************************************
 * SET COLOR
 *******************************************************************************/

void SetColor(long color) {
    if (allowColor) {
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
    if (allowColor) {
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
    if (allowColor) {
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
    if(edge == BDR_SUNKENOUTER && allowColor) {
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
    ControlHandle hCntl = FindControl(id);
    if(hCntl) {
        GetDC(hMainWnd);
        SetCTitle(hCntl, pStr);
        ReleaseDC(hMainWnd);
    }
}

/*******************************************************************************
 * ENABLE WINDOW
 *******************************************************************************/
void EnableWindow(int id, bool enabled) {
    ControlHandle hCntl = FindControl(id);
    if(hCntl) {
        GetDC(hMainWnd);
        HiliteControl(hCntl, enabled ? 0 : 255);
        ReleaseDC(hMainWnd);
    }
}

/*******************************************************************************
 * SHOW WINDOW
 *******************************************************************************/
void ShowWindow(int id, int state) {
    ControlHandle hCntl = FindControl(id);
    if(hCntl) {
        GetDC(hMainWnd);
        if(state == SW_SHOW)
            ShowControl(hCntl);
        else
            HideControl(hCntl);
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
