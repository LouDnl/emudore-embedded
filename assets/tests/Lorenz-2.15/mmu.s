; this file is part of the C64 Emulator Test Suite. public domain, no copyright

; original file was: mmu.asm
;-------------------------------------------------------------------------------

            .include "common.asm"
            .include "printhb.asm"
            .include "showregs.asm"

;------------------------------------------------------------------------------           
thisname   .null "mmu"      ; name of this test
nextname   .null "cpuport"  ; name of next test, "-" means no more tests
;------------------------------------------------------------------------------ 


pconfig  = 172  ;173
data     .byte 0,0,0,0
abackup  .byte 0,0

table    .byte $01,$01,$01,$02
         .byte $01,$01,$3d,$02
         .byte $95,$01,$3d,$02
         .byte $95,$86,$3d,$02
         .byte $01,$01,$01,$02
         .byte $01,$01,$00,$03
         .byte $95,$01,$00,$03
         .byte $95,$86,$00,$03


rom
         lda #$2f
         sta 0
         lda #$37
         sta 1
         cli
         rts


prepare
         jsr rom
         sei
         lda #$02
         sta $d000
         lda #$34
         sta 1
         lda #$00
         sta $a000
         sta $e000
         sta $d000
         rts


compare
         .block
         lda 0
         sta new0
         lda 1
         sta new1
         inc $a000
         inc $e000
         inc $d000
         lda #$2f
         sta 0
         lda #$30
         sta 1
         lda $a000
         ldy #0
         cmp (pconfig),y
         bne error
         lda $e000
         iny
         cmp (pconfig),y
         bne error
         lda $d000
         iny
         cmp (pconfig),y
         bne error
         lda #$37
         sta 1
         lda $d000
         iny
         cmp (pconfig),y
         bne error
         cli
         rts
new0     .byte 0
new1     .byte 0
error
         lda #$37
         sta 1
         lda $d000
         pha
         lda #$30
         sta 1
         lda $d000
         pha
         lda $e000
         pha
         lda $a000
         pha
         jsr rom
         jsr print
         .byte 13
         .null "0=0 1=0"
         ldy #0
printconf
         lda #32
         jsr $ffd2
         lda data,y
         and #$01
         ora #"0"
         jsr $ffd2
         lda #"="
         jsr $ffd2
         lda data,y
         lsr a
         ora #"0"
         jsr $ffd2
         iny
         cpy #4
         bcc printconf
         jsr print
         .byte 13
         .text "after  "
         .byte 0
         lda new0
         and #$07
         ora #"0"
         jsr $ffd2
         lda #32
         jsr $ffd2
         lda new1
         and #$07
         ora #"0"
         jsr $ffd2
         lda #32
         jsr $ffd2
         jsr $ffd2
         pla
         jsr printhb
         lda #32
         jsr $ffd2
         pla
         jsr printhb
         lda #32
         jsr $ffd2
         pla
         jsr printhb
         lda #32
         jsr $ffd2
         pla
         jsr printhb
         jsr print
         .byte 13
         .text "right  "
         .byte 0
         lda abackup+0
         and #$07
         ora #"0"
         jsr $ffd2
         lda #32
         jsr $ffd2
         lda abackup+0
         eor #$ff
         ora abackup+1
         and #$07
         ora #"0"
         jsr $ffd2
         lda #32
         jsr $ffd2
         jsr $ffd2
         ldy #0
         lda (pconfig),y
         jsr printhb
         lda #32
         jsr $ffd2
         ldy #1
         lda (pconfig),y
         jsr printhb
         lda #32
         jsr $ffd2
         ldy #2
         lda (pconfig),y
         jsr printhb
         lda #32
         jsr $ffd2
         ldy #3
         lda (pconfig),y
         jsr printhb
         lda #13
         jsr $ffd2

         #SET_EXIT_CODE_FAILURE

wait     jsr $ffe4
         beq wait
         rts
         .bend


main:
         lda #0
         sta data+0
         sta data+1
         sta data+2
         sta data+3

nextconfig

         jsr prepare

         ldy #0
         sty 0
         sty 1
         sty abackup+0
         sty abackup+1
store
         lda data,y
         and #1
         tax
         lda data,y
         sta 0,x
         sta abackup,x
         iny
         cpy #4
         bcc store

         lda abackup+0
         eor #$ff
         ora abackup+1
         and #$07
         asl a
         asl a
         adc #<table
         sta pconfig+0
         lda #0
         adc #>table
         sta pconfig+1
         jsr compare

         ldx data+0
         inx
         txa
         and #$0f
         sta data+0
         bne nextconfig
         ldx data+1
         inx
         txa
         and #$0f
         sta data+1
         bne nextconfig
         ldx data+2
         inx
         txa
         and #$0f
         sta data+2
         bne nextconfig
         ldx data+3
         inx
         txa
         and #$0f
         sta data+3
         bne nextconfig


ok
        rts         ; success
