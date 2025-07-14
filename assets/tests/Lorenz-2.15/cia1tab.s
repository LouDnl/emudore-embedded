
; this file is part of the C64 Emulator Test Suite. public domain, no copyright

; original file was: cia1tab.asm
;-------------------------------------------------------------------------------

            .include "common.asm"
            .include "printhb.asm"
            .include "waitborder.asm"
            .include "waitkey.asm"
;-------------------------------------------------------------------------------
thisname:
.ifeq NEWCIA - 1
         .text "cia1tab (new cia)"
.else
         .text "cia1tab (old cia)"
.endif
         .byte 0

nextname .null "loadth"
;-------------------------------------------------------------------------------

index    .byte 0
reg      .byte 0
areg     .byte $04,$06,$01,$0d

;-------------------------------------------------------------------------------
main

         ldx #$7e
         lda #$ea   ;nop
makechain
         sta $2000,x
         dex
         bpl makechain
         lda #$60   ;rts
         sta $207f

         sei
         lda #0
         sta write+1
         sta reg
nextreg
         lda #0
         sta index
nextindex
         lda #$ff
         sta $dc03
         lda #$00
         sta $dc01
         sta $dc0e
         sta $dc0f
         lda #$7f
         sta $dc0d
         bit $dc0d
         lda #21
         sta $dc04
         lda #2
         sta $dc06
         ldx #0
         stx $dc05
         stx $dc07
         sta $dc04
         lda #$82
         sta $dc0d
         lda index
         eor #$ff
         lsr a
         php
         sta jump+1
         ldx reg
         lda areg,x
         sta readreg+1
         jsr waitborder
         lda #%01000111
         sta $dc0f
         lda #%00000011
         sta $dc0e
         plp
         bcc jump
jump
         jsr $2011
readreg
         lda $dc11
write
         sta $2111
         inc write+1
         inc index
         lda index
         cmp #12
         bcc nextindex
         inc reg
         lda reg
         cmp #4
         bcc nextreg

;---------------------------------------
;compare result

         jmp compare
right    .byte $01,$02,$02,$01,$02,$02
         .byte $01,$02,$02,$01,$02,$02
         .byte $02,$02,$02,$01,$01,$01
         .byte $00,$00,$02,$02,$02,$02
         .byte $80,$c0,$80,$80,$c0,$80
         .byte $80,$c0,$00,$00,$40,$00
         .byte $00,$01,$01,$01,$01,$01
.ifeq NEWCIA - 1
         .byte $01,$01,$83,$83,$83,$83
.else
         .byte $01,$01,$03,$83,$83,$83
.endif
compare
         jsr $fda3
         sei
         ldx #0
comp
         lda $2100,x
         cmp right,x
         bne diff
         inx
         cpx #12*4
         bcc comp
         jmp ok
diff


;---------------------------------------
;print result

         ldy #0
         jsr print
         .byte 13
         .text "ta "
         .byte 13
         .text "   "
         .byte 0
         jsr print12
         jsr print
         .text "tb "
         .byte 13
         .text "   "
         .byte 0
         jsr print12
         jsr print
         .text "pb "
         .byte 13
         .text "   "
         .byte 0
         jsr print12
         jsr print
         .text "icr"
         .byte 13
         .text "   "
         .byte 0
         jsr print12
         
        #SET_EXIT_CODE_FAILURE 
         
         jsr waitkey
         jmp outend

print12
         ldx #12
loop12
         lda #32
         jsr $ffd2
         lda right,y
         jsr printhb
         dec 211
         dec 211
         dec 211
         lda #145
         jsr $ffd2
         lda #32
         jsr $ffd2
         lda 646
         pha
         lda $2100,y
         cmp right,y
         beq nodiff
         pha
         lda #2
         sta 646
         pla
nodiff
         jsr printhb
         pla
         sta 646
         lda #17
         jsr $ffd2
         iny
         dex
         bne loop12
         lda #13
         jmp $ffd2
outend


;---------------------------------------
;load next part of the test suite

ok

        rts ; SUCCESS
