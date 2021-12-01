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

    printf(
        "\n\n"
        "Trouble in Paradise\n"
        "===================\n\n"
        "A Macintosh port of \"Trouble in Paradise\" for Windows, made possible by a\n"
        "generous code donation by Steve Gibson from Gibson Research Corporation.\n\n"
        "This freeware utility determines whether an Iomega Zip or Jaz drive is prone\n"
        "to developing the dreaded \"Click of Death\" (COD) syndrome. Steve Gibson's\n"
        "research into the maintenance, repair and data recovery of Iomega's removable\n"
        "media mass storage products led to this capability.\n\n"
    );

    printf( "------------------------------------------------------------------------------\n" );
    printf( "This Mac port (c) 2021 Marcio Teixeira       http://github.com/marciot/mac-tip\n" );
    printf( "Based on code (c) 2006 Gibson Research Corp  http://grc.com/tip/clickdeath.htm\n" );
    printf( "------------------------------------------------------------------------------\n" );

    SIOUXSetTitle("\pTrouble in Paradise for Macintosh (" __DATE__ ")");

    // Confirm that the user wants to run TIP

    char cmd[80];
    printf("\n\nThis program is in BETA TESTING and may cause data loss!\n\nProceed [Y/N]? ");
    gets( cmd );
    if(tolower(cmd[0]) == 'y') {
        run_tip();
        printf("\n\nYou may need to REBOOT your Mac before cartridges are recognized by Mac OS.");
    }
}