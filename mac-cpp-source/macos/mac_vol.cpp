#include <stdio.h>
#include <Files.h>
#include "mac_vol.h"
#include "pstring.h"

void mac_list_volumes() {
    HParamBlockRec paramBlock;
    Str255 volName;

    paramBlock.volumeParam.ioCompletion = 0;
    paramBlock.volumeParam.ioNamePtr = volName;
    paramBlock.volumeParam.ioVRefNum = 0;
    paramBlock.volumeParam.ioVolIndex = 0;
    for (;;) {
        OSErr err = PBHGetVInfo(&paramBlock, false);
        if (err == nsvErr) break;
        printf("   %d: %#s\n", paramBlock.volumeParam.ioVolIndex, paramBlock.volumeParam.ioNamePtr);
        paramBlock.volumeParam.ioVolIndex++;
    }
}

OSErr mac_get_drive_volumes(int driveNum, Str255 str) {
    HParamBlockRec paramBlock;
    Str255 volName;
    Boolean first = true;

    paramBlock.volumeParam.ioCompletion = 0;
    paramBlock.volumeParam.ioNamePtr = volName;
    paramBlock.volumeParam.ioVRefNum = 0;
    paramBlock.volumeParam.ioVolIndex = 0;
    for (;;) {
        OSErr err = PBHGetVInfo(&paramBlock, false);
        if (err == nsvErr) return noErr;
        if (err != noErr) return err;
        if (paramBlock.volumeParam.ioVDrvInfo == driveNum) {
            if (!first) pstrcat(str, "\p, ");
            pstrcat(str, volName);
            first = false;
        }
        paramBlock.volumeParam.ioVolIndex++;
    }
}

OSErr mac_unmount_drive(int driveNum) {
    HParamBlockRec paramBlock;

    paramBlock.volumeParam.ioCompletion = 0;
    paramBlock.volumeParam.ioNamePtr = 0;
    paramBlock.volumeParam.ioVRefNum = 0;
    paramBlock.volumeParam.ioVolIndex = 0;

    for (;;) {
        OSErr err = PBHGetVInfo(&paramBlock, false);
        if (err == nsvErr) return noErr;
        if (err != noErr) return err;
        if (paramBlock.volumeParam.ioVDrvInfo == driveNum) {
            // We found a drive to unmount
            err = UnmountVol(0, paramBlock.volumeParam.ioVRefNum);
            if (err != noErr) return err;
            // Continue searching from the top of the list
            paramBlock.volumeParam.ioVolIndex = 0;
            continue;
        }
        paramBlock.volumeParam.ioVolIndex++;
    }
}

void mac_unmount(int id) {
    HParamBlockRec paramBlock;
    paramBlock.volumeParam.ioCompletion = 0;
    paramBlock.volumeParam.ioNamePtr = 0;
    paramBlock.volumeParam.ioVRefNum = 0;
    paramBlock.volumeParam.ioVolIndex = id;
    OSErr err = PBHGetVInfo(&paramBlock, false);
    if (err == nsvErr) {
        printf("No such volume\n");
        return;
    }
    err = UnmountVol(0, paramBlock.volumeParam.ioVRefNum);
    switch (err) {
        case noErr: printf("Okay\n"); break;
        case fBsyErr: printf("One or more files are open\n"); break;
        default: printf("Failed %d\n", err);
    }
}

void mac_eject(int id) {
    HParamBlockRec paramBlock;
    paramBlock.volumeParam.ioCompletion = 0;
    paramBlock.volumeParam.ioNamePtr = 0;
    paramBlock.volumeParam.ioVRefNum = 0;
    paramBlock.volumeParam.ioVolIndex = id;
    OSErr err = PBHGetVInfo(&paramBlock, false);
    if (err == nsvErr) {
        printf("No such volume\n");
        return;
    }
    err = Eject(0, paramBlock.volumeParam.ioVRefNum);
    switch (err) {
        case noErr: printf("Okay\n"); break;
        case fBsyErr: printf("One or more files are open\n"); break;
        default: printf("Failed %d\n", err);
    }
}