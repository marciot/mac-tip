/*
 * Functions for launching apps with documents
 *
 * by Thomas Tempelmann, macdev@tempel.org
 *
 * some of these routines are not from me (TT), but from some DTS sample code.
 * here's a link to it: <http://developer.apple.com/dev/techsupport/source/code/Snippets/Processes/LaunchWithDoc2.1/LaunchWithDoc2.c.html>
 */


//#include <Processes.h>
#include <AppleEvents.h>
#include <AERegistry.h>
#include <AEPackObject.h>
#include <AEObjects.h>
#include <Aliases.h>
#include <Errors.h>
#include "PascalLib.h"
#include "FileLib.h"
#include "LaunchLib.h"


#pragma cplusplus on

/*
 Korrekturen:
 TT 23.2.95: foundFlag wurde nicht auf "false" initialisiert.
 TT 5.11.96: LaunchAppl() neu, OpenSpecifiedDocument() kann auch APPLs direkt starten
 TT 30.1.99: FindApplicationFromDocument() stellt nun sicher, da§ die App wirklich existiert (das
             ist nicht der Fall, wenn das Volume not mounted ist oder wenn die App verschoben
             oder gelšscht wurde, oder wenn die Datei nicht vom Typ 'APPL' ist).
 TT 13.2.99: FindApplicationFromDocument() updated to use the proper algorith that the Finder
             uses. For that, the code has been largely rewritten (reordered)
             Note: it is not perfectly like the Finder: The Finder orders remote vols by their
             speed, thru the parm "volumeGrade", while I ignore this information, since most
             vols are having a grade of 0, anyways.
             Attention: It does not use the "fopn" Finder interception AE that has been introduced
             in Mac OS 8!
 TT 16.2.99: LaunchAppl(): launchNoFileFlags was set in wrong field (in launchFileFlags instead
             launchControlFlags)
 TT 23.3.99: added: FindApplicationByCreator(), OpenWithFinder(), FindRunningAppBySignature(),
             SendFSSEventToProcess().
 TT 15.4.99: added the file type 'APPD' to the list of launchable apps in checkThisVolume().
             (APPD is used for apps routed to the Apple Menu Items folder)
 TT 31.7.99: added "Boolean inBackground" parm to OpenSpecifiedDocument() and LaunchAppl(),
             and added SetFrontProcess() in OpenSpecifiedDocument() and in LaunchApplicationWithDocument()
             in order to bring the app to the front when it was already running.
 TT 24.4.00: added the file type 'appe' to the list of launchable apps in checkThisVolume(),
             added types 'APPC', 'APPD' and 'appe' to
*/

// OpenSpecifiedDocument searches to see if the application which
// created the document is already running.  If so, it sends
// an OpenSpecifiedDocuments Apple event to the target application
// (remember that, because of puppet strings, this works even
// if the target application is not Apple event-aware.)

OSErr OpenSpecifiedDocument(const FSSpec * documentFSSpecPtr, Boolean inBackground)
{
    OSErr retCode;
    ProcessSerialNumber currPSN;
    ProcessInfoRec currProcessInfo;
    FSSpec applicationSpec;
    FInfo documentFInfo;
    Boolean foundRunningProcessFlag;

    // verify the document file exists and get its creator type

    retCode = FSpGetFInfo(documentFSSpecPtr, &documentFInfo);
    if (retCode != noErr) goto Bail;

    // check the current processes to see if the creator app is already
    // running, and get its process serial number (as currPSN)

    currPSN.lowLongOfPSN = kNoProcess;
    currPSN.highLongOfPSN = 0;

    currProcessInfo.processInfoLength = sizeof(ProcessInfoRec);
    currProcessInfo.processName = nil;
    currProcessInfo.processAppSpec = &applicationSpec;

    foundRunningProcessFlag = false;
    while (GetNextProcess(&currPSN) == noErr) {
        if (GetProcessInformation(&currPSN, &currProcessInfo) == noErr) {
            if (currProcessInfo.processSignature == documentFInfo.fdCreator) {
                foundRunningProcessFlag = true;
                break;
            }
        }
    }

    if (foundRunningProcessFlag) {
        // if the creator is running, send it an OpenDocuments Apple event
        // since there is no need to launch it

        if (currProcessInfo.processType != documentFInfo.fdType) {  // this would be the case if the doc is an app
            retCode = SendOpenDocumentEventToProcess(&currPSN, documentFSSpecPtr);
        }
        if (retCode == noErr) {
            if (!inBackground) {
                SetFrontProcess (&currPSN);
            }
        }

    } else if (documentFInfo.fdType == 'APPL' || documentFInfo.fdType == 'APPC' || documentFInfo.fdType == 'APPD' || documentFInfo.fdType == 'appe') {
        // else if the creator is not running and if the document to be opened is a application,
        // then launch it directly.

        retCode = LaunchAppl (documentFSSpecPtr, inBackground);

    } else {
        // else if the creator is neither running nor an application, find it on disk and launch
        // it with the OpenDocuments event included as a part of the launch parameters

        retCode = FindApplicationFromDocument(documentFSSpecPtr, &applicationSpec);

        if (retCode == noErr) {
            retCode = LaunchApplicationWithDocument(&applicationSpec, documentFSSpecPtr, inBackground);
        }
    }

Bail:
    return retCode;
}

OSErr LaunchAppl (const FSSpec * applicationFSSpecPtr, Boolean inBackground)
{
    LaunchParamBlockRec launchParams;
    launchParams.launchAppParameters = nil;
    launchParams.launchBlockID = extendedBlock;
    launchParams.launchEPBLength = extendedBlockLen;
    launchParams.launchFileFlags = 0;
    launchParams.launchControlFlags = launchContinue | launchNoFileFlags;
    if (inBackground) launchParams.launchControlFlags |= launchDontSwitch;
    launchParams.launchAppSpec = (FSSpecPtr)applicationFSSpecPtr;
    return LaunchApplication (&launchParams);
}

// given an application and a document, LaunchApplicationWithDocument
// launches the application and passes the application an
// OpenDocuments event for the document
// if the app is already running, then it simply gets the open event

OSErr LaunchApplicationWithDocument(const FSSpec * applicationFSSpecPtr,
    const FSSpec * documentFSSpecPtr, Boolean inBackground)
{
    OSErr retCode;
    LaunchParamBlockRec launchParams;
    ProcessSerialNumber myPSN;
    AppleEvent theAppleEvent;
    AEDesc myAddrDesc, launchParamDesc, docDesc;
    AEDescList docDescList;
    AliasHandle docAlias;
    ProcessSerialNumber currPSN;
    ProcessInfoRec currProcessInfo;
    FSSpec applicationSpec;
    Boolean foundRunningProcessFlag;

    // to simplify cleanup, ensure that handles are nil to start
    theAppleEvent.dataHandle = nil;
    launchParamDesc.dataHandle = nil;
    docDescList.dataHandle = nil;
    docDesc.dataHandle = nil;
    docAlias = nil;

    // check if the app is already running
    currPSN.lowLongOfPSN = kNoProcess;
    currPSN.highLongOfPSN = 0;
    currProcessInfo.processInfoLength = sizeof(ProcessInfoRec);
    currProcessInfo.processName = nil;
    currProcessInfo.processAppSpec = &applicationSpec;
    foundRunningProcessFlag = false;
    while (GetNextProcess(&currPSN) == noErr) {
        if (GetProcessInformation(&currPSN, &currProcessInfo) == noErr) {
            if (applicationSpec.vRefNum == applicationFSSpecPtr->vRefNum
             && applicationSpec.parID == applicationFSSpecPtr->parID
             && pstrcmp (applicationSpec.name, (const StringPtr)applicationFSSpecPtr->name) == 0) {
                foundRunningProcessFlag = true;
                break;
            }
        }
    }
    if (foundRunningProcessFlag) {
        retCode = SendOpenDocumentEventToProcess (&currPSN, documentFSSpecPtr);
        if (retCode == noErr) {
            if (!inBackground) {
                SetFrontProcess (&currPSN);
            }
        }
        return retCode;
    }

    // the Apple event will need a valid address descriptor (even though its
    // contents will not matter since we will not be calling AESend) so make
    // one using my own serial number

    (void) GetCurrentProcess(&myPSN);
    retCode = AECreateDesc(typeProcessSerialNumber, (Ptr) &myPSN,
        sizeof(ProcessSerialNumber), &myAddrDesc);
    if (retCode != noErr) goto Bail;

    // make a descriptor list containing just a descriptor with an
    // alias to the document

    retCode = AECreateList(nil, 0, false, &docDescList);
    if (retCode != noErr) goto Bail;

    retCode = NewAlias(nil, documentFSSpecPtr, &docAlias);
    if (retCode != noErr) goto Bail;

    HLock((Handle) docAlias);
    retCode = AECreateDesc(typeAlias, (Ptr) *docAlias,
        GetHandleSize((Handle) docAlias), &docDesc);
    HUnlock((Handle) docAlias);
    if (retCode != noErr) goto Bail;

    retCode = AEPutDesc(&docDescList, 0, &docDesc);
    if (retCode != noErr) goto Bail;

    // now make the 'odoc' AppleEvent descriptor and insert the
    // document descriptor list as the direct object

    retCode = AECreateAppleEvent(kCoreEventClass, kAEOpenDocuments,
        &myAddrDesc, kAutoGenerateReturnID, kAnyTransactionID,
        &theAppleEvent);
    if (retCode != noErr) goto Bail;

    retCode = AEPutParamDesc(&theAppleEvent, keyDirectObject, &docDescList);
    if (retCode != noErr) goto Bail;

    // this Apple event will not be sent but rather will be used
    // as a parameter to the LaunchApplication call, so coerce it
    // to the magic type typeAppParamters

    retCode = AECoerceDesc(&theAppleEvent, typeAppParameters, &launchParamDesc);
    if (retCode != noErr) goto Bail;

    // finally, fill in the launch parameter block, including the
    // Apple event, and make the launch call

    HLock((Handle) launchParamDesc.dataHandle);
    launchParams.launchAppParameters =
        (AppParametersPtr) *(launchParamDesc.dataHandle);

    launchParams.launchBlockID = extendedBlock;
    launchParams.launchEPBLength = extendedBlockLen;
    launchParams.launchFileFlags = launchNoFileFlags;
    launchParams.launchControlFlags = launchContinue;
    if (inBackground) launchParams.launchControlFlags |= launchDontSwitch;
    launchParams.launchAppSpec = (FSSpecPtr) applicationFSSpecPtr;

    retCode = LaunchApplication(&launchParams);

Bail:
    // dispose of everything that was allocated

    if (theAppleEvent.dataHandle != nil)     (void) AEDisposeDesc(&theAppleEvent);
    if (launchParamDesc.dataHandle != nil)   (void) AEDisposeDesc(&launchParamDesc);
    if (docDescList.dataHandle != nil)       (void) AEDisposeDesc(&docDescList);
    if (docDesc.dataHandle != nil)           (void) AEDisposeDesc(&docDesc);
    if (launchParamDesc.dataHandle != nil)   (void) AEDisposeDesc(&launchParamDesc);
    if (docAlias != nil)
        DisposeHandle((Handle) docAlias);

    return retCode;

}


OSErr SendFSSEventToProcess (ProcessSerialNumber *targetPSN, OSType aeClass, OSType aeCmd, const FSSpec *documentFSSpecPtr)
{
    OSErr retCode;
    AppleEvent theAppleEvent = {typeNull, nil}, theReplyEvent = {typeNull, nil};
    AEDesc targetAddrDesc = {typeNull, nil}, docDesc = {typeNull, nil};
    //AEDescList docDescList;
    AliasHandle docAlias;

    // to simplify cleanup, ensure that handles are nil to start
    theAppleEvent.dataHandle = nil;
    //docDescList.dataHandle = nil;
    docDesc.dataHandle = nil;
    docAlias = nil;

    // create an address descriptor based on the serial number of
    // the target process

    retCode = AECreateDesc(typeProcessSerialNumber, (Ptr) targetPSN,
        sizeof(ProcessSerialNumber), &targetAddrDesc);
    if (retCode != noErr) goto Bail;

    /* I see no reason to make a list here, it's always just one item anyway
        // make a descriptor list containing just a descriptor with an
        // alias to the document
        retCode = AECreateList(nil, 0, false, &docDescList);
        if (retCode != noErr) goto Bail;
    */

    retCode = NewAlias(nil, documentFSSpecPtr, &docAlias);
    if (retCode != noErr) goto Bail;

    HLock((Handle) docAlias);
    retCode = AECreateDesc(typeAlias, (Ptr) *docAlias,
        GetHandleSize((Handle) docAlias), &docDesc);
    HUnlock((Handle) docAlias);
    if (retCode != noErr) goto Bail;

    /* I see no reason to make a list here, it's always just one item anyway
        retCode = AEPutDesc(&docDescList, 0, &docDesc);
        if (retCode != noErr) goto Bail;
    */

    // now make the AppleEvent descriptor and insert the
    // document descriptor list as the direct object

    retCode = AECreateAppleEvent(aeClass, aeCmd,
        &targetAddrDesc, kAutoGenerateReturnID, kAnyTransactionID,
        &theAppleEvent);
    if (retCode != noErr) goto Bail;

    // retCode = AEPutParamDesc(&theAppleEvent, keyDirectObject, &docDescList);
    retCode = AEPutParamDesc(&theAppleEvent, keyDirectObject, &docDesc);
    if (retCode != noErr) goto Bail;

    // finally, send the Apple event
    retCode = AESend (&theAppleEvent, &theReplyEvent, kAENoReply,
        kAEHighPriority, 2*60 /* timeout: 2 seconds */, nil, nil);

Bail:
    // dispose of everything that was allocated

    if (theAppleEvent.dataHandle != nil)  (void) AEDisposeDesc(&theAppleEvent);
    //if (docDescList.dataHandle != nil)  (void) AEDisposeDesc(&docDescList);
    if (docDesc.dataHandle != nil)  (void) AEDisposeDesc(&docDesc);
    if (docAlias != nil)  DisposeHandle((Handle) docAlias);

    return retCode;
}


// given an application's serial number and a document,
// SendOpenDocumentEventToProcess passes
// the application an OpenDocuments event for the document

OSErr SendOpenDocumentEventToProcess(ProcessSerialNumber *targetPSN, const FSSpec * documentFSSpecPtr)
{
    return SendFSSEventToProcess (targetPSN, kCoreEventClass, kAEOpenDocuments, documentFSSpecPtr);
}


static Boolean HasCatSearch (short vRefNum)
{
    IOParam pb;
    GetVolParmsInfoBuffer buf;
    pb.ioVRefNum = vRefNum;
    pb.ioBuffer = (Ptr)&buf;
    pb.ioReqCount = 6;
    pb.ioNamePtr = nil;
    if (PBHGetVolParmsSync ((HParmBlkPtr)&pb) == noErr) {
        if (pb.ioActCount >= 6 && (buf.vMAttrib & (1L<<bHasCatSearch)) != 0) return true;
    }
    return false;
}

static Boolean IsRemoteVolume (short vRefNum)
{
    IOParam pb;
    GetVolParmsInfoBuffer buf;
    pb.ioVRefNum = vRefNum;
    pb.ioBuffer = (Ptr)&buf;
    pb.ioReqCount = 14;
    pb.ioNamePtr = nil;
    if (PBHGetVolParmsSync ((HParmBlkPtr)&pb) == noErr) {
        if (pb.ioActCount >= 14 && buf.vMLocalHand != 0) return true;
    }
    return false;
}

static OSErr checkThisVolume (short currVRefNum, OSType creator, FSSpecPtr applicationFSSpecPtr, Boolean *foundRef)
{
    OSErr retCode;
    Boolean foundFlag = false;

    // find the path refNum for the desktop database for
    // the volume we're interested in

    DTPBRec desktopParams;
    desktopParams.ioVRefNum = currVRefNum;
    desktopParams.ioNamePtr = nil;
    retCode = PBDTGetPath(&desktopParams);

    if (retCode == noErr) {
        if (desktopParams.ioDTRefNum == 0) {
            // oops?!
            retCode = -1;   // !TT new 30 Jan 99
        } else {

            // iterate over all possible creators on one volume

            short idx = 0;
            do {
                // use the GetAPPL call to find the preferred application
                // for opening any document with this one's creator

                desktopParams.ioIndex = idx++;  // this is the way the Finder in OS 7.6.1 does it: starts with idx 0, then comes 1, 2, ...
                desktopParams.ioFileCreator = creator;
                desktopParams.ioNamePtr = applicationFSSpecPtr->name;
                retCode = PBDTGetAPPLSync(&desktopParams);

                if (retCode == noErr) {

                    // okay, found it; fill in the application file spec
                    // and set the flag indicating we're done

                    applicationFSSpecPtr->parID = desktopParams.ioAPPLParID;
                    applicationFSSpecPtr->vRefNum = currVRefNum;

                    // However, we have to make sure that the app
                    // really still exists and that it is a APPL in deed.
                    // If not, we have to continue to search

                    CInfoPBRec ci;
                    HFileInfo &fi = (HFileInfo&)ci;
                    fi.ioNamePtr = applicationFSSpecPtr->name;
                    fi.ioVRefNum = currVRefNum;
                    fi.ioDirID = applicationFSSpecPtr->parID;
                    fi.ioFDirIndex = 0;
                    if (PBGetCatInfoSync (&ci) == noErr) {
                        if (/* do not check the creation date, because the Finder doesn't do it either: fi.ioFlCrDat == desktopParams.ioTagInfo && */
                            fi.ioFlFndrInfo.fdCreator == desktopParams.ioFileCreator &&
                            fi.ioFlFndrInfo.fdType == 'APPL' || fi.ioFlFndrInfo.fdType == 'APPC' || fi.ioFlFndrInfo.fdType == 'APPD' || fi.ioFlFndrInfo.fdType == 'appe' // +++ add more types or use a better detection?
                        ) {
                            foundFlag = true;
                        }
                    }
                }
            } while (!foundFlag && retCode == noErr);
        }
    }
    *foundRef = foundFlag;
    return retCode;
}

// FindApplicationByCreator uses the Desktop Database to
// locate the creator application for the given document
//
// this routine will first check the desktop database of the disk
// containing the document, then the desktop database of all local
// disks, then the desktop databases of all server volumes
// (so up to three passes will be made)
// exception (13.2.99): when the file is located on a remote vol,
// the boot vol is searched first
// now returns fnfErr if app is not found

// Attention: It does not support the "fopn" Finder interception AE that has been introduced
//            in Mac OS 8!

OSErr FindApplicationByCreator (OSType creator, short firstVol, FSSpec *applicationFSSpecRef)
{
    short volumeIndex;
    OSErr err;
    Boolean found = false;
    short vRefNum, skipThisVol = 0;

    if (IsRemoteVolume (firstVol)) {
        // first, check the boot vol
        GetSysVolume (&vRefNum);
        checkThisVolume (vRefNum, creator, applicationFSSpecRef, &found);
        if (found) return noErr;
        skipThisVol = vRefNum;
    }

    // check the vol the doc is on
    checkThisVolume (firstVol, creator, applicationFSSpecRef, &found);
    if (found) return noErr;

    for (int remotePass = 0; remotePass <= 3; ++remotePass) {
        volumeIndex = 0;
        do {
            HParamBlockRec pb;
            HVolumeParam &vp = (HVolumeParam&)pb;

            vp.ioNamePtr = nil;
            vp.ioVRefNum = 0;
            vp.ioVolIndex = ++volumeIndex;
            err = PBHGetVInfoSync (&pb);
            if (err != nsvErr) {
                if (err != noErr) return err;
                if (vp.ioVRefNum != firstVol && vp.ioVRefNum != skipThisVol) {
                    // prio: 0 & 1 for local, 2 & 3 for remote; 0 & 2 with CatSearch, 1 & 3 without CatSearch
                    int volumePrio = IsRemoteVolume (vp.ioVRefNum) * 2 + (HasCatSearch (vp.ioVRefNum)?0:1);
                    if (remotePass == volumePrio) {
                        checkThisVolume (vp.ioVRefNum, creator, applicationFSSpecRef, &found);
                        if (found) {
                            return noErr;
                        }
                    }
                }
            }
        } while (err != nsvErr);
    }
    if (err == nsvErr) err = fnfErr;
    return err;
}


OSErr FindApplicationFromDocument (const FSSpec * documentFSSpecPtr, FSSpec *applicationFSSpecRef)
{
    OSErr err;
    OSType creator;

    // verify the document file exists and get its creator type
    {
        FInfo documentFInfo;
        err = FSpGetFInfo(documentFSSpecPtr, &documentFInfo);
        if (err != noErr) return err;
        creator = documentFInfo.fdCreator;
    }

    return FindApplicationByCreator (creator, documentFSSpecPtr->vRefNum, applicationFSSpecRef);
}

OSErr OpenWithFinder (const FSSpec *spec)
{
    ProcessSerialNumber finderPSN;
    FSSpec finderSpec;
    OSErr err = FindRunningAppBySignature ('FNDR', 'MACS', &finderPSN, &finderSpec);
    if (!err) {
        err = SendFSSEventToProcess (&finderPSN, kCoreEventClass, kAEOpenDocuments, spec);
    }
    return err;
}

OSErr FindRunningAppBySignature (OSType fileType, OSType creator, ProcessSerialNumber *psn, FSSpec *fileSpec)
// from: "SignatureToApp.c", by Jens Alfke, DTS, Apple Computer 1991
{
    OSErr err;
    ProcessInfoRec info;
    psn->highLongOfPSN = 0;
    psn->lowLongOfPSN  = kNoProcess;
    do{
        err = GetNextProcess(psn);
        if (!err) {
            info.processInfoLength = sizeof (info);
            info.processName = NULL;
            info.processAppSpec = fileSpec;
            err = GetProcessInformation (psn, &info);
        }
    } while (!err && info.processSignature != creator && info.processType != fileType);
    if (!err) *psn = info.processNumber;
    return err;
}


static long CountItems (AEDesc *d)
{
    long items = 1;
    if(d->descriptorType == typeNull) {
        items = 0;
    } else if((d->descriptorType == typeAEList) || (d->descriptorType == typeAERecord)) {
        AECountItems(d, &items);
    }
    return items;
}

static void ae_dispose (AEDesc &d)
{
    if (d.dataHandle) AEDisposeDesc (&d);
}

OSErr FindOpenCDEVInFinder (Boolean shouldUseAEs, OSType creator, FSSpec *itsSpec)
// algorithm: either uses AppleEvents to get list of all windows, coercing the results to FSSpecs
// The FSSpecs can then be used for both getting the creator code and for closing the window
// returns: 0 if not open in Finder, 1 if open, neg. value if error occured
{
    long theTimeout = 5*60; // wait no more than 5 seconds for Finder to respond

    if (!shouldUseAEs) {
        return FindOpenFileByTypeAndCreator ('cdev', creator, itsSpec);
    }

    Boolean found = false;
    OSErr err;

    AEDesc dataDescriptor = {typeNull,0};
    AppleEvent reply = {typeNull,0};
    AEDesc allCreators = {typeNull,0};
    AEDesc targetAddrDesc = {typeNull,0};
    AppleEvent ae = {typeNull,0};
    AEDesc allWinsSpecifier = {typeNull,0}, keyData = {typeNull,0};
    AEDesc keyData2 = {typeNull,0};
    AEDesc directObjectSpecifier = {typeNull,0}, nullDesc = {typeNull,0};

    ProcessSerialNumber finderPSN, frontPSN;
    FSSpec finderSpec;
    err = FindRunningAppBySignature ('FNDR', 'MACS', &finderPSN, &finderSpec);
    if (err) goto Bail;

    GetFrontProcess (&frontPSN);
    Boolean finderIsFrontProcess;
    SameProcess (&frontPSN, &finderPSN, &finderIsFrontProcess);

    if (finderIsFrontProcess) {
        // oops, we will not be successful sending the AE (a timeout would occur).
        // So fall back to the other method.
        err = FindOpenFileByTypeAndCreator ('cdev', creator, itsSpec);
        return err; // no cleanup necessary here yet
    }

    err = AECreateDesc (typeProcessSerialNumber, (Ptr)&finderPSN, sizeof(ProcessSerialNumber), &targetAddrDesc);
    if (err) goto Bail;

    err = AECreateAppleEvent(kAECoreSuite, kAEGetData, &targetAddrDesc, kAutoGenerateReturnID, kAnyTransactionID, &ae);

    // want:type(cwin), from:'null'(), form:indx, seld:abso('all ')
    OSType all = 'all ';
    err = AECreateDesc (typeAbsoluteOrdinal, &all, sizeof(all), &keyData);
    if (err) goto Bail;

    err = CreateObjSpecifier (cWindow, &nullDesc, formAbsolutePosition, &keyData, true, &allWinsSpecifier);
    if (err) goto Bail;

    // want:type(prop), from:<allWinsSpecifier>, form:prop, seld:type(cobj)
    OSType cobj = cObject;
    err = AECreateDesc (typeType, &cobj, sizeof(cobj), &keyData2);
    if (err) goto Bail;

    err = CreateObjSpecifier (cProperty, &allWinsSpecifier, formPropertyID, &keyData2, true, &directObjectSpecifier);
    if (err) goto Bail;

    err = AEPutParamDesc (&ae, keyDirectObject, &directObjectSpecifier);
    if (err) goto Bail;
    AEDisposeDesc (&directObjectSpecifier);

    // core,getd,'----':<>, rtyp:type(list)"
    OSType list = 'list';
    err = AECreateDesc (typeType, &list, sizeof(list), &dataDescriptor);
    if (err) goto Bail;

    err = AEPutParamDesc (&ae, keyAERequestedType, &dataDescriptor);
    if (err) goto Bail;
    AEDisposeDesc (&dataDescriptor);

    err = AESend (&ae, &reply, kAEWaitReply, kAEHighPriority, theTimeout, nil, nil);
    if (err) goto Bail;

    ae_dispose (ae);

    err = AEGetParamDesc (&reply, keyAEResult, typeWildCard, &allCreators);
    if (err) goto Bail;

    long n = CountItems (&allCreators);
    for (int i = 1; i < n; ++i) {
        AEDesc w;
        AEKeyword ignoreKey;
        if (AEGetNthDesc (&allCreators, i, typeFSS, &ignoreKey, &w) == noErr) {
            FSSpec spec;
            spec = *(FSSpec*)*w.dataHandle;
            AEDisposeDesc (&w);
            FInfo fi;
            if (FSpGetFInfo (&spec, &fi) == noErr && fi.fdCreator == creator) {
                // we found it
                *itsSpec = spec;
                found = true;
                break;
            }
        }
    }
    err = found;

Bail:
    ae_dispose (keyData);
    ae_dispose (keyData2);
    ae_dispose (targetAddrDesc);
    ae_dispose (dataDescriptor);
    ae_dispose (allWinsSpecifier);
    ae_dispose (directObjectSpecifier);
    ae_dispose (allCreators);
    ae_dispose (reply);
    ae_dispose (ae);

    return err;
}

// EOF
