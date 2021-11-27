#include <scsi.h>
#include "mac_scsi.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define READ_TIMEOUT 60   /* 300 ticks = 5 seconds */

OSErr scsi_reset() {
   return SCSIReset();
}

OSErr scsi_cmd(int id, void *cmd, size_t clen, void *buff, size_t siz, size_t cnt, int flags, char *status) {
   SCSIInstr       TIB[3];        /* Transfer instruction block */
   OSErr           err;

   /* Set up the TIB (used by Macintosh SCSI Manager) */

   if( cnt != 0 ) {
      /* Transfer cnt continguous blocks of size siz, one at a time.   */
      /* Use this sort of transfer when doing blind transfers to a     */
      /* disk drive, in order to force the SCSI to poll the bus        */
      /* between each block [Inside Macintosh: Devices, 3-22]          */

      TIB[0].scOpcode = scInc;
      TIB[0].scParam1 = (long) buff;
      TIB[0].scParam2 = siz;
      TIB[1].scOpcode = scLoop;
      TIB[1].scParam1 = -10;
      TIB[1].scParam2 = cnt;
      TIB[2].scOpcode = scStop;
      TIB[2].scParam1 = 0;
      TIB[2].scParam2 = 0;
   } else {
      /* Transfer siz bytes in one fell swoop. */
      //printf("CMD: %x SCSI Siz %ld\n", int(*((char*)cmd)), siz);
      TIB[0].scOpcode = scInc;
      TIB[0].scParam1 = (long) buff;
      TIB[0].scParam2 = siz;
      TIB[1].scOpcode = scStop;
      TIB[1].scParam1 = 0;
      TIB[1].scParam2 = 0;
   }

   /* Arbitrate for the bus and select the device. */
   err = SCSIGet();
   if (err != noErr) {printf("SCSIGet Error: %d\n", err); return err;}

   err = SCSISelect(id);
   if (err != noErr) {/*printf("SCSISelect Error: %d\n", err);*/ return err;}

   /* Send the command to the SCSI device and perform the requested I/O */
   err = SCSICmd( (Ptr) cmd, clen );
   if (err == noErr) {
      OSErr io_err = noErr;
      switch(flags) {
         case SCSI_WRITE | SCSI_BLIND: io_err = SCSIWBlind( (Ptr) TIB ); break;
         case SCSI_READ  | SCSI_BLIND: io_err = SCSIRBlind( (Ptr) TIB ); break;
         case SCSI_WRITE:              io_err = SCSIWrite( (Ptr) TIB ); break;
         case SCSI_READ:               io_err = SCSIRead( (Ptr) TIB ); break;
         default: break;
      }
      if (io_err != noErr) {
         printf("SCSI Read/Write Error: %d\n", io_err);
      }
   } else {
      printf("SCSICmd Error: %d\n", err);
   }

   /* Complete the transaction and release the bus */
   short cstat, cmsg;
   OSErr comperr = SCSIComplete( &cstat, &cmsg, READ_TIMEOUT );
   if(status) *status = cstat;
   if (comperr != noErr) {printf("SCSIComplete Error: %d (status: %d)\n", err, cstat); return err;}

   return err;
}

OSErr scsi_inquiry(int id, int lun, scsi_inq_reply *reply) {
   short err;
   unsigned char cmd[6] = { SCSI_Cmd_Inquiry, lun << 5, 0x00, 0x00, 0x05, 0x00 };

   memset(reply, 0, sizeof(scsi_inq_reply));

   /* First we issue a dummy command to get the additional data field, then   */
   /* we use that number to issue a second command with the corrected length. */

   if( (err = scsi_cmd(id, cmd, sizeof(cmd), reply, cmd[4], 0, SCSI_READ)) != noErr) {
      return err;
   }
   cmd[4] += reply->alen;
   return scsi_cmd(id, cmd, sizeof(cmd), reply, cmd[4], 0, SCSI_READ);
}

OSErr scsi_request_sense_data(int id, scsi_sense_reply *reply) {
   unsigned char cmd[6] = {SCSI_Cmd_RequestSense, 0x00, 0x00, 0x00, sizeof(scsi_sense_reply), 0x00};
   return scsi_cmd(id, cmd, sizeof(cmd), reply, cmd[4], 0, SCSI_READ);
}

OSErr scsi_get_class(int id, int lun, int &dev ) {
   scsi_inq_reply reply;
   OSErr err = scsi_inquiry(id, lun, &reply );
   dev = reply.inf1 & 0x1F;
   return err;
}
