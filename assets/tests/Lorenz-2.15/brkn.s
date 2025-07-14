; this file is part of the C64 Emulator Test Suite. public domain, no copyright

; original file was: brkn.asm
;-------------------------------------------------------------------------------

            .include "common.asm"
            .include "printhb.asm"
            .include "showregs.asm"

;------------------------------------------------------------------------------           
thisname   .null "brkn"      ; name of this test
nextname   .null "rtin"      ; name of next test, "-" means no more tests
;------------------------------------------------------------------------------ 
main:
         lda #%00011011
         sta db
         lda #%11000110
         sta ab
         lda #%10110001
         sta xb
         lda #%01101100
         sta yb
         lda #0
         sta pb
         ldx #4
s2       dex
         sta $200,x
         bne s2
         tsx
         stx sb

         tsx
         stx saves+1
         ldx #0
save     lda $0100,x
         sta MEMPAGE1000,x
         inx
         bne save

         lda #0
fill     sta MEMPAGE1100,x
         inx
         bne fill

         lda #<break
         sta $fffe
         lda #>break
         sta $ffff

next     lda db
         sta da
         sta dr

         lda ab
         sta ar

         lda xb
         sta xr

         lda yb
         sta yr

         lda pb
         ora #%00110100
.ifeq (TARGET - TARGETDTV)
         and #$cf
.endif
         sta pr

         lda sb
         sec
         sbc #3
         sta sr

         lda #127
         sta $dc0d
         lda $dc0d
         lda #$30
         sta 1

         ldx sb
         txs
         lda pb
         pha
         lda ab
         ldx xb
         ldy yb
         plp

cmd      jmp MEMPAGE1100

break    php
         cld
         sta aa
         stx xa
         sty ya
         pla
         sta pa
         tsx
         stx sa

         lda #$37
         sta 1
         lda #129
         sta $dc0d

         jsr check

         tsx
         lda pb
         ora #%00110000
         inx
         cmp $0100,x
         bne ste
         clc
         lda cmd+1
         ldy cmd+2
         adc #2
         bcc noinc
         iny
noinc    inx
         cmp $0100,x
         bne ste
         inx
         tya
         cmp $0100,x
         beq stok
ste      jsr print
         .byte 13
         .text "wrong data on stack"
         .byte 13,0
         jsr wait
stok

         inc cmd+1
         dec sb
         inc pb
         beq nonext
jmpnext  jmp next
nonext

saves    ldx #0
         txs
         ldx #0
restore  lda MEMPAGE1000,x
         sta $0100,x
         inx
         bne restore

         rts ; success

db       .byte 0
ab       .byte 0
xb       .byte 0
yb       .byte 0
pb       .byte 0
sb       .byte 0
da       .byte 0
aa       .byte 0
xa       .byte 0
ya       .byte 0
pa       .byte 0
sa       .byte 0
dr       .byte 0
ar       .byte 0
xr       .byte 0
yr       .byte 0
pr       .byte 0
sr       .byte 0

check    lda da
         cmp dr
         bne error
         lda aa
         cmp ar
         bne error
         lda xa
         cmp xr
         bne error
         lda ya
         cmp yr
         bne error
         lda pa
.ifeq (TARGET - TARGETDTV)
         and #$cf
.endif
         cmp pr
         bne error
         lda sa
         cmp sr
         bne error
         rts

error    jsr print
         .byte 13
         .null "before  "
         ldx #<db
         ldy #>db
         jsr showregs
         jsr print
         .byte 13
         .null "after   "
         ldx #<da
         ldy #>da
         jsr showregs
         jsr print
         .byte 13
         .null "right   "
         ldx #<dr
         ldy #>dr
         jsr showregs
         lda #13
         jsr $ffd2

         #SET_EXIT_CODE_FAILURE

wait     jsr $ffe4
         beq wait
         rts
