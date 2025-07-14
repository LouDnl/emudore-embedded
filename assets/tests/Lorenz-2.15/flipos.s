; this file is part of the C64 Emulator Test Suite. public domain, no copyright

; original file was: flipos.asm
;-------------------------------------------------------------------------------

            .include "common.asm"
            .include "waitkey.asm"
            .include "waitborder.asm"

;------------------------------------------------------------------------------
thisname   .null "flipos"   ; name of this test
nextname   .null "oneshot"   ; name of next test, "-" means no more tests
;------------------------------------------------------------------------------
main:

;set oneshot at underflow-1

         .block
         sei
         lda #0
         sta $dc0e
         sta $dc0f
         lda #$7f
         sta $dc0d
         bit $dc0d
         lda #3
         sta $dc04
         lda #0
         sta $dc05
         lda #%00100001
         sta $dc0e
         lda #255
         sta $dc04
         sta $dc05
         lda #%00000000
         sta $dc0e
         jsr waitborder
         lda #%00000001
         ldx #%00001001
         sta $dc0e
         stx $dc0e
         lda $dc04
         and $dc05
         cmp #255
         beq ok1
         jsr print
         .byte 13
         .text "set oneshot at t-1 "
         .text "did not stop counter"
         .byte 0
         jsr waitkey
ok1
         .bend

;---------------------------------------
;set oneshot at underflow

         .block
         sei
         lda #0
         sta $dc0e
         sta $dc0f
         lda #$7f
         sta $dc0d
         bit $dc0d
         lda #2
         sta $dc04
         lda #0
         sta $dc05
         lda #%00100001
         sta $dc0e
         lda #255
         sta $dc04
         sta $dc05
         lda #%00000000
         sta $dc0e
         jsr waitborder
         lda #%00000001
         ldx #%00001001
         sta $dc0e
         stx $dc0e
         lda $dc04
         and $dc05
         sta 16384
         cmp #252
         beq ok1
         jsr print
         .byte 13
         .text "set oneshot at t "
         .text "may not stop counter"
         .byte 0
         jsr waitkey
ok1
         .bend

;---------------------------------------
;clear oneshot at underflow-1

         .block
         sei
         lda #0
         sta $dc0e
         sta $dc0f
         lda #$7f
         sta $dc0d
         bit $dc0d
         lda #3
         sta $dc04
         lda #0
         sta $dc05
         lda #%00100001
         sta $dc0e
         lda #255
         sta $dc04
         sta $dc05
         lda #%00000000
         sta $dc0e
         jsr waitborder
         lda #%00001001
         ldx #%00000001
         sta $dc0e
         stx $dc0e
         lda $dc04
         and $dc05
         cmp #255
         beq ok1
         jsr print
         .byte 13
         .text "clr oneshot at t-1 "
         .text "did not stop counter"
         .byte 0
         jsr waitkey
ok1
         .bend

;---------------------------------------
;clear oneshot at underflow-2

         .block
         sei
         lda #0
         sta $dc0e
         sta $dc0f
         lda #$7f
         sta $dc0d
         bit $dc0d
         lda #4
         sta $dc04
         lda #0
         sta $dc05
         lda #%00100001
         sta $dc0e
         lda #255
         sta $dc04
         sta $dc05
         lda #%00000000
         sta $dc0e
         jsr waitborder
         lda #%00001001
         ldx #%00000001
         sta $dc0e
         stx $dc0e
         lda $dc04
         and $dc05
         cmp #254
         beq ok1
         jsr print
         .byte 13
         .text "clr oneshot at t-2 "
         .text "may not stop counter"
         .byte 0
         jsr waitkey
ok1
         .bend

;---------------------------------------

         rts        ; SUCCESS
