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

        * = $2000

main:
        ldy #>testname
        lda #<testname
        jsr _testInit
        
        jsr _screenOff
        jsr _interruptsOff

        ;test very famous bug
        jmp (jmpbugvec)
        
        * = $2200
jmpbugok:

        ;Test for BRK bug.
        ;
        ;We need an NMI to do this
        
        lda #$7f
        sta $dc0d
        sta $dd0d
        lda $dc0d
        lda $dd0d
        
        lda #<irq
        sta $fffe
        lda #>irq
        sta $ffff

        lda #<nmi
        sta $fffa
        lda #>nmi
        sta $fffb
        sei

-       bit $d011
        bpl -
-       bit $d011
        bmi -

        lda #0          ; stop timer
        sta $dd0e
        lda #>$0007 
        sta $dd05
        lda #<$0007
        sta $dd04
        lda #$81
        sta $dd0d
                
        lda #$11        ; start timer
brktest:
        sta $dd0e
        
        lda $0100
        lda $01
        nop
        	
        ;This needs to start executing at cycle 4-8. If it executes at cycle
        ;3 then it blocks the NMI!
        brk
after:
        +_FAIL 1
        ;"Execution went past a BRK insn."

;============================================================================
irq:
        +_FAIL 2
        ;c"BRK handler should not have executed."


;============================================================================
        * = $2300
nmi:
        pha
        txa
        pha
        tya
        pha
        
        lda #0          ; stop timer
        sta $dd0e
        
        ;check return location, and see if it is in our test code; ignore
        ;if it is in the OS or in the IRQ handler as then we need to let
        ;the test fail
        tsx
        lda $0106,x
        cmp #>brktest
        beq retok

        ;oops... return and let the IRQ routine fail
        pla
        tay
        pla
        tax
        pla
        rti
        
retok:
        ;check low return byte
        tsx
        lda $0105,x
        sec
        sbc #<(after+1)
        
        +_ASSERTA $00, 3
        ;c"NMI handler executed too early or late: %x"
        
        ;check the B flag; it should be set
        lda $0104,x
        and #$20
        
        +_ASSERTA $20, 4
        ;c"B flag was not set on entry to NMI handler."
        
        jmp _testPassed
        
        
;============================================================================
testname:
        !scr "CPU: Bugs",0
        
;============================================================================

        * = $2400
        !byte $24
        
        * = $2480
        jmp jmpbugok
        
        * = $24ff
jmpbugvec:
        !byte $80
        !byte $25
        
        * = $2580
        +_FAIL	5
        ;"JMP indirect bug not found.",0
