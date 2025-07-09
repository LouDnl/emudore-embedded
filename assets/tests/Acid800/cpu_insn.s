; Altirra Acid800 test suite
; Copyright (C) 2010 Avery Lee, All Rights Reserved.
;
; Permission is hereby granted, free of charge, to any person obtaining a copy
; of this software and associated documentation files (the "Software"), to deal
; in the Software without restriction, including without limitation the rights
; to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
; copies of the Software, and to permit persons to whom the Software is
; furnished to do so, subject to the following conditions:
;
; The above copyright notice and this permission notice shall be included in
; all copies or substantial portions of the Software.
;
; THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
; IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
; FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
; AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
; LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
; OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
; SOFTWARE. 

; initialized zp
		zpoffs =		$0080
dc_con	= zpoffs + $00  ; !byte		$00			;decompression control byte
dc_in0  = zpoffs + $01  ; !byte		$60			;instruction
dc_in1  = zpoffs + $02  ; !byte		$60
dc_in2  = zpoffs + $03  ; !byte		$60
		        ; !byte		$60
dc_a1   = zpoffs + $05  ; !byte		0			;input data
dc_x1   = zpoffs + $06  ; !byte		0
dc_y1   = zpoffs + $07  ; !byte		0
dc_p1   = zpoffs + $08  ; !byte		0
dc_m1   = zpoffs + $09  ; !byte		0
dc_a2   = zpoffs + $0a  ; !byte		0			;output data
dc_x2   = zpoffs + $0b  ; !byte		0
dc_y2   = zpoffs + $0c  ; !byte		0
dc_p2   = zpoffs + $0d  ; !byte		0
dc_m2   = zpoffs + $0e  ; !byte		0

rand0   = zpoffs + $0f  ; !byte		$78			;32-bit LFSR
rand1   = zpoffs + $10  ; !byte		$56
rand2   = zpoffs + $11  ; !byte		$34
rand3   = zpoffs + $12  ; !byte		$12
randt   = zpoffs + $13  ; !byte		0

                !src "common.s"

zpinit
_dc_con  !byte           $00                     ;decompression control byte
_dc_in0  !byte           $60                     ;instruction
_dc_in1  !byte           $60
_dc_in2  !byte           $60
         !byte           $60

_dc_a1   !byte           0                       ;input data
_dc_x1   !byte           0
_dc_y1   !byte           0
_dc_p1   !byte           0
_dc_m1   !byte           0
_dc_a2   !byte           0                       ;output data
_dc_x2   !byte           0
_dc_y2   !byte           0
_dc_p2   !byte           0
_dc_m2   !byte           0

_rand0   !byte           $78                     ;32-bit LFSR
_rand1   !byte           $56
_rand2   !byte           $34
_rand3   !byte           $12
_randt   !byte           0
zpend
                * =             $2000
main:
                ldy             #>testname
                lda             #<testname
                jsr             _testInit

                sei

                ldx #0
lp:
                lda zpinit,x
                sta zpoffs,x
                inx
                cpx #zpend-zpinit
                bne lp

                lda     #<test_start
                sta     a0
                lda     #>test_start
                sta     a0+1

loop_start:
		;load control byte
		ldy		#0
		lda		(a0),y
		iny
		sta             dc_con
		
		;check if we should reload the opcode
		asl		dc_con
		bcc		same_opcode
		
		lda		(a0),y ; new opcode
		iny
		sta             dc_in0

		lda		#$60
		sta		dc_in1
		sta		dc_in2
		
same_opcode:
		;update instruction bytes if needed
		asl		dc_con
		;scc
		bcc +
		lda	(a0),y
		iny
		sta             dc_in1
+
		asl		dc_con
		;scc
                bcc +
		lda	(a0),y
		iny
		sta             dc_in2
+
		;compute implicit A1
		jsr		detrand2
		sta		dc_a1
		
		;decompress X1 and Y1; note that X1 = ~Y1
		lda		(a0),y
		iny
		sta		dc_y1
		eor		#$ff
		sta		dc_x1
		
		;compute implicit P1 and M1
		jsr		detrand2
		ora		#$30
		and		#$f7
		sta		dc_p1
		
		jsr		detrand2
		sta		dc_m1
				
		;decompress A2, X2, Y2, P2, and M2
		asl		dc_con
		lda		dc_a1	
		;scc
                bcc +
		lda	(a0),y
		iny
+
		sta		dc_a2

		asl		dc_con
		lda		dc_x1
		;scc
                bcc +
		lda	(a0),y
		iny
+
		sta		dc_x2

		asl		dc_con
		lda		dc_y1	
		;scc
                bcc +
		lda	(a0),y
		iny
+
		sta		dc_y2

		asl		dc_con
		lda		dc_p1	
		;scc
                bcc +
		lda	(a0),y
		iny
+
		sta		dc_p2

		asl		dc_con
		lda		dc_m1	
		;scc
                bcc +
		lda	(a0),y
		iny
+
		sta		dc_m2
		
		;bump test pointer
		tya
		clc
		adc		a0
		sta		a0
		;scc
                bcc +
		inc	a0+1
+
		;setup temp registers
		;+mwa            d5, a1
		;+mwa            d4, a2
                lda #>d5
                sta a1+1
                lda #<d5
                sta a1
                lda #>d4
                sta a2+1
                lda #<d4
                sta a2

		;stash P
		lda		dc_p1
		pha
		
		;load d5
		lda	dc_m1
		sta     d5

		;load A, X, Y
		lda		dc_a1
		ldx		dc_x1
		ldy		dc_y1
		
		;load P
		plp
;inc $d020
;jmp * -3
		;execute insn
		jsr		dc_in0

		;store registers
		sta		d1
		stx		d2
		sty		d3
		php
		pla
		ora		#$30
		sta		d4
		
		;reset flags
		cli
		cld
!if (1 = 0) {
                lda dc_a1
                sta $0400+(4*40)+1
                lda dc_x1
                sta $0400+(4*40)+2
                lda dc_y1
                sta $0400+(4*40)+3
                lda dc_p1
                sta $0400+(4*40)+4
                lda dc_m1
                sta $0400+(4*40)+5

                lda d0
                sta $0400+(5*40)+0
                lda d1
                sta $0400+(5*40)+1
                lda d2
                sta $0400+(5*40)+2
                lda d3
                sta $0400+(5*40)+3
                lda d4
                sta $0400+(5*40)+4
                lda d5
                sta $0400+(5*40)+5

                lda dc_a2
                sta $0400+(6*40)+1
                lda dc_x2
                sta $0400+(6*40)+2
                lda dc_y2
                sta $0400+(6*40)+3
                lda dc_p2
                sta $0400+(6*40)+4
                lda dc_m2
                sta $0400+(6*40)+5
;jmp *
}
		;compare registers
		lda		dc_a2
		cmp		d1
		bne		fail
		
		cpx		dc_x2
		bne		fail
		
		cpy		dc_y2
		bne		fail
		
		lda		dc_p2
		cmp		d4
		bne		fail
		
		lda		dc_m2
		cmp		d5
		bne		fail
nofail:
		;go another round
		lda		a0		
		cmp		#<test_end
		bne		loop_end
		lda		a0+1
		cmp		#>test_end
		beq		loop_exit
loop_end:

;donecnt:        ldx     #0
;                inc $0400+(7*40),x
;                inc donecnt+1
;                inc $0400+(24*40)+39
		jmp		loop_start
loop_exit:
		jmp		_testPassed

fail:
		cld
		cli
;failcnt:        ldx     #0
;		ldx failcnt+1
;                inc $0400+(14*40),x
;                inc failcnt+1

                jsr             _printfinit
                jsr             _imprintf
		     ;1234567890123456789012345678901234567890
		!scr "fail:  a=00 x=00 y=00 p=00 m=00         ",0

		ldx		#5
copyloop:
		lda		dc_a1-1,x
		sta		d0,x
		dex
		bne		copyloop
		jsr		_imprintf
		!scr "input: a=00 x=00 y=00 p=00 m=00         ",0

;               jmp nofail

		;+mva		dc_in0, d1
		;+mva		dc_in1, d2
		;+mva		dc_in2, d3
		
		ldy		#>failmsg
		lda		#<failmsg
		jmp		_testFailed2
failmsg:
		!scr "  insn: 00 00 00                        ",0

;               jmp nofail

testname:
                !scr "cpu: basic instructions",0

;============================================================================
; This LFSR generator must match detrand2() in cputest.cpp.
;
detrand2:
		lda		rand0
		pha
		
		;shift LFSR
		;+mva		rand1, rand0
		;+mva		rand2, rand1
		;+mva		rand3, rand2
                lda rand1
                sta rand0
                lda rand2
                sta rand1
                lda rand3
                sta rand2

		;bit 31
		pla
		sta		rand3
		pha
		
		;bit 29 (shr 2)
		ldx		#0
		stx             randt
		lsr
		ror		randt	
		lsr
		ror		randt
		tax
		eor		rand3
		sta		rand3
		lda		rand2
		eor		randt
		sta		rand2
		txa
		
		;bit 25 (shr +4)
		lsr
		ror		randt
		lsr
		ror		randt
		lsr
		ror		randt
		lsr
		ror		randt
		tax
		eor		rand3
		sta		rand3
		lda		rand2
		eor		randt
		sta		rand2
		txa
		
		;bit 24 (shr +1)
		lsr
		ror		randt
		eor		rand3
		sta		rand3
		lda		rand2
		eor		randt
		sta		rand2
		
		pla
		rts

;============================================================================
test_start:
		;This test data is produced by cputest.cpp. The format is as follows:
		;
		; * Control byte
		;	D7=1	Opcode byte follows; reset op1 and op2 bytes to $60 (RTS)
		;	D6=1	New operand 1 byte follows
		;	D5=1	New operand 2 byte follows
		;	D4=1	New A result byte follows
		;	D3=1	New X result byte follows
		;	D2=1	New Y result byte follows
		;	D1=1	New P result byte follows
		;	D0=1	New M result byte follows
		; * [Opcode byte]
		; * [Operand 1 byte]
		; * [Operand 2 byte]
		; * Y input byte
		; * [A result byte]
		; * [X result byte]
		; * [Y result byte]
		; * [P result byte]
		; * [M result byte]
		;
		; A, P, and M input bytes are implicit based on a 32-bit LFSR with
		; defined seed (see detrand2). The Y input byte is always included,
		; and the X input byte is the complement of the Y input byte.

		; $01  ORA (zp,x)
		!byte	$D2,$01,$C3,$00,$7C,$74 ;Input: A=$78 Y=$00 P=$76 M=$34  Output: A=$7C X=$FF Y=$00 P=$74 M=$34
		!byte	$52,$E6,$23,$FA,$B1 ;Input: A=$02 Y=$23 P=$33 M=$FA  Output: A=$FA X=$DC Y=$23 P=$B1 M=$FA
		!byte	$50,$47,$84,$BF ;Input: A=$B5 Y=$84 P=$B0 M=$8B  Output: A=$BF X=$7B Y=$84 P=$B0 M=$8B
		!byte	$52,$2F,$6C,$7B,$71 ;Input: A=$38 Y=$6C P=$73 M=$7B  Output: A=$7B X=$93 Y=$6C P=$71 M=$7B
		!byte	$50,$71,$AE,$BA ;Input: A=$3A Y=$AE P=$B4 M=$AA  Output: A=$BA X=$51 Y=$AE P=$B4 M=$AA
		!byte	$52,$53,$90,$F8,$B4 ;Input: A=$78 Y=$90 P=$34 M=$D0  Output: A=$F8 X=$6F Y=$90 P=$B4 M=$D0
		!byte	$52,$B4,$F1,$FB,$B5 ;Input: A=$93 Y=$F1 P=$37 M=$FB  Output: A=$FB X=$0E Y=$F1 P=$B5 M=$FB
		!byte	$52,$7E,$BB,$FE,$B4 ;Input: A=$4C Y=$BB P=$B6 M=$FE  Output: A=$FE X=$44 Y=$BB P=$B4 M=$FE

		; $05  ORA zp
		!byte	$D2,$05,$CD,$EB,$7F,$30 ;Input: A=$6F Y=$EB P=$B2 M=$34  Output: A=$7F X=$14 Y=$EB P=$30 M=$34
		!byte	$10,$A6,$7B ;Input: A=$61 Y=$A6 P=$31 M=$3B  Output: A=$7B X=$59 Y=$A6 P=$31 M=$3B
		!byte	$12,$3C,$7F,$34 ;Input: A=$3F Y=$3C P=$36 M=$51  Output: A=$7F X=$C3 Y=$3C P=$34 M=$51
		!byte	$10,$0C,$FF ;Input: A=$77 Y=$0C P=$B4 M=$AB  Output: A=$FF X=$F3 Y=$0C P=$B4 M=$AB
		!byte	$12,$99,$BE,$B1 ;Input: A=$B6 Y=$99 P=$33 M=$88  Output: A=$BE X=$66 Y=$99 P=$B1 M=$88
		!byte	$12,$5E,$B7,$B0 ;Input: A=$B6 Y=$5E P=$32 M=$27  Output: A=$B7 X=$A1 Y=$5E P=$B0 M=$27
		!byte	$10,$1C,$BD ;Input: A=$9D Y=$1C P=$B4 M=$2C  Output: A=$BD X=$E3 Y=$1C P=$B4 M=$2C
		!byte	$10,$B7,$72 ;Input: A=$20 Y=$B7 P=$31 M=$72  Output: A=$72 X=$48 Y=$B7 P=$31 M=$72

		; $06  ASL zp
		!byte	$C3,$06,$CD,$DE,$75,$5A ;Input: A=$E7 Y=$DE P=$76 M=$AD  Output: A=$E7 X=$21 Y=$DE P=$75 M=$5A
		!byte	$03,$12,$B1,$B4 ;Input: A=$FD Y=$12 P=$30 M=$DA  Output: A=$FD X=$ED Y=$12 P=$B1 M=$B4
		!byte	$03,$C8,$F0,$D4 ;Input: A=$CB Y=$C8 P=$70 M=$6A  Output: A=$CB X=$37 Y=$C8 P=$F0 M=$D4
		!byte	$03,$BB,$31,$72 ;Input: A=$94 Y=$BB P=$33 M=$B9  Output: A=$94 X=$44 Y=$BB P=$31 M=$72
		!byte	$03,$A6,$B1,$96 ;Input: A=$09 Y=$A6 P=$30 M=$CB  Output: A=$09 X=$59 Y=$A6 P=$B1 M=$96
		!byte	$03,$03,$F0,$B0 ;Input: A=$E2 Y=$03 P=$F3 M=$58  Output: A=$E2 X=$FC Y=$03 P=$F0 M=$B0
		!byte	$03,$7D,$30,$2E ;Input: A=$37 Y=$7D P=$32 M=$17  Output: A=$37 X=$82 Y=$7D P=$30 M=$2E
		!byte	$03,$38,$35,$60 ;Input: A=$3D Y=$38 P=$B6 M=$B0  Output: A=$3D X=$C7 Y=$38 P=$35 M=$60

		; $09  ORA #imm
		!byte	$D2,$09,$5D,$1F,$DD,$B4 ;Input: A=$DC Y=$1F P=$36 M=$38  Output: A=$DD X=$E0 Y=$1F P=$B4 M=$38
		!byte	$52,$CB,$D4,$DF,$F5 ;Input: A=$57 Y=$D4 P=$75 M=$97  Output: A=$DF X=$2B Y=$D4 P=$F5 M=$97
		!byte	$52,$96,$FC,$96,$B5 ;Input: A=$04 Y=$FC P=$35 M=$D1  Output: A=$96 X=$03 Y=$FC P=$B5 M=$D1
		!byte	$52,$45,$F5,$ED,$F1 ;Input: A=$A9 Y=$F5 P=$73 M=$A8  Output: A=$ED X=$0A Y=$F5 P=$F1 M=$A8
		!byte	$52,$13,$3B,$53,$30 ;Input: A=$51 Y=$3B P=$32 M=$80  Output: A=$53 X=$C4 Y=$3B P=$30 M=$80
		!byte	$52,$89,$0D,$AF,$F0 ;Input: A=$27 Y=$0D P=$70 M=$34  Output: A=$AF X=$F2 Y=$0D P=$F0 M=$34
		!byte	$52,$1C,$0A,$BD,$F4 ;Input: A=$B1 Y=$0A P=$F6 M=$F7  Output: A=$BD X=$F5 Y=$0A P=$F4 M=$F7
		!byte	$52,$AE,$DB,$FF,$B0 ;Input: A=$DF Y=$DB P=$32 M=$31  Output: A=$FF X=$24 Y=$DB P=$B0 M=$31

		; $0A  ASL
		!byte	$92,$0A,$32,$94,$B5 ;Input: A=$CA Y=$32 P=$36 M=$FA  Output: A=$94 X=$CD Y=$32 P=$B5 M=$FA
		!byte	$12,$9A,$02,$74 ;Input: A=$01 Y=$9A P=$F6 M=$A7  Output: A=$02 X=$65 Y=$9A P=$74 M=$A7
		!byte	$12,$EE,$00,$37 ;Input: A=$80 Y=$EE P=$35 M=$ED  Output: A=$00 X=$11 Y=$EE P=$37 M=$ED
		!byte	$10,$78,$1A ;Input: A=$8D Y=$78 P=$35 M=$02  Output: A=$1A X=$87 Y=$78 P=$35 M=$02
		!byte	$12,$FD,$74,$71 ;Input: A=$BA Y=$FD P=$73 M=$BE  Output: A=$74 X=$02 Y=$FD P=$71 M=$BE
		!byte	$12,$49,$3C,$34 ;Input: A=$1E Y=$49 P=$35 M=$7E  Output: A=$3C X=$B6 Y=$49 P=$34 M=$7E
		!byte	$12,$F6,$AC,$B4 ;Input: A=$56 Y=$F6 P=$37 M=$9E  Output: A=$AC X=$09 Y=$F6 P=$B4 M=$9E
		!byte	$12,$7D,$28,$70 ;Input: A=$14 Y=$7D P=$F0 M=$D6  Output: A=$28 X=$82 Y=$7D P=$70 M=$D6

		; $0D  ORA abs
		!byte	$F2,$0D,$CD,$00,$DC,$FA,$B1 ;Input: A=$C2 Y=$DC P=$31 M=$BA  Output: A=$FA X=$23 Y=$DC P=$B1 M=$BA
		!byte	$02,$4F,$B4 ;Input: A=$ED Y=$4F P=$36 M=$21  Output: A=$ED X=$B0 Y=$4F P=$B4 M=$21
		!byte	$12,$F2,$BD,$B0 ;Input: A=$B9 Y=$F2 P=$30 M=$95  Output: A=$BD X=$0D Y=$F2 P=$B0 M=$95
		!byte	$02,$40,$F4 ;Input: A=$FF Y=$40 P=$74 M=$16  Output: A=$FF X=$BF Y=$40 P=$F4 M=$16
		!byte	$12,$D0,$F5,$B0 ;Input: A=$71 Y=$D0 P=$32 M=$B4  Output: A=$F5 X=$2F Y=$D0 P=$B0 M=$B4
		!byte	$12,$C4,$FD,$F0 ;Input: A=$75 Y=$C4 P=$70 M=$8C  Output: A=$FD X=$3B Y=$C4 P=$F0 M=$8C
		!byte	$12,$B7,$E5,$F1 ;Input: A=$E4 Y=$B7 P=$71 M=$45  Output: A=$E5 X=$48 Y=$B7 P=$F1 M=$45
		!byte	$02,$3B,$B1 ;Input: A=$F4 Y=$3B P=$31 M=$84  Output: A=$F4 X=$C4 Y=$3B P=$B1 M=$84

		; $0E  ASL abs
		!byte	$E3,$0E,$CD,$00,$22,$74,$76 ;Input: A=$6D Y=$22 P=$75 M=$3B  Output: A=$6D X=$DD Y=$22 P=$74 M=$76
		!byte	$03,$22,$30,$72 ;Input: A=$88 Y=$22 P=$31 M=$39  Output: A=$88 X=$DD Y=$22 P=$30 M=$72
		!byte	$03,$9D,$F5,$C2 ;Input: A=$05 Y=$9D P=$77 M=$E1  Output: A=$05 X=$62 Y=$9D P=$F5 M=$C2
		!byte	$03,$8B,$F4,$CE ;Input: A=$69 Y=$8B P=$F6 M=$67  Output: A=$69 X=$74 Y=$8B P=$F4 M=$CE
		!byte	$03,$DA,$F4,$CE ;Input: A=$ED Y=$DA P=$76 M=$67  Output: A=$ED X=$25 Y=$DA P=$F4 M=$CE
		!byte	$03,$CA,$B1,$D8 ;Input: A=$51 Y=$CA P=$30 M=$EC  Output: A=$51 X=$35 Y=$CA P=$B1 M=$D8
		!byte	$03,$02,$F4,$D0 ;Input: A=$D9 Y=$02 P=$74 M=$68  Output: A=$D9 X=$FD Y=$02 P=$F4 M=$D0
		!byte	$03,$72,$74,$48 ;Input: A=$43 Y=$72 P=$75 M=$24  Output: A=$43 X=$8D Y=$72 P=$74 M=$48

		; $11  ORA (zp),y
		!byte	$D2,$11,$C4,$01,$B9,$B4 ;Input: A=$39 Y=$01 P=$34 M=$B1  Output: A=$B9 X=$FE Y=$01 P=$B4 M=$B1
		!byte	$42,$C2,$00,$F5 ;Input: A=$FB Y=$00 P=$F7 M=$C1  Output: A=$FB X=$FF Y=$00 P=$F5 M=$C1
		!byte	$52,$C4,$01,$6C,$75 ;Input: A=$44 Y=$01 P=$F5 M=$2C  Output: A=$6C X=$FE Y=$01 P=$75 M=$2C
		!byte	$12,$01,$7F,$30 ;Input: A=$6B Y=$01 P=$B2 M=$1E  Output: A=$7F X=$FE Y=$01 P=$30 M=$1E
		!byte	$50,$C2,$00,$FF ;Input: A=$9D Y=$00 P=$F4 M=$6F  Output: A=$FF X=$FF Y=$00 P=$F4 M=$6F
		!byte	$10,$00,$5F ;Input: A=$17 Y=$00 P=$31 M=$5F  Output: A=$5F X=$FF Y=$00 P=$31 M=$5F
		!byte	$12,$00,$D7,$B4 ;Input: A=$C7 Y=$00 P=$34 M=$17  Output: A=$D7 X=$FF Y=$00 P=$B4 M=$17
		!byte	$52,$C4,$01,$9B,$F4 ;Input: A=$1B Y=$01 P=$74 M=$8B  Output: A=$9B X=$FE Y=$01 P=$F4 M=$8B

		; $15  ORA zp,x
		!byte	$D0,$15,$A4,$D6,$AC ;Input: A=$88 Y=$D6 P=$B5 M=$A4  Output: A=$AC X=$29 Y=$D6 P=$B5 M=$A4
		!byte	$52,$E2,$14,$DB,$F5 ;Input: A=$9A Y=$14 P=$77 M=$43  Output: A=$DB X=$EB Y=$14 P=$F5 M=$43
		!byte	$52,$4C,$7E,$5B,$35 ;Input: A=$52 Y=$7E P=$B5 M=$09  Output: A=$5B X=$81 Y=$7E P=$35 M=$09
		!byte	$52,$AA,$DC,$FF,$B1 ;Input: A=$3E Y=$DC P=$31 M=$DB  Output: A=$FF X=$23 Y=$DC P=$B1 M=$DB
		!byte	$52,$34,$66,$3F,$35 ;Input: A=$0F Y=$66 P=$B7 M=$39  Output: A=$3F X=$99 Y=$66 P=$35 M=$39
		!byte	$52,$BD,$EF,$7F,$74 ;Input: A=$0D Y=$EF P=$F6 M=$73  Output: A=$7F X=$10 Y=$EF P=$74 M=$73
		!byte	$42,$17,$49,$70 ;Input: A=$59 Y=$49 P=$72 M=$01  Output: A=$59 X=$B6 Y=$49 P=$70 M=$01
		!byte	$52,$CD,$FF,$FF,$F0 ;Input: A=$F8 Y=$FF P=$72 M=$1F  Output: A=$FF X=$00 Y=$FF P=$F0 M=$1F

		; $16  ASL zp,x
		!byte	$C3,$16,$5D,$8F,$F1,$DA ;Input: A=$11 Y=$8F P=$70 M=$ED  Output: A=$11 X=$70 Y=$8F P=$F1 M=$DA
		!byte	$43,$9B,$CD,$F4,$BA ;Input: A=$3E Y=$CD P=$75 M=$5D  Output: A=$3E X=$32 Y=$CD P=$F4 M=$BA
		!byte	$43,$EC,$1E,$B5,$E6 ;Input: A=$D0 Y=$1E P=$37 M=$F3  Output: A=$D0 X=$E1 Y=$1E P=$B5 M=$E6
		!byte	$43,$6A,$9C,$F5,$9C ;Input: A=$AB Y=$9C P=$F4 M=$CE  Output: A=$AB X=$63 Y=$9C P=$F5 M=$9C
		!byte	$43,$40,$72,$B1,$A6 ;Input: A=$F7 Y=$72 P=$32 M=$D3  Output: A=$F7 X=$8D Y=$72 P=$B1 M=$A6
		!byte	$41,$B4,$E6,$6C ;Input: A=$0D Y=$E6 P=$34 M=$36  Output: A=$0D X=$19 Y=$E6 P=$34 M=$6C
		!byte	$43,$BE,$F0,$74,$4A ;Input: A=$8B Y=$F0 P=$76 M=$25  Output: A=$8B X=$0F Y=$F0 P=$74 M=$4A
		!byte	$43,$1D,$4F,$B5,$E8 ;Input: A=$C1 Y=$4F P=$36 M=$F4  Output: A=$C1 X=$B0 Y=$4F P=$B5 M=$E8

		; $18  CLC
		!byte	$82,$18,$77,$36 ;Input: A=$EA Y=$77 P=$37 M=$21  Output: A=$EA X=$88 Y=$77 P=$36 M=$21
		!byte	$00,$D7 ;Input: A=$37 Y=$D7 P=$B0 M=$BC  Output: A=$37 X=$28 Y=$D7 P=$B0 M=$BC
		!byte	$00,$39 ;Input: A=$5B Y=$39 P=$F2 M=$03  Output: A=$5B X=$C6 Y=$39 P=$F2 M=$03
		!byte	$00,$53 ;Input: A=$8A Y=$53 P=$F0 M=$0C  Output: A=$8A X=$AC Y=$53 P=$F0 M=$0C
		!byte	$02,$C9,$72 ;Input: A=$BF Y=$C9 P=$73 M=$CE  Output: A=$BF X=$36 Y=$C9 P=$72 M=$CE
		!byte	$02,$1E,$F0 ;Input: A=$4D Y=$1E P=$F1 M=$7C  Output: A=$4D X=$E1 Y=$1E P=$F0 M=$7C
		!byte	$02,$74,$70 ;Input: A=$11 Y=$74 P=$71 M=$D9  Output: A=$11 X=$8B Y=$74 P=$70 M=$D9
		!byte	$02,$0C,$32 ;Input: A=$44 Y=$0C P=$33 M=$E4  Output: A=$44 X=$F3 Y=$0C P=$32 M=$E4

		; $19  ORA abs,y
		!byte	$F2,$19,$F8,$FF,$D5,$77,$74 ;Input: A=$75 Y=$D5 P=$76 M=$73  Output: A=$77 X=$2A Y=$D5 P=$74 M=$73
		!byte	$72,$2E,$00,$9F,$A3,$B5 ;Input: A=$A1 Y=$9F P=$35 M=$A2  Output: A=$A3 X=$60 Y=$9F P=$B5 M=$A2
		!byte	$52,$29,$A4,$EA,$B4 ;Input: A=$E8 Y=$A4 P=$34 M=$6A  Output: A=$EA X=$5B Y=$A4 P=$B4 M=$6A
		!byte	$42,$4F,$7E,$B0 ;Input: A=$F9 Y=$7E P=$30 M=$D1  Output: A=$F9 X=$81 Y=$7E P=$B0 M=$D1
		!byte	$70,$FE,$FF,$CF,$AF ;Input: A=$27 Y=$CF P=$F5 M=$AC  Output: A=$AF X=$30 Y=$CF P=$F5 M=$AC
		!byte	$72,$AB,$00,$22,$F7,$F0 ;Input: A=$F5 Y=$22 P=$70 M=$22  Output: A=$F7 X=$DD Y=$22 P=$F0 M=$22
		!byte	$52,$01,$CC,$FB,$B0 ;Input: A=$FA Y=$CC P=$32 M=$09  Output: A=$FB X=$33 Y=$CC P=$B0 M=$09
		!byte	$72,$FA,$FF,$D3,$F6,$B0 ;Input: A=$B6 Y=$D3 P=$B2 M=$56  Output: A=$F6 X=$2C Y=$D3 P=$B0 M=$56

		; $1D  ORA abs,x
		!byte	$F0,$1D,$FB,$FF,$2D,$FF ;Input: A=$3F Y=$2D P=$B4 M=$E3  Output: A=$FF X=$D2 Y=$2D P=$B4 M=$E3
		!byte	$70,$A1,$00,$D3,$2C ;Input: A=$00 Y=$D3 P=$30 M=$2C  Output: A=$2C X=$2C Y=$D3 P=$30 M=$2C
		!byte	$50,$43,$75,$DB ;Input: A=$D9 Y=$75 P=$F0 M=$CA  Output: A=$DB X=$8A Y=$75 P=$F0 M=$CA
		!byte	$50,$A7,$D9,$F3 ;Input: A=$B1 Y=$D9 P=$F5 M=$F2  Output: A=$F3 X=$26 Y=$D9 P=$F5 M=$F2
		!byte	$70,$F8,$FF,$2A,$5C ;Input: A=$1C Y=$2A P=$70 M=$50  Output: A=$5C X=$D5 Y=$2A P=$70 M=$50
		!byte	$72,$8E,$00,$C0,$8D,$B1 ;Input: A=$84 Y=$C0 P=$33 M=$8D  Output: A=$8D X=$3F Y=$C0 P=$B1 M=$8D
		!byte	$72,$F9,$FF,$2B,$5F,$74 ;Input: A=$5D Y=$2B P=$F4 M=$5B  Output: A=$5F X=$D4 Y=$2B P=$74 M=$5B
		!byte	$72,$4F,$00,$81,$FB,$F1 ;Input: A=$23 Y=$81 P=$73 M=$DB  Output: A=$FB X=$7E Y=$81 P=$F1 M=$DB

		; $1E  ASL abs,x
		!byte	$E3,$1E,$12,$00,$44,$F0,$90 ;Input: A=$46 Y=$44 P=$71 M=$48  Output: A=$46 X=$BB Y=$44 P=$F0 M=$90
		!byte	$43,$2D,$5F,$F1,$90 ;Input: A=$FB Y=$5F P=$70 M=$C8  Output: A=$FB X=$A0 Y=$5F P=$F1 M=$90
		!byte	$63,$CE,$FF,$00,$B5,$EA ;Input: A=$81 Y=$00 P=$37 M=$F5  Output: A=$81 X=$FF Y=$00 P=$B5 M=$EA
		!byte	$61,$2F,$00,$61,$A0 ;Input: A=$BE Y=$61 P=$B0 M=$50  Output: A=$BE X=$9E Y=$61 P=$B0 M=$A0
		!byte	$43,$8C,$BE,$B0,$FE ;Input: A=$CE Y=$BE P=$32 M=$7F  Output: A=$CE X=$41 Y=$BE P=$B0 M=$FE
		!byte	$63,$D3,$FF,$05,$F1,$F8 ;Input: A=$61 Y=$05 P=$F3 M=$FC  Output: A=$61 X=$FA Y=$05 P=$F1 M=$F8
		!byte	$43,$D5,$07,$F1,$DA ;Input: A=$67 Y=$07 P=$72 M=$ED  Output: A=$67 X=$F8 Y=$07 P=$F1 M=$DA
		!byte	$63,$01,$00,$33,$B0,$D2 ;Input: A=$53 Y=$33 P=$33 M=$69  Output: A=$53 X=$CC Y=$33 P=$B0 M=$D2

		; $21  AND (zp,x)
		!byte	$D2,$21,$E2,$1F,$20,$74 ;Input: A=$FE Y=$1F P=$F4 M=$21  Output: A=$20 X=$E0 Y=$1F P=$74 M=$21
		!byte	$52,$33,$70,$52,$31 ;Input: A=$F6 Y=$70 P=$B3 M=$53  Output: A=$52 X=$8F Y=$70 P=$31 M=$53
		!byte	$52,$9D,$DA,$88,$F1 ;Input: A=$9D Y=$DA P=$F3 M=$A8  Output: A=$88 X=$25 Y=$DA P=$F1 M=$A8
		!byte	$52,$17,$54,$08,$71 ;Input: A=$48 Y=$54 P=$73 M=$3D  Output: A=$08 X=$AB Y=$54 P=$71 M=$3D
		!byte	$52,$74,$B1,$00,$73 ;Input: A=$31 Y=$B1 P=$71 M=$88  Output: A=$00 X=$4E Y=$B1 P=$73 M=$88
		!byte	$52,$01,$3E,$54,$71 ;Input: A=$D4 Y=$3E P=$73 M=$5C  Output: A=$54 X=$C1 Y=$3E P=$71 M=$5C
		!byte	$52,$D8,$15,$00,$73 ;Input: A=$51 Y=$15 P=$F1 M=$84  Output: A=$00 X=$EA Y=$15 P=$73 M=$84
		!byte	$52,$09,$46,$CC,$B0 ;Input: A=$EC Y=$46 P=$32 M=$DD  Output: A=$CC X=$B9 Y=$46 P=$B0 M=$DD

		; $24  BIT zp
		!byte	$C2,$24,$CD,$04,$B1 ;Input: A=$CE Y=$04 P=$31 M=$98  Output: A=$CE X=$FB Y=$04 P=$B1 M=$98
		!byte	$02,$73,$71 ;Input: A=$CC Y=$73 P=$31 M=$67  Output: A=$CC X=$8C Y=$73 P=$71 M=$67
		!byte	$02,$D9,$B1 ;Input: A=$15 Y=$D9 P=$73 M=$B1  Output: A=$15 X=$26 Y=$D9 P=$B1 M=$B1
		!byte	$02,$2F,$B2 ;Input: A=$41 Y=$2F P=$72 M=$BE  Output: A=$41 X=$D0 Y=$2F P=$B2 M=$BE
		!byte	$02,$68,$74 ;Input: A=$58 Y=$68 P=$B4 M=$75  Output: A=$58 X=$97 Y=$68 P=$74 M=$75
		!byte	$02,$F7,$F5 ;Input: A=$42 Y=$F7 P=$B7 M=$FA  Output: A=$42 X=$08 Y=$F7 P=$F5 M=$FA
		!byte	$02,$4A,$31 ;Input: A=$65 Y=$4A P=$71 M=$11  Output: A=$65 X=$B5 Y=$4A P=$31 M=$11
		!byte	$02,$57,$71 ;Input: A=$D8 Y=$57 P=$B3 M=$76  Output: A=$D8 X=$A8 Y=$57 P=$71 M=$76

		; $25  AND zp
		!byte	$D2,$25,$CD,$76,$00,$36 ;Input: A=$C5 Y=$76 P=$B6 M=$22  Output: A=$00 X=$89 Y=$76 P=$36 M=$22
		!byte	$12,$16,$94,$F0 ;Input: A=$B4 Y=$16 P=$F2 M=$D4  Output: A=$94 X=$E9 Y=$16 P=$F0 M=$D4
		!byte	$12,$11,$00,$36 ;Input: A=$92 Y=$11 P=$B6 M=$08  Output: A=$00 X=$EE Y=$11 P=$36 M=$08
		!byte	$12,$AE,$00,$73 ;Input: A=$0F Y=$AE P=$71 M=$80  Output: A=$00 X=$51 Y=$AE P=$73 M=$80
		!byte	$12,$88,$40,$70 ;Input: A=$E8 Y=$88 P=$F2 M=$50  Output: A=$40 X=$77 Y=$88 P=$70 M=$50
		!byte	$12,$FE,$12,$74 ;Input: A=$D3 Y=$FE P=$F4 M=$1A  Output: A=$12 X=$01 Y=$FE P=$74 M=$1A
		!byte	$12,$DB,$09,$35 ;Input: A=$6F Y=$DB P=$B5 M=$09  Output: A=$09 X=$24 Y=$DB P=$35 M=$09
		!byte	$12,$43,$9A,$B1 ;Input: A=$BE Y=$43 P=$33 M=$DB  Output: A=$9A X=$BC Y=$43 P=$B1 M=$DB

		; $26  ROL zp
		!byte	$C1,$26,$CD,$3C,$0E ;Input: A=$0F Y=$3C P=$30 M=$07  Output: A=$0F X=$C3 Y=$3C P=$30 M=$0E
		!byte	$03,$45,$B5,$B0 ;Input: A=$0D Y=$45 P=$34 M=$D8  Output: A=$0D X=$BA Y=$45 P=$B5 M=$B0
		!byte	$03,$D8,$B5,$C6 ;Input: A=$68 Y=$D8 P=$B6 M=$E3  Output: A=$68 X=$27 Y=$D8 P=$B5 M=$C6
		!byte	$03,$CE,$74,$5D ;Input: A=$9C Y=$CE P=$F7 M=$2E  Output: A=$9C X=$31 Y=$CE P=$74 M=$5D
		!byte	$03,$F5,$31,$70 ;Input: A=$91 Y=$F5 P=$32 M=$B8  Output: A=$91 X=$0A Y=$F5 P=$31 M=$70
		!byte	$03,$60,$71,$60 ;Input: A=$03 Y=$60 P=$F2 M=$B0  Output: A=$03 X=$9F Y=$60 P=$71 M=$60
		!byte	$03,$3D,$B4,$B5 ;Input: A=$5F Y=$3D P=$37 M=$5A  Output: A=$5F X=$C2 Y=$3D P=$B4 M=$B5
		!byte	$03,$27,$B0,$D9 ;Input: A=$9D Y=$27 P=$33 M=$6C  Output: A=$9D X=$D8 Y=$27 P=$B0 M=$D9

		; $29  AND #imm
		!byte	$D2,$29,$76,$59,$42,$71 ;Input: A=$43 Y=$59 P=$73 M=$61  Output: A=$42 X=$A6 Y=$59 P=$71 M=$61
		!byte	$50,$D0,$2D,$10 ;Input: A=$3C Y=$2D P=$30 M=$54  Output: A=$10 X=$D2 Y=$2D P=$30 M=$54
		!byte	$52,$C9,$C2,$C0,$F1 ;Input: A=$F0 Y=$C2 P=$73 M=$E6  Output: A=$C0 X=$3D Y=$C2 P=$F1 M=$E6
		!byte	$40,$68,$CD ;Input: A=$60 Y=$CD P=$74 M=$AE  Output: A=$60 X=$32 Y=$CD P=$74 M=$AE
		!byte	$52,$49,$D4,$09,$71 ;Input: A=$9D Y=$D4 P=$F1 M=$30  Output: A=$09 X=$2B Y=$D4 P=$71 M=$30
		!byte	$52,$79,$6A,$08,$35 ;Input: A=$88 Y=$6A P=$B7 M=$7B  Output: A=$08 X=$95 Y=$6A P=$35 M=$7B
		!byte	$52,$08,$25,$08,$71 ;Input: A=$0C Y=$25 P=$F3 M=$49  Output: A=$08 X=$DA Y=$25 P=$71 M=$49
		!byte	$52,$40,$61,$40,$35 ;Input: A=$4C Y=$61 P=$B5 M=$25  Output: A=$40 X=$9E Y=$61 P=$35 M=$25

		; $2A  ROL
		!byte	$92,$2A,$14,$E4,$B1 ;Input: A=$F2 Y=$14 P=$B0 M=$05  Output: A=$E4 X=$EB Y=$14 P=$B1 M=$05
		!byte	$12,$3B,$00,$37 ;Input: A=$80 Y=$3B P=$34 M=$D5  Output: A=$00 X=$C4 Y=$3B P=$37 M=$D5
		!byte	$12,$A5,$09,$30 ;Input: A=$04 Y=$A5 P=$B3 M=$B1  Output: A=$09 X=$5A Y=$A5 P=$30 M=$B1
		!byte	$12,$28,$F5,$F5 ;Input: A=$FA Y=$28 P=$F7 M=$4C  Output: A=$F5 X=$D7 Y=$28 P=$F5 M=$4C
		!byte	$10,$8C,$04 ;Input: A=$02 Y=$8C P=$34 M=$6A  Output: A=$04 X=$73 Y=$8C P=$34 M=$6A
		!byte	$12,$A9,$A4,$B1 ;Input: A=$D2 Y=$A9 P=$32 M=$F9  Output: A=$A4 X=$56 Y=$A9 P=$B1 M=$F9
		!byte	$10,$87,$3A ;Input: A=$1D Y=$87 P=$30 M=$4A  Output: A=$3A X=$78 Y=$87 P=$30 M=$4A
		!byte	$12,$8C,$96,$B1 ;Input: A=$CB Y=$8C P=$B2 M=$0A  Output: A=$96 X=$73 Y=$8C P=$B1 M=$0A

		; $2C  BIT abs
		!byte	$E2,$2C,$CD,$00,$F1,$35 ;Input: A=$23 Y=$F1 P=$77 M=$17  Output: A=$23 X=$0E Y=$F1 P=$35 M=$17
		!byte	$02,$1D,$F3 ;Input: A=$02 Y=$1D P=$71 M=$E5  Output: A=$02 X=$E2 Y=$1D P=$F3 M=$E5
		!byte	$00,$95 ;Input: A=$9E Y=$95 P=$74 M=$78  Output: A=$9E X=$6A Y=$95 P=$74 M=$78
		!byte	$02,$9B,$70 ;Input: A=$1A Y=$9B P=$72 M=$50  Output: A=$1A X=$64 Y=$9B P=$70 M=$50
		!byte	$02,$C0,$B0 ;Input: A=$BB Y=$C0 P=$30 M=$B3  Output: A=$BB X=$3F Y=$C0 P=$B0 M=$B3
		!byte	$02,$E9,$74 ;Input: A=$1F Y=$E9 P=$F6 M=$7E  Output: A=$1F X=$16 Y=$E9 P=$74 M=$7E
		!byte	$02,$9A,$74 ;Input: A=$1E Y=$9A P=$B4 M=$4D  Output: A=$1E X=$65 Y=$9A P=$74 M=$4D
		!byte	$02,$86,$71 ;Input: A=$A4 Y=$86 P=$F1 M=$6A  Output: A=$A4 X=$79 Y=$86 P=$71 M=$6A

		; $2D  AND abs
		!byte	$F2,$2D,$CD,$00,$B5,$03,$70 ;Input: A=$87 Y=$B5 P=$F0 M=$33  Output: A=$03 X=$4A Y=$B5 P=$70 M=$33
		!byte	$10,$BF,$A2 ;Input: A=$A3 Y=$BF P=$F5 M=$BA  Output: A=$A2 X=$40 Y=$BF P=$F5 M=$BA
		!byte	$12,$E7,$34,$34 ;Input: A=$35 Y=$E7 P=$36 M=$FE  Output: A=$34 X=$18 Y=$E7 P=$34 M=$FE
		!byte	$10,$23,$29 ;Input: A=$69 Y=$23 P=$34 M=$BF  Output: A=$29 X=$DC Y=$23 P=$34 M=$BF
		!byte	$12,$55,$45,$70 ;Input: A=$F5 Y=$55 P=$72 M=$4D  Output: A=$45 X=$AA Y=$55 P=$70 M=$4D
		!byte	$12,$38,$A1,$B4 ;Input: A=$ED Y=$38 P=$B6 M=$A3  Output: A=$A1 X=$C7 Y=$38 P=$B4 M=$A3
		!byte	$10,$D1,$30 ;Input: A=$71 Y=$D1 P=$30 M=$BA  Output: A=$30 X=$2E Y=$D1 P=$30 M=$BA
		!byte	$12,$6C,$24,$34 ;Input: A=$EE Y=$6C P=$B4 M=$34  Output: A=$24 X=$93 Y=$6C P=$34 M=$34

		; $2E  ROL abs
		!byte	$E3,$2E,$CD,$00,$66,$34,$79 ;Input: A=$73 Y=$66 P=$37 M=$3C  Output: A=$73 X=$99 Y=$66 P=$34 M=$79
		!byte	$03,$4E,$35,$70 ;Input: A=$D3 Y=$4E P=$34 M=$B8  Output: A=$D3 X=$B1 Y=$4E P=$35 M=$70
		!byte	$01,$30,$6F ;Input: A=$19 Y=$30 P=$35 M=$B7  Output: A=$19 X=$CF Y=$30 P=$35 M=$6F
		!byte	$03,$FE,$F0,$F9 ;Input: A=$83 Y=$FE P=$71 M=$7C  Output: A=$83 X=$01 Y=$FE P=$F0 M=$F9
		!byte	$03,$71,$F4,$C8 ;Input: A=$53 Y=$71 P=$F6 M=$64  Output: A=$53 X=$8E Y=$71 P=$F4 M=$C8
		!byte	$03,$E2,$71,$5A ;Input: A=$48 Y=$E2 P=$72 M=$AD  Output: A=$48 X=$1D Y=$E2 P=$71 M=$5A
		!byte	$03,$E2,$B5,$D3 ;Input: A=$CC Y=$E2 P=$37 M=$E9  Output: A=$CC X=$1D Y=$E2 P=$B5 M=$D3
		!byte	$03,$9B,$34,$19 ;Input: A=$2D Y=$9B P=$B7 M=$0C  Output: A=$2D X=$64 Y=$9B P=$34 M=$19

		; $31  AND (zp),y
		!byte	$D2,$31,$C4,$01,$07,$74 ;Input: A=$7F Y=$01 P=$F4 M=$87  Output: A=$07 X=$FE Y=$01 P=$74 M=$87
		!byte	$52,$C2,$00,$05,$71 ;Input: A=$CD Y=$00 P=$F1 M=$25  Output: A=$05 X=$FF Y=$00 P=$71 M=$25
		!byte	$02,$00,$30 ;Input: A=$4B Y=$00 P=$B2 M=$5B  Output: A=$4B X=$FF Y=$00 P=$30 M=$5B
		!byte	$50,$C4,$01,$10 ;Input: A=$56 Y=$01 P=$34 M=$99  Output: A=$10 X=$FE Y=$01 P=$34 M=$99
		!byte	$02,$01,$30 ;Input: A=$38 Y=$01 P=$B2 M=$3B  Output: A=$38 X=$FE Y=$01 P=$30 M=$3B
		!byte	$52,$C2,$00,$24,$70 ;Input: A=$2C Y=$00 P=$F2 M=$E5  Output: A=$24 X=$FF Y=$00 P=$70 M=$E5
		!byte	$12,$00,$CC,$F1 ;Input: A=$DD Y=$00 P=$F3 M=$CC  Output: A=$CC X=$FF Y=$00 P=$F1 M=$CC
		!byte	$52,$C4,$01,$40,$70 ;Input: A=$50 Y=$01 P=$F2 M=$47  Output: A=$40 X=$FE Y=$01 P=$70 M=$47

		; $35  AND zp,x
		!byte	$D2,$35,$AD,$DF,$1C,$71 ;Input: A=$1D Y=$DF P=$F1 M=$9E  Output: A=$1C X=$20 Y=$DF P=$71 M=$9E
		!byte	$50,$F7,$29,$11 ;Input: A=$59 Y=$29 P=$34 M=$15  Output: A=$11 X=$D6 Y=$29 P=$34 M=$15
		!byte	$52,$0C,$3E,$08,$34 ;Input: A=$2C Y=$3E P=$B6 M=$19  Output: A=$08 X=$C1 Y=$3E P=$34 M=$19
		!byte	$52,$AE,$E0,$98,$F1 ;Input: A=$F8 Y=$E0 P=$73 M=$9A  Output: A=$98 X=$1F Y=$E0 P=$F1 M=$9A
		!byte	$52,$E0,$12,$02,$34 ;Input: A=$0F Y=$12 P=$36 M=$B2  Output: A=$02 X=$ED Y=$12 P=$34 M=$B2
		!byte	$50,$8D,$BF,$09 ;Input: A=$5D Y=$BF P=$70 M=$09  Output: A=$09 X=$40 Y=$BF P=$70 M=$09
		!byte	$52,$74,$A6,$01,$31 ;Input: A=$13 Y=$A6 P=$33 M=$05  Output: A=$01 X=$59 Y=$A6 P=$31 M=$05
		!byte	$50,$57,$89,$21 ;Input: A=$A1 Y=$89 P=$75 M=$6B  Output: A=$21 X=$76 Y=$89 P=$75 M=$6B

		; $36  ROL zp,x
		!byte	$C3,$36,$BF,$F1,$F5,$CA ;Input: A=$82 Y=$F1 P=$74 M=$E5  Output: A=$82 X=$0E Y=$F1 P=$F5 M=$CA
		!byte	$43,$DC,$0E,$F0,$C5 ;Input: A=$FC Y=$0E P=$F1 M=$62  Output: A=$FC X=$F1 Y=$0E P=$F0 M=$C5
		!byte	$43,$3A,$6C,$34,$13 ;Input: A=$D6 Y=$6C P=$B7 M=$09  Output: A=$D6 X=$93 Y=$6C P=$34 M=$13
		!byte	$43,$6F,$A1,$F1,$8F ;Input: A=$0F Y=$A1 P=$73 M=$C7  Output: A=$0F X=$5E Y=$A1 P=$F1 M=$8F
		!byte	$43,$EE,$20,$34,$28 ;Input: A=$E9 Y=$20 P=$36 M=$14  Output: A=$E9 X=$DF Y=$20 P=$34 M=$28
		!byte	$43,$99,$CB,$B4,$AF ;Input: A=$C2 Y=$CB P=$B5 M=$57  Output: A=$C2 X=$34 Y=$CB P=$B4 M=$AF
		!byte	$43,$D1,$03,$35,$2A ;Input: A=$1D Y=$03 P=$B6 M=$95  Output: A=$1D X=$FC Y=$03 P=$35 M=$2A
		!byte	$43,$0F,$41,$75,$70 ;Input: A=$4D Y=$41 P=$76 M=$B8  Output: A=$4D X=$BE Y=$41 P=$75 M=$70

		; $38  SEC
		!byte	$80,$38,$84 ;Input: A=$5D Y=$84 P=$F3 M=$F0  Output: A=$5D X=$7B Y=$84 P=$F3 M=$F0
		!byte	$00,$14 ;Input: A=$1B Y=$14 P=$71 M=$CF  Output: A=$1B X=$EB Y=$14 P=$71 M=$CF
		!byte	$00,$65 ;Input: A=$54 Y=$65 P=$B3 M=$26  Output: A=$54 X=$9A Y=$65 P=$B3 M=$26
		!byte	$02,$28,$33 ;Input: A=$06 Y=$28 P=$32 M=$C2  Output: A=$06 X=$D7 Y=$28 P=$33 M=$C2
		!byte	$00,$C9 ;Input: A=$BB Y=$C9 P=$F3 M=$10  Output: A=$BB X=$36 Y=$C9 P=$F3 M=$10
		!byte	$02,$E7,$B5 ;Input: A=$AA Y=$E7 P=$B4 M=$8F  Output: A=$AA X=$18 Y=$E7 P=$B5 M=$8F
		!byte	$00,$8E ;Input: A=$68 Y=$8E P=$B3 M=$4E  Output: A=$68 X=$71 Y=$8E P=$B3 M=$4E
		!byte	$00,$36 ;Input: A=$DF Y=$36 P=$71 M=$A6  Output: A=$DF X=$C9 Y=$36 P=$71 M=$A6

		; $39  AND abs,y
		!byte	$F0,$39,$F1,$FF,$DC,$0E ;Input: A=$5E Y=$DC P=$34 M=$0E  Output: A=$0E X=$23 Y=$DC P=$34 M=$0E
		!byte	$72,$25,$00,$A8,$88,$B0 ;Input: A=$C8 Y=$A8 P=$30 M=$BF  Output: A=$88 X=$57 Y=$A8 P=$B0 M=$BF
		!byte	$72,$CF,$FF,$FE,$00,$72 ;Input: A=$BD Y=$FE P=$F0 M=$42  Output: A=$00 X=$01 Y=$FE P=$72 M=$42
		!byte	$52,$DB,$F2,$48,$31 ;Input: A=$5D Y=$F2 P=$B1 M=$C8  Output: A=$48 X=$0D Y=$F2 P=$31 M=$C8
		!byte	$70,$5C,$00,$71,$10 ;Input: A=$DD Y=$71 P=$35 M=$12  Output: A=$10 X=$8E Y=$71 P=$35 M=$12
		!byte	$52,$2D,$A0,$62,$34 ;Input: A=$76 Y=$A0 P=$B6 M=$E2  Output: A=$62 X=$5F Y=$A0 P=$34 M=$E2
		!byte	$52,$92,$3B,$20,$74 ;Input: A=$A2 Y=$3B P=$F6 M=$68  Output: A=$20 X=$C4 Y=$3B P=$74 M=$68
		!byte	$52,$C7,$06,$90,$B5 ;Input: A=$94 Y=$06 P=$35 M=$B3  Output: A=$90 X=$F9 Y=$06 P=$B5 M=$B3

		; $3D  AND abs,x
		!byte	$F2,$3D,$4C,$00,$7E,$00,$76 ;Input: A=$0B Y=$7E P=$F4 M=$64  Output: A=$00 X=$81 Y=$7E P=$76 M=$64
		!byte	$52,$46,$78,$20,$71 ;Input: A=$66 Y=$78 P=$F1 M=$B1  Output: A=$20 X=$87 Y=$78 P=$71 M=$B1
		!byte	$52,$1B,$4D,$00,$32 ;Input: A=$A8 Y=$4D P=$B0 M=$15  Output: A=$00 X=$B2 Y=$4D P=$32 M=$15
		!byte	$50,$8C,$BE,$2A ;Input: A=$6E Y=$BE P=$31 M=$AB  Output: A=$2A X=$41 Y=$BE P=$31 M=$AB
		!byte	$12,$BE,$24,$70 ;Input: A=$F4 Y=$BE P=$72 M=$2F  Output: A=$24 X=$41 Y=$BE P=$70 M=$2F
		!byte	$52,$90,$C2,$20,$35 ;Input: A=$BA Y=$C2 P=$B7 M=$65  Output: A=$20 X=$3D Y=$C2 P=$35 M=$65
		!byte	$52,$0F,$41,$10,$35 ;Input: A=$38 Y=$41 P=$B5 M=$93  Output: A=$10 X=$BE Y=$41 P=$35 M=$93
		!byte	$50,$5A,$8C,$0D ;Input: A=$ED Y=$8C P=$70 M=$0D  Output: A=$0D X=$73 Y=$8C P=$70 M=$0D

		; $3E  ROL abs,x
		!byte	$E3,$3E,$FE,$FF,$30,$34,$3A ;Input: A=$9A Y=$30 P=$B4 M=$1D  Output: A=$9A X=$CF Y=$30 P=$34 M=$3A
		!byte	$63,$3E,$00,$70,$75,$01 ;Input: A=$D2 Y=$70 P=$77 M=$80  Output: A=$D2 X=$8F Y=$70 P=$75 M=$01
		!byte	$43,$75,$A7,$F4,$FE ;Input: A=$76 Y=$A7 P=$76 M=$7F  Output: A=$76 X=$58 Y=$A7 P=$F4 M=$FE
		!byte	$43,$54,$86,$75,$50 ;Input: A=$17 Y=$86 P=$F6 M=$A8  Output: A=$17 X=$79 Y=$86 P=$75 M=$50
		!byte	$43,$63,$95,$74,$36 ;Input: A=$D3 Y=$95 P=$76 M=$1B  Output: A=$D3 X=$6A Y=$95 P=$74 M=$36
		!byte	$41,$29,$5B,$B1 ;Input: A=$AB Y=$5B P=$B1 M=$D8  Output: A=$AB X=$A4 Y=$5B P=$B1 M=$B1
		!byte	$61,$E1,$FF,$13,$CC ;Input: A=$27 Y=$13 P=$B4 M=$66  Output: A=$27 X=$EC Y=$13 P=$B4 M=$CC
		!byte	$63,$B4,$00,$E6,$F4,$B4 ;Input: A=$FE Y=$E6 P=$F6 M=$5A  Output: A=$FE X=$19 Y=$E6 P=$F4 M=$B4

		; $41  EOR (zp,x)
		!byte	$D2,$41,$B8,$F5,$6F,$75 ;Input: A=$FA Y=$F5 P=$77 M=$95  Output: A=$6F X=$0A Y=$F5 P=$75 M=$95
		!byte	$52,$81,$BE,$84,$B4 ;Input: A=$D1 Y=$BE P=$34 M=$55  Output: A=$84 X=$41 Y=$BE P=$B4 M=$55
		!byte	$52,$62,$9F,$92,$F5 ;Input: A=$15 Y=$9F P=$77 M=$87  Output: A=$92 X=$60 Y=$9F P=$F5 M=$87
		!byte	$52,$4D,$8A,$E4,$B0 ;Input: A=$7F Y=$8A P=$32 M=$9B  Output: A=$E4 X=$75 Y=$8A P=$B0 M=$9B
		!byte	$50,$DE,$1B,$EB ;Input: A=$67 Y=$1B P=$B5 M=$8C  Output: A=$EB X=$E4 Y=$1B P=$B5 M=$8C
		!byte	$52,$7B,$B8,$A9,$B1 ;Input: A=$2C Y=$B8 P=$31 M=$85  Output: A=$A9 X=$47 Y=$B8 P=$B1 M=$85
		!byte	$50,$2B,$68,$0F ;Input: A=$44 Y=$68 P=$31 M=$4B  Output: A=$0F X=$97 Y=$68 P=$31 M=$4B
		!byte	$52,$35,$72,$44,$30 ;Input: A=$3F Y=$72 P=$32 M=$7B  Output: A=$44 X=$8D Y=$72 P=$30 M=$7B

		; $45  EOR zp
		!byte	$D0,$45,$CD,$14,$D6 ;Input: A=$1A Y=$14 P=$F4 M=$CC  Output: A=$D6 X=$EB Y=$14 P=$F4 M=$CC
		!byte	$10,$CD,$F9 ;Input: A=$B8 Y=$CD P=$F4 M=$41  Output: A=$F9 X=$32 Y=$CD P=$F4 M=$41
		!byte	$10,$0D,$48 ;Input: A=$6D Y=$0D P=$75 M=$25  Output: A=$48 X=$F2 Y=$0D P=$75 M=$25
		!byte	$12,$44,$97,$F1 ;Input: A=$7E Y=$44 P=$71 M=$E9  Output: A=$97 X=$BB Y=$44 P=$F1 M=$E9
		!byte	$12,$B4,$D0,$F4 ;Input: A=$A8 Y=$B4 P=$F6 M=$78  Output: A=$D0 X=$4B Y=$B4 P=$F4 M=$78
		!byte	$12,$66,$F8,$B5 ;Input: A=$21 Y=$66 P=$35 M=$D9  Output: A=$F8 X=$99 Y=$66 P=$B5 M=$D9
		!byte	$12,$33,$4F,$75 ;Input: A=$E1 Y=$33 P=$F7 M=$AE  Output: A=$4F X=$CC Y=$33 P=$75 M=$AE
		!byte	$12,$CB,$6F,$71 ;Input: A=$EB Y=$CB P=$F1 M=$84  Output: A=$6F X=$34 Y=$CB P=$71 M=$84

		; $46  LSR zp
		!byte	$C3,$46,$CD,$10,$75,$64 ;Input: A=$3C Y=$10 P=$F5 M=$C9  Output: A=$3C X=$EF Y=$10 P=$75 M=$64
		!byte	$03,$4C,$34,$14 ;Input: A=$2E Y=$4C P=$35 M=$28  Output: A=$2E X=$B3 Y=$4C P=$34 M=$14
		!byte	$03,$03,$30,$6B ;Input: A=$9D Y=$03 P=$B3 M=$D6  Output: A=$9D X=$FC Y=$03 P=$30 M=$6B
		!byte	$03,$73,$70,$6F ;Input: A=$2C Y=$73 P=$73 M=$DE  Output: A=$2C X=$8C Y=$73 P=$70 M=$6F
		!byte	$03,$05,$74,$0B ;Input: A=$09 Y=$05 P=$75 M=$16  Output: A=$09 X=$FA Y=$05 P=$74 M=$0B
		!byte	$01,$31,$41 ;Input: A=$9D Y=$31 P=$35 M=$83  Output: A=$9D X=$CE Y=$31 P=$35 M=$41
		!byte	$03,$AA,$34,$73 ;Input: A=$1D Y=$AA P=$37 M=$E6  Output: A=$1D X=$55 Y=$AA P=$34 M=$73
		!byte	$01,$CF,$7D ;Input: A=$AE Y=$CF P=$30 M=$FA  Output: A=$AE X=$30 Y=$CF P=$30 M=$7D

		; $49  EOR #imm
		!byte	$D2,$49,$CA,$B0,$73,$34 ;Input: A=$B9 Y=$B0 P=$B6 M=$96  Output: A=$73 X=$4F Y=$B0 P=$34 M=$96
		!byte	$50,$FF,$27,$EF ;Input: A=$10 Y=$27 P=$B0 M=$6C  Output: A=$EF X=$D8 Y=$27 P=$B0 M=$6C
		!byte	$52,$9D,$D8,$4D,$74 ;Input: A=$D0 Y=$D8 P=$F4 M=$E3  Output: A=$4D X=$27 Y=$D8 P=$74 M=$E3
		!byte	$52,$4D,$14,$DB,$B4 ;Input: A=$96 Y=$14 P=$36 M=$69  Output: A=$DB X=$EB Y=$14 P=$B4 M=$69
		!byte	$50,$79,$F4,$54 ;Input: A=$2D Y=$F4 P=$74 M=$2F  Output: A=$54 X=$0B Y=$F4 P=$74 M=$2F
		!byte	$52,$59,$27,$85,$B4 ;Input: A=$DC Y=$27 P=$36 M=$4A  Output: A=$85 X=$D8 Y=$27 P=$B4 M=$4A
		!byte	$52,$7C,$42,$90,$B5 ;Input: A=$EC Y=$42 P=$35 M=$25  Output: A=$90 X=$BD Y=$42 P=$B5 M=$25
		!byte	$52,$C1,$9C,$F0,$F1 ;Input: A=$31 Y=$9C P=$73 M=$B8  Output: A=$F0 X=$63 Y=$9C P=$F1 M=$B8

		; $4A  LSR
		!byte	$92,$4A,$F8,$65,$74 ;Input: A=$CA Y=$F8 P=$F7 M=$F4  Output: A=$65 X=$07 Y=$F8 P=$74 M=$F4
		!byte	$12,$8C,$54,$71 ;Input: A=$A9 Y=$8C P=$70 M=$E4  Output: A=$54 X=$73 Y=$8C P=$71 M=$E4
		!byte	$12,$20,$3E,$71 ;Input: A=$7D Y=$20 P=$F0 M=$2B  Output: A=$3E X=$DF Y=$20 P=$71 M=$2B
		!byte	$12,$64,$48,$71 ;Input: A=$91 Y=$64 P=$73 M=$F4  Output: A=$48 X=$9B Y=$64 P=$71 M=$F4
		!byte	$12,$A6,$03,$75 ;Input: A=$07 Y=$A6 P=$F4 M=$6A  Output: A=$03 X=$59 Y=$A6 P=$75 M=$6A
		!byte	$12,$95,$0C,$35 ;Input: A=$19 Y=$95 P=$36 M=$3D  Output: A=$0C X=$6A Y=$95 P=$35 M=$3D
		!byte	$12,$B0,$33,$31 ;Input: A=$67 Y=$B0 P=$B3 M=$C3  Output: A=$33 X=$4F Y=$B0 P=$31 M=$C3
		!byte	$12,$8D,$50,$34 ;Input: A=$A0 Y=$8D P=$35 M=$DC  Output: A=$50 X=$72 Y=$8D P=$34 M=$DC

		; $4D  EOR abs
		!byte	$F2,$4D,$CD,$00,$2D,$D5,$B5 ;Input: A=$31 Y=$2D P=$35 M=$E4  Output: A=$D5 X=$D2 Y=$2D P=$B5 M=$E4
		!byte	$12,$E7,$47,$31 ;Input: A=$0F Y=$E7 P=$33 M=$48  Output: A=$47 X=$18 Y=$E7 P=$31 M=$48
		!byte	$12,$B1,$8E,$F4 ;Input: A=$3D Y=$B1 P=$F6 M=$B3  Output: A=$8E X=$4E Y=$B1 P=$F4 M=$B3
		!byte	$12,$DE,$0A,$34 ;Input: A=$95 Y=$DE P=$36 M=$9F  Output: A=$0A X=$21 Y=$DE P=$34 M=$9F
		!byte	$12,$80,$0F,$75 ;Input: A=$A2 Y=$80 P=$F7 M=$AD  Output: A=$0F X=$7F Y=$80 P=$75 M=$AD
		!byte	$12,$08,$85,$B1 ;Input: A=$F7 Y=$08 P=$33 M=$72  Output: A=$85 X=$F7 Y=$08 P=$B1 M=$72
		!byte	$12,$E9,$C6,$F0 ;Input: A=$77 Y=$E9 P=$72 M=$B1  Output: A=$C6 X=$16 Y=$E9 P=$F0 M=$B1
		!byte	$12,$41,$3C,$35 ;Input: A=$9D Y=$41 P=$37 M=$A1  Output: A=$3C X=$BE Y=$41 P=$35 M=$A1

		; $4E  LSR abs
		!byte	$E3,$4E,$CD,$00,$D5,$70,$40 ;Input: A=$90 Y=$D5 P=$73 M=$80  Output: A=$90 X=$2A Y=$D5 P=$70 M=$40
		!byte	$03,$E4,$34,$38 ;Input: A=$EA Y=$E4 P=$35 M=$70  Output: A=$EA X=$1B Y=$E4 P=$34 M=$38
		!byte	$03,$9F,$34,$17 ;Input: A=$5F Y=$9F P=$B4 M=$2E  Output: A=$5F X=$60 Y=$9F P=$34 M=$17
		!byte	$03,$42,$70,$7A ;Input: A=$6F Y=$42 P=$F1 M=$F4  Output: A=$6F X=$BD Y=$42 P=$70 M=$7A
		!byte	$03,$26,$31,$65 ;Input: A=$87 Y=$26 P=$B3 M=$CB  Output: A=$87 X=$D9 Y=$26 P=$31 M=$65
		!byte	$03,$4C,$74,$73 ;Input: A=$19 Y=$4C P=$F7 M=$E6  Output: A=$19 X=$B3 Y=$4C P=$74 M=$73
		!byte	$03,$21,$74,$15 ;Input: A=$ED Y=$21 P=$75 M=$2A  Output: A=$ED X=$DE Y=$21 P=$74 M=$15
		!byte	$03,$93,$31,$0F ;Input: A=$F3 Y=$93 P=$B2 M=$1F  Output: A=$F3 X=$6C Y=$93 P=$31 M=$0F

		; $51  EOR (zp),y
		!byte	$D2,$51,$C4,$01,$F7,$B1 ;Input: A=$CA Y=$01 P=$31 M=$3D  Output: A=$F7 X=$FE Y=$01 P=$B1 M=$3D
		!byte	$10,$01,$FF ;Input: A=$24 Y=$01 P=$F4 M=$DB  Output: A=$FF X=$FE Y=$01 P=$F4 M=$DB
		!byte	$52,$C2,$00,$19,$75 ;Input: A=$EA Y=$00 P=$F5 M=$F3  Output: A=$19 X=$FF Y=$00 P=$75 M=$F3
		!byte	$12,$00,$27,$74 ;Input: A=$13 Y=$00 P=$F4 M=$34  Output: A=$27 X=$FF Y=$00 P=$74 M=$34
		!byte	$12,$00,$22,$35 ;Input: A=$67 Y=$00 P=$B7 M=$45  Output: A=$22 X=$FF Y=$00 P=$35 M=$45
		!byte	$52,$C4,$01,$E6,$B5 ;Input: A=$AB Y=$01 P=$35 M=$4D  Output: A=$E6 X=$FE Y=$01 P=$B5 M=$4D
		!byte	$12,$01,$B3,$F4 ;Input: A=$6F Y=$01 P=$74 M=$DC  Output: A=$B3 X=$FE Y=$01 P=$F4 M=$DC
		!byte	$12,$01,$6B,$75 ;Input: A=$FD Y=$01 P=$F5 M=$96  Output: A=$6B X=$FE Y=$01 P=$75 M=$96

		; $55  EOR zp,x
		!byte	$D2,$55,$9C,$CE,$BB,$F4 ;Input: A=$A7 Y=$CE P=$76 M=$1C  Output: A=$BB X=$31 Y=$CE P=$F4 M=$1C
		!byte	$50,$8C,$BE,$B6 ;Input: A=$A2 Y=$BE P=$B1 M=$14  Output: A=$B6 X=$41 Y=$BE P=$B1 M=$14
		!byte	$52,$F5,$27,$AF,$B5 ;Input: A=$57 Y=$27 P=$37 M=$F8  Output: A=$AF X=$D8 Y=$27 P=$B5 M=$F8
		!byte	$50,$C4,$F6,$03 ;Input: A=$23 Y=$F6 P=$31 M=$20  Output: A=$03 X=$09 Y=$F6 P=$31 M=$20
		!byte	$50,$7B,$AD,$0F ;Input: A=$CE Y=$AD P=$75 M=$C1  Output: A=$0F X=$52 Y=$AD P=$75 M=$C1
		!byte	$50,$8C,$BE,$BD ;Input: A=$0C Y=$BE P=$F1 M=$B1  Output: A=$BD X=$41 Y=$BE P=$F1 M=$B1
		!byte	$50,$74,$A6,$DA ;Input: A=$DB Y=$A6 P=$B1 M=$01  Output: A=$DA X=$59 Y=$A6 P=$B1 M=$01
		!byte	$50,$97,$C9,$42 ;Input: A=$84 Y=$C9 P=$71 M=$C6  Output: A=$42 X=$36 Y=$C9 P=$71 M=$C6

		; $56  LSR zp,x
		!byte	$C1,$56,$37,$69,$2D ;Input: A=$19 Y=$69 P=$30 M=$5A  Output: A=$19 X=$96 Y=$69 P=$30 M=$2D
		!byte	$43,$02,$34,$34,$30 ;Input: A=$E3 Y=$34 P=$B7 M=$60  Output: A=$E3 X=$CB Y=$34 P=$34 M=$30
		!byte	$43,$FA,$2C,$31,$69 ;Input: A=$C7 Y=$2C P=$B3 M=$D3  Output: A=$C7 X=$D3 Y=$2C P=$31 M=$69
		!byte	$43,$CF,$01,$74,$4A ;Input: A=$2B Y=$01 P=$76 M=$94  Output: A=$2B X=$FE Y=$01 P=$74 M=$4A
		!byte	$43,$E5,$17,$74,$06 ;Input: A=$DF Y=$17 P=$F5 M=$0C  Output: A=$DF X=$E8 Y=$17 P=$74 M=$06
		!byte	$41,$94,$C6,$6F ;Input: A=$B0 Y=$C6 P=$34 M=$DE  Output: A=$B0 X=$39 Y=$C6 P=$34 M=$6F
		!byte	$43,$6B,$9D,$35,$3E ;Input: A=$AF Y=$9D P=$37 M=$7D  Output: A=$AF X=$62 Y=$9D P=$35 M=$3E
		!byte	$43,$74,$A6,$34,$30 ;Input: A=$C9 Y=$A6 P=$B5 M=$60  Output: A=$C9 X=$59 Y=$A6 P=$34 M=$30

		; $58  CLI
		!byte	$82,$58,$0B,$F3 ;Input: A=$95 Y=$0B P=$F7 M=$F3  Output: A=$95 X=$F4 Y=$0B P=$F3 M=$F3
		!byte	$00,$81 ;Input: A=$47 Y=$81 P=$F1 M=$1E  Output: A=$47 X=$7E Y=$81 P=$F1 M=$1E
		!byte	$00,$61 ;Input: A=$9F Y=$61 P=$71 M=$1F  Output: A=$9F X=$9E Y=$61 P=$71 M=$1F
		!byte	$02,$68,$31 ;Input: A=$9B Y=$68 P=$35 M=$C6  Output: A=$9B X=$97 Y=$68 P=$31 M=$C6
		!byte	$00,$62 ;Input: A=$82 Y=$62 P=$B0 M=$0E  Output: A=$82 X=$9D Y=$62 P=$B0 M=$0E
		!byte	$00,$54 ;Input: A=$79 Y=$54 P=$31 M=$3B  Output: A=$79 X=$AB Y=$54 P=$31 M=$3B
		!byte	$00,$E7 ;Input: A=$5B Y=$E7 P=$30 M=$5B  Output: A=$5B X=$18 Y=$E7 P=$30 M=$5B
		!byte	$02,$17,$B0 ;Input: A=$2F Y=$17 P=$B4 M=$32  Output: A=$2F X=$E8 Y=$17 P=$B0 M=$32

		; $59  EOR abs,y
		!byte	$F2,$59,$55,$00,$78,$EE,$B4 ;Input: A=$6E Y=$78 P=$34 M=$80  Output: A=$EE X=$87 Y=$78 P=$B4 M=$80
		!byte	$52,$3B,$92,$D5,$F4 ;Input: A=$DA Y=$92 P=$74 M=$0F  Output: A=$D5 X=$6D Y=$92 P=$F4 M=$0F
		!byte	$52,$B0,$1D,$57,$34 ;Input: A=$FF Y=$1D P=$36 M=$A8  Output: A=$57 X=$E2 Y=$1D P=$34 M=$A8
		!byte	$52,$A4,$29,$11,$34 ;Input: A=$CE Y=$29 P=$B6 M=$DF  Output: A=$11 X=$D6 Y=$29 P=$34 M=$DF
		!byte	$52,$5B,$72,$17,$31 ;Input: A=$A5 Y=$72 P=$33 M=$B2  Output: A=$17 X=$8D Y=$72 P=$31 M=$B2
		!byte	$50,$59,$74,$51 ;Input: A=$74 Y=$74 P=$75 M=$25  Output: A=$51 X=$8B Y=$74 P=$75 M=$25
		!byte	$72,$D3,$FF,$FA,$52,$34 ;Input: A=$A5 Y=$FA P=$36 M=$F7  Output: A=$52 X=$05 Y=$FA P=$34 M=$F7
		!byte	$72,$96,$00,$37,$53,$71 ;Input: A=$B2 Y=$37 P=$73 M=$E1  Output: A=$53 X=$C8 Y=$37 P=$71 M=$E1

		; $5D  EOR abs,x
		!byte	$F2,$5D,$83,$00,$B5,$86,$F5 ;Input: A=$E4 Y=$B5 P=$77 M=$62  Output: A=$86 X=$4A Y=$B5 P=$F5 M=$62
		!byte	$52,$63,$95,$18,$35 ;Input: A=$83 Y=$95 P=$B5 M=$9B  Output: A=$18 X=$6A Y=$95 P=$35 M=$9B
		!byte	$52,$C3,$F5,$86,$F4 ;Input: A=$B1 Y=$F5 P=$F6 M=$37  Output: A=$86 X=$0A Y=$F5 P=$F4 M=$37
		!byte	$52,$4E,$80,$D5,$B0 ;Input: A=$58 Y=$80 P=$B2 M=$8D  Output: A=$D5 X=$7F Y=$80 P=$B0 M=$8D
		!byte	$52,$3B,$6D,$13,$71 ;Input: A=$EA Y=$6D P=$73 M=$F9  Output: A=$13 X=$92 Y=$6D P=$71 M=$F9
		!byte	$52,$42,$74,$41,$70 ;Input: A=$51 Y=$74 P=$F0 M=$10  Output: A=$41 X=$8B Y=$74 P=$70 M=$10
		!byte	$52,$59,$8B,$C7,$F4 ;Input: A=$63 Y=$8B P=$74 M=$A4  Output: A=$C7 X=$74 Y=$8B P=$F4 M=$A4
		!byte	$52,$33,$65,$06,$30 ;Input: A=$9E Y=$65 P=$B2 M=$98  Output: A=$06 X=$9A Y=$65 P=$30 M=$98

		; $5E  LSR abs,x
		!byte	$E3,$5E,$D6,$FF,$08,$35,$78 ;Input: A=$4A Y=$08 P=$36 M=$F1  Output: A=$4A X=$F7 Y=$08 P=$35 M=$78
		!byte	$61,$C4,$00,$F6,$2E ;Input: A=$81 Y=$F6 P=$75 M=$5D  Output: A=$81 X=$09 Y=$F6 P=$75 M=$2E
		!byte	$43,$8B,$BD,$35,$7C ;Input: A=$89 Y=$BD P=$B4 M=$F9  Output: A=$89 X=$42 Y=$BD P=$35 M=$7C
		!byte	$43,$B9,$EB,$30,$7D ;Input: A=$3D Y=$EB P=$B0 M=$FA  Output: A=$3D X=$14 Y=$EB P=$30 M=$7D
		!byte	$43,$86,$B8,$31,$1F ;Input: A=$0B Y=$B8 P=$32 M=$3F  Output: A=$0B X=$47 Y=$B8 P=$31 M=$1F
		!byte	$43,$AF,$E1,$34,$3E ;Input: A=$3C Y=$E1 P=$35 M=$7C  Output: A=$3C X=$1E Y=$E1 P=$34 M=$3E
		!byte	$43,$5D,$8F,$34,$12 ;Input: A=$B8 Y=$8F P=$B5 M=$24  Output: A=$B8 X=$70 Y=$8F P=$34 M=$12
		!byte	$43,$0A,$3C,$31,$2E ;Input: A=$F2 Y=$3C P=$33 M=$5D  Output: A=$F2 X=$C3 Y=$3C P=$31 M=$2E

		; $61  ADC (zp,x)
		!byte	$D2,$61,$B7,$F4,$3C,$75 ;Input: A=$81 Y=$F4 P=$B6 M=$BB  Output: A=$3C X=$0B Y=$F4 P=$75 M=$BB
		!byte	$52,$25,$62,$F7,$B4 ;Input: A=$0D Y=$62 P=$76 M=$EA  Output: A=$F7 X=$9D Y=$62 P=$B4 M=$EA
		!byte	$52,$4D,$8A,$B6,$B1 ;Input: A=$F8 Y=$8A P=$72 M=$BE  Output: A=$B6 X=$75 Y=$8A P=$B1 M=$BE
		!byte	$52,$EA,$27,$23,$31 ;Input: A=$C2 Y=$27 P=$70 M=$61  Output: A=$23 X=$D8 Y=$27 P=$31 M=$61
		!byte	$52,$BA,$F7,$E7,$B0 ;Input: A=$9E Y=$F7 P=$30 M=$49  Output: A=$E7 X=$08 Y=$F7 P=$B0 M=$49
		!byte	$52,$4A,$87,$9A,$B1 ;Input: A=$BC Y=$87 P=$72 M=$DE  Output: A=$9A X=$78 Y=$87 P=$B1 M=$DE
		!byte	$52,$FE,$3B,$F3,$B4 ;Input: A=$D2 Y=$3B P=$34 M=$21  Output: A=$F3 X=$C4 Y=$3B P=$B4 M=$21
		!byte	$52,$5E,$9B,$10,$75 ;Input: A=$87 Y=$9B P=$F4 M=$89  Output: A=$10 X=$64 Y=$9B P=$75 M=$89

		; $65  ADC zp
		!byte	$D2,$65,$CD,$40,$87,$B5 ;Input: A=$FB Y=$40 P=$35 M=$8B  Output: A=$87 X=$BF Y=$40 P=$B5 M=$8B
		!byte	$12,$C6,$67,$31 ;Input: A=$72 Y=$C6 P=$B1 M=$F4  Output: A=$67 X=$39 Y=$C6 P=$31 M=$F4
		!byte	$12,$B1,$BF,$B0 ;Input: A=$06 Y=$B1 P=$F1 M=$B8  Output: A=$BF X=$4E Y=$B1 P=$B0 M=$B8
		!byte	$12,$30,$DD,$F0 ;Input: A=$5F Y=$30 P=$B1 M=$7D  Output: A=$DD X=$CF Y=$30 P=$F0 M=$7D
		!byte	$12,$10,$90,$B5 ;Input: A=$97 Y=$10 P=$77 M=$F8  Output: A=$90 X=$EF Y=$10 P=$B5 M=$F8
		!byte	$12,$75,$37,$31 ;Input: A=$D1 Y=$75 P=$33 M=$65  Output: A=$37 X=$8A Y=$75 P=$31 M=$65
		!byte	$12,$03,$6C,$34 ;Input: A=$62 Y=$03 P=$75 M=$09  Output: A=$6C X=$FC Y=$03 P=$34 M=$09
		!byte	$12,$9E,$DB,$B4 ;Input: A=$B1 Y=$9E P=$B5 M=$29  Output: A=$DB X=$61 Y=$9E P=$B4 M=$29

		; $66  ROR zp
		!byte	$C3,$66,$CD,$EF,$31,$02 ;Input: A=$ED Y=$EF P=$B0 M=$05  Output: A=$ED X=$10 Y=$EF P=$31 M=$02
		!byte	$03,$50,$35,$6A ;Input: A=$0D Y=$50 P=$34 M=$D5  Output: A=$0D X=$AF Y=$50 P=$35 M=$6A
		!byte	$03,$C8,$75,$57 ;Input: A=$6A Y=$C8 P=$76 M=$AF  Output: A=$6A X=$37 Y=$C8 P=$75 M=$57
		!byte	$03,$43,$F4,$A4 ;Input: A=$1E Y=$43 P=$F5 M=$48  Output: A=$1E X=$BC Y=$43 P=$F4 M=$A4
		!byte	$03,$28,$F4,$A3 ;Input: A=$43 Y=$28 P=$F7 M=$46  Output: A=$43 X=$D7 Y=$28 P=$F4 M=$A3
		!byte	$01,$F6,$70 ;Input: A=$11 Y=$F6 P=$30 M=$E0  Output: A=$11 X=$09 Y=$F6 P=$30 M=$70
		!byte	$03,$9E,$B4,$A0 ;Input: A=$70 Y=$9E P=$35 M=$40  Output: A=$70 X=$61 Y=$9E P=$B4 M=$A0
		!byte	$03,$A1,$F0,$C8 ;Input: A=$FA Y=$A1 P=$73 M=$90  Output: A=$FA X=$5E Y=$A1 P=$F0 M=$C8

		; $69  ADC #imm
		!byte	$D2,$69,$31,$48,$FE,$B4 ;Input: A=$CD Y=$48 P=$F4 M=$26  Output: A=$FE X=$B7 Y=$48 P=$B4 M=$26
		!byte	$52,$CD,$6C,$26,$35 ;Input: A=$59 Y=$6C P=$B4 M=$81  Output: A=$26 X=$93 Y=$6C P=$35 M=$81
		!byte	$52,$9E,$A4,$57,$75 ;Input: A=$B9 Y=$A4 P=$36 M=$F4  Output: A=$57 X=$5B Y=$A4 P=$75 M=$F4
		!byte	$50,$81,$DE,$F5 ;Input: A=$74 Y=$DE P=$B0 M=$03  Output: A=$F5 X=$21 Y=$DE P=$B0 M=$03
		!byte	$52,$8C,$A3,$7F,$71 ;Input: A=$F3 Y=$A3 P=$B0 M=$41  Output: A=$7F X=$5C Y=$A3 P=$71 M=$41
		!byte	$52,$10,$98,$FA,$B4 ;Input: A=$E9 Y=$98 P=$35 M=$47  Output: A=$FA X=$67 Y=$98 P=$B4 M=$47
		!byte	$52,$9A,$FF,$01,$35 ;Input: A=$66 Y=$FF P=$37 M=$60  Output: A=$01 X=$00 Y=$FF P=$35 M=$60
		!byte	$52,$CD,$43,$50,$75 ;Input: A=$83 Y=$43 P=$F4 M=$58  Output: A=$50 X=$BC Y=$43 P=$75 M=$58

		; $6A  ROR
		!byte	$92,$6A,$CF,$59,$31 ;Input: A=$B3 Y=$CF P=$B0 M=$11  Output: A=$59 X=$30 Y=$CF P=$31 M=$11
		!byte	$12,$C7,$12,$75 ;Input: A=$25 Y=$C7 P=$74 M=$A7  Output: A=$12 X=$38 Y=$C7 P=$75 M=$A7
		!byte	$12,$59,$45,$75 ;Input: A=$8B Y=$59 P=$74 M=$64  Output: A=$45 X=$A6 Y=$59 P=$75 M=$64
		!byte	$12,$BD,$3B,$31 ;Input: A=$77 Y=$BD P=$32 M=$0C  Output: A=$3B X=$42 Y=$BD P=$31 M=$0C
		!byte	$12,$27,$C7,$F4 ;Input: A=$8E Y=$27 P=$77 M=$16  Output: A=$C7 X=$D8 Y=$27 P=$F4 M=$16
		!byte	$12,$28,$55,$75 ;Input: A=$AB Y=$28 P=$F4 M=$A3  Output: A=$55 X=$D7 Y=$28 P=$75 M=$A3
		!byte	$12,$5D,$14,$31 ;Input: A=$29 Y=$5D P=$B2 M=$CB  Output: A=$14 X=$A2 Y=$5D P=$31 M=$CB
		!byte	$12,$5F,$9F,$B4 ;Input: A=$3E Y=$5F P=$B7 M=$D1  Output: A=$9F X=$A0 Y=$5F P=$B4 M=$D1

		; $6D  ADC abs
		!byte	$F2,$6D,$CD,$00,$49,$1D,$31 ;Input: A=$FF Y=$49 P=$B3 M=$1D  Output: A=$1D X=$B6 Y=$49 P=$31 M=$1D
		!byte	$10,$4E,$B7 ;Input: A=$25 Y=$4E P=$B0 M=$92  Output: A=$B7 X=$B1 Y=$4E P=$B0 M=$92
		!byte	$12,$3C,$F1,$B4 ;Input: A=$84 Y=$3C P=$F4 M=$6D  Output: A=$F1 X=$C3 Y=$3C P=$B4 M=$6D
		!byte	$12,$DE,$74,$75 ;Input: A=$AD Y=$DE P=$76 M=$C7  Output: A=$74 X=$21 Y=$DE P=$75 M=$C7
		!byte	$12,$98,$00,$33 ;Input: A=$D9 Y=$98 P=$71 M=$26  Output: A=$00 X=$67 Y=$98 P=$33 M=$26
		!byte	$12,$B2,$8F,$F0 ;Input: A=$62 Y=$B2 P=$F3 M=$2C  Output: A=$8F X=$4D Y=$B2 P=$F0 M=$2C
		!byte	$12,$2B,$1F,$31 ;Input: A=$E3 Y=$2B P=$F1 M=$3B  Output: A=$1F X=$D4 Y=$2B P=$31 M=$3B
		!byte	$12,$14,$57,$75 ;Input: A=$AD Y=$14 P=$37 M=$A9  Output: A=$57 X=$EB Y=$14 P=$75 M=$A9

		; $6E  ROR abs
		!byte	$E3,$6E,$CD,$00,$BB,$B4,$D7 ;Input: A=$9B Y=$BB P=$37 M=$AE  Output: A=$9B X=$44 Y=$BB P=$B4 M=$D7
		!byte	$01,$2F,$31 ;Input: A=$1A Y=$2F P=$74 M=$62  Output: A=$1A X=$D0 Y=$2F P=$74 M=$31
		!byte	$01,$E2,$5D ;Input: A=$5A Y=$E2 P=$74 M=$BA  Output: A=$5A X=$1D Y=$E2 P=$74 M=$5D
		!byte	$03,$7E,$F4,$BA ;Input: A=$27 Y=$7E P=$75 M=$74  Output: A=$27 X=$81 Y=$7E P=$F4 M=$BA
		!byte	$03,$1D,$71,$28 ;Input: A=$85 Y=$1D P=$70 M=$51  Output: A=$85 X=$E2 Y=$1D P=$71 M=$28
		!byte	$03,$84,$F5,$F1 ;Input: A=$36 Y=$84 P=$77 M=$E3  Output: A=$36 X=$7B Y=$84 P=$F5 M=$F1
		!byte	$03,$1F,$F1,$EE ;Input: A=$70 Y=$1F P=$71 M=$DD  Output: A=$70 X=$E0 Y=$1F P=$F1 M=$EE
		!byte	$03,$BE,$F0,$FE ;Input: A=$F9 Y=$BE P=$73 M=$FC  Output: A=$F9 X=$41 Y=$BE P=$F0 M=$FE

		; $71  ADC (zp),y
		!byte	$D2,$71,$C4,$01,$03,$35 ;Input: A=$BE Y=$01 P=$F7 M=$44  Output: A=$03 X=$FE Y=$01 P=$35 M=$44
		!byte	$52,$C2,$00,$37,$31 ;Input: A=$C5 Y=$00 P=$B0 M=$72  Output: A=$37 X=$FF Y=$00 P=$31 M=$72
		!byte	$52,$C4,$01,$A5,$B4 ;Input: A=$8A Y=$01 P=$B6 M=$1B  Output: A=$A5 X=$FE Y=$01 P=$B4 M=$1B
		!byte	$12,$01,$FE,$B4 ;Input: A=$D3 Y=$01 P=$77 M=$2A  Output: A=$FE X=$FE Y=$01 P=$B4 M=$2A
		!byte	$52,$C2,$00,$6D,$34 ;Input: A=$37 Y=$00 P=$F7 M=$35  Output: A=$6D X=$FF Y=$00 P=$34 M=$35
		!byte	$52,$C4,$01,$74,$30 ;Input: A=$52 Y=$01 P=$B0 M=$22  Output: A=$74 X=$FE Y=$01 P=$30 M=$22
		!byte	$52,$C2,$00,$22,$35 ;Input: A=$54 Y=$00 P=$B7 M=$CD  Output: A=$22 X=$FF Y=$00 P=$35 M=$CD
		!byte	$52,$C4,$01,$49,$31 ;Input: A=$D2 Y=$01 P=$32 M=$77  Output: A=$49 X=$FE Y=$01 P=$31 M=$77

		; $75  ADC zp,x
		!byte	$D2,$75,$AA,$DC,$5C,$71 ;Input: A=$90 Y=$DC P=$F0 M=$CC  Output: A=$5C X=$23 Y=$DC P=$71 M=$CC
		!byte	$52,$BE,$F0,$5C,$34 ;Input: A=$0B Y=$F0 P=$37 M=$50  Output: A=$5C X=$0F Y=$F0 P=$34 M=$50
		!byte	$52,$DC,$0E,$EE,$B0 ;Input: A=$07 Y=$0E P=$F3 M=$E6  Output: A=$EE X=$F1 Y=$0E P=$B0 M=$E6
		!byte	$52,$3B,$6D,$D2,$B4 ;Input: A=$97 Y=$6D P=$34 M=$3B  Output: A=$D2 X=$92 Y=$6D P=$B4 M=$3B
		!byte	$52,$FF,$31,$B1,$F0 ;Input: A=$6F Y=$31 P=$F1 M=$41  Output: A=$B1 X=$CE Y=$31 P=$F0 M=$41
		!byte	$52,$42,$74,$9B,$B0 ;Input: A=$97 Y=$74 P=$33 M=$03  Output: A=$9B X=$8B Y=$74 P=$B0 M=$03
		!byte	$52,$5B,$8D,$C4,$B1 ;Input: A=$E2 Y=$8D P=$B3 M=$E1  Output: A=$C4 X=$72 Y=$8D P=$B1 M=$E1
		!byte	$52,$72,$A4,$5F,$71 ;Input: A=$CF Y=$A4 P=$B2 M=$90  Output: A=$5F X=$5B Y=$A4 P=$71 M=$90

		; $76  ROR zp,x
		!byte	$C3,$76,$A3,$D5,$71,$60 ;Input: A=$B9 Y=$D5 P=$72 M=$C1  Output: A=$B9 X=$2A Y=$D5 P=$71 M=$60
		!byte	$43,$38,$6A,$31,$54 ;Input: A=$61 Y=$6A P=$30 M=$A9  Output: A=$61 X=$95 Y=$6A P=$31 M=$54
		!byte	$43,$CA,$FC,$70,$40 ;Input: A=$F5 Y=$FC P=$F0 M=$80  Output: A=$F5 X=$03 Y=$FC P=$70 M=$40
		!byte	$43,$44,$76,$30,$68 ;Input: A=$FE Y=$76 P=$B2 M=$D0  Output: A=$FE X=$89 Y=$76 P=$30 M=$68
		!byte	$43,$19,$4B,$F5,$BB ;Input: A=$27 Y=$4B P=$F7 M=$77  Output: A=$27 X=$B4 Y=$4B P=$F5 M=$BB
		!byte	$41,$39,$6B,$0C ;Input: A=$F4 Y=$6B P=$34 M=$18  Output: A=$F4 X=$94 Y=$6B P=$34 M=$0C
		!byte	$43,$7F,$B1,$B1,$BB ;Input: A=$53 Y=$B1 P=$33 M=$77  Output: A=$53 X=$4E Y=$B1 P=$B1 M=$BB
		!byte	$43,$A9,$DB,$75,$6C ;Input: A=$34 Y=$DB P=$74 M=$D9  Output: A=$34 X=$24 Y=$DB P=$75 M=$6C

		; $78  SEI
		!byte	$82,$78,$5A,$B5 ;Input: A=$D3 Y=$5A P=$B1 M=$C8  Output: A=$D3 X=$A5 Y=$5A P=$B5 M=$C8
		!byte	$02,$72,$F7 ;Input: A=$C7 Y=$72 P=$F3 M=$06  Output: A=$C7 X=$8D Y=$72 P=$F7 M=$06
		!byte	$00,$82 ;Input: A=$AA Y=$82 P=$B6 M=$65  Output: A=$AA X=$7D Y=$82 P=$B6 M=$65
		!byte	$00,$71 ;Input: A=$7B Y=$71 P=$B7 M=$8C  Output: A=$7B X=$8E Y=$71 P=$B7 M=$8C
		!byte	$00,$8C ;Input: A=$A7 Y=$8C P=$B6 M=$8D  Output: A=$A7 X=$73 Y=$8C P=$B6 M=$8D
		!byte	$02,$5E,$B5 ;Input: A=$BE Y=$5E P=$B1 M=$F6  Output: A=$BE X=$A1 Y=$5E P=$B5 M=$F6
		!byte	$00,$4E ;Input: A=$A9 Y=$4E P=$74 M=$20  Output: A=$A9 X=$B1 Y=$4E P=$74 M=$20
		!byte	$02,$FA,$34 ;Input: A=$7F Y=$FA P=$30 M=$94  Output: A=$7F X=$05 Y=$FA P=$34 M=$94

		; $79  ADC abs,y
		!byte	$F0,$79,$1D,$00,$B0,$51 ;Input: A=$EA Y=$B0 P=$31 M=$66  Output: A=$51 X=$4F Y=$B0 P=$31 M=$66
		!byte	$52,$59,$74,$37,$35 ;Input: A=$4E Y=$74 P=$34 M=$E9  Output: A=$37 X=$8B Y=$74 P=$35 M=$E9
		!byte	$52,$58,$75,$69,$34 ;Input: A=$5A Y=$75 P=$F4 M=$0F  Output: A=$69 X=$8A Y=$75 P=$34 M=$0F
		!byte	$72,$F1,$FF,$DC,$CB,$B4 ;Input: A=$8D Y=$DC P=$35 M=$3D  Output: A=$CB X=$23 Y=$DC P=$B4 M=$3D
		!byte	$72,$75,$00,$58,$41,$31 ;Input: A=$62 Y=$58 P=$B3 M=$DE  Output: A=$41 X=$A7 Y=$58 P=$31 M=$DE
		!byte	$72,$F3,$FF,$DA,$EF,$B1 ;Input: A=$FE Y=$DA P=$F1 M=$F0  Output: A=$EF X=$25 Y=$DA P=$B1 M=$F0
		!byte	$72,$72,$00,$5B,$37,$35 ;Input: A=$6F Y=$5B P=$75 M=$C7  Output: A=$37 X=$A4 Y=$5B P=$35 M=$C7
		!byte	$52,$2D,$A0,$9C,$F0 ;Input: A=$6C Y=$A0 P=$73 M=$2F  Output: A=$9C X=$5F Y=$A0 P=$F0 M=$2F

		; $7D  ADC abs,x
		!byte	$F2,$7D,$2A,$00,$5C,$EE,$B4 ;Input: A=$9C Y=$5C P=$F4 M=$52  Output: A=$EE X=$A3 Y=$5C P=$B4 M=$52
		!byte	$72,$EB,$FF,$1D,$FD,$B0 ;Input: A=$6C Y=$1D P=$30 M=$91  Output: A=$FD X=$E2 Y=$1D P=$B0 M=$91
		!byte	$72,$B7,$00,$E9,$61,$34 ;Input: A=$2F Y=$E9 P=$36 M=$32  Output: A=$61 X=$16 Y=$E9 P=$34 M=$32
		!byte	$52,$42,$74,$D3,$B0 ;Input: A=$94 Y=$74 P=$F0 M=$3F  Output: A=$D3 X=$8B Y=$74 P=$B0 M=$3F
		!byte	$52,$23,$55,$EA,$B0 ;Input: A=$46 Y=$55 P=$72 M=$A4  Output: A=$EA X=$AA Y=$55 P=$B0 M=$A4
		!byte	$52,$7A,$AC,$C3,$B0 ;Input: A=$24 Y=$AC P=$32 M=$9F  Output: A=$C3 X=$53 Y=$AC P=$B0 M=$9F
		!byte	$70,$D9,$FF,$0B,$0D ;Input: A=$56 Y=$0B P=$31 M=$B6  Output: A=$0D X=$F4 Y=$0B P=$31 M=$B6
		!byte	$70,$77,$00,$A9,$D8 ;Input: A=$CF Y=$A9 P=$B4 M=$09  Output: A=$D8 X=$56 Y=$A9 P=$B4 M=$09

		; $7E  ROR abs,x
		!byte	$E3,$7E,$F1,$FF,$23,$34,$7C ;Input: A=$FA Y=$23 P=$36 M=$F8  Output: A=$FA X=$DC Y=$23 P=$34 M=$7C
		!byte	$63,$29,$00,$5B,$31,$1F ;Input: A=$97 Y=$5B P=$B2 M=$3F  Output: A=$97 X=$A4 Y=$5B P=$31 M=$1F
		!byte	$43,$01,$33,$F5,$FB ;Input: A=$76 Y=$33 P=$75 M=$F7  Output: A=$76 X=$CC Y=$33 P=$F5 M=$FB
		!byte	$43,$03,$35,$75,$5C ;Input: A=$84 Y=$35 P=$F4 M=$B9  Output: A=$84 X=$CA Y=$35 P=$75 M=$5C
		!byte	$43,$36,$68,$75,$04 ;Input: A=$D0 Y=$68 P=$F6 M=$09  Output: A=$D0 X=$97 Y=$68 P=$75 M=$04
		!byte	$43,$AF,$E1,$71,$5A ;Input: A=$74 Y=$E1 P=$72 M=$B5  Output: A=$74 X=$1E Y=$E1 P=$71 M=$5A
		!byte	$43,$A3,$D5,$35,$42 ;Input: A=$33 Y=$D5 P=$B4 M=$85  Output: A=$33 X=$2A Y=$D5 P=$35 M=$42
		!byte	$43,$00,$32,$F4,$E8 ;Input: A=$F1 Y=$32 P=$F7 M=$D0  Output: A=$F1 X=$CD Y=$32 P=$F4 M=$E8

		; $81  STA (zp,x)
		!byte	$C1,$81,$CD,$0A,$C1 ;Input: A=$C1 Y=$0A P=$75 M=$3C  Output: A=$C1 X=$F5 Y=$0A P=$75 M=$C1
		!byte	$41,$1D,$5A,$20 ;Input: A=$20 Y=$5A P=$75 M=$C3  Output: A=$20 X=$A5 Y=$5A P=$75 M=$20
		!byte	$41,$AC,$E9,$F3 ;Input: A=$F3 Y=$E9 P=$76 M=$29  Output: A=$F3 X=$16 Y=$E9 P=$76 M=$F3
		!byte	$41,$20,$5D,$1B ;Input: A=$1B Y=$5D P=$31 M=$C8  Output: A=$1B X=$A2 Y=$5D P=$31 M=$1B
		!byte	$41,$78,$B5,$B9 ;Input: A=$B9 Y=$B5 P=$33 M=$AF  Output: A=$B9 X=$4A Y=$B5 P=$33 M=$B9
		!byte	$41,$88,$C5,$2E ;Input: A=$2E Y=$C5 P=$76 M=$2B  Output: A=$2E X=$3A Y=$C5 P=$76 M=$2E
		!byte	$41,$2F,$6C,$E3 ;Input: A=$E3 Y=$6C P=$F1 M=$4E  Output: A=$E3 X=$93 Y=$6C P=$F1 M=$E3
		!byte	$41,$5B,$98,$AB ;Input: A=$AB Y=$98 P=$37 M=$D7  Output: A=$AB X=$67 Y=$98 P=$37 M=$AB

		; $84  STY zp
		!byte	$C1,$84,$CD,$57,$57 ;Input: A=$66 Y=$57 P=$30 M=$2A  Output: A=$66 X=$A8 Y=$57 P=$30 M=$57
		!byte	$01,$87,$87 ;Input: A=$34 Y=$87 P=$76 M=$7C  Output: A=$34 X=$78 Y=$87 P=$76 M=$87
		!byte	$01,$DF,$DF ;Input: A=$98 Y=$DF P=$B5 M=$68  Output: A=$98 X=$20 Y=$DF P=$B5 M=$DF
		!byte	$01,$68,$68 ;Input: A=$32 Y=$68 P=$73 M=$E1  Output: A=$32 X=$97 Y=$68 P=$73 M=$68
		!byte	$01,$A2,$A2 ;Input: A=$5F Y=$A2 P=$F4 M=$68  Output: A=$5F X=$5D Y=$A2 P=$F4 M=$A2
		!byte	$01,$E6,$E6 ;Input: A=$D9 Y=$E6 P=$B1 M=$93  Output: A=$D9 X=$19 Y=$E6 P=$B1 M=$E6
		!byte	$01,$C6,$C6 ;Input: A=$E5 Y=$C6 P=$33 M=$34  Output: A=$E5 X=$39 Y=$C6 P=$33 M=$C6
		!byte	$01,$77,$77 ;Input: A=$AA Y=$77 P=$34 M=$B1  Output: A=$AA X=$88 Y=$77 P=$34 M=$77

		; $85  STA zp
		!byte	$C1,$85,$CD,$8D,$45 ;Input: A=$45 Y=$8D P=$73 M=$CB  Output: A=$45 X=$72 Y=$8D P=$73 M=$45
		!byte	$01,$8F,$40 ;Input: A=$40 Y=$8F P=$77 M=$36  Output: A=$40 X=$70 Y=$8F P=$77 M=$40
		!byte	$01,$2A,$7B ;Input: A=$7B Y=$2A P=$33 M=$69  Output: A=$7B X=$D5 Y=$2A P=$33 M=$7B
		!byte	$01,$14,$E1 ;Input: A=$E1 Y=$14 P=$36 M=$09  Output: A=$E1 X=$EB Y=$14 P=$36 M=$E1
		!byte	$01,$D4,$74 ;Input: A=$74 Y=$D4 P=$77 M=$7B  Output: A=$74 X=$2B Y=$D4 P=$77 M=$74
		!byte	$01,$11,$33 ;Input: A=$33 Y=$11 P=$B2 M=$BB  Output: A=$33 X=$EE Y=$11 P=$B2 M=$33
		!byte	$01,$18,$0E ;Input: A=$0E Y=$18 P=$73 M=$D9  Output: A=$0E X=$E7 Y=$18 P=$73 M=$0E
		!byte	$01,$AE,$32 ;Input: A=$32 Y=$AE P=$77 M=$C4  Output: A=$32 X=$51 Y=$AE P=$77 M=$32

		; $86  STX zp
		!byte	$C1,$86,$CD,$73,$8C ;Input: A=$C1 Y=$73 P=$74 M=$CF  Output: A=$C1 X=$8C Y=$73 P=$74 M=$8C
		!byte	$01,$60,$9F ;Input: A=$31 Y=$60 P=$B3 M=$14  Output: A=$31 X=$9F Y=$60 P=$B3 M=$9F
		!byte	$01,$20,$DF ;Input: A=$18 Y=$20 P=$B7 M=$C6  Output: A=$18 X=$DF Y=$20 P=$B7 M=$DF
		!byte	$01,$5D,$A2 ;Input: A=$41 Y=$5D P=$34 M=$99  Output: A=$41 X=$A2 Y=$5D P=$34 M=$A2
		!byte	$01,$2F,$D0 ;Input: A=$33 Y=$2F P=$70 M=$19  Output: A=$33 X=$D0 Y=$2F P=$70 M=$D0
		!byte	$01,$53,$AC ;Input: A=$D6 Y=$53 P=$37 M=$71  Output: A=$D6 X=$AC Y=$53 P=$37 M=$AC
		!byte	$01,$D7,$28 ;Input: A=$6B Y=$D7 P=$F3 M=$42  Output: A=$6B X=$28 Y=$D7 P=$F3 M=$28
		!byte	$01,$59,$A6 ;Input: A=$D6 Y=$59 P=$32 M=$FD  Output: A=$D6 X=$A6 Y=$59 P=$32 M=$A6

		; $88  DEY
		!byte	$86,$88,$0D,$0C,$75 ;Input: A=$27 Y=$0D P=$F5 M=$7A  Output: A=$27 X=$F2 Y=$0C P=$75 M=$7A
		!byte	$04,$26,$25 ;Input: A=$D2 Y=$26 P=$70 M=$5C  Output: A=$D2 X=$D9 Y=$25 P=$70 M=$5C
		!byte	$06,$55,$54,$34 ;Input: A=$09 Y=$55 P=$B4 M=$B1  Output: A=$09 X=$AA Y=$54 P=$34 M=$B1
		!byte	$06,$80,$7F,$71 ;Input: A=$3C Y=$80 P=$F3 M=$68  Output: A=$3C X=$7F Y=$7F P=$71 M=$68
		!byte	$06,$D9,$D8,$B1 ;Input: A=$16 Y=$D9 P=$31 M=$95  Output: A=$16 X=$26 Y=$D8 P=$B1 M=$95
		!byte	$06,$F8,$F7,$B5 ;Input: A=$87 Y=$F8 P=$35 M=$21  Output: A=$87 X=$07 Y=$F7 P=$B5 M=$21
		!byte	$06,$B8,$B7,$F1 ;Input: A=$61 Y=$B8 P=$F3 M=$82  Output: A=$61 X=$47 Y=$B7 P=$F1 M=$82
		!byte	$06,$D5,$D4,$B0 ;Input: A=$2F Y=$D5 P=$B2 M=$4B  Output: A=$2F X=$2A Y=$D4 P=$B0 M=$4B

		; $8A  TXA
		!byte	$90,$8A,$A0,$5F ;Input: A=$83 Y=$A0 P=$70 M=$F3  Output: A=$5F X=$5F Y=$A0 P=$70 M=$F3
		!byte	$10,$4E,$B1 ;Input: A=$92 Y=$4E P=$F0 M=$99  Output: A=$B1 X=$B1 Y=$4E P=$F0 M=$99
		!byte	$10,$9F,$60 ;Input: A=$21 Y=$9F P=$75 M=$F0  Output: A=$60 X=$60 Y=$9F P=$75 M=$F0
		!byte	$12,$FC,$03,$35 ;Input: A=$3A Y=$FC P=$B7 M=$61  Output: A=$03 X=$03 Y=$FC P=$35 M=$61
		!byte	$12,$36,$C9,$B4 ;Input: A=$D2 Y=$36 P=$B6 M=$B7  Output: A=$C9 X=$C9 Y=$36 P=$B4 M=$B7
		!byte	$10,$8E,$71 ;Input: A=$14 Y=$8E P=$70 M=$D6  Output: A=$71 X=$71 Y=$8E P=$70 M=$D6
		!byte	$10,$B0,$4F ;Input: A=$E1 Y=$B0 P=$31 M=$19  Output: A=$4F X=$4F Y=$B0 P=$31 M=$19
		!byte	$10,$BF,$40 ;Input: A=$E7 Y=$BF P=$35 M=$2B  Output: A=$40 X=$40 Y=$BF P=$35 M=$2B

		; $8C  STY abs
		!byte	$E1,$8C,$CD,$00,$B1,$B1 ;Input: A=$8D Y=$B1 P=$32 M=$08  Output: A=$8D X=$4E Y=$B1 P=$32 M=$B1
		!byte	$01,$85,$85 ;Input: A=$4F Y=$85 P=$71 M=$26  Output: A=$4F X=$7A Y=$85 P=$71 M=$85
		!byte	$01,$12,$12 ;Input: A=$68 Y=$12 P=$B3 M=$04  Output: A=$68 X=$ED Y=$12 P=$B3 M=$12
		!byte	$01,$8B,$8B ;Input: A=$5F Y=$8B P=$F1 M=$A6  Output: A=$5F X=$74 Y=$8B P=$F1 M=$8B
		!byte	$01,$EC,$EC ;Input: A=$07 Y=$EC P=$77 M=$85  Output: A=$07 X=$13 Y=$EC P=$77 M=$EC
		!byte	$01,$E2,$E2 ;Input: A=$5E Y=$E2 P=$F4 M=$3F  Output: A=$5E X=$1D Y=$E2 P=$F4 M=$E2
		!byte	$01,$F3,$F3 ;Input: A=$E3 Y=$F3 P=$F0 M=$B5  Output: A=$E3 X=$0C Y=$F3 P=$F0 M=$F3
		!byte	$01,$F4,$F4 ;Input: A=$BA Y=$F4 P=$31 M=$18  Output: A=$BA X=$0B Y=$F4 P=$31 M=$F4

		; $8D  STA abs
		!byte	$E1,$8D,$CD,$00,$06,$87 ;Input: A=$87 Y=$06 P=$71 M=$67  Output: A=$87 X=$F9 Y=$06 P=$71 M=$87
		!byte	$01,$B9,$CC ;Input: A=$CC Y=$B9 P=$73 M=$C2  Output: A=$CC X=$46 Y=$B9 P=$73 M=$CC
		!byte	$01,$04,$D7 ;Input: A=$D7 Y=$04 P=$77 M=$76  Output: A=$D7 X=$FB Y=$04 P=$77 M=$D7
		!byte	$01,$2D,$C2 ;Input: A=$C2 Y=$2D P=$32 M=$DF  Output: A=$C2 X=$D2 Y=$2D P=$32 M=$C2
		!byte	$01,$E0,$66 ;Input: A=$66 Y=$E0 P=$34 M=$14  Output: A=$66 X=$1F Y=$E0 P=$34 M=$66
		!byte	$01,$01,$3E ;Input: A=$3E Y=$01 P=$36 M=$63  Output: A=$3E X=$FE Y=$01 P=$36 M=$3E
		!byte	$01,$F8,$15 ;Input: A=$15 Y=$F8 P=$35 M=$B1  Output: A=$15 X=$07 Y=$F8 P=$35 M=$15
		!byte	$01,$90,$44 ;Input: A=$44 Y=$90 P=$76 M=$E2  Output: A=$44 X=$6F Y=$90 P=$76 M=$44

		; $8E  STX abs
		!byte	$E1,$8E,$CD,$00,$A2,$5D ;Input: A=$06 Y=$A2 P=$70 M=$90  Output: A=$06 X=$5D Y=$A2 P=$70 M=$5D
		!byte	$01,$16,$E9 ;Input: A=$4C Y=$16 P=$37 M=$0D  Output: A=$4C X=$E9 Y=$16 P=$37 M=$E9
		!byte	$01,$08,$F7 ;Input: A=$1F Y=$08 P=$74 M=$40  Output: A=$1F X=$F7 Y=$08 P=$74 M=$F7
		!byte	$01,$1C,$E3 ;Input: A=$8C Y=$1C P=$B0 M=$DE  Output: A=$8C X=$E3 Y=$1C P=$B0 M=$E3
		!byte	$01,$37,$C8 ;Input: A=$79 Y=$37 P=$34 M=$DB  Output: A=$79 X=$C8 Y=$37 P=$34 M=$C8
		!byte	$01,$37,$C8 ;Input: A=$BD Y=$37 P=$76 M=$15  Output: A=$BD X=$C8 Y=$37 P=$76 M=$C8
		!byte	$01,$72,$8D ;Input: A=$21 Y=$72 P=$B5 M=$62  Output: A=$21 X=$8D Y=$72 P=$B5 M=$8D
		!byte	$01,$F2,$0D ;Input: A=$96 Y=$F2 P=$F7 M=$57  Output: A=$96 X=$0D Y=$F2 P=$F7 M=$0D

		; $91  STA (zp),y
		!byte	$C1,$91,$C2,$00,$8F ;Input: A=$8F Y=$00 P=$B2 M=$D2  Output: A=$8F X=$FF Y=$00 P=$B2 M=$8F
		!byte	$41,$C4,$01,$A1 ;Input: A=$A1 Y=$01 P=$33 M=$CD  Output: A=$A1 X=$FE Y=$01 P=$33 M=$A1
		!byte	$41,$C2,$00,$62 ;Input: A=$62 Y=$00 P=$B0 M=$C5  Output: A=$62 X=$FF Y=$00 P=$B0 M=$62
		!byte	$01,$00,$30 ;Input: A=$30 Y=$00 P=$73 M=$7D  Output: A=$30 X=$FF Y=$00 P=$73 M=$30
		!byte	$41,$C4,$01,$56 ;Input: A=$56 Y=$01 P=$F6 M=$2A  Output: A=$56 X=$FE Y=$01 P=$F6 M=$56
		!byte	$41,$C2,$00,$17 ;Input: A=$17 Y=$00 P=$B6 M=$A1  Output: A=$17 X=$FF Y=$00 P=$B6 M=$17
		!byte	$41,$C4,$01,$92 ;Input: A=$92 Y=$01 P=$F6 M=$36  Output: A=$92 X=$FE Y=$01 P=$F6 M=$92
		!byte	$41,$C2,$00,$66 ;Input: A=$66 Y=$00 P=$71 M=$E9  Output: A=$66 X=$FF Y=$00 P=$71 M=$66

		; $94  STY zp,x
		!byte	$C1,$94,$D6,$08,$08 ;Input: A=$EF Y=$08 P=$70 M=$4E  Output: A=$EF X=$F7 Y=$08 P=$70 M=$08
		!byte	$41,$0D,$3F,$3F ;Input: A=$73 Y=$3F P=$F6 M=$43  Output: A=$73 X=$C0 Y=$3F P=$F6 M=$3F
		!byte	$41,$AE,$E0,$E0 ;Input: A=$B6 Y=$E0 P=$72 M=$BF  Output: A=$B6 X=$1F Y=$E0 P=$72 M=$E0
		!byte	$41,$EA,$1C,$1C ;Input: A=$66 Y=$1C P=$34 M=$27  Output: A=$66 X=$E3 Y=$1C P=$34 M=$1C
		!byte	$41,$AD,$DF,$DF ;Input: A=$47 Y=$DF P=$76 M=$17  Output: A=$47 X=$20 Y=$DF P=$76 M=$DF
		!byte	$41,$2D,$5F,$5F ;Input: A=$7C Y=$5F P=$B3 M=$CC  Output: A=$7C X=$A0 Y=$5F P=$B3 M=$5F
		!byte	$41,$AB,$DD,$DD ;Input: A=$1A Y=$DD P=$B0 M=$08  Output: A=$1A X=$22 Y=$DD P=$B0 M=$DD
		!byte	$41,$87,$B9,$B9 ;Input: A=$21 Y=$B9 P=$F4 M=$B1  Output: A=$21 X=$46 Y=$B9 P=$F4 M=$B9

		; $95  STA zp,x
		!byte	$C1,$95,$E9,$1B,$8C ;Input: A=$8C Y=$1B P=$71 M=$33  Output: A=$8C X=$E4 Y=$1B P=$71 M=$8C
		!byte	$41,$74,$A6,$B6 ;Input: A=$B6 Y=$A6 P=$72 M=$3A  Output: A=$B6 X=$59 Y=$A6 P=$72 M=$B6
		!byte	$41,$EB,$1D,$0B ;Input: A=$0B Y=$1D P=$74 M=$6D  Output: A=$0B X=$E2 Y=$1D P=$74 M=$0B
		!byte	$41,$B3,$E5,$CE ;Input: A=$CE Y=$E5 P=$71 M=$52  Output: A=$CE X=$1A Y=$E5 P=$71 M=$CE
		!byte	$41,$8A,$BC,$53 ;Input: A=$53 Y=$BC P=$71 M=$28  Output: A=$53 X=$43 Y=$BC P=$71 M=$53
		!byte	$41,$F9,$2B,$6D ;Input: A=$6D Y=$2B P=$F0 M=$BE  Output: A=$6D X=$D4 Y=$2B P=$F0 M=$6D
		!byte	$41,$45,$77,$0C ;Input: A=$0C Y=$77 P=$B7 M=$E2  Output: A=$0C X=$88 Y=$77 P=$B7 M=$0C
		!byte	$41,$E6,$18,$BA ;Input: A=$BA Y=$18 P=$B5 M=$7D  Output: A=$BA X=$E7 Y=$18 P=$B5 M=$BA

		; $96  STX zp,y
		!byte	$C1,$96,$C3,$0A,$F5 ;Input: A=$C4 Y=$0A P=$71 M=$DF  Output: A=$C4 X=$F5 Y=$0A P=$71 M=$F5
		!byte	$41,$63,$6A,$95 ;Input: A=$FB Y=$6A P=$71 M=$4C  Output: A=$FB X=$95 Y=$6A P=$71 M=$95
		!byte	$41,$8B,$42,$BD ;Input: A=$30 Y=$42 P=$F1 M=$D0  Output: A=$30 X=$BD Y=$42 P=$F1 M=$BD
		!byte	$41,$5C,$71,$8E ;Input: A=$FE Y=$71 P=$F2 M=$13  Output: A=$FE X=$8E Y=$71 P=$F2 M=$8E
		!byte	$41,$BE,$0F,$F0 ;Input: A=$62 Y=$0F P=$77 M=$6C  Output: A=$62 X=$F0 Y=$0F P=$77 M=$F0
		!byte	$41,$0A,$C3,$3C ;Input: A=$DB Y=$C3 P=$71 M=$21  Output: A=$DB X=$3C Y=$C3 P=$71 M=$3C
		!byte	$41,$F2,$DB,$24 ;Input: A=$6C Y=$DB P=$B1 M=$E0  Output: A=$6C X=$24 Y=$DB P=$B1 M=$24
		!byte	$41,$9D,$30,$CF ;Input: A=$41 Y=$30 P=$B0 M=$D4  Output: A=$41 X=$CF Y=$30 P=$B0 M=$CF

		; $98  TYA
		!byte	$92,$98,$1A,$1A,$30 ;Input: A=$1C Y=$1A P=$B0 M=$73  Output: A=$1A X=$E5 Y=$1A P=$30 M=$73
		!byte	$12,$F8,$F8,$F1 ;Input: A=$AB Y=$F8 P=$73 M=$5D  Output: A=$F8 X=$07 Y=$F8 P=$F1 M=$5D
		!byte	$10,$22,$22 ;Input: A=$54 Y=$22 P=$70 M=$EA  Output: A=$22 X=$DD Y=$22 P=$70 M=$EA
		!byte	$12,$3C,$3C,$30 ;Input: A=$B3 Y=$3C P=$B0 M=$B3  Output: A=$3C X=$C3 Y=$3C P=$30 M=$B3
		!byte	$12,$3D,$3D,$74 ;Input: A=$B8 Y=$3D P=$F4 M=$DD  Output: A=$3D X=$C2 Y=$3D P=$74 M=$DD
		!byte	$10,$B3,$B3 ;Input: A=$0C Y=$B3 P=$B5 M=$4F  Output: A=$B3 X=$4C Y=$B3 P=$B5 M=$4F
		!byte	$12,$FD,$FD,$B1 ;Input: A=$C0 Y=$FD P=$31 M=$DB  Output: A=$FD X=$02 Y=$FD P=$B1 M=$DB
		!byte	$10,$9E,$9E ;Input: A=$DD Y=$9E P=$B4 M=$1B  Output: A=$9E X=$61 Y=$9E P=$B4 M=$1B

		; $99  STA abs,y
		!byte	$E1,$99,$FD,$FF,$D0,$61 ;Input: A=$61 Y=$D0 P=$70 M=$00  Output: A=$61 X=$2F Y=$D0 P=$70 M=$61
		!byte	$41,$CF,$FE,$1B ;Input: A=$1B Y=$FE P=$B0 M=$45  Output: A=$1B X=$01 Y=$FE P=$B0 M=$1B
		!byte	$41,$F9,$D4,$9A ;Input: A=$9A Y=$D4 P=$75 M=$63  Output: A=$9A X=$2B Y=$D4 P=$75 M=$9A
		!byte	$61,$C9,$00,$04,$89 ;Input: A=$89 Y=$04 P=$71 M=$D5  Output: A=$89 X=$FB Y=$04 P=$71 M=$89
		!byte	$61,$E0,$FF,$ED,$0C ;Input: A=$0C Y=$ED P=$36 M=$FA  Output: A=$0C X=$12 Y=$ED P=$36 M=$0C
		!byte	$61,$04,$00,$C9,$CA ;Input: A=$CA Y=$C9 P=$B3 M=$91  Output: A=$CA X=$36 Y=$C9 P=$B3 M=$CA
		!byte	$41,$B9,$14,$FA ;Input: A=$FA Y=$14 P=$F0 M=$A4  Output: A=$FA X=$EB Y=$14 P=$F0 M=$FA
		!byte	$40,$8E,$3F ;Input: A=$2A Y=$3F P=$76 M=$2A  Output: A=$2A X=$C0 Y=$3F P=$76 M=$2A

		; $9D  STA abs,x
		!byte	$E1,$9D,$C0,$00,$F2,$F2 ;Input: A=$F2 Y=$F2 P=$34 M=$2A  Output: A=$F2 X=$0D Y=$F2 P=$34 M=$F2
		!byte	$41,$36,$68,$8C ;Input: A=$8C Y=$68 P=$74 M=$45  Output: A=$8C X=$97 Y=$68 P=$74 M=$8C
		!byte	$41,$7B,$AD,$08 ;Input: A=$08 Y=$AD P=$B4 M=$B6  Output: A=$08 X=$52 Y=$AD P=$B4 M=$08
		!byte	$41,$93,$C5,$65 ;Input: A=$65 Y=$C5 P=$72 M=$86  Output: A=$65 X=$3A Y=$C5 P=$72 M=$65
		!byte	$61,$D0,$FF,$02,$86 ;Input: A=$86 Y=$02 P=$F1 M=$FB  Output: A=$86 X=$FD Y=$02 P=$F1 M=$86
		!byte	$41,$E0,$12,$30 ;Input: A=$30 Y=$12 P=$32 M=$3D  Output: A=$30 X=$ED Y=$12 P=$32 M=$30
		!byte	$61,$62,$00,$94,$67 ;Input: A=$67 Y=$94 P=$B0 M=$CC  Output: A=$67 X=$6B Y=$94 P=$B0 M=$67
		!byte	$61,$FE,$FF,$30,$A0 ;Input: A=$A0 Y=$30 P=$F7 M=$37  Output: A=$A0 X=$CF Y=$30 P=$F7 M=$A0

		; $A0  LDY #imm
		!byte	$C4,$A0,$AF,$83,$AF ;Input: A=$3D Y=$83 P=$B1 M=$98  Output: A=$3D X=$7C Y=$AF P=$B1 M=$98
		!byte	$46,$95,$63,$95,$F4 ;Input: A=$F4 Y=$63 P=$74 M=$F8  Output: A=$F4 X=$9C Y=$95 P=$F4 M=$F8
		!byte	$46,$E3,$41,$E3,$F1 ;Input: A=$85 Y=$41 P=$73 M=$44  Output: A=$85 X=$BE Y=$E3 P=$F1 M=$44
		!byte	$44,$DD,$BD,$DD ;Input: A=$9A Y=$BD P=$B5 M=$DE  Output: A=$9A X=$42 Y=$DD P=$B5 M=$DE
		!byte	$46,$58,$A1,$58,$71 ;Input: A=$88 Y=$A1 P=$F1 M=$E9  Output: A=$88 X=$5E Y=$58 P=$71 M=$E9
		!byte	$46,$4F,$DA,$4F,$35 ;Input: A=$DB Y=$DA P=$37 M=$D1  Output: A=$DB X=$25 Y=$4F P=$35 M=$D1
		!byte	$44,$38,$31,$38 ;Input: A=$CB Y=$31 P=$35 M=$AA  Output: A=$CB X=$CE Y=$38 P=$35 M=$AA
		!byte	$44,$FD,$CE,$FD ;Input: A=$9D Y=$CE P=$B5 M=$72  Output: A=$9D X=$31 Y=$FD P=$B5 M=$72

		; $A1  LDA (zp,x)
		!byte	$D2,$A1,$8D,$CA,$1F,$35 ;Input: A=$8D Y=$CA P=$B7 M=$1F  Output: A=$1F X=$35 Y=$CA P=$35 M=$1F
		!byte	$52,$1F,$5C,$27,$75 ;Input: A=$01 Y=$5C P=$77 M=$27  Output: A=$27 X=$A3 Y=$5C P=$75 M=$27
		!byte	$52,$28,$65,$73,$71 ;Input: A=$5E Y=$65 P=$F3 M=$73  Output: A=$73 X=$9A Y=$65 P=$71 M=$73
		!byte	$52,$2C,$69,$1B,$30 ;Input: A=$6A Y=$69 P=$32 M=$1B  Output: A=$1B X=$96 Y=$69 P=$30 M=$1B
		!byte	$50,$F3,$30,$98 ;Input: A=$92 Y=$30 P=$F5 M=$98  Output: A=$98 X=$CF Y=$30 P=$F5 M=$98
		!byte	$52,$72,$AF,$90,$F1 ;Input: A=$F1 Y=$AF P=$F3 M=$90  Output: A=$90 X=$50 Y=$AF P=$F1 M=$90
		!byte	$52,$A5,$E2,$A7,$B5 ;Input: A=$DB Y=$E2 P=$35 M=$A7  Output: A=$A7 X=$1D Y=$E2 P=$B5 M=$A7
		!byte	$52,$79,$B6,$02,$71 ;Input: A=$AD Y=$B6 P=$F1 M=$02  Output: A=$02 X=$49 Y=$B6 P=$71 M=$02

		; $A2  LDX #imm
		!byte	$CA,$A2,$BE,$4B,$BE,$B1 ;Input: A=$23 Y=$4B P=$33 M=$6B  Output: A=$23 X=$BE Y=$4B P=$B1 M=$6B
		!byte	$48,$56,$00,$56 ;Input: A=$08 Y=$00 P=$31 M=$91  Output: A=$08 X=$56 Y=$00 P=$31 M=$91
		!byte	$48,$87,$2F,$87 ;Input: A=$40 Y=$2F P=$B4 M=$0F  Output: A=$40 X=$87 Y=$2F P=$B4 M=$0F
		!byte	$48,$6A,$84,$6A ;Input: A=$36 Y=$84 P=$71 M=$4E  Output: A=$36 X=$6A Y=$84 P=$71 M=$4E
		!byte	$48,$36,$54,$36 ;Input: A=$38 Y=$54 P=$75 M=$42  Output: A=$38 X=$36 Y=$54 P=$75 M=$42
		!byte	$48,$6C,$5F,$6C ;Input: A=$CC Y=$5F P=$30 M=$7B  Output: A=$CC X=$6C Y=$5F P=$30 M=$7B
		!byte	$4A,$3C,$DA,$3C,$35 ;Input: A=$FB Y=$DA P=$B5 M=$C4  Output: A=$FB X=$3C Y=$DA P=$35 M=$C4
		!byte	$48,$0A,$C5,$0A ;Input: A=$BE Y=$C5 P=$71 M=$1D  Output: A=$BE X=$0A Y=$C5 P=$71 M=$1D

		; $A4  LDY zp
		!byte	$C6,$A4,$CD,$CB,$7C,$34 ;Input: A=$F3 Y=$CB P=$B4 M=$7C  Output: A=$F3 X=$34 Y=$7C P=$34 M=$7C
		!byte	$06,$3D,$86,$B5 ;Input: A=$F0 Y=$3D P=$35 M=$86  Output: A=$F0 X=$C2 Y=$86 P=$B5 M=$86
		!byte	$04,$F3,$84 ;Input: A=$42 Y=$F3 P=$F0 M=$84  Output: A=$42 X=$0C Y=$84 P=$F0 M=$84
		!byte	$06,$0E,$D6,$F1 ;Input: A=$A8 Y=$0E P=$73 M=$D6  Output: A=$A8 X=$F1 Y=$D6 P=$F1 M=$D6
		!byte	$06,$F2,$1A,$71 ;Input: A=$56 Y=$F2 P=$73 M=$1A  Output: A=$56 X=$0D Y=$1A P=$71 M=$1A
		!byte	$04,$F4,$AC ;Input: A=$95 Y=$F4 P=$F0 M=$AC  Output: A=$95 X=$0B Y=$AC P=$F0 M=$AC
		!byte	$06,$8F,$2C,$31 ;Input: A=$22 Y=$8F P=$B3 M=$2C  Output: A=$22 X=$70 Y=$2C P=$31 M=$2C
		!byte	$04,$18,$60 ;Input: A=$C8 Y=$18 P=$30 M=$60  Output: A=$C8 X=$E7 Y=$60 P=$30 M=$60

		; $A5  LDA zp
		!byte	$D0,$A5,$CD,$4F,$68 ;Input: A=$97 Y=$4F P=$30 M=$68  Output: A=$68 X=$B0 Y=$4F P=$30 M=$68
		!byte	$10,$3F,$46 ;Input: A=$CB Y=$3F P=$31 M=$46  Output: A=$46 X=$C0 Y=$3F P=$31 M=$46
		!byte	$10,$47,$3D ;Input: A=$09 Y=$47 P=$75 M=$3D  Output: A=$3D X=$B8 Y=$47 P=$75 M=$3D
		!byte	$12,$42,$AD,$F5 ;Input: A=$20 Y=$42 P=$75 M=$AD  Output: A=$AD X=$BD Y=$42 P=$F5 M=$AD
		!byte	$12,$2E,$FB,$F4 ;Input: A=$F2 Y=$2E P=$F6 M=$FB  Output: A=$FB X=$D1 Y=$2E P=$F4 M=$FB
		!byte	$10,$BE,$13 ;Input: A=$29 Y=$BE P=$70 M=$13  Output: A=$13 X=$41 Y=$BE P=$70 M=$13
		!byte	$12,$B6,$CD,$B1 ;Input: A=$71 Y=$B6 P=$33 M=$CD  Output: A=$CD X=$49 Y=$B6 P=$B1 M=$CD
		!byte	$12,$0C,$D1,$B4 ;Input: A=$71 Y=$0C P=$36 M=$D1  Output: A=$D1 X=$F3 Y=$0C P=$B4 M=$D1

		; $A6  LDX zp
		!byte	$C8,$A6,$CD,$1C,$A1 ;Input: A=$9A Y=$1C P=$F0 M=$A1  Output: A=$9A X=$A1 Y=$1C P=$F0 M=$A1
		!byte	$0A,$74,$42,$35 ;Input: A=$3B Y=$74 P=$B7 M=$42  Output: A=$3B X=$42 Y=$74 P=$35 M=$42
		!byte	$0A,$FE,$8B,$B5 ;Input: A=$D0 Y=$FE P=$37 M=$8B  Output: A=$D0 X=$8B Y=$FE P=$B5 M=$8B
		!byte	$0A,$9D,$E8,$F4 ;Input: A=$B3 Y=$9D P=$74 M=$E8  Output: A=$B3 X=$E8 Y=$9D P=$F4 M=$E8
		!byte	$08,$6E,$30 ;Input: A=$C0 Y=$6E P=$74 M=$30  Output: A=$C0 X=$30 Y=$6E P=$74 M=$30
		!byte	$0A,$48,$DC,$B0 ;Input: A=$50 Y=$48 P=$B2 M=$DC  Output: A=$50 X=$DC Y=$48 P=$B0 M=$DC
		!byte	$08,$44,$4B ;Input: A=$DC Y=$44 P=$31 M=$4B  Output: A=$DC X=$4B Y=$44 P=$31 M=$4B
		!byte	$0A,$3E,$4D,$35 ;Input: A=$21 Y=$3E P=$37 M=$4D  Output: A=$21 X=$4D Y=$3E P=$35 M=$4D

		; $A8  TAY
		!byte	$86,$A8,$B8,$DE,$F1 ;Input: A=$DE Y=$B8 P=$73 M=$DE  Output: A=$DE X=$47 Y=$DE P=$F1 M=$DE
		!byte	$04,$87,$1B ;Input: A=$1B Y=$87 P=$71 M=$34  Output: A=$1B X=$78 Y=$1B P=$71 M=$34
		!byte	$06,$30,$71,$31 ;Input: A=$71 Y=$30 P=$B3 M=$FC  Output: A=$71 X=$CF Y=$71 P=$31 M=$FC
		!byte	$06,$DD,$5F,$34 ;Input: A=$5F Y=$DD P=$36 M=$9E  Output: A=$5F X=$22 Y=$5F P=$34 M=$9E
		!byte	$06,$56,$C3,$F5 ;Input: A=$C3 Y=$56 P=$75 M=$FF  Output: A=$C3 X=$A9 Y=$C3 P=$F5 M=$FF
		!byte	$06,$61,$F0,$B5 ;Input: A=$F0 Y=$61 P=$B7 M=$A1  Output: A=$F0 X=$9E Y=$F0 P=$B5 M=$A1
		!byte	$04,$6E,$E2 ;Input: A=$E2 Y=$6E P=$B4 M=$15  Output: A=$E2 X=$91 Y=$E2 P=$B4 M=$15
		!byte	$06,$8C,$46,$70 ;Input: A=$46 Y=$8C P=$F0 M=$92  Output: A=$46 X=$73 Y=$46 P=$70 M=$92

		; $A9  LDA #imm
		!byte	$D2,$A9,$C4,$E7,$C4,$F4 ;Input: A=$04 Y=$E7 P=$76 M=$22  Output: A=$C4 X=$18 Y=$E7 P=$F4 M=$22
		!byte	$50,$BC,$67,$BC ;Input: A=$AD Y=$67 P=$B1 M=$26  Output: A=$BC X=$98 Y=$67 P=$B1 M=$26
		!byte	$52,$F2,$2F,$F2,$F1 ;Input: A=$84 Y=$2F P=$73 M=$CA  Output: A=$F2 X=$D0 Y=$2F P=$F1 M=$CA
		!byte	$52,$C9,$FF,$C9,$B4 ;Input: A=$37 Y=$FF P=$34 M=$46  Output: A=$C9 X=$00 Y=$FF P=$B4 M=$46
		!byte	$52,$0F,$CC,$0F,$70 ;Input: A=$88 Y=$CC P=$F2 M=$33  Output: A=$0F X=$33 Y=$CC P=$70 M=$33
		!byte	$50,$90,$13,$90 ;Input: A=$66 Y=$13 P=$F5 M=$8E  Output: A=$90 X=$EC Y=$13 P=$F5 M=$8E
		!byte	$52,$24,$A9,$24,$30 ;Input: A=$EB Y=$A9 P=$B0 M=$52  Output: A=$24 X=$56 Y=$A9 P=$30 M=$52
		!byte	$52,$1E,$11,$1E,$31 ;Input: A=$14 Y=$11 P=$33 M=$E7  Output: A=$1E X=$EE Y=$11 P=$31 M=$E7

		; $AA  TAX
		!byte	$8A,$AA,$AD,$3F,$31 ;Input: A=$3F Y=$AD P=$B3 M=$85  Output: A=$3F X=$3F Y=$AD P=$31 M=$85
		!byte	$0A,$41,$9E,$F0 ;Input: A=$9E Y=$41 P=$72 M=$C8  Output: A=$9E X=$9E Y=$41 P=$F0 M=$C8
		!byte	$0A,$92,$63,$74 ;Input: A=$63 Y=$92 P=$76 M=$C1  Output: A=$63 X=$63 Y=$92 P=$74 M=$C1
		!byte	$0A,$3F,$72,$74 ;Input: A=$72 Y=$3F P=$76 M=$90  Output: A=$72 X=$72 Y=$3F P=$74 M=$90
		!byte	$0A,$F5,$5F,$31 ;Input: A=$5F Y=$F5 P=$B3 M=$14  Output: A=$5F X=$5F Y=$F5 P=$31 M=$14
		!byte	$0A,$AE,$B5,$B1 ;Input: A=$B5 Y=$AE P=$B3 M=$D2  Output: A=$B5 X=$B5 Y=$AE P=$B1 M=$D2
		!byte	$08,$A9,$EF ;Input: A=$EF Y=$A9 P=$F1 M=$F0  Output: A=$EF X=$EF Y=$A9 P=$F1 M=$F0
		!byte	$0A,$2B,$46,$30 ;Input: A=$46 Y=$2B P=$B0 M=$EF  Output: A=$46 X=$46 Y=$2B P=$30 M=$EF

		; $AC  LDY abs
		!byte	$E6,$AC,$CD,$00,$51,$3D,$74 ;Input: A=$DA Y=$51 P=$F6 M=$3D  Output: A=$DA X=$AE Y=$3D P=$74 M=$3D
		!byte	$06,$80,$07,$70 ;Input: A=$8A Y=$80 P=$72 M=$07  Output: A=$8A X=$7F Y=$07 P=$70 M=$07
		!byte	$06,$2E,$9F,$F5 ;Input: A=$8E Y=$2E P=$F7 M=$9F  Output: A=$8E X=$D1 Y=$9F P=$F5 M=$9F
		!byte	$04,$1A,$4A ;Input: A=$A2 Y=$1A P=$74 M=$4A  Output: A=$A2 X=$E5 Y=$4A P=$74 M=$4A
		!byte	$04,$35,$76 ;Input: A=$F7 Y=$35 P=$71 M=$76  Output: A=$F7 X=$CA Y=$76 P=$71 M=$76
		!byte	$06,$BE,$E4,$B4 ;Input: A=$AB Y=$BE P=$36 M=$E4  Output: A=$AB X=$41 Y=$E4 P=$B4 M=$E4
		!byte	$06,$65,$55,$34 ;Input: A=$50 Y=$65 P=$36 M=$55  Output: A=$50 X=$9A Y=$55 P=$34 M=$55
		!byte	$06,$77,$91,$B1 ;Input: A=$3F Y=$77 P=$31 M=$91  Output: A=$3F X=$88 Y=$91 P=$B1 M=$91

		; $AD  LDA abs
		!byte	$F2,$AD,$CD,$00,$57,$33,$34 ;Input: A=$03 Y=$57 P=$36 M=$33  Output: A=$33 X=$A8 Y=$57 P=$34 M=$33
		!byte	$12,$D3,$79,$75 ;Input: A=$7C Y=$D3 P=$F7 M=$79  Output: A=$79 X=$2C Y=$D3 P=$75 M=$79
		!byte	$10,$17,$9E ;Input: A=$37 Y=$17 P=$B0 M=$9E  Output: A=$9E X=$E8 Y=$17 P=$B0 M=$9E
		!byte	$12,$4E,$73,$70 ;Input: A=$14 Y=$4E P=$72 M=$73  Output: A=$73 X=$B1 Y=$4E P=$70 M=$73
		!byte	$12,$32,$A7,$F5 ;Input: A=$C2 Y=$32 P=$75 M=$A7  Output: A=$A7 X=$CD Y=$32 P=$F5 M=$A7
		!byte	$12,$0F,$4D,$34 ;Input: A=$62 Y=$0F P=$36 M=$4D  Output: A=$4D X=$F0 Y=$0F P=$34 M=$4D
		!byte	$12,$10,$F7,$B5 ;Input: A=$41 Y=$10 P=$B7 M=$F7  Output: A=$F7 X=$EF Y=$10 P=$B5 M=$F7
		!byte	$12,$D2,$61,$30 ;Input: A=$99 Y=$D2 P=$32 M=$61  Output: A=$61 X=$2D Y=$D2 P=$30 M=$61

		; $AE  LDX abs
		!byte	$E8,$AE,$CD,$00,$84,$10 ;Input: A=$DE Y=$84 P=$70 M=$10  Output: A=$DE X=$10 Y=$84 P=$70 M=$10
		!byte	$0A,$26,$25,$31 ;Input: A=$3C Y=$26 P=$33 M=$25  Output: A=$3C X=$25 Y=$26 P=$31 M=$25
		!byte	$08,$A2,$97 ;Input: A=$9C Y=$A2 P=$F1 M=$97  Output: A=$9C X=$97 Y=$A2 P=$F1 M=$97
		!byte	$0A,$D8,$4B,$74 ;Input: A=$64 Y=$D8 P=$76 M=$4B  Output: A=$64 X=$4B Y=$D8 P=$74 M=$4B
		!byte	$08,$D1,$26 ;Input: A=$E9 Y=$D1 P=$70 M=$26  Output: A=$E9 X=$26 Y=$D1 P=$70 M=$26
		!byte	$08,$AF,$1B ;Input: A=$6E Y=$AF P=$31 M=$1B  Output: A=$6E X=$1B Y=$AF P=$31 M=$1B
		!byte	$0A,$21,$9B,$B0 ;Input: A=$CB Y=$21 P=$32 M=$9B  Output: A=$CB X=$9B Y=$21 P=$B0 M=$9B
		!byte	$0A,$59,$A4,$F5 ;Input: A=$67 Y=$59 P=$F7 M=$A4  Output: A=$67 X=$A4 Y=$59 P=$F5 M=$A4

		; $B1  LDA (zp),y
		!byte	$D0,$B1,$C2,$00,$38 ;Input: A=$2C Y=$00 P=$75 M=$38  Output: A=$38 X=$FF Y=$00 P=$75 M=$38
		!byte	$10,$00,$CF ;Input: A=$66 Y=$00 P=$F1 M=$CF  Output: A=$CF X=$FF Y=$00 P=$F1 M=$CF
		!byte	$50,$C4,$01,$9B ;Input: A=$E2 Y=$01 P=$B0 M=$9B  Output: A=$9B X=$FE Y=$01 P=$B0 M=$9B
		!byte	$52,$C2,$00,$33,$70 ;Input: A=$32 Y=$00 P=$F0 M=$33  Output: A=$33 X=$FF Y=$00 P=$70 M=$33
		!byte	$12,$00,$BA,$F4 ;Input: A=$92 Y=$00 P=$76 M=$BA  Output: A=$BA X=$FF Y=$00 P=$F4 M=$BA
		!byte	$52,$C4,$01,$40,$31 ;Input: A=$D3 Y=$01 P=$B1 M=$40  Output: A=$40 X=$FE Y=$01 P=$31 M=$40
		!byte	$12,$01,$36,$71 ;Input: A=$BD Y=$01 P=$F3 M=$36  Output: A=$36 X=$FE Y=$01 P=$71 M=$36
		!byte	$52,$C2,$00,$C5,$F1 ;Input: A=$9F Y=$00 P=$F3 M=$C5  Output: A=$C5 X=$FF Y=$00 P=$F1 M=$C5

		; $B4  LDY zp,x
		!byte	$C6,$B4,$76,$A8,$31,$31 ;Input: A=$B9 Y=$A8 P=$B1 M=$31  Output: A=$B9 X=$57 Y=$31 P=$31 M=$31
		!byte	$46,$8E,$C0,$6C,$30 ;Input: A=$20 Y=$C0 P=$32 M=$6C  Output: A=$20 X=$3F Y=$6C P=$30 M=$6C
		!byte	$46,$02,$34,$7E,$74 ;Input: A=$FD Y=$34 P=$F4 M=$7E  Output: A=$FD X=$CB Y=$7E P=$74 M=$7E
		!byte	$44,$EE,$20,$73 ;Input: A=$38 Y=$20 P=$70 M=$73  Output: A=$38 X=$DF Y=$73 P=$70 M=$73
		!byte	$46,$95,$C7,$A5,$F4 ;Input: A=$F0 Y=$C7 P=$F6 M=$A5  Output: A=$F0 X=$38 Y=$A5 P=$F4 M=$A5
		!byte	$46,$1D,$4F,$43,$30 ;Input: A=$4E Y=$4F P=$32 M=$43  Output: A=$4E X=$B0 Y=$43 P=$30 M=$43
		!byte	$46,$52,$84,$56,$30 ;Input: A=$AB Y=$84 P=$B0 M=$56  Output: A=$AB X=$7B Y=$56 P=$30 M=$56
		!byte	$46,$6E,$A0,$D7,$B0 ;Input: A=$68 Y=$A0 P=$B2 M=$D7  Output: A=$68 X=$5F Y=$D7 P=$B0 M=$D7

		; $B5  LDA zp,x
		!byte	$D2,$B5,$37,$69,$93,$F5 ;Input: A=$32 Y=$69 P=$F7 M=$93  Output: A=$93 X=$96 Y=$69 P=$F5 M=$93
		!byte	$52,$F0,$22,$68,$74 ;Input: A=$CC Y=$22 P=$F4 M=$68  Output: A=$68 X=$DD Y=$22 P=$74 M=$68
		!byte	$50,$F2,$24,$B1 ;Input: A=$1C Y=$24 P=$F5 M=$B1  Output: A=$B1 X=$DB Y=$24 P=$F5 M=$B1
		!byte	$50,$E6,$18,$2C ;Input: A=$3B Y=$18 P=$75 M=$2C  Output: A=$2C X=$E7 Y=$18 P=$75 M=$2C
		!byte	$52,$F6,$28,$95,$B1 ;Input: A=$C4 Y=$28 P=$33 M=$95  Output: A=$95 X=$D7 Y=$28 P=$B1 M=$95
		!byte	$52,$77,$A9,$1F,$75 ;Input: A=$BF Y=$A9 P=$F5 M=$1F  Output: A=$1F X=$56 Y=$A9 P=$75 M=$1F
		!byte	$50,$CF,$01,$7E ;Input: A=$F1 Y=$01 P=$75 M=$7E  Output: A=$7E X=$FE Y=$01 P=$75 M=$7E
		!byte	$50,$A7,$D9,$E7 ;Input: A=$7E Y=$D9 P=$B1 M=$E7  Output: A=$E7 X=$26 Y=$D9 P=$B1 M=$E7

		; $B6  LDX zp,y
		!byte	$CA,$B6,$E9,$E4,$30,$34 ;Input: A=$E4 Y=$E4 P=$36 M=$30  Output: A=$E4 X=$30 Y=$E4 P=$34 M=$30
		!byte	$4A,$15,$B8,$8F,$B1 ;Input: A=$84 Y=$B8 P=$B3 M=$8F  Output: A=$84 X=$8F Y=$B8 P=$B1 M=$8F
		!byte	$48,$42,$8B,$48 ;Input: A=$24 Y=$8B P=$74 M=$48  Output: A=$24 X=$48 Y=$8B P=$74 M=$48
		!byte	$48,$F4,$D9,$FA ;Input: A=$77 Y=$D9 P=$F5 M=$FA  Output: A=$77 X=$FA Y=$D9 P=$F5 M=$FA
		!byte	$48,$8D,$40,$42 ;Input: A=$A9 Y=$40 P=$75 M=$42  Output: A=$A9 X=$42 Y=$40 P=$75 M=$42
		!byte	$4A,$C7,$06,$65,$74 ;Input: A=$70 Y=$06 P=$F6 M=$65  Output: A=$70 X=$65 Y=$06 P=$74 M=$65
		!byte	$48,$96,$37,$DD ;Input: A=$73 Y=$37 P=$F1 M=$DD  Output: A=$73 X=$DD Y=$37 P=$F1 M=$DD
		!byte	$48,$9B,$32,$5F ;Input: A=$97 Y=$32 P=$70 M=$5F  Output: A=$97 X=$5F Y=$32 P=$70 M=$5F

		; $B8  CLV
		!byte	$82,$B8,$20,$31 ;Input: A=$5A Y=$20 P=$71 M=$4D  Output: A=$5A X=$DF Y=$20 P=$31 M=$4D
		!byte	$02,$AF,$33 ;Input: A=$15 Y=$AF P=$73 M=$96  Output: A=$15 X=$50 Y=$AF P=$33 M=$96
		!byte	$02,$FD,$32 ;Input: A=$61 Y=$FD P=$72 M=$AC  Output: A=$61 X=$02 Y=$FD P=$32 M=$AC
		!byte	$00,$A2 ;Input: A=$B6 Y=$A2 P=$B4 M=$99  Output: A=$B6 X=$5D Y=$A2 P=$B4 M=$99
		!byte	$00,$31 ;Input: A=$B0 Y=$31 P=$B0 M=$B0  Output: A=$B0 X=$CE Y=$31 P=$B0 M=$B0
		!byte	$00,$B3 ;Input: A=$1C Y=$B3 P=$B7 M=$03  Output: A=$1C X=$4C Y=$B3 P=$B7 M=$03
		!byte	$00,$DA ;Input: A=$D7 Y=$DA P=$B1 M=$71  Output: A=$D7 X=$25 Y=$DA P=$B1 M=$71
		!byte	$02,$09,$B6 ;Input: A=$31 Y=$09 P=$F6 M=$DA  Output: A=$31 X=$F6 Y=$09 P=$B6 M=$DA

		; $B9  LDA abs,y
		!byte	$F0,$B9,$6C,$00,$61,$95 ;Input: A=$8A Y=$61 P=$B1 M=$95  Output: A=$95 X=$9E Y=$61 P=$B1 M=$95
		!byte	$50,$58,$75,$96 ;Input: A=$52 Y=$75 P=$F5 M=$96  Output: A=$96 X=$8A Y=$75 P=$F5 M=$96
		!byte	$52,$7D,$50,$1C,$71 ;Input: A=$DF Y=$50 P=$F1 M=$1C  Output: A=$1C X=$AF Y=$50 P=$71 M=$1C
		!byte	$72,$E0,$FF,$ED,$B1,$B4 ;Input: A=$B2 Y=$ED P=$34 M=$B1  Output: A=$B1 X=$12 Y=$ED P=$B4 M=$B1
		!byte	$70,$6A,$00,$63,$FD ;Input: A=$37 Y=$63 P=$F5 M=$FD  Output: A=$FD X=$9C Y=$63 P=$F5 M=$FD
		!byte	$50,$90,$3D,$AC ;Input: A=$EC Y=$3D P=$B4 M=$AC  Output: A=$AC X=$C2 Y=$3D P=$B4 M=$AC
		!byte	$72,$EC,$FF,$E1,$4E,$75 ;Input: A=$A8 Y=$E1 P=$F5 M=$4E  Output: A=$4E X=$1E Y=$E1 P=$75 M=$4E
		!byte	$52,$D4,$F9,$D8,$F5 ;Input: A=$74 Y=$F9 P=$77 M=$D8  Output: A=$D8 X=$06 Y=$F9 P=$F5 M=$D8

		; $BC  LDY abs,x
		!byte	$E6,$BC,$FA,$FF,$2C,$A5,$F0 ;Input: A=$64 Y=$2C P=$F2 M=$A5  Output: A=$64 X=$D3 Y=$A5 P=$F0 M=$A5
		!byte	$64,$84,$00,$B6,$64 ;Input: A=$B4 Y=$B6 P=$70 M=$64  Output: A=$B4 X=$49 Y=$64 P=$70 M=$64
		!byte	$46,$65,$97,$09,$30 ;Input: A=$37 Y=$97 P=$32 M=$09  Output: A=$37 X=$68 Y=$09 P=$30 M=$09
		!byte	$46,$47,$79,$6A,$74 ;Input: A=$0E Y=$79 P=$F6 M=$6A  Output: A=$0E X=$86 Y=$6A P=$74 M=$6A
		!byte	$06,$79,$21,$71 ;Input: A=$AF Y=$79 P=$F1 M=$21  Output: A=$AF X=$86 Y=$21 P=$71 M=$21
		!byte	$44,$3E,$70,$6B ;Input: A=$53 Y=$70 P=$31 M=$6B  Output: A=$53 X=$8F Y=$6B P=$31 M=$6B
		!byte	$46,$4E,$80,$AF,$F0 ;Input: A=$03 Y=$80 P=$70 M=$AF  Output: A=$03 X=$7F Y=$AF P=$F0 M=$AF
		!byte	$46,$AE,$E0,$5B,$71 ;Input: A=$BA Y=$E0 P=$73 M=$5B  Output: A=$BA X=$1F Y=$5B P=$71 M=$5B

		; $BD  LDA abs,x
		!byte	$F2,$BD,$15,$00,$47,$48,$75 ;Input: A=$9B Y=$47 P=$F5 M=$48  Output: A=$48 X=$B8 Y=$47 P=$75 M=$48
		!byte	$50,$2D,$5F,$58 ;Input: A=$D6 Y=$5F P=$30 M=$58  Output: A=$58 X=$A0 Y=$5F P=$30 M=$58
		!byte	$72,$F8,$FF,$2A,$EC,$F1 ;Input: A=$2F Y=$2A P=$71 M=$EC  Output: A=$EC X=$D5 Y=$2A P=$F1 M=$EC
		!byte	$72,$11,$00,$43,$38,$70 ;Input: A=$6D Y=$43 P=$F2 M=$38  Output: A=$38 X=$BC Y=$43 P=$70 M=$38
		!byte	$52,$27,$59,$48,$31 ;Input: A=$FB Y=$59 P=$B3 M=$48  Output: A=$48 X=$A6 Y=$59 P=$31 M=$48
		!byte	$52,$28,$5A,$26,$35 ;Input: A=$EC Y=$5A P=$B5 M=$26  Output: A=$26 X=$A5 Y=$5A P=$35 M=$26
		!byte	$72,$F9,$FF,$2B,$ED,$F1 ;Input: A=$33 Y=$2B P=$F3 M=$ED  Output: A=$ED X=$D4 Y=$2B P=$F1 M=$ED
		!byte	$72,$AA,$00,$DC,$C1,$B5 ;Input: A=$45 Y=$DC P=$35 M=$C1  Output: A=$C1 X=$23 Y=$DC P=$B5 M=$C1

		; $BE  LDX abs,y
		!byte	$EA,$BE,$18,$00,$B5,$EA,$F1 ;Input: A=$0A Y=$B5 P=$F3 M=$EA  Output: A=$0A X=$EA Y=$B5 P=$F1 M=$EA
		!byte	$4A,$30,$9D,$07,$70 ;Input: A=$4F Y=$9D P=$72 M=$07  Output: A=$4F X=$07 Y=$9D P=$70 M=$07
		!byte	$48,$31,$9C,$BD ;Input: A=$B0 Y=$9C P=$F1 M=$BD  Output: A=$B0 X=$BD Y=$9C P=$F1 M=$BD
		!byte	$4A,$46,$87,$01,$71 ;Input: A=$A6 Y=$87 P=$F1 M=$01  Output: A=$A6 X=$01 Y=$87 P=$71 M=$01
		!byte	$4A,$A6,$27,$83,$F0 ;Input: A=$C5 Y=$27 P=$F2 M=$83  Output: A=$C5 X=$83 Y=$27 P=$F0 M=$83
		!byte	$4A,$85,$48,$24,$30 ;Input: A=$DF Y=$48 P=$B2 M=$24  Output: A=$DF X=$24 Y=$48 P=$30 M=$24
		!byte	$4A,$35,$98,$5B,$34 ;Input: A=$A2 Y=$98 P=$B6 M=$5B  Output: A=$A2 X=$5B Y=$98 P=$34 M=$5B
		!byte	$4A,$85,$48,$AA,$F5 ;Input: A=$61 Y=$48 P=$75 M=$AA  Output: A=$61 X=$AA Y=$48 P=$F5 M=$AA

		; $C0  CPY #imm
		!byte	$C2,$C0,$6E,$BD,$35 ;Input: A=$4A Y=$BD P=$36 M=$1F  Output: A=$4A X=$42 Y=$BD P=$35 M=$1F
		!byte	$42,$F1,$DE,$F4 ;Input: A=$BF Y=$DE P=$75 M=$B9  Output: A=$BF X=$21 Y=$DE P=$F4 M=$B9
		!byte	$42,$A1,$C2,$35 ;Input: A=$5A Y=$C2 P=$B5 M=$A1  Output: A=$5A X=$3D Y=$C2 P=$35 M=$A1
		!byte	$42,$0E,$C3,$B1 ;Input: A=$C8 Y=$C3 P=$B3 M=$17  Output: A=$C8 X=$3C Y=$C3 P=$B1 M=$17
		!byte	$42,$B5,$49,$B0 ;Input: A=$3A Y=$49 P=$32 M=$12  Output: A=$3A X=$B6 Y=$49 P=$B0 M=$12
		!byte	$42,$80,$C7,$31 ;Input: A=$0E Y=$C7 P=$30 M=$D2  Output: A=$0E X=$38 Y=$C7 P=$31 M=$D2
		!byte	$40,$3E,$BC ;Input: A=$B2 Y=$BC P=$75 M=$72  Output: A=$B2 X=$43 Y=$BC P=$75 M=$72
		!byte	$42,$0B,$C2,$B1 ;Input: A=$C8 Y=$C2 P=$33 M=$E7  Output: A=$C8 X=$3D Y=$C2 P=$B1 M=$E7

		; $C1  CMP (zp,x)
		!byte	$C2,$C1,$A5,$E2,$71 ;Input: A=$DF Y=$E2 P=$72 M=$85  Output: A=$DF X=$1D Y=$E2 P=$71 M=$85
		!byte	$42,$91,$CE,$B5 ;Input: A=$DE Y=$CE P=$B6 M=$19  Output: A=$DE X=$31 Y=$CE P=$B5 M=$19
		!byte	$42,$74,$B1,$75 ;Input: A=$E3 Y=$B1 P=$77 M=$B2  Output: A=$E3 X=$4E Y=$B1 P=$75 M=$B2
		!byte	$42,$76,$B3,$31 ;Input: A=$95 Y=$B3 P=$33 M=$4D  Output: A=$95 X=$4C Y=$B3 P=$31 M=$4D
		!byte	$42,$27,$64,$B0 ;Input: A=$A3 Y=$64 P=$31 M=$F3  Output: A=$A3 X=$9B Y=$64 P=$B0 M=$F3
		!byte	$42,$66,$A3,$B4 ;Input: A=$55 Y=$A3 P=$36 M=$C9  Output: A=$55 X=$5C Y=$A3 P=$B4 M=$C9
		!byte	$42,$90,$CD,$F4 ;Input: A=$73 Y=$CD P=$75 M=$C7  Output: A=$73 X=$32 Y=$CD P=$F4 M=$C7
		!byte	$42,$EB,$28,$31 ;Input: A=$13 Y=$28 P=$B0 M=$07  Output: A=$13 X=$D7 Y=$28 P=$31 M=$07

		; $C4  CPY zp
		!byte	$C2,$C4,$CD,$EB,$B5 ;Input: A=$5E Y=$EB P=$B7 M=$4D  Output: A=$5E X=$14 Y=$EB P=$B5 M=$4D
		!byte	$02,$5A,$B0 ;Input: A=$42 Y=$5A P=$32 M=$77  Output: A=$42 X=$A5 Y=$5A P=$B0 M=$77
		!byte	$02,$BF,$F4 ;Input: A=$53 Y=$BF P=$77 M=$C6  Output: A=$53 X=$40 Y=$BF P=$F4 M=$C6
		!byte	$02,$55,$35 ;Input: A=$41 Y=$55 P=$34 M=$49  Output: A=$41 X=$AA Y=$55 P=$35 M=$49
		!byte	$00,$5C ;Input: A=$33 Y=$5C P=$B0 M=$DB  Output: A=$33 X=$A3 Y=$5C P=$B0 M=$DB
		!byte	$02,$EA,$35 ;Input: A=$30 Y=$EA P=$37 M=$B3  Output: A=$30 X=$15 Y=$EA P=$35 M=$B3
		!byte	$02,$3E,$F4 ;Input: A=$4F Y=$3E P=$F6 M=$66  Output: A=$4F X=$C1 Y=$3E P=$F4 M=$66
		!byte	$00,$95 ;Input: A=$FE Y=$95 P=$31 M=$3F  Output: A=$FE X=$6A Y=$95 P=$31 M=$3F

		; $C5  CMP zp
		!byte	$C0,$C5,$CD,$DC ;Input: A=$FA Y=$DC P=$F5 M=$5D  Output: A=$FA X=$23 Y=$DC P=$F5 M=$5D
		!byte	$02,$AF,$71 ;Input: A=$AC Y=$AF P=$70 M=$6C  Output: A=$AC X=$50 Y=$AF P=$71 M=$6C
		!byte	$02,$80,$B5 ;Input: A=$A3 Y=$80 P=$B4 M=$0F  Output: A=$A3 X=$7F Y=$80 P=$B5 M=$0F
		!byte	$02,$E5,$71 ;Input: A=$7C Y=$E5 P=$F0 M=$50  Output: A=$7C X=$1A Y=$E5 P=$71 M=$50
		!byte	$02,$57,$F0 ;Input: A=$04 Y=$57 P=$72 M=$2E  Output: A=$04 X=$A8 Y=$57 P=$F0 M=$2E
		!byte	$00,$E9 ;Input: A=$5D Y=$E9 P=$31 M=$37  Output: A=$5D X=$16 Y=$E9 P=$31 M=$37
		!byte	$00,$C4 ;Input: A=$AB Y=$C4 P=$35 M=$79  Output: A=$AB X=$3B Y=$C4 P=$35 M=$79
		!byte	$02,$67,$F4 ;Input: A=$00 Y=$67 P=$74 M=$64  Output: A=$00 X=$98 Y=$67 P=$F4 M=$64

		; $C6  DEC zp
		!byte	$C3,$C6,$CD,$FA,$30,$05 ;Input: A=$66 Y=$FA P=$B0 M=$06  Output: A=$66 X=$05 Y=$FA P=$30 M=$05
		!byte	$03,$3B,$34,$14 ;Input: A=$A8 Y=$3B P=$B6 M=$15  Output: A=$A8 X=$C4 Y=$3B P=$34 M=$14
		!byte	$03,$CA,$B5,$8F ;Input: A=$F7 Y=$CA P=$35 M=$90  Output: A=$F7 X=$35 Y=$CA P=$B5 M=$8F
		!byte	$03,$E1,$74,$4B ;Input: A=$E2 Y=$E1 P=$76 M=$4C  Output: A=$E2 X=$1E Y=$E1 P=$74 M=$4B
		!byte	$01,$B9,$E9 ;Input: A=$7B Y=$B9 P=$B4 M=$EA  Output: A=$7B X=$46 Y=$B9 P=$B4 M=$E9
		!byte	$03,$2D,$34,$77 ;Input: A=$84 Y=$2D P=$B4 M=$78  Output: A=$84 X=$D2 Y=$2D P=$34 M=$77
		!byte	$03,$56,$B4,$BB ;Input: A=$CA Y=$56 P=$B6 M=$BC  Output: A=$CA X=$A9 Y=$56 P=$B4 M=$BB
		!byte	$03,$3C,$74,$25 ;Input: A=$5B Y=$3C P=$76 M=$26  Output: A=$5B X=$C3 Y=$3C P=$74 M=$25

		; $C8  INY
		!byte	$86,$C8,$CB,$CC,$B0 ;Input: A=$8A Y=$CB P=$30 M=$1C  Output: A=$8A X=$34 Y=$CC P=$B0 M=$1C
		!byte	$06,$AA,$AB,$B1 ;Input: A=$93 Y=$AA P=$B3 M=$42  Output: A=$93 X=$55 Y=$AB P=$B1 M=$42
		!byte	$06,$B1,$B2,$B4 ;Input: A=$B1 Y=$B1 P=$36 M=$B2  Output: A=$B1 X=$4E Y=$B2 P=$B4 M=$B2
		!byte	$06,$05,$06,$70 ;Input: A=$B5 Y=$05 P=$F2 M=$09  Output: A=$B5 X=$FA Y=$06 P=$70 M=$09
		!byte	$06,$29,$2A,$35 ;Input: A=$63 Y=$29 P=$37 M=$B0  Output: A=$63 X=$D6 Y=$2A P=$35 M=$B0
		!byte	$04,$82,$83 ;Input: A=$81 Y=$82 P=$B0 M=$A6  Output: A=$81 X=$7D Y=$83 P=$B0 M=$A6
		!byte	$06,$28,$29,$70 ;Input: A=$D9 Y=$28 P=$72 M=$D5  Output: A=$D9 X=$D7 Y=$29 P=$70 M=$D5
		!byte	$04,$EF,$F0 ;Input: A=$1A Y=$EF P=$B1 M=$F9  Output: A=$1A X=$10 Y=$F0 P=$B1 M=$F9

		; $C9  CMP #imm
		!byte	$C2,$C9,$42,$12,$F0 ;Input: A=$3E Y=$12 P=$72 M=$F4  Output: A=$3E X=$ED Y=$12 P=$F0 M=$F4
		!byte	$42,$3F,$E9,$F5 ;Input: A=$C1 Y=$E9 P=$75 M=$75  Output: A=$C1 X=$16 Y=$E9 P=$F5 M=$75
		!byte	$42,$31,$8B,$F4 ;Input: A=$0D Y=$8B P=$F5 M=$09  Output: A=$0D X=$74 Y=$8B P=$F4 M=$09
		!byte	$42,$B1,$4F,$30 ;Input: A=$07 Y=$4F P=$B0 M=$9E  Output: A=$07 X=$B0 Y=$4F P=$30 M=$9E
		!byte	$42,$77,$8A,$35 ;Input: A=$D9 Y=$8A P=$36 M=$67  Output: A=$D9 X=$75 Y=$8A P=$35 M=$67
		!byte	$42,$90,$F7,$F0 ;Input: A=$2C Y=$F7 P=$71 M=$95  Output: A=$2C X=$08 Y=$F7 P=$F0 M=$95
		!byte	$42,$04,$AE,$F1 ;Input: A=$97 Y=$AE P=$71 M=$58  Output: A=$97 X=$51 Y=$AE P=$F1 M=$58
		!byte	$42,$ED,$9F,$74 ;Input: A=$01 Y=$9F P=$F7 M=$BC  Output: A=$01 X=$60 Y=$9F P=$74 M=$BC

		; $CA  DEX
		!byte	$8A,$CA,$D6,$28,$31 ;Input: A=$09 Y=$D6 P=$33 M=$68  Output: A=$09 X=$28 Y=$D6 P=$31 M=$68
		!byte	$0A,$26,$D8,$F1 ;Input: A=$E6 Y=$26 P=$71 M=$4F  Output: A=$E6 X=$D8 Y=$26 P=$F1 M=$4F
		!byte	$0A,$EC,$12,$71 ;Input: A=$A7 Y=$EC P=$F3 M=$1A  Output: A=$A7 X=$12 Y=$EC P=$71 M=$1A
		!byte	$0A,$FC,$02,$35 ;Input: A=$4F Y=$FC P=$B7 M=$33  Output: A=$4F X=$02 Y=$FC P=$35 M=$33
		!byte	$0A,$01,$FD,$F5 ;Input: A=$7E Y=$01 P=$F7 M=$DB  Output: A=$7E X=$FD Y=$01 P=$F5 M=$DB
		!byte	$0A,$D7,$27,$70 ;Input: A=$BB Y=$D7 P=$F2 M=$CC  Output: A=$BB X=$27 Y=$D7 P=$70 M=$CC
		!byte	$0A,$9F,$5F,$30 ;Input: A=$B5 Y=$9F P=$B2 M=$58  Output: A=$B5 X=$5F Y=$9F P=$30 M=$58
		!byte	$0A,$A7,$57,$75 ;Input: A=$03 Y=$A7 P=$77 M=$6F  Output: A=$03 X=$57 Y=$A7 P=$75 M=$6F

		; $CC  CPY abs
		!byte	$E2,$CC,$CD,$00,$5A,$70 ;Input: A=$85 Y=$5A P=$71 M=$F5  Output: A=$85 X=$A5 Y=$5A P=$70 M=$F5
		!byte	$02,$CD,$B1 ;Input: A=$2B Y=$CD P=$31 M=$3A  Output: A=$2B X=$32 Y=$CD P=$B1 M=$3A
		!byte	$02,$A9,$F5 ;Input: A=$F0 Y=$A9 P=$77 M=$1D  Output: A=$F0 X=$56 Y=$A9 P=$F5 M=$1D
		!byte	$02,$87,$75 ;Input: A=$14 Y=$87 P=$74 M=$71  Output: A=$14 X=$78 Y=$87 P=$75 M=$71
		!byte	$00,$F4 ;Input: A=$62 Y=$F4 P=$F1 M=$2C  Output: A=$62 X=$0B Y=$F4 P=$F1 M=$2C
		!byte	$00,$AF ;Input: A=$A0 Y=$AF P=$F5 M=$05  Output: A=$A0 X=$50 Y=$AF P=$F5 M=$05
		!byte	$02,$EE,$35 ;Input: A=$E7 Y=$EE P=$B5 M=$8A  Output: A=$E7 X=$11 Y=$EE P=$35 M=$8A
		!byte	$02,$A6,$71 ;Input: A=$96 Y=$A6 P=$72 M=$33  Output: A=$96 X=$59 Y=$A6 P=$71 M=$33

		; $CD  CMP abs
		!byte	$E2,$CD,$CD,$00,$25,$35 ;Input: A=$5F Y=$25 P=$B4 M=$39  Output: A=$5F X=$DA Y=$25 P=$35 M=$39
		!byte	$02,$8B,$F0 ;Input: A=$3D Y=$8B P=$F1 M=$46  Output: A=$3D X=$74 Y=$8B P=$F0 M=$46
		!byte	$02,$D8,$75 ;Input: A=$F9 Y=$D8 P=$F4 M=$E7  Output: A=$F9 X=$27 Y=$D8 P=$75 M=$E7
		!byte	$02,$B0,$F4 ;Input: A=$00 Y=$B0 P=$F5 M=$59  Output: A=$00 X=$4F Y=$B0 P=$F4 M=$59
		!byte	$02,$08,$F5 ;Input: A=$DC Y=$08 P=$76 M=$56  Output: A=$DC X=$F7 Y=$08 P=$F5 M=$56
		!byte	$02,$4E,$75 ;Input: A=$86 Y=$4E P=$F5 M=$28  Output: A=$86 X=$B1 Y=$4E P=$75 M=$28
		!byte	$02,$D1,$F1 ;Input: A=$D6 Y=$D1 P=$72 M=$0C  Output: A=$D6 X=$2E Y=$D1 P=$F1 M=$0C
		!byte	$02,$77,$F4 ;Input: A=$56 Y=$77 P=$F5 M=$71  Output: A=$56 X=$88 Y=$77 P=$F4 M=$71

		; $CE  DEC abs
		!byte	$E1,$CE,$CD,$00,$FB,$8D ;Input: A=$7B Y=$FB P=$F4 M=$8E  Output: A=$7B X=$04 Y=$FB P=$F4 M=$8D
		!byte	$03,$4D,$74,$58 ;Input: A=$B6 Y=$4D P=$F4 M=$59  Output: A=$B6 X=$B2 Y=$4D P=$74 M=$58
		!byte	$01,$CD,$6A ;Input: A=$9A Y=$CD P=$30 M=$6B  Output: A=$9A X=$32 Y=$CD P=$30 M=$6A
		!byte	$03,$72,$B5,$85 ;Input: A=$92 Y=$72 P=$37 M=$86  Output: A=$92 X=$8D Y=$72 P=$B5 M=$85
		!byte	$03,$21,$B5,$8B ;Input: A=$9C Y=$21 P=$37 M=$8C  Output: A=$9C X=$DE Y=$21 P=$B5 M=$8B
		!byte	$03,$BF,$70,$11 ;Input: A=$EC Y=$BF P=$F2 M=$12  Output: A=$EC X=$40 Y=$BF P=$70 M=$11
		!byte	$03,$BE,$71,$15 ;Input: A=$C4 Y=$BE P=$F1 M=$16  Output: A=$C4 X=$41 Y=$BE P=$71 M=$15
		!byte	$03,$39,$71,$24 ;Input: A=$8E Y=$39 P=$F1 M=$25  Output: A=$8E X=$C6 Y=$39 P=$71 M=$24

		; $D1  CMP (zp),y
		!byte	$C2,$D1,$C4,$01,$71 ;Input: A=$B7 Y=$01 P=$70 M=$6D  Output: A=$B7 X=$FE Y=$01 P=$71 M=$6D
		!byte	$42,$C2,$00,$71 ;Input: A=$5E Y=$00 P=$F1 M=$5D  Output: A=$5E X=$FF Y=$00 P=$71 M=$5D
		!byte	$42,$C4,$01,$F4 ;Input: A=$33 Y=$01 P=$76 M=$5F  Output: A=$33 X=$FE Y=$01 P=$F4 M=$5F
		!byte	$42,$C2,$00,$B0 ;Input: A=$21 Y=$00 P=$B3 M=$62  Output: A=$21 X=$FF Y=$00 P=$B0 M=$62
		!byte	$42,$C4,$01,$71 ;Input: A=$CF Y=$01 P=$73 M=$5A  Output: A=$CF X=$FE Y=$01 P=$71 M=$5A
		!byte	$42,$C2,$00,$B4 ;Input: A=$19 Y=$00 P=$34 M=$32  Output: A=$19 X=$FF Y=$00 P=$B4 M=$32
		!byte	$02,$00,$75 ;Input: A=$5B Y=$00 P=$77 M=$3D  Output: A=$5B X=$FF Y=$00 P=$75 M=$3D
		!byte	$42,$C4,$01,$74 ;Input: A=$24 Y=$01 P=$F6 M=$B1  Output: A=$24 X=$FE Y=$01 P=$74 M=$B1

		; $D5  CMP zp,x
		!byte	$C2,$D5,$EB,$1D,$F1 ;Input: A=$EA Y=$1D P=$71 M=$0D  Output: A=$EA X=$E2 Y=$1D P=$F1 M=$0D
		!byte	$42,$A0,$D2,$75 ;Input: A=$62 Y=$D2 P=$F4 M=$1C  Output: A=$62 X=$2D Y=$D2 P=$75 M=$1C
		!byte	$42,$6C,$9E,$31 ;Input: A=$C2 Y=$9E P=$33 M=$97  Output: A=$C2 X=$61 Y=$9E P=$31 M=$97
		!byte	$42,$C4,$F6,$70 ;Input: A=$17 Y=$F6 P=$F2 M=$99  Output: A=$17 X=$09 Y=$F6 P=$70 M=$99
		!byte	$42,$07,$39,$34 ;Input: A=$03 Y=$39 P=$B6 M=$D0  Output: A=$03 X=$C6 Y=$39 P=$34 M=$D0
		!byte	$42,$5C,$8E,$35 ;Input: A=$76 Y=$8E P=$B7 M=$4E  Output: A=$76 X=$71 Y=$8E P=$35 M=$4E
		!byte	$42,$92,$C4,$F0 ;Input: A=$52 Y=$C4 P=$70 M=$A9  Output: A=$52 X=$3B Y=$C4 P=$F0 M=$A9
		!byte	$42,$0E,$40,$74 ;Input: A=$30 Y=$40 P=$77 M=$D1  Output: A=$30 X=$BF Y=$40 P=$74 M=$D1

		; $D6  DEC zp,x
		!byte	$C3,$D6,$00,$32,$B4,$E4 ;Input: A=$20 Y=$32 P=$36 M=$E5  Output: A=$20 X=$CD Y=$32 P=$B4 M=$E4
		!byte	$43,$7D,$AF,$34,$12 ;Input: A=$27 Y=$AF P=$B4 M=$13  Output: A=$27 X=$50 Y=$AF P=$34 M=$12
		!byte	$43,$32,$64,$34,$05 ;Input: A=$CC Y=$64 P=$36 M=$06  Output: A=$CC X=$9B Y=$64 P=$34 M=$05
		!byte	$43,$C0,$F2,$F1,$92 ;Input: A=$BF Y=$F2 P=$71 M=$93  Output: A=$BF X=$0D Y=$F2 P=$F1 M=$92
		!byte	$41,$0F,$41,$D7 ;Input: A=$45 Y=$41 P=$B5 M=$D8  Output: A=$45 X=$BE Y=$41 P=$B5 M=$D7
		!byte	$43,$14,$46,$71,$5E ;Input: A=$6A Y=$46 P=$F3 M=$5F  Output: A=$6A X=$B9 Y=$46 P=$71 M=$5E
		!byte	$43,$D7,$09,$B1,$F8 ;Input: A=$10 Y=$09 P=$33 M=$F9  Output: A=$10 X=$F6 Y=$09 P=$B1 M=$F8
		!byte	$43,$CD,$FF,$74,$5E ;Input: A=$29 Y=$FF P=$F6 M=$5F  Output: A=$29 X=$00 Y=$FF P=$74 M=$5E

		; $D8  CLD
		!byte	$80,$D8,$E6 ;Input: A=$73 Y=$E6 P=$F7 M=$D5  Output: A=$73 X=$19 Y=$E6 P=$F7 M=$D5
		!byte	$00,$C5 ;Input: A=$A3 Y=$C5 P=$34 M=$4A  Output: A=$A3 X=$3A Y=$C5 P=$34 M=$4A
		!byte	$00,$C5 ;Input: A=$E8 Y=$C5 P=$30 M=$0F  Output: A=$E8 X=$3A Y=$C5 P=$30 M=$0F
		!byte	$00,$FC ;Input: A=$29 Y=$FC P=$F0 M=$E2  Output: A=$29 X=$03 Y=$FC P=$F0 M=$E2
		!byte	$00,$63 ;Input: A=$BA Y=$63 P=$F3 M=$2A  Output: A=$BA X=$9C Y=$63 P=$F3 M=$2A
		!byte	$00,$84 ;Input: A=$C4 Y=$84 P=$F5 M=$8D  Output: A=$C4 X=$7B Y=$84 P=$F5 M=$8D
		!byte	$00,$AB ;Input: A=$B8 Y=$AB P=$71 M=$86  Output: A=$B8 X=$54 Y=$AB P=$71 M=$86
		!byte	$00,$A3 ;Input: A=$3D Y=$A3 P=$F3 M=$F2  Output: A=$3D X=$5C Y=$A3 P=$F3 M=$F2

		; $D9  CMP abs,y
		!byte	$E2,$D9,$8E,$00,$3F,$71 ;Input: A=$6A Y=$3F P=$70 M=$5D  Output: A=$6A X=$C0 Y=$3F P=$71 M=$5D
		!byte	$02,$3F,$70 ;Input: A=$30 Y=$3F P=$71 M=$E9  Output: A=$30 X=$C0 Y=$3F P=$70 M=$E9
		!byte	$42,$78,$55,$B1 ;Input: A=$EB Y=$55 P=$32 M=$4E  Output: A=$EB X=$AA Y=$55 P=$B1 M=$4E
		!byte	$42,$7C,$51,$75 ;Input: A=$6B Y=$51 P=$F7 M=$10  Output: A=$6B X=$AE Y=$51 P=$75 M=$10
		!byte	$42,$09,$C4,$31 ;Input: A=$E6 Y=$C4 P=$32 M=$9E  Output: A=$E6 X=$3B Y=$C4 P=$31 M=$9E
		!byte	$42,$37,$96,$B0 ;Input: A=$C0 Y=$96 P=$31 M=$D2  Output: A=$C0 X=$69 Y=$96 P=$B0 M=$D2
		!byte	$42,$6F,$5E,$B4 ;Input: A=$3A Y=$5E P=$34 M=$51  Output: A=$3A X=$A1 Y=$5E P=$B4 M=$51
		!byte	$42,$45,$88,$75 ;Input: A=$F8 Y=$88 P=$74 M=$B7  Output: A=$F8 X=$77 Y=$88 P=$75 M=$B7

		; $DD  CMP abs,x
		!byte	$E2,$DD,$4F,$00,$81,$75 ;Input: A=$54 Y=$81 P=$74 M=$2C  Output: A=$54 X=$7E Y=$81 P=$75 M=$2C
		!byte	$42,$5A,$8C,$B0 ;Input: A=$61 Y=$8C P=$30 M=$9E  Output: A=$61 X=$73 Y=$8C P=$B0 M=$9E
		!byte	$42,$B7,$E9,$30 ;Input: A=$21 Y=$E9 P=$B0 M=$E6  Output: A=$21 X=$16 Y=$E9 P=$30 M=$E6
		!byte	$42,$3B,$6D,$B0 ;Input: A=$3C Y=$6D P=$31 M=$7D  Output: A=$3C X=$92 Y=$6D P=$B0 M=$7D
		!byte	$40,$96,$C8 ;Input: A=$55 Y=$C8 P=$35 M=$51  Output: A=$55 X=$37 Y=$C8 P=$35 M=$51
		!byte	$62,$D7,$FF,$09,$75 ;Input: A=$DD Y=$09 P=$F7 M=$8A  Output: A=$DD X=$F6 Y=$09 P=$75 M=$8A
		!byte	$42,$E7,$19,$71 ;Input: A=$CA Y=$19 P=$F2 M=$56  Output: A=$CA X=$E6 Y=$19 P=$71 M=$56
		!byte	$62,$18,$00,$4A,$B4 ;Input: A=$97 Y=$4A P=$36 M=$A6  Output: A=$97 X=$B5 Y=$4A P=$B4 M=$A6

		; $DE  DEC abs,x
		!byte	$E1,$DE,$FF,$FF,$31,$52 ;Input: A=$F0 Y=$31 P=$35 M=$53  Output: A=$F0 X=$CE Y=$31 P=$35 M=$52
		!byte	$61,$26,$00,$58,$05 ;Input: A=$AC Y=$58 P=$70 M=$06  Output: A=$AC X=$A7 Y=$58 P=$70 M=$05
		!byte	$43,$65,$97,$F4,$D0 ;Input: A=$AE Y=$97 P=$74 M=$D1  Output: A=$AE X=$68 Y=$97 P=$F4 M=$D0
		!byte	$63,$DB,$FF,$0D,$F4,$D9 ;Input: A=$63 Y=$0D P=$F6 M=$DA  Output: A=$63 X=$F2 Y=$0D P=$F4 M=$D9
		!byte	$43,$E3,$15,$B4,$B6 ;Input: A=$6D Y=$15 P=$36 M=$B7  Output: A=$6D X=$EA Y=$15 P=$B4 M=$B6
		!byte	$63,$78,$00,$AA,$71,$42 ;Input: A=$C0 Y=$AA P=$73 M=$43  Output: A=$C0 X=$55 Y=$AA P=$71 M=$42
		!byte	$61,$DD,$FF,$0F,$23 ;Input: A=$19 Y=$0F P=$30 M=$24  Output: A=$19 X=$F0 Y=$0F P=$30 M=$23
		!byte	$63,$96,$00,$C8,$F5,$C5 ;Input: A=$44 Y=$C8 P=$77 M=$C6  Output: A=$44 X=$37 Y=$C8 P=$F5 M=$C5

		; $E0  CPX #imm
		!byte	$C2,$E0,$A3,$14,$35 ;Input: A=$B5 Y=$14 P=$36 M=$49  Output: A=$B5 X=$EB Y=$14 P=$35 M=$49
		!byte	$42,$E8,$91,$B4 ;Input: A=$0B Y=$91 P=$B7 M=$CD  Output: A=$0B X=$6E Y=$91 P=$B4 M=$CD
		!byte	$42,$50,$B3,$B0 ;Input: A=$A0 Y=$B3 P=$33 M=$69  Output: A=$A0 X=$4C Y=$B3 P=$B0 M=$69
		!byte	$42,$60,$2F,$31 ;Input: A=$3C Y=$2F P=$B1 M=$17  Output: A=$3C X=$D0 Y=$2F P=$31 M=$17
		!byte	$42,$2B,$90,$75 ;Input: A=$FA Y=$90 P=$F5 M=$2C  Output: A=$FA X=$6F Y=$90 P=$75 M=$2C
		!byte	$42,$E3,$85,$F0 ;Input: A=$8E Y=$85 P=$70 M=$0A  Output: A=$8E X=$7A Y=$85 P=$F0 M=$0A
		!byte	$02,$B7,$34 ;Input: A=$83 Y=$B7 P=$B6 M=$DB  Output: A=$83 X=$48 Y=$B7 P=$34 M=$DB
		!byte	$42,$20,$1D,$B5 ;Input: A=$C2 Y=$1D P=$B4 M=$88  Output: A=$C2 X=$E2 Y=$1D P=$B5 M=$88

		; $E1  SBC (zp,x)
		!byte	$D2,$E1,$F2,$2F,$24,$31 ;Input: A=$E3 Y=$2F P=$30 M=$BE  Output: A=$24 X=$D0 Y=$2F P=$31 M=$BE
		!byte	$52,$25,$62,$FD,$B0 ;Input: A=$23 Y=$62 P=$31 M=$26  Output: A=$FD X=$9D Y=$62 P=$B0 M=$26
		!byte	$52,$96,$D3,$21,$75 ;Input: A=$98 Y=$D3 P=$B5 M=$77  Output: A=$21 X=$2C Y=$D3 P=$75 M=$77
		!byte	$52,$D3,$10,$34,$31 ;Input: A=$7F Y=$10 P=$B3 M=$4B  Output: A=$34 X=$EF Y=$10 P=$31 M=$4B
		!byte	$52,$20,$5D,$C3,$B0 ;Input: A=$A9 Y=$5D P=$33 M=$E6  Output: A=$C3 X=$A2 Y=$5D P=$B0 M=$E6
		!byte	$52,$AB,$E8,$10,$31 ;Input: A=$EE Y=$E8 P=$72 M=$DD  Output: A=$10 X=$17 Y=$E8 P=$31 M=$DD
		!byte	$52,$87,$C4,$4E,$30 ;Input: A=$39 Y=$C4 P=$73 M=$EB  Output: A=$4E X=$3B Y=$C4 P=$30 M=$EB
		!byte	$52,$6B,$A8,$5C,$34 ;Input: A=$3E Y=$A8 P=$75 M=$E2  Output: A=$5C X=$57 Y=$A8 P=$34 M=$E2

		; $E4  CPX zp
		!byte	$C2,$E4,$CD,$C8,$F4 ;Input: A=$D7 Y=$C8 P=$F7 M=$93  Output: A=$D7 X=$37 Y=$C8 P=$F4 M=$93
		!byte	$02,$3A,$F1 ;Input: A=$EA Y=$3A P=$F2 M=$40  Output: A=$EA X=$C5 Y=$3A P=$F1 M=$40
		!byte	$02,$CA,$B4 ;Input: A=$48 Y=$CA P=$36 M=$58  Output: A=$48 X=$35 Y=$CA P=$B4 M=$58
		!byte	$02,$2C,$35 ;Input: A=$E1 Y=$2C P=$B7 M=$C9  Output: A=$E1 X=$D3 Y=$2C P=$35 M=$C9
		!byte	$02,$E2,$F0 ;Input: A=$49 Y=$E2 P=$71 M=$4D  Output: A=$49 X=$1D Y=$E2 P=$F0 M=$4D
		!byte	$00,$60 ;Input: A=$0F Y=$60 P=$F4 M=$A0  Output: A=$0F X=$9F Y=$60 P=$F4 M=$A0
		!byte	$02,$2C,$B5 ;Input: A=$BD Y=$2C P=$B4 M=$3D  Output: A=$BD X=$D3 Y=$2C P=$B5 M=$3D
		!byte	$02,$05,$F1 ;Input: A=$45 Y=$05 P=$71 M=$40  Output: A=$45 X=$FA Y=$05 P=$F1 M=$40

		; $E5  SBC zp
		!byte	$D2,$E5,$CD,$62,$12,$31 ;Input: A=$EC Y=$62 P=$B3 M=$DA  Output: A=$12 X=$9D Y=$62 P=$31 M=$DA
		!byte	$12,$C5,$65,$34 ;Input: A=$39 Y=$C5 P=$F7 M=$D4  Output: A=$65 X=$3A Y=$C5 P=$34 M=$D4
		!byte	$10,$B0,$26 ;Input: A=$38 Y=$B0 P=$35 M=$12  Output: A=$26 X=$4F Y=$B0 P=$35 M=$12
		!byte	$10,$40,$9E ;Input: A=$73 Y=$40 P=$F0 M=$D4  Output: A=$9E X=$BF Y=$40 P=$F0 M=$D4
		!byte	$12,$C3,$FB,$B5 ;Input: A=$FC Y=$C3 P=$F6 M=$00  Output: A=$FB X=$3C Y=$C3 P=$B5 M=$00
		!byte	$12,$BA,$00,$37 ;Input: A=$EB Y=$BA P=$B5 M=$EB  Output: A=$00 X=$45 Y=$BA P=$37 M=$EB
		!byte	$10,$24,$9D ;Input: A=$BA Y=$24 P=$B5 M=$1D  Output: A=$9D X=$DB Y=$24 P=$B5 M=$1D
		!byte	$12,$10,$2C,$31 ;Input: A=$CF Y=$10 P=$F1 M=$A3  Output: A=$2C X=$EF Y=$10 P=$31 M=$A3

		; $E6  INC zp
		!byte	$C1,$E6,$CD,$C4,$D0 ;Input: A=$78 Y=$C4 P=$B0 M=$CF  Output: A=$78 X=$3B Y=$C4 P=$B0 M=$D0
		!byte	$03,$F1,$B5,$E4 ;Input: A=$98 Y=$F1 P=$B7 M=$E3  Output: A=$98 X=$0E Y=$F1 P=$B5 M=$E4
		!byte	$03,$D8,$35,$3C ;Input: A=$AE Y=$D8 P=$37 M=$3B  Output: A=$AE X=$27 Y=$D8 P=$35 M=$3C
		!byte	$01,$4A,$57 ;Input: A=$BD Y=$4A P=$74 M=$56  Output: A=$BD X=$B5 Y=$4A P=$74 M=$57
		!byte	$03,$E6,$71,$09 ;Input: A=$FB Y=$E6 P=$F1 M=$08  Output: A=$FB X=$19 Y=$E6 P=$71 M=$09
		!byte	$03,$02,$B1,$CA ;Input: A=$98 Y=$02 P=$31 M=$C9  Output: A=$98 X=$FD Y=$02 P=$B1 M=$CA
		!byte	$03,$20,$F1,$CC ;Input: A=$5A Y=$20 P=$73 M=$CB  Output: A=$5A X=$DF Y=$20 P=$F1 M=$CC
		!byte	$03,$E8,$75,$37 ;Input: A=$A5 Y=$E8 P=$77 M=$36  Output: A=$A5 X=$17 Y=$E8 P=$75 M=$37

		; $E8  INX
		!byte	$8A,$E8,$F2,$0E,$35 ;Input: A=$65 Y=$F2 P=$B5 M=$77  Output: A=$65 X=$0E Y=$F2 P=$35 M=$77
		!byte	$0A,$EA,$16,$31 ;Input: A=$25 Y=$EA P=$B3 M=$63  Output: A=$25 X=$16 Y=$EA P=$31 M=$63
		!byte	$0A,$D7,$29,$74 ;Input: A=$F5 Y=$D7 P=$76 M=$16  Output: A=$F5 X=$29 Y=$D7 P=$74 M=$16
		!byte	$0A,$87,$79,$74 ;Input: A=$04 Y=$87 P=$F6 M=$A2  Output: A=$04 X=$79 Y=$87 P=$74 M=$A2
		!byte	$0A,$70,$90,$F1 ;Input: A=$0B Y=$70 P=$71 M=$A7  Output: A=$0B X=$90 Y=$70 P=$F1 M=$A7
		!byte	$0A,$87,$79,$75 ;Input: A=$73 Y=$87 P=$F7 M=$42  Output: A=$73 X=$79 Y=$87 P=$75 M=$42
		!byte	$08,$D7,$29 ;Input: A=$67 Y=$D7 P=$34 M=$F2  Output: A=$67 X=$29 Y=$D7 P=$34 M=$F2
		!byte	$0A,$E9,$17,$75 ;Input: A=$C1 Y=$E9 P=$77 M=$A3  Output: A=$C1 X=$17 Y=$E9 P=$75 M=$A3

		; $E9  SBC #imm
		!byte	$D2,$E9,$E1,$31,$29,$30 ;Input: A=$0A Y=$31 P=$F1 M=$49  Output: A=$29 X=$CE Y=$31 P=$30 M=$49
		!byte	$52,$73,$1D,$C0,$B4 ;Input: A=$34 Y=$1D P=$F6 M=$05  Output: A=$C0 X=$E2 Y=$1D P=$B4 M=$05
		!byte	$50,$8D,$82,$55 ;Input: A=$E2 Y=$82 P=$35 M=$A1  Output: A=$55 X=$7D Y=$82 P=$35 M=$A1
		!byte	$52,$9C,$73,$2B,$35 ;Input: A=$C8 Y=$73 P=$F6 M=$9C  Output: A=$2B X=$8C Y=$73 P=$35 M=$9C
		!byte	$52,$CE,$C6,$6B,$34 ;Input: A=$3A Y=$C6 P=$B4 M=$A9  Output: A=$6B X=$39 Y=$C6 P=$34 M=$A9
		!byte	$52,$57,$D4,$4C,$75 ;Input: A=$A4 Y=$D4 P=$34 M=$1A  Output: A=$4C X=$2B Y=$D4 P=$75 M=$1A
		!byte	$52,$AC,$3D,$AB,$F4 ;Input: A=$58 Y=$3D P=$F6 M=$C7  Output: A=$AB X=$C2 Y=$3D P=$F4 M=$C7
		!byte	$52,$A1,$B0,$2B,$31 ;Input: A=$CC Y=$B0 P=$73 M=$A7  Output: A=$2B X=$4F Y=$B0 P=$31 M=$A7

		; $EA  NOP
		!byte	$80,$EA,$06 ;Input: A=$5C Y=$06 P=$F7 M=$5E  Output: A=$5C X=$F9 Y=$06 P=$F7 M=$5E
		!byte	$00,$37 ;Input: A=$45 Y=$37 P=$F0 M=$98  Output: A=$45 X=$C8 Y=$37 P=$F0 M=$98
		!byte	$00,$EF ;Input: A=$96 Y=$EF P=$B5 M=$BC  Output: A=$96 X=$10 Y=$EF P=$B5 M=$BC
		!byte	$00,$CC ;Input: A=$49 Y=$CC P=$F6 M=$2F  Output: A=$49 X=$33 Y=$CC P=$F6 M=$2F
		!byte	$00,$8C ;Input: A=$66 Y=$8C P=$B6 M=$F5  Output: A=$66 X=$73 Y=$8C P=$B6 M=$F5
		!byte	$00,$B6 ;Input: A=$F0 Y=$B6 P=$72 M=$EC  Output: A=$F0 X=$49 Y=$B6 P=$72 M=$EC
		!byte	$00,$86 ;Input: A=$EA Y=$86 P=$72 M=$0D  Output: A=$EA X=$79 Y=$86 P=$72 M=$0D
		!byte	$00,$19 ;Input: A=$29 Y=$19 P=$B6 M=$29  Output: A=$29 X=$E6 Y=$19 P=$B6 M=$29

		; $EC  CPX abs
		!byte	$E2,$EC,$CD,$00,$36,$B5 ;Input: A=$B8 Y=$36 P=$37 M=$24  Output: A=$B8 X=$C9 Y=$36 P=$B5 M=$24
		!byte	$02,$25,$B4 ;Input: A=$B3 Y=$25 P=$B7 M=$F6  Output: A=$B3 X=$DA Y=$25 P=$B4 M=$F6
		!byte	$02,$72,$75 ;Input: A=$47 Y=$72 P=$76 M=$11  Output: A=$47 X=$8D Y=$72 P=$75 M=$11
		!byte	$02,$8E,$F0 ;Input: A=$9B Y=$8E P=$73 M=$7A  Output: A=$9B X=$71 Y=$8E P=$F0 M=$7A
		!byte	$00,$A7 ;Input: A=$8F Y=$A7 P=$74 M=$F2  Output: A=$8F X=$58 Y=$A7 P=$74 M=$F2
		!byte	$02,$3C,$F4 ;Input: A=$87 Y=$3C P=$77 M=$EC  Output: A=$87 X=$C3 Y=$3C P=$F4 M=$EC
		!byte	$02,$28,$B5 ;Input: A=$1E Y=$28 P=$B7 M=$2B  Output: A=$1E X=$D7 Y=$28 P=$B5 M=$2B
		!byte	$02,$43,$31 ;Input: A=$11 Y=$43 P=$B3 M=$8B  Output: A=$11 X=$BC Y=$43 P=$31 M=$8B

		; $ED  SBC abs
		!byte	$F2,$ED,$CD,$00,$C6,$8F,$B4 ;Input: A=$07 Y=$C6 P=$37 M=$78  Output: A=$8F X=$39 Y=$C6 P=$B4 M=$78
		!byte	$12,$15,$43,$35 ;Input: A=$78 Y=$15 P=$34 M=$34  Output: A=$43 X=$EA Y=$15 P=$35 M=$34
		!byte	$12,$A1,$E2,$F4 ;Input: A=$77 Y=$A1 P=$B7 M=$95  Output: A=$E2 X=$5E Y=$A1 P=$F4 M=$95
		!byte	$10,$F7,$1E ;Input: A=$CB Y=$F7 P=$31 M=$AD  Output: A=$1E X=$08 Y=$F7 P=$31 M=$AD
		!byte	$12,$3E,$3C,$35 ;Input: A=$C9 Y=$3E P=$75 M=$8D  Output: A=$3C X=$C1 Y=$3E P=$35 M=$8D
		!byte	$12,$B1,$42,$35 ;Input: A=$73 Y=$B1 P=$37 M=$31  Output: A=$42 X=$4E Y=$B1 P=$35 M=$31
		!byte	$12,$D9,$52,$34 ;Input: A=$47 Y=$D9 P=$F4 M=$F4  Output: A=$52 X=$26 Y=$D9 P=$34 M=$F4
		!byte	$12,$8A,$9E,$F4 ;Input: A=$6F Y=$8A P=$B7 M=$D1  Output: A=$9E X=$75 Y=$8A P=$F4 M=$D1

		; $EE  INC abs
		!byte	$E3,$EE,$CD,$00,$FC,$75,$1E ;Input: A=$69 Y=$FC P=$F7 M=$1D  Output: A=$69 X=$03 Y=$FC P=$75 M=$1E
		!byte	$01,$73,$C7 ;Input: A=$D1 Y=$73 P=$B0 M=$C6  Output: A=$D1 X=$8C Y=$73 P=$B0 M=$C7
		!byte	$03,$50,$F5,$B8 ;Input: A=$BC Y=$50 P=$F7 M=$B7  Output: A=$BC X=$AF Y=$50 P=$F5 M=$B8
		!byte	$03,$83,$B0,$AF ;Input: A=$7D Y=$83 P=$32 M=$AE  Output: A=$7D X=$7C Y=$83 P=$B0 M=$AF
		!byte	$03,$B4,$75,$67 ;Input: A=$D7 Y=$B4 P=$F7 M=$66  Output: A=$D7 X=$4B Y=$B4 P=$75 M=$67
		!byte	$03,$A3,$70,$03 ;Input: A=$B4 Y=$A3 P=$72 M=$02  Output: A=$B4 X=$5C Y=$A3 P=$70 M=$03
		!byte	$03,$8C,$B4,$E0 ;Input: A=$C6 Y=$8C P=$B6 M=$DF  Output: A=$C6 X=$73 Y=$8C P=$B4 M=$E0
		!byte	$03,$8B,$B1,$B3 ;Input: A=$16 Y=$8B P=$31 M=$B2  Output: A=$16 X=$74 Y=$8B P=$B1 M=$B3

		; $F1  SBC (zp),y
		!byte	$D2,$F1,$C4,$01,$F1,$B4 ;Input: A=$1E Y=$01 P=$75 M=$2D  Output: A=$F1 X=$FE Y=$01 P=$B4 M=$2D
		!byte	$12,$01,$6A,$34 ;Input: A=$59 Y=$01 P=$B7 M=$EF  Output: A=$6A X=$FE Y=$01 P=$34 M=$EF
		!byte	$12,$01,$80,$B5 ;Input: A=$B0 Y=$01 P=$74 M=$2F  Output: A=$80 X=$FE Y=$01 P=$B5 M=$2F
		!byte	$12,$01,$0E,$35 ;Input: A=$76 Y=$01 P=$77 M=$68  Output: A=$0E X=$FE Y=$01 P=$35 M=$68
		!byte	$52,$C2,$00,$5C,$71 ;Input: A=$90 Y=$00 P=$70 M=$33  Output: A=$5C X=$FF Y=$00 P=$71 M=$33
		!byte	$52,$C4,$01,$EE,$B4 ;Input: A=$13 Y=$01 P=$77 M=$25  Output: A=$EE X=$FE Y=$01 P=$B4 M=$25
		!byte	$12,$01,$B4,$B4 ;Input: A=$95 Y=$01 P=$B5 M=$E1  Output: A=$B4 X=$FE Y=$01 P=$B4 M=$E1
		!byte	$52,$C2,$00,$71,$34 ;Input: A=$12 Y=$00 P=$F5 M=$A1  Output: A=$71 X=$FF Y=$00 P=$34 M=$A1

		; $F5  SBC zp,x
		!byte	$D2,$F5,$78,$AA,$E4,$B0 ;Input: A=$37 Y=$AA P=$30 M=$52  Output: A=$E4 X=$55 Y=$AA P=$B0 M=$52
		!byte	$52,$3C,$6E,$9D,$B1 ;Input: A=$F8 Y=$6E P=$B2 M=$5A  Output: A=$9D X=$91 Y=$6E P=$B1 M=$5A
		!byte	$52,$91,$C3,$77,$30 ;Input: A=$57 Y=$C3 P=$B0 M=$DF  Output: A=$77 X=$3C Y=$C3 P=$30 M=$DF
		!byte	$52,$08,$3A,$E8,$F0 ;Input: A=$7F Y=$3A P=$F3 M=$97  Output: A=$E8 X=$C5 Y=$3A P=$F0 M=$97
		!byte	$52,$E0,$12,$D1,$B0 ;Input: A=$28 Y=$12 P=$73 M=$57  Output: A=$D1 X=$ED Y=$12 P=$B0 M=$57
		!byte	$52,$40,$72,$D6,$B0 ;Input: A=$41 Y=$72 P=$70 M=$6A  Output: A=$D6 X=$8D Y=$72 P=$B0 M=$6A
		!byte	$50,$DD,$0F,$D1 ;Input: A=$85 Y=$0F P=$B0 M=$B3  Output: A=$D1 X=$F0 Y=$0F P=$B0 M=$B3
		!byte	$52,$3C,$6E,$66,$34 ;Input: A=$2F Y=$6E P=$B7 M=$C9  Output: A=$66 X=$91 Y=$6E P=$34 M=$C9

		; $F6  INC zp,x
		!byte	$C3,$F6,$BB,$ED,$34,$7C ;Input: A=$BE Y=$ED P=$36 M=$7B  Output: A=$BE X=$12 Y=$ED P=$34 M=$7C
		!byte	$43,$F3,$25,$B4,$E2 ;Input: A=$FD Y=$25 P=$B6 M=$E1  Output: A=$FD X=$DA Y=$25 P=$B4 M=$E2
		!byte	$41,$40,$72,$8B ;Input: A=$2A Y=$72 P=$B4 M=$8A  Output: A=$2A X=$8D Y=$72 P=$B4 M=$8B
		!byte	$41,$6C,$9E,$0F ;Input: A=$A7 Y=$9E P=$70 M=$0E  Output: A=$A7 X=$61 Y=$9E P=$70 M=$0F
		!byte	$43,$D3,$05,$F5,$EC ;Input: A=$B9 Y=$05 P=$75 M=$EB  Output: A=$B9 X=$FA Y=$05 P=$F5 M=$EC
		!byte	$43,$76,$A8,$B0,$F2 ;Input: A=$DB Y=$A8 P=$32 M=$F1  Output: A=$DB X=$57 Y=$A8 P=$B0 M=$F2
		!byte	$43,$D7,$09,$31,$7B ;Input: A=$C9 Y=$09 P=$33 M=$7A  Output: A=$C9 X=$F6 Y=$09 P=$31 M=$7B
		!byte	$43,$99,$CB,$B1,$A4 ;Input: A=$39 Y=$CB P=$B3 M=$A3  Output: A=$39 X=$34 Y=$CB P=$B1 M=$A4

		; $F8  SED
		!byte	$82,$F8,$69,$BD ;Input: A=$B3 Y=$69 P=$B5 M=$BE  Output: A=$B3 X=$96 Y=$69 P=$BD M=$BE
		!byte	$02,$7E,$BA ;Input: A=$E2 Y=$7E P=$B2 M=$BD  Output: A=$E2 X=$81 Y=$7E P=$BA M=$BD
		!byte	$02,$74,$3C ;Input: A=$5E Y=$74 P=$34 M=$7B  Output: A=$5E X=$8B Y=$74 P=$3C M=$7B
		!byte	$02,$9A,$F8 ;Input: A=$D5 Y=$9A P=$F0 M=$E3  Output: A=$D5 X=$65 Y=$9A P=$F8 M=$E3
		!byte	$02,$EC,$FA ;Input: A=$DA Y=$EC P=$F2 M=$44  Output: A=$DA X=$13 Y=$EC P=$FA M=$44
		!byte	$02,$0F,$FA ;Input: A=$85 Y=$0F P=$F2 M=$68  Output: A=$85 X=$F0 Y=$0F P=$FA M=$68
		!byte	$02,$59,$7B ;Input: A=$0A Y=$59 P=$73 M=$A8  Output: A=$0A X=$A6 Y=$59 P=$7B M=$A8
		!byte	$02,$D6,$BA ;Input: A=$CF Y=$D6 P=$B2 M=$80  Output: A=$CF X=$29 Y=$D6 P=$BA M=$80

		; $F9  SBC abs,y
		!byte	$F2,$F9,$2A,$00,$A3,$45,$31 ;Input: A=$E3 Y=$A3 P=$F2 M=$9D  Output: A=$45 X=$5C Y=$A3 P=$31 M=$9D
		!byte	$52,$A6,$27,$3F,$34 ;Input: A=$29 Y=$27 P=$B5 M=$EA  Output: A=$3F X=$D8 Y=$27 P=$34 M=$EA
		!byte	$72,$ED,$FF,$E0,$A8,$B4 ;Input: A=$0F Y=$E0 P=$F5 M=$67  Output: A=$A8 X=$1F Y=$E0 P=$B4 M=$67
		!byte	$52,$E2,$EB,$B5,$B0 ;Input: A=$30 Y=$EB P=$B2 M=$7A  Output: A=$B5 X=$14 Y=$EB P=$B0 M=$7A
		!byte	$72,$62,$00,$6B,$A1,$B1 ;Input: A=$DF Y=$6B P=$B0 M=$3D  Output: A=$A1 X=$94 Y=$6B P=$B1 M=$3D
		!byte	$52,$59,$74,$15,$31 ;Input: A=$67 Y=$74 P=$72 M=$51  Output: A=$15 X=$8B Y=$74 P=$31 M=$51
		!byte	$52,$97,$36,$A1,$B0 ;Input: A=$A0 Y=$36 P=$73 M=$FF  Output: A=$A1 X=$C9 Y=$36 P=$B0 M=$FF
		!byte	$52,$61,$6C,$F4,$B0 ;Input: A=$84 Y=$6C P=$F1 M=$90  Output: A=$F4 X=$93 Y=$6C P=$B0 M=$90

		; $FD  SBC abs,x
		!byte	$F2,$FD,$2F,$00,$61,$46,$31 ;Input: A=$DA Y=$61 P=$70 M=$93  Output: A=$46 X=$9E Y=$61 P=$31 M=$93
		!byte	$72,$D9,$FF,$0B,$17,$35 ;Input: A=$EB Y=$0B P=$B6 M=$D3  Output: A=$17 X=$F4 Y=$0B P=$35 M=$D3
		!byte	$12,$0B,$62,$34 ;Input: A=$0E Y=$0B P=$B7 M=$AC  Output: A=$62 X=$F4 Y=$0B P=$34 M=$AC
		!byte	$72,$5B,$00,$8D,$D0,$B4 ;Input: A=$41 Y=$8D P=$77 M=$71  Output: A=$D0 X=$72 Y=$8D P=$B4 M=$71
		!byte	$52,$36,$68,$3A,$31 ;Input: A=$42 Y=$68 P=$B2 M=$07  Output: A=$3A X=$97 Y=$68 P=$31 M=$07
		!byte	$52,$B2,$E4,$F9,$B4 ;Input: A=$60 Y=$E4 P=$B7 M=$67  Output: A=$F9 X=$1B Y=$E4 P=$B4 M=$67
		!byte	$72,$E1,$FF,$13,$45,$31 ;Input: A=$46 Y=$13 P=$33 M=$01  Output: A=$45 X=$EC Y=$13 P=$31 M=$01
		!byte	$72,$78,$00,$AA,$F7,$B4 ;Input: A=$6B Y=$AA P=$34 M=$73  Output: A=$F7 X=$55 Y=$AA P=$B4 M=$73

		; $FE  INC abs,x
		!byte	$E3,$FE,$74,$00,$A6,$F0,$E6 ;Input: A=$BB Y=$A6 P=$70 M=$E5  Output: A=$BB X=$59 Y=$A6 P=$F0 M=$E6
		!byte	$43,$27,$59,$74,$52 ;Input: A=$34 Y=$59 P=$76 M=$51  Output: A=$34 X=$A6 Y=$59 P=$74 M=$52
		!byte	$43,$57,$89,$B5,$F1 ;Input: A=$66 Y=$89 P=$35 M=$F0  Output: A=$66 X=$76 Y=$89 P=$B5 M=$F1
		!byte	$43,$8C,$BE,$70,$07 ;Input: A=$90 Y=$BE P=$F0 M=$06  Output: A=$90 X=$41 Y=$BE P=$70 M=$07
		!byte	$63,$D5,$FF,$07,$75,$73 ;Input: A=$AE Y=$07 P=$77 M=$72  Output: A=$AE X=$F8 Y=$07 P=$75 M=$73
		!byte	$61,$79,$00,$AB,$F0 ;Input: A=$63 Y=$AB P=$B4 M=$EF  Output: A=$63 X=$54 Y=$AB P=$B4 M=$F0
		!byte	$43,$75,$A7,$70,$39 ;Input: A=$E5 Y=$A7 P=$F2 M=$38  Output: A=$E5 X=$58 Y=$A7 P=$70 M=$39
		!byte	$43,$BB,$ED,$70,$61 ;Input: A=$C8 Y=$ED P=$F2 M=$60  Output: A=$C8 X=$12 Y=$ED P=$70 M=$61
test_end:
