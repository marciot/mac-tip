;+-----------------------------------------------------------------------------+
;|   TIP.ASM			TROUBLE	IN PARADISE		   05/22/98    |
;+-----------------------------------------------------------------------------+
;
;------------------------------------ TO DO ------------------------------------
;
; o Inhibit Power Management during TIP	... systems are	sleeping!
;
; o WINME and USB!
;
; o TIP	sees Iomega CDRW as a ZIP drive!
;
; o Check operation with Jaz Traveller
;
; o Under NT4 the screensaver blanking killed TIP in mid-run.
;
; o Don't blank	the results after a run	and before the "view results" tab
;   is presed.	Somehow	keep the results around	longer.
;
; o Norton Anti-Virus (NAV) incompatibility.
;
; o Consider not forcing an eject of the "system" (boot) drive!
;
; o Suggest disconnection of other devices, scanners, toasters,	etc.
;
; o Do NOT allow TIP to	run on the main	system BOOT drive!!!!!!
;
; o Remove the ASPI_VERSION Page
;
; o Tell'em about Firmware revision: B.29 won't	work, 23.D does!
;
; o Add	suggestion to REMOVE ALL OTHER DEVICES from drive chain
;
; o Add	some cache flushing before locking for exclusive use!
;
; o Extra FAST,	extra SAFE "Read-Only" mode ...	(FOUR TIMES FASTER!)
;   -- OR should we save this for CIA??? --
;
; o Check for Eject-ability on 2GB Jaz while running!
;   mvongunten@acm.org (Marcel)
;
; o TIP	is not working with the	JazJet card and	the IOmegaNT.sys driver.
;   The	driver is invisible to the ASPI	layer.
;   "Earl, George" <George.Earl@ssa.gov>
;   "James Wynn" <jwynn@mail.mcg.edu>
;   Ron	Charlton <charltoncr@mindspring.com>
;
; o TIP	lockups	under NT4/SP3 after using ASPI_ME with an Adaptec 1542
;   controller.	 Several reports of this configuration problem.
;   Des	Gordon <Des.Gordon@digital.com>
;
; o NT Startup delay ?????
;   "Keating, Dan  (Miami)" <DKeating@herald.com>
;
; o "Don't Show	This Anymore" option on	the ASPI_ME warning screen.
;
; o Suppress Head Scrubs while running TIP?
;
; o Make ASPI_ME and TIP aware of Adaptec 4.57(1013) ...
;
; o Leave the Explain Results page in place, even AFTER	they remove the
;   disk, until	they insert another one	OR leave the Action Pages.

;-------------------------------- Fixed	in 2.2 ---------------------------------
;
; o USB	drive hangs.
;
; o 250	Mb Parallel port errors	at end
;
;------------------------------------- 3.0 -------------------------------------
;
; o Fixed spastic requests for passwords once the system is running.
;
; o Updated references Iomega to w95_551 to ioware9x.
;
;-------------------------------------------------------------------------------
;
; o Division error in: ASPI.ASM	-- main	work loop (line	964)
;   From: James	Bodine <bodine_j@FORTLEWIS.EDU>
;
; o She's getting the "buffer too big" error on	a SCSI Jaz drive!
;   Tip-test reports "00FFFFE6"
;   Dorothy Nyberg <dnyberg@premier1.net>
;
; o Somehow, SERIOUS ERRORS are	being missed at	the end	and people are
;   being told that everything is fine!	 (On Jaz disks at least!)
;
; o NOTIFY WHEN	1.01 is	ready ... Jason	Strickland <jstrick@oz.net>
;
;------------------------------------ 1.01 -------------------------------------
;
; Fixed	the links and pointers into Iomega's Site.
;
; Fixed	undefined error	reporting (when	drives report a	new error type)
;
; Added	Auto-adaptive buffer sizing.  It works to cure "buffer too big"
;
; Fixed	nasty Adapter=0/Device=0 bug!
;
; Fixed	"Copy" bug which had skewed pages!
;
; Fixed	non-password Write Protected cartridge dialog.
;
; Fixed	a "happened once" divison overflow error in: GetElapsedTimeInSeconds
;
; Fixed	Macintosh disks	reporting "needs exclusive access."
;   (Of	course,	since we can't log the drive's file system!)
;   Tim	Carter <tcarter@piedmontpublishing.com>
;
; Fixed	the "get exclusive access to drive" logic. If it can't find the
;   drive, it doesn't bother with trying to get	exclusive access.
;   -- Have Bennet confirm the fix!
;   paullim@singnet.com.sg
;   Rusty Gardhouse <imagenex@sentry.npsnet.com>
;   "Vince D. Kimball" <vkimball@shore.net>
;   Franz <starhan@kapsch.net>
;   Harold Annen <jhha@execpc.com>
;
; Fixed	weird window background	colors being used for background of
;   the	RichEdit controls!  let	"Jonathan Tham"	<jtham@snet.net> know.
;
; Fixed	2 Gb drive troubles: "Tom D. Bruce" <tdb@iname.com>
;
; Fixed	the re-word of the ASPI	upgrade	advice to down play its	requirement.
;
; Fixed	"Buffer	Too Big" now stops TIP cold!
;
;
;------------------------------------- 1.0 -------------------------------------
;
; Complete file	system locking and unlocking system to prevent TIP's scanning
; and modification of an in-use	volume,	and to prevent the filesystem
; from viewing any cartridge while TIP is scanning it.
;
; Major	rework of the cartridge	eject system --	through	the device driver
; rather than directly at the ASPI layer -- so the system KNOWS	a cartridge
; has been ejected and can remove it from the file system.
;
;------------------------------------ 0.99 -------------------------------------
;
; Eject	ALL media on TIP startup! (prevent weird post-format status reads)
;
;------------------------------------ 0.98 -------------------------------------
;
; ACTIVATE Self-Encryption!
;
; Activate PPA detection
;
; COPY button
;
; Handle Drive A: support
;
; Wheel	Mouse NOT operating in Win95!
;
; Hourglass after Win95	termination
;
; Spin-Down report gets	cleared!
;
; Change ASPI trouble to recommend ASPI_ME
;
;-------------------------------------------------------------------------------
;
; o determine TRUE maximum number of available JAZ relocations
;   This still needs to	be done, but for now I've clipped the test so if
;   we are told	there are more than the	maximum	we'll be okay.
;
; o lock drives	for use	under Win95/98 and NT ??????
; o perhaps LOCKING the	drive for exclusive use?
;   Can't Lock drive for exclusive use,	since the locking semantics are	for
;   LOGICAL (drive letter) devices, and	there's	no clear and simple API	for
;   determining	the LOGICAL drive letter from the PHYSICAL SCSI	device.
;
;-------------------------------------------------------------------------------

include	standard.inc
include	aspi.inc
include	richedit.inc
include	commctrl.inc
include	macros.inc
include	winioctl.inc

INITIAL_PAGE		=	INTRO_PAGE
; INITIAL_PAGE		= PERFORM_TEST_PAGE ; INTRO_PAGE


VERSION_EXPIRATION	= (2010 * 12	+ 1) ; if in or	after JAN 1, 2010
; VERSION_EXPIRATION	= (2004	* 12 + 10) ; if	in or after OCT	1st, 2004
; VERSION_EXPIRATION	= (2003	* 12 + 7) ; if in or after JULY	1st, 2003
; VERSION_EXPIRATION	= (2000	* 12 + 10) ; if	in or after OCT	1st, 2000
; VERSION_EXPIRATION	= (1999	* 12 + 7) ; if in or after JULY	1st, 1999
; VERSION_EXPIRATION	= (1998	* 12 + 10) ; if	in or after OCT	1st, 1998
;-------------------------------------------------------------------------------
SUPPRESS_LOGO_ANIMATION	= 0	; no logo
CORNER_THE_WINDOW	= 0	;	place dialog in	upper-left corner
SUPPRESS_STARTUP_EJECT	= 0
COPY_BUTTON_DAMAGES	= 0
DEVELOPMENT		= 0
NO_FLOATING_ANIMATION	=	0
FORCE_TROUBLE 		= 0
FORCE_NO_DRIVES		= 0
MINIMUM_VERSION		= 0	; set it to zero for NO minimum version checking
FORCE_STARTING_SECTOR	=	0 ; 489344 (near end of	250 Mb drive)
;-------------------------------------------------------------------------------

EXTERNDEF	FontBitmapImage	:DWORD	; our floating	"?" bitmap image
EXTERNDEF	RTF_Data	:DWORD
EXTERNDEF	SECTOR_Data	:DWORD

;+-----------------------------------------------------------------------------+
;|		     F U N C T I O N   P R O T O T Y P E S		       |
;+-----------------------------------------------------------------------------+
WndProc			PROTO	:HWND,	:UINT, :WPARAM,	:LPARAM
CenterWindow		PROTO 	:HWND
DecompressBinary	PROTO	:LPVOID
AllocateSixteenColorDib	PROTO	Hwidth:DWORD, Vheight:DWORD
SplashTheBitmap		PROTO	hDC:HDC
PrepForFastBlitting	PROTO	:HGLOBAL
ApplicationTimerProc	PROTO	:HWND, :UINT, :UINT,	:DWORD
EjectIomegaCartridge	PROTO	Adapter:DWORD, Device:DWORD
SpinUpIomegaCartridge	PROTO	Adapter:DWORD, Device:DWORD
SetRichEditText		PROTO	phRichEditControl:LPHWND, pText:LPSTR
PasswordWndProc		PROTO	:HWND, :UINT, :WPARAM, :LPARAM
ExclusiveAccess		PROTO	DriveNumber:DWORD, Exclusive:BOOL

SetTabErrorMode		PROTO	DisplayMode:BOOL
EjectAllMedia		PROTO	; unlock and eject	all drives

IF DEVELOPMENT
PostToScreen		PROTO	PostVal:DWORD
ENDIF

;+-----------------------------------------------------------------------------+
;|				 E Q U A T E S				       |
;+-----------------------------------------------------------------------------+
SCSI_TIMEOUT		equ	120 *	1000	; two minutes!

WIZARD_WINDOW_HEIGHT	equ	337	; 358 - 19
WIZARD_WINDOW_WIDTH	equ	467	; 480

NUMBER_OF_FLOATERS	equ	6
VELOCITY_MULTIPLIER	equ	1000

SMALLER_FONT_SIZE	equ	8
LARGER_FONT_SIZE	equ	10
HEADLINE_FONT_SIZE	equ	21

SPLASH_WIDTH		equ	120
SPLASH_HEIGHT		equ	258

COPY_LEFT		equ	0
COPY_TOP		equ	0
COPY_WIDTH		equ	462
COPY_HEIGHT		equ	289

FONT_WIDTH		equ	151
FONT_HEIGHT		equ	251

LOGO_1_LEFT		equ	157
LOGO_1_TOP		equ	57

LOGO_2_LEFT		equ	400
LOGO_2_TOP		equ	17

HYPERLINK_HEIGHT	equ	248

FILE_TIME_TICKS_PER_SECOND	equ	10000000

SOURCE_BITMAP_BYTE_WIDTH  equ	((FONT_WIDTH+31)/32)*4
DEST_BITMAP_BYTE_WIDTH		 equ	(SPLASH_WIDTH / 2)
FONT_TOP_OFFSET		  equ	SIZEOF BITMAPFILEHEADER + SIZEOF	BITMAPINFOHEADER + 2*4 + ((FONT_HEIGHT-1) * SOURCE_BITMAP_BYTE_WIDTH)

RANDOMULT		equ	0B6CAEB15h

PALETTE_SIZE		equ	16	; Splash is a 16-color bitmap

IDB_LINEUP		equ	WM_APP
IDB_LINEDN		equ	WM_APP+1
IDB_PAGEUP		equ	WM_APP+2
IDB_PAGEDN		equ	WM_APP+3
IDB_END			equ	WM_APP+4
IDB_HOME		equ	WM_APP+5

FLASH_COUNT		equ	16
RANDOMULT		equ	0B6CAEB15h

FILE_COUNT		equ	4
DRIVER_DATE		equ	9111	;	12/23/97

; offsets to the various sector	data images

ZIP_100_PART		equ	0000h
ZIP_100_BOOT		equ	0200h
ZIP_250_PART		equ	0400h
ZIP_250_BOOT		equ	0600h
JAZ_1GB_PART		equ	0800h
JAZ_1GB_BOOT		equ	0A00h
JAZ_2GB_PART		equ	0C00h
JAZ_2GB_BOOT 		equ	0E00h

;--------------------------- Password Dialog Defines ---------------------------

IDC_PASSWORD		equ	3

;-------------------------- Drive Array	Status Flags ---------------------------

;				     +---- Device
;				     | +--- Adapter
;				     v v
JAZ_DRIVE		equ	00010000h
MEDIA_CHANGED		equ	00020000h
DISK_EJECTING		equ	00040000h ; we've asked for eject and waiting ...
ODD_BYTE_COMPENSATION	equ	00080000h ; special handling for ODD length PSWD
MAX_DRIVE_COUNT		equ	16	; we can handle	up to 16 Zip/Jaz drives

FORMAT_STATUS_PAGE	equ	1
DISK_STATUS_PAGE	equ	2
 NEW_DISK_STATUS_OFFSET	equ	3	;	newer offset of	the Disk Status	Byte
 OLD_DISK_STATUS_OFFSET	equ	1	;	older "	"	"	"

JAZ_SPARES_COUNT_OFFSET			equ	68 ; offsets into	DiskStat tbl
NEW_ZIP_SIDE_0_SPARES_COUNT_OFFSET	equ	13
NEW_ZIP_SIDE_1_SPARES_COUNT_OFFSET	equ	17
OLD_ZIP_SIDE_0_SPARES_COUNT_OFFSET	equ	11
OLD_ZIP_SIDE_1_SPARES_COUNT_OFFSET	equ	15
JAZ_PROTECT_MODE_OFFSET			equ	21
NEW_ZIP_PROTECT_MODE_OFFSET		equ	21
OLD_ZIP_PROTECT_MODE_OFFSET		equ	19
JAZ_LAST_LBA_OFFSET			equ	5
NEW_ZIP_LAST_LBA_OFFSET			equ	5
OLD_ZIP_LAST_LBA_OFFSET			equ	3

DRIVE_A_SUPPORT_BIAS			equ	32 ;	reduce total by	32 for DRIVE A support

MINIMUM_JAZ_SPARES			equ	500
MAXIMUM_JAZ_SPARES			equ	2557
MINIMUM_ZIP_SPARES			equ	50
MAXIMUM_ZIP_SPARES			equ	126

BYTES_PER_SECTOR	equ	512
LOG2_BYTES_PER_SECTOR	equ	9
MAX_SECTORS_PER_TEST	equ	128
MAX_TRANSFER_PER_TEST	equ	(MAX_SECTORS_PER_TEST	* BYTES_PER_SECTOR)
BPB_OFFSET		equ	11
SECTORS_PER_FAT		equ	11

BADNESS_THRESHOLD	equ	10

;------------------------------	Cartridge Status -------------------------------

DISK_STATUS_UNKNOWN	equ	1
DISK_AT_SPEED		equ	2
DISK_SPINNING_UP	equ	3
DISK_NOT_PRESENT	equ	4
DISK_SPUN_DOWN		equ	5
DISK_STALLED		equ	6
DISK_Z_TRACK_FAILURE	equ	7
DISK_PROTECTED		equ	8
DISK_LOW_SPARES		equ	9
DISK_TEST_UNDERWAY	equ	10
DISK_TEST_FAILURE	equ	11
;-------------------------------------------------------------------------------
LAST_CART_STATUS	equ	11

;---------------------------- Testing Phase Status -----------------------------

UNTESTED		equ	0
READY_TO_TEST		equ	1
TESTING_STARTUP		equ	2
READING_DATA		equ	3
WRITING_PATT		equ	4
READING_PATT		equ	5
WRITING_DATA		equ	6

;----------------------------- Drive Letter Search -----------------------------

COMPARISON_SECTORS	equ	8
COMPARISON_BYTES	equ	(COMPARISON_SECTORS * BYTES_PER_SECTOR)

;+-----------------------------------------------------------------------------+
;|			  NON-COMPRESSED GLOBAL	DATA			       |
;+-----------------------------------------------------------------------------+
				.data
				ALIGN	4

;------------------------- ANIMATION MOTION CONSTANTS --------------------------

xleft		DWORD	NUMBER_OF_FLOATERS	dup (0)
ytop		DWORD	NUMBER_OF_FLOATERS dup (FONT_TOP_OFFSET)
xwidth		DWORD	NUMBER_OF_FLOATERS dup (FONT_WIDTH)
yheight		DWORD	NUMBER_OF_FLOATERS dup (FONT_HEIGHT)
transform	DWORD	NUMBER_OF_FLOATERS dup (Xform1)
Xform1		DWORD	 4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14, 15, 15, 15, 15, 15
ObjectTypes	DWORD	0,1*4,2*4,3*4,4*4,5*4
; this gives same-width	objects	different velocities, otherwise	they
; would	all be running at exactly the same speed with boring overlaps
velocities	DWORD	FONT_WIDTH,	FONT_WIDTH+20, FONT_WIDTH+40
		DWORD	FONT_WIDTH+60, FONT_WIDTH+80, FONT_WIDTH+100

LogoPalette	WORD	0300h		    ; LOGPALETTE HEADER	WORDS
		WORD	PALETTE_SIZE
SplashPalette	DWORD	00A10303h, 00AD1F20h, 00B32E30h, 00B93E40h
		DWORD	00BC4649h, 00C35559h, 00C65E62h, 00CB6A6Eh
		DWORD	00D27A80h, 00DA8D93h, 00DF99A0h, 00E09DA4h
		DWORD	00E6ABB3h, 00EDBCC5h, 00FCE1EAh, 00FEEAF1h

FourBitMasks	DWORD	0000000Fh, 000000F0h, 00000F00h, 0000F000h
		DWORD	000F0000h, 00F00000h, 0F000000h, 0F0000000h

;===============================================================================
CommandDetailsTable		LABEL	BYTE	;  Command
;-------------------------------------------------------------------------------
db	SCSI_Cmd_RequestSense	, SRB_DIR_IN	;	03  IN == get from drive
db	SCSI_Cmd_FormatUnit	, 0		; 04 OUT ==	send to	drive
db	SCSI_Cmd_NonSenseData	, SRB_DIR_IN	;	06
db	SCSI_Cmd_Read		, SRB_DIR_IN	; 08
db	SCSI_Cmd_Write		, SRB_DIR_OUT	; 0A
db	SCSI_Cmd_CartProtect	, SRB_DIR_OUT	;	0C
db	SCSI_Cmd_Inquiry	, SRB_DIR_IN	; 12
db	SCSI_Cmd_ModeSelect	, SRB_DIR_OUT	; 15
db	SCSI_Cmd_ModeSense	,	SRB_DIR_IN	; 1A
db	SCSI_Cmd_StartStopUnit	, 0		; 1B
db	SCSI_Cmd_SendDiagnostic	, 0		; 1D
db	SCSI_Cmd_PreventAllow	, 0		;	1E
db	SCSI_Cmd_TranslateLBA	, SRB_DIR_IN	;	22
db	SCSI_Cmd_FormatTest	, 0		; 24
db	SCSI_Cmd_ReadMany	, SRB_DIR_IN	; 28
db	SCSI_Cmd_WriteMany	,	SRB_DIR_OUT	; 2A
db	SCSI_Cmd_Verify		, 0		; 2F
db	SCSI_Cmd_ReadDefectData	, SRB_DIR_IN	; 37
db	SCSI_Cmd_ReadLong	, SRB_DIR_IN	; 3E
db	SCSI_Cmd_WriteLong	,	SRB_DIR_OUT	; 3F

;-------------------------------------------------------------------------------
LENGTH_OF_DETAILS_TABLE		= $ - CommandDetailsTable

RichEditStreamIn	EDITSTREAM <NULL,NULL,OFFSET StreamSource>

;----------------------- Test Monitor Panel Definitions	------------------------

CS_Stat		RECT	<114,  8,	242, 28>
TP_Perc		RECT	<	12, 56,	409, 72>
SS_Jaz		RECT	< 12, 94, 409,126>
SS_Sid0		RECT	<	12, 94,	409,110>
TL_Sect		RECT	<	75,154,	203,170>
ES_Read		RECT	<346,154,	409,170>
SE_Rect		RECT	<222,154,	255,221>

;+-----------------------------------------------------------------------------+
;|			    COMPRESSED GLOBAL DATA			       |
;+-----------------------------------------------------------------------------+
			ALIGN	4
StartEncryption		CHAR	"SOE!"
;-------------------------------------------------------------------------------
IF VERSION_EXPIRATION
szBetaExpiredTitle	CHAR	"TIP Version 2.1b Has Expired .	. .",0
szBetaExpiredText	CHAR	"In order to insure that	only the highest quality software is in	use, this version 2.1b",CR
CHAR	"release of 'Trouble In Paradise' was designed to expire on January 1st of	2010.",CR
CHAR	"In this way, every copy can soon be replaced with	a more mature edition that",CR
CHAR	"gains from everything we will have learned during	TIP's initial months of	life.",CR
CHAR	CR
CHAR	"Since this program is FREEWARE, you may download the replacement release",CR
CHAR	"immediately from my web site at http://grc.com.	I apologize for	the trouble,",CR
CHAR	"and I hope this delay will not cause you any undue inconvenience.",CR
CHAR	CR
CHAR	"Thanks for understanding						    -- Steve Gibson",0
ENDIF
;-------------------------------------------------------------------------------
include	tip.dat
include	tip.txt
;-------------------------------------------------------------------------------
szAppName		CHAR	"TIP",0
szNullString		CHAR	0
szWindowTitle		CHAR	" TIP 2.1b -- Zip &	Jaz Drive and Cartridge	Testing	System",0

szRedBitmap		CHAR	"REDBITMAP",0
szGreenBitmap		CHAR	"GRNBITMAP",0
szOffBitmap		CHAR	"OFFBITMAP",0
szSoundBitmap		CHAR	"SOUND",0
szWheelRollMessage	CHAR	"MSWHEEL_ROLLMSG",0

ChildRegionRect		RECT	<0,0, 460,285>
TabWindowRegionRect	RECT	<20,45, 440,270>
SplashRect		RECT	<14,16,138,278>
WebButton		RECT	<12,295,140,330>

szCopyright_1		CHAR	"Copyright (c) 2006	by",0
szCopyright_2		CHAR	"Gibson Research Corp.",0
szCrLf			CHAR	CR,LF

szASPI32DLLName		CHAR	"wnaspi32.dll",0
szASPI32SupportInfo	CHAR	"GetASPI32SupportInfo",0
szASPI32Command		CHAR	"SendASPI32Command",0
szCloseCmd		CHAR	"Close",0
szRichEditLibrary	CHAR	"RichEd32.Dll",0
szIomega		CHAR	"Iomega",0
szZip			CHAR	"Zip",0
szJaz			CHAR	"Jaz",0
szPPA3			CHAR	"ppa3",0

szShellOpenOp		CHAR	"open",0
szShellOpenFile		CHAR	"http://grc.com",0
szURLHellier1		CHAR	"http://grc.com/files/hellier1.wav",0
szURLHellier2		CHAR	"http://grc.com/files/hellier2.wav",0

szFullWin95Text		CHAR	"All Files for Win95/98 (4,375k)",0
szFullWin95URL		CHAR	"ftp://ftp.iomega.com/pub/english.public/ioware9x.exe",0
szMinimumWin95Text	CHAR	"Fewest	Files for 95/98	(811k)",0
szMinimumWin95URL	CHAR	"ftp://ftp.iomega.com/pub/english.public/ioware9xdrv.exe",0

szFullWinNTText		CHAR	"All Files for WinNT (4,240k)",0
szFullWinNTURL		CHAR	"ftp://ftp.iomega.com/pub/english/iowarent.exe",0
szMinimumWinNTText	CHAR	"Fewest	Files for NT (937k)",0
szMinimumWinNTURL	CHAR	"ftp://ftp.iomega.com/pub/english/iowarentdrv.exe",0

;-------------------------------------------------------------------------------
szWebFailedTitle	CHAR	"Unable to Launch	a Web Browser",0
szWebFailedText		CHAR	"TIP was unable to successfully launch",CR,LF
			CHAR	"this system's Internet	web Browser.",CR,LF
			CHAR	CR,LF
			CHAR	"Please	check out the http://grc.com website",CR,LF
			CHAR	"to find out what's new	!",0
;-------------------------------------------------------------------------------
szCantUnlockTitle	CHAR	"TIP was	unable to UNLOCK this drive!",0
szCantUnlockText	CHAR	"Trouble In Paradise was unable to successfully",CR
			CHAR	"remove	the Read or Read/Write protection from",CR
			CHAR	"this cartridge.  In order to use it you will need",CR
			CHAR	"to use	the standard Iomega Password Utility, then",CR
			CHAR	"return	to TIP for it's	analysis of the	drive and",CR
			CHAR	"cartridge.	Sorry for the inconvenience!",0
;-------------------------------------------------------------------------------
szDisablingWPTitle	CHAR	"Disabling Cartridge's Write Protection",0
szDisablingWPText	CHAR	"The cartridge you've inserted was write	protected",CR
			CHAR	"without a password.  TIP must disable the write ",CR
			CHAR	"protection in order to	operate.  It will be restored",CR
			CHAR	"when the cartridge is removed from the	drive.",0
;-------------------------------------------------------------------------------
szBadFileTitle		CHAR	"This Program File	is Damaged!",0
szBadFileText		CHAR	"Whoops!!	This program's original	file image",CR
			CHAR	"has been altered somehow and is NOT SAFE",CR
			CHAR	"to use!  Please obtain	a fresh	copy of	tip.exe.",0

;-------------------------------------------------------------------------------
szNoExclusiveTitle	CHAR	"TIP Needs Exclusive Access to Cartridge "
szExclusiveDrive1	CHAR	"X:",0
szNoExclusiveText	CHAR	"TIP was	unable to acquire exclusive access to cartridge	"
szExclusiveDrive2	CHAR	"X:",CR
			CHAR	"because some of the cartridge's files are open	and in use,",CR
			CHAR	"or there is an	active view into the contents of the cartridge.",CR
			CHAR	CR
			CHAR	"Please	click the  OK  button below, close any open files or",CR
			CHAR	"views of the cartridge's contents, then click the ",22h,"Press	to",CR
			CHAR	"Begin",22h, " button again to start TIP's analysis of this cartridge.",0

;-------------------------------------------------------------------------------
IF MINIMUM_VERSION
szVersionErrorTitle	CHAR	"Windows Version Trouble",0
szVersionErrorText	CHAR	"This 32-bit Windows client program requires",CR,LF
 			CHAR	"Windows 95/98	or later, or Windows NT	4.0 or",CR,LF
			CHAR	"later,	 These operating environments natively ",CR,LF
			CHAR	"support the ",34,"ASPI	for Win32",34,"	subsystem which",CR,LF
			CHAR	"this program requires for operation.",0
ENDIF
;-------------------------------------------------------------------------------
szPerhapsBlastTitle	CHAR	"Partition Table Not Found on "
CartSize1		CHAR	"Zip 100 Cartridge!",0
szPerhapsBlastText	CHAR	"TIP has not found a valid partition table on this "
CartSize2		CHAR	"Zip 100 cartridge.",CR
			CHAR	CR
			CHAR	"If this cartridge does	NOT contain a standard DOS/Windows file",CR
			CHAR	"system, then this is to be expected.  (For example, if	the cartridge is in",CR
			CHAR	"Macintosh format.)  But if this cartridge has been recently damaged and",CR
			CHAR	"has just become unreadable, TIP has the ability to recreate the original",CR
			CHAR	"DOS/Windows FAT-16 file system	partition table, boot sector, etc.",CR
			CHAR	CR
			CHAR	"You should -- ONLY -- press OK	below if this cartridge	has just been",CR
			CHAR	"damaged -- presumably by a previous troubled run of TIP.  This	will",CR
			CHAR	"recreate a standard partition table and file system on	this cartridge.",CR
			CHAR	CR
			CHAR	"DO NOT	PRESS OK unless	you are	SURE that you want to reconstitute",CR
			CHAR	"a damaged and lost DOS/Windows	FAT-16 file system. This automatic",CR
			CHAR	"reconstructor is NOT COMPATIBLE with FAT-32, NTFS, Mac, or any",CR
			CHAR	"other non-FAT-16 file system.	If you are not sure ...",CR
			CHAR	CR
			CHAR	"Press CANCEL to leave the cartridge as	is, and	proceed	to run TIP.",0

;-------------------------------------------------------------------------------
szBlastTroubleTitle	CHAR	"Unable to Restore this "
CartSize3		CHAR	"Zip 100 Cartridge's File System!",0
szBlastTroubleText	CHAR	"Basic Sector Read/Write Operations on the Cartridge Have Failed!",CR
			CHAR	CR
			CHAR	'This means that the damage on the cartridge is	at a "lower level"',CR
			CHAR	"than TIP or the operating system is reading and writing. TIP could",CR
			CHAR	"not have been responsible for causing the damage to this cartridge",CR
			CHAR	"because TIP only performs standard read and write operations using",CR
			CHAR	"standard operating system I/O.",CR
			CHAR	CR
			CHAR	"The data on this cartridge *might* be salvageable through other means",CR
			CHAR	"but it	is beyond the scope of TIP's automatic file system reconstruction.",0

;-------------------------------------------------------------------------------
szBlastSuccessTitle	CHAR	"Successfully Restored	this "
CartSize4		CHAR	"Zip 100 Cartridge's File System!",0
szBlastSuccessText	CHAR	"TIP has successfully reconstructed this "
CartSize5		CHAR	"Zip 100 cartridge's Partition",CR
			CHAR	"Table,	FAT-16 Boot Sector, and	16-bit File Allocation Tables (FAT's).",CR
			CHAR	CR
			CHAR	"The cartridge should now be completely	readable and writeable through",CR
			CHAR	"any standard Microsoft	operating system.",0

;-------------------------------------------------------------------------------
szArialFontName		CHAR	"Arial",0
szCourierNew		CHAR	"Courier New",0

;--------------------------- FONT CREATION CONSTANTS ---------------------------

CurrentPage		DWORD	INITIAL_PAGE	; 0 -to- MAX ... the page we're	seeing
PreviousPage		DWORD	-1	; forces	a first	draw of	every control
ActionsSubPage		DWORD	FIRST_ACTION_PAGE
;-------------------------------------------------------------------------------
OurTypeFaceSpec	DWORD	-9, 0, 0,	0	; both fonts are 9 pixels high
OurFontWeight	DWORD	FW_NORMAL	;	either bold or normal weight
		BYTE	0
OurUnderline	BYTE	0, 0
OurCharSet	BYTE	DEFAULT_CHARSET
OurFontPrecise	BYTE	OUT_RASTER_PRECIS, 0, PROOF_QUALITY
PitchAndFamily	BYTE	0
OurFontName	CHAR	"MS Sans Serif", 0

szSide0		CHAR	"Side 0",0
szSide1		CHAR	"Side 1",0
szSpaceDashSpace	CHAR	"	- ",0

szBarChartPercent	CHAR	" %ld%% ",0
szCenteredDecimal	CHAR	"%ld",0
szCenteredHex		CHAR	"ErrorCode:	%06lX",0
szHoursMinsSecs		CHAR	"%ld:%02ld:%02ld",0

LBAMidPoints		DWORD	350000, 1250000, 3000000

IomegaDeviceNames	CHAR	'Zip 100	'
			CHAR	'Zip 250 '
			CHAR	'Jaz 1GB '
			CHAR	'Jaz 2GB '

;------------------------------	Driver FileNames -------------------------------

szWnaspi3295		CHAR	"\wnaspi32.dll",0
szWnaspi32NT		CHAR	"32\wnaspi32.dll",0
szWinaspi		CHAR	"\winaspi.dll",0
szAspi32		CHAR	"32\drivers\aspi32.sys",0
szWowpost		CHAR	"\wowpost.exe",0
szApix			CHAR	"\iosubsys\apix.vxd",0
szAspienum		CHAR	"\aspienum.vxd",0

DriversFor95	DWORD	szWnaspi3295, szWinaspi, szApix, szAspienum
DriversForNT	DWORD	szWnaspi32NT, szWinaspi, szAspi32, szWowpost

;-------------------------- Drive Letter Search	Data ---------------------------

DiskDrive		CHAR	"\\.\"	; used by NT to open direct drive
DriveLetter		CHAR	"A:",0	; drive letter	to open	...
SystemVXD		CHAR	"\\.\vwin32",0
DriveRootName		CHAR	"A:\",0

;-------------------------------------------------------------------------------
			ALIGN	4
EndEncryption		CHAR	"EOE!"
;-------------------------------------------------------------------------------

CurrentAdapter		DWORD	-1	; the adapter that's been recognized
CurrentDevice		DWORD	-1	; the device that's been recognized
RandomSeed		DWORD	100

;---------------------------- Development Only Data ----------------------------
IF DEVELOPMENT
POSTING_SPACE		equ	5
szLongHexFormat		CHAR	"	%08lX ",0	; a wsprintf format spec.
LastPostedValue		DWORD	-1		; posting to	the screen
LastPostedTop		DWORD	0		; the height of	the posting
LastPostedLeft		DWORD	0
ENDIF

JazDrive		DWORD	BOOL	; true if the current drive is JAZ
CartridgeStatus		DWORD	DISK_NOT_PRESENT

;---------------------------- Win95 Sound Variables ----------------------------

Period			WORD	0
Duration		WORD	0
PhasorPosition		WORD	0

;-------------------------------------------------------------------------------
CopySourceHandles	LABEL	HWND	; the table of RichEdit hWnds
;-------------------------------------------------------------------------------
			HWND	0, 0	; 0 (Intro	Page)
hRichEdit		HWND	?, ?	; 1 handle	and pTextLastSet
hPPAVersion		HWND	?, ?	; 2 text	of the PPA version message
hRichVersion		HWND	?, ?	; 3 text of the	ASPI version message
			HWND	0, 0	; 4 (Trouble Page)	/ (Select Drive)
			HWND	0, 0	; 6 (Perform Test)
hTabText		HWND	?, ?	; 7	text appearing on the perform test
hQuoteEdit		HWND	?, ?	;	8 the Iomega Quote page
			HWND	0, 0	; 9 (Credits Page)
;-------------------------------------------------------------------------------

INTRO_PAGE		equ	0
INSTRUCTION_PAGE	equ	1
PPA_VERSION_PAGE	equ	2
ASPI_VERSION_PAGE	equ	3
SELECT_DRIVE_PAGE	equ	4
PERFORM_TEST_PAGE	equ	5
EXPLAIN_RESULTS		equ	6
QUOTE_PAGE		equ	7
CREDITS_PAGE		equ	8


;-------------------------------------------------------------------------------
				  .data?
;+-----------------------------------------------------------------------------+
;|		       Uninitialized Static Global Data			       |
;+-----------------------------------------------------------------------------+
hMainWnd		HWND	?	;
MainWndMsg		MSG	<>
hInst	   		HINSTANCE ?	; and handle to our main instance
hRichEditLibrary	HANDLE	?
hAccel			HACCEL	?
hCompletionEvent	HANDLE	?
hApplicationTimer	DWORD	?
hSplashPalette		HPALETTE ?
WinNT			BOOL	?
OldPPA3Driver		BOOL	?
BadASPIDrivers		BOOL	?
MouseWheelAccrual	DWORD	?	; amount of pending mouse wheel movement

HyperlinkRect		RECT	<?>
HyperlinkPoint		POINT	<?>
hHyperlinkCursor	HCURSOR	?
hStandardCursor		HCURSOR	?
WindowFillColor		DWORD	?

WheelRollMessage	DWORD	?	; wheel mouse roll message ...	if any

;------------------- Pointers to all of	the richtext blocks --------------------

pFontBitmap		LPVOID	?
pSplashDIB		LPVOID	?
pRTF_Data		LPVOID	?, ?	; pointer followed by stream length
pSectorData		LPVOID	?
pTargetBuffer		LPVOID	?
pSearchBuffer		LPVOID	?

hDriveAccess		HANDLE	?
OneBasedDrive		DWORD	?
PartitionOffset		DWORD	?

StreamPointer		LPBYTE	?	; pointer to richedit stream
StreamEnd		LPBYTE	?	; pointer to past the end of stream

hASPIDLL		HANDLE	?
lpGetASPI32SupportInfo	LPVOID	?
lpSendASPI32Command	LPVOID	?

SystemTime		SYSTEMTIME	<?>

ASPI_CmdBlock		ScsiRequestBlock	<>

ErrorMode		BOOL	?

hActionTabs		HWND	?
hTestMonitor		HWND	?
hSoundCheckbox		HWND	?
hTestButton		HWND	?
hExitButton		HWND	?
hFullURL		HWND	?
hMinURL			HWND	?

hNullPen		HPEN	?
hWhitePen		HPEN	?
hGrayPen		HPEN	?
hDkGrayPen		HPEN	?
hBlackPen		HPEN	?

hNullBrush		HBRUSH	?
hWhiteBrush		HBRUSH	?
hGrayBrush		HBRUSH	?
hDkGrayBrush		HBRUSH	?
hBlackBrush		HBRUSH	?
hWindowFillBrush	HBRUSH	?

hNormalFont		HFONT	?	; 00 - small / normal
hBoldFont		HFONT	?	; 01	- small	/ bold
hDialogTextFont		HFONT	?	; 10 -	large /	normal
hTitleFont		HFONT	?	; 11 - large / bold
hHeadlineFont		HFONT	?	; way big font
hFixedWidthFont		HFONT	?
hDialogTextUnderlined	HFONT	?

hRedBitmap		HBITMAP	?
hGreenBitmap		HBITMAP	?
hOffBitmap		HBITMAP	?
hSoundBitmap		HBITMAP	?

;------------------------ FastTransparentBlt Parameters	------------------------

PixelsPerLine		DWORD	?
PixelsToBlt		DWORD	?	; copied from PixelsPerLine per line
LineCount		DWORD	?
SourceLineStart		DWORD	?
SourceBitStart		DWORD	?
DestLineStart		DWORD	?
DestBitStart		DWORD	?

xform			DWORD	16 dup(?)

HorzVelocity		DWORD	NUMBER_OF_FLOATERS dup (?)	; 16o16 format
VertVelocity		DWORD	NUMBER_OF_FLOATERS dup (?)	;    "	 "
HorzPosition		DWORD	NUMBER_OF_FLOATERS dup (?)	;    "	 "
VertPosition		DWORD	NUMBER_OF_FLOATERS dup (?)	;    "	 "

ActionButtonFlasher	DWORD	?

pUserDataBuffer		LPVOID	?	; pointer to user's data
pPatternBuffer		LPVOID	?	; pointer to test pattern

DriveCount		DWORD	?
DriveArray		DWORD	MAX_DRIVE_COUNT dup (?)

LastLBAOnCartridge	DWORD	?
StartingInstant		FILETIME <?>
NumberOfLBAs		DWORD	?
AdapterMaxSectors	DWORD	?
AdapterMaxBytes		DWORD	?

;----------------------------- Run Time	Variables ------------------------------

Side_0_SparesCount	DWORD	?	; JAZ has only one count
Side_1_SparesCount	DWORD	?	; ZIP has counts for	both sides
Initial_Side_0_Spares	DWORD	?
Initial_Side_1_Spares	DWORD	?

CartridgePassword	CHAR	32+1 dup(?)

FirstTestingVariable	LABEL	DWORD
;-------------------------------------------------------------------------------
TestingPhase		DWORD	?	;	0 = not	testing, no data ...
PercentComplete		DWORD	?
FirstLBASector		DWORD	?
LastLBASector		DWORD	?
SecondsElapsed		DWORD	?
SoftErrors		DWORD	?
FirmErrors		DWORD	?
HardErrors		DWORD	?
SectorsNotFound		DWORD	?
TracksNotFound		DWORD	?
ElapsedTimeOfLastEstimate DWORD	?
CurrentTotalTimeEstimate  DWORD	?
UserInterrupt		DWORD	?
LastError		DWORD	?
SingleTransferLBA	DWORD	?
UnknownErrorMode	DWORD	?
;-------------------------------------------------------------------------------
TESTING_VARIABLE_COUNT	equ	($ -	FirstTestingVariable)/SIZEOF DWORD


				.code
;+-----------------------------------------------------------------------------+
;|				  WinMain				       |
;|-----------------------------------------------------------------------------|
;|									       |
;|   Startup the Windows program. If no	previous instances are running then    |
;|   define and	register a new window class, perform various initialization    |
;|   chores, then setup	the main message loop.				       |
;|									       |
;+-----------------------------------------------------------------------------+
Start:	; call	DecryptOurselves
	call	VerifyFileChecksum	; THIS	DOES NOT RETURN	IF ERROR!

IF VERSION_EXPIRATION
;+-----------------------------------------------------------------------------+
;|	For the	Beta Test Period ... check to see if it's before April!	       |
;+-----------------------------------------------------------------------------+
	invoke	GetLocalTime, ADDR SystemTime
	movzx	eax, SystemTime.wYear
	imul	eax, 12		; scale the year	into number of months
	add	ax, SystemTime.wMonth
	.IF (eax >= VERSION_EXPIRATION)
		invoke	MessageBox, hMainWnd, ADDR szBetaExpiredText, ADDR szBetaExpiredTitle,	MB_APPLMODAL or	MB_OK
		jmp	Close
	.ENDIF
ENDIF

	invoke	GetVersion	; find out what platform we're running ...
IF MINIMUM_VERSION
	.IF (al < MINIMUM_VERSION)
		invoke	MessageBox, NULL, ADDR	szVersionErrorText, ADDR szVersionErrorTitle, MB_OK
		invoke	ExitProcess, 1
	.ENDIF
ENDIF
	.IF !(eax & 80000000h)
		set	WinNT
	.ENDIF
	;컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴
	invoke	GetModuleHandle, NULL	;	get our	module's handle
	mov	hInst, eax
	call	CheckPriorInstances
	jc	Close
	call	RegisterWindowClasses
	jc	Close
	call	CreateStaticResources
	jc	Close
	call	CreateAppWindows
	jc	Close

	;컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴
	; so far so good. now see if we have aspi services ...
	call	CheckForASPI		; returns the adapter count
	.IF (eax)
		; aspi is available, so count	the Iomega devices
		call	EnumerateIomegaDevices
;		.IF (eax == 1)
			; we	have only one, so select it for	the user
;			movmov	byte ptr CurrentAdapter, al, byte ptr DriveArray[0]
;			movmov	byte ptr CurrentDevice,	al, byte ptr DriveArray[1]
;		.ELSEIF	(eax > 1)
			; we	have more than one, so eject'em	all ...
;			call	EjectAllMedia
;		.ENDIF
	.ENDIF
	call	InitializeWizardControls
	call	InitializeTheFloaterSystem
	; now startup the timer for real-time features
	call	StartApplicationTimer

;----------------------- Our application's message loop	------------------------

	invoke	ShowWindow, hMainWnd, SW_NORMAL
	invoke	UpdateWindow, hMainWnd
	; get the main	window up on to	the screen
	call	ProcessPendingMessages

	; now wait while we eject all of the cartridges!
IFE SUPPRESS_STARTUP_EJECT
	call	EjectAllMedia
ENDIF

	.WHILE	TRUE
		invoke  GetMessage, ADDR MainWndMsg, NULL, 0,	0
		.BREAK .IF (!eax)
		invoke	TranslateAccelerator, hMainWnd, hAccel, ADDR MainWndMsg
		.CONTINUE .IF	(eax)	;	skip if	we handled
		invoke  TranslateMessage, ADDR MainWndMsg
		invoke  DispatchMessage,  ADDR MainWndMsg
	.ENDW

	.IF (hASPIDLL)			; only if we have ASPI services
		call	UnlockAllMedia	;	within this system ...
	.ENDIF

Close:	call	DeleteStaticResources
	invoke	ExitProcess, 0
;-/////////////////////////////////////////////////////////////////////////////-


;+-----------------------------------------------------------------------------+
;|			    START APPLICATION TIMER			       |
;+-----------------------------------------------------------------------------+
StartApplicationTimer	PROC
;-------------------------------------------------------------------------------
	invoke	SetTimer, NULL,	NULL, 50, ADDR ApplicationTimerProc
	mov	hApplicationTimer,	eax
	ret
;-------------------------------------------------------------------------------
StartApplicationTimer	ENDP


;+-----------------------------------------------------------------------------+
;|			    STOP APPLICATION TIMER			       |
;+-----------------------------------------------------------------------------+
StopApplicationTimer	PROC
;-------------------------------------------------------------------------------
	mov	eax, hApplicationTimer
	.IF (eax)
		invoke	KillTimer, NULL, eax
		reset	hApplicationTimer
	.ENDIF
	ret
;-------------------------------------------------------------------------------
StopApplicationTimer	ENDP


;-------------------------------------------------------------------------------
COMMENT	@
;+-----------------------------------------------------------------------------+
;|			       DECRYPT OURSELVES			       |
;+-----------------------------------------------------------------------------+
DecryptOurselves	PROC
;-------------------------------------------------------------------------------
	mov	esi, OFFSET StartEncryption
	.IF (dword ptr	[esi] != "!EOS")
		mov	ecx, OFFSET EndEncryption
		sub	ecx, esi
		shr	ecx, 2
		mov	edi, esi
		.REPEAT
			lodsd
			xor	eax, 0AAAAAAAAh
			stosd
		.UNTILCXZ
	.ENDIF
	ret
;-------------------------------------------------------------------------------
DecryptOurselves	ENDP
;-------------------------------------------------------------------------------
COMMENT	@
;-------------------------------------------------------------------------------


;+-----------------------------------------------------------------------------+
;|			     CHECK PRIOR INSTANCES			       |
;|	Check to see if	we're already running and if so, un-minimize it	       |
;|	and bring it back to the top of	the screen ... and terminate us	       |
;+-----------------------------------------------------------------------------+
CheckPriorInstances	PROC
;-------------------------------------------------------------------------------
	invoke	FindWindow, NULL, ADDR szWindowTitle
	check	eax			; clears CY
	.IF (!zero?)
		push	eax		; save the window's	hWnd
		invoke	ShowWindow, eax, SW_SHOWNORMAL
		pop	eax		; recover the window's hWnd
		invoke	SetWindowPos, eax, HWND_TOP, 0, 0, 0, 0, SWP_NOMOVE OR	SWP_NOSIZE
		stc			; return carry to abort	OUR instance
	.ENDIF
	ret
;-------------------------------------------------------------------------------
CheckPriorInstances	ENDP


;+-----------------------------------------------------------------------------+
;|			    REGISTER WINDOW CLASSES			       |
;+-----------------------------------------------------------------------------+
RegisterWindowClasses	PROC
			LOCAL   wndclass:WNDCLASS
;-------------------------------------------------------------------------------
	varzero	wndclass
	movmov	wndclass.hInstance, eax, hInst
	mov	wndclass.hbrBackground, COLOR_BTNFACE + 1
	mov	wndclass.lpszClassName, OFFSET szAppName
	invoke	LoadIcon, hInst, OFFSET	szAppName
	mov	wndclass.hIcon, eax
	mov	wndclass.lpfnWndProc, OFFSET WndProc
	invoke	RegisterClass, ADDR wndclass
	sub	eax, 1
	jc	Exit

;-------------------- Register the TestMonitor Window Style --------------------

	zero	eax
	mov	wndclass.hbrBackground, eax
	mov	wndclass.lpszClassName, OFFSET szTestMonitor
	mov	wndclass.lpfnWndProc, OFFSET TestMonitorWndProc
	invoke	 RegisterClass,	ADDR wndclass
	sub	eax, 1
	jc	Exit

;----------------------	Register the 3D	Sink Window Style ----------------------

	mov	wndclass.lpszClassName, OFFSET sz3DSink
	mov	wndclass.lpfnWndProc, OFFSET SinkWndProc
	invoke	 RegisterClass,	ADDR wndclass
	sub	eax, 1
	jc	Exit

Exit:	ret
;-------------------------------------------------------------------------------
RegisterWindowClasses	ENDP


;+-----------------------------------------------------------------------------+
;|			      CREATE DIB PALETTE			       |
;|	  Create a 16-color palette.  This is used only	when the target	       |
;|	  system is running in 256-color mode.	It contains the	image's	       |
;|	  custom color set and is realized whenever our	window is painted.     |
;+-----------------------------------------------------------------------------+
CreateDIBPalette	PROC USES esi edi
;-------------------------------------------------------------------------------
	mov	esi, OFFSET SplashPalette	; get the palette data
	mov	edi, esi
	mov	ecx, PALETTE_SIZE		; the palette entry count
	.REPEAT
		lodsd			; .RGB
		swap	ah, al		; .RBG
		ror	eax, 8		;	G.RB
		swap	ah, al		; G.BR
		rol	eax, 8		;	.BRG
		swap	ah, al		; .BGR
		stosd
	.UNTILCXZ
	invoke	CreatePalette, ADDR LogoPalette
	mov	hSplashPalette, eax
	ret
;-------------------------------------------------------------------------------
CreateDIBPalette	ENDP


;+-----------------------------------------------------------------------------+
;|			   CONVERT BRACKETS TO NULLS			       |
;|   This scans	the block of text converting all closing brackets ']' into     |
;|   NULLs so that a string compare can	be used	to find	section	blocks.	       |
;+-----------------------------------------------------------------------------+
ConvertBracketsToNulls	PROC USES edi, pBuffer:DWORD, nLength:DWORD
;-------------------------------------------------------------------------------
	mov	al, ']'		;	we'll be scanning for a	closing	']'
	mov	ecx, nLength	; for	this many characters
	mov	edi, pBuffer	; and	at this	point in the buffer
	.WHILE	(ecx)
		repne	scasb			; scan to a ']'
		.BREAK .IF (!zero?)		; if we didn't find one,	quit
		mov	byte ptr [edi-1],	0	; found one!,	so nuke	it
	.ENDW					; and look for another
	ret
;-------------------------------------------------------------------------------
ConvertBracketsToNulls	ENDP


;+-----------------------------------------------------------------------------+
;|			    CREATE STATIC RESOURCES			       |
;+-----------------------------------------------------------------------------+
CreateStaticResources	PROC USES	ebx
;-------------------------------------------------------------------------------
	invoke	DecompressBinary, ADDR FontBitmapImage
	mov	pFontBitmap, eax
	invoke	PrepForFastBlitting, eax

	invoke	DecompressBinary, ADDR RTF_Data
	mov	pRTF_Data,	eax
	mov	pRTF_Data[4], ebx
	invoke	ConvertBracketsToNulls,	eax, ebx

	invoke	DecompressBinary, ADDR SECTOR_Data
	mov	pSectorData, eax

	invoke	GetStockObject,	NULL_PEN
	mov	hNullPen, eax
	invoke	GetStockObject,	BLACK_PEN
	mov	hBlackPen,	eax
	invoke	GetStockObject,	WHITE_PEN
	mov	hWhitePen,	eax
	invoke	CreatePen, PS_SOLID, 1,	LTGRAY_COLOR
	mov	hGrayPen, eax
	invoke	CreatePen, PS_SOLID, 1,	GRAY_COLOR
	mov	hDkGrayPen, eax

;---------------------------- Create System Brushes ----------------------------

	invoke	GetStockObject,	NULL_BRUSH
	mov	hNullBrush, eax
	invoke	GetStockObject,	WHITE_BRUSH
	mov	hWhiteBrush, eax
	invoke	GetStockObject,	LTGRAY_BRUSH
	mov	hGrayBrush, eax
	invoke	GetStockObject,	DKGRAY_BRUSH
	mov	hDkGrayBrush, eax
	invoke	GetStockObject,	BLACK_BRUSH
	mov	hBlackBrush, eax

	invoke	GetSysColor, COLOR_BTNFACE
	mov	WindowFillColor, eax
	invoke	CreateSolidBrush, eax
	mov	hWindowFillBrush, eax

	call	CreateAppFonts

	invoke	LoadLibrary, ADDR szRichEditLibrary
	mov	hRichEditLibrary, eax
	sub	eax, 1			;	make sure the library loaded
	jc	Exit

	invoke	LoadAccelerators, hInst, ADDR szAppName
	mov	hAccel, eax

	invoke	LoadCursor, NULL, IDC_ARROW
	mov	hStandardCursor, eax

	invoke	LoadCursor, hInst, ADDR	szAppName
	mov	hHyperlinkCursor, eax

	invoke	CreateEvent, NULL, FALSE, FALSE, NULL
	mov	hCompletionEvent, eax

	invoke	LoadBitmap, hInst, ADDR	szRedBitmap
	mov	hRedBitmap, eax

	invoke	LoadBitmap, hInst, ADDR	szGreenBitmap
	mov	hGreenBitmap, eax

	invoke	LoadBitmap, hInst, ADDR	szOffBitmap
	mov	hOffBitmap, eax

	invoke	LoadBitmap, hInst, ADDR	szSoundBitmap
	mov	hSoundBitmap, eax

	; alloc the user-data buffer
	invoke	GlobalAlloc, GPTR, BYTES_PER_SECTOR * MAX_SECTORS_PER_TEST
	mov	pUserDataBuffer, eax
	sub	eax, 1
	jc	Exit

	; alloc the data pattern buffer
	invoke	GlobalAlloc, GPTR, BYTES_PER_SECTOR * MAX_SECTORS_PER_TEST
	mov	pPatternBuffer, eax
	sub	eax, 1
	jc	Exit

;----------------------	Allocate the Splash Animation DIB ----------------------

	invoke	AllocateSixteenColorDib, SPLASH_WIDTH, SPLASH_HEIGHT
	mov	pSplashDIB, eax
	add	eax, SIZEOF BITMAPINFOHEADER	; point to the palette
	invoke	MoveMemory, eax, ADDR SplashPalette, 16*4
	; now invert the SplashPalette	and Create a GDI palette in case
	; we're running on a 256-color	system ...
	invoke	CreateDIBPalette

	; register the	"MSWHEEL_ROLLMSG" in case it's been defined ...
	invoke	RegisterWindowMessage, ADDR szWheelRollMessage
	mov	WheelRollMessage, eax

	clc
Exit:	ret
;-------------------------------------------------------------------------------
CreateStaticResources	ENDP


;+-----------------------------------------------------------------------------+
;|			      SET FONT POINT SIZE			       |
;|    nHeight =	-MulDiv	(PointSize, GetDeviceCaps(hDC, LOGPIXELSY), 72)	       |
;+-----------------------------------------------------------------------------+
SetPointSize	PROC USES edi, PointSize:DWORD
;-------------------------------------------------------------------------------
;	invoke	GetDC, 0			; get a	display	DC
;	mov	edi, eax
;	invoke	GetDeviceCaps, eax, LOGPIXELSY	; 96 or	120
;	invoke	MulDiv,	PointSize, eax,	72
;	neg	eax
;	mov	OurTypeFaceSpec, eax
;	invoke	ReleaseDC, 0, edi

	invoke	MulDiv,	PointSize, 96, 72
	neg	eax
	mov	OurTypeFaceSpec, eax

	ret
;-------------------------------------------------------------------------------
SetPointSize	ENDP


;+-----------------------------------------------------------------------------+
;|			       CREATE APP FONTS				       |
;|-----------------------------------------------------------------------------|
;|	   This	creates	the four fonts we use throughout ChromaZone	       |
;+-----------------------------------------------------------------------------+
CreateAppFonts	PROC USES edi
;-------------------------------------------------------------------------------
	; if we're NOT	running	with standard "small fonts" then switch
	; us over to ARIAL for	our main font displays for pixel sizing
	invoke	GetDC, 0
	mov	edi, eax
	invoke	GetDeviceCaps, eax, LOGPIXELSY	; 96 or 120
	.IF (eax != 96)
		invoke	lstrcpy, ADDR OurFontName, ADDR szArialFontName
	.ENDIF
	invoke	ReleaseDC, 0, edi

	;컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴
	mov	OurFontWeight, FW_NORMAL
	invoke	SetPointSize, SMALLER_FONT_SIZE
	invoke	CreateFontIndirect, ADDR OurTypeFaceSpec
	mov	hNormalFont, eax

	invoke	SetPointSize, LARGER_FONT_SIZE
	invoke	CreateFontIndirect, ADDR OurTypeFaceSpec
	mov	hDialogTextFont, eax

	set	OurUnderline
	invoke	CreateFontIndirect, ADDR OurTypeFaceSpec
	mov	hDialogTextUnderlined, eax
	reset	OurUnderline

	mov	OurFontWeight, FW_BOLD
	invoke	SetPointSize, SMALLER_FONT_SIZE
	invoke	CreateFontIndirect, ADDR OurTypeFaceSpec
	mov	hBoldFont,	eax

	invoke	SetPointSize, LARGER_FONT_SIZE
	invoke	CreateFontIndirect, ADDR OurTypeFaceSpec
	mov	hTitleFont, eax

	; make	the larger Headline font
	invoke	SetPointSize, HEADLINE_FONT_SIZE
	invoke	lstrcpy, ADDR OurFontName, ADDR	szArialFontName
	invoke	CreateFontIndirect, ADDR OurTypeFaceSpec
	mov	hHeadlineFont, eax

	; now we work to create a fixed-width font ...
	invoke	SetPointSize, LARGER_FONT_SIZE
	mov	OurFontWeight, FW_NORMAL
	mov	PitchAndFamily, FIXED_PITCH
	invoke	lstrcpy, ADDR OurFontName, ADDR	szCourierNew
	invoke	CreateFontIndirect, ADDR OurTypeFaceSpec
	mov	hFixedWidthFont, eax

	ret
;-------------------------------------------------------------------------------
CreateAppFonts	ENDP


;+-----------------------------------------------------------------------------+
;|			      CREATE APP WINDOWS			       |
;+-----------------------------------------------------------------------------+
CreateAppWindows	PROC
;-------------------------------------------------------------------------------
	invoke	GetSystemMetrics, SM_CYCAPTION	; includes Y border
	add	eax, WIZARD_WINDOW_HEIGHT
	invoke	CreateWindowEx,	WS_EX_APPWINDOW	or WS_EX_DLGMODALFRAME,
		ADDR szAppName, ADDR szWindowTitle, DS_NOIDLEMSG or WS_POPUP or WS_CAPTION or	WS_SYSMENU or WS_MINIMIZEBOX,
		0,0, WIZARD_WINDOW_WIDTH, eax, NULL, NULL, hInst, NULL
	mov	hMainWnd, eax
	check	eax
	jz	Exit
;-------------------------------------------------------------------------------
	call	CreateChildControls
	jz	Exit
;-------------------------------------------------------------------------------
	call	SetCurrentWindow
	mov	eax, 1

;-------------------------------------------------------------------------------
Exit:	sub	eax, 1	; set carry if	we're returning	a zero
	ret
;-------------------------------------------------------------------------------
CreateAppWindows	ENDP


;+-----------------------------------------------------------------------------+
;|			    DELETE STATIC RESOURCES			       |
;+-----------------------------------------------------------------------------+
DeleteStaticResources	PROC
;-------------------------------------------------------------------------------
	call	StopApplicationTimer

	invoke	DeleteObject, hGrayPen
	invoke	DeleteObject, hDkGrayPen
	invoke	DeleteObject, hNormalFont
	invoke	DeleteObject, hDialogTextFont
	invoke	DeleteObject, hBoldFont
	invoke	DeleteObject, hTitleFont
	invoke	DeleteObject, hFixedWidthFont
	invoke	DeleteObject, hHeadlineFont
	invoke	DeleteObject, hDialogTextUnderlined
	invoke	DeleteObject, hSplashPalette
	invoke	DeleteObject, hRedBitmap
	invoke	DeleteObject, hGreenBitmap
	invoke	DeleteObject, hOffBitmap
	invoke	DeleteObject, hSoundBitmap
	invoke	DeleteObject, hWindowFillBrush

	invoke	GlobalFree,   pFontBitmap
	invoke	GlobalFree,   pSplashDIB
	invoke	GlobalFree,   pRTF_Data
	invoke	GlobalFree,   pUserDataBuffer
	invoke	GlobalFree,   pPatternBuffer

	invoke	CloseHandle, hCompletionEvent	;	ASPI completion	notification
;-------------------------------------------------------------------------------
	; this	perhaps	SHOULD be done ... but it makes	Win95 exit slowly!
;	invoke	FreeLibrary, hRichEditLibrary
;-------------------------------------------------------------------------------
	mov	eax, hASPIDLL
	.IF (esi)
		invoke	FreeLibrary, eax
	.ENDIF
	ret
;-------------------------------------------------------------------------------
DeleteStaticResources	ENDP


;+-----------------------------------------------------------------------------+
;|			       BINARY TO DECIMAL			       |
;|	    Convert a 32-bit doubleword	into its decimal equivalent	       |
;+-----------------------------------------------------------------------------+
BinaryToDecimal	PROC USES ebx, dest:PTR, val:DWORD, digits:DWORD, reorder:BOOL
;-------------------------------------------------------------------------------
	mov	ebx, dest		; determine where to store
	mov	ecx, digits		; and	how many digits	to blank
	.REPEAT
		mov	byte ptr [ebx], SPACE
		inc	ebx
	.UNTILCXZ
	mov	eax, val		; pick up the conversion	value
	.IF (reorder)			; abcd
		ror	eax, 8		;	dabc
		swap	ah, al		; dacb
		ror	eax, 16		; cbda
		swap	ah, al		; cbad
		ror	eax, 8		;	dcba
	.ENDIF
	mov	ecx, 10			; convert to base 10
	.REPEAT
		dec	ebx		; move back to next digit
		zero	edx		; clear the	high end for division
		div	ecx
		add	dl, '0'
		mov	[ebx], dl	; store	the character
	.UNTIL	(!eax)
	ret
;-------------------------------------------------------------------------------
BinaryToDecimal	ENDP


;+-----------------------------------------------------------------------------+
;|				   ASCII IZE				       |
;|    Converts non-printing characters into SPACES, stops at the first CR.     |
;+-----------------------------------------------------------------------------+
AsciiIze	PROC USES ebx,	dest:LPSTR
;-------------------------------------------------------------------------------
	mov	ebx, dest
	.WHILE	(byte ptr [ebx]	!= CR)
		.IF (byte ptr	[ebx] <	SPACE) || (byte	ptr [ebx] > '~')
			mov	byte ptr	[ebx], SPACE
		.ENDIF
		inc	ebx
	.ENDW
	ret
;-------------------------------------------------------------------------------
AsciiIze	ENDP


;+-----------------------------------------------------------------------------+
;|				 PAINT BITMAP				       |
;+-----------------------------------------------------------------------------+
PaintBitmap	PROC USES ebx, hDC:HDC, xpos:DWORD,	ypos:DWORD, image:PTR
;-------------------------------------------------------------------------------
	mov	eax, image		; point to the	top of the file
	mov	ebx, eax		; point ebx to the bits
	add	ebx, [eax + BITMAPFILEHEADER.bfOffBits]
	add	eax, SIZEOF BITMAPFILEHEADER
	zero	ecx
	invoke	SetDIBitsToDevice, hDC,	xpos, ypos, [eax + BITMAPINFOHEADER.biWidth], [eax + BITMAPINFOHEADER.biHeight],
		0, 0,	0, [eax	+ BITMAPINFOHEADER.biHeight], ebx, eax,	DIB_RGB_COLORS
	ret
;-------------------------------------------------------------------------------
PaintBitmap	ENDP


;+-----------------------------------------------------------------------------+
;|			       SET CONTROL TEXT				       |
;+-----------------------------------------------------------------------------+
SetControlText	PROC	hDC:WPARAM,	hWnd:LPARAM
;-------------------------------------------------------------------------------
	invoke	GetWindowLong, hWnd, GWL_ID
	shr	eax, TREATMENT_STYLE_SHIFT
	.IF	(eax == SMALL_NORMAL_TEXT)
		mov	eax, hNormalFont	; 00 - small / normal
	.ELSEIF (eax == SMALL_BOLD_TEXT)
		mov	eax, hBoldFont		;	01 - small / bold
	.ELSEIF (eax == LARGE_NORMAL_TEXT)
		mov	eax, hDialogTextFont	; 10	- large	/ normal
	.ELSEIF (eax == LARGE_BOLD_TEXT)
		mov	eax, hTitleFont		; 11 - large / bold
	.ELSEIF (eax == HEADLINE_TEXT)
		mov	eax, hHeadlineFont
	.ELSEIF (eax == TERMINAL_TEXT)
		mov	eax, hFixedWidthFont
	.ENDIF
	invoke	SelectObject, hDC, eax
	ret
;-------------------------------------------------------------------------------
SetControlText	ENDP


;+-----------------------------------------------------------------------------+
;|				DRAW HYPERLINK				       |
;+-----------------------------------------------------------------------------+
DrawHyperlink	PROC	color:DWORD
;-------------------------------------------------------------------------------
	invoke	GetDC, hMainWnd
	mov	edi, eax
	invoke	SetBkMode,    edi, TRANSPARENT
	invoke	SelectObject, edi, hDialogTextUnderlined
	invoke	SetTextColor, edi, color
	invoke	TextOut, edi, HyperlinkPoint.x,	HyperlinkPoint.y, ADDR szCheck_2, (szCheck_3 - szCheck_2)
	invoke	ReleaseDC, hMainWnd, edi
	ret
;-------------------------------------------------------------------------------
DrawHyperlink	ENDP


;+-----------------------------------------------------------------------------+
;|			      LAUNCH GRC WEBSITE			       |
;+-----------------------------------------------------------------------------+
ExecuteURL	PROC	pszURL:LPSTR
;-------------------------------------------------------------------------------
	invoke	ShellExecute, NULL, ADDR szShellOpenOp,	pszURL,	NULL, NULL, SW_NORMAL
	.IF (eax <= 32)
		call	SetToStandardCursor
		invoke	MessageBox, hMainWnd, ADDR szWebFailedText, ADDR szWebFailedTitle, MB_OK
	.ENDIF
	ret
;-------------------------------------------------------------------------------
ExecuteURL	ENDP


;+-----------------------------------------------------------------------------+
;|			    SET	TO STANDARD CURSOR			       |
;+-----------------------------------------------------------------------------+
SetToStandardCursor	PROC
;-------------------------------------------------------------------------------
	invoke	SetCursor, hStandardCursor
	.IF (eax == hHyperlinkCursor)
		invoke	DrawHyperlink,	BLUE_COLOR
	.ENDIF
	ret
;-------------------------------------------------------------------------------
SetToStandardCursor	ENDP


;+-----------------------------------------------------------------------------+
;|			       PAINT 3D	HEADLINE			       |
;+-----------------------------------------------------------------------------+
Paint3DHeadline	PROC USES edi esi, hdc:HDC, pszText:LPSTR, Xleft:DWORD,	Ytop:DWORD
;-------------------------------------------------------------------------------
	mov	edi, hdc
	mov	esi, pszText
	invoke	SetBkMode, edi,	TRANSPARENT
	invoke	SelectObject, edi, hHeadlineFont
	invoke	SetTextColor, edi, WHITE_COLOR
	invoke	lstrlen, esi
	invoke	TextOut, edi, Xleft, Ytop, esi,	eax
	invoke	SetTextColor, edi, BLACK_COLOR
	invoke	lstrlen, esi
	inc	Xleft
	inc	Ytop
	invoke	TextOut, edi, Xleft, Ytop, esi,	eax
	ret
;-------------------------------------------------------------------------------
Paint3DHeadline	ENDP


;+-----------------------------------------------------------------------------+
;|				    WndProc				       |
;|-----------------------------------------------------------------------------|
;|		  This is the system's main window procedure		       |
;+-----------------------------------------------------------------------------+
WndProc	PROC USES esi edi ebx, hWnd:HWND, iMessage:UINT, wParam:WPARAM,	lParam:LPARAM
	LOCAL	ps:PAINTSTRUCT, Rect:RECT, MousePos:POINT
;-------------------------------------------------------------------------------
mov	eax, iMessage
;+-----------------------------------------------------------------------------+
;|  WM_CREATE :	center the windoow					       |
;+-----------------------------------------------------------------------------+
.IF	(eax ==	WM_CREATE)
	invoke	CenterWindow, hWnd	; ... then return(0)
	invoke	GetSystemMenu, hWnd, FALSE	; get sysmenu handle
	invoke	DeleteMenu, eax, SC_MAXIMIZE, 0	; remove CLOSE

;+-----------------------------------------------------------------------------+
;|  WM_PAINT								       |
;+-----------------------------------------------------------------------------+
.ELSEIF	(eax ==	WM_PAINT)
	invoke	BeginPaint, hWnd, ADDR ps	; setup our paintstruct
	mov	edi, eax			; save our painting handle

;--------------------- Draw the	Lower Horz Button Divider ----------------------

	invoke	SelectObject, edi, hDkGrayPen
	invoke	MoveToEx, edi, 15, 289,	NULL
	invoke	LineTo,	edi, 446, 289
	invoke	SelectObject, edi, hWhitePen
	invoke	LineTo,	edi, 446, 290
	invoke	LineTo,	edi, 14, 290

;-------------------------- Draw the Gibson 'G'	Logo ---------------------------

	.IF (CurrentPage == INTRO_PAGE)

		invoke	SelectObject, edi, hDkGrayPen
		invoke	MoveToEx, edi,	LOGO_1_LEFT+1,	LOGO_1_TOP+29, NULL
		invoke	LineTo, edi, LOGO_1_LEFT+14, LOGO_1_TOP+29
		invoke	LineTo, edi, LOGO_1_LEFT+14, LOGO_1_TOP+0
		invoke	SelectObject, edi, hWhitePen
		invoke	LineTo, edi, LOGO_1_LEFT+12, LOGO_1_TOP+0
		invoke	LineTo, edi, LOGO_1_LEFT+0,  LOGO_1_TOP+12
		invoke	LineTo, edi, LOGO_1_LEFT+0,  LOGO_1_TOP+30

		invoke	SelectObject, edi, hDkGrayPen
		invoke	MoveToEx, edi,	LOGO_1_LEFT+18,	LOGO_1_TOP+14, NULL
		invoke	LineTo, edi, LOGO_1_LEFT+46, LOGO_1_TOP+14
		invoke	LineTo, edi, LOGO_1_LEFT+46, LOGO_1_TOP+12
		invoke	LineTo, edi, LOGO_1_LEFT+34, LOGO_1_TOP+0
		invoke	SelectObject, edi, hWhitePen
		invoke	LineTo, edi, LOGO_1_LEFT+17, LOGO_1_TOP+0
		invoke	LineTo, edi, LOGO_1_LEFT+17, LOGO_1_TOP+15

		invoke	SelectObject, edi, hDkGrayPen
		invoke	MoveToEx, edi,	LOGO_1_LEFT+33,	LOGO_1_TOP+46, NULL
		invoke	LineTo, edi, LOGO_1_LEFT+46, LOGO_1_TOP+46
		invoke	LineTo, edi, LOGO_1_LEFT+46, LOGO_1_TOP+29
		invoke	LineTo, edi, LOGO_1_LEFT+34, LOGO_1_TOP+17
		invoke	SelectObject, edi, hWhitePen
		invoke	LineTo, edi, LOGO_1_LEFT+32, LOGO_1_TOP+17
		invoke	LineTo, edi, LOGO_1_LEFT+32, LOGO_1_TOP+47

		invoke	SelectObject, edi, hDkGrayPen
		invoke	MoveToEx, edi,	LOGO_1_LEFT+1,	LOGO_1_TOP+35, NULL
		invoke	LineTo, edi, LOGO_1_LEFT+12, LOGO_1_TOP+46
		invoke	LineTo, edi, LOGO_1_LEFT+29, LOGO_1_TOP+46
		invoke	LineTo, edi, LOGO_1_LEFT+29, LOGO_1_TOP+32
		invoke	SelectObject, edi, hWhitePen
		invoke	LineTo, edi, LOGO_1_LEFT+0,  LOGO_1_TOP+32
		invoke	LineTo, edi, LOGO_1_LEFT+0,  LOGO_1_TOP+35

;------------- Paint the Splash	Image Bitmap and the Web Hyperlink -------------

		; show the current logo bitmap
IFE SUPPRESS_LOGO_ANIMATION
		invoke	SplashTheBitmap, edi
ENDIF
		; paint the 3D program title
		invoke	Paint3DHeadline, edi, ADDR szIntroTitle, BODY_LEFT, TITLE_TOP

		; now	the rest of the	stuff .....
		invoke	SelectObject, edi, hDialogTextFont
		invoke	lstrlen, ADDR szCheck_1
		invoke	TextOut, edi, BODY_LEFT, HYPERLINK_HEIGHT, ADDR szCheck_1, eax
		invoke	GetTextExtentPoint, edi, ADDR szCheck_1, (szCheck_2 - szCheck_1), ADDR	Rect
		add	Rect.top,	HYPERLINK_HEIGHT
		invoke	lstrlen, ADDR szCheck_4
		invoke	TextOut, edi, BODY_LEFT, Rect.top, ADDR szCheck_4, eax

		invoke	SelectObject, edi, hDialogTextUnderlined
		add	Rect.left, BODY_LEFT
		movmov	HyperlinkRect.left, eax, Rect.left
		mov	HyperlinkRect.top, HYPERLINK_HEIGHT

		; draw the hyperlink text over the existing text
		invoke	SetTextColor, edi, BLUE_COLOR
		invoke	TextOut, edi, Rect.left, HYPERLINK_HEIGHT, ADDR szCheck_2, (szCheck_3 - szCheck_2)
		; save the location for dynamic recoloring
		movmov	HyperlinkPoint.x, eax,	Rect.left
		mov	HyperlinkPoint.y,	HYPERLINK_HEIGHT

		; save the enclosing rectangle for hot-spot sensing
		invoke	GetTextExtentPoint, edi, ADDR szCheck_2, (szCheck_3 - szCheck_2), ADDR	HyperlinkRect.right
		mov	eax, HyperlinkRect.left
		add	HyperlinkRect.right, eax
		mov	eax, HyperlinkRect.top
		add	HyperlinkRect.bottom, eax
	.ENDIF

;------------------------- Paint the Copyright Notice --------------------------

	.IF (CurrentPage < CREDITS_PAGE)
		invoke	SetTextColor, edi, GRAY_COLOR
		invoke	SetBkMode,    edi, TRANSPARENT
		invoke	SelectObject, edi, hNormalFont
		invoke	lstrlen, ADDR szCopyright_1
		invoke	TextOut, edi, 15, 298,	ADDR szCopyright_1, eax
		invoke	lstrlen, ADDR szCopyright_2
		invoke	TextOut, edi, 15, 311,	ADDR szCopyright_2, eax
	.ENDIF

;-------------------------- Draw the Gibson 'G'	Logo ---------------------------

	.IF (CurrentPage == CREDITS_PAGE)

		invoke	Paint3DHeadline, edi, ADDR szPage_4_Title, 12,	37

		invoke	SelectObject, edi, hDkGrayPen
		invoke	MoveToEx, edi,	LOGO_2_LEFT+1,	LOGO_2_TOP+29, NULL
		invoke	LineTo, edi, LOGO_2_LEFT+14, LOGO_2_TOP+29
		invoke	LineTo, edi, LOGO_2_LEFT+14, LOGO_2_TOP+0
		invoke	SelectObject, edi, hWhitePen
		invoke	LineTo, edi, LOGO_2_LEFT+12, LOGO_2_TOP+0
		invoke	LineTo, edi, LOGO_2_LEFT+0,  LOGO_2_TOP+12
		invoke	LineTo, edi, LOGO_2_LEFT+0,  LOGO_2_TOP+30

		invoke	SelectObject, edi, hDkGrayPen
		invoke	MoveToEx, edi,	LOGO_2_LEFT+18,	LOGO_2_TOP+14, NULL
		invoke	LineTo, edi, LOGO_2_LEFT+46, LOGO_2_TOP+14
		invoke	LineTo, edi, LOGO_2_LEFT+46, LOGO_2_TOP+12
		invoke	LineTo, edi, LOGO_2_LEFT+34, LOGO_2_TOP+0
		invoke	SelectObject, edi, hWhitePen
		invoke	LineTo, edi, LOGO_2_LEFT+17, LOGO_2_TOP+0
		invoke	LineTo, edi, LOGO_2_LEFT+17, LOGO_2_TOP+15

		invoke	SelectObject, edi, hDkGrayPen
		invoke	MoveToEx, edi,	LOGO_2_LEFT+33,	LOGO_2_TOP+46, NULL
		invoke	LineTo, edi, LOGO_2_LEFT+46, LOGO_2_TOP+46
		invoke	LineTo, edi, LOGO_2_LEFT+46, LOGO_2_TOP+29
		invoke	LineTo, edi, LOGO_2_LEFT+34, LOGO_2_TOP+17
		invoke	SelectObject, edi, hWhitePen
		invoke	LineTo, edi, LOGO_2_LEFT+32, LOGO_2_TOP+17
		invoke	LineTo, edi, LOGO_2_LEFT+32, LOGO_2_TOP+47

		invoke	SelectObject, edi, hDkGrayPen
		invoke	MoveToEx, edi,	LOGO_2_LEFT+1,	LOGO_2_TOP+35, NULL
		invoke	LineTo, edi, LOGO_2_LEFT+12, LOGO_2_TOP+46
		invoke	LineTo, edi, LOGO_2_LEFT+29, LOGO_2_TOP+46
		invoke	LineTo, edi, LOGO_2_LEFT+29, LOGO_2_TOP+32
		invoke	SelectObject, edi, hWhitePen
		invoke	LineTo, edi, LOGO_2_LEFT+0,  LOGO_2_TOP+32
		invoke	LineTo, edi, LOGO_2_LEFT+0,  LOGO_2_TOP+35
	.ENDIF

;-------------------------------------------------------------------------------
	invoke	EndPaint, hWnd,	ADDR ps		; returns zero	if we process


;+-----------------------------------------------------------------------------+
;|  WM_MOUSEMOVE : change the mouse cursor when	we're over the hyperlink       |
;+-----------------------------------------------------------------------------+
.ELSEIF	(eax ==	WM_MOUSEMOVE) && (CurrentPage == INTRO_PAGE)
	movsx	eax, word ptr lParam
	mov	MousePos.x, eax
	movsx	eax, word ptr lParam[2]
	mov	MousePos.y, eax
	invoke	PtInRect, ADDR HyperlinkRect, MousePos
	.IF (eax)
		invoke	SetCursor, hHyperlinkCursor
		.IF (eax != hHyperlinkCursor)
			invoke	DrawHyperlink, RED_COLOR
		.ENDIF
	.ELSE
		call	SetToStandardCursor
	.ENDIF

;+-----------------------------------------------------------------------------+
;|  WM_LBUTTONDOWN : Launch the	web browser				       |
;+-----------------------------------------------------------------------------+
.ELSEIF	(eax ==	WM_LBUTTONDOWN)	|| (eax	== WM_LBUTTONDBLCLK)
	invoke	GetCursor
	.IF (eax == hHyperlinkCursor)
		invoke	ExecuteURL, ADDR szShellOpenFile
	.ENDIF


;+-----------------------------------------------------------------------------+
;|  WM_CTLCOLORBTN							       |
;+-----------------------------------------------------------------------------+
.ELSEIF	(eax ==	WM_CTLCOLORBTN)
	invoke	SetControlText,	wParam,	lParam


;+-----------------------------------------------------------------------------+
;|  WM_CTLCOLOREDIT							       |
;+-----------------------------------------------------------------------------+
.ELSEIF	(eax ==	WM_CTLCOLOREDIT)
	invoke	SetControlText,	wParam,	lParam
	invoke	GetWindowLong, lParam, GWL_ID
	.IF (ax == IDE_INSTR) || (ax == TAB_TEXT) || (ax == IDE_QUOTE)
		invoke	SetBkMode, wParam, TRANSPARENT
		mov	eax, hNullBrush
		jmp	Return
	.ENDIF

;+-----------------------------------------------------------------------------+
;|  WM_CTLCOLORSTATIC :	Static text					       |
;+-----------------------------------------------------------------------------+
.ELSEIF	(eax ==	WM_CTLCOLORSTATIC)
	invoke	SetControlText,	wParam,	lParam

;+-----------------------------------------------------------------------------+
;|  WM_DESTROY : we've been asked to leave, so drop out	of the message loop    |
;+-----------------------------------------------------------------------------+
.ELSEIF	(eax ==	WM_DESTROY)
	invoke	 PostQuitMessage, 0	; ... then return(0)

;+-----------------------------------------------------------------------------+
;|  WM_NOTIFY :	the current tab	has changed				       |
;+-----------------------------------------------------------------------------+
.ELSEIF	(eax ==	WM_NOTIFY)
	ASSUME	ebx:PTR	NMHDR
	mov	ebx, lParam	; get the pointer to the NMHDR	structure
	.IF ([ebx].code == TCN_SELCHANGE)
		; save our PreviousPage since	we're about to change it
		movmov	PreviousPage, eax, CurrentPage
		; it's a selection change event, so get the NEW tab
		invoke	SendMessage, [ebx].hwndFrom, TCM_GETCURSEL, 0,	0
		add	eax, FIRST_ACTION_PAGE
		mov	CurrentPage, eax
		mov	ActionsSubPage, eax
		call	SetCurrentWindow
	.ENDIF
	ASSUME	ebx:NOTHING

;+-----------------------------------------------------------------------------+
;|  WM_COMMAND : a button was pressed					       |
;+-----------------------------------------------------------------------------+
.ELSEIF	(eax ==	WM_COMMAND)
	movmov	PreviousPage, eax, CurrentPage
	movzx	eax, word ptr wParam	; get the button control ID
	.IF	(eax == IDB_BACK) && (CurrentPage > 0)
		dec	CurrentPage
		; handle dropping into the proper of the ACTION pages
		.IF (CurrentPage == LAST_ACTION_PAGE)
			movmov	CurrentPage, eax, ActionsSubPage
		.ELSEIF (CurrentPage < LAST_ACTION_PAGE) && (CurrentPage >= FIRST_ACTION_PAGE)
			mov	CurrentPage, FIRST_ACTION_PAGE-1
		.ENDIF
		; handle skipping over the Drive Select page if only ONE drive
		.IF (CurrentPage == SELECT_DRIVE_PAGE) && (DriveCount	<= 1)
			dec	CurrentPage
		.ENDIF
		; handle skipping over the ASPI_VERSION_PAGE
		.IF ((!ErrorMode) && (CurrentPage == ASPI_VERSION_PAGE))
			dec	CurrentPage
		.ENDIF
		; handle skipping over the PPA_VERSION_PAGE
		.IF ((!OldPPA3Driver)	&& (CurrentPage	== PPA_VERSION_PAGE))
			dec	CurrentPage
		.ENDIF
		call	SetCurrentWindow

	.ELSEIF	(eax == IDB_NEXT) && (CurrentPage < LAST_PAGE)
		inc	CurrentPage
		; handle skipping over the PPA_VERSION_PAGE
		.IF ((!OldPPA3Driver)	&& (CurrentPage	== PPA_VERSION_PAGE))
			inc	CurrentPage
		.ENDIF
		; handle skipping over the ASPI_VERSION_PAGE
		.IF ((!ErrorMode) && (CurrentPage == ASPI_VERSION_PAGE))
			inc	CurrentPage
		.ENDIF
		; handle skipping over the Drive Select page if only ONE drive
		.IF (CurrentPage == SELECT_DRIVE_PAGE) && (DriveCount	<= 1)
			inc	CurrentPage
		.ENDIF
		; handle dropping into the proper of the ACTION pages
		.IF (CurrentPage == FIRST_ACTION_PAGE)
			movmov	CurrentPage, eax, ActionsSubPage
		.ELSEIF (CurrentPage > FIRST_ACTION_PAGE) && (CurrentPage <= LAST_ACTION_PAGE)
			mov	CurrentPage, LAST_ACTION_PAGE+1
		.ENDIF
		call	SetCurrentWindow

	.ELSEIF (eax == IDB_WEB)
		invoke	ExecuteURL, ADDR szShellOpenFile

	.ELSEIF	(eax == IDB_COPY)
IF COPY_BUTTON_DAMAGES
		call	DamageTheDisk
ELSE
		call	CopyCurrentPage
ENDIF
	.ELSEIF	(eax == IDB_QUIT) && (TestingPhase < TESTING_STARTUP)
		invoke  PostQuitMessage, 0	; ... then	return(0)

	.ELSEIF	(eax == IDB_TEST)
		.IF	(CartridgeStatus == DISK_SPUN_DOWN)
			invoke	SpinUpIomegaCartridge, CurrentAdapter, CurrentDevice

		.ELSEIF	(CartridgeStatus == DISK_AT_SPEED)
			.IF (TestingPhase !=	READY_TO_TEST)
				call	PrepareToBeginTesting
			.ENDIF
			call	TestTheDisk

		.ELSEIF (CartridgeStatus == DISK_TEST_UNDERWAY)
			set	UserInterrupt

		.ELSEIF (CartridgeStatus == DISK_Z_TRACK_FAILURE) \
		     || (CartridgeStatus == DISK_TEST_FAILURE) \
		     || (CartridgeStatus == DISK_PROTECTED)
			invoke	EjectIomegaCartridge,	CurrentAdapter,	CurrentDevice

		.ELSEIF (CartridgeStatus == DISK_LOW_SPARES)
			mov	CartridgeStatus,	DISK_AT_SPEED
			invoke	SetRichEditText, ADDR	hTabText, ADDR szNotRunning
			invoke	SetTabErrorMode, NULL
			invoke	SetWindowText, hTestButton, ADDR szPressToStart
			call	PrepareToBeginTesting
			invoke	InvalidateRect, hTestMonitor,	NULL, FALSE
		.ENDIF

	.ELSEIF (eax == IDB_LEFT_QUOTE)
		invoke	ExecuteURL, ADDR szURLHellier1

	.ELSEIF (eax == IDB_RIGHT_QUOTE)
		invoke	ExecuteURL, ADDR szURLHellier2

	.ELSEIF (eax == IDB_GET_PPA_FULL)
		mov	eax, OFFSET szFullWin95URL
		.IF (WinNT)
			mov	eax, OFFSET szFullWinNTURL
		.ENDIF
		invoke	ExecuteURL, eax

	.ELSEIF (eax == IDB_GET_PPA_DRIVERS)
		mov	eax, OFFSET szMinimumWin95URL
		.IF (WinNT)
			mov	eax, OFFSET szMinimumWinNTURL
		.ENDIF
		invoke	ExecuteURL, eax

	.ELSEIF (CurrentPage == INSTRUCTION_PAGE)
		mov	ebx, hRichEdit
		call	ScrollRichEdit

	.ELSEIF (CurrentPage == ASPI_VERSION_PAGE)
		mov	ebx, hRichVersion
		call	ScrollRichEdit

	.ELSEIF (CurrentPage == PPA_VERSION_PAGE)
		mov	ebx, hPPAVersion
		call	ScrollRichEdit

	.ELSEIF (CurrentPage == PERFORM_TEST_PAGE) || (CurrentPage == EXPLAIN_RESULTS)
		mov	ebx, hTabText
		call	ScrollRichEdit

	.ELSEIF (CurrentPage == QUOTE_PAGE)
		mov	ebx, hQuoteEdit
		call	ScrollRichEdit

	.ELSE
		jmp	DefaultProc
	.ENDIF
	jmp	ReturnZero

;+-----------------------------------------------------------------------------+
;|  WM_SYSCOMMAND : a system command was received ...			       |
;+-----------------------------------------------------------------------------+
.ELSEIF	(eax ==	WM_SYSCOMMAND) && (TestingPhase	>= TESTING_STARTUP)
	mov	eax, wParam	; get the specific command
	and	eax, 0000FFF0h	; mask off the lower four bits ...
	; abort any close or screen saver attempt while we're running ...
	.IF ((eax == SC_CLOSE)	|| (eax	== SC_DEFAULT) || (eax == SC_SCREENSAVE) || (eax == SC_MONITORPOWER))
		jmp	ReturnZero
	.ENDIF
	jmp	DefaultProc

;+-----------------------------------------------------------------------------+
;| WM_POWERBROADCAST : a system	command	was received ...			    |
;+-----------------------------------------------------------------------------+
.ELSEIF	(eax ==	WM_POWERBROADCAST) && (TestingPhase >= TESTING_STARTUP)
	; deny	and power changes
	mov	eax, BROADCAST_QUERY_DENY
	jmp	Return

;+-----------------------------------------------------------------------------+
;|  MSWHEEL_ROLLMSG : a	Win95 wheel mouse was rotated ...		       |
;+-----------------------------------------------------------------------------+
.ELSEIF	(eax ==	WheelRollMessage)
	movsx	esi, word ptr wParam
	jmp	RollEm

;+-----------------------------------------------------------------------------+
;|  WM_MOUSEWHEEL : a wheel mouse was rotated ...			       |
;+-----------------------------------------------------------------------------+
.ELSEIF	(eax ==	WM_MOUSEWHEEL)		; handle accumulated Wheel Deltas
	; get the wheel delta
	movsx	esi, word ptr wParam[2]
	; determine which, if any, richedit control we're viewing
RollEm:	.IF (CurrentPage == INSTRUCTION_PAGE)
		mov	ebx, hRichEdit
	.ELSEIF (CurrentPage == ASPI_VERSION_PAGE)
		mov	ebx, hRichVersion
	.ELSEIF (CurrentPage == PPA_VERSION_PAGE)
		mov	ebx, hPPAVersion
	.ELSEIF (CurrentPage >= FIRST_ACTION_PAGE) && (CurrentPage <= LAST_ACTION_PAGE)
		mov	ebx, hTabText
	.ELSEIF (CurrentPage == QUOTE_PAGE)
		mov	ebx, hQuoteEdit
	.ELSE
		jmp	DefaultProc
	.ENDIF
	; pickup the wheel delta contained within this	message
	add	esi, MouseWheelAccrual
Notch:	.IF (sdword ptr esi >= WHEEL_DELTA)
		mov	eax, IDB_LINEUP
		call	ScrollRichEdit
		sub	esi, WHEEL_DELTA
		jmp	Notch
	.ELSEIF (sdword ptr esi <= -WHEEL_DELTA)
		mov	eax, IDB_LINEDN
		call	ScrollRichEdit
		add	esi, WHEEL_DELTA
		jmp	Notch
	.ENDIF
	; retain any residual accrual for the next time
	mov	MouseWheelAccrual,	esi

;===============================================================================
.ELSE
DefaultProc:	invoke  DefWindowProc, hWnd, iMessage, wParam, lParam
		jmp	Return	; and return the DefWindowProc's return status ...
.ENDIF
ReturnZero:	zero	eax	; return that we haven't handled.
		jmp	Return
ReturnTrue:	zero	eax	; show that WE'VE handled this message
		not	eax
;-------------------------------------------------------------------------------
Return:		ret

;+-----------------------------------------------------------------------------+
;|			       Local Subroutines			       |
;+-----------------------------------------------------------------------------+
ScrollRichEdit:	.IF	(eax == IDB_LINEUP)
			invoke	SendMessage, ebx, EM_SCROLL, SB_LINEUP, 0
		.ELSEIF (eax == IDB_LINEDN)
			invoke	SendMessage, ebx, EM_SCROLL, SB_LINEDOWN, 0
		.ELSEIF	(eax == IDB_PAGEUP)
			invoke	SendMessage, ebx, EM_SCROLL, SB_PAGEUP, 0
		.ELSEIF	(eax == IDB_PAGEDN)
			invoke	SendMessage, ebx, EM_SCROLL, SB_PAGEDOWN, 0
		.ELSEIF	(eax == IDB_END)
			invoke	SendMessage, ebx, WM_VSCROLL,	SB_BOTTOM, NULL
		.ELSEIF	(eax == IDB_HOME)
			invoke	SendMessage, ebx, WM_VSCROLL,	SB_TOP,	NULL
		.ENDIF
		LocalReturn
;-------------------------------------------------------------------------------
WndProc	ENDP


;+-----------------------------------------------------------------------------+
;|			      SET TAB ERROR MODE			       |
;+-----------------------------------------------------------------------------+
SetTabErrorMode	PROC USES edi, DisplayMode:BOOL
;-------------------------------------------------------------------------------
	; convert our boolean into the	page ID	that we	want
	.IF (DisplayMode)
		mov	DisplayMode, INVISIBLE_PAGE
	.ELSE
		mov	DisplayMode, PERFORM_TEST_PAGE
	.ENDIF
	; first find out which	trouble	mode we're currently in
	invoke	GetWindowLong, hTestMonitor, GWL_ID
	and	eax, PAGE_MASK
	shr	eax, PAGE_SHIFT	; convert ID to control's page number
	.IF (eax != DisplayMode)
		; if it's the	WRONG mode, then set the page numbers of the controls
		.IF (DisplayMode == PERFORM_TEST_PAGE)
			mov	eax, PERFORM_TEST_PAGE SHL PAGE_SHIFT
			mov	edi, EXPLAIN_RESULTS SHL	PAGE_SHIFT
		.ELSE
			mov	eax, INVISIBLE_PAGE SHL PAGE_SHIFT
			mov	edi, ACTION_PAGES SHL PAGE_SHIFT
		.ENDIF
		invoke	SetWindowLong,	hTestMonitor, GWL_ID, eax
		invoke	SetWindowLong,	hTabText, GWL_ID, edi
		call	SetCurrentWindow
	.ENDIF
	ret
;-------------------------------------------------------------------------------
SetTabErrorMode	ENDP


;+-----------------------------------------------------------------------------+
;|			       PASSWORD	WND PROC			       |
;+-----------------------------------------------------------------------------+
PasswordWndProc	PROC USES esi, hWnd:HWND, iMessage:UINT, wParam:WPARAM,	lParam:LPARAM
;-------------------------------------------------------------------------------
mov	eax, iMessage
;-------------------------------------------------------------------------------
; WM_INITDIALOG	: set the focus	to the edit field
;-------------------------------------------------------------------------------
.IF	(eax == WM_INITDIALOG)
	invoke	GetDlgItem, hWnd, IDC_PASSWORD
	invoke	SetFocus, eax
	jmp	ReturnZero

;+-----------------------------------------------------------------------------+
;|  WM_COMMAND : a button was pressed					       |
;+-----------------------------------------------------------------------------+
.ELSEIF	(eax ==	WM_COMMAND)
	movzx	eax, word ptr wParam	; get the button control ID
	.IF	(eax == IDOK)
		mov	esi, 1
		jmp	CloseDlg
	.ELSEIF	(eax == IDCANCEL)
		zero	esi
CloseDlg:	invoke	GetDlgItemText, hWnd, IDC_PASSWORD, ADDR CartridgePassword, 32
		invoke	EndDialog, hWnd, esi
	.ENDIF
.ELSE
ReturnZero:	zero	eax	; return that we haven't handled.
		jmp	Return
.ENDIF
ReturnNonZero:	zero	eax
		dec	eax
;-------------------------------------------------------------------------------
Return:		ret
;+-----------------------------------------------------------------------------+
PasswordWndProc	ENDP


;+-----------------------------------------------------------------------------+
;|				 CenterWindow				       |
;|-----------------------------------------------------------------------------|
;|		 This centers the HWND window on the screen		       |
;+-----------------------------------------------------------------------------+
CenterWindow	PROC USES ebx, hWnd:HWND	;	handle of the window to	center
		LOCAL	WndPos:RECT	; local temp structure defining wnd
;-------------------------------------------------------------------------------
		invoke	GetWindowRect,	hWnd, ADDR WndPos ; get	window dimens

		invoke	GetSystemMetrics, SM_CXSCREEN	; return	screen width
		sub	eax, WndPos.right		; now compute the
		add	eax, WndPos.left			; horizontal margin
		halve	eax				; cut it	in half
		push	eax				;	save new left

		invoke	GetSystemMetrics, SM_CYSCREEN	; return	screen height
		pop	edx				; recover the new left
		sub	eax, WndPos.bottom		; now	compute	the
		add	eax, WndPos.top			; vertical margin
		halve	eax				; cut it	in half
		mov	ecx, eax			; save	new right

		mov	ebx, WndPos.right	; since	we're FORCED to	provide
		sub	ebx, WndPos.left	; a new width, let's recompute
		inc	ebx			; it from the existing width

		mov	eax, WndPos.bottom	; and we'll do	the same for
		sub	eax, WndPos.top		; the old and new heights
		inc	eax

IF CORNER_THE_WINDOW
		zero	ecx
		zero	edx
ENDIF
		invoke	MoveWindow, hWnd, edx,	ecx, ebx, eax, TRUE ; and move it!
		ret
;-------------------------------------------------------------------------------
CenterWindow	ENDP


;+-----------------------------------------------------------------------------+
;|			     CREATE CHILD CONTROLS			       |
;|-----------------------------------------------------------------------------|
;|		This creates all static	windows	for our	program		       |
;|	  Now create the block of standard windows from	our array ...	       |
;+-----------------------------------------------------------------------------+
CreateChildControls	PROC USES ebx
;-------------------------------------------------------------------------------
	mov	esi, OFFSET WindowCreationTable
	mov	edi, WINDOW_CREATION_ENTRIES
	.REPEAT
		push	dword ptr NULL		; no window creation data
		push	hInst			; our local hInstance
		push	dword ptr [esi+32]	; Ctrl ID
		mov	ebx, [esi+28]		; get the parent's	offset
		.IF (ebx)			;	if parent's offset non-null
			mov	ebx, [ebx]	; then load its value
		.ENDIF
		push	ebx			; hWndParent
		push	dword ptr [esi+24]	; nHeight
		push	dword ptr [esi+20]	; nWidth
		push	dword ptr [esi+16]	; y
		push	dword ptr [esi+12]	; x
		push	dword ptr [esi+ 8]	; the	Style
		push	dword ptr [esi+ 4]	; stack the window's name
		push	dword ptr [esi	]	; stack the ClassName
		push	dword ptr 0		; NULL extended style
		call	CreateWindowEx
		check	eax			;	if create failed
		jz	AbortedCreation		;	then take us out!
		;------------------------------------------------------------
		mov	ebx, [esi+36]		; get the destination offset
		.IF (ebx)			;	if we have a place to store
			mov	[ebx], eax	; the	resulting hWnd,	do so
		.ENDIF
		add	esi, SIZE_OF_CREATE_ENTRY	; bump up to the next entry
		dec	edi
	.UNTIL	(zero?)
	dec	edi		; dec	DI again to clear the ZERO flag	...
;-------------------------------------------------------------------------------
AbortedCreation:	ret
;-------------------------------------------------------------------------------
CreateChildControls	ENDP


;+-----------------------------------------------------------------------------+
;|			     SET CHILD VISIBILITY			       |
;|-----------------------------------------------------------------------------|
;|   Sets the CurrentPage based	window visibility state	for the	window whose   |
;|  handle we're being handed. This is an enumeration of all hMainWnd.	 |
;+-----------------------------------------------------------------------------+
SetChildVisibility	PROC	hWnd:HWND, lParam:LPARAM
;-------------------------------------------------------------------------------
	invoke	GetWindowLong, hWnd, GWL_ID	; pickup the child's ID
	and	eax, PAGE_MASK
	shr	eax, PAGE_SHIFT	; convert ID to control's page number
	.IF (eax == ACTION_PAGES)	; if	it's on	all three pages
		.IF (CurrentPage >= FIRST_ACTION_PAGE) && (CurrentPage <= LAST_ACTION_PAGE)
			jmp	ShowTheControl
		.ENDIF
		jmp	HideTheControl

	.ELSEIF (eax != EVERY_PAGE); munge only if it's NOT on	every page
		.IF (eax == CurrentPage)
ShowTheControl:		invoke	GetWindowLong, hWnd, GWL_STYLE
			or	eax, WS_VISIBLE
		.ELSE
HideTheControl:		invoke	GetWindowLong, hWnd, GWL_STYLE
			and	eax, NOT	WS_VISIBLE
		.ENDIF
		invoke	SetWindowLong,	hWnd, GWL_STYLE, eax
	.ENDIF
	mov	eax, TRUE	; return	non-zero to continue enumerating ...
	ret
;-------------------------------------------------------------------------------
SetChildVisibility	ENDP


;+-----------------------------------------------------------------------------+
;|			   SET NAV BUTTON ENABLE			       |
;+-----------------------------------------------------------------------------+
SetNavButtonEnable	PROC	USES edi ebx, id:DWORD,	enabled:DWORD
;-------------------------------------------------------------------------------
	invoke	GetDlgItem, hMainWnd, id	; get the handle
	mov	edi, eax			; save hWnd in EDI
	mov	ebx, enabled			; get the presence bitarray
	mov	cl, byte ptr PreviousPage	; get the PREVIOUS Page
	shr	ebx, cl				; "1" bit has PREVIOUS status
	mov	eax, enabled			; get the presence bitarray
	mov	cl, byte ptr CurrentPage	;	get the	CURRENT	Page
	shr	eax, cl				; "1" bit has CURRNET status
	xor	ebx, eax			; one bit has "delta"
	.IF (ebx & 1)
		shr	eax, 1			; place new status into CARRY
		.IF (carry?)
			invoke	GetWindowLong, edi, GWL_STYLE
			and	eax, NOT	WS_DISABLED
		.ELSE
			invoke	GetWindowLong, edi, GWL_STYLE
			or	eax, WS_DISABLED
		.ENDIF
		invoke	SetWindowLong,	edi, GWL_STYLE,	eax
		invoke	InvalidateRect, edi, NULL, TRUE
	.ENDIF
	ret
;-------------------------------------------------------------------------------
SetNavButtonEnable	ENDP


;+-----------------------------------------------------------------------------+
;|	     This sets the visibility of windows for a given page	       |
;+-----------------------------------------------------------------------------+
SetCurrentWindow	PROC
;-------------------------------------------------------------------------------
	invoke	ValidateRect, hMainWnd,	NULL
	invoke	LockWindowUpdate, hMainWnd
	invoke	EnumChildWindows, hMainWnd, ADDR SetChildVisibility, NULL
	invoke	LockWindowUpdate, NULL
	.IF	(PreviousPage >= FIRST_ACTION_PAGE) && (PreviousPage <= LAST_ACTION_PAGE)\
	     && (CurrentPage  >= FIRST_ACTION_PAGE) &&	(CurrentPage  <= LAST_ACTION_PAGE)
		invoke	InvalidateRect, hMainWnd, ADDR	TabWindowRegionRect, TRUE
	.ELSE
		invoke	InvalidateRect, hMainWnd, ADDR	ChildRegionRect, TRUE
	.ENDIF

	.IF (PreviousPage >= FIRST_ACTION_PAGE) || (CurrentPage >= FIRST_ACTION_PAGE)
		invoke	InvalidateRect, hMainWnd, ADDR	WebButton, TRUE
	.ENDIF
	invoke	SetNavButtonEnable, IDB_BACK, BackButtonEnabled
	invoke	SetNavButtonEnable, IDB_NEXT, NextButtonEnabled
	invoke	UpdateWindow, hMainWnd
	ret
;-------------------------------------------------------------------------------
SetCurrentWindow	ENDP


;+-----------------------------------------------------------------------------+
;|				3D SINK	WndProc				       |
;+-----------------------------------------------------------------------------+
SinkWndProc	PROC USES esi edi, hWnd:HWND, msg:DWORD, wParam:WPARAM, lParam:LPARAM
	 	LOCAL	ps:PAINTSTRUCT, Rect:RECT
;-------------------------------------------------------------------------------
	mov	eax, msg
;+-----------------------------------------------------------------------------+
;|  WM_PAINT : Simply pass this	onto the Dialog's Frame	painter	...	       |
;+-----------------------------------------------------------------------------+
.IF (eax == WM_PAINT)
	invoke	BeginPaint, hWnd, ADDR ps	; setup our paintstruct
	mov	edi, eax
	invoke	GetClientRect, hWnd, ADDR Rect
	invoke	GetWindowLong, hWnd, GWL_ID
	shr	eax, TREATMENT_STYLE_SHIFT
	.IF (ax == THREE_DEE_BOX)
		invoke	DrawEdge, edi,	ADDR Rect, EDGE_SUNKEN,	BF_RECT
	.ELSEIF (ax !=	NO_SINK_OUTLINE)
		dec	Rect.right
		dec	Rect.bottom
		invoke	MoveToEx, edi,	0, Rect.bottom,	NULL
		invoke	SelectObject, edi, hDkGrayPen
		invoke	LineTo, edi, 0, 0
		invoke	LineTo, edi, Rect.right, 0
		invoke	SelectObject, edi, hWhitePen
		invoke	LineTo, edi, Rect.right, Rect.bottom
		invoke	LineTo, edi, -1, Rect.bottom
	.ENDIF
	invoke	EndPaint, hWnd,	ADDR ps		; returns zero	if we process
.ELSE
	invoke	 DefWindowProc,	hWnd, msg, wParam,lParam
.ENDIF
	ret
;-------------------------------------------------------------------------------
SinkWndProc	ENDP


;+-----------------------------------------------------------------------------+
;|				 SUNKEN	FIELDS				       |
;+-----------------------------------------------------------------------------+
SunkenFields	PROC USES esi edi,	hDC:HDC, pFirstRect:LPRECT, count:DWORD, yspacing:DWORD
		LOCAL	DrawRect:RECT
;-------------------------------------------------------------------------------
	invoke	CopyRect, ADDR DrawRect, pFirstRect	; make a local copy
	mov	edi, hDC
	mov	esi, count
	.REPEAT
		invoke	DrawEdge, edi,	ADDR DrawRect, BDR_SUNKENOUTER,	BF_RECT
		mov	eax, yspacing
		add	DrawRect.top, eax
		add	DrawRect.bottom, eax
		dec	esi
	.UNTIL	(zero?)
	ret
;-------------------------------------------------------------------------------
SunkenFields	ENDP


;+-----------------------------------------------------------------------------+
;|			       PAINT TEXT ARRAY				       |
;+-----------------------------------------------------------------------------+
PaintTextArray	PROC USES esi edi ebx, hdc:HDC, pArray:LPVOID, color:DWORD
;-------------------------------------------------------------------------------
	mov	edi, hdc
	invoke	SetTextColor, edi, color
	mov	esi, pArray
	.REPEAT
		add	esi, SIZEOF DWORD	; skip past the	coords
		invoke	lstrlen, esi
		movzx	ebx, word ptr [esi-4]
		movzx	ecx, word ptr [esi-2]
		invoke	TextOut, edi, ebx, ecx, esi, eax
		invoke	lstrlen, esi
		inc	eax		; account for the last zero
		add	esi, eax	; point to the next set of coords
	.UNTIL	!(word ptr [esi])
	ret
;-------------------------------------------------------------------------------
PaintTextArray	ENDP


;+-----------------------------------------------------------------------------+
;|			       PAINT TEST PHASE				       |
;+-----------------------------------------------------------------------------+
PaintTestPhase	PROC USES esi edi, hdc:HDC
;-------------------------------------------------------------------------------
	mov	edi, hdc
	invoke	CreateCompatibleDC, NULL ; and a DC for	our bitmap source
	mov	esi, eax
	invoke	SelectObject, esi, hOffBitmap
	push	eax
	;컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴
	.IF (TestingPhase == READING_DATA)
		invoke	SelectObject, esi, hGreenBitmap
		push	eax
	.ENDIF
	invoke	BitBlt,	edi, 320, 5, 11, 11, esi, 0, 0,	SRCCOPY
	.IF (TestingPhase == READING_DATA)
		pop	eax
		invoke	SelectObject, esi, eax
	.ENDIF
	;--------------------------------------------------------------------------
	.IF (TestingPhase == WRITING_PATT)
		invoke	SelectObject, esi, hRedBitmap
		push	eax
	.ENDIF
	invoke	BitBlt,	edi, 336, 5, 11, 11, esi, 0, 0,	SRCCOPY
	.IF (TestingPhase == WRITING_PATT)
		pop	eax
		invoke	SelectObject, esi, eax
	.ENDIF
	;--------------------------------------------------------------------------
	.IF (TestingPhase == READING_PATT)
		invoke	SelectObject, esi, hGreenBitmap
		push	eax
	.ENDIF
	invoke	BitBlt,	edi, 336,21, 11, 11, esi, 0, 0,	SRCCOPY
	.IF (TestingPhase == READING_PATT)
		pop	eax
		invoke	SelectObject, esi, eax
	.ENDIF
	;--------------------------------------------------------------------------
	.IF (TestingPhase == WRITING_DATA)
		invoke	SelectObject, esi, hRedBitmap
		push	eax
	.ENDIF
	invoke	BitBlt,	edi, 320,21, 11, 11, esi, 0, 0,	SRCCOPY
	.IF (TestingPhase == WRITING_DATA)
		pop	eax
		invoke	SelectObject, esi, eax
	.ENDIF
	;컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴
	pop	eax
	invoke	SelectObject, esi, eax
	invoke	DeleteDC, esi
	ret
;-------------------------------------------------------------------------------
PaintTestPhase	ENDP


;+-----------------------------------------------------------------------------+
;|			       PAINT CART STATUS			       |
;|		  Paints the textual cartridge status window		       |
;+-----------------------------------------------------------------------------+
PaintCartStatus	PROC USES esi edi ebx, hDC:HDC
		LOCAL	TextSize:_SIZE
;-------------------------------------------------------------------------------
	mov	edi, hDC
	; set DC's default text modes
	invoke	SetTextColor, edi, BLACK_COLOR
	invoke	SelectObject, edi, hDialogTextFont
	invoke	SetBkMode,    edi, TRANSPARENT
	; blank out any previous status display
	invoke	SelectObject, edi, hNullPen
	invoke	SelectObject, edi, hWindowFillBrush
	invoke	Rectangle, edi,	115, 9,	241, 27
	; display the new cartridge status
	mov	eax, CartridgeStatus
	dec	eax
	.IF (sign?) ||	(eax > (LAST_CART_STATUS-1))
		zero	eax
	.ENDIF
	; pickup the pointer to the string
	.IF (DriveCount)
		mov	esi, CartStatStrings[eax*4]
	.ELSE
		mov	esi, OFFSET szNoIomegaDrives
	.ENDIF
	invoke	lstrlen, esi
	lea	ebx, TextSize
	invoke	GetTextExtentPoint32, edi, esi,	eax, ebx
	mov	ebx, (241+115)/2	;	get the	center X
	halve	TextSize._cx
	sub	ebx, TextSize._cx
	invoke	lstrlen, esi
	invoke	TextOut, edi, ebx, 9, esi, eax
	ret
;-------------------------------------------------------------------------------
PaintCartStatus	ENDP


;+-----------------------------------------------------------------------------+
;|				PAINT BAR GRAPH				       |
;+-----------------------------------------------------------------------------+
PaintBarGraph	PROC USES	esi edi	ebx, hDC:HDC, Xleft:DWORD, Ytop:DWORD, Xwidth:DWORD, Yheight:DWORD, BarColor:DWORD, BarValue:DWORD, pRightText:DWORD, Active:BOOL
		LOCAL	AbsoluteBarWidth:DWORD,	Extent:_SIZE, PercentString[8]:CHAR
;-------------------------------------------------------------------------------
	; create a temporary drawing bitmap for double-buffererd rendering
	invoke	GetDC, hTestMonitor
	mov	esi, eax		; hold the creation DC in esi
	invoke	CreateCompatibleBitmap,	esi, Xwidth, Yheight
	swap	esi, eax		; eax gets DC, esi gets	bitmap
	invoke	ReleaseDC, hTestMonitor, eax

	; create a context for	drawing	onto the double	buffer bitmap
	invoke	CreateCompatibleDC, NULL
	swap	esi, eax		; eax gets bitmap, esi gets new	DC
	invoke	SelectObject, esi, eax
	push	eax			; save the old bitmap to remove bitmap
	invoke	SetBkMode, esi,	TRANSPARENT

	; fill	the entire rectangle with background gray
	invoke	SelectObject, esi, hNullPen
	invoke	SelectObject, esi, hWindowFillBrush
	mov	eax, Xwidth
	inc	eax
	mov	ebx, Yheight
	inc	ebx
	invoke	Rectangle, esi,	0, 0, eax, ebx

.IF (Active)	; now fleshout the	bitmap ONLY IF we're active

	; if RightText	string is non-null, paint it in	darker gray
	mov	edi, pRightText
	.IF (edi)
		invoke	SelectObject, esi, hDialogTextFont
		invoke	SetTextColor, esi, GRAY_COLOR
		invoke	lstrlen, edi
		lea	ebx, Extent
		invoke	GetTextExtentPoint32, esi, edi, eax, ebx
		invoke	lstrlen, edi
		mov	ebx, Xwidth		; get the total width
		sub	ebx, Extent._cx
		halve	ebx
		mov	ecx, Yheight
		sub	ecx, Extent._cy
		halve	ecx
		invoke	TextOut, esi, ebx, ecx, edi, eax
	.ENDIF

	; now paint the active	portion
	invoke	CreateSolidBrush, BarColor
	invoke	SelectObject, esi, eax
	push	eax				; save prior brush
	mov	eax, Xwidth
	inc	eax
	mul	BarValue	;	edx will have the new width
	add	eax, 80000000h	; now round	it up!
	adc	edx, 0
	mov	AbsoluteBarWidth, edx
	mov	eax, Yheight
	inc	eax
	invoke	Rectangle, esi,	0, 0, edx, eax
	pop	eax				; recover prior brush
	invoke	SelectObject, esi, eax		; recover newly	created	brush
	invoke	DeleteObject, eax		; delete the	brush

	; now place the floating percentage into the middle (if it fits there)
	invoke	SelectObject, esi, hTitleFont
	invoke	SetTextColor, esi, WHITE_COLOR
	mov	eax, 100	;	calculate the percentage
	mul	BarValue	;	edx will have the new width
	add	eax, 80000000h	; now round	it up!
	adc	edx, 0
	invoke	wsprintf, ADDR PercentString, ADDR szBarChartPercent, edx
	mov	edi, eax
	lea	ebx, Extent
	invoke	GetTextExtentPoint32, esi, ADDR	PercentString, eax, ebx
	mov	ebx, AbsoluteBarWidth	; get the active bar's width
	sub	ebx, Extent._cx
	sar	ebx, 1			;	scale back but keep sign
	.IF (sign?)
		invoke	SelectObject, esi, hDialogTextFont
		mov	ebx, AbsoluteBarWidth
	.ENDIF
	mov	ecx, Yheight
	sub	ecx, Extent._cy
	halve	ecx
	invoke	TextOut, esi, ebx, ecx,	ADDR PercentString, edi

.ENDIF
	; now blit the	final result out to the	screen
	invoke	BitBlt,	hDC, Xleft, Ytop, Xwidth, Yheight, esi,	0, 0, SRCCOPY

	; and clean up	after all the work
	pop	eax				; recover prior bitmap
	invoke	SelectObject, esi, eax		; recover newly	created	bitmap
	invoke	DeleteObject, eax		; delete the	bitmap
	invoke	DeleteDC, esi			; delete created container DC
Exit:	ret
;-------------------------------------------------------------------------------
PaintBarGraph	ENDP


;+-----------------------------------------------------------------------------+
;|			     PAINT THE BAR GRAPHS			       |
;|     This paints the two or three bar	graphs on the test monitor window.     |
;+-----------------------------------------------------------------------------+
PaintTheBarGraphs	PROC USES ebx, hDC:HDC, Active:BOOL
;-------------------------------------------------------------------------------
	invoke	PaintBarGraph, hDC, 13,	 57, 395, 14, BLUE_COLOR, PercentComplete, NULL, Active
	.IF (JazDrive)
		mov	ebx, Side_0_SparesCount
		mov	ecx, MAXIMUM_JAZ_SPARES
		.IF (ebx > ecx)			; if Spares	>  MAXIMUM
			mov	ebx, ecx	; clip Spares down to MAX
		.ENDIF
		call	CvrtIntoPrcnt
		mov	ebx, eax
		invoke	PaintBarGraph,	hDC, 13,  95, 395, 30, RED_COLOR, ebx, NULL, Active
	.ELSE
		mov	ebx, Side_0_SparesCount
		mov	ecx, MAXIMUM_ZIP_SPARES
		call	CvrtIntoPrcnt
		invoke	PaintBarGraph,	hDC, 13,  95, 395, 14, RED_COLOR, eax, ADDR szSide0, Active
		mov	ebx, Side_1_SparesCount
		mov	ecx, MAXIMUM_ZIP_SPARES
		call	CvrtIntoPrcnt
		invoke	PaintBarGraph,	hDC, 13, 111, 395, 14, RED_COLOR, eax, ADDR szSide1, Active
	.ENDIF
;-------------------------------------------------------------------------------
	ret

;-------------------------------------------------------------------------------
CvrtIntoPrcnt:	zero	edx
	   	zero	eax
	   	dec	eax	; set edx:eax to [0:FFFFFFFF]
	   	div	ecx	; divide by the maximum possible
	   	neg	ebx
	   	add	ebx, ecx
	   	mul	ebx
;-------------------------------------------------------------------------------
		LocalReturn

;-------------------------------------------------------------------------------
PaintTheBarGraphs	ENDP


;+-----------------------------------------------------------------------------+
;|			     PAINT CENTERED STRING			       |
;|	   Paints a Double-Buffered string into	a rectangular region	       |
;+-----------------------------------------------------------------------------+
PaintCenteredString	PROC USES esi edi ebx, hDC:HDC, Xleft:DWORD, Ytop:DWORD, Xwidth:DWORD, Yheight:DWORD, pText:DWORD, Active:BOOL
			LOCAL	Extent:_SIZE
;-------------------------------------------------------------------------------
	; create a temporary drawing bitmap for double-buffererd rendering
	invoke	GetDC, hTestMonitor
	mov	esi, eax		; hold the creation DC in esi
	invoke	CreateCompatibleBitmap,	esi, Xwidth, Yheight
	swap	esi, eax		; eax gets DC, esi gets	bitmap
	invoke	ReleaseDC, hTestMonitor, eax

	; create a context for	drawing	onto the double	buffer bitmap
	invoke	CreateCompatibleDC, NULL
	swap	esi, eax		; eax gets bitmap, esi gets new	DC
	invoke	SelectObject, esi, eax
	push	eax			; save the old bitmap to remove bitmap
	invoke	SetBkMode, esi,	TRANSPARENT

	; fill	the entire rectangle with background gray
	invoke	SelectObject, esi, hNullPen
	invoke	SelectObject, esi, hWindowFillBrush
	mov	eax, Xwidth
	inc	eax
	mov	ebx, Yheight
	inc	ebx
	invoke	Rectangle, esi,	0, 0, eax, ebx

.IF (Active)	; now fleshout the	bitmap ONLY IF we're active

	; now place the floating string into the middle
	invoke	SelectObject, esi, hDialogTextFont
	invoke	lstrlen, pText
	mov	edi, eax
	lea	ebx, Extent
	invoke	GetTextExtentPoint32, esi, pText, eax, ebx
	mov	ebx, Xwidth		; get	the region's width
	sub	ebx, Extent._cx
	halve	ebx
	mov	ecx, Yheight
	sub	ecx, Extent._cy
	halve	ecx
	invoke	TextOut, esi, ebx, ecx,	pText, edi

.ENDIF
	; now blit the	final result out to the	screen
	invoke	BitBlt,	hDC, Xleft, Ytop, Xwidth, Yheight, esi,	0, 0, SRCCOPY

	; and clean up	after all the work
	pop	eax				; recover prior bitmap
	invoke	SelectObject, esi, eax		; recover newly	created	bitmap
	invoke	DeleteObject, eax		; delete the	bitmap
	invoke	DeleteDC, esi			; delete created container DC
Exit:	ret
;-------------------------------------------------------------------------------
PaintCenteredString	ENDP


;+-----------------------------------------------------------------------------+
;|			     PAINT CENTERED VALUE			       |
;|	Paints a Double-Buffered decimal value into a rectangular region       |
;+-----------------------------------------------------------------------------+
PaintCenteredValue	PROC	USES esi edi, hDC:HDC, Xleft:DWORD, Ytop:DWORD,	Xwidth:DWORD, Yheight:DWORD, value:DWORD, Active:BOOL
			LOCAL	szString[40]:CHAR
;-------------------------------------------------------------------------------
	; convert our value into a string
	invoke	wsprintf, ADDR szString, ADDR szCenteredDecimal, value
	; and paint it's value
	invoke	PaintCenteredString, hDC, Xleft, Ytop, Xwidth, Yheight,	ADDR szString, Active
	ret
;-------------------------------------------------------------------------------
PaintCenteredValue	ENDP


;+-----------------------------------------------------------------------------+
;|			     PAINT TEST	STATISTICS			       |
;|	       This paints the two columns of testing statistics	       |
;|			  on the test monitor window.			       |
;+-----------------------------------------------------------------------------+
PaintTestStatistics	PROC USES esi edi ebx, hDC:HDC, Active:BOOL
			LOCAL	szString[40]:CHAR
;-------------------------------------------------------------------------------
	mov	edi, hDC
	; assemble and	paint the sector testing range
	mov	eax, SingleTransferLBA	; are we testing a single sector?
	.IF (eax)
		invoke	wsprintf, ADDR	szString, ADDR szCenteredDecimal, eax
	.ELSE
		mov	eax, FirstLBASector
		.IF (!TestingPhase)
			zero	eax
		.ENDIF
		invoke	wsprintf, ADDR	szString, ADDR szCenteredDecimal, eax
		invoke	lstrcat, ADDR szString, ADDR szSpaceDashSpace
		invoke	lstrlen, ADDR szString
		lea	esi, szString
		add	esi, eax
		mov	eax, LastLBASector
		.IF (TestingPhase == READY_TO_TEST)
			mov	eax, LastLBAOnCartridge
		.ENDIF
		invoke	wsprintf, esi,	ADDR szCenteredDecimal,	eax
	.ENDIF
	invoke	PaintCenteredString,  edi,  76,	155, 126, 14, ADDR szString, Active

	; show	the LastError
	push	edi				; save our	display	context
	mov	edi, OFFSET ErrorTypeTest	; point to the top of the list
TryIt:	mov	eax,	[edi]			; get a	candidate error	#
	add	edi, 4
	.IF (eax != (-1))			; if we haven't hit the end
		cmp	eax, LastError
		je	ShowIt
		mov	al, 0			;	nope, so scan to the end
		mov	ecx, 40
		repne	scasb			; of the string
		jmp	TryIt
	; it *is* unknown, so we show the value of the	error
	.ELSEIF (UnknownErrorMode)
		invoke	wsprintf, ADDR	szString, ADDR szCenteredHex, LastError
		lea	edi, szString
	.ENDIF
ShowIt:	mov	eax, edi
	pop	edi
	invoke	PaintCenteredString,  edi,  76,	172, 126, 14, eax, Active

	; show	the elapsed time
	mov	eax, SecondsElapsed
	call	CvrtSecondsToHMSstring
	invoke	PaintCenteredString,  edi,  76,	189, 126, 14, ADDR szString, Active

	; see if it's time for	us to estimate ...
	mov	edx, SecondsElapsed
	mov	eax, edx
	sub	eax, ElapsedTimeOfLastEstimate
	.IF (eax > 15)
		; assemble the remaining time
		mov	ElapsedTimeOfLastEstimate, edx	; note this estimate time
		zero	eax
		mov	ebx, PercentComplete
		.IF (ebx > edx)		; if	it's safe to divide, show remaining time
			roundiv	ebx	; eax will have estimated completion
		.ENDIF
		mov	CurrentTotalTimeEstimate,	eax
	.ELSE
		mov	eax, CurrentTotalTimeEstimate
	.ENDIF

	; given the current estimate time, show the remaining time!
	.IF (eax)
		sub	eax, SecondsElapsed
		.IF (carry?)		; if it	went negative
			zero	eax	; clamp it to zero
		.ENDIF
		call	CvrtSecondsToHMSstring
	.ELSE
		invoke	lstrcpy, ADDR szString, ADDR szEstimating
	.ENDIF
	.IF (TestingPhase <= READY_TO_TEST)
		invoke	lstrcpy, ADDR szString, ADDR szOneMoment
	.ENDIF
	invoke	PaintCenteredString,  edi,  76,	206, 126, 14, ADDR szString, Active

	; now show the	error accumulations ...
	mov	esi, SoftErrors
	invoke	PaintCenteredValue, edi, 347, 155,  61,	14, esi, Active
	mov	eax, FirmErrors
	add	esi, eax
	invoke	PaintCenteredValue, edi, 347, 172,  61,	14, eax, Active
	mov	eax, HardErrors
	add	esi, eax
	invoke	PaintCenteredValue, edi, 347, 189,  61,	14, eax, Active
	invoke	PaintCenteredValue, edi, 347, 206,  61,	14, esi, Active

;-------------------------------------------------------------------------------
	ret

CvrtSecondsToHMSstring:
;-------------------------------------------------------------------------------
	mov	ebx, 60
	zero	edx
	div	ebx		; edx	= seconds / eax	= minutes
	mov	ecx, edx	;	ecx = seconds
	zero	edx
	div	ebx		; edx	= minutes / eax	= hours
	invoke	wsprintf, ADDR szString, ADDR szHoursMinsSecs, eax, edx, ecx
;-------------------------------------------------------------------------------
	LocalReturn

;-------------------------------------------------------------------------------
PaintTestStatistics	ENDP


;+-----------------------------------------------------------------------------+
;|			     TEST MONITOR WND PROC			       |
;+-----------------------------------------------------------------------------+
TestMonitorWndProc	PROC	USES esi edi, hWnd:HWND, msg:DWORD, wParam:WPARAM, lParam:LPARAM
	 		LOCAL	ps:PAINTSTRUCT, Rect:RECT
;-------------------------------------------------------------------------------
	mov	eax, msg
;+-----------------------------------------------------------------------------+
;|  WM_PAINT : Paint our entire	test monitor window			       |
;+-----------------------------------------------------------------------------+
.IF (eax == WM_PAINT)
	invoke	BeginPaint, hWnd, ADDR ps	; setup our paintstruct
	mov	edi, eax
	;컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴
	invoke	SelectObject, edi, hDialogTextFont
	invoke	SetBkMode,    edi, TRANSPARENT
	invoke	lstrlen, ADDR szCartStatus
	invoke	TextOut, edi, 12, 9, ADDR szCartStatus,	eax
	invoke	DrawEdge, edi, ADDR CS_Stat, BDR_SUNKENOUTER, BF_RECT
	invoke	PaintCartStatus, edi

	; draw	the sunken rectangles
	invoke	DrawEdge, edi, ADDR TP_Perc, BDR_SUNKENOUTER, BF_RECT
	invoke	DrawEdge, edi, ADDR SE_Rect, BDR_SUNKENOUTER, BF_RECT
	invoke	SunkenFields, edi, ADDR	TL_Sect, 4, 17
	invoke	SunkenFields, edi, ADDR	ES_Read, 4, 17
	.IF (JazDrive)	; draw a single	LARGE rectangle
		invoke	DrawEdge, edi,	ADDR SS_Jaz, BDR_SUNKENOUTER, BF_RECT
	.ELSE		; draw a pair of smaller rectangles
		invoke	SunkenFields, edi, ADDR SS_Sid0, 2, 16
	.ENDIF
	.IF (CartridgeStatus == DISK_AT_SPEED)	|| (CartridgeStatus == DISK_SPUN_DOWN) || (CartridgeStatus >= DISK_LOW_SPARES)
		invoke	PaintTheBarGraphs, edi, TRUE
		invoke	PaintTestStatistics, edi, TRUE
		mov	eax, BLACK_COLOR
	.ELSE
		invoke	PaintTheBarGraphs, edi, FALSE
		invoke	PaintTestStatistics, edi, FALSE
		mov	eax, GRAY_COLOR
	.ENDIF
	invoke	PaintTextArray,	edi, ADDR TestBlackText, eax
	invoke	PaintTextArray,	edi, ADDR TestGrayText,	 GRAY_COLOR
	invoke	PaintTestPhase,	edi

	; paint the little speaker icon
	invoke	CreateCompatibleDC, NULL ; and a DC for	our bitmap source
	mov	esi, eax
	invoke	SelectObject, esi, hSoundBitmap
	push	eax
	invoke	BitBlt,	edi, 232, 191, 16, 16, esi, 0, 0, SRCCOPY
	pop	eax
	invoke	SelectObject, esi, eax
	invoke	DeleteDC, esi

	;컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴
	invoke	EndPaint, hWnd,	ADDR ps		; returns zero	if we process
.ELSE
	invoke	 DefWindowProc,	hWnd, msg, wParam,lParam
.ENDIF
	ret
;-------------------------------------------------------------------------------
TestMonitorWndProc	ENDP


;+-----------------------------------------------------------------------------+
;|				  DATA SOURCE				       |
;+-----------------------------------------------------------------------------+
DataSource	PROC	USES ebx ecx edx,	pDest:PTR SBYTE, pCount:PTR DWORD, pCompData:PTR COMP_DATA
;-------------------------------------------------------------------------------
	ASSUME	ebx:PTR	COMP_DATA
;-------------------------------------------------------------------------------
	mov	ebx, pCompData
	mov	eax, [ebx].EndOfBuffer	 ; get the end of the buffer
	mov	edx, [ebx].SourcePointer ;	get the	current	start
	sub	eax, edx			; calc the remaining amount
	mov	ecx, pCount		 ; get the pointer to	the count
	mov	ecx, [ecx]		 ; and	get the	actual count
	.IF (eax > ecx)			 ; if it's more than	we can take
		mov	eax, ecx		; diminish it to the limit
	.ENDIF
	add	[ebx].SourcePointer, eax ;	bump the offset	forward	for next
	push	eax			 ; save the	amount transferred
	invoke	MoveMemory, pDest, edx,	eax ; move the data
	pop	eax			 ; recover the amount we moved
	ret	; and return the Count that we placed into	the buffer
;-------------------------------------------------------------------------------
	ASSUME	ebx:NOTHING
;-------------------------------------------------------------------------------
DataSource	ENDP


;+-----------------------------------------------------------------------------+
;|				   DATA	SINK				       |
;+-----------------------------------------------------------------------------+
DataSink	PROC USES ebx ecx edx,	pSource:PTR SBYTE, pCount:PTR DWORD, pCompData:PTR COMP_DATA
;-------------------------------------------------------------------------------
	ASSUME	ebx:PTR	COMP_DATA
;-------------------------------------------------------------------------------
	mov	ebx, pCompData
	mov	eax, pCount		; get	the pointer to the count
	mov	eax, [eax]		; and the count itself
	mov	edx, [ebx].SinkPointer
	add	[ebx].SinkPointer,	eax
	push	eax
	invoke	MoveMemory, edx, pSource, eax
	pop	eax
	ret
;-------------------------------------------------------------------------------
	ASSUME	ebx:NOTHING
;-------------------------------------------------------------------------------
DataSink	ENDP


;+-----------------------------------------------------------------------------+
;|			       DECOMPRESS BINARY			       |
;+-----------------------------------------------------------------------------+
DecompressBinary	PROC USES esi,	pBitmapImage:LPVOID
			LOCAL	pWorkingBuffer:LPSTR, CompData:COMP_DATA
;-------------------------------------------------------------------------------
	invoke	GlobalAlloc, GPTR, EXP_BUFFER_SIZE
	mov	pWorkingBuffer, eax
	mov	esi, pBitmapImage		; get the original size
	mov	eax, [esi]			; eax	has uncompressed size
	push	eax				; save for	function return
	invoke	GlobalAlloc, GPTR, eax
	push	eax				; save pointer to return
	mov	CompData.SinkPointer, eax
	add	esi, 8				; point to the data's start
	mov	CompData.SourcePointer, esi	; set the starting point
	add	esi, [esi-4]
	mov	CompData.EndOfBuffer, esi
	invoke	explode, ADDR DataSource, ADDR DataSink, pWorkingBuffer, ADDR CompData
	invoke	GlobalFree, pWorkingBuffer
	pop	eax	; return the saved decompressed pointer
	pop	ebx	; return the original file size
	ret
;-------------------------------------------------------------------------------
DecompressBinary	ENDP


GetRandomNumber:
;+-----------------------------------------------------------------------------+
;|		     Simply returns a 32-bit random number		       |
;+-----------------------------------------------------------------------------+
	mov	eax, RandomSeed
	imul	eax, RANDOMULT
	inc	eax
	mov	RandomSeed, eax
	swap	ah, al
	ror	eax, 8
;-------------------------------------------------------------------------------
	ret


;+-----------------------------------------------------------------------------+
;|			    APPLICATION	TIMER PROC			       |
;+-----------------------------------------------------------------------------+
ApplicationTimerProc	PROC	hWnd:HWND, Msg:UINT, id:UINT,	time:DWORD
;-------------------------------------------------------------------------------
	pushad	; save ALL the regs so we return things	as we found them!
	; see whether we need to animate the titlepage
IFE SUPPRESS_LOGO_ANIMATION
	.IF (CurrentPage == INTRO_PAGE)
		call	FloatTheFloaters
	.ENDIF
ENDIF
	; only	if we have at least ONE	Iomega drive
	.IF (DriveCount)
		call	HandleDriveChanging
	.ENDIF
	; to flash the	ring around the	"Start Testing"	button
	.IF (ActionButtonFlasher) && (CurrentPage >= FIRST_ACTION_PAGE) && (CurrentPage <= LAST_ACTION_PAGE)
		invoke	GetDC,	hMainWnd
		mov	edi, eax
		mov	eax, hBlackPen
		.IF (ActionButtonFlasher & 1)
			mov	eax, hGrayPen
		.ENDIF
		invoke	SelectObject, edi, eax
		invoke	SelectObject, edi, hNullBrush
		invoke	Rectangle, edi, TEST_BUTTON_LEFT-1, TEST_BUTTON_TOP-1,	TEST_BUTTON_LEFT+TEST_BUTTON_WIDTH+1, TEST_BUTTON_TOP+TEST_BUTTON_HEIGHT+1
		invoke	Rectangle, edi, TEST_BUTTON_LEFT-2, TEST_BUTTON_TOP-2,	TEST_BUTTON_LEFT+TEST_BUTTON_WIDTH+2, TEST_BUTTON_TOP+TEST_BUTTON_HEIGHT+2
		invoke	ReleaseDC, hMainWnd, edi
		dec	ActionButtonFlasher
	.ENDIF
	popad
	ret
;-------------------------------------------------------------------------------
ApplicationTimerProc	ENDP


;+-----------------------------------------------------------------------------+
;|			 INITIALIZE THE	FLOATER	SYSTEM			       |
;+-----------------------------------------------------------------------------+
InitializeTheFloaterSystem	PROC	USES esi edi
;-------------------------------------------------------------------------------
	zero	esi		; ESI will be the OBJECT INSTANCE index
	.REPEAT			; EDI will be the OBJECT CLASS index
		mov	edi, ObjectTypes[esi]
		Call	InitPositionAndVelocity
		call	GetValidObjectHorzPos
		mov	HorzPosition[esi], eax
		call	GetValidObjectVertPos
		mov	VertPosition[esi], eax
		add	esi, SIZEOF DWORD
	.UNTIL	(esi >=	NUMBER_OF_FLOATERS * SIZEOF DWORD)
	; place the objects into their	initial	positions
	call	BlitTheObjects
	ret
;-------------------------------------------------------------------------------
InitializeTheFloaterSystem	ENDP


;+-----------------------------------------------------------------------------+
;|			   GET VALID OBJECT HORZ POS			       |
;+-----------------------------------------------------------------------------+
GetValidObjectHorzPos	PROC USES	ebx edx
;-------------------------------------------------------------------------------
	call	GetRandomNumber		; returns EAX
	mov	ebx, SPLASH_WIDTH
	mov	ecx, xwidth[edi]
	dec	ecx
	add	ebx, ecx
	mul	ebx
	sub	edx, ecx
	shl	edx, 16			; convert into 16o16
	mov	eax, edx
	ret
;-------------------------------------------------------------------------------
GetValidObjectHorzPos	ENDP


;+-----------------------------------------------------------------------------+
;|			   GET VALID OBJECT VERT POS			       |
;+-----------------------------------------------------------------------------+
GetValidObjectVertPos	PROC USES	ebx edx
;-------------------------------------------------------------------------------
	call	GetRandomNumber		; returns EAX
	mov	ebx, SPLASH_HEIGHT
	mov	ecx, yheight[edi]
	dec	ecx
	add	ebx, ecx
	mul	ebx
	sub	edx, ecx
	shl	edx, 16			; convert into 16o16
	mov	eax, edx
	ret
;-------------------------------------------------------------------------------
GetValidObjectVertPos	ENDP


;+-----------------------------------------------------------------------------+
;|			       MIX EDX BY THREE				       |
;|  With 1/3rd probability, this negates EDX, leaves it	alone, of zeroes it.   |
;+-----------------------------------------------------------------------------+
MixEDXbyThree	PROC
;-------------------------------------------------------------------------------
	push	edx
	call	GetRandomNumber		; eax
	mov	edx, 3
	mul	edx
	mov	eax, edx
	pop	edx
	.IF (!eax)
		zero	edx
	.ELSEIF (eax == 1)
		neg	edx
	.ENDIF
	ret
;-------------------------------------------------------------------------------
MixEDXbyThree	ENDP


;+-----------------------------------------------------------------------------+
;|			    PREP FOR FAST BLITTING			       |
;|    This BSWAPS every	DWORD of the bitmap's data placing the first bitmap    |
;|    bits up at the high end of the DWORD and linearizing them	all along.     |
;+-----------------------------------------------------------------------------+
PrepForFastBlitting	PROC USES esi edi, pBitmapFile:HGLOBAL
;-------------------------------------------------------------------------------
	mov	eax, pBitmapFile			  ; get	the data block
	mov	ecx, [eax].BITMAPFILEHEADER.bfSize	; pickup the file's size
	mov	esi, [eax].BITMAPFILEHEADER.bfOffBits
	sub	ecx, esi	;	calc the total size to bswap
	shr	ecx, 2		; divide by 4 for dwords
	add	esi, eax	;	cvrt offset into actual	pointer
	mov	edi, esi	;	setup our source and destination
	.REPEAT
		lodsd		; [abcd]
		ror	eax, 8	; [dabc]
		swap	ah, al	;	[dacb]
		ror	eax, 16	;	[cbda]
		swap	ah, al	;	[cbad]
		ror	eax, 8	; [dcba]
		stosd
	.UNTILCXZ
Exit:	ret
;-------------------------------------------------------------------------------
PrepForFastBlitting	ENDP


;+-----------------------------------------------------------------------------+
;|			  ALLOCATE SIXTEEN COLOR DIB			       |
;+-----------------------------------------------------------------------------+
AllocateSixteenColorDib	PROC USES ebx, Hwidth:DWORD, Vheight:DWORD
;-------------------------------------------------------------------------------
	mov	eax, Hwidth		; get	the width
	add	eax, 7			;	round up to dword boundary
	and	eax, 0FFFFFFF8h		;	(8 pixels per dword)
	mul	Vheight			; multiply by the number	of lines
	add	eax, 16*4 + SIZEOF	BITMAPINFOHEADER
	invoke	GlobalAlloc, GPTR, eax
	ASSUME	EAX:PTR	BITMAPINFOHEADER
	mov	[eax].biSize, SIZEOF BITMAPINFOHEADER
	movmov	[eax].biWidth, ebx, Hwidth
	movmov	[eax].biHeight,	ebx, Vheight
	mov	[eax].biPlanes, 1
	mov	[eax].biBitCount, 4
	ASSUME	EAX:NOTHING
	ret
;-------------------------------------------------------------------------------
AllocateSixteenColorDib	ENDP


;+-----------------------------------------------------------------------------+
;|			       SPLASH THE BITMAP			       |
;+-----------------------------------------------------------------------------+
SplashTheBitmap	PROC USES esi edi ebx, hDC:HDC
;-------------------------------------------------------------------------------
	mov	edi, hDC
	invoke	SelectPalette, edi, hSplashPalette, FALSE
	invoke	RealizePalette,	edi
	mov	eax, pSplashDIB
	mov	ebx, eax		; point ebx to the bits
	add	ebx, 16*4 + SIZEOF	BITMAPINFOHEADER
	zero	ecx
	zero	edx
	zero	esi
	invoke	SetDIBitsToDevice, edi,	16, 18,	[eax + BITMAPINFOHEADER.biWidth], [eax + BITMAPINFOHEADER.biHeight],0, 0, 0, [eax + BITMAPINFOHEADER.biHeight],	ebx, eax, DIB_RGB_COLORS
	ret
;-------------------------------------------------------------------------------
SplashTheBitmap	ENDP


;+-----------------------------------------------------------------------------+
;|			      FLOAT THE	FLOATERS			       |
;+-----------------------------------------------------------------------------+
FloatTheFloaters	PROC
;-------------------------------------------------------------------------------
	zero	esi			; ESI is the OBJECT	INSTANCE index
	.REPEAT				; EDI is the OBJECT	CLASS index
		mov	edi, ObjectTypes[esi]
;-------------------------------------------------------------------------------
DoHorz:		mov	eax, HorzPosition[esi]	; get the 16o16 format
		add	eax, HorzVelocity[esi]	; compute new location
		mov	HorzPosition[esi], eax	; and save	it, presuming okay
		shr	eax, 16			; discard the fractional part
		movsx	eax, ax
		cmp	eax, SPLASH_WIDTH
		jge	ReFloat
		check	eax
		jns	DoVert
		neg	eax			; make the offset positive
		cmp	eax, xwidth[edi]	; see if	we're too negative
		jge	ReFloat

DoVert:		mov	eax, VertPosition[esi]	; get the 16o16 format
		add	eax, VertVelocity[esi]	; compute new location
		mov	VertPosition[esi], eax	; and save	it, presuming okay
		shr	eax, 16
		movsx	eax, ax
		cmp	eax, SPLASH_HEIGHT
		jge	ReFloat
		check	eax
		jns	NextObject
		neg	eax			; make the offset positive
		cmp	eax, yheight[edi]	; see if we're too negative
		jl	NextObject

ReFloat:	Call	InitPositionAndVelocity
;-------------------------------------------------------------------------------
NextObject:	add	esi, SIZEOF DWORD
	.UNTIL	(esi >=	NUMBER_OF_FLOATERS * SIZEOF DWORD)
	; place the objects into their	initial	positions
	call	BlitTheObjects
	; and update the image	in the window
	mov	esi, hMainWnd
	.IF (esi)			; make sure the window exists!
		invoke	GetDC,	esi
		mov	edi, eax
		invoke	SplashTheBitmap, eax
		invoke	ReleaseDC, esi, edi
	.ENDIF
	ret
;-------------------------------------------------------------------------------
FloatTheFloaters	ENDP


;+-----------------------------------------------------------------------------+
;|			  INIT POSITION	AND VELOCITY			       |
;+-----------------------------------------------------------------------------+
InitPositionAndVelocity	PROC USES ebx
;-------------------------------------------------------------------------------
	; get a size-based vbelocity
	mov	eax, VELOCITY_MULTIPLIER
	mul	velocities[edi]
;	mul	xwidth[edi]		; EDX has the size-based velocity
	mov	edx, eax
	; choose a border from	which to emerge
	call	GetRandomNumber		; returns EAX
	mov	ebx, eax
	.IF (ebx & 1)
		call	GetValidObjectHorzPos
		mov	HorzPosition[esi], eax
		.IF (ebx & 2)		; TOP EDGE
			mov	eax, yheight[edi]
			dec	eax
			shl	eax, 16
			neg	eax
		.ELSE			; BOTTOM EDGE
			mov	eax, (SPLASH_HEIGHT-1) SHL 16
			neg	edx
		.ENDIF
		mov	VertPosition[esi], eax
		mov	VertVelocity[esi], edx
		Call	MixEDXbyThree
		mov	HorzVelocity[esi], edx
	.ELSE
		call	GetValidObjectVertPos
		mov	VertPosition[esi], eax
		.IF (ebx & 2)		; LEFT	EDGE
			mov	eax, xwidth[edi]
			dec	eax
			shl	eax, 16
			neg	eax
		.ELSE			; RIGHT EDGE
			mov	eax, (SPLASH_WIDTH-1) SHL 16
			neg	edx
		.ENDIF
		mov	HorzPosition[esi], eax
		mov	HorzVelocity[esi], edx
		Call	MixEDXbyThree
		mov	VertVelocity[esi], edx
	.ENDIF
	ret
;-------------------------------------------------------------------------------
InitPositionAndVelocity	ENDP


;+-----------------------------------------------------------------------------+
;|			       BLIT THE	OBJECTS				       |
;+-----------------------------------------------------------------------------+
BlitTheObjects	PROC USES esi edi ebx
;-------------------------------------------------------------------------------
	; Zero	the entire destination bitmap before summing into it ...
	mov	eax, pSplashDIB
	add	eax, SIZEOF BITMAPINFOHEADER + 16*4
	invoke	ZeroMemory, eax, (SPLASH_HEIGHT	* DEST_BITMAP_BYTE_WIDTH)

	; Now BLIT the	entire set of Objects into the bitmap
	zero	esi
	.REPEAT	; ESI will be the OBJECT INSTANCE index
		; EDI	will be	the OBJECT CLASS index
		mov	edi, ObjectTypes[esi]	; EDI is the pointer to the	OBJECT CLASS

		; establish the object's translucency	transformation table
		invoke	MoveMemory, ADDR xform, transform[edi], 16*4

		; let's figure out the horizontal
		mov	eax, HorzPosition[esi]	; get the current Horz
		shr	eax, 16			; discard the fractional
		movsx	eax, ax			; and	sign-extend it
		.IF (sdword ptr eax >= 0)
			; our X coord is positive, so no left clipping
			mov	ebx, SPLASH_WIDTH
			sub	ebx, eax
			mov	edx, xwidth[edi]
			.IF (ebx > edx)
				mov	ebx, edx
			.ENDIF
			mov	edx, eax
			zero	eax
		.ELSE
			; we	have a negative	X, so some of our left is clipped
			mov	ebx, xwidth[edi]
			add	ebx, eax	; EBX has the pixel count
			neg	eax		; EAX has the left SRC pixel
			zero	edx		; EDX has the left	DEST pixel
		.ENDIF
		.IF (ebx > SPLASH_WIDTH)	; trim our blit to window width
			mov	ebx, SPLASH_WIDTH
		.ENDIF
		mov	PixelsPerLine, ebx

		; EAX	has the	starting pixel in the SOURCE bitmap
		add	eax, xleft[edi]		; add-in	the source offset
		mov	ecx, eax		; first	setup the bit mask
		and	cl, 1Fh			; get the lower	FIVE bits
		mov	ebx, 80000000h		;	setup the source mask
		shr	ebx, cl
		mov	SourceBitStart, ebx

		and	eax, 0FFFFFFE0h		; mask off the lower five bits
		shr	eax, 3			; now the byte offset
		add	eax, pFontBitmap
		add	eax, ytop[edi]		;	!!! This is pre-computed !!!
		mov	SourceLineStart, eax

		; EDX	has the	starting pixel in the DEST bitmap
		mov	ecx, edx
		and	ecx, 00000007h		;	get 0-7
		shl	ecx, 2			; * 4 ==	0-28
		neg	ecx
		add	ecx, 28			; 28 .... 0
		mov	DestBitStart, ecx	; (28 for the 0th bit!)

		mov	eax, pSplashDIB
		add	eax, SIZEOF BITMAPINFOHEADER + 16*4 + ((SPLASH_HEIGHT-1) * DEST_BITMAP_BYTE_WIDTH)
		and	edx, 0FFFFFFF8h
		shr	edx, 1			; get the byte offset
		add	eax, edx
		mov	DestLineStart, eax

		; let's figure out the vertical
		mov	eax, VertPosition[esi]	; get the current Horz
		shr	eax, 16			; discard the fractional
		movsx	eax, ax			; and	sign-extend it
		.IF (sdword ptr eax >= 0)
			; our X coord is positive, so no top	clipping
			mov	ebx, eax
			imul	ebx, DEST_BITMAP_BYTE_WIDTH
			sub	DestLineStart, ebx
			mov	ebx, SPLASH_HEIGHT
			sub	ebx, eax
			mov	edx, yheight[edi]
			.IF (ebx > edx)
				mov	ebx, edx
			.ENDIF
		.ELSE
			; we	have a negative	Y, so some of our top is clipped
			mov	ebx, yheight[edi]
			add	ebx, eax	; EBX has the new line count
			imul	eax, SOURCE_BITMAP_BYTE_WIDTH
			add	SourceLineStart,	eax
		.ENDIF
		mov	LineCount, ebx
		call	FastTransparentBlt
		add	esi, SIZEOF DWORD
	.UNTIL	(esi >=	NUMBER_OF_FLOATERS * SIZEOF DWORD)
	ret
;-------------------------------------------------------------------------------
BlitTheObjects	ENDP


;+-----------------------------------------------------------------------------+
;|			     FAST TRANSPARENT BLT			       |
;|-----------------------------------------------------------------------------|
;|  This performs an ULTRA FAST	transparent BitBlt of one DIB into another.    |
;|-----------------------------------------------------------------------------|
;|  EAX	is the working scratch register					       |
;|  EBX	contains the destination bitmap	data for 8 pixels		       |
;|  ECX	has the	shift over count					       |
;|  EDX	has the	4-bit pixel mask					       |
;|  EBP	has the	1-bit pixel mask					       |
;|  ESI	has the	source bitmap dword pointer				       |
;|  EDI	has the	dest bitmap dword pointer				       |
;+-----------------------------------------------------------------------------+
FastTransparentBlt	PROC	USES esi edi ebp ebx
;-------------------------------------------------------------------------------
LinePrep:movmov	PixelsToBlt, eax, PixelsPerLine
	mov	esi, SourceLineStart
	mov	edi, DestLineStart
	mov	ebp, SourceBitStart
	mov	ecx, DestBitStart
	mov	edx, FourBitMasks[ecx]
	mov	ebx, [edi]		; get a dword set of bitmaps from the dest
	;컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴
;	bswap	ebx			; de-scramble the nibbles!
	ror	ebx, 8	; [dabc]
	swap	bh, bl	; [dacb]
	ror	ebx, 16	; [cbda]
	swap	bh, bl	; [cbad]
	ror	ebx, 8	; [dcba]
;-------------------------------------------------------------------------------
TstPixel:.IF ([esi] & ebp)		; see if this source-bit is	set
		mov	eax, ebx	; get the destination bitmap dword
		and	eax, edx	; mask out the 4-bit pixel we want
		not	edx		; invert the	mask to	zero the dest pixel
		and	ebx, edx	; zero the dest for eventual or-ing
		not	edx		; return the	mask to	it's positive state
		shr	eax, cl		; bring the pixel down to bit 0
		mov	eax, xform[eax*4]	; perform the transparent transformation
		shl	eax, cl		; move the new pixel back up where it goes
		or	ebx, eax	;	and OR it back into the	DIB
	.ENDIF
	rcr	ebp, 1			;	setup for the next source pixel
	.IF (carry?)			; if we're done, get the next word
		rcr	ebp, 1		;	set the	low-order bit
		add	esi, 4		;	bump to	the next source	dword
	.ENDIF
	ror	edx, 4			;	move the mask around four bits
	sub	ecx, 4			;	decrease the shift-over	amount by 4
	.IF (carry?)			; if we're done	with this pixel
		;컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴
;		bswap	ebx		; re-scramble the nibbles
		ror	ebx, 8	; [dabc]
		swap	bh, bl	;	[dacb]
		ror	ebx, 16	;	[cbda]
		swap	bh, bl	;	[cbad]
		ror	ebx, 8	; [dcba]
		;컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴
		mov	[edi], ebx	; replace the modified	dest bitmap dword
		add	edi, 4		;	bump up	to the next dest dword
		mov	ebx, [edi]	; get the next	dest bitmap dword
		;컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴
;		bswap	ebx		; de-scramble the nibbles
		ror	ebx, 8	; [dabc]
		swap	bh, bl	;	[dacb]
		ror	ebx, 16	;	[cbda]
		swap	bh, bl	;	[cbad]
		ror	ebx, 8	; [dcba]
		;컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴
		mov	ecx, 28		; and reset the shift-over amount
	.ENDIF
	dec	PixelsToBlt
	jnz	TstPixel
	;--------------------------------------------------------------------------
;	bswap	ebx			; re-scramble the nibbles
	ror	ebx, 8	; [dabc]
	swap	bh, bl	; [dacb]
	ror	ebx, 16	; [cbda]
	swap	bh, bl	; [cbad]
	ror	ebx, 8	; [dcba]
	;컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴
	mov	[edi], ebx		; replace the modified	dest bitmap dword
	;--------------------------------------------------------------------------
	sub	SourceLineStart, SOURCE_BITMAP_BYTE_WIDTH
	sub	DestLineStart, DEST_BITMAP_BYTE_WIDTH
	dec	LineCount		; see if we're all done	or not
	jnz	LinePrep
	;--------------------------------------------------------------------------
	ret
;-------------------------------------------------------------------------------
FastTransparentBlt	ENDP


;+-----------------------------------------------------------------------------+
;|			      SET RICH EDIT TEXT			       |
;+-----------------------------------------------------------------------------+
SetRichEditText	PROC USES esi edi ebx, phRichEditControl:LPHWND, pszSection:LPSTR
		LOCAL	HeaderLength:DWORD, BodyStart:DWORD, BodyLength:DWORD
;-------------------------------------------------------------------------------
	; see whether we've already set THIS text into	THIS control ...
	mov	eax, pszSection		;	get the	section	we're setting
	mov	ebx, phRichEditControl	; and address of the control's handle
	cmp	eax, [ebx+4]		; see if we've already set it
	je	Exit			; YEP!  so we're all	done!
	;--------------------------------------------------------------------------
	mov	[ebx+4], eax		; nope, so show it set now!

	; now check to	see if we're pointing to a NULL	string ??
	.IF (!byte ptr	[eax])
		mov	esi, eax	; setup the stream to NULL
		mov	edi, 1		;	set the	length to 1
		jmp	SetIt
	.ENDIF

	; now search to the first '[' to size the first (header) section
	mov	al, '['			; search	for the	first '[' bracket
	mov	edi, pRTF_Data
	mov	ecx, pRTF_Data+4
	repne scasb			; and scan past the rtf header
	mov	eax, edi
	sub	eax, pRTF_Data
	dec	eax
	mov	HeaderLength, eax	; establish the header's	length

	; now scan down to the	opening	of the proper section
	mov	edi, pRTF_Data		; restart at the top of the file
	mov	ecx, pRTF_Data+4
NextOne:.REPEAT
		mov	al, '['		; we're looking for a new section
		repne	scasb		; look for the next '['
		jne	Exit		; if we didn't find	one ...	abort
		mov	esi, ecx	; save our counter around the call
		invoke	lstrcmp, edi, pszSection
		mov	ecx, esi	; restore our counter after the call
	.UNTIL	(!eax)			; keep	looking	'till we find it

	; now find the	start of the NEXT line after the section header
	mov	al, LF			;	look for line feeds
	repne scasb			; scan for next line start
	jne	Exit			; abort if we didn't have one!
	mov	BodyStart,	edi		; mark the	start of the body text

	; now find the	start of the NEXT section (and the end of this one)
	mov	al, '['			; we're looking for a new section
	repne scasb			; look for the next '['
	jne	Exit			; if we didn't find	one ...	abort

	; now scan BACK to the	end of the PRIOR line
	mov	al, LF			;	scan backward past line	start
	mov	ecx, -1			; set the scan lenght to	infinity
	std				; set direction	to backwards
	repne scasb			; and land on the CR above
	cld				; restore standard forward direction
	sub	edi, BodyStart		; compute the Body's length
	mov	BodyLength, edi
	add	edi, HeaderLength	; get the total allocation needed
	inc	edi			; and add ONE more byte for '}'

	; allocate a block of mem for stream assembly
	invoke	GlobalAlloc, GPTR, edi
	mov	esi, eax		; esi will be our stream	pointer

	; fill	the allocation with the	header and the section's body
	invoke	MoveMemory, esi, pRTF_Data, HeaderLength
	mov	eax, esi
	add	eax, HeaderLength
	invoke	MoveMemory, eax, BodyStart, BodyLength
	mov	eax, esi
	add	eax, HeaderLength
	add	eax, BodyLength
	invoke	MoveMemory, eax, ADDR CloseCurly, 1

	; setup the stream parameters and initiate the	stream
SetIt:	mov	StreamPointer, esi	;	setup the start	of the stream
	add	edi, esi		; add the length	to the start
	mov	StreamEnd,	edi		; and set the stream's end	point
	mov	eax, phRichEditControl
	invoke	SendMessage, dword ptr [eax], EM_STREAMIN, SF_RTF or SFF_PLAINRTF, ADDR	RichEditStreamIn
	mov	eax, phRichEditControl
	invoke	SetWindowPos, dword ptr	[eax], HWND_TOP, 0,0,0,0, SWP_FRAMECHANGED or SWP_NOMOVE or SWP_NOSIZE or SWP_NOZORDER
	; release the temporary stream	assembly allocation
	.IF (byte ptr [esi])		; if we were not	pointing to a NULL
		invoke	GlobalFree, esi
	.ENDIF
Exit:	ret
;-------------------------------------------------------------------------------
SetRichEditText	ENDP


;+-----------------------------------------------------------------------------+
;|			  INITIALIZE WIZARD CONTROLS			       |
;|-----------------------------------------------------------------------------|
;|	  This presets and preloads the	Wizard's Child Controls		       |
;+-----------------------------------------------------------------------------+
InitializeWizardControls	PROC USES esi edi
				LOCAL	TabItem:TC_ITEM
;-------------------------------------------------------------------------------
	; prep	the RichEdit controls background colors	...
	mov	esi, WindowFillColor
	invoke	SendMessage, hRichEdit,	EM_SETBKGNDCOLOR, FALSE, esi
	invoke	SendMessage, hRichVersion, EM_SETBKGNDCOLOR, FALSE, esi
	invoke	SendMessage, hPPAVersion, EM_SETBKGNDCOLOR, FALSE, esi
	invoke	SendMessage, hTabText, EM_SETBKGNDCOLOR, FALSE,	esi
	invoke	SendMessage, hQuoteEdit, EM_SETBKGNDCOLOR, FALSE, esi
	; set the static text for the main RichEdit control
	invoke	SetRichEditText, ADDR hRichEdit, ADDR szInstructions

	; set the static text for the ASPI version problem control
	.IF (BadASPIDrivers)	;	if drivers are missing or non-functional
		invoke	SetRichEditText, ADDR hRichVersion, ADDR szNoASPI
	.ELSEIF (ErrorMode)
		invoke	SetRichEditText, ADDR hRichVersion, ADDR szASPITrouble
	.ENDIF
	; set the static text for the ASPI version problem control
	invoke	SetRichEditText, ADDR hPPAVersion, ADDR	szPPAVersion
	; set the default text	for the	action page RichEdit
	invoke	SetRichEditText, ADDR hTabText,	ADDR szNotRunning
	; set the static text for the Iomega Quote RichEdit control
	invoke	SetRichEditText, ADDR hQuoteEdit, ADDR szIomegaQuote
	; setup the tabs for the ACTION page
	invoke	InitCommonControls	; initialize	the common controls

	; loadup the tab text
	varzero	TabItem
	mov	TabItem._mask, TCIF_TEXT
	mov	TabItem.iImage, -1
	mov	TabItem.pszText, OFFSET szActionTabOne
	invoke	SendMessage, hActionTabs, TCM_INSERTITEM, 0, ADDR TabItem
	mov	TabItem.pszText, OFFSET szActionTabTwo
	invoke	SendMessage, hActionTabs, TCM_INSERTITEM, 1, ADDR TabItem
	; set the tab width
	invoke	SendMessage, hActionTabs, TCM_SETITEMSIZE, 0, 143
	; set the tab font
	invoke	SendMessage, hActionTabs, WM_SETFONT, hDialogTextFont, NULL

	; setup the proper URL's for the PPA Driver updating
	.IF (WinNT)
		mov	esi, OFFSET szFullWinNTText
		mov	edi, OFFSET szMinimumWinNTText
	.ELSE
		mov	esi, OFFSET szFullWin95Text
		mov	edi, OFFSET szMinimumWin95Text
	.ENDIF
	invoke	SetWindowText, hFullURL, esi
	invoke	SetWindowText, hMinURL,	edi

	ret
;-------------------------------------------------------------------------------
InitializeWizardControls	ENDP


;+-----------------------------------------------------------------------------+
;|				 STREAM	SOURCE				       |
;+-----------------------------------------------------------------------------+
StreamSource	PROC	dwCookie:DWORD, pBuff:LPBYTE,	cb:DWORD, pcb:LPDWORD
;-------------------------------------------------------------------------------
	mov	eax, StreamEnd		; get the end of the buffer
	mov	edx, StreamPointer	; get the current start
	sub	eax, edx		; calc the remaining amount
	mov	ecx, cb			; get the count to read
	.IF (eax > ecx)			; if	it's more than we have left
		mov	eax, ecx		; diminish it to the limit
	.ENDIF
	add	StreamPointer, eax	; bump the offset forward for next
	push	eax			; save the amount transferred
	invoke	MoveMemory, pBuff, edx,	eax ; move the data
	mov	eax, pcb
	pop	dword ptr [eax]
	zero	eax			; return zero to continue streaming
	ret
;-------------------------------------------------------------------------------
StreamSource	ENDP

IF DEVELOPMENT
;+-----------------------------------------------------------------------------+
;|				POST TO	SCREEN				       |
;+-----------------------------------------------------------------------------+
PostToScreen	PROC USES eax ebx esi edi,	PostVal:DWORD
		LOCAL	PostString[20]:CHAR, Extent:_SIZE
;-------------------------------------------------------------------------------
	mov	eax, PostVal
;	.IF (eax != LastPostedValue)
		mov	LastPostedValue, eax
		invoke	GetDC,	HWND_DESKTOP
		mov	edi, eax
		invoke	SelectObject, edi, hTitleFont
		invoke	SelectObject, edi, hBlackPen
		invoke	SelectObject, edi, hBlackBrush
		invoke	wsprintf, ADDR	PostString, ADDR szLongHexFormat, LastPostedValue
		mov	esi, eax
		invoke	TextOut, edi, 0, LastPostedTop, ADDR PostString, eax
		invoke	GetTextExtentPoint32, edi, ADDR PostString, esi, ADDR Extent
		mov	esi, Extent._cy
		add	esi, POSTING_SPACE
		add	LastPostedTop, esi
		invoke	GetSystemMetrics, SM_CYSCREEN
		sub	eax, esi
		.IF (LastPostedTop > eax)
			reset	LastPostedTop
		.ENDIF
		mov	eax, Extent._cx
		mov	ebx, Extent._cy
		add	ebx, LastPostedTop
		invoke	Rectangle, edi, 0, LastPostedTop, eax,	ebx
		invoke	ReleaseDC, HWND_DESKTOP, edi
;	.ENDIF
	ret
;-------------------------------------------------------------------------------
PostToScreen	ENDP
ENDIF


;+-----------------------------------------------------------------------------+
;|			   PROCESS PENDING MESSAGES			       |
;|-----------------------------------------------------------------------------|
;|    This allows all windows to receive and process any and all pending       |
;|    messages ... such	as old WM_PAINT's that they may	be holding ...	       |
;|-----------------------------------------------------------------------------|
;|    Note also	that this loop DELIBERATELY AVOIDS the removal of WM_QUIT      |
;|    messages,	since they should only be removed by the GetMessage call       |
;|    which returns FALSE when it encounters the WM_QUIT		       |
;+-----------------------------------------------------------------------------+
ProcessPendingMessages	PROC USES esi
			LOCAL	msg:MSG
;-------------------------------------------------------------------------------
; USB seems to generate	insane WM_TIMER	messages, so we	now stop the timer
; during data read/write processing.
	.REPEAT
		invoke	PeekMessage, ADDR msg,	NULL, 0, 0, PM_REMOVE
		.IF (eax)
			mov	esi, eax
			invoke	TranslateAccelerator,	hMainWnd, hAccel, ADDR msg
			.IF (!eax)	;	skip if	we already handled it ...
				invoke  TranslateMessage, ADDR msg
				invoke  DispatchMessage,  ADDR msg
			.ENDIF
			mov	eax, esi
		.ENDIF
	.UNTIL	(!eax)
	ret
;-------------------------------------------------------------------------------
ProcessPendingMessages	ENDP


;+-----------------------------------------------------------------------------+
;|			    UPDATE RUN TIME DISPLAY			       |
;+-----------------------------------------------------------------------------+
UpdateRunTimeDisplay	PROC USES edi
;-------------------------------------------------------------------------------
	invoke	GetDC, hTestMonitor
	mov	edi, eax
	invoke	PaintTestPhase,	edi
	invoke	PaintTheBarGraphs, edi,	TRUE
	invoke	PaintTestStatistics, edi, TRUE
	invoke	ReleaseDC, hTestMonitor, edi
	call	ProcessPendingMessages		;	paint all current status
	ret
;-------------------------------------------------------------------------------
UpdateRunTimeDisplay	ENDP


;+-----------------------------------------------------------------------------+
;|			     UPDATE CURRENT SECTOR			       |
;+-----------------------------------------------------------------------------+
UpdateCurrentSector	PROC USES edi
			LOCAL	szString[40]:CHAR
;-------------------------------------------------------------------------------
	invoke	GetDC, hTestMonitor
	mov	edi, eax
	invoke	wsprintf, ADDR szString, ADDR szCenteredDecimal, SingleTransferLBA
	invoke	PaintCenteredString,  edi,  76,	155, 126, 14, ADDR szString, TRUE
	invoke	ReleaseDC, hTestMonitor, edi
	ret
;-------------------------------------------------------------------------------
UpdateCurrentSector	ENDP


;+-----------------------------------------------------------------------------+
;|			    UPDATE RUN TIME DISPLAY			       |
;+-----------------------------------------------------------------------------+
UpdateRunPhaseDisplay	PROC USES	edi
;-------------------------------------------------------------------------------
	invoke	GetDC, hTestMonitor
	mov	edi, eax
	invoke	PaintTestPhase,	edi
	invoke	ReleaseDC, hTestMonitor, edi
	call	ProcessPendingMessages		;	paint all current status
	ret
;-------------------------------------------------------------------------------
UpdateRunPhaseDisplay	ENDP


;+-----------------------------------------------------------------------------+
;|			     PREVENT PROGRAM EXIT			       |
;+-----------------------------------------------------------------------------+
PreventProgramExit	PROC
;-------------------------------------------------------------------------------
	invoke	GetSystemMenu, hMainWnd, FALSE ; get sysmenu handle
	invoke	DeleteMenu, eax, SC_CLOSE, MF_BYCOMMAND
	invoke	SendMessage, hMainWnd, WM_NCPAINT, NULL, NULL
	invoke	SetWindowPos, hMainWnd,	0, 0,0,0,0,
		SWP_DRAWFRAME	or SWP_NOMOVE or SWP_NOSIZE or SWP_NOZORDER
	invoke	EnableWindow, hExitButton, FALSE
	ret
;-------------------------------------------------------------------------------
PreventProgramExit	ENDP


;+-----------------------------------------------------------------------------+
;|			      ALLOW PROGRAM EXIT			       |
;+-----------------------------------------------------------------------------+
AllowProgramExit	PROC
;-------------------------------------------------------------------------------
	invoke	GetSystemMenu, hMainWnd, FALSE ; get sysmenu handle
	invoke	AppendMenu, eax, MF_STRING, SC_CLOSE, ADDR szCloseCmd
	invoke	SendMessage, hMainWnd, WM_NCPAINT, NULL, NULL
	invoke	SetWindowPos, hMainWnd,	0, 0,0,0,0,
		SWP_DRAWFRAME	or SWP_NOMOVE or SWP_NOSIZE or SWP_NOZORDER
	invoke	EnableWindow, hExitButton, TRUE
	ret
;-------------------------------------------------------------------------------
AllowProgramExit	ENDP


;+-----------------------------------------------------------------------------+
;|			       COPY CURRENT PAGE			       |
;+-----------------------------------------------------------------------------+
CopyCurrentPage	PROC USES ebx esi edi
;-------------------------------------------------------------------------------
	mov	ebx, CurrentPage		; get the page we're seeing
	; if we're trying to copy the test results page ...
	.IF (ebx == PERFORM_TEST_PAGE)
		; see	whether	the test results are being viewed
		invoke	GetWindowLong,	hTestMonitor, GWL_ID
		.IF (eax == (INVISIBLE_PAGE SHL PAGE_SHIFT))
			; nope, so copy from	the Explantion Page instead
			inc	ebx
		.ENDIF
	.ENDIF
	mov	ebx, CopySourceHandles[ebx*8]	; and the RichEdit hWnd
	; okay	... now	if it's	a TEXT copy rather than	a bitmap ...
	; EBX has the HANDLE of the control to	copy from!
	.IF (ebx)
		; suppress showing the selection region
		invoke	SendMessage, ebx, EM_HIDESELECTION, TRUE, TRUE
		; select ALL of the text
		invoke	SendMessage, ebx, EM_SETSEL, 0, -1
		; copy it to the clipboard
		invoke	SendMessage, ebx, WM_COPY, 0, 0
	.ELSE	; no RichEdit control on	this page ... so we'll snap a
		; picture of the bitmap
		invoke	OpenClipboard,	hMainWnd
		.IF (eax)
			mov	ebx, eax	; ebx has the clipboard	handle
			invoke	EmptyClipboard	; free	up any previous	memory
			invoke	GetDC, hMainWnd		; pickup a source context
			mov	esi, eax
			invoke	CreateCompatibleDC, NULL
			mov	edi, eax		; and a destination context
			invoke	CreateCompatibleBitmap, esi, COPY_WIDTH, COPY_HEIGHT
			invoke	SelectObject,	edi, eax
			push	eax		; save DC's orig bitmap
			invoke	BitBlt, edi, 0,0, COPY_WIDTH,	COPY_HEIGHT, esi, COPY_LEFT, COPY_TOP, SRCCOPY
			pop	eax		; recover DC's orig	bitmap
			invoke	SelectObject,	edi, eax	; put it back and our new bitmap back
			mov	ebx, eax	; save our bitmap for handing over to clipboard
			invoke	DeleteDC, edi
			invoke	ReleaseDC, hMainWnd, esi
			invoke	SetClipboardData, CF_BITMAP, ebx
			invoke	CloseClipboard
		.ENDIF
	.ENDIF

	ret
;-------------------------------------------------------------------------------
CopyCurrentPage	ENDP


;+-----------------------------------------------------------------------------+
;|			     VERIFY FILE CHECKSUM			       |
;+-----------------------------------------------------------------------------+
VerifyFileChecksum	PROC	USES ebx esi edi
			LOCAL	FileName[MAX_PATH]:CHAR, BytesRead:DWORD
;-------------------------------------------------------------------------------
	; load	ourselves into a ram buffer ...
	invoke	GetModuleFileName, NULL, ADDR FileName,	MAX_PATH
	invoke	CreateFile, ADDR FileName, GENERIC_READ, FILE_SHARE_READ or FILE_SHARE_WRITE, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL or FILE_FLAG_SEQUENTIAL_SCAN, NULL
	mov	ebx, eax			; hold the file	handle
	invoke	GetFileSize, eax, NULL		; get the size
	add	eax, 3				; allocate read buffer
	and	eax, 0FFFFFFFCh			; an even dword size
	mov	edi, eax			; hold the size
	invoke	GlobalAlloc, GPTR, eax
	push	eax				; save for	buffer free
	mov	esi, eax			; hold the buffer's start
	invoke	ReadFile, ebx, esi, edi, ADDR BytesRead, NULL
	invoke	CloseHandle, ebx
	; we have the file in RAM ... now let's sum it	up!
	zero	ebx				; clear the sumation
	mov	ecx, edi
	shr	ecx, 2				; divide	it down	by four
	.REPEAT
		lodsd				; pickup a dword
		add	ebx, eax
	.UNTILCXZ
	pop	eax				; recover the buffer start
	invoke	GlobalFree, eax
	.IF (ebx)
		invoke	MessageBox, hMainWnd, ADDR szBadFileText, ADDR	szBadFileTitle,	MB_APPLMODAL or	MB_OK
		invoke	ExitProcess, -1		; exit with ERROR!
	.ENDIF
	ret
;-------------------------------------------------------------------------------
VerifyFileChecksum	ENDP


;+-----------------------------------------------------------------------------+
;|				  ErrorSound				       |
;+-----------------------------------------------------------------------------+
ErrorSound	PROC
;-------------------------------------------------------------------------------
	; see whether we have sounds enabled
	invoke	SendMessage, hSoundCheckbox, BM_GETCHECK, NULL,	NULL
	.IF (eax == BST_CHECKED)
		invoke	GetVersion	; find out what platform we're running ...
		.IF !(eax & 80000000h)
			invoke	Beep,	745, 13
		.ELSE
			mov	Period, 800
			mov	Duration, 10
			cli
			call	GetPhasorPosition	; set	initial	angle
			mov	PhasorPosition, ax
			.REPEAT
				mov	ax, Period	; update the	phasor angle
				sub	PhasorPosition,	ax
				.REPEAT
					call	GetPhasorPosition
					sub	ax, PhasorPosition
					.IF (carry?)
						neg	ax
					.ENDIF
				.UNTIL (ax < 100)
				call	ToggleSpeaker
				dec	Duration
			.UNTIL (zero?)
			sti
		.ENDIF
	.ENDIF
;-------------------------------------------------------------------------------
	ret

GetPhasorPosition:
;-------------------------------------------------------------------------------
	mov	al, 00000100B	; latch timer_0
	out	TIMER_MODE, al
	jmp	$+2
	in	al,	TIMER_0	; LSB
	jmp	$+2
	xchg	ah, al
	in	al,	TIMER_0	; MSB
	jmp	$+2
	xchg	ah, al		;	finally	form the count
;-------------------------------------------------------------------------------
	LocalReturn

ToggleSpeaker:
;-------------------------------------------------------------------------------
	in	al,	SPEAKER_GATE
	jmp	$+2
	and	al, NOT OSCILLATOR_GATE_BIT	; kill	OSC Input
	xor	al, SPEAKER_DATA_BIT		; invert Spkr Data
	out	SPEAKER_GATE, al
	jmp	$+2
;-------------------------------------------------------------------------------
	LocalReturn

;-------------------------------------------------------------------------------
ErrorSound	ENDP



;-------------------------------------------------------------------------------
include	aspi.asm

;-/////////////////////////////////////////////////////////////////////////////-
END	Start
;-/////////////////////////////////////////////////////////////////////////////-
