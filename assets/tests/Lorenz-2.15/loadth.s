; this file is part of the C64 Emulator Test Suite. public domain, no copyright

; original file was: loadth.asm
;-------------------------------------------------------------------------------

            .include "common.asm"
            .include "waitkey.asm"

;------------------------------------------------------------------------------
thisname   .null "loadth"    ; name of this test
nextname   .null "cnto2"     ; name of next test, "-" means no more tests
;------------------------------------------------------------------------------
main:

;---------------------------------------
;check force load

         .block
         sei
         lda #0
         sta $dc04
         sta $dc05
         sta $dc06
         sta $dc07
         sta $dd04
         sta $dd05
         sta $dd06
         sta $dd07
         lda #%00010000
         sta $dc0e
         sta $dc0f
         sta $dd0e
         sta $dd0f
         lda $dc04
         ora $dc05
         ora $dc06
         ora $dc07
         ora $dd04
         ora $dd05
         ora $dd06
         ora $dd07
         beq ok1
         jsr print
         .byte 13
         .text "force load does "
         .text "not work"
         .byte 0
         jsr waitkey
ok1
         .bend

;---------------------------------------
;write tl while timers are stopped

         .block
         sei
         lda #255
         sta $dc04
         sta $dc06
         sta $dd04
         sta $dd06
         lda $dc04
         ora $dc05
         ora $dc06
         ora $dc07
         ora $dd04
         ora $dd05
         ora $dd06
         ora $dd07
         beq ok1
         jsr print
         .byte 13
         .text "writing tl may not "
         .text "load counter"
         .byte 0
         jsr waitkey
ok1
         .bend

;---------------------------------------
;write th while timers are stopped

         .block
         sei
         lda #255
         sta $dc05
         sta $dc07
         sta $dd05
         sta $dd07
         lda $dc04
         and $dc05
         and $dc06
         and $dc07
         and $dd04
         and $dd05
         and $dd06
         and $dd07
         cmp #255
         beq ok1
         jsr print
         .byte 13
         .text "writing th while "
         .text "stopped didn't load"
         .byte 0
         jsr waitkey
ok1
         .bend

;---------------------------------------
;write th while timers are running

         .block
         sei
         lda #%00100001
         sta $dc0e
         sta $dc0f
         sta $dd0e
         sta $dd0f
         lda #0
         sta $dc05
         sta $dc07
         sta $dd05
         sta $dd07
         lda $dc04
         and $dc05
         and $dc06
         and $dc07
         and $dd04
         and $dd05
         and $dd06
         and $dd07
         cmp #255
         beq ok1
         jsr print
         .byte 13
         .text "writing th while "
         .text "started may not load"
         .byte 0
         jsr waitkey
ok1
         .bend

;---------------------------------------

         rts        ; success

