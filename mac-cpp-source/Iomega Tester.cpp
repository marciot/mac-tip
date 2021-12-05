#include <stdio.h>
#include <SIOUX.h>
#include <ctype.h>

#include "tip.h"


void main() {
    SIOUXSettings.autocloseonquit = TRUE;
    SIOUXSettings.asktosaveonclose = FALSE;
    SIOUXSettings.standalone = FALSE;
    SIOUXSettings.leftpixel = 8;
    SIOUXSettings.toppixel = 44;

    printf( "------------------------------------------------------------------------------\n" );
    printf( "This Mac port (c) 2021 Marcio Teixeira       http://github.com/marciot/mac-tip\n" );
    printf( "Based on code (c) 2006 Gibson Research Corp  http://grc.com/tip/clickdeath.htm\n" );
    printf( "------------------------------------------------------------------------------\n" );
    printf("\nStarting tip ...\n");

    SIOUXSetTitle("\pTrouble in Paradise for Macintosh (" __DATE__ ")");

    run_tip();
}