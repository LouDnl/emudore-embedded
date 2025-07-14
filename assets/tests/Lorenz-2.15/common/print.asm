; this file is part of the C64 Emulator Test Suite. public domain, no copyright

;print text which immediately follows
;the JSR and return to address after 0
; return addr on stack == ptr to string
print      
            .block
            pla
            sta print0+1
            pla
            sta print0+2

            ldx #1
print0      lda $dead,x
            beq print1
            jsr cbmk_bsout
            inx
            bne print0
print1     
            sec
            txa
            adc print0+1
            sta print2+1
            lda #0
            adc print0+2
            sta print2+2
print2      jmp $dead
            .bend

