#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <ctype.h>
#include <SIOUX.h>

#include <Menus.h>
#include <Windows.h>
#include <Quickdraw.h>

#include "TrapAvail.h"
#include "pstring.h"
#include "LaunchLib.h"
#include "mac_vol.h"
#include "text_box.h"
#include "tip.h"
#include "command_line.h"

static int           gDone;
static bool          allowColor;
static bool          inited = false;
static bool          timerEnabled = false;
static WindowPtr     tipWindow;
static MenuHandle    tipMenu;
static PicHandle     tipIntroPic;
static TBHandle      richText;
static const char   *textFileName;

void NewTipWindow();
void DisposeTipWindow();
void AddTipMenus();
void RunCommandLine();
void DoEvent(EventRecord &event, RgnHandle *cursorRgn);
void DoMenuEventPostSIOUX(EventRecord &event);
bool DoMenuSelection(long choice);
void DoUpdate(WindowPtr window);
void DoMouseDown(EventRecord &event);
void DoMouseMove(EventRecord &event, RgnHandle *cursorRegion);
void DoDiskEvent(EventRecord &event);
void SetPage(TipPage page);
ControlHandle FindCntlHandle(int id);
OSErr GetExplanationFSSpec(const char *name, FSSpec *docSpec);
void OpenExplanationInSimpleText();

#define SET_POINT(x,y) {y,x};

const Point mainWndOrigin = SET_POINT(0, 40);

void run_tip() {
    AddTipMenus();

    RgnHandle cursorRgn = NewRgn();

    NewTipWindow();
    EnableWindow(hTestButton, false);

    gDone = false;
    do {
        EventRecord event;
        if (WaitNextEvent(everyEvent, &event, GetCaretTime(), cursorRgn)) {
            DoEvent(event, &cursorRgn);
            if(!inited && CurrentPage == PERFORM_TEST_PAGE) {
                // Start TIP as soon as the user dismisses the intro screen
                inited = true;
                uint8_t drivesSkipped;
                WinMain(&drivesSkipped);
                if(drivesSkipped) {
                    const char *s = drivesSkipped > 1 ? "s" : "";
                    if(ShowAlert(YN_DLG,
                        "Found media in %d drive%s. If you wish to test that "
                        "media, you must quit to the Finder and eject it "
                        "prior to running TIP. Would you like to do so?",
                            drivesSkipped, s
                        ) == 1) {
                        gDone = true;
                    }
                }
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

    SetPort(tipWindow);
    SetBackColor(BACK_COLOR);
    EraseRect(&tipWindow->portRect);

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
        tipBtns[i].hndl = NewControl(tipWindow, &rect, title, false, 0, 0, 1, tipBtns[i].type, tipBtns[i].id);
    }

    SetControlValue(FindCntlHandle(IDB_BEEP),1); // Check the sound control

    CurrentPage = EXPLAIN_RESULTS;
    GetDC(hExplainWnd);

    // Create the text edit widget
    SetRect(&rect, 15, 15, 447, 250);
    richText = TBNew(tipWindow, &rect);

    ReleaseDC(hExplainWnd);

    // Load the About box picture
    tipIntroPic = GetPicture(128);

    SetPage(INTRO_PAGE);
}

void AddTipMenus() {
    if(!TrapAvailable(0xAA66)) {
        // If MenuChoice is available, we can let SIOUX handle the menus,
        // otherwise we have to handle it ourselves
        SIOUXSettings.setupmenus = FALSE;
    }

    // Add our menu
    tipMenu = GetMenu(128);
    InsertMenu(tipMenu, 0);
    DrawMenuBar();
}

void RunCommandLine() {
    HideWindow(tipWindow);
    DisableItem(tipMenu, 0);
    DrawMenuBar();
    command_line_event_loop();
    EnableItem(tipMenu, 0);
    DrawMenuBar();
    ShowWindow(tipWindow);
    SelectWindow(tipWindow);
    InitCursor();
}

void DisposeTipWindow() {
    TBDispose(richText);
    DisposeWindow(tipWindow);
}

ControlHandle FindCntlHandle(int id) {
    for(int i = 0; tipBtns[i].name; i++) {
        if (tipBtns[i].id == id)
            return tipBtns[i].hndl;
    }
    return 0;
}

bool PrepareDC(int which) {
    SetPort(tipWindow);
    switch(which) {
        case hIntroWnd:
            if(CurrentPage != INTRO_PAGE) return false;
            break;
        case hExplainWnd:
            if(CurrentPage != EXPLAIN_RESULTS) return false;
            break;
        case hTestMonitor:
            if(CurrentPage != PERFORM_TEST_PAGE) return false;
            SetOrigin(-20, -10);
            break;
        case hMainWnd:
            SetOrigin(mainWndOrigin.h,  mainWndOrigin.v);
            break;
        case hDefault:
            SetOrigin(0, 0);
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
    } else { // Trap unhandled SIOUX menu events
        DoMenuEventPostSIOUX(event);
    }
}

void DoMenuEventPostSIOUX(EventRecord &event) {
    if(!SIOUXSettings.setupmenus) return;

    /* If MenuChoice is available, it is best to let SIOUX handle the menu
     * event so Copy and Paste will work. We can check after the fact
     * to see whether the user selected one of our menus using MenuChoice.
     * However, if that trap is not available, we must handle the menu
     * ourselves and certain menu items will not work
     */

    WindowPtr thisWindow;
    if((event.what == mouseDown) && (FindWindow(event.where, &thisWindow) == inMenuBar)) {
        DoMenuSelection(MenuChoice());
    }
}

bool DoMenuSelection(long choice) {
    bool handled = false;
    int menuId = HiWord(choice);
    int itemId = LoWord(choice);
    //printf("Menu choice: %d,  %d\n", menuId, itemId);
    switch(menuId)  {
        case 32000: // Apple menu             SysBeep(10);
            break;
        case 32001: // File menu
            if (itemId == 9) { // Quit
                WndProc(WM_COMMAND, IDB_QUIT);
                handled = true;
            }
            break;
        case 32002: // Edit menu
            break;
        case 128:   // TIP menu
            switch(itemId) {
                case 1: // Run Command Line...
                    HiliteMenu(0);
                    RunCommandLine();
                    handled = true;
                    break;
            }
            break;
    }
    HiliteMenu(0);
    return handled;
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

    SetColor(BLACK_COLOR);
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
                int part;
                const bool hitCntl = (!TBMouseDown(richText, mouse, thisWindow)) &&
                                       (FindControl(mouse, thisWindow, &thisControl)) &&
                                       (part = TrackControl(thisControl, mouse, 0));
                SetPort(oldPort);
                if(hitCntl && (part == inButton) || (part == inCheckBox)) {
                    int id = GetControlReference(thisControl);
                    switch(id) {
                        case IDB_OKAY:
                            SetPage(PERFORM_TEST_PAGE);
                            break;
                        case IDB_EXPL:
                            SetPage(EXPLAIN_RESULTS);
                            break;
                        case IDB_NEXT:
                            SetRichEditText(szInstructions);
                            break;
                        case IDB_READ:
                            OpenExplanationInSimpleText();
                            break;
                        case IDB_BEEP:
                            SetPort(thisWindow);
                            SetControlValue(thisControl, 1 - GetControlValue(thisControl));
                            SetPort(oldPort);
                            printf("Value: %d\n", GetControlValue(thisControl));
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
        case inMenuBar:
            if(!DoMenuSelection(MenuSelect(event.where))) {
                SysBeep(10);
            }
            break;
        case inDrag:
            DragWindow(thisWindow, event.where, &(*GetGrayRgn())->rgnBBox);
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
    if(CurrentPage == newPage) return;
    CurrentPage = newPage;
    switch(CurrentPage) {
        case INTRO_PAGE:
            ShowWindow(IDB_TEST, SW_HIDE);
            ShowWindow(IDB_BACK, SW_HIDE);
            ShowWindow(IDB_EXPL, SW_HIDE);
            ShowWindow(IDB_BEEP, SW_HIDE);
            ShowWindow(IDB_OKAY, SW_HIDE);
            ShowWindow(IDB_READ, SW_HIDE);
            ShowWindow((*richText)->scroll, SW_HIDE);
            ShowWindow(IDB_QUIT, SW_SHOW);
            ShowWindow(IDB_NEXT, SW_SHOW);
            break;
        case PERFORM_TEST_PAGE:
            ShowWindow((*richText)->scroll, SW_HIDE);
            ShowWindow(IDB_BACK, SW_HIDE);
            ShowWindow(IDB_NEXT, SW_HIDE);
            ShowWindow(IDB_OKAY, SW_HIDE);
            ShowWindow(IDB_READ, SW_HIDE);
            ShowWindow(IDB_TEST, SW_SHOW);
            ShowWindow(IDB_EXPL, SW_SHOW);
            ShowWindow(IDB_QUIT, SW_SHOW);
            ShowWindow(IDB_BEEP, SW_SHOW);
            break;
        case EXPLAIN_RESULTS:
            ShowWindow(IDB_TEST, SW_HIDE);
            ShowWindow(IDB_BACK, SW_HIDE);
            ShowWindow(IDB_NEXT, SW_HIDE);
            ShowWindow(IDB_EXPL, SW_HIDE);
            ShowWindow(IDB_QUIT, SW_HIDE);
            ShowWindow(IDB_BEEP, SW_HIDE);
            ShowWindow(IDB_OKAY, SW_SHOW);
            ShowWindow(IDB_READ, SW_SHOW);
            ShowWindow((*richText)->scroll, SW_SHOW);
            TBSetScroll(richText, 0);
            break;
    }
    InvalidateRect(hDefault);
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
    // Don't reload a file that is already loaded

    if(textFileName == name) return;
    textFileName = name;

    // Get the specification for the explanation file

    FSSpec docSpec;
    OSErr err = GetExplanationFSSpec(textFileName, &docSpec);
    if(err != noErr) return;

    // Load the text from the file

    TBReadSimpleText(richText, &docSpec);

    if (name != szRunning && name != szNotRunning) {
        SetPage(EXPLAIN_RESULTS);
    } else {
        InvalidateRect(hDefault);
    }
}

/*******************************************************************************
 * SET COLOR
 *******************************************************************************/

void SetRGBColor(long color, RGBColor *rgbColor) {
    if(color == BACK_COLOR) color = LTGRAY_COLOR;
    // Use colors when available
    rgbColor->red   = (color & 0xFF0000) >> 8;
    rgbColor->green = (color & 0x00FF00) >> 0;
    rgbColor->blue  = (color & 0x0000FF) << 8;
}

void SetColor(long color) {
    if (allowColor) {
        RGBColor rgbColor;
        SetRGBColor(color, &rgbColor);
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

void SetBackColor(long color) {
    if (allowColor) {
        RGBColor rgbColor;
        SetRGBColor(color, &rgbColor);
        RGBBackColor(&rgbColor);
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
    ControlHandle hCntl = FindCntlHandle(id);
    if(hCntl) {
        GetDC(hDefault);
        SetCTitle(hCntl, pStr);
        ReleaseDC(hDefault);
    }
}

/*******************************************************************************
 * ENABLE WINDOW
 *******************************************************************************/
void EnableWindow(int id, bool enabled) {
    ControlHandle hCntl = FindCntlHandle(id);
    if(hCntl) {
        GetDC(hDefault);
        HiliteControl(hCntl, enabled ? 0 : 255);
        ReleaseDC(hDefault);
    }
}

/*******************************************************************************
 * SHOW WINDOW
 *******************************************************************************/
void ShowWindow(ControlHandle hCntl, int state) {
    // Show/hide a control by invalidating, rather than drawing it
    (*hCntl)->contrlVis = (state == SW_SHOW) ? 255 : 0;
    InvalRect(&(*hCntl)->contrlRect);
}

void ShowWindow(int id, int state) {
    ControlHandle hCntl = FindCntlHandle(id);
    if(hCntl) {
        GetDC(hDefault);
        ShowWindow(hCntl, state);
        ReleaseDC(hDefault);
    }
}

/*******************************************************************************
 * SEND MESSAGE
 *******************************************************************************/
long SendMessage(int id, int msg) {
    if(msg == BM_GETCHECK) {
        ControlHandle hCntl = FindCntlHandle(id);
        if(hCntl) {
            return GetControlValue(hCntl) ? BST_CHECKED: 0;
        }
    }
    return 0;
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

/*******************************************************************************
 * SPLASH THE BITMAP
 *******************************************************************************/
void SplashTheBitmap() {
    GetDC(hIntroWnd);
    if(tipIntroPic) {
        Rect rect;
        SetRect(&rect, 16, 18, 16+120, 18/*+258*/ +220);
        DrawPicture(tipIntroPic, &rect);
    }
    ReleaseDC(hIntroWnd);
}