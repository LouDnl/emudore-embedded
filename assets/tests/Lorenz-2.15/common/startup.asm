; this file is part of the C64 Emulator Test Suite. public domain, no copyright

            *= STARTADDR

;-------------------------------------------------------------------------------
        .word nextline
        .word 2016
        .byte $9e
rem .var START
        .byte $30 + (rem / 10000)
rem .var rem - ((rem / 10000) * 10000)
        .byte $30 + (rem / 1000)
rem .var rem - ((rem / 1000) * 1000)
        .byte $30 + (rem / 100)
rem .var rem - ((rem / 100) * 100)
        .byte $30 + (rem / 10)
rem .var rem - ((rem / 10) * 10)
        .byte $30 + (rem / 1)
nextline:
        .byte 0,0,0

;-------------------------------------------------------------------------------
START:
           .block

           #RESET_CURSOR
           #RESET_COLORS

           ldx #0
           lda thisname
printthis
           jsr cbmk_bsout
           inx
           lda thisname,x
           bne printthis

           jsr main

           ; success

           #RESET_MEMORY_MAP
           #RESET_KERNAL_IO

           jsr print
           .text " - ok"
           .byte 13,0

           #SET_EXIT_CODE_SUCCESS

           .bend

        ; entry point used by waitkey
loadnext:
           .block
           ldx #$f8
           txs

           #RESET_MEMORY_MAP

           lda nextname
           cmp #"-"
           bne notempty
           jmp $a474
notempty
           ldx #0
printnext
           jsr cbmk_bsout
           inx
           lda nextname,x
           bne printnext

;            lda #0
;            sta $0a    ; load flag
;            sta $b9    ; secondary address
; 
;            stx $b7  ; namelen
;            lda #<nextname
;            sta $bb  ; namelo
;            lda #>nextname
;            sta $bc  ; namehi
;
;           jmp $e16f

            txa  ; name len
            ldx #<nextname
            ldy #>nextname
            jsr cbmk_setnam

            lda #0   ; file nr.
            ldx #8   ; device nr
            ldy #1   ; secondary addr.
            jsr cbmk_setlfs

            lda #>(START-1)
            pha
            lda #<(START-1)
            pha

            lda #0 ; load flag
            jsr cbmk_load
            bcs loaderror
            rts
loaderror
            jsr print
            .text " - load error!"
            .byte 13,0
            #RESET_KERNAL_IO
            cli
            jmp *
           .bend
