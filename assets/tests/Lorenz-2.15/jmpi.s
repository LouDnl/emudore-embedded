; this file is part of the C64 Emulator Test Suite. public domain, no copyright

; original file was: jmpi.asm
;-------------------------------------------------------------------------------

            .include "common.asm"
            .include "printhb.asm"
            .include "showregs.asm"

;------------------------------------------------------------------------------           
thisname   .null "jmpi"      ; name of this test
nextname   .null "beqr"      ; name of next test, "-" means no more tests
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
         tsx
         stx sb

         lda #$13
         sta MEMPAGE1200
         ldx #<cont
         lda #$4c
         sta MEMPAGE1300,x
         lda #<perr
         sta MEMPAGE1300+$01,x
         lda #>perr
         sta MEMPAGE1300+$02,x
         jmp next

perr     cld
         jsr print
         .byte 13
         .text "page is incremented"
         .byte 13,0
         jsr wait
         jmp weiter

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
         ora #%00110000
.ifeq (TARGET - TARGETDTV)
         and #$cf
.endif
         sta pr

         lda sb
         sta sr

         ldx cmd+1
         lda #<cont
         sta MEMPAGE1100,x
         lda #>cont
         inx
         sta MEMPAGE1100,x

         ldx sb
         txs
         lda pb
         pha
         lda ab
         ldx xb
         ldy yb
         plp

cmd      jmp (MEMPAGE1100)

cont     php
         cld
         sta aa
         stx xa
         sty ya
         pla
         sta pa
         tsx
         stx sa
         jsr check

weiter   inc cmd+1
         inc pb
         beq nonext
jmpnext  jmp next
nonext

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
