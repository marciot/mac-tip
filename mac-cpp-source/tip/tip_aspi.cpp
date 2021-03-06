#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "mac_scsi.h"
#include "tip.h"

//#define DEMO

/* The original TIP seems to request more data than is supplied by
 * certain commands. While this appears to be allowed, it causes
 * SCSI phase errors to be reported. Setting NO_EXCESS_READS will
 * adjust the reads to to the max size before such errors occur.
 */
#define NO_EXCESS_READS

/* The original TIP will always try to enable Early Recovery. This
 * fails on certain Jaz drives. While the original TIP will then
 * retry without Early Recovery, this will cause many errors to be
 * reported. Enable SUPRESS_ER_ERRORS to prevent this from problem
 * from happening as frequently
 */
#define SUPRESS_ER_ERRORS

/* The original TIP will always try to read the defects list, but
 * not all drives support this, causing many errors to be shown.
 * Setting SUPPRESS_DEFECTS_ERROR will silence these errors.
 */
#define SUPPRESS_DEFECTS_ERROR

#define MAKE_LITTLE_ENDIAN(a) a // Don't do anything on 68000
#define MAKE_BIG_ENDIAN(a)  a // Don't do anything on 68000

// The following crashes on the 68000 due to unaligned access
//#define BYTE_AT(s, a)  *((char*)(s + a))
//#define WORD_AT(s, a)  *((short*)(s + a))
//#define DWORD_AT(s, a) *((long*)(s + a))

// Allows unaligned memory access
#define SET_BYTE_AT(s, a, v)  s[a  ] = v;
#define SET_WORD_AT(s, a, v)  s[a  ] = (v & 0xFF00) >> 8; \
                              s[a+1] = (v & 0x00FF);
#define SET_DWORD_AT(s, a, v) s[a  ] = (v & 0xFF000000) >> 24; \
                              s[a+1] = (v & 0x00FF0000) >> 16; \
                              s[a+2] = (v & 0x0000FF00) >>  8; \
                              s[a+3] = (v & 0x000000FF);

#define GET_BYTE_AT(s, a)    ((unsigned char)((unsigned char)s[a  ]))
#define GET_WORD_AT(s, a)  (((unsigned short)((unsigned char)s[a  ])) << 8)  | \
                            ((unsigned short)((unsigned char)s[a+1]))
#define GET_DWORD_AT(s, a) (((unsigned  long)((unsigned char)s[a  ])) << 24) | \
                           (((unsigned  long)((unsigned char)s[a+1])) << 16) | \
                           (((unsigned  long)((unsigned char)s[a+2])) <<  8) | \
                            ((unsigned  long)((unsigned char)s[a+3]))

// offsets to the various sector data images

#define ZIP_100_PART 0x0000
#define ZIP_100_BOOT 0x0200
#define ZIP_250_PART 0x0400
#define ZIP_250_BOOT 0x0600
#define JAZ_1GB_PART 0x0800
#define JAZ_1GB_BOOT 0x0A00
#define JAZ_2GB_PART 0x0C00
#define JAZ_2GB_BOOT 0x0E00

struct DEFECT_LIST_HEADER {
    char  DLH_reserved; // (00h)
    char  DLH_BitFlags; // [000] [P] [G] [xxx - defect list format]
    short DLH_DefectListLength;
};

#define ERROR_RECOVERY_PAGE                  1 // From disassembly
#define FORMAT_STATUS_PAGE                   1
#define DISK_STATUS_PAGE                     2

#define NEW_DISK_STATUS_OFFSET               3 // newer offset of the Disk Status Byte
#define OLD_DISK_STATUS_OFFSET               1 // older offset of the Disk Status Byte

#define JAZ_SPARES_COUNT_OFFSET              68 // offsets into DiskStat tbl
#define NEW_ZIP_SIDE_0_SPARES_COUNT_OFFSET   13
#define NEW_ZIP_SIDE_1_SPARES_COUNT_OFFSET   17
#define OLD_ZIP_SIDE_0_SPARES_COUNT_OFFSET   11
#define OLD_ZIP_SIDE_1_SPARES_COUNT_OFFSET   15
#define JAZ_PROTECT_MODE_OFFSET              21
#define NEW_ZIP_PROTECT_MODE_OFFSET          21
#define OLD_ZIP_PROTECT_MODE_OFFSET          19
#define JAZ_LAST_LBA_OFFSET                  5
#define NEW_ZIP_LAST_LBA_OFFSET              5
#define OLD_ZIP_LAST_LBA_OFFSET              3

#define DRIVE_A_SUPPORT_BIAS                 32 // reduce total by 32 for DRIVE A support

#define BYTES_PER_SECTOR                     512
#define MAX_SECTORS_PER_TEST                 128

#define BADNESS_THRESHOLD                    10

#define SS_ERR                               0x00000004
#define DEFECT_LIST_READ_ERROR               0x001c0003
#define LBA_TOO_LARGE                        0x00210005 // accessed a non-exist LBA
#define MEDIA_CHANGE_CODE                    0x00280006 // media was changed
#define INCOMPATIBLE_MEDIA                   0x00300002 // 2Gb / 1Gb combo on "Read Defects"
#define MEDIA_NOT_PRESENT                    0x003a0002
#define DRIVE_COMING_READY                   0x00040102
#define SCSI_CMD_TIMED_OUT                   0x00FFFF00
#define BUFFER_TOO_BIG                       0x00FFFFE6
#define MANUAL_INTERRUPTION                  0xFFFFFFFF

#define CHECK_CONDITION 0x02

TipPage CurrentPage;
long CurrentDevice = -1; // the device that's been recognized
long DriveCount = 0;

long JazDrive = 0; // true if the current drive
long CartridgeStatus = DISK_NOT_PRESENT;

#ifdef SUPRESS_ER_ERRORS
    Boolean SupressEarlyRecovery = false;
#endif
#ifdef SUPPRESS_DEFECTS_ERROR
    Boolean SupressDefectsError = false;
#endif

unsigned long StartingInstant;

// ----------------------------- Run Time Variables ------------------------------

long Side_0_SparesCount; // JAZ has only one count
long Side_1_SparesCount; // ZIP has counts for both sides
long Initial_Side_0_Spares;
long Initial_Side_1_Spares;

long TestingPhase = 0; // 0 = not testing, no data ...
long PercentComplete;
long FirstLBASector;
long NumberOfLBAs;
long AdapterMaxSectors;
long LastLBAOnCartridge;
long SecondsElapsed;
long SoftErrors;
long FirmErrors;
long HardErrors;
long ElapsedTimeOfLastEstimate;
long CurrentTotalTimeEstimate;
bool UserInterrupt;
long LastError;
long SingleTransferLBA;

DriveEntry DriveArray[MAX_DRIVE_COUNT];

/*******************************************************************************
 * GET DRIVE ENTRY OFFSET
 *
 * Returns the offset of the chosen drive's status word
 *******************************************************************************/
int GetDriveEntryOffset(short Device) {
    for(int i = 0; i < MAX_DRIVE_COUNT; i++)
        if(DriveArray[i].scsi_id == Device) // did we find the right table slot?
            return i;
    return 0;
}

/*******************************************************************************
 * GET COMMAND DETAILS
 *
 * Given a SCSI command byte, this returns the command
 * block length in AL  and the Command Flags in AH
 *******************************************************************************/

#define TEN_BYTE_CMDS 0x1F
#define SRB_DIR_IN SCSI_READ
#define SRB_DIR_OUT SCSI_WRITE

void GetCommandDetails(char command, char &cmd_flags, char &cmd_length) {
    char CommandDetailsTable[] = {
        SCSI_Cmd_RequestSense,    SRB_DIR_IN,   // 03 IN == get from drive
        SCSI_Cmd_FormatUnit,      0,            // 04 OUT == send to drive
        SCSI_Cmd_NonSenseData,    SRB_DIR_IN,   // 06
        SCSI_Cmd_Read,            SRB_DIR_IN,   // 08
        SCSI_Cmd_Write,           SRB_DIR_OUT,  // 0A
        SCSI_Cmd_CartProtect,     SRB_DIR_OUT,  // 0C
        SCSI_Cmd_Inquiry,         SRB_DIR_IN,   // 12
        SCSI_Cmd_ModeSelect,      SRB_DIR_OUT,  // 15
        SCSI_Cmd_ModeSense,       SRB_DIR_IN,   // 1A
        SCSI_Cmd_StartStopUnit,   0,            // 1B
        SCSI_Cmd_SendDiagnostic,  0,            // 1D
        SCSI_Cmd_PreventAllow,    0,            // 1E
        SCSI_Cmd_TranslateLBA,    SRB_DIR_IN,   // 22
        SCSI_Cmd_FormatTest,      0,            // 24
        SCSI_Cmd_ReadMany,        SRB_DIR_IN,   // 28
        SCSI_Cmd_WriteMany,       SRB_DIR_OUT,  // 2A
        SCSI_Cmd_Verify,          0,            // 2F
        SCSI_Cmd_ReadDefectData,  SRB_DIR_IN,   // 37
        SCSI_Cmd_ReadLong,        SRB_DIR_IN,   // 3E
        SCSI_Cmd_WriteLong,       SRB_DIR_OUT   // 3F
    };
    cmd_flags = 0; // ; if we don't locate it ... return ZERO
    // search the table for the command entry
    for(int i = 0; i < sizeof(CommandDetailsTable); i += 2) {
        if(CommandDetailsTable[i] == command) { // if we match we're done
            cmd_flags = CommandDetailsTable[i+1];
            break;
        }
    }
    cmd_length = 6; // presume a short (6 byte) command
    if(command > TEN_BYTE_CMDS) // but if it's a LONG one ....
        cmd_length = 10;
}

/*******************************************************************************
 * SCSI COMMAND
 *
 * This executes a SCSI command through the interface. It receives a
 * pointer to a standard SCSI command block (SCB) and a pointer and
 * length to an IoBuffer for the command. It returns the complete
 * three-byte sense code from the command.
 *******************************************************************************/
long SCSICommand(short Device, char *lpCmdBlk, void *lpIoBuf, size_t IoBufLen) {
    char cmd_length, cmd_flags, cmd_status;
    GetCommandDetails(lpCmdBlk[0], cmd_flags, cmd_length);
    // call the SCSI interface to forward the command to the device
    OSErr err = scsi_cmd(Device, lpCmdBlk, cmd_length, lpIoBuf, IoBufLen, 0, cmd_flags, &cmd_status);
    if(err != noErr) {
        // else, if it's *NOT* a "Sense Data" error (SS_ERR)
        LastError = err | 0x00FFFF00; // [00 FF FF er]
        return SS_ERR;
    }
    if(cmd_status == 0) {
        // if the command did not generate any Sense Data, just return NULL
        return 0;
    }
    else if(cmd_status == 2) { // Check Condition
        // Request sense data
        scsi_sense_reply sense_data;
        scsi_request_sense_data(Device, &sense_data);
        printf("SCSI CHECK CONDITION (KEY %x, ASC %x, ASCQ %x)\n", sense_data.key, sense_data.asc, sense_data.ascq);
        // okay, we have an SS_ERR condition, let's check the SENSE DATA
        // assemble [00 ASC ASCQ SenseKey]
        const long res = (long(sense_data.asc) << 16) |
                         (long(sense_data.ascq) << 8) |
                         (long(sense_data.key)      );
        if(res == MEDIA_CHANGE_CODE) {
            printf("Media change signalled. Most recent error can be ignored\n\n");
            int index = GetDriveEntryOffset(Device);
            DriveArray[index].flags |= MEDIA_CHANGED;
            return 0;
        }
        return res;
    }
    else {
        // else, if it's *NOT* a "Sense Data" error (SS_ERR)
        return cmd_status | 0x00FFFF00; // [00 FF FF er]
    }
}

/*******************************************************************************
 * ENUMERATE IOMEGA DEVICES
 *******************************************************************************/
short stricmp( const char *str1, const char *str2 );
short stricmp( const char *str1, const char *str2 ) {
    while (*str1 && *str2) {
        short cmp = tolower( *str1++ ) - tolower( *str2++ );
        if(cmp != 0) return cmp;
    }
    return 0;
}

long EnumerateIomegaDevices(uint8_t *DrivesSkipped) {
    DriveCount = 0;
    if(DrivesSkipped) *DrivesSkipped = 0;

    printf("\nEnumerating Iomega Devices:\n");
    // now scan the devices on the SCSI host adapter
    for(int Device = 0; Device < 8; Device++) {
        char flags = 0;
        //-----------------------------------------------------------
        #ifdef NO_EXCESS_READS
            scsi_inq_reply reply;
            if(scsi_inquiry(Device, 0, &reply) != noErr) continue;
            char *InqData = (char*) &reply;
        #else
            char InqData[96];
            char Scsi[6] = {0};
            Scsi[0] = SCSI_Cmd_Inquiry;
            Scsi[4] = sizeof(InqData);
            if(SCSICommand(Device, Scsi, InqData, sizeof(InqData))) continue;
        #endif

        //-----------------------------------------------------------
        InqData[14] = '\0';
        InqData[19] = '\0';
        const bool isIomega = !stricmp(szIomega, InqData + 8);
        const bool isZip    = !stricmp(szZip, InqData + 16);
        const bool isJaz    = !stricmp(szJaz, InqData + 16);
        //-----------------------------------------------------------

        if (isIomega && (isZip || isJaz)) {
            char flags = isJaz ? JAZ_DRIVE : 0;
            // check for ANSI SCSI to see whether we need to play
            // the Odd/Even password length game ...
            if(InqData[2] & 0x07 == 0) {
                flags |= ODD_BYTE_COMPENSATION; // turn on compensation
            }

            // On the Mac, we want to ignore drives that have media in them at
            // program entry, as this means the volume is mounted in Mac OS
            const bool driveEmpty = (GetCartridgeStatus(Device, flags) == DISK_NOT_PRESENT);
            if(driveEmpty) {
                DriveArray[DriveCount].flags = flags;
                DriveArray[DriveCount].scsi_id = Device;
                DriveCount++;
            } else {
                if(DrivesSkipped) (*DrivesSkipped)++;
            }

            printf("    %d: %s %s %s\n", Device,
                (flags & JAZ_DRIVE) ? "JAZ" : "ZIP",
                (flags & ODD_BYTE_COMPENSATION) ? "OBC" : "   ",
                (driveEmpty) ? "EMPTY" : "MEDIA");
        }
    }

    printf("\n");
    return DriveCount;
}

/*******************************************************************************
 * GET MODE PAGE
 *******************************************************************************/
long GetModePage(short Device, short PageToGet, void *pBuffer, short BufLen) {
    char Scsi[6] = {0};
    Scsi[0] = SCSI_Cmd_ModeSense;
    Scsi[2] = PageToGet;
    Scsi[4] = BufLen;
    return SCSICommand(Device, Scsi, pBuffer, BufLen);
 }

/*******************************************************************************
 * SET MODE PAGE
 *******************************************************************************/
long SetModePage(short Device, void *pBuffer, short BufLen) {
    unsigned char* ebx = (unsigned char*) pBuffer; // get a pointer to the top of buffer
    unsigned char ecx = ebx[0] + 1; // adjust it up by one

    ebx[0] = 0; // now clear the two reserved bytes
    ebx[2] = 0;

    if(ecx != BufLen) {
        printf("Length error in SetModePage %d != %d\n\n", BufLen, (int) ecx);
        return 0;
    }

    char Scsi[6] = {0}; // init the SCSI parameter block
    Scsi[0] = SCSI_Cmd_ModeSelect; // set the command
    Scsi[1] = 0x10; // set the Page Format bit
    Scsi[4] = ecx; // set the parameter list length
    return SCSICommand(Device, Scsi, pBuffer, ecx);
 }

/*******************************************************************************
 * SET ERROR RECOVERY
 *******************************************************************************/
void ModifyModePage(char *PageBuff, char ecc, char retries) {
    long eax = PageBuff[3]; // get the Block Descriptor Length

    char *ebx = PageBuff + 4; // get just past the header address
    // form ebx == the offset to the top of the page we've read ...
    ebx += eax;

    ebx[0] &= ~0x80; // always turn off the PS bit (parameters savable)
    ebx[2] = 0xC0 | ecc; // set the ECC fields
    ebx[3] = retries; // set the common retry count
    if(ebx[1] > 6) // if we have a large format page...
        ebx[8] = retries; // then set the write count too
}

long SetErrorRecovery(bool Retries, bool ECC, bool Testing) {
    char PageBuff[40];
    #ifdef NO_EXCESS_READS
        // Limit reads to 20 bytes on Zip (24 bytes on Jaz) to prevent controller errors
        const short pageBuffLen = JazDrive ? 24 : 20;
    #else
        const short pageBuffLen = sizeof(PageBuff);
    #endif

    long eax = GetModePage(CurrentDevice, ERROR_RECOVERY_PAGE, PageBuff, pageBuffLen);
    if(eax) {
        printf("SetErrorRecovery failed\n");
        return eax;
    }

    #define EARLY_RECOVERY 0x08
    #define PER            0x04
    #define SUPPRESS_ECC   0x01

    // set the ECC fields
    char ecc = SUPPRESS_ECC; // presume ECC suppression
    if(ECC) {
        #ifdef SUPRESS_ER_ERRORS
            if(!SupressEarlyRecovery)
        #endif
        ecc = EARLY_RECOVERY; // enable ECC and Early Recovery
        if(Testing) {
            ecc = EARLY_RECOVERY | PER; // we're testing, so EER & PER
        }
    }

    // set the retry counts
    char retries = 0x16; // set retries to 22 for Zip drive
    if(JazDrive)
        retries = 0x64; // and to 100 for Jaz drive
    if(!Retries) // But if we have no retries ...
        retries = 0;

    ModifyModePage(PageBuff, ecc, retries);

    eax = SetModePage(CurrentDevice, PageBuff, pageBuffLen);
    // if we had an invalid field in the CDB (the EER bit was on)
    if (eax == 0x00260005) {
        GetModePage(CurrentDevice, ERROR_RECOVERY_PAGE, PageBuff, pageBuffLen);
        ecc &= ~EARLY_RECOVERY; // same, *BUT*NOT* Early Recovery
        ModifyModePage(PageBuff, ecc, retries);
        eax = SetModePage(CurrentDevice, PageBuff, pageBuffLen);
        #ifdef SUPRESS_ER_ERRORS
            if(!eax) {
                printf("  Early recovery not supported on this drive. Ignoring.\n\n");
                SupressEarlyRecovery = true;
            }
        #endif
    }
    return eax;
}

/*******************************************************************************
 * GET NON-SENSE PAGE DATA
 *
 * Given Adapter, Device, DataPage, and a Buffer to receive the data, this
 * fills the buffer we're given and returns with the SCSI Completion Code
 *******************************************************************************/

long GetNonSenseData(short Device, short DataPage, void *Buffer, short BufLen) {
    char Scsi[6] = {0};
    Scsi[0] = SCSI_Cmd_NonSenseData; // do a Non-Sense Data Read
    Scsi[2] = DataPage; // which page to read
    Scsi[4] = BufLen; // tell drive page is this long
    return SCSICommand(Device, Scsi, Buffer, BufLen);
}

/*******************************************************************************
 * LOCK CURRENT DRIVE
 *******************************************************************************/
long LockCurrentDrive() {
    char Scsi[6] = {0};
    Scsi[0] = SCSI_Cmd_PreventAllow;
    Scsi[4] = 1; // set to ONE to  lock the drive
    return SCSICommand(CurrentDevice, Scsi, NULL, 0);
}

/*******************************************************************************
 * UNLOCK CURRENT DRIVE
 *******************************************************************************/
long UnlockCurrentDrive() {
    char Scsi[6] = {0};
    Scsi[0] = SCSI_Cmd_PreventAllow;
    return SCSICommand(CurrentDevice, Scsi, NULL, 0);
}

/*******************************************************************************
 * UNLOCK ALL MEDIA
 *******************************************************************************/
void UnlockAllMedia() {
    // make sure the media is not locked as we exit...
    char Scsi[6] = {0};
    Scsi[0] = SCSI_Cmd_PreventAllow;
    for(int i = 0; i < MAX_DRIVE_COUNT; i++) {
        SCSICommand(DriveArray[i].scsi_id, Scsi, NULL, 0);
    }
}

/*******************************************************************************
* SPIN UP IOMEGA CARTRIDGE
*******************************************************************************/
long SpinUpIomegaCartridge(short Device) {
   char Scsi[6] = {0};
   Scsi[0] = SCSI_Cmd_StartStopUnit;
   Scsi[1] = 1; // set the IMMED bit for offline
   Scsi[4] = 1; // start the disk spinning
   return SCSICommand(Device, Scsi, NULL, 0);
}

/*******************************************************************************
 * EJECT ALL MEDIA
 *******************************************************************************/
void EjectAllMedia() {
    // setup the SCSI command block for the operation
    for(int i = 0; i < MAX_DRIVE_COUNT; i++) {
        EjectIomegaCartridge(DriveArray[i].scsi_id);
    }
}

/*******************************************************************************
 * GET SPARE SECTOR COUNTS
 *
 * This returns NON-ZERO if we have trouble and posted the error message
 * into the RichText control, else it sets the number of spares available
 *******************************************************************************/

long GetSpareSectorCounts(char checkPassword) {
    DEFECT_LIST_HEADER DefectHeader;
    long eax = 0, ebx, edx;
    short ch, cl;
    ListChk:
    // ask for the defect list to make sure we're able to read it
    char Scsi[10] = {0};
    Scsi[0] = SCSI_Cmd_ReadDefectData;
    Scsi[2] = 0x1e; // 0b00011110 defect format, G/P bits
    Scsi[8] = 4; // ask for only FOUR bytes
    #ifdef SUPPRESS_DEFECTS_ERROR
        if(SupressDefectsError)
            eax = INCOMPATIBLE_MEDIA;
        else
    #endif
    eax = SCSICommand(CurrentDevice, Scsi, &DefectHeader, sizeof(DefectHeader));
    #ifdef SUPPRESS_DEFECTS_ERROR
        if(!SupressDefectsError && eax == INCOMPATIBLE_MEDIA) {
            printf("Defects list not supported on this drive. Ignoring.\n\n");
            SupressDefectsError = true;
        }
    #endif
    if ((!eax) || (eax == INCOMPATIBLE_MEDIA)) {
        // we could read its defect list ... so show it!
        // --------------------------------------------------------------------------
        // MLT: looks like on the Iomega Zip 100, the maximum size for DiskStat is 63
        // rather than 72; it looks like this code is causing a SCSI transfer error
        // here... might be better to conditionally check for Jaz drive
        char DiskStat[72];
        #ifdef NO_EXCESS_READS
            eax = GetNonSenseData(CurrentDevice, DISK_STATUS_PAGE, DiskStat, JazDrive ? sizeof(DiskStat) : 63);
            if (eax) return eax;
        #else
            eax = GetNonSenseData(CurrentDevice, DISK_STATUS_PAGE, DiskStat, sizeof(DiskStat));
            if (!eax) return eax;
        #endif
        // --------------------------------------------------------------------------
        ch = 0; // clear the DRIVE_A_SUPPORT
        if (JazDrive) {
            eax = GET_WORD_AT(DiskStat,  JAZ_SPARES_COUNT_OFFSET);
            ebx = 0;
            cl  = GET_BYTE_AT(DiskStat,  JAZ_PROTECT_MODE_OFFSET);
            edx = GET_DWORD_AT(DiskStat, JAZ_LAST_LBA_OFFSET);
        } else {
            if (DiskStat[0] == DISK_STATUS_PAGE) {
                eax = GET_WORD_AT(  DiskStat, NEW_ZIP_SIDE_0_SPARES_COUNT_OFFSET);
                ebx = GET_WORD_AT(  DiskStat, NEW_ZIP_SIDE_1_SPARES_COUNT_OFFSET);
                cl  = GET_BYTE_AT(  DiskStat, NEW_ZIP_PROTECT_MODE_OFFSET);
                edx = GET_DWORD_AT( DiskStat, NEW_ZIP_LAST_LBA_OFFSET);
                ch--; // set the DRIVE_A_SUPPORT
            }
            else {
                eax = GET_WORD_AT(  DiskStat, OLD_ZIP_SIDE_0_SPARES_COUNT_OFFSET);
                ebx = GET_WORD_AT(  DiskStat, OLD_ZIP_SIDE_1_SPARES_COUNT_OFFSET);
                cl  = GET_BYTE_AT(  DiskStat, OLD_ZIP_PROTECT_MODE_OFFSET);
                edx = GET_DWORD_AT( DiskStat, OLD_ZIP_LAST_LBA_OFFSET);
            }
            if(ebx == 0) {
                goto NoSpares;
            }
        }
        //---------------------------
        // bswap edx; save the last LBA in any event
        //---------------------------
        if(ch) {
            edx -= DRIVE_A_SUPPORT_BIAS;
        }
        LastLBAOnCartridge = edx;
        MAKE_LITTLE_ENDIAN(eax); // make it little endian
        Side_0_SparesCount = eax;
        MAKE_LITTLE_ENDIAN(ebx); // make it little endian
        Side_1_SparesCount = ebx;

        // compute the  number of troubles we encountered during the testing
        FirmErrors =  Initial_Side_0_Spares - Side_0_SparesCount;
        FirmErrors += Initial_Side_1_Spares - Side_1_SparesCount;
        // check to see whether we have ANY spare sectors remaining
        if(!Side_0_SparesCount && !Side_1_SparesCount) {
            NoSpares:
            CartridgeStatus = DISK_TEST_FAILURE;
            const char *eax = szNoSpares;
            // if were running give them a different error message
            if(TestingPhase) {
                eax = szOutOfSpares;
            }
            SetRichEditText(eax);
            goto ErrorExit;
        }

        // MLT: The code for removing the ZIP protection has been omitted
        return 0; // return zero since no erro
    }
    else {
        // trouble of some sort ... so suppress controls and
        // show the richedit control for the trouble
        if (eax == DEFECT_LIST_READ_ERROR) {
            CartridgeStatus = DISK_Z_TRACK_FAILURE;
            SetRichEditText(szDefectList);
            ErrorExit:
            return -1;
        }
        else if (eax == MEDIA_NOT_PRESENT) {
            CartridgeStatus = MEDIA_NOT_PRESENT;
        }
    }
    return eax;
}

/*******************************************************************************
 * HANDLE DRIVE CHANGING
 *
 * If we're NOT in the middle of drive testing, check for any new drives
 * going ready, eject all others, and Select the newer drive.
 * If we *ARE* in the middle of drive testing, EJECT any new drive that's
 * attempting to go ready.
 *******************************************************************************/
void HandleDriveChanging() {
    bool Selecting = false; // true while we're changing selections
    uint8_t status;
Rescan:
    for(int i = 0; i < DriveCount; i++) {
        const int scsi_id = DriveArray[i].scsi_id;

        // query the current state of the drive
        do {
            // clear media changed status
            DriveArray[i].flags &= ~MEDIA_CHANGED;
            GetCartridgeStatus(scsi_id, DriveArray[i].flags);
        } while(DriveArray[i].flags & MEDIA_CHANGED); // do it until NO media change!
        //--------------------------------------------------------------------------
        status = GetCartridgeStatus(scsi_id, DriveArray[i].flags);
        if (status == DISK_STATUS_UNKNOWN) continue; // added by MLT
        // if the device we have is NOT the currently selected one
        if(scsi_id != CurrentDevice) {
            // if the disk is ANYTHING other than not present ...
            if(status != DISK_NOT_PRESENT) {
                // we have a PRESENT DISK in a non-current drive
                // if we're testing, reject it
                if (Selecting || TestingPhase >= TESTING_STARTUP) {
                    EjectIomegaCartridge(scsi_id);
                    // flag that we're waiting for spindown
                    DriveArray[i].flags |= DISK_EJECTING;
                }
                // if we're not testing, and not awaiting eject
                // then set the current drive ...
                else if ((DriveArray[i].flags & DISK_EJECTING) == 0) {
                    CurrentDevice = scsi_id;
                    printf("Selected SCSI ID %ld\n", CurrentDevice);
                    TestingPhase = 0;
                    Selecting = true;
                    //goto Rescan;
                    break;
                    // the PREVIOUS drive (if any) will be ejected on the next pass
                }
            } else {
                // the drive HAS spun down, so clear "waiting"
                DriveArray[i].flags &= ~DISK_EJECTING;
            }
        } else {
            // we're checking the current drive ... make SURE that
            // it is *NOT* empty! If it *IS* empty, kill current
            if(status == DISK_NOT_PRESENT) {
                CurrentDevice = -1;
                SetCartridgeStatusToEAX(status, DriveArray[i].flags);
            }
            // if it's not already set correctly *and* either
            // the cart status is one of the pre-test ones, or
            // the NEW status from the cart is NOT "at speed" ...
            if((status != CartridgeStatus) && ((CartridgeStatus <= DISK_STALLED) || (status != DISK_AT_SPEED))) {
                SetCartridgeStatusToEAX(status, DriveArray[i].flags);
            }
        }
    }
    // if nothing was chosen ... set us to the "Awaiting Cartridge" status
    if ((CurrentDevice == -1) &&
        (status == DISK_NOT_PRESENT) &&
        (CartridgeStatus != DISK_NOT_PRESENT)) {
            SetCartridgeStatusToEAX(status, 0);
    }
}

//-----------------------------------------------------------------------------
// GET CARTRIDGE STATUS
//-----------------------------------------------------------------------------
uint8_t GetCartridgeStatus(long Device, uint8_t flags) {
    long eax;
    char DiskStat[72];
    #ifdef NO_EXCESS_READS
        eax = GetNonSenseData(Device, DISK_STATUS_PAGE, DiskStat, 4);
        if (eax) return DISK_STATUS_UNKNOWN;
    #else
        eax = GetNonSenseData(Device, DISK_STATUS_PAGE, DiskStat, sizeof(DiskStat));
        if (!eax) return DISK_STATUS_UNKNOWN;
    #endif
    if (DiskStat[0] == DISK_STATUS_PAGE) {
        return DiskStat[NEW_DISK_STATUS_OFFSET];
    } else {
        return DiskStat[OLD_DISK_STATUS_OFFSET];
    }
}

//-----------------------------------------------------------------------------
// SetCartridgeStatusToEAX
//-----------------------------------------------------------------------------

void SetCartridgeStatusToEAX(long eax, uint8_t flags) {
    JazDrive = flags & JAZ_DRIVE;

    long PriorStatus = CartridgeStatus;
    CartridgeStatus = eax;

    // Set the text of the "action initiate button"
    const char *esi = 0;
    switch (CartridgeStatus) {
        case DISK_SPUN_DOWN:
            // set the button to "Start Disk Spinning"
            esi = szPressToSpin;
            EnableWindow(hTestButton, true);
            break;
        case DISK_TEST_UNDERWAY:
            // set the button to "Stop Testing"
            esi = szPressToStop;
            break;
        case DISK_NOT_PRESENT:
            SetRichEditText(szNotRunning);
            goto DisableActions;
        case DISK_AT_SPEED:
            printf("Disk at speed\n");
            eax = GetSpareSectorCounts(true); // update the Cart Condition
            if(eax == MEDIA_NOT_PRESENT) {
                goto DisableActions;
            }
            //TroubleFlag = eax; // decide whether we're in trouble
            esi = szPressToEject; // presume trouble
            if(!eax) {
                Initial_Side_0_Spares = Side_0_SparesCount;
                Initial_Side_1_Spares = Side_1_SparesCount;
                FirmErrors = 0;
                // check to see if we have enough spares to start
                if(JazDrive) {
                    printf("Spare Sectors: %ld/%d\n", Side_0_SparesCount, MAXIMUM_JAZ_SPARES);
                    if(Side_0_SparesCount < MINIMUM_JAZ_SPARES)
                        goto InsufficientSpares;
                }
                else {
                    printf("Spare Sectors:\n");
                    printf("  Side 1: %ld/%d\n", Side_0_SparesCount, MAXIMUM_ZIP_SPARES);
                    printf("  Side 2: %ld/%d\n", Side_1_SparesCount, MAXIMUM_ZIP_SPARES);
                    if(Side_0_SparesCount < MINIMUM_ZIP_SPARES) {
                        goto InsufficientSpares;
                    }
                    if(Side_1_SparesCount < MINIMUM_ZIP_SPARES) {
                    InsufficientSpares:
                        SetRichEditText(szFewSpares);
                        CartridgeStatus = DISK_LOW_SPARES;
                        esi = szPressToProceed;
                        goto EnableTestBtn;
                    }
                }
                // if no trouble, get ready to start testing...
                PrepareToBeginTesting();
                esi = szPressToStart;
            }
            // The disk *IS* at speed so enable the action button!
            EnableTestBtn:
            EnableWindow(hTestButton, true);
            esi = szPressToStart;
            break;
        default:
            // set the button to "One Moment Please"
            DisableActions:
            EnableWindow(hTestButton, false);
            esi = szOneMoment;
    }
    // set the Window's text based upon setting of esi
    SetWindowText(hTestButton, esi);
    // based upon the TroubleFlag, show them the proper page set
    // SetTabErrorMode(TroubleFlag)
    // and if CartridgeStatus has changed, refresh the entire panel!
    bool ecx = false;
    if((PriorStatus == DISK_AT_SPEED) ||
       (PriorStatus == DISK_SPUN_DOWN)||
       (PriorStatus >= DISK_LOW_SPARES)) {
       ecx = !ecx;
    }
    if((CartridgeStatus == DISK_AT_SPEED) ||
       (CartridgeStatus >= DISK_LOW_SPARES)) {
        ecx = !ecx;
    }
    if (ecx) { // update the entire panel's data
        InvalidateRect(hTestMonitor);
    } else { // only paint the new cartridge status
        GetDC(hTestMonitor);
        PaintCartStatus();
        ReleaseDC(hTestMonitor);
    }
}

/*******************************************************************************
 * GET ELAPSED TIME IN SECONDS
 *******************************************************************************/
long GetElapsedTimeInSeconds() {
    return GetSystemTime() - StartingInstant;
}

/*******************************************************************************
 * PREPARE TO BEGIN TESTING
 *******************************************************************************/
void PrepareToBeginTesting() {
    // Zero all of the testing variables
    TestingPhase              = 0; // 0 = not testing, no data ...
    PercentComplete           = 0;
    FirstLBASector            = 0;
    NumberOfLBAs              = 0;
    SoftErrors                = 0;
    FirmErrors                = 0;
    HardErrors                = 0;
    UserInterrupt             = 0;
    LastError                 = 0;
    #ifdef SUPRESS_ER_ERRORS
        SupressEarlyRecovery = false;
    #endif
    #ifdef SUPPRESS_DEFECTS_ERROR
        SupressDefectsError = false;
    #endif
    #ifdef DEMO
        LastLBAOnCartridge        = 99999;
        SoftErrors                = 6;
        FirmErrors                = 2;
        HardErrors                = 1;
        UserInterrupt             = 0;
        LastError                 = 0x0C8001;
        Side_0_SparesCount        = 12;
        Side_1_SparesCount        = 20;
    #endif
}

/*******************************************************************************
 * BUMP ERROR COUNTS
 *
 * See: https://en.wikipedia.org/wiki/Key_Code_Qualifier
 *******************************************************************************/
void BumpErrorCounts(long ErrorCode) {
    long eax = ErrorCode;
    if (eax == BUFFER_TOO_BIG) { // if we got BUFFER TOO BIG, halt!
        UserInterrupt = 1;
    }
    long ebx = eax & 0x00FF00FF; // mask off the middle byte
    if (ebx == 0x00150004) // if it was one of the many seek
        eax = ebx; // errors, cvrt to seek error
    if (eax)
        LastError = eax;
    if (eax == 0x320003 || eax == 0x328F03)
        CartridgeStatus = DISK_LOW_SPARES;
    if ((eax & 0xFF) == 1) // recovered error
        SoftErrors++;
    else
        HardErrors++;
}

/*******************************************************************************
 * EJECT IOMEGA CARTRIDGE
 *******************************************************************************/
void EjectIomegaCartridge(int Device) {
    // Could NOT do it through the IOCTL layer ... so eject with SCSI
    // make sure the media is not locked...
    char Scsi[6] = {0};
    Scsi[0] = SCSI_Cmd_PreventAllow;
    SCSICommand(Device, Scsi, 0, 0);
    // issue an Asynchronous STOP command to induce spindown and ejection
    memset(Scsi, 0, sizeof(Scsi));
    Scsi[0] = SCSI_Cmd_StartStopUnit;
    Scsi[1] = 1; // Set the IMMED bit for offline
    Scsi[4] = 2; // eject a Jaz disk after stopping
    SCSICommand(Device, Scsi, 0, 0);
}

/*******************************************************************************
 * PERFORM REGION TRANSFER
 *******************************************************************************/
long PerformRegionTransfer(short XferCmd, void *pBuffer) {
    char Scsi[10] = {0}; // clear out the SCSI CDB
    const long InitialHardErrors = HardErrors;

    long eax = SetErrorRecovery(false, false, true); // disable Retries & ECC

    Scsi[0] = XferCmd;
    SET_DWORD_AT(Scsi, 2, MAKE_BIG_ENDIAN(FirstLBASector)); // WHICH LBA's to read, BIG endian
    SET_WORD_AT (Scsi, 7, MAKE_BIG_ENDIAN(NumberOfLBAs));   // HOW MANY to read, BIG endian
    eax = SCSICommand(CurrentDevice, Scsi, pBuffer, NumberOfLBAs * BYTES_PER_SECTOR);
    // if we failed somewhere during our transfer ... let's zero in on it
    if (eax) {
        if ( eax == SS_ERR || // if it's a CONTROLLER ERROR, skip!
             eax == BUFFER_TOO_BIG ||
             eax == LBA_TOO_LARGE) {
            goto Exit;
        }

        printf("Starting detailed search...\n");

        //--------------------------------------------------------------------------
        // Save error and current Soft + Hard Error count to see if we do FIND the glitch ...
        const long GlitchError = eax; // save the error which stopped us!
        const long GlitchCount = SoftErrors + HardErrors;
        char *LocalBuffer = (char*) pBuffer;
        ErrorSound();

        SingleTransferLBA = FirstLBASector;

        // Perform transfer LBA block at a time
        for(long i = 0; i < NumberOfLBAs; ++i) {
            UpdateCurrentSector();

            // setup for our series of transfer tests ...

            // disable all recovery techniques
            SetErrorRecovery(false, false, true); // disable Retries & ECC

            memset(Scsi, 0, sizeof(Scsi)); // clear out the SCSI CDB
            Scsi[0] = XferCmd;
            SET_DWORD_AT(Scsi, 2, MAKE_BIG_ENDIAN(SingleTransferLBA)); // WHICH LBA to read, BIG endian
            SET_WORD_AT (Scsi, 7, MAKE_BIG_ENDIAN(1));                 // a single sector
            eax = SCSICommand(CurrentDevice, Scsi, LocalBuffer, BYTES_PER_SECTOR);

            if (eax) {
                // some sort of problem encountered!
                if (eax == SS_ERR) goto Exit; // if it's a CONTROLLER ERROR, skip!
                if (eax & 0xFF == 1) goto PostTheError; // did we recover?

                printf("  Found error, retesting with retries\n");
                SetErrorRecovery(true, false, true); // enable retries
                eax = SCSICommand(CurrentDevice, Scsi, LocalBuffer, BYTES_PER_SECTOR);
                if (eax) {
                    // failed with retries
                    if (eax == SS_ERR) goto Exit; // if it's a CONTROLLER ERROR, skip!
                    if (eax & 0xFF == 1) goto PostTheError; // did we recover?

                    printf("  Found error, retesting with retries & ECC\n");
                    eax = SetErrorRecovery(true, true, true); // enable retries AND EEC
                    eax = SCSICommand(CurrentDevice, Scsi, LocalBuffer, BYTES_PER_SECTOR);
                    if (eax) {
                        // failed with retries and EEC
                        if (eax == SS_ERR) goto Exit; // if it's a CONTROLLER ERROR, skip!
                        if (eax & 0xFF == 1) goto PostTheError; // did we recover?
                    }
                    else { // succeeded with ECC
                        eax = 0x180101; // "ECC & Retries"
                    }
                } // succeeded with retries
                else {
                    eax = 0x170101; // "Read with Retries"
                    if (XferCmd == SCSI_Cmd_WriteMany)
                        eax = 0x0C8001; // "Wrote with Retries"
                }

            PostTheError:
                printf("  %s (Sector %ld)\n", FindErrorString(eax), SingleTransferLBA);
                printf("--------------------------------------------\n");
                BumpErrorCounts(eax); // given eax, count the errors
                GetSpareSectorCounts(false); // update the Cart's Condition
                UpdateRunTimeDisplay();
            }
            LocalBuffer += BYTES_PER_SECTOR;
            SingleTransferLBA++;
            ProcessPendingMessages();
        }

        printf("... detailed search finished\n");
        // now see whether we *did* found something to complain about ...
        eax = SoftErrors + HardErrors;
        if (eax == GlitchCount) {
            // we missed it ... but SOMETHING happened!  So let's report it ...
            const long SavedSoftErrors = SoftErrors; // save the existing counts
            const long SavedHardErrors = HardErrors;
            eax = GlitchError; // get the error that triggered our search
            long ebx = eax & 0x00FF00FF; // strip the ASCQ byte
            if(ebx == 0x00110003) // if we're about to say "unrecovered read"
                eax = 0x170101; // change it to: "Read with Retries"
            printf("%s\n", FindErrorString(eax));
            BumpErrorCounts(eax); // given eax, count the errors
            HardErrors = SavedHardErrors; // restore the counts
            SoftErrors = SavedSoftErrors;
            SoftErrors++;
            UpdateRunTimeDisplay();
        }
        SingleTransferLBA = 0;
        eax = 0; // now let's return happiness to our caller
        if (HardErrors != InitialHardErrors) // UNRECOVERABLE errors!
            eax = -1;
        printf("\n");
    }

Exit:
    SetErrorRecovery(true, true, false); // reenable Retries & ECC
    return eax;
}

/*******************************************************************************
 * TEST THE DISK
 *******************************************************************************/

void TestTheDisk() {
    // setup the initital maximum tranfer ...
    AdapterMaxSectors = MAX_SECTORS_PER_TEST; // limit to our max

    void *pDataBuffer = 0;
    for(;;) {
        pDataBuffer = malloc(AdapterMaxSectors * BYTES_PER_SECTOR * 2);
        if(pDataBuffer) break;
        AdapterMaxSectors >>= 2; // we need to make it smaller
        if(AdapterMaxSectors == 0) {
            printf("Test buffer allocation failed!\n");
            return;
        }
    }

    printf("Allocated buffer of %ld bytes\n", AdapterMaxSectors * BYTES_PER_SECTOR);

    void *pPatternBuffer  = pDataBuffer;
    void *pUserDataBuffer = (char*) pDataBuffer + AdapterMaxSectors * BYTES_PER_SECTOR;

    StopApplicationTimer();

    PreventProgramExit();
    SetRichEditText(szRunning);
    CartridgeStatus = DISK_TEST_UNDERWAY;
    TestingPhase = TESTING_STARTUP; // inhibit stopping now
    SetWindowText(hTestButton, szPressToStop);
    InvalidateRect(hTestMonitor);

    LockCurrentDrive(); // prevent media removal
    GetSpareSectorCounts(false); // update the Cart's Condition
    UpdateRunTimeDisplay();

    // Standard Testing Operation
    StartingInstant = GetSystemTime();
    long eax;

    do {
        ProcessPendingMessages();

        NumberOfLBAs = AdapterMaxSectors;

        if(LastLBAOnCartridge) {
            if (FirstLBASector + NumberOfLBAs > LastLBAOnCartridge + 1) {
                NumberOfLBAs = LastLBAOnCartridge - FirstLBASector + 1;
            }
            // compute the percentage complete
            PercentComplete = FirstLBASector * 100 / LastLBAOnCartridge;
        }

        if(NumberOfLBAs == 0) break;

        // uppdate the elapsed time
        SecondsElapsed = GetElapsedTimeInSeconds();

        // get a random pattern of data to write
        const long DataPattern = rand();
        memset(pPatternBuffer, DataPattern, AdapterMaxSectors * BYTES_PER_SECTOR);

        // update the cartridge's status
        GetSpareSectorCounts(false); // update the Cart's Condition

        TestingPhase = READING_DATA;

        UpdateRunTimeDisplay();

        eax = PerformRegionTransfer(SCSI_Cmd_ReadMany, pUserDataBuffer);

        if(eax == 0) {
            // -------------------------------
            TestingPhase = WRITING_PATT;
            UpdateRunPhaseDisplay();
            PerformRegionTransfer(SCSI_Cmd_WriteMany, pPatternBuffer);
            // -------------------------------
            TestingPhase = READING_PATT;
            UpdateRunPhaseDisplay();
            PerformRegionTransfer(SCSI_Cmd_Verify, pPatternBuffer);
            // -------------------------------
            TestingPhase = WRITING_DATA;
            UpdateRunPhaseDisplay();
            PerformRegionTransfer(SCSI_Cmd_WriteMany, pUserDataBuffer);
        }
        else if (eax == LBA_TOO_LARGE) {
            // if we hit the end of the disk ... exit gracefully!
            goto GetOut;
        }
        else if (eax == SS_ERR) {
            // added by MLT, exit on controller errors
            goto GetOut;
        }
        if (CartridgeStatus != DISK_TEST_UNDERWAY) {
            break;
        }
        // bump the FirstLBASector up for the next transfer
        FirstLBASector += NumberOfLBAs;
    } while(!UserInterrupt);
    // show that we're post-test

GetOut:
    free(pDataBuffer);

    TestingPhase = UNTESTED;
    UnlockAllMedia();
    SetErrorRecovery(true, true, false); // reenable Retries & ECC
    SetWindowText(hTestButton, szPressToStart);
    CartridgeStatus = DISK_AT_SPEED;
    AllowProgramExit();

    // compute the number of serious troubles
    const char *result;
    long errors = FirmErrors + HardErrors;
    if (errors >= BADNESS_THRESHOLD) {
        result = szBadResult;
    }
    else if (UserInterrupt || (eax == SS_ERR)) {
        result = szInterrupted;
    }
    else {
        // it wasn't interrupted, nor seriously bad, was it perfect?
        errors += SoftErrors;
        if(errors) {
            result = szExplainResult;
        } else {
            result = szPerfectResult;
        }
    }
    SetRichEditText(result);
    InvalidateRect(hTestMonitor);
    Exit:
    StartApplicationTimer();
}
