; this file is part of the C64 Emulator Test Suite. public domain, no copyright

; original file was: cntdef.asm
;-------------------------------------------------------------------------------

            .include "common.asm"
            .include "waitkey.asm"
            .include "waitborder.asm"

;------------------------------------------------------------------------------
thisname   .null "cntdef"   ; name of this test
nextname   .null "cia1ta"   ; name of next test, "-" means no more tests
;------------------------------------------------------------------------------
main:

;---------------------------------------
;check cnt with counting ab cascaded

         .block
         sei
         lda #0
         sta $dc0e
         sta $dc0f
         lda #$7f
         sta $dc0d
         bit $dc0d
         lda #0
         sta $dc04
         sta $dc05
         lda #%00000001
         sta $dc0e
         lda #255
         sta $dc06
         sta $dc07
         jsr waitborder
         lda #%01100001
         sta $dc0f
         lda $dc06
         cmp $dc06
         bne ok1
         jsr print
         .byte 13
         .text "cnt is not high "
         .text "by default"
         .byte 0
         jsr waitkey
ok1
         .bend

;---------------------------------------

         rts    ; SUCCESS
