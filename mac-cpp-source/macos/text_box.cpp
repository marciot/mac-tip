/************************************************************

    text_box.cpp

       AUTHOR: Marcio Luis Teixeira
       CREATED: 3/16/95

       LAST REVISION: 11/29/2021

       (c) 1994-2021 by Marcio Luis Teixeira.
       All rights reserved.

*************************************************************/

#include "text_box.h"

#include <stdio.h>
#include <string.h>

TBHandle gCurTB;

static pascal void TBScrollProc( ControlHandle cntl, short part );
static void TBGetRects(const Rect *r, Rect *viewRect, Rect *destRect, Rect *scrollRect);

#define kSlowScrollSpeed    12

TBHandle TBNew( WindowPtr wind, const Rect *r ) {
    TBHandle    tb;
    Rect        destRect;
    Rect        viewRect;
    Rect        scrollRect;

    SetPort( wind );

    TextFont( geneva );

    tb = (TBHandle) NewHandle( sizeof( TBRec ) );
    if( tb == NULL )
        return NULL;

    HLock( (Handle) tb );
    TBRec &my = **tb;

    my.frame = *r;
    TBGetRects(r, &viewRect, &destRect, &scrollRect);
    my.tbox = TEStylNew( &destRect, &viewRect );
    if (my.tbox == NULL) {
        DisposeHandle( (Handle) tb );
        return NULL;
    }

    my.scroll = NewControl( wind, &scrollRect, "\p", TRUE, 0, 0, 0, scrollBarProc, 1 );
    if (my.scroll == NULL) {
        TEDispose( my.tbox );
        DisposeHandle( (Handle) tb );
        return NULL;
    }

    my.lastV    = 0;
    my.maxV     = 0;
    TEActivate( my.tbox );
    HUnlock( (Handle) tb );

    return tb;
}

void TBDispose( TBHandle tb ) {
    HLock( (Handle) tb );
    TBRec &my = **tb;

    DisposeControl( my.scroll );
    TEDispose( my.tbox );
    DisposeHandle( (Handle) tb );
}

void TBUpdate( TBHandle tb ) {
    HLock( (Handle) tb );
    TBRec &my = **tb;

    TEUpdate( &(*my.tbox)->viewRect, my.tbox );
    FrameRect( &my.frame );

    HUnlock( (Handle) tb );
}

void TBGetRects(const Rect *r, Rect *viewRect, Rect *destRect, Rect *scrollRect) {
    *viewRect = *r;
    viewRect->right -= 16;
    *destRect = *viewRect;
    InsetRect( viewRect, 1, 1 );
    InsetRect( destRect, 4, 4 );
    *scrollRect = *r;
    scrollRect->right--;
    scrollRect->left = scrollRect->right - 16;
}

void TBResize( TBHandle tb, const Rect *r ) {
    Rect        destRect;
    Rect        viewRect;
    Rect        scrollRect;

    TBSetScroll( tb, 0 );

    HLock( (Handle) tb );
    TBRec &my = **tb;

    my.frame = *r;
    TBGetRects(r, &viewRect, &destRect, &scrollRect);
    (*my.tbox)->destRect = destRect;
    (*my.tbox)->viewRect = viewRect;
    TECalText( my.tbox );

    MoveControl( my.scroll, scrollRect.left, scrollRect.top );
    SizeControl( my.scroll, scrollRect.right - scrollRect.left,
                            scrollRect.bottom - scrollRect.top );

    my.maxV = TEGetHeight( 0, 32767, my.tbox );
    my.maxV -= my.frame.bottom - my.frame.top - 30;
    SetCtlMax( my.scroll, my.maxV );

    HUnlock( (Handle) tb );
}


bool TBMouseDown( TBHandle tb, Point where, WindowPtr whichWindow ) {
    if (!PtInRect( where, &(*tb)->frame )) {
        return false;
    }

    HLock( (Handle) tb );
    TBRec &my = **tb;

    // Return if hidden
    if((*my.scroll)->contrlVis == 0) return 0;

    ControlHandle   whichCntl;
    short partCode = FindControl(where, whichWindow, &whichCntl);
    if (partCode) {
        if( partCode == inThumb )
            partCode = TrackControl( whichCntl, where, (ControlActionUPP) -1L );
        else {
            ControlActionUPP tbAction;

            tbAction = NewControlActionProc( TBScrollProc );
            gCurTB = tb;
            partCode = TrackControl( whichCntl, where, tbAction );
            DisposeRoutineDescriptor( tbAction );
        }

        if (partCode == inThumb) {
            const short newV = GetCtlValue( my.scroll );
            TBSetScroll( tb, newV );
        }
    }
    HUnlock( (Handle) tb );
    return true;
}

static pascal void TBScrollProc( ControlHandle cntl, short part ) {
    short       newV;
    short       max;
    short       height;
    long        lastTicks;

    TBRec &my = **gCurTB;

    newV = my.lastV;
    height = my.frame.bottom - my.frame.top;

    switch( part ) {
        case inDownButton:  newV += kSlowScrollSpeed; break;
        case inUpButton:    newV -= kSlowScrollSpeed; break;
        case inPageUp:      newV -= height; break;
        case inPageDown:    newV += height; break;
    }

    TBSetScroll( gCurTB, newV );

    for( lastTicks = TickCount(); lastTicks == TickCount(); );
}

void TBSetScroll( TBHandle tb, short scroll ) {
    TBRec &my = **tb;

    if( scroll < 0 )
        scroll = 0;
    if( scroll > my.maxV )
        scroll = my.maxV;

    SetCtlValue( my.scroll, scroll );
    TEScroll( 0, my.lastV - scroll, my.tbox );
    my.lastV = scroll;
}

OSErr TBReadSimpleText( TBHandle tb, const FSSpec *docSpec) {
    short fRefNum;

    TBSetScroll(tb, 0);

    HLock( (Handle) tb );
    TBRec &my = **tb;

    // Load the text from the data fork

    OSErr err = FSpOpenDF(docSpec, fsRdPerm, &fRefNum);
    if(err != noErr) return err;

    long dataSize;
    GetEOF(fRefNum, &dataSize);
    if(dataSize > 31767) {
        dataSize = 31767;
    }

    Ptr data = NewPtr(dataSize);
    if(!data) {
        FSClose(fRefNum);
        return memFullErr;
    }

    err = FSRead(fRefNum, &dataSize, data);
    if(err != noErr) {
        FSClose(fRefNum);
        DisposePtr(data);
        return err;
    }

    // Set the text and clean up
    TESetText(data, dataSize, my.tbox);
    FSClose(fRefNum);
    DisposePtr(data);

    // Load the style information from the resource fork

    fRefNum = FSpOpenResFile(docSpec, fsRdPerm);
    if (fRefNum != -1) {
        short oldResFile = CurResFile();
        UseResFile(fRefNum);

        Handle hStyle = Get1Resource('styl', 128);
        if (hStyle) {
            HNoPurge(hStyle);
            TEUseStyleScrap(0, dataSize, (StScrpHandle) hStyle, true, my.tbox);
            TECalText(my.tbox);
            ReleaseResource(hStyle);
        }
        UseResFile(oldResFile);
        CloseResFile(fRefNum);
    }

    // Adjust the scrollbar max

    my.lastV = 0;
    my.maxV = TEGetHeight( 0, 32767, my.tbox );
    my.maxV -= my.frame.bottom - my.frame.top - 30;
    if( my.maxV < 0 )
        my.maxV = 0;
    SetCtlMax( my.scroll, my.maxV );

    HUnlock( (Handle) tb );
    return noErr;
}