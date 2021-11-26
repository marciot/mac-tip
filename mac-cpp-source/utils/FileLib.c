/*
 * File Utilities (System 7 and above)
 *
 * by Thomas Tempelmann, macdev@tempel.org
 */


#include <Types.h>
#include <Files.h>
#include <Errors.h>
#include <Devices.h>
#include <Aliases.h>
#include <Resources.h>
#include <Folders.h>
//#include <LowMem.h>

#define CALL_NOT_IN_CARBON 1    // for Universal Headers 3.3
#include <FSM.h>

#include "FileLib.h"
#include "TrapAvail.h"

#pragma push
#pragma cplusplus on

static Boolean inited = false;
static FSSpec ThisPath = {0,1,"\p"};    // 27.3.96: dirID mu§te von 0 auf 1 geŠndert werden, weil sonst u.U. ResolveAlias einen Endlos-Loop machte

static OSErr getInfo (CInfoPBPtr pb, FSSpec &fs)
{
    pb->hFileInfo.ioVRefNum = fs.vRefNum;
    pb->hFileInfo.ioNamePtr = fs.name;
    pb->hFileInfo.ioFDirIndex = 0;
    pb->hFileInfo.ioDirID = fs.parID;
    return PBGetCatInfoSync (pb);
}

OSErr FSpResolveAlias (FSSpec &spec)
// PrŸft, ob die Datei ein Alias ist und lšst den ggf. auf.
// Liefert Fehlercode, z.B., wenn ein Login-Dialog vom User abgebrochen wurde.
{
    Boolean isFolder, wasAlias;
    return ResolveAliasFile (&spec, true, &isFolder, &wasAlias);
}


DrvQElPtr FindDrvQ (short drvNum)
{
    DrvQElPtr qep = (DrvQElPtr)(GetDrvQHdr()->qHead);
    while (qep) {
        if (qep->dQDrive == drvNum) return qep;
        qep = (DrvQElPtr)qep->qLink;
    }
    return nil;
}

short FindVolByDriveNum (short drvNum)
{
    HVolumeParam pb;
    short err, idx;

    idx = 1;
    do {
        pb.ioVolIndex = idx;
        pb.ioNamePtr = NULL;
        err = PBHGetVInfoSync ((HParamBlockRec*)&pb);
        if (err) break;
        if (pb.ioVDrvInfo == drvNum) {
            return pb.ioVRefNum;
        }
        ++idx;
    } while (true);
    return 0;
}

OSErr GetSysVolume (short *vRefNum)
// returns the boot volume
{
    long dir;
    OSErr err = FindFolder (kOnSystemDisk, kSystemFolderType, false, vRefNum, &dir);
    if (err) *vRefNum = -1;
    return err;
}


OSErr FindInFolderByCreator (short vRefNum, long dirID, OSType creator, OSType fileType, FSSpec *foundSpec)
{
    OSErr err;
    HFileParam pb;
    short idx = 1;
    do {
        pb.ioVRefNum = vRefNum;
        pb.ioDirID = dirID;
        pb.ioFDirIndex = idx++;
        pb.ioNamePtr = foundSpec->name;
        pb.ioFVersNum = 0;
        err = PBHGetFInfoSync ((HParmBlkPtr)&pb);
        if (!err) {
            if (pb.ioFlFndrInfo.fdCreator == creator && (fileType == 0 || pb.ioFlFndrInfo.fdType == fileType)) {
                foundSpec->vRefNum = vRefNum;
                foundSpec->parID = dirID;
                return noErr;
            }
        }
    } while (!err);
    return err;
}

OSErr FindOnDiskByCreator (short vRefNum, OSType creator, OSType fileType, FSSpec *foundSpec)
{
    OSErr err;
    CSParam pb;
    CInfoPBRec  info1;
    CInfoPBRec  info2 = {0};

    pb.ioVRefNum = vRefNum;
    pb.ioNamePtr = nil;
    pb.ioMatchPtr = foundSpec;
    pb.ioReqMatchCount = 1;
    pb.ioCatPosition.initialize = 0;
    pb.ioOptBuffer = nil;
    pb.ioOptBufSize = 0;
    pb.ioSearchTime = 0;
    pb.ioSearchBits = fsSBFlAttrib | fsSBFlFndrInfo;
    pb.ioSearchInfo1 = &info1;
    pb.ioSearchInfo2 = &info2;
    if (fileType) {
        info1.hFileInfo.ioFlFndrInfo.fdType = fileType;
        info2.hFileInfo.ioFlFndrInfo.fdType = (UInt32) -1;  // mask
    }
    info1.hFileInfo.ioFlFndrInfo.fdCreator = creator;
    info2.hFileInfo.ioFlFndrInfo.fdCreator = (UInt32) -1;   // mask
    info1.hFileInfo.ioFlAttrib &= ~ioDirMask;   // looking for files, not dirs
    info2.hFileInfo.ioFlAttrib |= ioDirMask;    // set the mask for files/dirs
    err = PBCatSearchSync (&pb);
    if (!err && pb.ioActMatchCount > 0) {
        return noErr;
    }
    return fnfErr;
}
#if 0 // Disable by MLT
EXTERN_API(Ptr) GetFCBSPtr(void) TWOWORDINLINE(0x2EB8, 0x034E);

OSErr FindOpenFileByTypeAndCreator (OSType type, OSType creator, FSSpec *itsSpec)
// type = 0 -> don't care
{
    OSErr err;
    short idx = 1;
    Boolean hasFSM = TrapAvailable(0xA824);
    do {
        FCBPBRec pb;
        pb.ioFCBIndx = idx++;
        pb.ioVRefNum = 0;   // means: index through all volumes
        pb.ioNamePtr = itsSpec->name;
        err = PBGetFCBInfoSync (&pb);
        if (!err) {
            FCBRecPtr fcb;
            if (hasFSM) {
                UTResolveFCB (pb.ioRefNum, &fcb);
            } else {
                fcb = (FCBRecPtr) ((Ptr)GetFCBSPtr() + pb.ioRefNum);
            }
            if (type == 0 || type == fcb->fcbFType) {
                if (creator) {
                    itsSpec->vRefNum = pb.ioVRefNum;
                    itsSpec->parID = pb.ioFCBParID;
                    FInfo fi;
                    err = FSpGetFInfo (itsSpec, &fi);
                    if (!err) {
                        if (creator == fi.fdCreator) {
                            return noErr;
                        }
                    }
                }
            }
        }
    } while (!err);
    return err;
}
#endif

#pragma pop

// EOF
