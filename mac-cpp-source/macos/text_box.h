/************************************************************

    Text Box.h

       AUTHOR: Marcio Luis Teixeira
       CREATED: 3/16/95

       LAST REVISION: 11/29/2021

       (c) 1994-2021 by Marcio Luis Teixeira.
       All rights reserved.

*************************************************************/

#include "ctype.h"

typedef struct {
    TEHandle            tbox;
    Rect                frame;
    ControlHandle       scroll;
    short               lastV;
    short               maxV;
} TBRec, *TBPtr, **TBHandle;

TBHandle TBNew( WindowPtr wind, const Rect *r );
void TBDispose( TBHandle html );
void TBUpdate( TBHandle html );
void TBResize( TBHandle html, const Rect *r );
void TBSetScroll( TBHandle html, short scroll );
bool TBMouseDown( TBHandle html, Point where, WindowPtr whichWindow );
OSErr TBReadSimpleText( TBHandle tb, const FSSpec *docSpec, bool redraw);
