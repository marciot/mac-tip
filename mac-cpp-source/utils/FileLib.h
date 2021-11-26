/*
 * File Utilities (System 7 and above)
 *
 * by Thomas Tempelmann, macdev@tempel.org
 */

#include <Devices.h>

#pragma push
#pragma cplusplus on

Boolean FSpIsAlias (FSSpec &spec);
OSErr FSpResolveAlias (FSSpec &spec);       // Lšst ggf. Aliase auf

DrvQElPtr FindDrvQ (short drv);
short FindVolByDriveNum (short drvNum);

OSErr GetSysVolume (short *vRefNum);        // gets the boot volume

OSErr FindInFolderByCreator (short vRefNum, long dirID, OSType creator, OSType fileType, FSSpec *foundSpec);
OSErr FindOnDiskByCreator (short vRefNum, OSType creator, OSType fileType, FSSpec *foundSpec);

OSErr FindOpenFileByTypeAndCreator (OSType type, OSType creator, FSSpec *itsSpec);

#pragma pop

// EOF
