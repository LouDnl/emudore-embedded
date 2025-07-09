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

                !src "common.s"

                * =             $2000
main:
                ldy             #>testname
                lda             #<testname
                jsr             _testInit

                lda #0
                !byte $ab, $ff
                sta laxmagic

                ldx #0
-
                lda laximmtest+3,x ; A
                ora laxmagic
                and laximmtest+1,x ; imm
                sta laximmtest+8,x ; A out
                sta laximmtest+9,x ; X out
                txa
                clc
                adc #13
                tax
                cpx #13*3
                bne -

		;mwa		#test_start a0
                lda #<test_start
                sta a0
                lda #>test_start
                sta a0+1

loop_start:
		;load instruction
		ldy		#0
                lda (a0),y
                sta insn
		iny
		lda (a0),y
		sta insn+1
		iny
		lda (a0),y
		sta insn+2
		iny
		
		;setup temp registers
		;mwa		#d5 a1
		;mwa		#d4 a2
                lda #<d5
                sta a1
                lda #>d5
                sta a1+1
                lda #<d4
                sta a2
                lda #>d4
                sta a2+1

		;stash A
		;mva		(a0),y d0
                lda (a0),y
                sta d0
		iny
		
		;load X
		lda		(a0),y
		tax
		iny
		
		;stash Y
		;mva		(a0),y d1
                lda (a0),y
                sta d1
		iny
		
		;stash P
		lda		(a0),y
		iny
		pha
		
		;load d5
		;mva		(a0),y d5
                lda (a0),y
                sta d5
		iny
		;load Y
		ldy		d1
		
		;load A
		lda		d0
		
		;load P
		plp
		
;inc $d020
;jmp * - 3
		;execute insn
		jsr		insn

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
                ldy             #3
                lda             (a0),y
                sta $0400+(3*40)+1
                iny
                lda             (a0),y
                sta $0400+(3*40)+2
                iny
                lda             (a0),y
                sta $0400+(3*40)+3
                iny
                lda             (a0),y
                sta $0400+(3*40)+4
                iny
                lda             (a0),y
                sta $0400+(3*40)+5

                ldy             #8
                lda             (a0),y
                sta $0400+(4*40)+1
                iny
                lda             (a0),y
                sta $0400+(4*40)+2
                iny
                lda             (a0),y
                sta $0400+(4*40)+3
                iny
                lda             (a0),y
                sta $0400+(4*40)+4
                iny
                lda             (a0),y
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

;jmp *
}

		;compare registers
		ldy		#8
		lda		(a0),y ; A
		cmp		d1
		bne		fail
		iny
		lda		(a0),y ; X
		cmp		d2
		bne		fail
		iny
		lda		(a0),y ; Y
		cmp		d3
		bne		fail
		iny
		lda		(a0),y ; P
		cmp		d4
		bne		fail
                iny
		lda		(a0),y ; M
		cmp		d5
		bne		fail
		
		;go another round
		lda		a0
		clc
		adc		#13
		sta		a0
		bcc +
		inc	a0+1
+

                inc $0400+(24*40)+39

		cmp		#<test_end
		bne		loop_end
		lda		a0+1
		cmp		#>test_end
		beq		loop_exit
loop_end:
		jmp		loop_start
loop_exit:
		jmp		_testPassed

fail:
		cld
		cli
                jsr             _printfinit
 		jsr		_imprintf
		!scr "fail: A=00 X=00 Y=00 P=00 M=00",0
                ldy #>($0400+(0*40)+8)
                lda #<($0400+(0*40)+8)
                jsr  _setprinthex
                lda d1
                jsr  _printhex

                ldy #>($0400+(0*40)+8+5)
                lda #<($0400+(0*40)+8+5)
                jsr  _setprinthex
                lda d2
                jsr  _printhex

                ldy #>($0400+(0*40)+8+10)
                lda #<($0400+(0*40)+8+10)
                jsr  _setprinthex
                lda d3
                jsr  _printhex

                ldy #>($0400+(0*40)+8+15)
                lda #<($0400+(0*40)+8+15)
                jsr  _setprinthex
                lda d4
                jsr  _printhex

                ldy #>($0400+(0*40)+8+20)
                lda #<($0400+(0*40)+8+20)
                jsr  _setprinthex
                lda d5
                jsr  _printhex

		;mva		insn d1
                lda insn
                sta d1
		;mva		insn+1 d2
                lda insn+1
                sta d2
		;mva		insn+2 d3
                lda insn+2
                sta d3

		ldy		#>failmsg
		lda		#<failmsg
		jsr		_testFailed2
failmsg:
                     ;012345678
		!scr "  Insn: 00 00 00",0
testname:
                !scr "CPU: Illegal instructions",0

laxmagic:       !byte 0

;============================================================================
test_start:
		;                insn          A   X   Y   P   M    A   X   Y   P   M
		;SLO (zp,X) (ASL + ORA)
		!byte		$03,<a0,$60, $00,$02,$00,$30,$81, $02,$02,$00,$31,$02
		!byte		$03,<a0,$60, $f0,$02,$00,$30,$81, $f2,$02,$00,$b1,$02
		
		;NOP zp
		!byte		$04,<d5,$60, $00,$00,$00,$30,$00, $00,$00,$00,$30,$00
		!byte		$04,<d5,$60, $FF,$FF,$FF,$FB,$FF, $FF,$FF,$FF,$FB,$FF
		
		;SLO zp (ASL + ORA)
		!byte		$07,<d5,$60, $00,$00,$00,$30,$81, $02,$00,$00,$31,$02
		!byte		$07,<d5,$60, $f0,$00,$00,$30,$81, $f2,$00,$00,$b1,$02
		
		;AAC #imm (modified AND)
		!byte		$0b,$00,$60, $00,$00,$00,$30,$00, $00,$00,$00,$32,$00
		!byte		$0b,$55,$60, $ff,$00,$00,$30,$00, $55,$00,$00,$30,$00
		!byte		$0b,$a0,$60, $f0,$00,$00,$30,$00, $a0,$00,$00,$b1,$00
		!byte		$0b,$55,$60, $aa,$00,$00,$30,$00, $00,$00,$00,$32,$00

		;NOP abs
		!byte		$0c,<d5,>d5, $00,$00,$00,$30,$00, $00,$00,$00,$30,$00
		!byte		$0c,<d5,>d5, $FF,$FF,$FF,$FB,$FF, $FF,$FF,$FF,$FB,$FF
		
		;SLO abs (ASL + ORA)
		!byte		$0f,<d5,>d5, $00,$00,$00,$30,$81, $02,$00,$00,$31,$02
		!byte		$0f,<d5,>d5, $f0,$00,$00,$30,$81, $f2,$00,$00,$b1,$02

		;SLO (zp),Y (ASL + ORA)
		!byte		$13,<a2,$60, $00,$00,$01,$30,$81, $02,$00,$01,$31,$02
		!byte		$13,<a2,$60, $f0,$00,$01,$30,$81, $f2,$00,$01,$b1,$02
		
		;NOP zp,X
		!byte		$14,<d5,$60, $00,$00,$00,$30,$00, $00,$00,$00,$30,$00
		!byte		$14,<d5,$60, $FF,$FF,$FF,$FB,$FF, $FF,$FF,$FF,$FB,$FF
		
		;SLO zp,X (ASL + ORA)
		!byte		$17,<d4,$60, $00,$01,$00,$30,$81, $02,$01,$00,$31,$02
		!byte		$17,<d4,$60, $f0,$01,$00,$30,$81, $f2,$01,$00,$b1,$02
		
		;NOP
		!byte		$1A,$60,$60, $00,$00,$00,$30,$00, $00,$00,$00,$30,$00
		!byte		$1A,$60,$60, $FF,$FF,$FF,$FB,$FF, $FF,$FF,$FF,$FB,$FF
		
		;SLO abs,Y (ASL + ORA)
		!byte		$1B,<d2,>d2, $00,$00,$03,$30,$81, $02,$00,$03,$31,$02
		!byte		$1B,<d2,>d2, $f0,$00,$03,$30,$81, $f2,$00,$03,$b1,$02
		
		;NOP abs,X
		!byte		$1C,<d5,>d5, $00,$00,$00,$30,$00, $00,$00,$00,$30,$00
		!byte		$1C,<d5,>d5, $FF,$FF,$FF,$FB,$FF, $FF,$FF,$FF,$FB,$FF
		
		;SLO abs,X (ASL + ORA)
		!byte		$1F,<d3,>d3, $00,$02,$03,$30,$81, $02,$02,$03,$31,$02
		!byte		$1F,<d3,>d3, $f0,$02,$03,$30,$81, $f2,$02,$03,$b1,$02
		
		;RLA (zp,X) (ROL + AND)
		!byte		$23,<a0,$60, $ff,$02,$00,$31,$81, $03,$02,$00,$31,$03
		
		;RLA zp (ROL + AND)
		!byte		$27,<d5,$60, $ff,$00,$00,$31,$81, $03,$00,$00,$31,$03

		;RLA abs (ROL + AND)
		!byte		$2F,<d5,>d5, $ff,$00,$00,$31,$81, $03,$00,$00,$31,$03
		
		;RLA (zp),Y (ROL + AND)
		!byte		$33,<a2,$60, $ff,$00,$01,$31,$81, $03,$00,$01,$31,$03
		
		;NOP zp,X
		!byte		$34,<d5,$60, $00,$00,$00,$30,$00, $00,$00,$00,$30,$00
		!byte		$34,<d5,$60, $FF,$FF,$FF,$FB,$FF, $FF,$FF,$FF,$FB,$FF
		
		;RLA zp,X (ROL + AND)
		!byte		$37,<d4,$60, $ff,$01,$00,$31,$81, $03,$01,$00,$31,$03
		
		;NOP
		!byte		$3A,$60,$60, $00,$00,$00,$30,$00, $00,$00,$00,$30,$00
		!byte		$3A,$60,$60, $FF,$FF,$FF,$FB,$FF, $FF,$FF,$FF,$FB,$FF
		
		;RLA abs,Y (ROL + AND)
		!byte		$3B,<d4,>d4, $ff,$00,$01,$31,$81, $03,$00,$01,$31,$03
		
		;NOP abs,X
		!byte		$3C,<d5,>d5, $00,$00,$00,$30,$00, $00,$00,$00,$30,$00
		!byte		$3C,<d5,>d5, $FF,$FF,$FF,$FB,$FF, $FF,$FF,$FF,$FB,$FF
		
		;RLA abs,X (ROL + AND)
		!byte		$3F,<d4,>d4, $ff,$01,$00,$31,$81, $03,$01,$00,$31,$03
		
		;SRE (zp,X) (LSR + EOR)
		!byte		$43,<a0,$60, $0f,$02,$00,$30,$55, $25,$02,$00,$31,$2a
		
		;NOP zp
		!byte		$44,<d5,$60, $00,$00,$00,$30,$00, $00,$00,$00,$30,$00
		!byte		$44,<d5,$60, $FF,$FF,$FF,$FB,$FF, $FF,$FF,$FF,$FB,$FF
		
		;SRE zp (LSR + EOR)
		!byte		$47,<d5,$60, $0f,$00,$00,$30,$55, $25,$00,$00,$31,$2a
		
		;ASR #imm (AND + LSR)
		!byte		$4B,$55,$60, $ff,$00,$00,$30,$55, $2a,$00,$00,$31,$55
		
		;SRE abs (LSR + EOR)
		!byte		$4F,<d5,>d5, $0f,$00,$00,$30,$55, $25,$00,$00,$31,$2a
		
		;SRE (zp),Y (LSR + EOR)
		!byte		$53,<a2,$60, $0f,$00,$01,$30,$55, $25,$00,$01,$31,$2a
		
		;NOP zp,X
		!byte		$54,<d5,$60, $00,$00,$00,$30,$00, $00,$00,$00,$30,$00
		!byte		$54,<d5,$60, $FF,$FF,$FF,$FB,$FF, $FF,$FF,$FF,$FB,$FF
		
		;SRE zp,X (LSR + EOR)
		!byte		$57,<d2,$60, $0f,$03,$00,$30,$55, $25,$03,$00,$31,$2a
		
		;NOP
		!byte		$5A,$60,$60, $00,$00,$00,$30,$00, $00,$00,$00,$30,$00
		!byte		$5A,$60,$60, $FF,$FF,$FF,$FB,$FF, $FF,$FF,$FF,$FB,$FF
		
		;SRE abs,Y (LSR + EOR)
		!byte		$5B,<d1,>d1, $0f,$00,$04,$30,$55, $25,$00,$04,$31,$2a
		
		;NOP abs,X
		!byte		$5C,<d5,>d5, $00,$00,$00,$30,$00, $00,$00,$00,$30,$00
		!byte		$5C,<d5,>d5, $FF,$FF,$FF,$FB,$FF, $FF,$FF,$FF,$FB,$FF
		
		;SRE abs,X (LSR + EOR)
		!byte		$5F,<d1,>d1, $0f,$04,$00,$30,$55, $25,$04,$00,$31,$2a
		
		;RRA (zp,X) (ROR + ADC)
		!byte		$63,<a0,$60, $05,$02,$00,$31,$01, $86,$02,$00,$b0,$80
		
		;NOP zp
		!byte		$64,<d5,$60, $00,$00,$00,$30,$00, $00,$00,$00,$30,$00
		!byte		$64,<d5,$60, $FF,$FF,$FF,$FB,$FF, $FF,$FF,$FF,$FB,$FF
		
		;RRA zp (ROR + ADC)
		!byte		$67,<d5,$60, $05,$00,$00,$31,$01, $86,$00,$00,$b0,$80
		
		;ARR #imm (AND + ROR + N/V fiddling)
		!byte		$6B,$2a,$60, $f0,$00,$00,$31,$01, $90,$00,$00,$b0,$01
		!byte		$6B,$6a,$60, $f0,$00,$00,$31,$01, $b0,$00,$00,$f0,$01
		!byte		$6B,$aa,$60, $f0,$00,$00,$31,$01, $d0,$00,$00,$f1,$01
		!byte		$6B,$ea,$60, $f0,$00,$00,$31,$01, $f0,$00,$00,$b1,$01

		;RRA abs (ROR + ADC)
		!byte		$6F,<d5,>d5, $05,$00,$00,$31,$01, $86,$00,$00,$b0,$80
		
		;RRA (zp),Y (ROR + ADC)
		!byte		$73,<a2,$60, $05,$00,$01,$31,$01, $86,$00,$01,$b0,$80
		
		;NOP zp,X
		!byte		$74,<d5,$60, $00,$00,$00,$30,$00, $00,$00,$00,$30,$00
		!byte		$74,<d5,$60, $FF,$FF,$FF,$FB,$FF, $FF,$FF,$FF,$FB,$FF
		
		;RRA zp,X (ROR + ADC)
		!byte		$77,<d4,$60, $05,$01,$00,$31,$01, $86,$01,$00,$b0,$80
		
		;NOP
		!byte		$7A,$60,$60, $00,$00,$00,$30,$00, $00,$00,$00,$30,$00
		!byte		$7A,$60,$60, $FF,$FF,$FF,$FB,$FF, $FF,$FF,$FF,$FB,$FF
		
		;RRA abs,Y (ROR + ADC)
		!byte		$7B,<d4,>d4, $05,$00,$01,$31,$01, $86,$00,$01,$b0,$80
		
		;NOP abs,X
		!byte		$7C,<d5,>d5, $00,$00,$00,$30,$00, $00,$00,$00,$30,$00
		!byte		$7C,<d5,>d5, $FF,$FF,$FF,$FB,$FF, $FF,$FF,$FF,$FB,$FF
		
		;RRA abs,X (ROR + ADC)
		!byte		$7F,<d4,>d4, $05,$01,$00,$31,$01, $86,$01,$00,$b0,$80
		
		;NOP #imm
		!byte		$80,$00,$60, $00,$00,$00,$30,$00, $00,$00,$00,$30,$00
		!byte		$80,$00,$60, $FF,$FF,$FF,$FB,$FF, $FF,$FF,$FF,$FB,$FF
		
		;NOP #imm
		!byte		$82,$00,$60, $00,$00,$00,$30,$00, $00,$00,$00,$30,$00
		!byte		$82,$00,$60, $FF,$FF,$FF,$FB,$FF, $FF,$FF,$FF,$FB,$FF
		
		;SAX (zp,X) (store A&X)
		!byte		$83,<(a1-$ff),$60, $ff,$ff,$00,$31,$00, $ff,$ff,$00,$31,$ff
		!byte		$83,<(a1-$55),$60, $ff,$55,$00,$31,$00, $ff,$55,$00,$31,$55
		!byte		$83,<(a1-$ff),$60, $aa,$ff,$00,$31,$00, $aa,$ff,$00,$31,$aa
		!byte		$83,<(a1-$00),$60, $00,$00,$00,$31,$00, $00,$00,$00,$31,$00
		
		;SAX zp (store A&X)
		!byte		$87,<d5,$60, $ff,$ff,$00,$31,$00, $ff,$ff,$00,$31,$ff
		!byte		$87,<d5,$60, $ff,$55,$00,$31,$00, $ff,$55,$00,$31,$55
		!byte		$87,<d5,$60, $aa,$ff,$00,$31,$00, $aa,$ff,$00,$31,$aa
		!byte		$87,<d5,$60, $00,$00,$00,$31,$00, $00,$00,$00,$31,$00
		
		;SAX abs (store A&X)
		!byte		$8F,<d5,>d5, $ff,$ff,$00,$31,$00, $ff,$ff,$00,$31,$ff
		!byte		$8F,<d5,>d5, $ff,$55,$00,$31,$00, $ff,$55,$00,$31,$55
		!byte		$8F,<d5,>d5, $aa,$ff,$00,$31,$00, $aa,$ff,$00,$31,$aa
		!byte		$8F,<d5,>d5, $00,$00,$00,$31,$00, $00,$00,$00,$31,$00

		;NOP #imm
		!byte		$89,$00,$60, $00,$00,$00,$30,$00, $00,$00,$00,$30,$00
		!byte		$89,$00,$60, $FF,$FF,$FF,$FB,$FF, $FF,$FF,$FF,$FB,$FF
		
		;SAX zp,Y (store A&X)
		!byte		$97,<d4,$60, $ff,$ff,$01,$31,$00, $ff,$ff,$01,$31,$ff
		!byte		$97,<d4,$60, $ff,$55,$01,$31,$00, $ff,$55,$01,$31,$55
		!byte		$97,<d4,$60, $aa,$ff,$01,$31,$00, $aa,$ff,$01,$31,$aa
		!byte		$97,<d4,$60, $00,$00,$01,$31,$00, $00,$00,$01,$31,$00
		
		;LAX (zp,X) (load A&X)
		!byte		$A3,<a0,$60, $aa,$02,$00,$31,$00, $00,$00,$00,$33,$00
		!byte		$A3,<a0,$60, $aa,$02,$00,$31,$01, $01,$01,$00,$31,$01
		!byte		$A3,<a0,$60, $aa,$02,$00,$31,$ff, $ff,$ff,$00,$b1,$ff
		
		;LAX zp (load A&X)
		!byte		$A7,<d5,$60, $aa,$55,$00,$31,$00, $00,$00,$00,$33,$00
		!byte		$A7,<d5,$60, $aa,$55,$00,$31,$01, $01,$01,$00,$31,$01
		!byte		$A7,<d5,$60, $aa,$55,$00,$31,$ff, $ff,$ff,$00,$b1,$ff

		;LAX abs (load A&X)
		!byte		$AF,<d5,>d5, $aa,$55,$00,$31,$00, $00,$00,$00,$33,$00
		!byte		$AF,<d5,>d5, $aa,$55,$00,$31,$01, $01,$01,$00,$31,$01
		!byte		$AF,<d5,>d5, $aa,$55,$00,$31,$ff, $ff,$ff,$00,$b1,$ff
		
		;LAX (zp),Y (load A&X)
		!byte		$B3,<a2,$60, $aa,$55,$01,$31,$00, $00,$00,$01,$33,$00
		!byte		$B3,<a2,$60, $aa,$55,$01,$31,$01, $01,$01,$01,$31,$01
		!byte		$B3,<a2,$60, $aa,$55,$01,$31,$ff, $ff,$ff,$01,$b1,$ff
		
		;LAX zp,Y (load A&X)
		!byte		$B7,<d3,$60, $aa,$55,$02,$31,$00, $00,$00,$02,$33,$00
		!byte		$B7,<d3,$60, $aa,$55,$02,$31,$01, $01,$01,$02,$31,$01
		!byte		$B7,<d3,$60, $aa,$55,$02,$31,$ff, $ff,$ff,$02,$b1,$ff
		
		;*** LAS $BB
		
		;LAX abs,Y (load A&X)
		!byte		$BF,<d4,>d4, $aa,$55,$01,$31,$00, $00,$00,$01,$33,$00
		!byte		$BF,<d4,>d4, $aa,$55,$01,$31,$01, $01,$01,$01,$31,$01
		!byte		$BF,<d4,>d4, $aa,$55,$01,$31,$ff, $ff,$ff,$01,$b1,$ff
		
		;NOP #imm
		!byte		$C2,$00,$60, $00,$00,$00,$30,$00, $00,$00,$00,$30,$00
		!byte		$C2,$00,$60, $FF,$FF,$FF,$FB,$FF, $FF,$FF,$FF,$FB,$FF
		
		;DCP (zp,X) (DEC + CMP)
		!byte		$C3,<a0,$60, $f0,$02,$00,$31,$01, $f0,$02,$00,$b1,$00
		!byte		$C3,<a0,$60, $f0,$02,$00,$31,$00, $f0,$02,$00,$b0,$ff
		!byte		$C3,<a0,$60, $ff,$02,$00,$31,$01, $ff,$02,$00,$b1,$00
		!byte		$C3,<a0,$60, $00,$02,$00,$31,$00, $00,$02,$00,$30,$ff
		
		;DCP zp (DEC + CMP)
		!byte		$C7,<d5,$60, $f0,$55,$00,$31,$01, $f0,$55,$00,$b1,$00
		!byte		$C7,<d5,$60, $f0,$55,$00,$31,$00, $f0,$55,$00,$b0,$ff
		!byte		$C7,<d5,$60, $ff,$55,$00,$31,$01, $ff,$55,$00,$b1,$00
		!byte		$C7,<d5,$60, $00,$55,$00,$31,$00, $00,$55,$00,$30,$ff
		
		;SBX #imm (A&X -> X, X-imm -> X)
		!byte		$CB,$00,$60, $ff,$ff,$00,$31,$01, $ff,$ff,$00,$b1,$01
		!byte		$CB,$00,$60, $aa,$55,$00,$31,$01, $aa,$00,$00,$33,$01
		!byte		$CB,$00,$60, $f0,$55,$00,$31,$01, $f0,$50,$00,$31,$01
		!byte		$CB,$05,$60, $f0,$55,$00,$31,$01, $f0,$4b,$00,$31,$01
		!byte		$CB,$85,$60, $f0,$55,$00,$31,$01, $f0,$cb,$00,$b0,$01
		!byte		$CB,$50,$60, $f0,$55,$00,$31,$01, $f0,$00,$00,$33,$01

		;DCP abs (DEC + CMP)
		!byte		$CF,<d5,>d5, $f0,$55,$00,$31,$01, $f0,$55,$00,$b1,$00
		!byte		$CF,<d5,>d5, $f0,$55,$00,$31,$00, $f0,$55,$00,$b0,$ff
		!byte		$CF,<d5,>d5, $ff,$55,$00,$31,$01, $ff,$55,$00,$b1,$00
		!byte		$CF,<d5,>d5, $00,$55,$00,$31,$00, $00,$55,$00,$30,$ff
		
		;DCP (zp),Y (DEC + CMP)
		!byte		$D3,<a2,$60, $f0,$55,$01,$31,$01, $f0,$55,$01,$b1,$00
		!byte		$D3,<a2,$60, $f0,$55,$01,$31,$00, $f0,$55,$01,$b0,$ff
		!byte		$D3,<a2,$60, $ff,$55,$01,$31,$01, $ff,$55,$01,$b1,$00
		!byte		$D3,<a2,$60, $00,$55,$01,$31,$00, $00,$55,$01,$30,$ff
		
		;NOP zp,X
		!byte		$D4,<d5,$60, $00,$00,$00,$30,$00, $00,$00,$00,$30,$00
		!byte		$D4,<d5,$60, $FF,$FF,$FF,$FB,$FF, $FF,$FF,$FF,$FB,$FF
		
		;DCP zp,X (DEC + CMP)
		!byte		$D7,<d3,$60, $f0,$02,$00,$31,$01, $f0,$02,$00,$b1,$00
		!byte		$D7,<d3,$60, $f0,$02,$00,$31,$00, $f0,$02,$00,$b0,$ff
		!byte		$D7,<d3,$60, $ff,$02,$00,$31,$01, $ff,$02,$00,$b1,$00
		!byte		$D7,<d3,$60, $00,$02,$00,$31,$00, $00,$02,$00,$30,$ff
		
		;NOP
		!byte		$DA,$60,$60, $00,$00,$00,$30,$00, $00,$00,$00,$30,$00
		!byte		$DA,$60,$60, $FF,$FF,$FF,$FB,$FF, $FF,$FF,$FF,$FB,$FF
		
		;DCP abs,Y (DEC + CMP)
		!byte		$DB,<d4,>d4, $f0,$55,$01,$31,$01, $f0,$55,$01,$b1,$00
		!byte		$DB,<d4,>d4, $f0,$55,$01,$31,$00, $f0,$55,$01,$b0,$ff
		!byte		$DB,<d4,>d4, $ff,$55,$01,$31,$01, $ff,$55,$01,$b1,$00
		!byte		$DB,<d4,>d4, $00,$55,$01,$31,$00, $00,$55,$01,$30,$ff
		
		;NOP abs,X
		!byte		$DC,<d5,>d5, $00,$00,$00,$30,$00, $00,$00,$00,$30,$00
		!byte		$DC,<d5,>d5, $FF,$FF,$FF,$FB,$FF, $FF,$FF,$FF,$FB,$FF
		
		;DCP abs,Y (DEC + CMP)
		!byte		$DF,<d4,>d4, $f0,$01,$55,$31,$01, $f0,$01,$55,$b1,$00
		!byte		$DF,<d4,>d4, $f0,$01,$55,$31,$00, $f0,$01,$55,$b0,$ff
		!byte		$DF,<d4,>d4, $ff,$01,$55,$31,$01, $ff,$01,$55,$b1,$00
		!byte		$DF,<d4,>d4, $00,$01,$55,$31,$00, $00,$01,$55,$30,$ff
		
		;NOP #imm
		!byte		$E2,$00,$60, $00,$00,$00,$30,$00, $00,$00,$00,$30,$00
		!byte		$E2,$00,$60, $FF,$FF,$FF,$FB,$FF, $FF,$FF,$FF,$FB,$FF
		
		;ISB (zp,X) (INC + SBC)
		!byte		$E3,<a0,$60, $00,$02,$00,$31,$00, $ff,$02,$00,$b0,$01
		!byte		$E3,<a0,$60, $00,$02,$00,$31,$7f, $80,$02,$00,$f0,$80
		!byte		$E3,<a0,$60, $00,$02,$00,$31,$ff, $00,$02,$00,$33,$00
		!byte		$E3,<a0,$60, $00,$02,$00,$30,$00, $fe,$02,$00,$b0,$01
		!byte		$E3,<a0,$60, $00,$02,$00,$30,$7f, $7f,$02,$00,$30,$80
		!byte		$E3,<a0,$60, $00,$02,$00,$30,$ff, $ff,$02,$00,$b0,$00
		!byte		$E3,<a0,$60, $55,$02,$00,$31,$00, $54,$02,$00,$31,$01
		!byte		$E3,<a0,$60, $55,$02,$00,$31,$7f, $d5,$02,$00,$f0,$80
		!byte		$E3,<a0,$60, $55,$02,$00,$31,$ff, $55,$02,$00,$31,$00
		!byte		$E3,<a0,$60, $55,$02,$00,$30,$00, $53,$02,$00,$31,$01
		!byte		$E3,<a0,$60, $55,$02,$00,$30,$7f, $d4,$02,$00,$f0,$80
		!byte		$E3,<a0,$60, $55,$02,$00,$30,$ff, $54,$02,$00,$31,$00
		
		;ISB zp (INC + SBC)
		!byte		$E7,<d5,$60, $00,$00,$00,$31,$00, $ff,$00,$00,$b0,$01
		!byte		$E7,<d5,$60, $00,$00,$00,$31,$7f, $80,$00,$00,$f0,$80
		!byte		$E7,<d5,$60, $00,$00,$00,$31,$ff, $00,$00,$00,$33,$00
		!byte		$E7,<d5,$60, $00,$00,$00,$30,$00, $fe,$00,$00,$b0,$01
		!byte		$E7,<d5,$60, $00,$00,$00,$30,$7f, $7f,$00,$00,$30,$80
		!byte		$E7,<d5,$60, $00,$00,$00,$30,$ff, $ff,$00,$00,$b0,$00
		!byte		$E7,<d5,$60, $55,$00,$00,$31,$00, $54,$00,$00,$31,$01
		!byte		$E7,<d5,$60, $55,$00,$00,$31,$7f, $d5,$00,$00,$f0,$80
		!byte		$E7,<d5,$60, $55,$00,$00,$31,$ff, $55,$00,$00,$31,$00
		!byte		$E7,<d5,$60, $55,$00,$00,$30,$00, $53,$00,$00,$31,$01
		!byte		$E7,<d5,$60, $55,$00,$00,$30,$7f, $d4,$00,$00,$f0,$80
		!byte		$E7,<d5,$60, $55,$00,$00,$30,$ff, $54,$00,$00,$31,$00
		
		;SBC #imm
		!byte		$EB,$00,$60, $00,$00,$00,$31,$00, $00,$00,$00,$33,$00
		!byte		$EB,$01,$60, $00,$00,$00,$31,$00, $ff,$00,$00,$b0,$00
		!byte		$EB,$80,$60, $00,$00,$00,$31,$00, $80,$00,$00,$f0,$00
		!byte		$EB,$ff,$60, $00,$00,$00,$31,$00, $01,$00,$00,$30,$00
		!byte		$EB,$00,$60, $55,$00,$00,$30,$00, $54,$00,$00,$31,$00
		!byte		$EB,$01,$60, $55,$00,$00,$30,$00, $53,$00,$00,$31,$00
		!byte		$EB,$80,$60, $55,$00,$00,$30,$00, $d4,$00,$00,$f0,$00
		!byte		$EB,$ff,$60, $55,$00,$00,$30,$00, $55,$00,$00,$30,$00
		
		;ISB abs (INC + SBC)
		!byte		$EF,<d5,>d5, $00,$00,$00,$31,$00, $ff,$00,$00,$b0,$01
		!byte		$EF,<d5,>d5, $00,$00,$00,$31,$7f, $80,$00,$00,$f0,$80
		!byte		$EF,<d5,>d5, $00,$00,$00,$31,$ff, $00,$00,$00,$33,$00
		!byte		$EF,<d5,>d5, $00,$00,$00,$30,$00, $fe,$00,$00,$b0,$01
		!byte		$EF,<d5,>d5, $00,$00,$00,$30,$7f, $7f,$00,$00,$30,$80
		!byte		$EF,<d5,>d5, $00,$00,$00,$30,$ff, $ff,$00,$00,$b0,$00
		!byte		$EF,<d5,>d5, $55,$00,$00,$31,$00, $54,$00,$00,$31,$01
		!byte		$EF,<d5,>d5, $55,$00,$00,$31,$7f, $d5,$00,$00,$f0,$80
		!byte		$EF,<d5,>d5, $55,$00,$00,$31,$ff, $55,$00,$00,$31,$00
		!byte		$EF,<d5,>d5, $55,$00,$00,$30,$00, $53,$00,$00,$31,$01
		!byte		$EF,<d5,>d5, $55,$00,$00,$30,$7f, $d4,$00,$00,$f0,$80
		!byte		$EF,<d5,>d5, $55,$00,$00,$30,$ff, $54,$00,$00,$31,$00

		;ISB (zp),Y (INC + SBC)
		!byte		$F3,<a2,$60, $00,$02,$01,$31,$00, $ff,$02,$01,$b0,$01
		!byte		$F3,<a2,$60, $00,$02,$01,$31,$7f, $80,$02,$01,$f0,$80
		!byte		$F3,<a2,$60, $00,$02,$01,$31,$ff, $00,$02,$01,$33,$00
		!byte		$F3,<a2,$60, $00,$02,$01,$30,$00, $fe,$02,$01,$b0,$01
		!byte		$F3,<a2,$60, $00,$02,$01,$30,$7f, $7f,$02,$01,$30,$80
		!byte		$F3,<a2,$60, $00,$02,$01,$30,$ff, $ff,$02,$01,$b0,$00
		!byte		$F3,<a2,$60, $55,$02,$01,$31,$00, $54,$02,$01,$31,$01
		!byte		$F3,<a2,$60, $55,$02,$01,$31,$7f, $d5,$02,$01,$f0,$80
		!byte		$F3,<a2,$60, $55,$02,$01,$31,$ff, $55,$02,$01,$31,$00
		!byte		$F3,<a2,$60, $55,$02,$01,$30,$00, $53,$02,$01,$31,$01
		!byte		$F3,<a2,$60, $55,$02,$01,$30,$7f, $d4,$02,$01,$f0,$80
		!byte		$F3,<a2,$60, $55,$02,$01,$30,$ff, $54,$02,$01,$31,$00

		;NOP zp,X
		!byte		$F4,<d5,$60, $00,$00,$00,$30,$00, $00,$00,$00,$30,$00
		!byte		$F4,<d5,$60, $FF,$FF,$FF,$FB,$FF, $FF,$FF,$FF,$FB,$FF

		;ISB zp,X (INC + SBC)
		!byte		$F7,<d3,$60, $00,$02,$00,$31,$00, $ff,$02,$00,$b0,$01
		!byte		$F7,<d3,$60, $00,$02,$00,$31,$7f, $80,$02,$00,$f0,$80
		!byte		$F7,<d3,$60, $00,$02,$00,$31,$ff, $00,$02,$00,$33,$00
		!byte		$F7,<d3,$60, $00,$02,$00,$30,$00, $fe,$02,$00,$b0,$01
		!byte		$F7,<d3,$60, $00,$02,$00,$30,$7f, $7f,$02,$00,$30,$80
		!byte		$F7,<d3,$60, $00,$02,$00,$30,$ff, $ff,$02,$00,$b0,$00
		!byte		$F7,<d3,$60, $55,$02,$00,$31,$00, $54,$02,$00,$31,$01
		!byte		$F7,<d3,$60, $55,$02,$00,$31,$7f, $d5,$02,$00,$f0,$80
		!byte		$F7,<d3,$60, $55,$02,$00,$31,$ff, $55,$02,$00,$31,$00
		!byte		$F7,<d3,$60, $55,$02,$00,$30,$00, $53,$02,$00,$31,$01
		!byte		$F7,<d3,$60, $55,$02,$00,$30,$7f, $d4,$02,$00,$f0,$80
		!byte		$F7,<d3,$60, $55,$02,$00,$30,$ff, $54,$02,$00,$31,$00

		;NOP
		!byte		$FA,$60,$60, $00,$00,$00,$30,$00, $00,$00,$00,$30,$00
		!byte		$FA,$60,$60, $FF,$FF,$FF,$FB,$FF, $FF,$FF,$FF,$FB,$FF
		
		;ISB abs,Y (INC + SBC)
		!byte		$FB,<d3,>d3, $00,$00,$02,$31,$00, $ff,$00,$02,$b0,$01
		!byte		$FB,<d3,>d3, $00,$00,$02,$31,$7f, $80,$00,$02,$f0,$80
		!byte		$FB,<d3,>d3, $00,$00,$02,$31,$ff, $00,$00,$02,$33,$00
		!byte		$FB,<d3,>d3, $00,$00,$02,$30,$00, $fe,$00,$02,$b0,$01
		!byte		$FB,<d3,>d3, $00,$00,$02,$30,$7f, $7f,$00,$02,$30,$80
		!byte		$FB,<d3,>d3, $00,$00,$02,$30,$ff, $ff,$00,$02,$b0,$00
		!byte		$FB,<d3,>d3, $55,$00,$02,$31,$00, $54,$00,$02,$31,$01
		!byte		$FB,<d3,>d3, $55,$00,$02,$31,$7f, $d5,$00,$02,$f0,$80
		!byte		$FB,<d3,>d3, $55,$00,$02,$31,$ff, $55,$00,$02,$31,$00
		!byte		$FB,<d3,>d3, $55,$00,$02,$30,$00, $53,$00,$02,$31,$01
		!byte		$FB,<d3,>d3, $55,$00,$02,$30,$7f, $d4,$00,$02,$f0,$80
		!byte		$FB,<d3,>d3, $55,$00,$02,$30,$ff, $54,$00,$02,$31,$00
		
		;NOP abs,X
		!byte		$FC,<d5,>d5, $00,$00,$00,$30,$00, $00,$00,$00,$30,$00
		!byte		$FC,<d5,>d5, $FF,$FF,$FF,$FB,$FF, $FF,$FF,$FF,$FB,$FF

		;ISB abs,X (INC + SBC)
		!byte		$FF,<d3,>d3, $00,$02,$00,$31,$00, $ff,$02,$00,$b0,$01
		!byte		$FF,<d3,>d3, $00,$02,$00,$31,$7f, $80,$02,$00,$f0,$80
		!byte		$FF,<d3,>d3, $00,$02,$00,$31,$ff, $00,$02,$00,$33,$00
		!byte		$FF,<d3,>d3, $00,$02,$00,$30,$00, $fe,$02,$00,$b0,$01
		!byte		$FF,<d3,>d3, $00,$02,$00,$30,$7f, $7f,$02,$00,$30,$80
		!byte		$FF,<d3,>d3, $00,$02,$00,$30,$ff, $ff,$02,$00,$b0,$00
		!byte		$FF,<d3,>d3, $55,$02,$00,$31,$00, $54,$02,$00,$31,$01
		!byte		$FF,<d3,>d3, $55,$02,$00,$31,$7f, $d5,$02,$00,$f0,$80
		!byte		$FF,<d3,>d3, $55,$02,$00,$31,$ff, $55,$02,$00,$31,$00
		!byte		$FF,<d3,>d3, $55,$02,$00,$30,$00, $53,$02,$00,$31,$01
		!byte		$FF,<d3,>d3, $55,$02,$00,$30,$7f, $d4,$02,$00,$f0,$80
		!byte		$FF,<d3,>d3, $55,$02,$00,$30,$ff, $54,$02,$00,$31,$00

; "instable address highbyte" type opcodes:
!if (1 = 1) {
                ;SHA (zp),Y
                ; Test disabled -- AND mask is unstable on different CPUs.
                ;!byte          $93,<a2,$60, $00,$00,$01,$30,$55, $00,$00,$01,$30,$00
                ;!byte          $93,<a2,$60, $33,$55,$01,$30,$55, $33,$55,$01,$30,$01

                ;**** XAS $9B

                ;SHY abs,X
                !byte           $9C,<d3,>d3, $00,$02,$00,$30,$aa, $00,$02,$00,$30,$00
                !byte           $9C,<d3,>d3, $00,$02,$01,$30,$aa, $00,$02,$01,$30,$01
                !byte           $9C,<d3,>d3, $00,$02,$ff,$30,$aa, $00,$02,$ff,$30,$01

                ;SHX abs,Y
                !byte           $9E,<d3,>d3, $00,$00,$02,$30,$aa, $00,$00,$02,$30,$00
                !byte           $9E,<d3,>d3, $00,$01,$02,$30,$aa, $00,$01,$02,$30,$01
                !byte           $9E,<d3,>d3, $00,$ff,$02,$30,$aa, $00,$ff,$02,$30,$01

}
; instable ("magic constant") type opcodes:
!if (1 = 1) {
                ;XAA **** ($8B)

                ;ATX #imm (AND + TAX)
laximmtest:
                !byte           $AB,$00,$60, $00,$ff,$00,$30,$00, $00,$00,$00,$32,$00
                !byte           $AB,$55,$60, $33,$ff,$00,$30,$00, $11,$11,$00,$30,$00
                !byte           $AB,$80,$60, $ff,$ff,$00,$30,$00, $80,$80,$00,$b0,$00
}
test_end:

;============================================================================
		* = $3f00
insn:
		nop
		nop
		nop
		rts
