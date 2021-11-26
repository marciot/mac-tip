/*
 * Functions for launching apps with documents
 *
 * by Thomas Tempelmann, macdev@tempel.org
 *
 * some of these routines are not from me (TT), but from some DTS sample code
 * here's a link to it: <http://developer.apple.com/dev/techsupport/source/code/Snippets/Processes/LaunchWithDoc2.1/LaunchWithDoc2.c.html>
 */

#pragma once

#include <Processes.h>

#ifdef __cplusplus
extern "C" {
#endif

/*
    -> also see "SignatureToApp.c" as a demo (comes from Apple and does some things differently)
*/
OSErr OpenWithFinder (const FSSpec * applicationFSSpecPtr); // uses AppleEvent to open the file
OSErr LaunchAppl (const FSSpec * applicationFSSpecPtr, Boolean inBackground);   // starts APPLs only
OSErr OpenSpecifiedDocument(const FSSpec * documentFSSpecPtr, Boolean inBackground);    // starts APPLs or opens docs with its APPLs
OSErr LaunchApplicationWithDocument(const FSSpec * applicationFSSpecPtr, const FSSpec * documentFSSpecPtr, Boolean inBackground);
OSErr SendOpenDocumentEventToProcess(ProcessSerialNumber *targetPSN, const FSSpec * documentFSSpecPtr);
OSErr FindApplicationFromDocument(const FSSpec * documentFSSpecPtr, FSSpec *applicationFSSpecRef);
OSErr FindApplicationByCreator (OSType creator, short firstVol, FSSpec *applicationFSSpecRef);
OSErr FindRunningAppBySignature (OSType fileType, OSType creator, ProcessSerialNumber *psn, FSSpec *fileSpec);
OSErr SendFSSEventToProcess (ProcessSerialNumber *targetPSN, OSType aeClass, OSType aeCmd, const FSSpec *documentFSSpecPtr);
OSErr FindOpenCDEVInFinder (Boolean shouldUseAEs, OSType creator, FSSpec *itsSpec);

#ifdef __cplusplus
}
#endif

// EOF
