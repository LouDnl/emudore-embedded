; this file is part of the C64 Emulator Test Suite. public domain, no copyright

; original file was: cnto2.asm
;-------------------------------------------------------------------------------

            .include "common.asm"
            .include "waitkey.asm"
            .include "waitborder.asm"

;------------------------------------------------------------------------------
thisname   .null "cnto2"     ; name of this test
nextname   .null "icr01"     ; name of next test, "-" means no more tests
;------------------------------------------------------------------------------
main:

;---------------------------------------
;switch from cnt to o2 and back

         .block
         sei
         lda #0
         sta $dc0e
         sta $dc0f
         lda #$7f
         sta $dc0d
         bit $dc0d
         lda #255
         sta $dc04
         sta $dc05
         lda #%00100001
         sta $dc0e
         jsr waitborder
         lda #%00000001
         ldx #%00100001
         sta $dc0e
         lda $dc04
         stx $dc0e
         ldx $dc04
         cmp #253
         beq ok1
         jsr print
         .byte 13
         .text "cnt to o2 does not "
         .text "delay 2 clocks"
         .byte 0
         jsr waitkey
ok1
         cpx #247
         beq ok2
         jsr print
         .byte 13
         .text "o2 to cnt does not "
         .text "delay 2 clocks"
         .byte 0
         jsr waitkey
ok2
         .bend

;---------------------------------------

         rts    ; success

