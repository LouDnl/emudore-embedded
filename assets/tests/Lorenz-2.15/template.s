; this file is part of the C64 Emulator Test Suite. public domain, no copyright

; original file was: template.asm
;------------------------------------------------------------------------------

            .include "common.asm"
            ;.include "printhb.asm"
            ;.include "waitborder.asm"
            ;.include "waitkey.asm"
            ;.include "showregs.asm"
           
;------------------------------------------------------------------------------           
thisname   .null "template"     ; name of this test
nextname   .null "-"            ; name of next test, "-" means no more tests
;------------------------------------------------------------------------------           

main:

            .block


        ; insert your test code here


            .bend

            rts
