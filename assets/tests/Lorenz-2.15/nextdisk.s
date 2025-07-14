; this file is part of the C64 Emulator Test Suite. public domain, no copyright

; original source file: n/a
;-------------------------------------------------------------------------------

            .include "common.asm"

;------------------------------------------------------------------------------           
thisname   .null "nextdisk"     ; name of this test
nextname   ; name of next test, "-" means no more tests
.ifeq NEXT - 1
        .null "beqr"
.endif
.ifeq NEXT - 2
        .null "irq"
.endif
;-------------------------------------------------------------------------------           

main:
            jsr print
            .byte $0d
            .text "please insert disk "
            .byte $31 + NEXT
            .byte $20
            .byte 0

wait
            jsr $ffe4
            beq wait

            rts
