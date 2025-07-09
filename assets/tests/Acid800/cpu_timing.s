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
		ldy		#>testname
		lda		#<testname
		jsr		_testInit
		
		jsr		_screenOff
		jsr		_interruptsOff

		;test empty timing loop
		lda		#0
		jsr		_waitVCount
		;inc            wsync

		ldx		#210
tloop1:
		dex
		bne		tloop1
		ldx		vcount
		
		!if (MODE = 0) {
		+_ASSERTX ((3*$05)+1), 1
		}
		!if (MODE = 1) {
		+_ASSERTX ((3*$05)+1), 1
		}
		;c"Incorrect DEX/BNE cycle count: %d"

		;test NOP
		lda		#0
		jsr		_waitVCount
		;inc            wsync

		ldx		#210
tloop2:
		nop
		dex
		bne		tloop2
		ldx		vcount
		
		!if (MODE = 0) {
		+_ASSERTX ((3 * $07)+2), 2
		}
		!if (MODE = 1) {
		+_ASSERTX ((3 * $07)+1), 2
		}
		;c"Incorrect NOP cycle count: %d-5"

		;test LDA abs
		lda		#0
		jsr		_waitVCount
		;inc            wsync

		ldx		#210
tloop3:
		lda		$0100
		dex
		bne		tloop3
		ldx		vcount
		
		!if (MODE = 0) {
		+_ASSERTX ((3*$09)+3), 3
		}
		!if (MODE = 1) {
		+_ASSERTX ((3*$09)+2), 3
		}
		;c"Incorrect LDA abs cycle count: %d-5"

		;test LDA abs,X, no page crossing
		lda		#0
		jsr		_waitVCount
		;inc            wsync

		ldx		#210
tloop4:
		lda		$0100,x
		dex
		bne		tloop4
		ldx		vcount
		
		!if (MODE = 0) {
		+_ASSERTX ((3*$09)+3), 4
		}
		!if (MODE = 1) {
		+_ASSERTX ((3*$09)+2), 4
		}
		;c"Incorrect LDA abs,X (1) cycle count: %d-5"

		;test LDA abs,X, page crossing
		lda		#0
		jsr		_waitVCount

		;inc            wsync

		ldx		#210
tloop5:
		lda		$01FF,x
		dex
		bne		tloop5
		ldx		vcount
		
		!if (MODE = 0) {
		+_ASSERTX ((3*$0a)+3), 5
		}
		!if (MODE = 1) {
		+_ASSERTX ((3*$0a)+2), 5
		}
		;c"Incorrect LDA abs,X (2) cycle count: %d-5"
		
		;test STA abs,X, no page crossing
		lda		#0
		jsr		_waitVCount

		;inc            wsync

		ldx		#210
tloop6:
		sta		$2800,x
		dex
		bne		tloop6
		ldx		vcount
		
		!if (MODE = 0) {
		+_ASSERTX ((3*$0a)+3), 6
		}
		!if (MODE = 1) {
		+_ASSERTX ((3*$0a)+2), 6
		}
		;c"Incorrect STA abs,X (1) cycle count: %d-5"
		
		;test STA abs,X, page crossing
		lda		#0
		jsr		_waitVCount

		;inc            wsync

		ldx		#210
tloop7:
		sta		$28FF,x
		dex
		bne		tloop7
		ldx		vcount
		
		!if (MODE = 0) {
		+_ASSERTX ((3*$0a)+3), 7
		}
		!if (MODE = 1) {
		+_ASSERTX ((3*$0a)+2), 7
		}
		;c"Incorrect STA abs,X (2) cycle count: %d-5"
		
		;test LDA zp
		lda		#0
		jsr		_waitVCount

		;inc            wsync

		ldx		#210
tloop8:
		lda		$00
		dex
		bne		tloop8
		ldx		vcount
		
		!if (MODE = 0) {
		+_ASSERTX ((3*$08)+2), 8
		}
		!if (MODE = 1) {
		+_ASSERTX ((3*$08)+2), 8
		}
		;c"Incorrect LDA zp cycle count: %d-5"
		
		;test LDA zp,X
		lda		#0
		jsr		_waitVCount

		;inc            wsync

		ldx		#210
tloop9:
		lda		$00,x
		dex
		bne		tloop9
		ldx		vcount
		
		!if (MODE = 0) {
		+_ASSERTX ((3*$09)+3), 9
		}
		!if (MODE = 1) {
		+_ASSERTX ((3*$09)+2), 9
		}
		;c"Incorrect LDA zp,X cycle count: %d-5"
		
		;test INC zp
		lda		#0
		jsr		_waitVCount

		;inc            wsync

		ldx		#210
tloop10:
		inc		d0
		dex
		bne		tloop10
		ldx		vcount
		
		!if (MODE = 0) {
		+_ASSERTX ((3*$0a)+3), 10
		}
		!if (MODE = 1) {
		+_ASSERTX ((3*$0a)+2), 10
		}
		;c"Incorrect INC zp cycle count: %d-5"
		
		;test INC zp,X
		ldx		#0
		txa
		jsr		_waitVCount

		;inc            wsync

		ldy		#210
tloop11:
		inc		d0,x
		dey
		bne		tloop11
		ldx		vcount
		
		!if (MODE = 0) {
		+_ASSERTX ((3*$0b)+3), 11
		}
		!if (MODE = 1) {
		+_ASSERTX ((3*$0b)+2), 11
		}
		;c"Incorrect INC zp,X cycle count: %d-5"

		;test INC abs
		ldx		#0
		txa
		jsr		_waitVCount

		;inc            wsync

		ldy		#210
tloop12:
		inc		$2800
		dey
		bne		tloop12
		ldx		vcount
		
		!if (MODE = 0) {
		+_ASSERTX ((3*$0b)+3), 12
		}
		!if (MODE = 1) {
		+_ASSERTX ((3*$0b)+2), 12
		}
		;c"Incorrect INC abs cycle count: %d-5"
		
		;test INC abs,X
		ldx		#0
		txa
		jsr		_waitVCount

		;inc            wsync

		ldx		#210
tloop13:
		inc		$2800,x
		dex
		bne		tloop13
		ldx		vcount
		
		!if (MODE = 0) {
		+_ASSERTX ((3*$0c)+4), 13
		}
		!if (MODE = 1) {
		+_ASSERTX ((3*$0c)+3), 13
		}
		;c"Incorrect INC abs,X cycle count: %d-5"
		
		;test LDA (zp),y, no page crossing
		;+mwa             $2800, $a0
                lda #<$2800
                sta $a0
                lda #>$2800
                sta $a0+1

		ldy		#0
		tya
		jsr		_waitVCount

		;inc            wsync

		ldx		#210
tloop14:
		lda		(a0),y
		dex
		bne		tloop14
		ldx		vcount
		
		!if (MODE = 0) {
		+_ASSERTX ((3*$0a)+3), 14
		}
		!if (MODE = 1) {
		+_ASSERTX ((3*$0a)+2), 14
		}
		;c"Incorrect LDA (zp),Y (1) cycle count: %d-5"

		;test LDA (zp),y, page crossing
		;+mwa             $28FF, $a0
                lda #<$28ff
                sta $a0
                lda #>$28ff
                sta $a0+1

		ldy		#1
		lda		#0
		jsr		_waitVCount

		;inc            wsync

		ldx		#210
tloop15:
		lda		(a0),y
		dex
		bne		tloop15
		ldx		vcount
		
		!if (MODE = 0) {
		+_ASSERTX ((3*$0b)+0), 15
		}
		!if (MODE = 1) {
		+_ASSERTX ((3*$0a)+2), 15
		}
		;c"Incorrect LDA (zp),Y (2) cycle count: %d-5"
		
		;test LDA (zp),X
		;+mwa             $28FF, $a0
                lda #<$28ff
                sta $a0
                lda #>$28ff
                sta $a0+1

		ldx		#0
		lda		#0
		jsr		_waitVCount

		;inc            wsync

		ldy		#210
tloop16:
		lda		(a0,x)
		dey
		bne		tloop16
		ldx		vcount
		
		!if (MODE = 0) {
		+_ASSERTX ((3*$0b)+3), 16
		}
		!if (MODE = 1) {
		+_ASSERTX ((3*$0b)+2), 16
		}
		;c"Incorrect LDA (zp,X) cycle count: %d-5"
		
		jmp		_testPassed
		
testname:
		!scr		"CPU: Timing",0
		
testbuf	=		$2800		;$2800-2FFF reserved
