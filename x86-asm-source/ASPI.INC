;+-----------------------------------------------------------------------------+
;|				   ASPI.INC				       |
;+-----------------------------------------------------------------------------+
BITS16 = 0	; 1 == MS-DOS/Win16	0 == WIN32
;-------------------------------------------------------------------------------

LPSRB			typedef	PTR

PFNPOST			PROTO	STDCALL

; aspi_GetSupportInfo	PROTO STDCALL
; aspi_SendCommand	PROTO STDCALL :LPSRB

GetASPI32SupportInfoProto typedef PROTO	SYSCALL
GetASPI32SupportInfoPtr	  typedef PTR GetASPI32SupportInfoProto
SendASPI32CommandProto		 typedef PROTO SYSCALL :LPVOID
SendASPI32CommandPtr	  typedef PTR SendASPI32CommandProto
GetASPI32BufferProto	  typedef PROTO SYSCALL :LPVOID
GetASPI32BufferPtr	  typedef PTR GetASPI32BufferProto
FreeASPI32BufferProto		typedef	PROTO SYSCALL :LPVOID
FreeASPI32BufferPtr	  typedef PTR FreeASPI32BufferProto


;-------------------------------- SCSI Commands	--------------------------------

SCSI_Cmd_TestUnitReady		equ	00h
SCSI_Cmd_Rewind			equ	01h
SCSI_Cmd_RezeroUnit		equ	01h	; iomega ZIP
SCSI_Cmd_RequestSense		equ	03h
SCSI_Cmd_FormatUnit		equ	04h
SCSI_Cmd_ReadBlkLimits		equ	05h
SCSI_Cmd_ReadSectorIDs		equ	05h	; iomega ZIP
SCSI_Cmd_NonSenseData		equ	06h	; iomega	ZIP
SCSI_Cmd_ReassignBlocks		equ	07h	; iomega ZIP
SCSI_Cmd_Read			equ	08h
SCSI_Cmd_Write			equ	0Ah
SCSI_Cmd_Seek			equ	0Bh
SCSI_Cmd_CartProtect		equ	0Ch	;	iomega ZIP
SCSI_Cmd_WriteFilemarks		equ	10h
SCSI_Cmd_Space			equ	11h
SCSI_Cmd_Inquiry		equ	12h
SCSI_Cmd_ModeSelect		equ	15h
SCSI_Cmd_ReserveUnit		equ	16h
SCSI_Cmd_ReleaseUnit		equ	17h
SCSI_Cmd_Erase			equ	19h
SCSI_Cmd_ModeSense		equ	1Ah
SCSI_Cmd_LoadUnload		equ	1Bh
SCSI_Cmd_StartStopUnit		equ	1Bh	; iomega ZIP
SCSI_Cmd_SendDiagnostic		equ	1Dh
SCSI_Cmd_PreventAllow		equ	1Eh	; iomega	ZIP
SCSI_Cmd_TranslateLBA		equ	22h	; iomega	ZIP
SCSI_Cmd_FormatTest		equ	24h	; iomega ZIP
SCSI_Cmd_ReadCapacity		equ	25h
SCSI_Cmd_ReadMany		equ	28h	; iomega ZIP
SCSI_Cmd_WriteMany		equ	2Ah	; iomega ZIP
SCSI_Cmd_Locate			equ	2Bh
SCSI_Cmd_SeekMany		equ	2Bh	; iomega ZIP
SCSI_Cmd_Verify			equ	2Fh	; iomega ZIP
SCSI_Cmd_ReadPosition		equ	34h
SCSI_Cmd_SynchCache		equ	35h	; iomega ZIP
SCSI_Cmd_ReadDefectData		equ	37h	; iomega ZIP
SCSI_Cmd_WriteBuffer		equ	3Bh	;	iomega ZIP
SCSI_Cmd_ReadBuffer		equ	3Ch	; iomega ZIP
SCSI_Cmd_ReadLong		equ	3Eh	; iomega ZIP
SCSI_Cmd_WriteLong		equ	3Fh	; iomega ZIP
SCSI_Cmd_ReadToc		equ	43h
SCSI_Cmd_WriteFieldFormat	equ	 0E3h	;	iomega ZIP

TEN_BYTE_CMDS			equ	20h	; if >=	then 10	bytes

;------------------------------	ASPI Definitions -------------------------------

SENSE_LEN		equ	128t	; default sense buffer length
MAX_ASPI_BLOCK_LENGTH	equ	256t	; (SENSE_LEN + 80t was probably enough)

SRB_DIR_SCSI		equ	00000h	; direction determined	by SCSI	command
SRB_DIR_IN		equ	00008h	; transfer from SCSI target to host
SRB_DIR_OUT		equ	00010h	; transfer from	host to	SCSI target
SRB_POSTING		equ	00001h	; enable ASPI posting
SRB_EVENT_NOTIFY	equ	00040h	; enable ASPI event	notification
SRB_ENABLE_RESIDUAL_COUNT  equ	00004h	;	enable residual	byte count reporting
SRB_DATA_SG_LIST	equ	00002h	; data buffer points to scatter-gather list

WM_ASPIPOST		equ	04d42h	; ASPI Post message

;+-----------------------------------------------------------------------------+
;|		     %%% ASPI Command Definitions %%%			       |
;+-----------------------------------------------------------------------------+
SC_HA_INQUIRY		equ	00000h	; host adapter inquiry
SC_GET_DEV_TYPE		equ	00001h	; get device type
SC_EXEC_SCSI_CMD	equ	00002h	; execute SCSI command
SC_ABORT_SRB		equ	00003h	; abort an SRB
SC_RESET_DEV		equ	00004h	; SCSI	bus device reset
SC_GET_DISK_INFO	equ	00006h	; get Disk information
SC_RESCAN_SCSI_BUS	equ	00007h	;	rebuild	SCSI device map
SC_GETSET_TIMEOUTS	equ	00008h	;	get/set	target timeouts

;+-----------------------------------------------------------------------------+
;|			    %%%	SRB Status %%%				       |
;+-----------------------------------------------------------------------------+
SS_PENDING		equ	00000h	; SRB being processed
SS_COMP			equ	00001h	; SRB completed without error
SS_ABORTED		equ	00002h	; SRB aborted
SS_ABORT_FAIL		equ	00003h	; unable to abort SRB
SS_ERR			equ	00004h	; SRB completed with error
SS_INVALID_CMD		equ	00080h	; invalid ASPI command
SS_INVALID_HA		equ	00081h	; invalid host adapter number
SS_NO_DEVICE		equ	00082h	; SCSI	device not installed
SS_INVALID_SRB		equ	000e0h	; invalid parameter set in SRB
SS_FAILED_INIT		equ	000e4h	; ASPI for windows failed init
SS_ASPI_IS_BUSY		equ	000e5h	; no resources available to	execute	cmd
SS_BUFFER_TO_BIG	equ	000e6h	; buffer size to big to handle!

;+-----------------------------------------------------------------------------+
;|		       %%% Host	Adapter	Status %%%			       |
;+-----------------------------------------------------------------------------+
HASTAT_OK		equ	00000h	;	host adapter did not detect an error
HASTAT_SEL_TO		equ	00011h	; selection Timeout
HASTAT_DO_DU		equ	00012h	; data	overrun	data underrun
HASTAT_BUS_FREE		equ	00013h	; unexpected bus free
HASTAT_PHASE_ERR	equ	00014h	; target bus phase sequence	failure
HASTAT_TIMEOUT		equ	00009h	; timed out while SRB was waiting to	beprocessed.
HASTAT_COMMAND_TIMEOUT	equ	0000bh	; while processing the SRB, the adapter timed	out.
HASTAT_MESSAGE_REJECT	equ	0000dh	; while processing SRB	the adapter received a MESSAGE REJECT.
HASTAT_BUS_RESET	equ	0000eh	; a	bus reset was detected.
HASTAT_PARITY_ERROR	equ	0000fh	; a parity error	was detected.
HASTAT_REQUEST_SENSE_FAILED equ	00010h	; the adapter failed in issuing REQUEST SENSE.


;+-----------------------------------------------------------------------------+
;|	    %%%	SRB - HOST ADAPTER INQUIRY - SC_HA_INQUIRY %%%		       |
;+-----------------------------------------------------------------------------+
SRB_HAInquiry	struct 1t
;-------------------------------------------------------------------------------
SRB_Cmd		BYTE	?	; w: ASPI command code = SC_HA_INQUIRY
SRB_Status	BYTE	?	; r: ASPI command status byte
SRB_HaId	BYTE	?	; w: ASPI host adapter number
SRB_Flags	BYTE	?	; 0: SCSI request flags - undef - should be zero
SRB_Ext_Req_Sig	WORD	?	; rw: set to AA55h by ASPI client (swapped if supported)
SRB_Ext_Buf_Len	WORD	?	; rw: # of extended buf bytes desired
HA_Count	BYTE	?	; r: number of host adapters present
HA_SCSI_ID	BYTE	?	; r: SCSI ID of host adapter
HA_ManagerId	BYTE 16t dup (?); r: string describing the	manager
HA_Identifier	BYTE 16t dup (?);	r: string describing the host adapter
HA_Unique	BYTE 16t dup (?); r: host Adapter Unique parameters
HA_Extensions	WORD	?	; r: bitfield of supported	extensions
;-------------------------------------------------------------------------------
SRB_HAInquiry	ends
;-------------------------------------------------------------------------------
PSRB_HAInquiry	typedef	PTR SRB_HAInquiry


;+-----------------------------------------------------------------------------+
;|	     %%% SRB - GET DEVICE TYPE - SC_GET_DEV_TYPE %%%		       |
;+-----------------------------------------------------------------------------+
SRB_GDEVBlock	struct 1t
;-------------------------------------------------------------------------------
SRB_Cmd		BYTE	?	; w: ASPI command code = SC_GET_DEV_TYPE
SRB_Status	BYTE	?	; r: ASPI command status byte
SRB_HaId	BYTE	?	; w: ASPI host adapter number
SRB_Flags	BYTE	?	; 0: SCSI request flags - undef - should be zero
SRB_Hdr_Rsvd	DWORD	?	; 0: undef	- reserved for expansion
SRB_Target	BYTE	?	; w: target's	SCSI ID
SRB_Lun		BYTE	?	; w: target's LUN number
SRB_DeviceType	BYTE	?	;	r: target's peripheral device type
IFE BITS16	; only under	Win32
SRB_Rsvd1	BYTE	?
ENDIF
;-------------------------------------------------------------------------------
SRB_GDEVBlock	ends
;-------------------------------------------------------------------------------
PSRB_GDEVBlock	typedef	PTR SRB_GDEVBlock


IF BITS16	; EXECUTE SCSI COMMAND DEFINITION for	MS-DOS
;+-----------------------------------------------------------------------------+
;|	  %%% SRB - EXECUTE SCSI COMMAND - SC_EXEC_SCSI_CMD %%%		       |
;+-----------------------------------------------------------------------------+
SRB_ExecSCSICmd	struct 1t
;-------------------------------------------------------------------------------
SRB_Cmd		BYTE	?	; w: ASPI command code = SC_EXEC_SCSI_CMD
SRB_Status	BYTE	?	; r: ASPI command status byte
SRB_HaId	BYTE	?	; w: ASPI host adapter number
SRB_Flags	BYTE	?	; w: SCSI request flags
SRB_Hdr_Rsvd	DWORD	?	; 0: undef	- reserved
SRB_Target	BYTE	?	; w: target's	SCSI ID
SRB_Lun		BYTE	?	; w: target's LUN number
SRB_BufLen	DWORD	?	; w:	data allocation	length
SRB_SenseLen	BYTE	?	; w: sense allocation length
SRB_BufPointer	LPBYTE	?	; w: data buffer pointer
SRB_LinkPointer	LPVOID	?	; w: SRB link pointer
SRB_CDBLen	BYTE	?	; w: CDB Length
SRB_HaStat	BYTE	?	; r: host Adapter Status
SRB_TargStat	BYTE	?	; r: target	Status
SRB_PostProc	LPPROC	?	;	w: post	routine
SRB_Rsvd	BYTE 34 dup (?)	; 0: reserved for ASPI	workspace
;-------------------------------------------------------------------------------
SRB_CDBByte    	BYTE 16	dup (?)	; w: SCSI CDB
;-------------------------------------------------------------------------------
SRB_SenseArea  	BYTE (SENSE_LEN+2) dup (?); r: request Sense buffer
;-------------------------------------------------------------------------------
SRB_ExecSCSICmd		ends
;-------------------------------------------------------------------------------
PSRB_ExecSCSICmd 	typedef	PTR SRB_ExecSCSICmd


ELSE		;	EXECUTE	SCSI COMMAND DEFINITION	for WIN32
;+-----------------------------------------------------------------------------+
;|	  %%% SRB - EXECUTE SCSI COMMAND - SC_EXEC_SCSI_CMD %%%		       |
;+-----------------------------------------------------------------------------+
SRB_ExecSCSICmd	struct 1t
;-------------------------------------------------------------------------------
SRB_Cmd		BYTE	?	; w: ASPI command code = SC_EXEC_SCSI_CMD
SRB_Status	BYTE	?	; r: ASPI command status byte
SRB_HaId	BYTE	?	; w: ASPI host adapter number
SRB_Flags	BYTE	?	; w: SCSI request flags
SRB_Hdr_Rsvd	DWORD	?	; 0: undef	- reserved
SRB_Target	BYTE	?	; w: target's	SCSI ID
SRB_Lun		BYTE	?	; w: target's LUN number
SRB_Rsvd1	WORD	?	; 0: reserved for alignment
SRB_BufLen	DWORD	?	; w:	data allocation	length
SRB_BufPointer	LPBYTE	?	; w: data buffer pointer
SRB_SenseLen	BYTE	?	; w: sense allocation length
SRB_CDBLen	BYTE	?	; w: CDB Length
SRB_HaStat	BYTE	?	; r: host Adapter Status
SRB_TargStat	BYTE	?	; r: target	Status
SRB_PostProc	PROCPTR	?	; w: post routine
SRB_Rsvd2	LPVOID	?	; w:	reserved
SRB_Rsvd3      BYTE 16t	dup (?)	; 0: reserved for alignment
;-------------------------------------------------------------------------------
SRB_CDBByte    BYTE 16t	dup (?)	; w: SCSI CDB
;-------------------------------------------------------------------------------
SRB_SenseArea  BYTE (SENSE_LEN+2) dup (?); r: request Sense buffer
;-------------------------------------------------------------------------------
SRB_ExecSCSICmd		ends
;-------------------------------------------------------------------------------
PSRB_ExecSCSICmd 	typedef	PTR SRB_ExecSCSICmd

ENDIF

;+-----------------------------------------------------------------------------+
;|		%%% SRB	- ABORT	AN SRB - SC_ABORT_SRB %%%		       |
;+-----------------------------------------------------------------------------+
SRB_Abort	struct 1t
;-------------------------------------------------------------------------------
SRB_Cmd		BYTE	?	; w: ASPI command code = SC_EXEC_SCSI_CMD
SRB_Status	BYTE	?	; r: ASPI command status byte
SRB_HaId	BYTE	?	; w: ASPI host adapter number
SRB_Flags	BYTE	?	; 0: undef for	command	- reserved
SRB_Hdr_Rsvd	DWORD	?	; 0: reserved for expansion
SRB_ToAbort	LPSRB	?	; w: pointer to SRB	to abort
;-------------------------------------------------------------------------------
SRB_Abort	ends
;-------------------------------------------------------------------------------
PSRB_Abort	typedef	PTR SRB_Abort


;+-----------------------------------------------------------------------------+
;|	      %%% SRB -	BUS DEVICE RESET - SC_RESET_DEV	%%%		       |
;+-----------------------------------------------------------------------------+
SRB_BusDeviceReset	struct 1t
;-------------------------------------------------------------------------------
SRB_Cmd		BYTE	?	; w: ASPI command code = SC_EXEC_SCSI_CMD
SRB_Status	BYTE	?	; r: ASPI command status byte
SRB_HaId	BYTE	?	; w: ASPI host adapter number
SRB_Flags	BYTE	?	; w: SCSI request flags (post enable/disable)
SRB_Hdr_Rsvd	DWORD	?	; 0: reserved
SRB_Target	BYTE	?	; w: target's	SCSI ID
SRB_Lun		BYTE	?	; w: target's LUN number
SRB_Rsvd1	BYTE 12t dup (?); 0: reserved	for alignment
SRB_HaStat	BYTE	?	; r: host adapter status
SRB_TargStat	BYTE	?	; r: target	status
SRB_PostProc	PROCPTR	?	; w: post routine
SRB_Rsvd2	LPVOID	?	; 0:	reserved
SRB_Rsvd3	BYTE 16t dup (?); 0: reserved
SRB_CDBByte	BYTE 16t dup (?); SCSI CDB
;-------------------------------------------------------------------------------
SRB_BusDeviceReset	ends
;-------------------------------------------------------------------------------
PSRB_BusDeviceReset	typedef		PTR SRB_BusDeviceReset


;+-----------------------------------------------------------------------------+
;|	      %%% SRB -	GET DISK INFORMATION - SC_GET_DISK_INFO	%%%	       |
;+-----------------------------------------------------------------------------+
SRB_GetDiskInfo		struct	1t
;-------------------------------------------------------------------------------
SRB_Cmd		BYTE 	?	; w: ASPI command code	= SC_EXEC_SCSI_CMD
SRB_Status	BYTE		?	; r:	ASPI command status byte
SRB_HaId	BYTE 	?	; w: ASPI host	adapter	number
SRB_Flags	BYTE 	?	; 0: SCSI request flags - undef
SRB_Hdr_Rsvd	DWORD	?	; 0: reserved
SRB_Target	BYTE		?	; w:	target's SCSI ID
SRB_Lun		BYTE 	?	; w: target's LUN number
SRB_DriveFlags	BYTE 	?	; r: drive flags
SRB_Int13HDriveInfo  BYTE  ?	; r: host adapter int13 drive number
SRB_Heads	BYTE 	?	; r: preferred number	of heads translation
SRB_Sectors	BYTE 	?	; r: preferred number of sectors translation
SRB_Rsvd1	BYTE 10t dup (?); 0: reserved
;-------------------------------------------------------------------------------
SRB_GetDiskInfo		ends
;-------------------------------------------------------------------------------
PSRB_GetDiskInfo	typedef		PTR SRB_GetDiskInfo


;+-----------------------------------------------------------------------------+
;|	      %%% SRB -	RESCAN SCSI BUS	- SC_RESCAN_SCSI_BUS (7) %%%	       |
;+-----------------------------------------------------------------------------+
SRB_RescanPort	struct 1t
;-------------------------------------------------------------------------------
SRB_Cmd		BYTE	?	; w: ASPI command code = SC_RESCAN_SCSI_BUS
SRB_Status	BYTE	?	; r: ASPI command status byte
SRB_HaId	BYTE	?	; w: ASPI host adapter number
SRB_Flags	BYTE	?	; 0: reserved
SRB_Hdr_Rsvd	DWORD	?	; 0: reserved
;-------------------------------------------------------------------------------
SRB_RescanPort	ends
;-------------------------------------------------------------------------------
PSRB_RescanPort	typedef	PTR SRB_RescanPort


;+-----------------------------------------------------------------------------+
;|	  %%% SRB - GET/SET TARGET TIMEOUTS SC_GETSET_TIMEOUTS (8) %%%	       |
;+-----------------------------------------------------------------------------+
SRB_GetSetTimeouts	struct 1t
;-------------------------------------------------------------------------------
SRB_Cmd		BYTE	?	; w: ASPI command code = SC_GETSET_TIMEOUTS
SRB_Status	BYTE	?	; r: ASPI command status byte
SRB_HaId	BYTE	?	; w: ASPI host adapter number
SRB_Flags	BYTE	?	; 0; SCSI request flags - undef
SRB_Hdr_Rsvd	DWORD	?	; 0: reserved
SRB_Target	BYTE	?	; w: target's	SCSI ID
SRB_Lun		BYTE	?	; w: target's LUN number
SRB_Timeout	DWORD	?	; w: timeout in half seconds
;-------------------------------------------------------------------------------
SRB_GetSetTimeouts	ends
;-------------------------------------------------------------------------------
PSRB_GetSetTimeouts		typedef		PTR SRB_GetSetTimeouts


;+-----------------------------------------------------------------------------+
;|			   %%% Iomega Sense Data %%%			       |
;+-----------------------------------------------------------------------------+
IOMEGA_SENSE_DATA	struct 1t
;-------------------------------------------------------------------------------
ISD_ErrorCode		BYTE	?	;	70h or 71h
ISD_reserved_0		BYTE	?
ISD_SenseKey		BYTE	?
ISD_Information		DWORD	?
ISD_AdditionalSenseLen	BYTE	?	;	always 11h
ISD_reserved_1		DWORD	?
ISD_ASC			BYTE	?
ISD_ASCQ		BYTE	?
ISD_reserved_2		BYTE	?
ISD_SKSV		BYTE	?
ISD_FormatProgress	WORD	?
ISD_CurrentTrack	WORD	?
ISD_Interleave		BYTE	?	; always	01
ISD_AcquireTrack	WORD	?
ISD_RetryCount		BYTE	?
ISD_RetryPhase		BYTE	?
;-------------------------------------------------------------------------------
IOMEGA_SENSE_DATA	ends


ScsiRequestBlock	UNION
;-------------------------------------------------------------------------------
hai		SRB_HAInquiry		<>
gdt		SRB_GDEVBlock		<>
abort		SRB_Abort		<>
io		SRB_ExecSCSICmd		<>
busreset	SRB_BusDeviceReset	<>
gdi		SRB_GetDiskInfo		<>
rsb		SRB_RescanPort		<>
gst		SRB_GetSetTimeouts	<>
;-------------------------------------------------------------------------------
ScsiRequestBlock	ENDS

;------------------------ Mode Sense / Select Page ID's	------------------------

ERROR_RECOVERY_PAGE	equ	01h
CACHING_PAGE		equ	08h
MODES_OF_OPERATION	equ	2Fh

;------------------------ GRC SCSI COMMAND RETURN CODES	------------------------

DEFECT_LIST_READ_ERROR	equ	001C0003h
LBA_TOO_LARGE		equ	00210005h	; accessed	a non-exist LBA
MEDIA_CHANGE_CODE	equ	00280006h	; media	was changed
INCOMPATIBLE_MEDIA	equ	00300002h	; 2Gb / 1Gb combo on "Read Defects"
MEDIA_NOT_PRESENT	equ	003A0002h
MEDIA_NOT_PRESENT	equ	003A0002h
DRIVE_COMING_READY	equ	00040102h
SCSI_CMD_TIMED_OUT	equ	00FFFF00h
BUFFER_TOO_BIG		equ	00FFFFE6h
MANUAL_INTERRUPTION	equ	   0FFFFFFFFh

;-/////////////////////////////////////////////////////////////////////////////-

