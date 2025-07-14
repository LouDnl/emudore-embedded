; this file is part of the C64 Emulator Test Suite. public domain, no copyright

; original file was: icr01.asm
;-------------------------------------------------------------------------------

            .include "common.asm"
            .include "printhb.asm"
            .include "waitkey.asm"
            .include "waitborder.asm"

;------------------------------------------------------------------------------
thisname   .null "icr01"     ; name of this test
nextname   .null "imr"       ; name of next test, "-" means no more tests
;------------------------------------------------------------------------------
main:

; FIXME: save/restore vectors shouldnt be necessary here
           sec
           jsr vector
           jsr oldmain
           clc
           jsr vector

           rts  ; SUCCESS

;---------------------------------------
; vector save/restore

; FIXME: shouldnt complete vector reset be good enough?
; used before/after main, and in waitkey
vector     
;           ldx #<$334
;           ldy #>$334
;           jmp $ff8d
            rts

;---------------------------------------

nmiadr     .word 0

onnmi
           pha
           txa
           pha
           tsx
           lda $0104,x
           sta nmiadr+0
           lda $0105,x
           sta nmiadr+1
           pla
           tax
           pla
           rti

oldmain:

;---------------------------------------
;read icr when it is $01 and check if
;$81 follows

           .block
           sei
           lda #0
           sta $dc0e
           sta $dc0f
           lda #$7f
           sta $dc0d
           lda #$81
           sta $dc0d
           bit $dc0d
.ifeq NEWCIA - 1
           lda #2 + 1
.else
           lda #2
.endif
           sta $dc04
           lda #0
           sta $dc05
           jsr waitborder
           lda #%00001001
           sta $dc0e

           lda $dc0d
           ldx $dc0d
           ldy $dc0d
           sta got1+1
           stx got2+1
           sty got3+1

.ifeq NEWCIA - 1
           cmp #$00
           beq ok1
           jsr print
           .byte 13
           .text "cia1 icr is not $00, got $"
           .byte 0
got1       lda #0
           jsr printhb
           jsr waitkey
ok1
           ldx got2+1
           cpx #$81
           beq ok2
           jsr print
           .byte 13
           .text "cia1 icr is not $81, got $"
           .byte 0
got2       lda #0
           jsr printhb
ok2
           lda got3+1
           cmp #$00
           beq ok3
           jsr print
           .byte 13
           .text "cia1 reading icr=81 did "
           .text "not clear int, got $"
           .byte 0
got3       lda #0
           jsr printhb
ok3
.else
           cmp #$01
           beq ok1
           jsr print
           .byte 13
           .text "cia1 icr is not $01, got $"
           .byte 0
got1       lda #0
           jsr printhb
           jsr waitkey
ok1
           ldx got2+1
           cpx #$00
           beq ok2
           jsr print
           .byte 13
           .text "cia1 reading icr=01 did "
           .text "not clear int, got $"
           .byte 0
got2       lda #0
           jsr printhb
ok2
           lda got3+1
           cmp #$00
           beq ok3
           jsr print
           .byte 13
           .text "reading icr=00 had "
           .text "unexpected result, got $"
           .byte 0
got3       lda #0
           jsr printhb
ok3
.endif
           .bend

;---------------------------------------
;read icr when it is $01 and check if
;nmi follows

           .block
           sei
           lda #0
           sta nmiadr+0
           sta nmiadr+1
           sta $dd0e
           sta $dd0f
           lda #$7f
           sta $dd0d
           lda #$81
           sta $dd0d
           bit $dd0d
           lda #<onnmi
           sta $0318
           lda #>onnmi
           sta $0319
.ifeq NEWCIA - 1
           lda #2 + 1
.else
           lda #2
.endif
           sta $dd04
           lda #0
           sta $dd05
           jsr waitborder
           lda #%00001001
           sta $dd0e

           lda $dd0d
           ldx $dd0d
           ldy $dd0d
           sta got1+1
           stx got2+1

.ifeq NEWCIA - 1
           sty got3+1

           cmp #$00
           beq ok1
           jsr print
           .byte 13
           .text "cia2 icr is not $00, got $"
           .byte 0
got1       lda #0
           jsr printhb
           jsr waitkey
ok1
           ldx got2+1
           cpx #$81
           beq ok2
           jsr print
           .byte 13
           .text "cia2 icr is not $81, got $"
           .byte 0
got2       lda #0
           jsr printhb
ok2
           lda got3+1
           cmp #$00
           beq ok3
           jsr print
           .byte 13
           .text "cia2 reading icr=81 did "
           .text "not clear int, got $"
           .byte 0
got3       lda #0
           jsr printhb
ok3
           lda nmiadr+1
           bne ok4
           jsr print
           .byte 13
           .text "cia2 reading icr=81 did "
           .text "prevent nmi"
           .byte 0
           jsr waitkey
ok4
.else
           cmp #$01
           beq ok1
           jsr print
           .byte 13
           .text "cia2 icr is not $01, got $"
           .byte 0
got1       lda #0
           jsr printhb
           jsr waitkey
ok1
           ldx got2+1
           cpx #$00
           beq ok2
           jsr print
           .byte 13
           .text "cia2 reading icr=01 did "
           .text "not clear icr, got $"
           .byte 0
got2       lda #0
           jsr printhb
           jsr waitkey
ok2
           lda nmiadr+1
           beq ok3
           jsr print
           .byte 13
           .text "cia2 reading icr=01 did "
           .text "not prevent nmi"
           .byte 0
           jsr waitkey
ok3
.endif
           .bend

;---------------------------------------
;read icr when it is $81 and check if
;nmi follows

           .block
           sei
           lda #0
           sta nmiadr+0
           sta nmiadr+1
           sta $dd0e
           sta $dd0f
           lda #$7f
           sta $dd0d
           lda #$81
           sta $dd0d
           bit $dd0d
           lda #<onnmi
           sta $0318
           lda #>onnmi
           sta $0319
.ifeq NEWCIA - 1
           lda #1 + 1
.else
           lda #1
.endif
           sta $dd04
           lda #0
           sta $dd05
           jsr waitborder
           lda #%00001001
           sta $dd0e
           lda $dd0d
           ldx $dd0d
nmi
           cmp #$81
           beq ok1
           jsr print
           .byte 13
           .text "cia2 icr is not $81"
           .byte 0
           jsr waitkey
ok1
           cpx #$00
           beq ok2
           jsr print
           .byte 13
           .text "reading icr=81 did "
           .text "not clear icr"
           .byte 0
           jsr waitkey
ok2
           lda nmiadr+1
           bne ok3
           jsr print
           .byte 13
           .text "reading icr=81 must "
           .text "pass nmi"
           .byte 0
           jsr waitkey
ok3
           .bend

;---------------------------------------
;read icr when it is $00 and check if
;nmi follows

           .block
           sei
           lda #0
           sta nmiadr+0
           sta nmiadr+1
           sta $dd0e
           sta $dd0f
           lda #$7f
           sta $dd0d
           lda #$81
           sta $dd0d
           bit $dd0d
           lda #<onnmi
           sta $0318
           lda #>onnmi
           sta $0319
.ifeq NEWCIA - 1
           lda #3 + 1
.else
           lda #3
.endif
           sta $dd04
           lda #0
           sta $dd05
           jsr waitborder
           lda #%00001001
           sta $dd0e
           lda $dd0d
           ldx $dd0d
nmi
           cmp #$00
           beq ok1
           jsr print
           .byte 13
           .text "cia2 icr is not $00"
           .byte 0
           jsr waitkey
ok1
           cpx #$81
           beq ok2
           jsr print
           .byte 13
           .text "reading icr=00 may "
           .text "not clear icr"
           .byte 0
           jsr waitkey
ok2
           lda nmiadr+1
           bne ok3
           jsr print
           .byte 13
           .text "reading icr=00 may "
           .text "not prevent nmi"
           .byte 0
           jsr waitkey
ok3
           .bend

;---------------------------------------

           rts


