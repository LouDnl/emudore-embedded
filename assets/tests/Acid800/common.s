videoram = $0400
colorram = $d800

vcount = $d012

ERROR = $02

; LIBRARY ZP
a0              =     $c0
a1              =     $c2
a2              =     $c4
a3              =     $c6

d0              =     $c8
; d1,d2,d3,d4,d5 used for A/X/Y/p/m in printf
d1              =     $c9
d2              =     $ca
d3              =     $cb
d4              =     $cc
d5              =     $cd
d6              =     $ce
d7              =     $cf

ptr = $f0

scrptr = $f2

!macro _ASSERT1 .addr, .val, .num {
        lda .addr
        cmp #.val
        beq +
        lda #.num
        jmp     _testFailed
+:
}

!macro _ASSERTA .val, .num {
        cmp #.val
        beq +
        lda #.num
        jmp     _testFailed
+:
}
!macro _ASSERTX .val, .num {
        cpx #.val
        beq +
        lda #.num
        jmp     _testFailed
+:
}
!macro _ASSERTY .val, .num {
        cpy #.val
        beq +
        lda #.num
        jmp     _testFailed
+:
}

!macro _FAIL num {
        lda #num
        jmp     _testFailed
}

!macro _SKIP num {
        lda #num
        jmp     _testFailed
}

;-------------------------------------------------------------------------------
            *=$07ff
            !word $0801
            !word bend
            !word 10
            !byte $9e
            !text "2061", 0
bend:       !word 0
;-------------------------------------------------------------------------------

            sei
            lda #$17
            sta $d018
            lda #$35
            sta $01

            jsr _interruptsOff

            ldx #0
            stx ERROR
-
            lda #$20
            sta videoram,x
            sta videoram+$0100,x
            sta videoram+$0200,x
            sta videoram+$0300,x
            lda #1
            sta colorram,x
            sta colorram+$0100,x
            sta colorram+$0200,x
            sta colorram+$0300,x
            inx
            bne -

            ldx #2
            lda #0
-
            sta 0,x
            inx
            bne -

            jsr main

            lda ERROR
            bne _fail

_testPassed:
            lda #5
            sta $d020
            lda #$00
            sta $d7ff
            jmp *
_fail
            sta videoram+(24*40)+0
            lda #10
            sta $d020
            lda #$ff
            sta $d7ff
            jmp *

_testFailed2
            sta pra2+1 ; lo
            sty pra2+2 ; hi
            ldx #0
pra2:       lda $dead,x
            beq +
            sta videoram+(20*40),x
            inx
            bne pra2
+
            ldy #>(videoram+(20*40)+8)
            lda #<(videoram+(20*40)+8)
            jsr _setprinthex
            lda d1
            jsr _printhex
            jsr _printspc
            lda d2
            jsr _printhex
            jsr _printspc
            lda d3
            jsr _printhex
            jsr _printspc
            jmp _testFailed1

_testFailed
            sta videoram+(24*40)+1
            stx videoram+(24*40)+2
            sty videoram+(24*40)+3
_testFailed1
            inc ERROR
            lda ERROR
            jmp _fail
 
_testInit:
            sta pra1+1
            sty pra1+2
            ldx #0
pra1:       lda $dead,x
            beq +
            sta videoram+(22*40)+1,x
            inx
            bne pra1
+
_printfinit:
            pha
            lda #0
            sta lprn+1
            pla
            rts

_screenOff:
            rts

_interruptsOff:
            lda #$7f
            sta $dc0d
            sta $dd0d
            lda #0
            sta $d01a

            inc $d019
            lda $dc0d
            lda $dd0d
            rts

_waitVCount:
            lda $d011
            bpl * - 3
            lda $d011
            bmi * - 3
            rts

_printhex:
        pha
        lsr
        lsr
        lsr
        lsr
        tax
        lda _hextab,x
        ldy #0
        sta (scrptr),y
        pla
        and #$0f
        tax
        lda _hextab,x
        iny
        sta (scrptr),y

        lda scrptr
        ldy scrptr+1
        clc
        adc #2
        bcc +
        iny
+
_setprinthex:
        sta scrptr
        sty scrptr+1
        rts

_hextab: !scr "0123456789abcdef"

_printspc:
        lda #$20
_printchr:
        ldy #0
        sta (scrptr),y
        inc scrptr
        bne +
        inc scrptr+1
+
        rts

_printf:

;    jmp *
        sta ptr
        sty ptr+1
        ldy #0
-
        lda (ptr),y
        bne ++
        ;dey
        tya
        clc
        adc ptr
        sta ptr
        bcc +
        inc ptr+1
+
        tya
        clc
        adc lprn+1
        sta lprn+1

        rts
++
lprn    sta $400,y
        iny
        bne -
        inc ptr+1
        rts

_imprintf:

        pla
        tax
        pla
        tay

        inx ; lo
        bne +
        iny ; hi
+
        txa
        jsr     _printf

        lda     ptr+1
        pha
        lda     ptr
        pha
        rts

            inc $d020
            jmp * -3