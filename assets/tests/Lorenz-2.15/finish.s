; this file is part of the C64 Emulator Test Suite. public domain, no copyright

; original file was: n/a
;------------------------------------------------------------------------------

            .include "common.asm"
           
;------------------------------------------------------------------------------           
thisname   .null "finish"       ; name of this test
nextname   .null "-"            ; name of next test, "-" means no more tests
;------------------------------------------------------------------------------           

main:
            jsr print
            .byte 13
            .text "test suite 2.15+ completed"
            .byte 0

            rts
