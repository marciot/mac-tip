#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <SIOUX.h>

#include "tip.h"
#include "mac_vol.h"
#include "mac_scsi.h"
#include "iomega_cmds.h"

bool process_command();
void confirm_run_tip(int id);
void printn( unsigned char *c, int n );
void print_help();
void scan_bus();
void dev_info( int id );
void show_spares( int id );

void main() {
    SIOUXSettings.autocloseonquit = TRUE;
    SIOUXSettings.asktosaveonclose = FALSE;
    SIOUXSettings.standalone = FALSE;

    printf( "This Mac port (c) 2021 Marcio Teixeira       http://github.com/marciot/mac-tip\n" );
    printf( "Based on code (c) 2006 Gibson Research Corp  http://grc.com/tip/clickdeath.htm\n" );

    SIOUXSetTitle("\pIomega Tester Beta (" __DATE__ ")");

    print_help();

    do {
        EventRecord event;
        if (WaitNextEvent(everyEvent, &event, GetCaretTime(), NULL))
            SIOUXHandleOneEvent(&event);

    } while (process_command());

    printf( "Goodbye.\n" );
}

bool process_command() {
    short int arg_val = 0;
    char cmd[80];
    printf( "\nCmd> " );
    gets( cmd );
    printf("\n");

    char *arg_str = strchr(cmd, ' ');
    while(*arg_str == ' ') arg_str++;
    if(arg_str) arg_val = atoi(arg_str);

    switch( tolower(cmd[0]) ) {
        case 'h': print_help(); break;
        case 'l': scan_bus(); break;
        case 's': iomega_spin_up_cartridge(arg_val); break;
        case 'r': scsi_reset(); break;
        case 'e': iomega_eject_cartridge(arg_val); break;
        case 'i': dev_info(arg_val); break;
        case 'v': mac_list_volumes(); break;
        case 'u': mac_unmount(arg_val); break;
        case 't': confirm_run_tip(arg_val); break;
        case 'q': return false;
        case 'o': SetRichEditText(arg_str); break;
        default: printf("Unknown command, type 'h' for help\n");
    }
    return true;
}

void confirm_run_tip(int id) {
    char cmd[80];
    printf("\nThis program is in BETA TESTING and may cause severe data loss!\n\nProceed [Y/N]? ");
    gets( cmd );
    if(tolower(cmd[0]) == 'y') {
        run_tip(id);
    }
}

void print_help() {
    printf(
        "\nGeneral commands:\n"
        "  help        : print this help\n"
        "  quit        : quit the program\n"

        "\nMacintosh commands (please unmount Zip prior to testing):\n"
        "  volumes     : list Mac volumes\n"
        "  unmount [n] : unmount a volume\n"

        "\nGeneral SCSI operations:\n"
        "  reset       : reset the SCSI bus\n"
        "  list        : list devices on the SCSI bus\n"
        "  info    [n] : display SCSI inquiry\n"

        "\nIomega device operations on SCSI device:\n"
        "  spin    [n] : spin up a cartridge\n"
        "  eject   [n] : eject cartridge\n"
        "  tip     [n] : run Steve Gibson's TIP 2.1\n"
    );
}

void scan_bus() {
   short          err, id;
   scsi_inq_reply reply;

   for( id=0; id<8; id++ ) {
      err = scsi_inquiry( id, 0, &reply);
      if( err != 0 ) {
         printf( "   %hd: (Not installed)\n", id );
      } else {
         printf( "   %hd: ", id );
         printn( reply.vend, 8 );
         printf( ", " );
         printn( reply.prod, 16 );
         printf( ", " );
         printn( reply.rvsn, 4 );
         putchar( '\n' );
      }
   }
}

void dev_info( int id ) {
   short err, lun;
   scsi_inq_reply reply;

   for( lun = 0; lun < 8; lun++ ) {
      err = scsi_inquiry( id, lun, &reply);
      if( err ) {
         printf( "Device not installed\n" );
         return;
      }

      printf( "   LUN %hd: ", lun );
      switch( (reply.inf1 & 0xE0) >> 5 ) {
         case 0x00: printf( "supported and connected\n" ); break;
         case 0x01: printf( "not connected\n" ); continue;
         case 0x03: printf( "not supported\n" ); continue;
      }

      printf( "      Device class (%02X): ", reply.inf1 & 0x1F );
      switch( reply.inf1 & 0x1F ) {
         case 0x00: printf( "Disk drive\n" ); break;
         case 0x01: printf( "Tape drive\n" ); break;
         case 0x02: printf( "Printer\n" ); break;
         case 0x03: printf( "Processor device\n" ); break;
         case 0x04: printf( "WORM drive\n" ); break;
         case 0x05: printf( "CD-ROM drive\n" ); break;
         case 0x06: printf( "Scanner\n" ); break;
         case 0x07: printf( "Optical disk\n" ); break;
         case 0x08: printf( "Media changer\n" ); break;
         case 0x09: printf( "Communication device\n" ); break;
         case 0x1F: printf( "Unknown device\n" ); break;
         default: printf( "Reserved\n" );
      }

      printf( "      ANSI version (%02X): ", reply.vers & 0x07 );
      switch( reply.vers & 0x07 ) {
         case 0x00: printf( "SCSI-1\n" ); break;
         case 0x01: printf( "SCSI-1 w/ CCS\n" ); break;
         case 0x02: printf( "SCSI-2\n" ); break;
         default: printf( "???\n" );
      }
      printf( "      ISO version  (%02X)\n", reply.vers & 0xC0 >> 6 );
      printf( "      ECMA version (%02X)\n", reply.vers & 0x78 >> 3 );

      printf( "      Flags: " );
      if( reply.flg1 & 0x80 ) printf( "rmb " );
      if( reply.flg2 & 0x80 ) printf( "rel " );
      if( reply.flg2 & 0x40 ) printf( "w32 " );
      if( reply.flg2 & 0x20 ) printf( "w16 " );
      if( reply.flg2 & 0x10 ) printf( "syn " );
      if( reply.flg2 & 0x08 ) printf( "lnk " );
      if( reply.flg2 & 0x04 ) printf( "??? " );
      if( reply.flg2 & 0x02 ) printf( "que " );
      if( reply.flg2 & 0x01 ) printf( "sfR " );
      if( reply.inf2 & 0x80 ) printf( "aen " );
      if( reply.inf2 & 0x40 ) printf( "tio " );
      printf( "\n" );
   }
}

void printn( unsigned char *c, int n ) {
   while( n-- ) {
      putchar( isprint(*c) ? *c : '.' );
      c++;
   }
}
