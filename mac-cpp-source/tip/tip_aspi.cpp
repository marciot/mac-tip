#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "mac_scsi.h"
#include "tip.h"

//#define DEMO
#define NO_EXCESS_READS

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

//-------------------------- Drive Array Status Flags ---------------------------

#define JAZ_DRIVE             0x00010000
#define MEDIA_CHANGED         0x00020000
#define DISK_EJECTING         0x00040000 // we've asked for eject and waiting ...
#define ODD_BYTE_COMPENSATION 0x00080000 // special handling for ODD length PSWD
#define MAX_DRIVE_COUNT       16         // we can handle up to 16 Zip/Jaz drives

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

#define BYTES_PER_SECTOR 512
#define MAX_SECTORS_PER_TEST 25

#define BADNESS_THRESHOLD 10

#define SS_ERR                               0x00000004 // From disassembly
#define BUFFER_TOO_BIG                       0x00FFFFE6 // From disassembly
#define LBA_TOO_LARGE                        0x00210005 // From disassembly
#define INCOMPATIBLE_MEDIA                   0x00300002 // From disassembly
#define MEDIA_NOT_PRESENT                    0x003a0002 // From disassembly
#define DEFECT_LIST_READ_ERROR               0x001c0003 // From disassembly

#define CHECK_CONDITION 0x02

enum {
    szBadResult,
    szInterrupted,
    szExplainResult,
    szPerfectResult,
    szNoSpares,
    szOutOfSpares
};

long CurrentDevice = 0;
long DriveCount = 0;

long JazDrive = 0; // true if the current drive
long CartridgeStatus = DISK_NOT_PRESENT;

unsigned long StartingInstant;

// ----------------------------- Run Time Variables ------------------------------

long Side_0_SparesCount; // JAZ has only one count
long Side_1_SparesCount; // ZIP has counts for both sides
long Initial_Side_0_Spares;
long Initial_Side_1_Spares;

long TestingPhase; // 0 = not testing, no data ...
long PercentComplete;
long FirstLBASector;
long NumberOfLBAs;
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
long SCSICommand(short Device, char *lpCmdBlk, void *lpIoBuf, short IoBufLen) {
    char cmd_length, cmd_flags, cmd_status;
    GetCommandDetails(lpCmdBlk[0], cmd_flags, cmd_length);
    // call the SCSI interface to forward the command to the device
    OSErr err = scsi_cmd(Device, lpCmdBlk, cmd_length, lpIoBuf, IoBufLen, 0, cmd_flags, &cmd_status);
    if(err != noErr) {
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
        return (long(sense_data.asc) << 16) |
               (long(sense_data.ascq) << 8) |
               (long(sense_data.key)      );
    }
    else {
        // else, if it's *NOT* a "Sense Data" error (SS_ERR)
        return cmd_status | 0x00FFFF00; // [00 FF FF er]
    }
}

/*******************************************************************************
 * ENUMERATE IOMEGA DEVICES
 *******************************************************************************/
void EnumerateIomegaDevices(long Device) {
    DriveCount = 0;
    JazDrive = 0;
    //-----------------------------------------------------------
    char InqData[96];
    char Scsi[6] = {0};
    Scsi[0] = SCSI_Cmd_Inquiry;
    Scsi[4] = sizeof(InqData);
    long eax = SCSICommand(Device, Scsi, InqData, sizeof(InqData));
    if(eax) goto TryNextDrive;
    //-----------------------------------------------------------
    InqData[14] = '\0';
    if (strcmp(szIomega, InqData + 8)) goto TryNextDrive;
    //-----------------------------------------------------------
    InqData[19] = '\0';
    if (!strcmp(szZip, InqData + 16)) goto FoundZipOrJaz;
    //-----------------------------------------------------------
    if (strcmp(szJaz, InqData + 16)) goto TryNextDrive;
    JazDrive = 1;
    FoundZipOrJaz:
    DriveCount = 1;
    if(JazDrive)
        printf("Found Jaz drive\n");
    else
        printf("Found Zip drive\n");
TryNextDrive:
    return;
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
long SetModePage(short Device, void *pBuffer) {
    char* ebx = (char*) pBuffer; // get a pointer to the top of buffer
    char ecx = ebx[0] + 1; // adjust it up by one

    ebx[0] = 0; // now clear the two reserved bytes
    ebx[2] = 0;

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

void SetErrorRecovery(bool Retries, bool ECC, bool Testing) {
    char PageBuff[40];

    #ifdef NO_EXCESS_READS
        // Limit reads to 20 bytes on Zip to prevent controller errors
        GetModePage(CurrentDevice, ERROR_RECOVERY_PAGE, PageBuff, JazDrive ? sizeof(PageBuff) : 20);
    #else
        GetModePage(CurrentDevice, ERROR_RECOVERY_PAGE, PageBuff, sizeof(PageBuff));
    #endif

    #define EARLY_RECOVERY 0x08
    #define PER            0x04
    #define SUPPRESS_ECC   0x01

    // set the ECC fields
    char ecc = SUPPRESS_ECC; // presume ECC suppression
    if(ECC) {
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
    const long eax = SetModePage(CurrentDevice, PageBuff);
    // if we had an invalid field in the CDB (the EER bit was on)
    if (eax == 0x00260005) {
        GetModePage(CurrentDevice, ERROR_RECOVERY_PAGE, PageBuff, sizeof(PageBuff));
        ecc &= ~0x08; // same, *BUT*NOT* Early Recovery
        ModifyModePage(PageBuff, ecc, retries);
        SetModePage(CurrentDevice, PageBuff);
    }
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
    eax = SCSICommand(CurrentDevice, Scsi, &DefectHeader, sizeof(DefectHeader));
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
            eax = szNoSpares;
            // if were running give them a different error message
            if(TestingPhase) {
                eax = szOutOfSpares;
            }
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
 *******************************************************************************/
void HandleDriveChanging() {
    // MLT: At the moment, we only handle one drive.
    long eax;
    char DiskStat[72];
    #ifdef NO_EXCESS_READS
        eax = GetNonSenseData(CurrentDevice, DISK_STATUS_PAGE, DiskStat, JazDrive ? sizeof(DiskStat) : 63);
        if (eax) return;
    #else
        eax = GetNonSenseData(CurrentDevice, DISK_STATUS_PAGE, DiskStat, sizeof(DiskStat));
        if (!eax) return;
    #endif
    char status;
    if (DiskStat[0] == DISK_STATUS_PAGE) {
        status = DiskStat[NEW_DISK_STATUS_OFFSET];
    } else {
        status = DiskStat[OLD_DISK_STATUS_OFFSET];
    }
    // we're checking the current drive ... make SURE that
    // it is *NOT* empty! If it *IS* empty, kill current
    if(status == DISK_NOT_PRESENT) {
        SetCartridgeStatusToEAX(status);
    }
    // if it's not already set correctly *and* either
    // the cart status is one of the pre-test ones, or
    // the NEW status from the cart is NOT "at speed" ...
    if((status != CartridgeStatus) && ((CartridgeStatus <= DISK_STALLED) || (status != DISK_AT_SPEED))) {
        SetCartridgeStatusToEAX(status);
    }
}

//-----------------------------------------------------------------------------
// SetCartridgeStatusToEAX
//-----------------------------------------------------------------------------

void SetCartridgeStatusToEAX(long eax) {
    long PriorStatus = CartridgeStatus;
    CartridgeStatus = eax;

    // Set the text of the "action initiate button"
    const char *esi = 0;
    switch (CartridgeStatus) {
        case DISK_SPUN_DOWN:
            // set the button to "Start Disk Spinning"
            esi = szPressToSpin;
            break;
        case DISK_TEST_UNDERWAY:
            // set the button to "Stop Testing"
            esi = szPressToStop;
            break;
        case DISK_NOT_PRESENT:
            goto DisableActions;
        case DISK_AT_SPEED:
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
                    if(Side_0_SparesCount < MINIMUM_JAZ_SPARES)
                        goto InsufficientSpares;
                }
                else {
                    if(Side_0_SparesCount < MINIMUM_ZIP_SPARES) {
                        goto InsufficientSpares;
                    }
                    if(Side_1_SparesCount < MINIMUM_ZIP_SPARES) {
                    InsufficientSpares:
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
    if (eax & 0xFF == 1) // recovered error
        SoftErrors++;
    else
        HardErrors++;
}

/*******************************************************************************
 * PERFORM REGION TRANSFER
 *******************************************************************************/
long PerformRegionTransfer(short XferCmd, void *pBuffer) {
    char Scsi[10] = {0}; // clear out the SCSI CDB
    const long InitialHardErrors = HardErrors;

    SetErrorRecovery(false, false, true); // disable Retries & ECC

    Scsi[0] = XferCmd;
    SET_DWORD_AT(Scsi, 2, MAKE_BIG_ENDIAN(FirstLBASector)); // WHICH LBA's to read, BIG endian
    SET_WORD_AT (Scsi, 7, MAKE_BIG_ENDIAN(NumberOfLBAs));   // HOW MANY to read, BIG endian
    long eax = SCSICommand(CurrentDevice, Scsi, pBuffer, NumberOfLBAs * BYTES_PER_SECTOR);
    // if we failed somewhere during our transfer ... let's zero in on it
    if (eax) {
        printf("Error during transfer %lx (lba: %ld)\n", eax, FirstLBASector);

        if ( eax == SS_ERR || // if it's a CONTROLLER ERROR, skip!
             eax == BUFFER_TOO_BIG ||
             eax == LBA_TOO_LARGE) {
            goto Exit;
        }

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

                SetErrorRecovery(true, false, true); // enable retries
                eax = SCSICommand(CurrentDevice, Scsi, LocalBuffer, BYTES_PER_SECTOR);
                if (eax) {
                    // failed with retries
                    if (eax == SS_ERR) goto Exit; // if it's a CONTROLLER ERROR, skip!
                    if (eax & 0xFF == 1) goto PostTheError; // did we recover?

                    SetErrorRecovery(true, true, true); // enable retries AND EEC
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
                BumpErrorCounts(eax); // given eax, count the errors
                GetSpareSectorCounts(false); // update the Cart's Condition
                UpdateRunTimeDisplay();

                if(UserInterrupt) break;
            }
            LocalBuffer += BYTES_PER_SECTOR;
            SingleTransferLBA++;
            ProcessPendingMessages();
        }

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
    }

Exit:
    SetErrorRecovery(true, true, false); // reenable Retries & ECC
    return eax;
}

/*******************************************************************************
 * TEST THE DISK
 *******************************************************************************/

long TestTheDisk() {
    void *pPatternBuffer  = malloc(MAX_SECTORS_PER_TEST * BYTES_PER_SECTOR);
    void *pUserDataBuffer = malloc(MAX_SECTORS_PER_TEST * BYTES_PER_SECTOR);

    if(pPatternBuffer == NULL || pUserDataBuffer == NULL) {
        printf("Allocation error\n");
        return -1;
    }

    StopApplicationTimer();

    PreventProgramExit();
    CartridgeStatus = DISK_TEST_UNDERWAY;
    TestingPhase = TESTING_STARTUP; // inhibit stopping now
    SetWindowText(hTestButton, szPressToStop);
    InvalidateRect(hTestMonitor);

    LockCurrentDrive(); // prevent media removal

    GetSpareSectorCounts(false); // update the Cart's Condition
    UpdateRunTimeDisplay();

    // Standard Testing Operation
    StartingInstant = GetSystemTime();

    do {
        ProcessPendingMessages();

        NumberOfLBAs = MAX_SECTORS_PER_TEST;

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
        memset(pPatternBuffer, DataPattern, MAX_SECTORS_PER_TEST * BYTES_PER_SECTOR);

        // update the cartridge's status
        GetSpareSectorCounts(false); // update the Cart's Condition

        TestingPhase = READING_DATA;

        UpdateRunTimeDisplay();

        long eax = PerformRegionTransfer(SCSI_Cmd_ReadMany, pUserDataBuffer);

        /*if(eax == 0) {
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
            PerformRegionTransfer(SCSI_Cmd_Verify, pUserDataBuffer);
        }
        else if (eax == LBA_TOO_LARGE) {
            // if we hit the end of the disk ... exit gracefully!
            goto GetOut;
        }
        if (CartridgeStatus != DISK_TEST_UNDERWAY) {
            break;
        }*/
        // bump the FirstLBASector up for the next transfer
        FirstLBASector += NumberOfLBAs;
    } while(!UserInterrupt);
    // show that we're post-test

GetOut:
    free(pPatternBuffer);
    free(pUserDataBuffer);

    TestingPhase = UNTESTED;
    UnlockCurrentDrive();
    SetErrorRecovery(true, true, false); // reenable Retries & ECC
    SetWindowText(hTestButton, szPressToStart);
    CartridgeStatus = DISK_AT_SPEED;
    AllowProgramExit();

    // compute the number of serious troubles
    long eax, errors = FirmErrors + HardErrors;
    if (errors >= BADNESS_THRESHOLD) {
        eax = szBadResult;
    }
    else if (UserInterrupt) {
        eax = szInterrupted;
    }
    else {
        // it wasn't interrupted, nor seriously bad, was it perfect?
        errors += SoftErrors;
        if(errors) {
            eax = szExplainResult;
        } else {
            eax = szPerfectResult;
        }
    }

    InvalidateRect(hTestMonitor);
    StartApplicationTimer();
    return eax;
}
