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

		* =		$2000

main:
		ldy		#>testname
		lda		#<testname
		jsr		_testInit
		
		jsr		_screenOff
		jsr		_interruptsOff

		sed
		clc
		lda		#$06
		adc		#$19
		sta		d0
		php
		pla
		and		#$c3
		sta		d1
		
;               _ASSERT1 d0, $25, c"$06+$19=$%x, should be $25.",0
;		_ASSERT1 d1, $00, c"$00+$10 flags incorrect: $%x, should be $00.",0
                +_ASSERT1 d0, $25, 0
                +_ASSERT1 d1, $00, 1
		
		sec
		lda		#$7e
		adc		#$11
		sta		d0
		php
		pla
		and		#$c3
		sta		d1
		
;		_ASSERT1 d0, $96, c"$7e+$11+1=$%x, should be $96.",0
;		_ASSERT1 d1, $c0, c"$7e+$11+1 flags incorrect: $%x, should be $C0.",0
                +_ASSERT1 d0, $96, 2
                +_ASSERT1 d1, $c0, 3
;                +_ASSERT1 d1, $11, 3

		jmp		_testPassed
		
testname:
		!scr "CPU: Decimal mode",0

