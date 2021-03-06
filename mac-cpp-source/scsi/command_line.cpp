#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <SIOUX.h>

#include "command_line.h"
#include "mac_vol.h"
#include "mac_scsi.h"
#include "iomega_cmds.h"

bool process_command();
void printn( unsigned char *c, int n );
void print_help();
void scan_bus();
void dev_info( int id );
void show_spares( int id );

void command_line_event_loop() {
    for(int i = 0; i < 20; i++) printf("\n");
    print_help();

    do {
        EventRecord event;
        if (WaitNextEvent(everyEvent, &event, GetCaretTime(), NULL))
            SIOUXHandleOneEvent(&event);

    } while (process_command());

    printf( "Returning to TIP...\n" );
}

bool process_command() {
    short int arg_val = 0;
    char cmd[80];
    printf( "\nCmd> " );
    gets( cmd );
    printf("\n");

    char *arg_str = strchr(cmd, ' ');
    if(arg_str) {
        while(*arg_str == ' ') arg_str++;
        arg_val = atoi(arg_str);
    }

    switch( tolower(cmd[0]) ) {
        case 'h': print_help(); break;
        case 'l': scan_bus(); break;
        case 's': iomega_spin_up_cartridge(arg_val); break;
        case 'p': iomega_spin_down_cartridge(arg_val); break;
        case 'r': scsi_reset(); break;
        case 'e': mac_eject(arg_val); break;
        case 'i': dev_info(arg_val); break;
        case 'v': mac_list_volumes(); break;
        case 'u': mac_unmount(arg_val); break;
        case 'q': return false;
        case 'd': mac_list_drives(); break;
        case 'm': if(arg_str) mac_mount_drive(arg_val); else mac_mount_drives(); break;
        default: printf("Unknown command, type 'h' for help\n");
    }
    return true;
}

void print_help() {
    printf(
        "\nGeneral commands:\n"
        "  help        : print this help\n"
        "  quit        : exit the command line\n"

        "\nMacintosh volume commands:\n"
        "  volumes     : list Mac volumes\n"
        "  eject   [n] : eject a volume\n"
        "  unmount [n] : unmount a volume\n"

        "\nMacintosh drive commands:\n"
        "  drives      : list all drives\n"
        "  mount   [n] : mount a drive\n"
        "  mount       : mount all drives\n"

        "\nGeneral SCSI operations:\n"
        "  reset       : reset the SCSI bus\n"
        "  list        : list devices on the SCSI bus\n"
        "  info    [n] : display SCSI inquiry\n"

        "\nIomega device operations on SCSI device:\n"
        "  spin    [n] : spin up a cartridge\n"
        "  pause   [n] : spin down a cartridge\n"
    );
}

void scan_bus() {
   short          err, id;
   scsi_inq_reply reply;

   for( id=0; id<8; id++ ) {
      err = scsi_inquiry( id, 0, &reply);
      if( err != 0 ) {
         printf( "%4hd: (Not installed)\n", id );
      } else {
         printf( "%4hd: ", id );
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