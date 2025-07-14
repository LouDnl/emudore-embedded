; this file is part of the C64 Emulator Test Suite. public domain, no copyright

;wait for a key after failure
waitkey:
            #SET_EXIT_CODE_FAILURE

            .block
            #RESET_KERNAL_IO
            cli

wait        jsr cbmk_getin
            beq wait
            cmp #3
            beq stop
            rts
stop
            jsr print
            .byte 13
            .text "break"
            .byte 13,0
            jmp loadnext
            ;jmp $a474
            
;load
;            jsr print
;            .byte 13
;            .text "skip"
;            .byte 13,0
;            jmp loadnext
            .bend
