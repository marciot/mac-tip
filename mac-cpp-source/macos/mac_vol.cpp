#include <stdio.h>
#include <Files.h>
#include "mac_vol.h"

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