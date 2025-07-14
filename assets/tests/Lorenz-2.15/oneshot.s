; this file is part of the C64 Emulator Test Suite. public domain, no copyright

; original file was: oneshot.asm
;-------------------------------------------------------------------------------

            .include "common.asm"
            .include "waitborder.asm"
            .include "waitkey.asm"
           
;------------------------------------------------------------------------------           
thisname    .null "oneshot" ; name of this test
nextname    .null "cntdef"  ; name of next test, "-" means no more tests
;-------------------------------------------------------------------------------           
main:

;---------------------------------------
;read cra when icr is $01 and check if
;start has been cleared

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
         lda #2
         sta $dc04
         lda #0
         sta $dc05
         jsr waitborder
         lda #%00001001
         sta $dc0e
         lda $dc0e
         cmp #%00001000
         beq ok1
         jsr print
         .byte 13
         .text "cra is not $08 at "
         .text "icr=$01"
         .byte 0
         jsr waitkey
ok1
         .bend

;---------------------------------------
;read cra when icr is $00 and check if
;start has been cleared

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
         lda #3
         sta $dc04
         lda #0
         sta $dc05
         jsr waitborder
         lda #%00001001
         sta $dc0e
         lda $dc0e
         cmp #%00001001
         beq ok1
         jsr print
         .byte 13
         .text "cra is not $09 at "
         .text "icr=$00"
         .byte 0
         jsr waitkey
ok1
         .bend

         rts
