;嬪様様様様様様様様様様様様様様様様様様様様様様様様様様様様様様様様様様様様邑
;�  macros.inc              by: Steven M. Gibson         created: 04/03/98  �
;塒様様様様様様様様様様様様様様様様様様様様様様様様様様様様様様様様様様様様余
pagectrl = 0	; initialize static globals used by fancy macros
crntpage = -1


varzero		MACRO	p1:REQ
;陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳
		invoke	FillMemory, ADDR p1, SIZEOF p1, NULL
;陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳
		ENDM

ZeroASPI	MACRO
;陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳
		invoke	ZeroMemory, ADDR ASPI_CmdBlock, SIZEOF ASPI_CmdBlock
;陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳
		ENDM

;様様様様様様様様様様様様様様様 PAGECTL macro 様様様様様様様様様様様様様様様�
; 		pagectl PAGE_ID, STYLE, (CTRL_CONST), (hWnd)
;陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳
pagectl	MACRO	p1:REQ, p2:REQ, p3, p4:=<NULL>
;陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳
	IF (p1 EQ crntpage)
		pagectrl = pagectrl + 1
	ELSE
		pagectrl = 0
		crntpage = p1
	ENDIF

	DWORD	hMainWnd, ((p2 SHL 16) + (p1 shl 8) + pagectrl), p4

	IFNB <p3>
p3	equ	((p1 shl 8) + pagectrl)
	ENDIF
;陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳
		ENDM


;陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳� ERRCODE 陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳
errcode	MACRO	p1:REQ, p2:REQ
;陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳
.RADIX 16
	dd	p1
.RADIX 10
	db	p2,0
;陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳
	ENDM

