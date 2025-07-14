!to "sidasm.prg", cbm
!cpu 6510
!ct pet
; https://www.lemon64.com/forum/viewtopic.php?t=61054
SIDLoad = $5000          ; found in the 125th/126th byte of the PSID header (7C bytes in)
;!!!NOTE!!! this is a C64 load address and therefore in low/high order.
SIDInit = $b25f          ; found in the 11th/12th byte of the PSID header (10 bytes in)
SIDUpdate = $4043        ; found in the 13th/14th byte of the PSID header (12 bytes in)
PSIDHeaderSize = $7c     ; v2 ie size found at the 8th byte of the PSID header (7 bytes in)

TrackIndex = 0           ; check 15th/16th byte to see how many tracks are
                         ; in the file and hence how high you can
                         ; make this value.

ZPProcessorPortDDR		    = $00
ProcessorPortDDRDefault	  = %00101111
ZPProcessorPort			      = $01
ProcessorPortAllRAMWithIO = %00100101

CIA1InterruptControl		= $dc0d
CIA2InterruptControl		= $dd0d

VIC2ScreenControlV  = $d011
VIC2Raster          = $d012
VIC2InteruptStatus  = $d019
VIC2InteruptControl = $d01a
VIC2BorderColour    = $d020
VIC2ScreenColour    = $d021

VIC2Colour_White = 1
VIC2Colour_LBlue = $e

KERNALIRQServiceRoutineLo = $fffe
KERNALIRQServiceRoutineHi = $ffff

!macro AckRasterIRQ {
	lda #1
	sta VIC2InteruptStatus
}

!macro AckAllIRQs {
	lda CIA1InterruptControl
	lda CIA2InterruptControl
	lda #$ff
	sta VIC2InteruptStatus
}

*= $0801
!byte $0b,$08,$01,$00,$9e		; Line 1 SYS2061
!convtab pet
!tx "2061"
!byte $00,$00,$00

!zn
	lda #ProcessorPortAllRAMWithIO
	sei
	cld
	ldx #ProcessorPortDDRDefault
	stx ZPProcessorPortDDR
	sta ZPProcessorPort
	lda #$7f
	sta CIA1InterruptControl
	sta CIA2InterruptControl
 	and VIC2ScreenControlV
	sta VIC2ScreenControlV
	+AckAllIRQs

	lda #<IrqTopOfScreen
	ldx #>IrqTopOfScreen
	sta KERNALIRQServiceRoutineLo
	stx KERNALIRQServiceRoutineHi
	lda #1
	sta VIC2InteruptControl

	lda #TrackIndex
	jsr SIDInit

	lda #$5f		; mid screen
	sta VIC2Raster
	lda #%00000001
	sta VIC2InteruptControl

	+AckAllIRQs

	cli

.Forever
	jmp .Forever

IrqTopOfScreen
	pha
	txa
	pha
	tya
	pha

	lda #VIC2Colour_White
	sta VIC2BorderColour

	jsr SIDUpdate

	lda #VIC2Colour_LBlue
	sta VIC2BorderColour

	+AckRasterIRQ

	pla
	tay
	pla
	tax
	pla
	rti

*=SIDLoad-PSIDHeaderSize-2  ;-2 to skip the load address
!bin "assets/Commando.sid"
