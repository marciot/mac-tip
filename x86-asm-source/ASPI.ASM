;+-----------------------------------------------------------------------------+
;|	       ASPI.ASM	 Aspi Functions	for Trouble In Paradise?	       |
;+-----------------------------------------------------------------------------+

DEFECT_LIST_HEADER		struct	1t
;-------------------------------------------------------------------------------
DLH_reserved			BYTE  ? ; (00h)
DLH_BitFlags			BYTE  ? ; [000] [P] [G] [xxx - defect list format]
DLH_DefectListLength		WORD  ? ;
;-------------------------------------------------------------------------------
DEFECT_LIST_HEADER		ends


;+-----------------------------------------------------------------------------+
;|				CHECK FOR ASPI				       |
;|-----------------------------------------------------------------------------|
;|  This locates the system's ASPI functions.  If all goes well	it sets	two    |
;|  pointers to	the WinASPI entrypoints	and returns with EAX !=	0.  In the     |
;|  event of trouble it	sets the error message into the	TROUBLE	window,	and    |
;|  sets the ErrorMode flag to cause the Trouble panel to be displayed.	       |
;+-----------------------------------------------------------------------------+
CheckForASPI	PROC USES esi edi
;-------------------------------------------------------------------------------
	; suppress any	DLL not	found errors and try to	load our DLL
	invoke	SetErrorMode, SEM_NOOPENFILEERRORBOX
	mov	edi, eax
	invoke	LoadLibrary, ADDR szASPI32DLLName
IF FORCE_TROUBLE
	zero	eax
ENDIF
	mov	esi, eax
	invoke	SetErrorMode, edi
	.IF (esi)
		mov	hASPIDLL,	esi
		invoke	GetProcAddress, esi, ADDR szASPI32SupportInfo
		mov	lpGetASPI32SupportInfo, eax
		invoke	GetProcAddress, esi, ADDR szASPI32Command
		mov	lpSendASPI32Command, eax
		invoke	GetASPI32SupportInfoPtr PTR lpGetASPI32SupportInfo
		cmp	ah, SS_COMP		; did we succeed perfectly?
		je	Exit			; yep, so off we go	...
		cmp	ah, SS_FAILED_INIT	; nope, did we	init fail?
		jne	No_ASPI			; nope,	so some	error
		set	ErrorMode
	.ELSE	; set the static	text for the ASPI version problem control
No_ASPI:	set	BadASPIDrivers
	.ENDIF
	zero	eax
Exit:	movzx	eax, al
	ret
;-------------------------------------------------------------------------------
CheckForASPI	ENDP


;+-----------------------------------------------------------------------------+
;|			     HOST ADAPTER INQUIRY			       |
;+-----------------------------------------------------------------------------+
HostAdapterInquiry	PROC	Adapter:DWORD
;-------------------------------------------------------------------------------
	ZeroASPI	; clear out our calling block's parameters
	mov	ASPI_CmdBlock.hai.SRB_Cmd,	SC_HA_INQUIRY
	movmov	ASPI_CmdBlock.hai.SRB_HaId, al,	byte ptr Adapter
	invoke	SendASPI32CommandPtr PTR lpSendASPI32Command, ADDR ASPI_CmdBlock
	movzx	eax, ASPI_CmdBlock.hai.SRB_Status
	dec	eax
	ret
;-------------------------------------------------------------------------------
HostAdapterInquiry	ENDP


;+-----------------------------------------------------------------------------+
;|				GET DEVICE TYPE				       |
;+-----------------------------------------------------------------------------+
GetDeviceType	PROC	Adapter:DWORD, Device:DWORD
;-------------------------------------------------------------------------------
	ZeroASPI
	mov	ASPI_CmdBlock.gdt.SRB_Cmd,	SC_GET_DEV_TYPE	  ; w: ASPI command code
	movmov	ASPI_CmdBlock.gdt.SRB_HaId, al,	byte ptr Adapter  ; w: ASPI host adapter number
	movmov	ASPI_CmdBlock.gdt.SRB_Target, al, byte ptr Device ; w: target's	SCSI ID
	; do the command ...
	invoke	SendASPI32CommandPtr PTR lpSendASPI32Command, ADDR ASPI_CmdBlock
	movzx	eax, ASPI_CmdBlock.gdt.SRB_Status
	ret
;-------------------------------------------------------------------------------
GetDeviceType	ENDP


;+-----------------------------------------------------------------------------+
;|			    GET	DRIVE ENTRY OFFSET			       |
;|	  Returns the offset of	the chosen drive's status word in EAX	       |
;+-----------------------------------------------------------------------------+
GetDriveEntryOffset	PROC	Adapter:DWORD,	Device:DWORD
;-------------------------------------------------------------------------------
	mov	eax, OFFSET DriveArray - SIZEOF DWORD
	mov	bl, byte ptr Adapter
	mov	bh, byte ptr Device
Next:	add	eax, SIZEOF DWORD
	cmp	[eax], bx	; did we	find the right table slot?
	jne	Next
	ret
;-------------------------------------------------------------------------------
GetDriveEntryOffset	ENDP


;+-----------------------------------------------------------------------------+
;|			      GET COMMAND DETAILS			       |
;|	      Given a SCSI command byte, this returns the command	       |
;|	      block length in AL  and the Command Flags	in AH.		       |
;+-----------------------------------------------------------------------------+
GetCommandDetails	PROC USES ebx	esi, command:BYTE
;-------------------------------------------------------------------------------
	mov	bl, command
	mov	ecx, LENGTH_OF_DETAILS_TABLE
	mov	esi, OFFSET CommandDetailsTable
	; search the table for	the command entry
	.REPEAT
		lodsw			; pickup a command/FLAG pair
		.BREAK .IF (al == bl)	; if we	match we're done
		.IF (ecx == 1)		; if we didn't locate	it ...
			zero	eax	; return ZERO
			jmp	Exit
		.ENDIF
	.UNTILCXZ
	mov	al, 6			; presume a short (6 byte)	command
	.IF (bl > TEN_BYTE_CMDS)	; but	if it's	a LONG one ....
		mov	al, 10		;	then return that
	.ENDIF
Exit:	ret
;-------------------------------------------------------------------------------
GetCommandDetails	ENDP


;+-----------------------------------------------------------------------------+
;|				 SCSI COMMAND				       |
;|   This executes a SCSI command through the ASPI interface. It receives a    |
;|   NEAR pointer to a standard	SCSI command block (SCB) and a pointer and     |
;|   length to an IoBuffer for the command.  It	returns	in EAX the complete    |
;|   three-byte	sense code from	the command.				       |
;+-----------------------------------------------------------------------------+
SCSICommand	PROC USES esi edi, Adapter:DWORD, Device:DWORD, lpCmdBlk:LPVOID, lpIoBuf:LPVOID, IoBufLen:DWORD
		LOCAL	AbortSRB:SRB_Abort
;-------------------------------------------------------------------------------
	ZeroASPI
	mov	ASPI_CmdBlock.io.SRB_Cmd, SC_EXEC_SCSI_CMD	 ; w: ASPI command code	= SC_EXEC_SCSI_CMD
	movmov	ASPI_CmdBlock.io.SRB_HaId, al, byte ptr	Adapter	 ; w: ASPI host	adapter	number
	movmov	ASPI_CmdBlock.io.SRB_Target, al, byte ptr Device ; w: target's SCSI ID
	movmov	ASPI_CmdBlock.io.SRB_BufPointer, eax, lpIoBuf	 ; w: data buffer	pointer
	movmov	ASPI_CmdBlock.io.SRB_BufLen, eax, IoBufLen	 ; w: data allocation length
	mov	ASPI_CmdBlock.io.SRB_SenseLen, SIZEOF ASPI_CmdBlock.io.SRB_SenseArea  ; w:	sense allocation length
	mov	ASPI_CmdBlock.io.SRB_Flags, SRB_EVENT_NOTIFY	 ; w:	SCSI request flags
	movmov	ASPI_CmdBlock.io.SRB_PostProc, eax, hCompletionEvent

	; assemble our	scsi command block
	mov	esi, lpCmdBlk		 ; get a pointer to	the scsi command
	invoke	GetCommandDetails, [esi] ; al == cmd_length, ah	== cmd_flags
	or	ASPI_CmdBlock.io.SRB_Flags,	ah  ; w: SCSI request flags
	mov	ASPI_CmdBlock.io.SRB_CDBLen, al ; w: CDB Length

	; move	the command block into the ASPI	structure
	lea	edi, ASPI_CmdBlock.io.SRB_CDBByte
	movzx	ecx, al
	rep movsb

	; call	the ASPI manager to forward the	command	to the device
	invoke	SendASPI32CommandPtr PTR lpSendASPI32Command, ADDR ASPI_CmdBlock
	invoke	WaitForSingleObject, hCompletionEvent, SCSI_TIMEOUT
	movzx	eax, ASPI_CmdBlock.io.SRB_Status

	; if the command did not generate any Sense Data, just	return NULL
	.IF (al == SS_COMP)
NoError:	zero	eax

	; else, if it's *NOT* a "Sense	Data" error (SS_ERR)
	.ELSEIF (al !=	SS_ERR)
		movzx	eax, al
		or	eax, 00FFFF00h	; [00 FF FF	er]

	; okay, we have an SS_ERR condition, let's check the SENSE DATA
	.ELSE	; assemble [00 ASC ASCQ SenseKey]
		movzx	eax, word ptr ASPI_CmdBlock.io.SRB_SenseArea[12] ; [00 00 ASCQ ASC]
		swap	ah, al					; [00	00 ASC ASCQ]
		shl	eax, 8					; [00 ASC ASCQ	00]
		mov	al, ASPI_CmdBlock.io.SRB_SenseArea[2]	; [00 ASC ASCQ xSenseKey]
		and	al, 0Fh					; [00	ASC ASCQ SenseKey]
		.IF (eax == MEDIA_CHANGE_CODE)
			invoke	GetDriveEntryOffset, Adapter,	Device
			or	dword ptr	[eax], MEDIA_CHANGED
			jmp	NoError
		.ELSEIF (!eax)	; if we had NO	SENSE error, but SS_ERR
			mov	eax, SS_ERR	; return SS_ERR
		.ENDIF
	.ENDIF
	ret
;-------------------------------------------------------------------------------
SCSICommand	ENDP


;+-----------------------------------------------------------------------------+
;|			   ENUMERATE IOMEGA DEVICES			       |
;+-----------------------------------------------------------------------------+
EnumerateIomegaDevices	PROC	USES ebx
			LOCAL	FirstInvalidAdapter:DWORD, Scsi[6]:BYTE,
				InqData[96]:CHAR, Adapter:DWORD, Device:DWORD
;-------------------------------------------------------------------------------
IFE FORCE_NO_DRIVES
	; get the total adapter count
	invoke	HostAdapterInquiry, 0
	movzx	eax, ASPI_CmdBlock.hai.HA_Count
	; see whether it completed successfully
	.IF (ASPI_CmdBlock.hai.SRB_Status != SS_COMP) || (!eax)
		jmp	Exit
	.ENDIF
	mov	FirstInvalidAdapter, eax
	reset	Adapter
	reset	DriveCount	; reset the global DriveCount	variable
	.REPEAT
		; check each adapter for "ppa3" string and small blk xfers
		invoke	HostAdapterInquiry, Adapter
		.IF (ASPI_CmdBlock.hai.SRB_Status == SS_COMP)
			; truncate at four chars so "PPA3NT"	and "ppa3" both	match!
			mov	byte ptr	ASPI_CmdBlock.hai.HA_Identifier[4], 0
			invoke	lstrcmpi, ADDR ASPI_CmdBlock.hai.HA_Identifier, ADDR szPPA3
			.IF (!eax)
				mov	eax, dword ptr ASPI_CmdBlock.hai.HA_Unique[4]
				.IF	(eax < 65536)
					set	OldPPA3Driver
				.ENDIF
			.ENDIF
		.ENDIF

		; now	scan the devices on this host adapter
		reset	Device
		.REPEAT
			invoke	GetDeviceType, Adapter, Device
			cmp	eax, SS_COMP
			jne	TryNextDrive
			;--------------------------------------------------------------------------
			varzero	Scsi
			mov	Scsi, SCSI_Cmd_Inquiry
			mov	Scsi[4],	SIZEOF InqData
			invoke	SCSICommand, Adapter,	Device,	ADDR Scsi, ADDR	InqData, SIZEOF	InqData
			check	eax
			jnz	TryNextDrive
			;--------------------------------------------------------------------------
			mov	byte ptr	InqData[14], 0
			invoke	lstrcmpi, ADDR szIomega, ADDR	InqData[8]
			check	eax
			jnz	TryNextDrive
			;--------------------------------------------------------------------------
			mov	byte ptr	InqData[19], 0
			invoke	lstrcmpi, ADDR szZip,	ADDR InqData[16]
			check	eax
			jz	FoundZipOrJaz
			;--------------------------------------------------------------------------
			invoke	lstrcmpi, ADDR szJaz,	ADDR InqData[16]
			check	eax
			jnz	TryNextDrive
			mov	eax, JAZ_DRIVE
			;--------------------------------------------------------------------------
			; check for ANSI SCSI to see	whether	we need	to play
			; the Odd/Even password length game ...
FoundZipOrJaz:		.IF !(byte ptr InqData[2] & 07h)
				or	eax, ODD_BYTE_COMPENSATION ; turn on compensation
			.ENDIF
			mov	ebx, DriveCount
			mov	DriveArray[ebx*4], eax
			movmov	byte ptr DriveArray[ebx*4+0],	al, byte ptr Adapter
			movmov	byte ptr DriveArray[ebx*4+1],	al, byte ptr Device
			inc	ebx
			mov	DriveCount, ebx
			cmp	ebx, MAX_DRIVE_COUNT
			jae	Exit
			;--------------------------------------------------------------------------
TryNextDrive:		inc	Device
		.UNTIL (Device >= 16)
		inc	Adapter
		mov	eax, Adapter
	.UNTIL	(eax >=	FirstInvalidAdapter)
;-------------------------------------------------------------------------------
ENDIF
Exit:	.IF (!DriveCount)
		set	ErrorMode
	.ENDIF
	mov	eax, DriveCount
	ret
;-------------------------------------------------------------------------------
EnumerateIomegaDevices	ENDP


;+-----------------------------------------------------------------------------+
;|				 GET MODE PAGE				       |
;+-----------------------------------------------------------------------------+
GetModePage	PROC	Adapter:DWORD,	Device:DWORD, PageToGet:DWORD, pBuffer:PTR, BufLen:DWORD
		LOCAL	Scsi[6]:BYTE
;-------------------------------------------------------------------------------
	varzero	Scsi
	mov	Scsi, SCSI_Cmd_ModeSense
	movmov	Scsi[2], al, byte ptr PageToGet
	movmov	Scsi[4], al, byte ptr BufLen
	invoke	SCSICommand, Adapter, Device, ADDR Scsi, pBuffer, BufLen
	ret
;-------------------------------------------------------------------------------
GetModePage	ENDP


;+-----------------------------------------------------------------------------+
;|				 SET MODE PAGE				       |
;+-----------------------------------------------------------------------------+
SetModePage	PROC USES ebx, Adapter:DWORD, Device:DWORD,	pBuffer:PTR
		LOCAL	Scsi[6]:BYTE
;-------------------------------------------------------------------------------
	varzero	Scsi			; init the SCSI	parameter block
	mov	ebx, pBuffer		; get a pointer to the top of buffer
	movzx	ecx, byte ptr [ebx]	; get the total length
	inc	ecx			; adjust it up by one
	reset	byte ptr	[ebx]		; now clear the two reserved bytes
	reset	byte ptr	[ebx+2]	;
	mov	Scsi, SCSI_Cmd_ModeSelect	; set the command
	mov	Scsi[1], 10h			; set the Page Format bit
	mov	Scsi[4], cl			; set the parameter list length
	invoke	SCSICommand, Adapter, Device, ADDR Scsi, pBuffer, ecx
	ret
;-------------------------------------------------------------------------------
SetModePage	ENDP


;+-----------------------------------------------------------------------------+
;|			      SET ERROR	RECOVERY			       |
;+-----------------------------------------------------------------------------+
SetErrorRecovery	PROC USES ebx,	Retries:BOOL, ECC:BOOL,	Testing:BOOL
			LOCAL	PageBuff[40]:BYTE
;-------------------------------------------------------------------------------
	invoke	GetModePage, CurrentAdapter, CurrentDevice, ERROR_RECOVERY_PAGE, ADDR PageBuff,	SIZEOF PageBuff
	lea	ebx, PageBuff[4]	;	get just past the header address
	movzx	eax, PageBuff[3]	; get the Block	Descriptor Length
	; form	ebx == the offset to the top of	the page we've read ...
	add	ebx, eax
	; always turn off the PS bit (parameters savable)
	and	byte ptr [ebx], NOT 80h
	; set the ECC fields
	mov	al, 0C1h		; presume ECC suppression
	.IF (ECC)
		mov	al, 0C8h	; enable	ECC and	Early Recovery
		.IF (Testing)
			mov	al, 0CCh	; we're	testing, so EER	& PER
		.ENDIF
	.ENDIF
	mov	[ebx+2], al		; and	set the	value
	; set the retry counts
	mov	al, 16h			; set retries to	22 for Zip drive
	.IF (JazDrive)
		mov	al, 64h		; and to	100 for	Jaz drive
	.ENDIF
	.IF (!Retries)			; but	if we have no retries ...
		zero	al		; then kill it.
	.ENDIF
	mov	byte ptr [ebx+3], al	; set	the common retry count
	.IF (byte ptr [ebx+1] > 6)	; if we have a large format	page ...
		mov	byte ptr [ebx+8],	al	; then set the write	count too
	.ENDIF
	invoke	SetModePage, CurrentAdapter, CurrentDevice, ADDR PageBuff

	; if we had an	invalid	field in the CDB (the EER bit was on)
	.IF (eax == 00260005h)
		invoke	GetModePage, CurrentAdapter, CurrentDevice, ERROR_RECOVERY_PAGE, ADDR PageBuff, SIZEOF	PageBuff
		lea	ebx, PageBuff[4]	; get just past the header address
		movzx	eax, PageBuff[3]	; get the Block Descriptor Length
		; form ebx ==	the offset to the top of the page we've	read ...
		add	ebx, eax
		; always turn	off the	PS bit (parameters savable)
		and	byte ptr [ebx], NOT 80h
		; set	the ECC	fields
		mov	al, 0C1h		; presume ECC suppression
		.IF (ECC)
			mov	al, 0C0h	; enable ECC *BUT*NOT* Early Recovery
			.IF (Testing)
				mov	al, 0C4h ; we're testing, PER
			.ENDIF
		.ENDIF
		mov	[ebx+2], al		; and set the value
		; set	the retry counts
		mov	al, 16h			; set retries to 22 for	Zip drive
		.IF (JazDrive)
			mov	al, 64h		; and to 100 for Jaz drive
		.ENDIF
		.IF (!Retries)			; but if we have no retries ...
			zero	al		; then kill	it.
		.ENDIF
		mov	byte ptr [ebx+3],	al	; set the common retry count
		.IF (byte ptr	[ebx+1]	> 6)	; if we have a large format page ...
			mov	byte ptr	[ebx+8], al	; then set the write count too
		.ENDIF
		invoke	SetModePage, CurrentAdapter, CurrentDevice, ADDR PageBuff
	.ENDIF
	ret
;-------------------------------------------------------------------------------
SetErrorRecovery	ENDP


;-------------------------------------------------------------------------------
COMMENT	~
;+-----------------------------------------------------------------------------+
;|			       VENDOR SPECIFIC				       |
;+-----------------------------------------------------------------------------+
VendorSpecific	PROC USES ebx, EnableVS:BOOL
		LOCAL	PageBuff[40]:BYTE
;-------------------------------------------------------------------------------
	invoke	GetModePage, CurrentAdapter, CurrentDevice, MODES_OF_OPERATION,	ADDR PageBuff, SIZEOF PageBuff
	lea	ebx, PageBuff[4]	;	get just past the header address
	movzx	eax, PageBuff[3]	; get the Block	Descriptor Length
	add	ebx, eax
	; now ebx has the offset to the top of	the page we've read ...
	.IF (EnableVS)
		or	byte ptr [ebx+2], 20h	; VSC
	.ELSE
		and	byte ptr [ebx+2],	NOT 20h	; turn off VSC bit
	.ENDIF
	and	byte ptr [ebx], NOT 80h		;	turn off the PS	bit
	invoke	SetModePage, CurrentAdapter, CurrentDevice, ADDR PageBuff
	ret
;-------------------------------------------------------------------------------
VendorSpecific	ENDP

;+-----------------------------------------------------------------------------+
;|				DISABLE	CACHING				       |
;+-----------------------------------------------------------------------------+
DisableCaching	PROC USES ebx, Adapter:DWORD, Device:DWORD
		LOCAL	PageBuff[40]:BYTE
;-------------------------------------------------------------------------------
	invoke	GetModePage, Adapter, Device, CACHING_PAGE, ADDR PageBuff, SIZEOF PageBuff
	lea	ebx, PageBuff[4]	;	get just past the header address
	movzx	eax, PageBuff[3]	; get the Block	Descriptor Length
	add	ebx, eax
	; now ebx has the offset to the top of	the page we've read ...
	mov	byte ptr [ebx+2], 1	; disable all caching
	invoke	SetModePage, Adapter, Device, ADDR PageBuff
	ret
;-------------------------------------------------------------------------------
DisableCaching	ENDP
;-------------------------------------------------------------------------------
COMMENT	~
;-------------------------------------------------------------------------------


;+-----------------------------------------------------------------------------+
;|			    GET	NON-SENSE PAGE DATA			       |
;|-----------------------------------------------------------------------------|
;|  Given Adapter, Device, DataPage, and a Buffer to receive the data, this    |
;|  fills the buffer we're given and returns with the SCSI Completion Code     |
;+-----------------------------------------------------------------------------+
GetNonSenseData	PROC	Adapter:DWORD, Device:DWORD, DataPage:DWORD, Buffer:DWORD,	BufLen:DWORD
		LOCAL	Scsi[6]:BYTE
;-------------------------------------------------------------------------------
	varzero	Scsi
	mov	Scsi, SCSI_Cmd_NonSenseData	; do a	Non-Sense Data Read
	movmov	Scsi[2], al, byte ptr DataPage	; which page to read
	movmov	Scsi[4], al, byte ptr BufLen	; tell drive page is this long
	invoke	SCSICommand, Adapter, Device, ADDR Scsi, Buffer, BufLen
	ret
;-------------------------------------------------------------------------------
GetNonSenseData	ENDP


;+-----------------------------------------------------------------------------+
;|			       UNLOCK ALL MEDIA				       |
;+-----------------------------------------------------------------------------+
UnlockAllMedia	PROC USES esi ebx
		LOCAL	DrivesLeft:DWORD, Scsi[6]:BYTE
;-------------------------------------------------------------------------------
	; make	sure the media is not locked as	we exit	...
	varzero	Scsi
	mov	Scsi, SCSI_Cmd_PreventAllow
	; setup the SCSI command block	for the	operation
	movmov	DrivesLeft, eax, DriveCount
	.IF (eax)
		mov	esi, OFFSET DriveArray
		.REPEAT
			movzx	ebx, byte ptr [esi]	; pickup the Adapter
			movzx	ecx, byte ptr [esi+1]	; pickup	the DeviceID
			invoke	SCSICommand, ebx, ecx, ADDR Scsi, NULL, 0
			add	esi, SIZEOF DWORD
			dec	DrivesLeft
		.UNTIL (zero?)
	.ENDIF
	ret
;-------------------------------------------------------------------------------
UnlockAllMedia	ENDP


;+-----------------------------------------------------------------------------+
;|			      LOCK CURRENT DRIVE			       |
;+-----------------------------------------------------------------------------+
LockCurrentDrive	PROC
			LOCAL	Scsi[6]:BYTE
;-------------------------------------------------------------------------------
	; lock	the media to prevent its ejection
	varzero	Scsi
	mov	Scsi, SCSI_Cmd_PreventAllow
	mov	Scsi[4], 1	; set to ONE to	lock the drive
	invoke	SCSICommand, CurrentAdapter, CurrentDevice, ADDR Scsi, NULL, 0
	ret
;-------------------------------------------------------------------------------
LockCurrentDrive	ENDP


;+-----------------------------------------------------------------------------+
;|			   SPIN	UP IOMEGA CARTRIDGE			       |
;+-----------------------------------------------------------------------------+
SpinUpIomegaCartridge	PROC	Adapter:DWORD, Device:DWORD
			LOCAL	Scsi[6]:BYTE
;-------------------------------------------------------------------------------
	; issue an Asynchronous START command to induce spinup
	varzero	Scsi
	mov	Scsi[0], SCSI_Cmd_StartStopUnit
	mov	Scsi[1], 1	; set the IMMED	bit for	offline
	mov	Scsi[4], 1	; start	the disk spinning
	invoke	SCSICommand, Adapter, Device, ADDR Scsi, NULL, 0
	ret
;-------------------------------------------------------------------------------
SpinUpIomegaCartridge	ENDP


;+-----------------------------------------------------------------------------+
;|				EJECT ALL MEDIA				       |
;+-----------------------------------------------------------------------------+
EjectAllMedia	PROC USES	esi ebx
		LOCAL	DrivesLeft:DWORD
;-------------------------------------------------------------------------------
	; setup the SCSI command block	for the	operation
	movmov	DrivesLeft, eax, DriveCount
	.IF (eax)
		mov	esi, OFFSET DriveArray
		.REPEAT
			movzx	eax, byte ptr [esi]	; pickup the Adapter
			movzx	ebx, byte ptr [esi+1]	; pickup	the DeviceID
			invoke	EjectIomegaCartridge,	eax, ebx
			add	esi, SIZEOF DWORD
			dec	DrivesLeft
		.UNTIL (zero?)
	.ENDIF
	ret
;-------------------------------------------------------------------------------
EjectAllMedia	ENDP


;+-----------------------------------------------------------------------------+
;|			    GET	SPARE SECTOR COUNTS			       |
;|-----------------------------------------------------------------------------|
;|   This returns NON-ZERO if we have trouble and posted the error message     |
;|   into the RichText control,	else it	sets the number	of spares available    |
;+-----------------------------------------------------------------------------+
GetSpareSectorCounts	PROC USES ebx, CheckPassword:BOOL
			LOCAL	Scsi[10]:BYTE,	DefectHeader:DEFECT_LIST_HEADER, DiskStat[72]:BYTE
;-------------------------------------------------------------------------------
	; ask for the defect list to make sure	we're able to read it
ListChk:varzero	Scsi
	mov	Scsi[0], SCSI_Cmd_ReadDefectData
	mov	Scsi[2], 00011110b	; defect format, G/P bits
	mov	Scsi[8], 4		; ask for only	FOUR bytes
	invoke	SCSICommand, CurrentAdapter, CurrentDevice, ADDR Scsi, ADDR DefectHeader, SIZEOF DefectHeader
	.IF (!eax) || (eax == INCOMPATIBLE_MEDIA)
		; we could read its defect list ... so show it!
		invoke	GetNonSenseData, CurrentAdapter, CurrentDevice, DISK_STATUS_PAGE, ADDR	DiskStat, SIZEOF DiskStat
		check	eax
		jnz	ListChk
		;--------------------------------------------------------------------------
		zero	ch	; clear the DRIVE_A_SUPPORT
		.IF (JazDrive)
			movzx	eax, word ptr DiskStat[JAZ_SPARES_COUNT_OFFSET]
			zero	ebx
			mov	cl, DiskStat[JAZ_PROTECT_MODE_OFFSET]
			mov	edx, dword ptr DiskStat[JAZ_LAST_LBA_OFFSET]
		.ELSE
			.IF (DiskStat == DISK_STATUS_PAGE)
				movzx	eax, word ptr	DiskStat[NEW_ZIP_SIDE_0_SPARES_COUNT_OFFSET]
				movzx	ebx, word ptr	DiskStat[NEW_ZIP_SIDE_1_SPARES_COUNT_OFFSET]
				mov	cl, DiskStat[NEW_ZIP_PROTECT_MODE_OFFSET]
				mov	edx, dword ptr DiskStat[NEW_ZIP_LAST_LBA_OFFSET]
				dec	ch ; set the DRIVE_A_SUPPORT
			.ELSE
				movzx	eax, word ptr	DiskStat[OLD_ZIP_SIDE_0_SPARES_COUNT_OFFSET]
				movzx	ebx, word ptr	DiskStat[OLD_ZIP_SIDE_1_SPARES_COUNT_OFFSET]
				mov	cl, DiskStat[OLD_ZIP_PROTECT_MODE_OFFSET]
				mov	edx, dword ptr DiskStat[OLD_ZIP_LAST_LBA_OFFSET]
			.ENDIF
			check	ebx
			jz	NoSpares
		.ENDIF
		;컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴
;		bswap	edx		; save the last	LBA in any event
		ror	edx, 8	; [dabc]
		swap	dh, dl	;	[dacb]
		ror	edx, 16	;	[cbda]
		swap	dh, dl	;	[cbad]
		ror	edx, 8	; [dcba]
		;컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴
		.IF (ch)
			sub	edx, DRIVE_A_SUPPORT_BIAS
		.ENDIF
		mov	LastLBAOnCartridge, edx
		swap	al, ah		; make it little	endian
		mov	Side_0_SparesCount, eax
		swap	bl, bh		; make it little	endian
		mov	Side_1_SparesCount, ebx
		; compute the	number of troubles we encountered during the testing
		push	eax
		mov	eax, Initial_Side_0_Spares
		sub	eax, Side_0_SparesCount
		add	eax, Initial_Side_1_Spares
		sub	eax, Side_1_SparesCount
		mov	FirmErrors, eax
		pop	eax
		; check to see whether we have ANY spare sectors remaining
		.IF (!eax)
NoSpares:		mov	CartridgeStatus,	DISK_TEST_FAILURE
			mov	eax, OFFSET szNoSpares
			; if	were running give them a different error message
			.IF (TestingPhase)
				mov	eax, OFFSET szOutOfSpares
			.ENDIF
			invoke	SetRichEditText, ADDR	hTabText, eax
			jmp	ErrorExit
		.ENDIF
		; now	let's checkout the cartridge's protection mode ...
		.IF (cl & 07h) && (!(cl & 08h)) && CheckPassword ; if	we have	some protection	mode
			; if	it's simply WRITE protected ...	disable	that ...
			.IF (cl == 2)
				invoke	MessageBox, hMainWnd, ADDR szDisablingWPText, ADDR szDisablingWPTitle, MB_APPLMODAL or MB_OK
				mov	CartridgePassword, 0	; set pswd	to null
				jmp	TempDisablePswd
			.ENDIF
			; we	need a password	from the operator ...
			invoke	DialogBoxParam, hInst, ADDR szAppName, hMainWnd, ADDR	PasswordWndProc, 0
			.IF (eax)
TempDisablePswd:		invoke	GetDriveEntryOffset, CurrentAdapter, CurrentDevice
				push	eax		;	save the table entry
				varzero	Scsi
				mov	Scsi, SCSI_Cmd_CartProtect
				mov	Scsi[1], 08h	; request temporary unlock	mode
				invoke	lstrlen, ADDR CartridgePassword
				mov	Scsi[4], al	; set THIS to the true transfer length
				pop	ebx		; recover the entry offset
				.IF	(dword ptr [ebx] & ODD_BYTE_COMPENSATION) && (al & 1) ;	if password is ODD
					or	Scsi[1], 10h	; set the WA bit!
					inc	eax	; and round it up!
				.ENDIF
				invoke	SCSICommand,	CurrentAdapter,	CurrentDevice, ADDR Scsi, ADDR CartridgePassword, eax
				.IF	(eax ==	SCSI_CMD_TIMED_OUT)
					invoke	MessageBox,	hMainWnd, ADDR szCantUnlockText, ADDR szCantUnlockTitle, MB_APPLMODAL or MB_OK
					jmp	StillLocked
				.ENDIF
				jmp	ListChk
			.ENDIF
StillLocked:		mov	CartridgeStatus, DISK_PROTECTED
			invoke	SetRichEditText, ADDR	hTabText, ADDR szLocked
			jmp	ErrorExit
		.ENDIF
		zero	eax	; return zero since no error
		;컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴
	.ELSE	; trouble of some sort ... so suppress controls and
		; show the richedit control for the trouble
		.IF (eax == DEFECT_LIST_READ_ERROR)
			mov	CartridgeStatus,	DISK_Z_TRACK_FAILURE
			invoke	SetRichEditText, ADDR	hTabText, ADDR szDefectList
ErrorExit:		zero	eax
			dec	eax	; return NON-ZERO since we have an error!
		.ELSEIF (eax == MEDIA_NOT_PRESENT)
			mov	CartridgeStatus,	DISK_NOT_PRESENT
IF DEVELOPMENT
;		.ELSE
;			invoke	PostToScreen, eax
ENDIF
		.ENDIF
	.ENDIF
Exit:	ret
;-------------------------------------------------------------------------------
GetSpareSectorCounts	ENDP


;+-----------------------------------------------------------------------------+
;|			     HANDLE DRIVE CHANGING			       |
;|-----------------------------------------------------------------------------|
;|   If	we're NOT in the middle	of drive testing, check	for any	new drives     |
;|   going ready, eject	all others, and	Select the newer drive.		       |
;|   If	we *ARE* in the	middle of drive	testing, EJECT any new drive that's    |
;|   attempting	to go ready.						       |
;+-----------------------------------------------------------------------------+
HandleDriveChanging	PROC USES esi edi ebx
			LOCAL	DrivesLeft:DWORD, DiskStat[72]:BYTE, Selecting:BOOL, TroubleFlag:DWORD, PriorStatus:DWORD
;-------------------------------------------------------------------------------
	test	hASPIDLL,	-1	; don't do ANYTHING if we didn't find
	jz	Exit		; the	required ASPI goodies in the system
	;--------------------------------------------------------------------------
	reset	Selecting	; true	while we're changing selections
Rescan:	movmov	DrivesLeft, eax,	DriveCount
	check	eax
	jz	Exit
	;--------------------------------------------------------------------------
	mov	esi, OFFSET DriveArray
	.REPEAT	; query the current state of the drive
		.REPEAT	; clear media	changed	status
			and	dword ptr [esi],	NOT MEDIA_CHANGED
			movzx	ebx, byte ptr [esi]	; pickup the Adapter
			movzx	ecx, byte ptr [esi+1]	; pickup	the DeviceID
			invoke	GetNonSenseData, ebx,	ecx, DISK_STATUS_PAGE, ADDR DiskStat, SIZEOF DiskStat
		.UNTIL !(dword ptr [esi] & MEDIA_CHANGED) ; do it until NO media change!
		;--------------------------------------------------------------------------
		.IF (DiskStat	== DISK_STATUS_PAGE)
			movzx	eax, DiskStat[NEW_DISK_STATUS_OFFSET]
		.ELSE
			movzx	eax, DiskStat[OLD_DISK_STATUS_OFFSET]
		.ENDIF
		; if the device we have is NOT the currently selected	one
		movzx	ebx, byte ptr [esi]	; pickup the Adapter
		movzx	ecx, byte ptr [esi+1]	;	pickup the DeviceID
		.IF (ebx != CurrentAdapter) || (ecx != CurrentDevice)
			; if	the disk is ANYTHING other than	not present ...
			.IF (al != DISK_NOT_PRESENT)
				; we have a	PRESENT	DISK in	a non-current drive
				; if we're testing,	reject it
				.IF	(Selecting) || (TestingPhase >=	TESTING_STARTUP)
					invoke	EjectIomegaCartridge, ebx, ecx
					; flag that we're waiting for spindown
					or	dword ptr [esi], DISK_EJECTING
				; if we're not testing, and	not awaiting eject
				; then set the current drive ...
				.ELSEIF !(dword ptr	[esi] &	DISK_EJECTING)
					mov	CurrentAdapter, ebx
					mov	CurrentDevice,	ecx
					reset	TestingPhase	; show idle
					set	Selecting
					jmp	Rescan
				; the PREVIOUS drive (if any) will be ejected on the next pass
				.ENDIF
			.ELSE	; the drive HAS spun down, so clear "waiting"
				and	dword ptr [esi], NOT DISK_EJECTING
			.ENDIF
		.ELSE	; we're	checking the current drive ... make SURE that
			; it	is *NOT* empty!	 If it *IS* empty, kill	current
			.IF (al == DISK_NOT_PRESENT)
				mov	CurrentAdapter,	-1
				mov	CurrentDevice, -1
				call	SetCartridgeStatusToEAX
			.ENDIF
			; if	it's not already set correctly *and* either
			; the cart status is	one of the pre-test ones, or
			; the NEW status from the cart is NOT "at speed" ...
			.IF (eax != CartridgeStatus)	&& ((CartridgeStatus <=	DISK_STALLED) || (eax != DISK_AT_SPEED))
				call	SetCartridgeStatusToEAX
			.ENDIF
		.ENDIF
		add	esi, SIZEOF DWORD
		dec	DrivesLeft
	.UNTIL	(zero?)
	; if nothing was chosen ... set us to "Awaiting Cartridge" status
	.IF (CurrentAdapter ==	(-1)) && (CurrentDevice	== (-1)) && (al	== DISK_NOT_PRESENT) &&	(CartridgeStatus != DISK_NOT_PRESENT)
		call	SetCartridgeStatusToEAX
	.ENDIF
Exit:	ret

;===============================================================================
SetCartridgeStatusToEAX:
;-------------------------------------------------------------------------------
	reset	JazDrive
	.IF (dword ptr	[esi] &	JAZ_DRIVE)
		set	JazDrive
	.ENDIF
	push	eax
	push	esi
	reset	TroubleFlag			; presume everything's okay
	movmov	PriorStatus, ebx, CartridgeStatus
	mov	CartridgeStatus, eax

	; set the text	of the "action initiate	button"
	.IF	(CartridgeStatus == DISK_SPUN_DOWN)
		; set	the button to "Start Disk Spinning"
		mov	esi, OFFSET szPressToSpin

	.ELSEIF	(CartridgeStatus == DISK_TEST_UNDERWAY)
		; set	the button to "Stop Testing"
		mov	esi, OFFSET szPressToStop

	.ELSEIF	(CartridgeStatus == DISK_NOT_PRESENT)
Ejecting:	invoke	SetRichEditText, ADDR hTabText, ADDR szNotRunning
		jmp	DisableActions

	.ELSEIF	(CartridgeStatus == DISK_AT_SPEED)
		invoke	GetSpareSectorCounts, TRUE ; update the Cart Condition
		cmp	eax, MEDIA_NOT_PRESENT
		je	Ejecting
		;--------------------------------------------------------------------------
		mov	TroubleFlag, eax	; decide	whether	we're in trouble
		mov	esi, OFFSET szPressToEject ; presume trouble
		.IF (!eax)
			movmov	Initial_Side_0_Spares, eax, Side_0_SparesCount
			movmov	Initial_Side_1_Spares, eax, Side_1_SparesCount
			reset	FirmErrors
			; check to see if we	have enough spares to start
			.IF (JazDrive)
				cmp	Side_0_SparesCount, MINIMUM_JAZ_SPARES
				jb	InsufficientSpares
			.ELSE
				cmp	Side_0_SparesCount, MINIMUM_ZIP_SPARES
				jb	InsufficientSpares
				.IF	(Side_1_SparesCount < MINIMUM_ZIP_SPARES)
InsufficientSpares:			invoke	SetRichEditText, ADDR hTabText, ADDR szFewSpares
					mov	CartridgeStatus, DISK_LOW_SPARES
					set TroubleFlag	; show the	richedit control
					mov	esi, OFFSET szPressToProceed
					jmp	EnableTestBtn
				.ENDIF
			.ENDIF
			; if	no trouble, get	ready to start testing ...
			call	PrepareToBeginTesting
			mov	esi, OFFSET szPressToStart
		.ENDIF
		; the	disk *IS* at speed so enable the action	button!
EnableTestBtn:	invoke	EnableWindow, hTestButton, TRUE
		.IF (eax)
			mov	ActionButtonFlasher, FLASH_COUNT
			invoke	SetFocus, hTestButton
		.ENDIF

	.ELSE	; set the button	to "One	Moment Please"

DisableActions:	invoke	EnableWindow, hTestButton, FALSE
		mov	esi, OFFSET szOneMoment

	.ENDIF
	; set the Window's text based upon setting of esi
	invoke	SetWindowText, hTestButton, esi
	; based upon the TroubleFlag, show them the proper page set
	invoke	SetTabErrorMode, TroubleFlag
	; and if CartridgeStatus has changed, refresh the entire panel!
	.IF (CurrentPage == PERFORM_TEST_PAGE)
		zero	ecx
		.IF (PriorStatus == DISK_AT_SPEED) ||	(PriorStatus ==	DISK_SPUN_DOWN)	|| (PriorStatus	>= DISK_LOW_SPARES)
			not	ecx
		.ENDIF
		.IF (CartridgeStatus == DISK_AT_SPEED) || (CartridgeStatus >=	DISK_LOW_SPARES)
			not	ecx
		.ENDIF
		.IF (ecx)	; update the entire	panel's	data
			invoke	InvalidateRect, hTestMonitor,	NULL, FALSE
		.ELSE	; only paint the new cartridge status
			invoke	GetDC, hTestMonitor
			mov	edi, eax
			invoke	PaintCartStatus, edi
			invoke	ReleaseDC, hTestMonitor, edi
		.ENDIF
	; auto	change to the first tab	when changing disks.
;	.ELSEIF	(CurrentPage ==	EXPLAIN_RESULTS)
;		.IF (ebx == DISK_NOT_PRESENT) || (eax == DISK_NOT_PRESENT)
;			invoke	SendMessage, hActionTabs, TCM_SETCURSEL, 0, NULL
;			.IF (eax)
;				mov	CurrentPage, PERFORM_TEST_PAGE
;				mov	ActionsSubPage,	PERFORM_TEST_PAGE
;				call	SetCurrentWindow
;			.ENDIF
;		.ENDIF
	.ENDIF
;-------------------------------------------------------------------------------
	pop	esi
	pop	eax
	LocalReturn

;-------------------------------------------------------------------------------
HandleDriveChanging	ENDP


;+-----------------------------------------------------------------------------+
;|			  GET ELAPSED TIME IN SECONDS			       |
;|	  Returns seconds elapsed, or -1 if we could not determine it	       |
;+-----------------------------------------------------------------------------+
GetElapsedTimeInSeconds	PROC USES ebx
			LOCAL	CurrentInstant:FILETIME
;-------------------------------------------------------------------------------
	invoke	GetSystemTimeAsFileTime, ADDR CurrentInstant
	mov	eax, dword	ptr CurrentInstant
	mov	edx, dword	ptr CurrentInstant[4]
	sub	eax, dword	ptr StartingInstant
	sbb	edx, dword	ptr StartingInstant[4]
	mov	ebx, FILE_TIME_TICKS_PER_SECOND
	; verify that the result won't	overflow
	.IF (edx < ebx)
		roundiv	ebx
	.ELSE
		mov	eax, -1
	.ENDIF
	ret
;-------------------------------------------------------------------------------
GetElapsedTimeInSeconds	ENDP


;+-----------------------------------------------------------------------------+
;|			   PREPARE TO BEGIN TESTING			       |
;+-----------------------------------------------------------------------------+
PrepareToBeginTesting	PROC
;-------------------------------------------------------------------------------
	; zero	all of the testing variables
	zero	eax
	mov	ecx, TESTING_VARIABLE_COUNT
	mov	edi, OFFSET FirstTestingVariable
	rep stosd
	; catch up the	initial	spares
	movmov	Initial_Side_0_Spares, eax, Side_0_SparesCount
	movmov	Initial_Side_1_Spares, eax, Side_1_SparesCount
	reset	FirmErrors
	; set the flag	to enable the "test" button to call "TestTheDisk"
	mov	TestingPhase, READY_TO_TEST
	ret
;-------------------------------------------------------------------------------
PrepareToBeginTesting	ENDP


;+-----------------------------------------------------------------------------+
;|			       BUMP ERROR COUNTS			       |
;+-----------------------------------------------------------------------------+
BumpErrorCounts	PROC USES ebx, ErrorCode:DWORD
;-------------------------------------------------------------------------------
	mov	eax, ErrorCode		; get the error code
IF DEVELOPMENT
	invoke	PostToScreen, eax
ENDIF
	.IF (eax == BUFFER_TOO_BIG)	; if we got BUFFER	TOO BIG, halt!
		set	UserInterrupt
	.ENDIF
	mov	ebx, eax
	and	ebx, 00FF00FFh		; mask off	the middle byte
	.IF (ebx == 00150004h)		; if it was one of the	many seek
		mov	eax, ebx	; errors, cvrt to seek error
	.ENDIF
	.IF (eax)
		mov	LastError, eax
	.ENDIF
	.IF (eax == 320003h) || (eax == 328F03h)
		mov	CartridgeStatus, DISK_LOW_SPARES
	.ENDIF
	.IF (al == 1)			; recovered error
		inc	SoftErrors
	.ELSE
		inc	HardErrors
	.ENDIF
	ret
;-------------------------------------------------------------------------------
BumpErrorCounts	ENDP


;+-----------------------------------------------------------------------------+
;|				 READ SECTORS				       |
;+-----------------------------------------------------------------------------+
ReadSectors	PROC	pBuffer:LPSTR,	LBAtoRead:DWORD, LBACount:DWORD
		LOCAL	Scsi[10]:BYTE
;-------------------------------------------------------------------------------
	varzero	Scsi
	mov	Scsi, SCSI_Cmd_ReadMany
	mov	eax, LBAtoRead
	;컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴
;	bswap	eax	; [abcd]	; convert it into BIG endian format
	ror	eax, 8	; [dabc]
	swap	ah, al	; [dacb]
	ror	eax, 16	; [cbda]
	swap	ah, al	; [cbad]
	ror	eax, 8	; [dcba]
 	;컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴
	mov	dword ptr Scsi[2],	eax
	mov	eax, LBACount		; and the NUMBER of	LBA's to read
	swap	ah, al			; convert it into BIG endian format
	mov	word ptr Scsi[7], ax
	swap	ah, al
	shl	eax, LOG2_BYTES_PER_SECTOR
	invoke	SCSICommand, CurrentAdapter, CurrentDevice, ADDR Scsi, pBuffer,	eax
	ret
;-------------------------------------------------------------------------------
ReadSectors	ENDP


;+-----------------------------------------------------------------------------+
;|				 WRITE SECTORS				       |
;+-----------------------------------------------------------------------------+
WriteSectors	PROC	pBuffer:LPSTR, LBAtoWrite:DWORD, LBACount:DWORD
		LOCAL	Scsi[10]:BYTE
;-------------------------------------------------------------------------------
	varzero	Scsi
	mov	Scsi, SCSI_Cmd_WriteMany
	mov	eax, LBAtoWrite
	;컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴
;	bswap	eax	; [abcd]	; convert it into BIG endian format
	ror	eax, 8	; [dabc]
	swap	ah, al	; [dacb]
	ror	eax, 16	; [cbda]
	swap	ah, al	; [cbad]
	ror	eax, 8	; [dcba]
 	;컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴
	mov	dword ptr Scsi[2],	eax
	mov	eax, LBACount		; and the NUMBER of	LBA's to read
	swap	ah, al			; convert it into BIG endian format
	mov	word ptr Scsi[7], ax
	swap	ah, al
	shl	eax, LOG2_BYTES_PER_SECTOR
	invoke	SCSICommand, CurrentAdapter, CurrentDevice, ADDR Scsi, pBuffer,	eax
	ret
;-------------------------------------------------------------------------------
WriteSectors	ENDP


;-/////////////////////////////////////////////////////////////////////////////-
IF COPY_BUTTON_DAMAGES
;-/////////////////////////////////////////////////////////////////////////////-

LONG_IO_XFER		equ	564	;	(512+52) for Zip drives
; LONG_IO_XFER		equ	550	; (512+38) for Jaz drives

FIRST_SECTOR_TO_HURT	equ	2246
LAST_SECTOR_TO_HURT	equ	100000
SECTOR_HURT_SPACING	equ	527


;+-----------------------------------------------------------------------------+
;|			       READ SECTOR LONG				       |
;+-----------------------------------------------------------------------------+
ReadSectorLong	PROC	pBuffer:LPSTR, LBAtoRead:DWORD
		LOCAL	Scsi[10]:BYTE
;-------------------------------------------------------------------------------
	varzero	Scsi
	mov	Scsi, SCSI_Cmd_ReadLong
	mov	eax, LBAtoRead
	;컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴
;	bswap	eax			; convert it into BIG endian format
	ror	eax, 8	; [dabc]
	swap	ah, al	; [dacb]
	ror	eax, 16	; [cbda]
	swap	ah, al	; [cbad]
	ror	eax, 8	; [dcba]
	;컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴
	mov	dword ptr Scsi[2],	eax
	mov	ax, LONG_IO_XFER
	swap	ah, al
	mov	word ptr Scsi[7], ax	; read LONG number of	bytes
	invoke	SCSICommand, CurrentAdapter, CurrentDevice, ADDR Scsi, pBuffer,	LONG_IO_XFER
	ret
;-------------------------------------------------------------------------------
ReadSectorLong	ENDP


;+-----------------------------------------------------------------------------+
;|			       WRITE SECTOR LONG			       |
;+-----------------------------------------------------------------------------+
WriteSectorLong	PROC	LBAtoWrite:DWORD, pBuffer:LPSTR
		LOCAL	Scsi[10]:BYTE
;-------------------------------------------------------------------------------
	varzero	Scsi
	mov	Scsi, SCSI_Cmd_WriteLong
	mov	eax, LBAtoWrite
	;컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴
;	bswap	eax			; convert it into BIG endian format
	ror	eax, 8	; [dabc]
	swap	ah, al	; [dacb]
	ror	eax, 16	; [cbda]
	swap	ah, al	; [cbad]
	ror	eax, 8	; [dcba]
	;컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴
	mov	dword ptr Scsi[2],	eax
	mov	ax, LONG_IO_XFER
	swap	ah, al
	mov	word ptr Scsi[7], ax	; read LONG number of	bytes
	invoke	SCSICommand, CurrentAdapter, CurrentDevice, ADDR Scsi, pBuffer,	LONG_IO_XFER
	ret
;-------------------------------------------------------------------------------
WriteSectorLong	ENDP


;+-----------------------------------------------------------------------------+
;|				DAMAGE THE DISK				       |
;+-----------------------------------------------------------------------------+
DamageTheDisk	PROC USES	esi edi
		LOCAL	IOBuffer[LONG_IO_XFER]:BYTE
;-------------------------------------------------------------------------------
	mov	esi, FIRST_SECTOR_TO_HURT
	.WHILE	(esi <=	LAST_SECTOR_TO_HURT)

		; first we repair any	prior damage
		invoke	ReadSectorLong, ADDR IOBuffer,	esi
		invoke	WriteSector, esi, ADDR	IOBuffer
		invoke	ReadSectorLong, ADDR IOBuffer,	esi

		; zap	the buffer
		lea	edi, IOBuffer
		xor	dword ptr	[edi + 100], 0FFFFFFFFh
		xor	dword ptr	[edi + 104], 0FFFFFFFFh
		xor	dword ptr	[edi + 108], 0FFFFFFFFh
		xor	dword ptr	[edi + 112], 0FFFFFFFFh
		xor	dword ptr	[edi + 116], 0FFFFFFFFh
		xor	dword ptr	[edi + 120], 0FFFFFFFFh
		xor	dword ptr	[edi + 124], 0FFFFFFFFh
		xor	dword ptr	[edi + 128], 0FFFFFFFFh
		xor	dword ptr	[edi + 132], 0FFFFFFFFh
		xor	dword ptr	[edi + 136], 0FFFFFFFFh

		invoke	WriteSectorLong, esi, ADDR IOBuffer

;		invoke	ReadSector, ADDR IOBuffer, esi

 		add	esi, SECTOR_HURT_SPACING
	.ENDW
	ret
;-------------------------------------------------------------------------------
DamageTheDisk	ENDP

;-/////////////////////////////////////////////////////////////////////////////-
ENDIF
;-/////////////////////////////////////////////////////////////////////////////-

;+-----------------------------------------------------------------------------+
;|			       OPEN DRIVE ACCESS			       |
;|-----------------------------------------------------------------------------|
;|			    Returns TRUE on success			       |
;+-----------------------------------------------------------------------------+
OpenDriveAccess	PROC	DriveNumber:DWORD
;-------------------------------------------------------------------------------
	invoke	SetErrorMode, SEM_FAILCRITICALERRORS
	push	eax		; stack the original	error mode ...
	;컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴
	mov	eax, DriveNumber
	inc	eax
	mov	OneBasedDrive, eax
	add	al, 'A'-1	; cvrt 0-based drive into capital letter
	mov	DriveLetter, al
	mov	DriveRootName, al
	.IF (WinNT)
		invoke	CreateFile, ADDR DiskDrive, GENERIC_READ or GENERIC_WRITE,
			FILE_SHARE_READ or FILE_SHARE_WRITE,	NULL, OPEN_EXISTING,
			FILE_FLAG_WRITE_THROUGH or FILE_FLAG_NO_BUFFERING, NULL
	.ELSE
		invoke	CreateFile, ADDR SystemVXD, GENERIC_READ or GENERIC_WRITE,
			FILE_SHARE_READ or FILE_SHARE_WRITE,	NULL, OPEN_EXISTING,
			FILE_FLAG_WRITE_THROUGH or FILE_FLAG_NO_BUFFERING, NULL
	.ENDIF
	mov	hDriveAccess, eax
	call	SetErrorMode	; restore the original error	mode ...
	mov	eax, hDriveAccess
	.IF (eax == INVALID_HANDLE_VALUE)
		zero	eax
	.ENDIF
	ret
;-------------------------------------------------------------------------------
OpenDriveAccess	ENDP


;+-----------------------------------------------------------------------------+
;|			      CLOSE DRIVE ACCESS			       |
;+-----------------------------------------------------------------------------+
CloseDriveAccess	PROC
;-------------------------------------------------------------------------------
	mov	eax, hDriveAccess
	.IF (eax)
		invoke	CloseHandle, eax
	.ENDIF
	ret
;-------------------------------------------------------------------------------
CloseDriveAccess	ENDP


;+-----------------------------------------------------------------------------+
;|			       PERFORM DEVICE IO			       |
;|-----------------------------------------------------------------------------|
;|	      Returns ZERO on success, and ERROR CODE on failure	       |
;+-----------------------------------------------------------------------------+
Win95DeviceIO	PROC USES	esi, _eax:DWORD, _ebx:DWORD, _ecx:DWORD, _edx:DWORD, _esi:DWORD, _edi:DWORD
		LOCAL	Registers:DEVIOCTL_REGISTERS, BytesReturned:DWORD
;-------------------------------------------------------------------------------
	movmov	Registers.reg_EAX, eax,	_eax
	.IF (ah == 44h)
		mov	esi, VWIN32_DIOC_DOS_IOCTL	; (1)
	.ELSEIF (ah ==	25h)
		mov	esi, VWIN32_DIOC_DOS_INT25	; (2)
	.ELSEIF (ah ==	26h)
		mov	esi, VWIN32_DIOC_DOS_INT26	; (3)
	.ELSEIF (ah ==	73h)
		mov	esi, VWIN32_DIOC_DOS_DRIVEINFO	; (6)
	.ELSE
		zero	eax
		jmp	Exit
	.ENDIF
	movmov	Registers.reg_EBX, eax,	_ebx
	movmov	Registers.reg_ECX, eax,	_ecx
	movmov	Registers.reg_EDX, eax,	_edx
	movmov	Registers.reg_ESI, eax,	_esi
	movmov	Registers.reg_EDI, eax,	_edi
	mov	Registers.reg_Flags, 1	; preset the carry
	invoke	DeviceIoControl, hDriveAccess, esi,	; non-zero on success
	    	ADDR Registers, SIZEOF DEVIOCTL_REGISTERS,
		ADDR Registers, SIZEOF DEVIOCTL_REGISTERS,
		ADDR BytesReturned, NULL
	.IF (eax)
		; DeviceIoControl succeeded, but what	about the called func?
		zero	eax	; presume success (no carry set)
		.IF (Registers.reg_Flags & 1)
			; return was	CY=1, so return	the function's error val
			mov	eax, Registers.reg_EAX
		.ENDIF
	.ELSE
		; DeviceIoControl failed, so return -1
		mov	eax, -1
	.ENDIF
Exit:	ret
;-------------------------------------------------------------------------------
Win95DeviceIO	ENDP


;+-----------------------------------------------------------------------------+
;|				 QUALIFY DRIVE				       |
;|-----------------------------------------------------------------------------|
;|	  Returns TRUE if this drive might be a	Local Removable	Iomega	       |
;+-----------------------------------------------------------------------------+
QualifyDrive	PROC
		LOCAL	BytesReceived:DWORD, DriveMapInfo:DRIVE_MAP_INFO, MediaID:MEDIA_ID,
			DiskGeometry:DISK_GEOMETRY, PartitionInfo:PARTITION_INFORMATION
;-------------------------------------------------------------------------------
	; ask Windows what it knows about the drive
	invoke	GetDriveType, ADDR DriveRootName
	cmp	eax, DRIVE_REMOVABLE	; if it's NOT	removable or FIXED
	jb	Disqualify		; then we're out of the	game.
	cmp	eax, DRIVE_FIXED	;	but it COULD be	removable MASQUERADING
	ja	Disqualify		; as fixed ... so we need to check that!
	;컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴
	; Windows says	it's fixed or removable	... let's look closer ...
	.IF (WinNT)
		; check to see if the	media is accessible
		invoke	DeviceIoControl, hDriveAccess,	IOCTL_STORAGE_CHECK_VERIFY,
			NULL, 0, NULL, 0, ADDR BytesReceived, NULL
		check	eax	; returns FALSE if it's NOT	accessible
		jz	Exit
		;--------------------------------------------------------------------------
		invoke	DeviceIoControl, hDriveAccess,	IOCTL_DISK_GET_DRIVE_GEOMETRY,
			NULL, 0, ADDR DiskGeometry, SIZEOF DiskGeometry, ADDR BytesReceived,	NULL
		mov	eax,  DiskGeometry.MediaType
		cmp	eax, RemovableMedia
		jne	Disqualify
		;--------------------------------------------------------------------------
		invoke	DeviceIoControl, hDriveAccess,	IOCTL_DISK_GET_PARTITION_INFO,
			NULL, 0, ADDR DiskGeometry, SIZEOF PartitionInfo, ADDR PartitionInfo, NULL
		movmov	PartitionOffset, eax, dword ptr PartitionInfo.StartingOffset
	.ELSE
		; look more closely at a status that ISN'T foolable
		mov	DriveMapInfo.dmiAllocationLength,	SIZEOF DRIVE_MAP_INFO
		invoke	Win95DeviceIO,	440Dh, OneBasedDrive, 086Fh, ADDR DriveMapInfo,0,0
		check	eax	; returns an error code if the call	failed
		jnz	Disqualify
		movmov	PartitionOffset, ecx, dword ptr DriveMapInfo.dmiPartitionStartRBA
		;--------------------------------------------------------------------------
		; we found a drive, so see if	it supports EJECT!
		; if not if can't be an IOMEGA removable drive
		.IF (DriveMapInfo.dmiFlags & PROT_MODE_EJECT)
			; yes, now can we read its MediaID ??
			invoke	Win95DeviceIO, 440Dh,	OneBasedDrive, 0866h, ADDR MediaID,0,0
			.IF (eax)
				; MediaID failed, so fail our function
Disqualify:			zero	eax
			.ELSE
				; this looks like a	PERFECT	candidate ...
				dec	eax	; set EAX to -1 for	success!
			.ENDIF
		.ENDIF
	.ENDIF
Exit:	ret
;-------------------------------------------------------------------------------
QualifyDrive	ENDP


;+-----------------------------------------------------------------------------+
;|			       EXCLUSIVE ACCESS				       |
;|-----------------------------------------------------------------------------|
;|			   Returns TRUE	for Success			       |
;+-----------------------------------------------------------------------------+
ExclusiveAccess	PROC	DriveNumber:DWORD,	Exclusive:BOOL
		LOCAL	BytesReceived
;-------------------------------------------------------------------------------
.IF (DriveNumber != (-1))
	.IF (Exclusive)
		invoke	OpenDriveAccess, DriveNumber
		.IF (WinNT)
			invoke	DeviceIoControl, hDriveAccess, FSCTL_LOCK_VOLUME, NULL, 0, NULL, 0, ADDR BytesReceived, NULL
		.ELSE
			invoke	Win95DeviceIO, 440Dh,	OneBasedDrive, 084Ah, 0,0,0
			.IF (eax)
				; MediaID failed, so fail our function
				zero	eax
			.ELSE
				; this looks like a	PERFECT	candidate ...
				dec	eax	; set EAX to -1 for	success!
			.ENDIF
		.ENDIF
	.ELSE
		.IF (WinNT)
			invoke	DeviceIoControl, hDriveAccess, FSCTL_UNLOCK_VOLUME, NULL, 0, NULL, 0,	ADDR BytesReceived, NULL
		.ELSE
			invoke	Win95DeviceIO, 440Dh,	OneBasedDrive, 086Ah, 0,0,0
			.IF (eax)
				; MediaID failed, so fail our function
				zero	eax
			.ELSE
				; this looks like a	PERFECT	candidate ...
				dec	eax	; set EAX to -1 for	success!
			.ENDIF
		.ENDIF
		push	eax
		call	CloseDriveAccess
		pop	eax
	.ENDIF
.ENDIF
	ret
;-------------------------------------------------------------------------------
ExclusiveAccess	ENDP


;+-----------------------------------------------------------------------------+
;|			   FILL	BUFFER THROUGH ASPI			       |
;+-----------------------------------------------------------------------------+
FillBufferThroughASPI	PROC
			LOCAL	Scsi[10]:BYTE
;-------------------------------------------------------------------------------
	varzero	Scsi
	mov	Scsi, SCSI_Cmd_ReadMany
	mov	eax, PartitionOffset
	;컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴
;	bswap	eax			; convert it into BIG endian format
	ror	eax, 8	; [dabc]
	swap	ah, al	; [dacb]
	ror	eax, 16	; [cbda]
	swap	ah, al	; [cbad]
	ror	eax, 8	; [dcba]
	;컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴
	mov	dword ptr Scsi[2],	eax	; specify WHICH LBA's to read
	mov	Scsi[8], COMPARISON_SECTORS
	invoke	SCSICommand, CurrentAdapter, CurrentDevice, ADDR Scsi, pTargetBuffer, COMPARISON_BYTES
	ret
;-------------------------------------------------------------------------------
FillBufferThroughASPI	ENDP


;+-----------------------------------------------------------------------------+
;|			     READ PHYSICAL SECTORS			       |
;|-----------------------------------------------------------------------------|
;|			  Returns TRUE if it succeeds			       |
;+-----------------------------------------------------------------------------+
ReadPhysicalSectors	PROC	Sector:DWORD, Count:WORD, pBuffer:LPVOID
			LOCAL	DiskIO:DISKIO,	BytesRead:DWORD
;-------------------------------------------------------------------------------
	; we must first read the sector containing the	current	Serial Number
	.IF (WinNT)
		mov	eax, Sector	; get	the starting sector number
		shl	eax, 9		;	multiply by 512	for starting BYTE
		invoke	SetFilePointer, hDriveAccess, eax, 0, FILE_BEGIN
		movzx	ebx, Count	; get the total sector count
		shl	ebx, 9		;	multiply by 512	for total bytes	to read
		invoke	ReadFile, hDriveAccess, pBuffer, ebx, ADDR BytesRead, NULL
	.ELSE
		; setup the data transfer packet
		movmov	DiskIO.StartSector, eax, Sector
		movmov	DiskIO.Sectors, ax, Count
		movmov	DiskIO.Buffer,	eax, pBuffer
		; first try an Int25 (read) call ... for build 950
		mov	ecx, OneBasedDrive
		dec	ecx	; cvrt into 0-baed drive number
		mov	ch, 25h	;	perform	INT_25 direct access read
		invoke	Win95DeviceIO,	ecx, ADDR DiskIO, -1, 0, 0, 0
		check	eax	; zero means we succeeded!
		jz	Success
		;--------------------------------------------------------------------------
		; for	SR2 and	later
		invoke	Win95DeviceIO,	7305h, ADDR DiskIO, -1,	OneBasedDrive, 0,0
		.IF (eax) ; non-zero means we	failed,	so return FALSE
			zero	eax
		.ELSE	; zero means we	succeeded ... to return	-1
Success:		dec	eax	; set	EAX to -1 for success!
		.ENDIF
	.ENDIF
	ret
;-------------------------------------------------------------------------------
ReadPhysicalSectors	ENDP


;+-----------------------------------------------------------------------------+
;|				COMPARE	BUFFERS				       |
;|-----------------------------------------------------------------------------|
;|		   Returns TRUE	if the buffers are identical		       |
;+-----------------------------------------------------------------------------+
CompareBuffers	PROC USES esi edi
;-------------------------------------------------------------------------------
	zero	eax
	mov	ecx, COMPARISON_BYTES / 4
	mov	esi, pTargetBuffer
	mov	edi, pSearchBuffer
	repe cmpsd
	.IF (zero?)
		dec	eax
	.ENDIF
	ret
;-------------------------------------------------------------------------------
CompareBuffers	ENDP


;+-----------------------------------------------------------------------------+
;|			       FIND DRIVE LETTER			       |
;|-----------------------------------------------------------------------------|
;|	  Returns the Zero-Based DOS drive letter, or -1 if not	found	       |
;+-----------------------------------------------------------------------------+
FindDriveLetter	PROC
		LOCAL	DriveNumber:DWORD
;-------------------------------------------------------------------------------
	; allocate two	page-aligned read buffers for source disk reads
	invoke	VirtualAlloc, NULL, COMPARISON_BYTES, MEM_COMMIT, PAGE_READWRITE
	mov	pTargetBuffer, eax
	invoke	VirtualAlloc, NULL, COMPARISON_BYTES, MEM_COMMIT, PAGE_READWRITE
	mov	pSearchBuffer, eax
	;--------------------------------------------------------------------------
	; now search the drive	letter space for the same data on a valid drive
	;컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴
	mov	DriveNumber, 'Z'-'A'	; start at "Z" and work downward ...
	.REPEAT
		invoke	OpenDriveAccess, DriveNumber
		call	QualifyDrive	; returns true if it might be
		.IF (eax)
			; read the partition's first	8 sectors through the ASPI interface
AbsoluteRead:		call	FillBufferThroughASPI
			.IF (!eax)
				; now read the sectors through the device driver interface
				invoke	ReadPhysicalSectors,	0, COMPARISON_SECTORS, pSearchBuffer
				.IF	(eax)	;	got a TRUE return, so compare'em
					call	CompareBuffers	; returns TRUE	for equality
					.IF (eax)
						;	yipeee!	 we found the drive! ...
						call	CloseDriveAccess
						jmp	Exit
					.ELSEIF (PartitionOffset == 0)
						add	PartitionOffset, 20h
						jmp	AbsoluteRead
					.ENDIF
				.ENDIF
			.ENDIF
		.ENDIF
		call	CloseDriveAccess
		dec	DriveNumber
	.UNTIL	(sign?)	; 0-based, so this is past the end
	; if DriveNumber == -1, we never found	the drive
Exit:	invoke	VirtualFree, pTargetBuffer, 0, MEM_RELEASE
	invoke	VirtualFree, pSearchBuffer, 0, MEM_RELEASE
	mov	eax, DriveNumber
	ret
;-------------------------------------------------------------------------------
FindDriveLetter	ENDP


;+-----------------------------------------------------------------------------+
;|			    EJECT IOMEGA CARTRIDGE			       |
;+-----------------------------------------------------------------------------+
EjectIomegaCartridge	PROC	Adapter:DWORD, Device:DWORD
			LOCAL	UnlockParams:DWORD, BytesReceived:DWORD
			LOCAL	Scsi[6]:BYTE
;-------------------------------------------------------------------------------
	push	CurrentAdapter
	push	CurrentDevice
	movmov	CurrentAdapter,	eax, Adapter
	movmov	CurrentDevice, eax, Device
	call	FindDriveLetter
	.IF (eax <= 'Z'-'A')
		invoke	OpenDriveAccess, eax
		.IF (WinNT)
			invoke	DeviceIoControl, hDriveAccess, IOCTL_STORAGE_EJECT_MEDIA, NULL, 0, NULL, 0, ADDR BytesReceived, NULL
		.ELSE
			mov	UnlockParams, 1
			invoke	Win95DeviceIO, 440Dh,	OneBasedDrive, 0848h, ADDR UnlockParams,0,0
			invoke	Win95DeviceIO, 440Dh,	OneBasedDrive, 0849h, 0,0,0
		.ENDIF
		invoke	CloseDriveAccess
	.ELSE
		; Could NOT do it through the	IOCTL layer ...	so eject with SCSI
		; make sure the media	is not locked ...
		varzero	Scsi
		mov	Scsi, SCSI_Cmd_PreventAllow
		invoke	SCSICommand, Adapter, Device, ADDR Scsi, NULL,	0
		; issue an Asynchronous STOP command to induce spindown and ejection
		varzero	Scsi
		mov	Scsi[0], SCSI_Cmd_StartStopUnit
		mov	Scsi[1], 1	; set the IMMED bit for offline
		mov	Scsi[4], 2	; eject a Jaz disk after stopping
		invoke	SCSICommand, Adapter, Device, ADDR Scsi, NULL,	0
	.ENDIF
	pop	CurrentDevice
	pop	CurrentAdapter
	ret
;-------------------------------------------------------------------------------
EjectIomegaCartridge	ENDP


;+-----------------------------------------------------------------------------+
;|			    PERFORM REGION TRANSFER			       |
;+-----------------------------------------------------------------------------+
PerformRegionTransfer	PROC USES	ebx, XferCmd:DWORD, pBuffer:DWORD
			LOCAL	Scsi[10]:BYTE,	LocalBuffer:DWORD, LocalCount:DWORD,
				InitialHardErrors:DWORD, GlitchCount:DWORD,	GlitchError:DWORD
;-------------------------------------------------------------------------------
	reset	SingleTransferLBA	; show	we're testing a	range ...
	movmov	InitialHardErrors, eax,	HardErrors
	varzero	Scsi			; clear	out the	SCSI CDB
	movmov	Scsi, al, byte ptr XferCmd
	;컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴
	mov	eax, FirstLBASector	; setup the first block to read
;	bswap	eax			; convert it into BIG endian format
	ror	eax, 8	; [dabc]
	swap	ah, al	; [dacb]
	ror	eax, 16	; [cbda]
	swap	ah, al	; [cbad]
	ror	eax, 8	; [dcba]
	mov	dword ptr Scsi[2],	eax	; specify WHICH LBA's to read
	;컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴
	mov	eax, NumberOfLBAs	; and the NUMBER	of LBA's to read
	swap	ah, al			; convert it into BIG endian format
	mov	word ptr Scsi[7], ax	;
	invoke	SetErrorRecovery, FALSE, FALSE,	TRUE	; disable Retries & ECC
	invoke	SCSICommand, CurrentAdapter, CurrentDevice, ADDR Scsi, pBuffer,	AdapterMaxBytes
	; if we failed	somewhere during our transfer ... let's	zero in	on it
	.IF (eax)
		cmp	eax, SS_ERR		; if	it's a CONTROLLER ERROR, skip!
		je	Exit
		cmp	eax, BUFFER_TOO_BIG	; if we need to reduce the size
		je	Exit
		cmp	eax, LBA_TOO_LARGE
		je	Exit
		;--------------------------------------------------------------------------
		mov	GlitchError, eax	; save the error	which stopped us!
		mov	eax, SoftErrors		; get the current Soft +	Hard Error count
		add	eax, HardErrors
		mov	GlitchCount, eax	; save it to see	if we do FIND the glitch ...
		movmov	SingleTransferLBA,	eax, FirstLBASector
		call	UpdateCurrentSector	; show what we're doing ...
		movmov	LocalBuffer,		eax, pBuffer
		movmov	LocalCount,		eax, AdapterMaxSectors
		call	ErrorSound
		.REPEAT
			; setup for our series of transfer tests ...
			varzero	Scsi			; clear out the SCSI CDB
			movmov	Scsi,	al, byte ptr XferCmd
			;컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴
			mov	eax, SingleTransferLBA	;	the single sector to xfer
;			bswap	eax			; convert it into BIG endian format
			ror	eax, 8	;	[dabc]
			swap	ah, al	; [dacb]
			ror	eax, 16	; [cbda]
			swap	ah, al	; [cbad]
			ror	eax, 8	;	[dcba]
			mov	dword ptr Scsi[2], eax	;	specify	WHICH single LBA to read
			;컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴
			mov	Scsi[8],	1		; a single sector
			; disable all recovery techniques
			invoke	SetErrorRecovery, FALSE, FALSE, TRUE	; disable Retries & ECC
			invoke	SCSICommand, CurrentAdapter, CurrentDevice, ADDR Scsi, LocalBuffer, BYTES_PER_SECTOR
			.IF (eax)
				; some sort	of problem encountered!
				cmp	eax, SS_ERR	; if it's a	CONTROLLER ERROR, skip!
				je	Exit
				;--------------------------------------------------------------------------
				cmp	al, 1	;	did we recover?
				je	PostTheError
				;--------------------------------------------------------------------------
				invoke	SetErrorRecovery, TRUE, FALSE, TRUE	; enable	retries
				invoke	SCSICommand,	CurrentAdapter,	CurrentDevice, ADDR Scsi, LocalBuffer, BYTES_PER_SECTOR
				.IF	(eax)
					; failed even with	retries
					cmp	eax, SS_ERR	; if it's a CONTROLLER ERROR, skip!
					je	Exit
					;--------------------------------------------------------------------------
					cmp	al, 1	; did we	recover?
					je	PostTheError
					;--------------------------------------------------------------------------
					invoke	SetErrorRecovery, TRUE, TRUE, TRUE	; enable	retries	AND ECC
					invoke	SCSICommand, CurrentAdapter, CurrentDevice,	ADDR Scsi, LocalBuffer,	BYTES_PER_SECTOR
					.IF (eax)
						;	failed with retries and	ECC
						cmp	eax, SS_ERR	;	if it's	a CONTROLLER ERROR, skip!
						je	Exit
					.ELSE	; succeeded with ECC
						mov	eax, 180101h	; "ECC &	Retries"
					.ENDIF
				.ELSE	; succeeded using retries
					mov	eax, 170101h	;	"Read with Retries"
					.IF (XferCmd == SCSI_Cmd_WriteMany)
						mov eax, 0C8001h ; "Wrote	with Retries"
					.ENDIF
				.ENDIF
				;--------------------------------------------------------------------------
PostTheError:			invoke	BumpErrorCounts,	eax	; given eax, count the errors
				invoke	GetSpareSectorCounts, FALSE ; update	the Cart's Condition
				call	UpdateRunTimeDisplay
			.ENDIF
			inc	SingleTransferLBA
			call	UpdateCurrentSector
			add	LocalBuffer, BYTES_PER_SECTOR
			call	ProcessPendingMessages
			dec	LocalCount
		.UNTIL	(zero?) || (UserInterrupt)
		; now	see whether we *did* found something to	complain about ...
		mov	eax, SoftErrors
		add	eax, HardErrors
		.IF (eax == GlitchCount)
			; we	missed it ... but SOMETHING happened!  So let's	report it ...
			push	SoftErrors		; save the existing	counts
			push	HardErrors
			mov	eax, GlitchError	; get the error	that triggered our search
			mov	ebx, eax
			and	ebx, 00FF00FFh		; strip the ASCQ	byte
			.IF (ebx == 00110003h)		; if	we're about to say "unrecovered	read"
				mov	eax, 170101h	; change it to: "Read with	Retries"
			.ENDIF
			invoke	BumpErrorCounts, eax	; given eax, count the errors
			pop	HardErrors		; restore the counts
			pop	SoftErrors
			inc	SoftErrors		; and bump the SOFT error count
			call	UpdateRunTimeDisplay
		.ENDIF
		reset	SingleTransferLBA
		zero	eax	; now let's return happiness	to our caller
		mov	ebx, HardErrors		; UNLESS	we had some
		.IF (ebx != InitialHardErrors)	; UNRECOVERABLE errors!
			dec	eax		; in which case return -1
		.ENDIF
	.ENDIF
	; restore standard error recovery modes
Exit:	push	eax
	invoke	SetErrorRecovery, TRUE,	TRUE, FALSE ; reenable Retries & ECC
	pop	eax
	ret
;-------------------------------------------------------------------------------
PerformRegionTransfer	ENDP


;+-----------------------------------------------------------------------------+
;|				 TEST THE DISK				       |
;+-----------------------------------------------------------------------------+
TestTheDisk	PROC
		LOCAL	TransferLength:DWORD, DataPattern:DWORD,
			DriveNumber:DWORD, BlastRecoveryMode:BOOL
;-------------------------------------------------------------------------------
	; USB seems to	generate insane	WM_TIMER messages, so we now stop
	; the timer during data read/write processing.
	call	StopApplicationTimer
	; determine whether to	show "Unknown Error" or	the Hex	code ...
	invoke	GetAsyncKeyState, VK_SHIFT
	shl	eax, 1		; move the	high bit into  CY
	.IF (carry?)
		set	UnknownErrorMode
	.ENDIF

	; determine which drive to lock
	call	FindDriveLetter	;	returns	drive letter, or -1 if not found
	mov	DriveNumber, eax
	.IF (eax == (-1))
		mov	al, "?"
	.ELSE
		invoke	ExclusiveAccess, eax, TRUE
		.IF (!eax)	; could NOT succeed in locking the	drive!
			mov	al, byte	ptr DriveNumber
			add	al, 'A'
			mov	szExclusiveDrive1, al
			mov	szExclusiveDrive2, al
			invoke	MessageBox, hMainWnd,	ADDR szNoExclusiveText,	ADDR szNoExclusiveTitle, MB_APPLMODAL or MB_OK
			; allow operation on	an unlocked drive!
			invoke	GetAsyncKeyState, VK_SHIFT
			shl	eax, 1		; move the high bit into	 CY
			jnc	Exit
		.ENDIF
		mov	al, byte ptr DriveNumber
		add	al, 'A'
	.ENDIF
	mov	DriveUnderTest, al
;===============================================================================
	call	PreventProgramExit
	invoke	SetRichEditText, ADDR hTabText,	ADDR szRunning
	mov	CartridgeStatus, DISK_TEST_UNDERWAY
	mov	TestingPhase, TESTING_STARTUP	; inhibit stopping now
	invoke	SetWindowText, hTestButton, ADDR szPressToStop
	invoke	InvalidateRect,	hTestMonitor, NULL, FALSE
	call	LockCurrentDrive		; prevent media	removal

	; setup the initial maximum transfer ...
	invoke	HostAdapterInquiry, CurrentAdapter
	mov	eax, dword	ptr ASPI_CmdBlock.hai.HA_Unique[4]	; get the max Xfer
	.IF (!eax) || (eax > MAX_TRANSFER_PER_TEST)	; limit to	our max
		mov	eax, MAX_TRANSFER_PER_TEST
	.ENDIF
SizeIt:	mov	AdapterMaxBytes, eax
	shr	eax, 9			;	divide by Bytes/Sector
	mov	AdapterMaxSectors,	eax

;----------------------- Check for "Blast Recovery Mode" -----------------------

	call	CheckForPartitionTable
	; returns NON-ZERO if partition table was NOT found
	.IF (eax)
		call	PerhapsBlastRecovery
		; returns NON-ZERO if	BlastRecovery *was* performed ...
	.ENDIF
	mov	BlastRecoveryMode,	eax
	check	eax
	jnz	GetOut

IF FORCE_STARTING_SECTOR
	mov	FirstLBASector, FORCE_STARTING_SECTOR ; and the starting point
ENDIF

;------------------------- Standard Testing Operation --------------------------

	; log the starting time
	invoke	GetSystemTimeAsFileTime, ADDR StartingInstant
	.REPEAT
		; compute the	number of LBS and Last LBA
		mov	eax, LastLBAOnCartridge		; get the maximum LBA count
		sub	eax, FirstLBASector		; and the starting point
		inc	eax				; number of sectors to go
		.IF (eax > AdapterMaxSectors)
			mov	eax, AdapterMaxSectors	;	trim it	to the sector count
		.ENDIF
		mov	NumberOfLBAs, eax

		; compute the	LastLBA
		add	eax, FirstLBASector		; get just past the end
		dec	eax				; get the last one
		mov	LastLBASector, eax

		; compute the	percentage complete
		mov	edx, FirstLBASector
		zero	eax
		div	LastLBAOnCartridge
		mov	PercentComplete, eax

		; uppdate the	elapsed	time
		call	GetElapsedTimeInSeconds
		.IF (eax != (-1))
			mov	SecondsElapsed, eax
		.ENDIF

		; get	a random pattern of data to write
		call	GetRandomNumber
		mov	DataPattern, eax
		mov	ecx, AdapterMaxBytes
		shr	ecx, 2
		mov	edi, pPatternBuffer
		rep stosd

		; update the cartridge's status
		invoke	GetSpareSectorCounts, FALSE ; update the Cart's Condition

		mov	TestingPhase, READING_DATA
		call	UpdateRunTimeDisplay
		invoke	PerformRegionTransfer,	SCSI_Cmd_ReadMany, pUserDataBuffer
		.IF (!eax)
			;--------------------------------------------------------------------------
			mov	TestingPhase, WRITING_PATT
			invoke	UpdateRunPhaseDisplay
			invoke	PerformRegionTransfer, SCSI_Cmd_WriteMany, pPatternBuffer
			;--------------------------------------------------------------------------
			mov	TestingPhase, READING_PATT
			invoke	UpdateRunPhaseDisplay
			invoke	PerformRegionTransfer, SCSI_Cmd_Verify, pPatternBuffer
			;--------------------------------------------------------------------------
			mov	TestingPhase, WRITING_DATA
			invoke	UpdateRunPhaseDisplay
			invoke	PerformRegionTransfer, SCSI_Cmd_WriteMany, pUserDataBuffer
			;--------------------------------------------------------------------------
		.ELSEIF (eax == BUFFER_TOO_BIG)
			; we	need to	make it	smaller
			mov	eax, AdapterMaxBytes	; get the current size
			halve	eax
			jmp	SizeIt
		.ENDIF
		; if we hit the end of the disk ... exit gracefully!
		cmp	eax, LBA_TOO_LARGE
		je	GetOut
		; see	if everything's	still okay ...
		.BREAK .IF (CartridgeStatus != DISK_TEST_UNDERWAY)
		; bump the FirstLBASector up for the next transfer
		mov	eax, FirstLBASector
		add	eax, NumberOfLBAs
		mov	FirstLBASector, eax
	.UNTIL	(eax > LastLBAOnCartridge) || (UserInterrupt)
	; show	that we're post-test
GetOut:	reset	TestingPhase
	call	UnlockAllMedia
	invoke	SetErrorRecovery, TRUE,	TRUE, FALSE	; reenable Retries & ECC

	invoke	SetWindowText, hTestButton, ADDR szPressToStart
	mov	CartridgeStatus, DISK_AT_SPEED
	call	AllowProgramExit

	; compute the number of serious troubles
	mov	eax, FirmErrors
	add	eax, HardErrors
	.IF (eax >= BADNESS_THRESHOLD)
		mov	eax, OFFSET szBadResult
	.ELSEIF (UserInterrupt)
		mov	eax, OFFSET szInterrupted
	.ELSE	; it wasn't interrupted,	nor seriously bad, was it perfect?
		add	eax, SoftErrors
		.IF (eax)
			mov	eax, OFFSET szExplainResult
		.ELSE
			mov	eax, OFFSET szPerfectResult
		.ENDIF
	.ENDIF
	invoke	SetRichEditText, ADDR hTabText,	eax
	invoke	InvalidateRect,	hTestMonitor, NULL, FALSE
	; release any exclusive filesystem lock we may	have on	the drive
Exit:	invoke	ExclusiveAccess, DriveNumber, FALSE
	; if we've just un-zapped the disk ...	let's eject it!
	.IF (BlastRecoveryMode)
		invoke	EjectIomegaCartridge, CurrentAdapter, CurrentDevice
	.ENDIF
	; we're done (with USB'ness) so it's safe to restart the timer
	call	StartApplicationTimer
	ret
;-------------------------------------------------------------------------------
TestTheDisk	ENDP


;+-----------------------------------------------------------------------------+
;|			   CHECK FOR PARTITION TABLE			       |
;|	       Return NON-ZERO if partition table was NOT found!	       |
;+-----------------------------------------------------------------------------+
CheckForPartitionTable	PROC USES ebx
	LOCAL	Return:DWORD, ShiftCount:DWORD
;-------------------------------------------------------------------------------
	reset	Return
	; determine how many shift keys we're holding down ...
	reset	ShiftCount
	invoke	GetAsyncKeyState, VK_SHIFT
	shl	eax, 1		; move the	high bit into  CY
	.IF (carry?)
		inc	ShiftCount
	.ENDIF
	invoke	GetAsyncKeyState, VK_CONTROL
	shl	eax, 1		; move the	high bit into  CY
	.IF (carry?)
		inc	ShiftCount
	.ENDIF
	invoke	GetAsyncKeyState, VK_MENU
	shl	eax, 1		; move the	high bit into  CY
	.IF (carry?)
		inc	ShiftCount
	.ENDIF
	; allocate a page-aligned read	buffer for disk	read
	invoke	GlobalAlloc, GPTR, BYTES_PER_SECTOR
	mov	ebx, eax
	; read	the cartridge's	partition table	...
	invoke	ReadSectors, ebx, 0, 1

	; only	if we got a good table read *AND* it's not a partition table
	.IF (ShiftCount == 3) || ((!eax) && (word ptr [ebx+01FEh] != 0AA55h))
		set	Return
	.ENDIF
	invoke	GlobalFree, ebx
	mov	eax, Return
	ret
;-------------------------------------------------------------------------------
CheckForPartitionTable	ENDP


;+-----------------------------------------------------------------------------+
;|			    PERHAPS BLAST RECOVERY			       |
;|-----------------------------------------------------------------------------|
;|   No	partition table	was found ... so give them the option of performing    |
;|   a "blast recovery".  Return TRUE if they took the option.		       |
;+-----------------------------------------------------------------------------+
PerhapsBlastRecovery	PROC USES ebx esi
			LOCAL	StartOfFAT1:DWORD, StartOfFAT2:DWORD,
				SectorsToCopy:DWORD
;-------------------------------------------------------------------------------
	; update the LBA count
	invoke	GetSpareSectorCounts, FALSE ; update the Cart's	Condition
	; get the drive index {0-3}
	mov	eax, LastLBAOnCartridge
	zero	ebx
	.WHILE	(LBAMidPoints[ebx*4] < eax) && (ebx < 3)
		inc	ebx
	.ENDW
	; patch the cartridge size into the mesage box	text
	; get the starting address of the name	strings
	lea	esi, IomegaDeviceNames[ebx*8]

	movmov	dword ptr CartSize1[0],	eax, [esi]
	movmov	dword ptr CartSize2[0],	eax, [esi]
	movmov	dword ptr CartSize3[0],	eax, [esi]
	movmov	dword ptr CartSize4[0],	eax, [esi]
	movmov	dword ptr CartSize5[0],	eax, [esi]

	movmov	dword ptr CartSize1[4],	eax, [esi+4]
	movmov	dword ptr CartSize2[4],	eax, [esi+4]
	movmov	dword ptr CartSize3[4],	eax, [esi+4]
	movmov	dword ptr CartSize4[4],	eax, [esi+4]
	movmov	dword ptr CartSize5[4],	eax, [esi+4]

	invoke	MessageBox, hMainWnd, ADDR szPerhapsBlastText, ADDR szPerhapsBlastTitle, MB_APPLMODAL or MB_OKCANCEL
	sub	eax, IDCANCEL
	jz	Exit

;=================== Restore the cartridge's Partition Table ===================

	; make	the offset to the drive's partition sector data
	shl	ebx, 10		;	multiply the drive index by 1024
	add	ebx, pSectorData
	invoke	WriteSectors, ebx, 0, 1
	check	eax
	jnz	Trouble

	; get the number of sectors per track for the LBA offset of the
	; partition boot sector ...
	mov	eax, [ebx+01F6h]	;	eax == 32 or 63
	mov	StartOfFAT1, eax
	inc	StartOfFAT1		; save the LBA sector	for FAT1

;===================== Restore the Cartridge's Boot Sector =====================

	add	ebx, BYTES_PER_SECTOR	; skip up to	the Boot Sector	data
	invoke	WriteSectors, ebx, eax,	1 ; and	send the boot sector out
	check	eax
	jnz	Trouble

;======================= Calc the FAT recovery variables =======================

	mov	eax, MAX_SECTORS_PER_TEST
	sub	eax, StartOfFAT1
	mov	SectorsToCopy, eax

	movzx	eax, word ptr [ebx + BPB_OFFSET + SECTORS_PER_FAT]
	add	eax, StartOfFAT1
	invoke	ReadSectors, pUserDataBuffer, eax, SectorsToCopy
	check	eax
	jnz	Trouble

	invoke	WriteSectors, pUserDataBuffer, StartOfFAT1, SectorsToCopy
	.IF (eax)
Trouble:	invoke	MessageBox, hMainWnd, ADDR szBlastTroubleText, ADDR szBlastTroubleTitle, MB_APPLMODAL or MB_OK
	.ELSE
		invoke	MessageBox, hMainWnd, ADDR szBlastSuccessText,	ADDR szBlastSuccessTitle, MB_APPLMODAL or MB_OK
	.ENDIF
	zero	eax
	dec	eax
Exit:	ret
;-------------------------------------------------------------------------------
PerhapsBlastRecovery	ENDP

