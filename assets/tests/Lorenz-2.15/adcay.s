; this file is part of the C64 Emulator Test Suite. public domain, no copyright

; original file was: adcay.asm
;-------------------------------------------------------------------------------

            .include "common.asm"
            .include "printhb.asm"
            .include "showregs.asm"
           
;------------------------------------------------------------------------------           
thisname   .null "adcay"      ; name of this test
nextname   .null "adcix"      ; name of next test, "-" means no more tests
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

         lda #0
         sta db
         sta ab
         sta yb

next     lda pb
         and #%00001000
         bne decmode
         lda db
         sta da
         sta dr
         sta cmd0+1
         and #$7f
         sta cmd1+1
         clc
         lda pb
         and #1
         beq noc
         sec
noc      php
         lda ab
cmd0     adc #0
         sta ar
         lda pb
         ora #%00110000
         and #%00111100
         bcc noc1
         ora #1
noc1     tax
         lda ab
         and #$7f
         plp
cmd1     adc #0
         bmi neg
         txa
         and #1
         beq cont
set      txa
         ora #%01000000
         tax
         jmp cont
neg      txa
         and #1
         beq set
cont     lda ar
         cmp #0
         bne nozero
         txa
         ora #%00000010
         tax
nozero   lda ar
         bpl noneg
         txa
         ora #%10000000
         tax
.ifne (TARGET - TARGETDTV)
noneg    stx pr
.else
noneg    txa
         and #$cf
         sta pr
.endif
         jmp deccont

decmode
         .block
         lda db
         sta da
         sta dr
         and #$0f
         sta l0+1
         lda pb
         ora #%00110000
         and #%00111100
         tax
         lda pb
         lsr a
         lda ab
         and #$0f
l0       adc #0
         ldy #$00
         cmp #$0a
         bcc l1
         sec
         sbc #$0a
         and #$0f
         ldy #$08
l1       sta ar
         sty l2+1
         sty l3+1
         lda db
         and #$f0
         ora l3+1
         sta l3+1
         lda ab
         and #$f0
l2       ora #0
         clc
l3       adc #0
         php
         bcs l4
         cmp #$a0
         bcc l5
l4       sec
         sbc #$a0
         inx
l5       ora ar
         sta ar
         plp
         bvc nov
         php
         txa
         ora #%01000000
         tax
         plp
nov      bpl non
         txa
         ora #%10000000
         tax
non      lda pb
         lsr a
         lda ab
         adc db
         bne noz
         txa
         ora #%00000010
         tax
.ifne (TARGET - TARGETDTV)
noz      stx pr
.else
noz      txa
         and #$cf
         sta pr
.endif
         .bend

deccont  lda xb
         sta xr

         lda yb
         sta yr

         lda sb
         sta sr

         ldx sb
         txs
         lda pb
         pha
         lda ab
         ldx xb
         ldy yb
         plp

cmd      adc da,y

         php
         cld
         sta aa
         stx xa
         sty ya
         pla
         sta pa
         tsx
         stx sa
         jsr check

         inc cmd+1
         bne noinc
         inc cmd+2
noinc    lda yb
         bne nodec
         dec cmd+2
nodec    dec yb

         clc
         lda db
         adc #17
         sta db
         bcc jmpnext
         lda #0
         sta db
         clc
         lda ab
         adc #17
         sta ab
         bcc jmpnext
         lda #0
         sta ab
         inc pb
         beq nonext
jmpnext  jmp next
nonext

        rts ; SUCCESS

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

check
         lda da
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
