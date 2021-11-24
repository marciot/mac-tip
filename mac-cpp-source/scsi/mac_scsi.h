#include <stdio.h>

typedef Boolean bool;

enum {
    SCSI_Cmd_RequestSense   = 0x03,
    SCSI_Cmd_FormatUnit     = 0x04,
    SCSI_Cmd_NonSenseData   = 0x06,
    SCSI_Cmd_Read           = 0x08,
    SCSI_Cmd_Write          = 0x0a,
    SCSI_Cmd_CartProtect    = 0x0c,
    SCSI_Cmd_Inquiry        = 0x12,
    SCSI_Cmd_ModeSelect     = 0x15,
    SCSI_Cmd_ModeSense      = 0x1a,
    SCSI_Cmd_StartStopUnit  = 0x1b,
    SCSI_Cmd_SendDiagnostic = 0x1d,
    SCSI_Cmd_PreventAllow   = 0x1e,
    SCSI_Cmd_TranslateLBA   = 0x22,
    SCSI_Cmd_FormatTest     = 0x24,
    SCSI_Cmd_ReadMany       = 0x28,
    SCSI_Cmd_WriteMany      = 0x2a,
    SCSI_Cmd_Verify         = 0x2f,
    SCSI_Cmd_ReadDefectData = 0x37,
    SCSI_Cmd_ReadLong       = 0x3e,
    SCSI_Cmd_WriteLong      = 0x3f
};

enum {
   SCSI_DISK          = 0x00,
   SCSI_TAPE          = 0x01,
   SCSI_PRINTER       = 0x02,
   SCSI_CPU_DEVICE    = 0x03,
   SCSI_WORM          = 0x04,
   SCSI_CDROM         = 0x05,
   SCSI_SCANNER       = 0x06,
   SCSI_OPTICAL       = 0x07,
   SCSI_MEDIA_CHANGER = 0x08,
   SCSI_COM           = 0x09,
   SCSI_UNKNOWN       = 0x1F
};

typedef struct {                 /*    | 7   6   5   4   3   2   1   0 |  */
   unsigned char inf1;           /* 0  | qualifier |     device class  |  */
   unsigned char flg1;           /* 1  |RMB|         reserved          |  */
   unsigned char vers;           /* 2  |  ISO  |   ECMA    |    ANSI   |  */
   unsigned char inf2;           /* 3  |AEN|TIO|  rsv  |  data format  |  */
   unsigned char alen;           /* 4  |       additional length       |  */
   unsigned char rsv1[2];        /* 5  |       reserved                |  */
   unsigned char flg2;           /* 7  |rel|W32|W16|Syn|Lnk|Rsv|Que|SRs|  */
   unsigned char vend[8];        /* 8  |       manufacturer            |  */
   unsigned char prod[16];       /* 16 |       product                 |  */
   unsigned char rvsn[4];        /* 32 |       revision                |  */
   unsigned char rsv2[20];       /* 36 |       vendor unique           |  */
   unsigned char rsv3[40];       /* 56 |       reserved                |  */
   unsigned char rsv4[160];      /* 96 |       vendor unique           |  */
} scsi_inq_reply;

// Reference: https://docs.oracle.com/en/storage/tape-storage/storagetek-sl150-modular-tape-library/slorm/request-sense-03h.html#GUID-9309F2C0-ABF8-470E-AE25-E9535C821B39

typedef struct {                 /*    | 7   6   5   4   3   2   1   0 |  */
   unsigned char err;            /* 0  |OK |       error code          |  */
   unsigned char seg;            /* 1  |     segment number            |  */
   unsigned char key;            /* 2  |    reserved   |   sense key   |  */
   unsigned char inf1[4];        /* 3  |          information          |  */
   unsigned char alen;           /* 7  |       additional length       |  */
   unsigned char inf2[4];        /* 8  | command specific information  |  */
   unsigned char asc;            /* 12 |    additional sense code      |  */
   unsigned char ascq;           /* 13 |        asc qualifier          |  */
   unsigned char fruc;           /* 14 |  field replaceable unit code  |  */
   unsigned char flg1;           /* 15 |SKV|C/D|  Rsv  |bpv|  bit ptr  |  */
   unsigned char fp[2];          /* 16 |       field pointer           |  */
   unsigned char rsv[2];         /* 18 |       reserved                |  */
} scsi_sense_reply;

/* Flags for scsi_cmd */

#define SCSI_READ  0x0001
#define SCSI_WRITE 0x0002
#define SCSI_BLIND 0x0004

OSErr scsi_reset();
OSErr scsi_cmd(int id, void *cmd, size_t clen, void *buff, size_t siz, size_t cnt, int flags, char *status = 0);
OSErr scsi_inquiry(int id, int lun, scsi_inq_reply *buff);
OSErr scsi_request_sense_data(int id, scsi_sense_reply *buff);
OSErr scsi_get_class(int id, int lun, int &dev );


