; this file is part of the C64 Emulator Test Suite. public domain, no copyright

; original file was: cia1tb123.asm
;-------------------------------------------------------------------------------

            .include "common.asm"
            .include "printhb.asm"
            .include "waitborder.asm"
            .include "waitkey.asm"
           
;-------------------------------------------------------------------------------           
thisname:   .null "cia1tb123"
nextname:   .null "cia2tb123"
;-------------------------------------------------------------------------------           

newbrk
         pla
         pla
         pla
         pla
         pla
         pla
         rts

setbrk
         sei
         lda #$00
         sta $dc0e
         bit $dc0b
         sta $dc0b
         sta $dc09
         sta $dc08
         bit $dc0b
         lda #$7f
         sta $dc0d
         bit $dc0d
         lda #<newbrk
         sta $0316
         lda #>newbrk
         sta $0317
         rts

restorebrk
         pha
         lda #$66
         sta $0316
         lda #$fe
         sta $0317
         jsr $fda3
         pla
         cli
         rts

;------------------------------------------------------------------------------
main:

         .block
         jmp start
code
         nop
         sta $dc0f
         asl a
         rts
start
         jsr setbrk
         ldx #0
         stx $dc0f
copy
         lda code,x
         sta $dc02,x
         inx
         cpx #6
         bcc copy
         jsr waitborder
         lda #$01
         jsr $dc02
         jsr restorebrk
         cmp #2
         beq ok
         jsr print
         .byte 13,13
         .text "error 00 01 cycle 1"
         .byte 0
         jsr waitkey
ok
         .bend

         .block
         jmp start
code
         sta $dc0f
         lda #$0a
         rts
start
         jsr setbrk
         ldx #0
         stx $dc0f
copy
         lda code,x
         sta $dc02,x
         inx
         cpx #6
         bcc copy
         jsr waitborder
         lda #$01
         jsr $dc02
         jsr restorebrk
         cmp #$0a
         beq ok
         jsr print
         .byte 13,13
         .text "error 00 01 cycle 2"
         .byte 0
         jsr waitkey
ok
         .bend


         .block
         jmp start
code
         sta $dc0f
         nop
         .byte $0b
         rts
start
         jsr setbrk
         ldx #0
         stx $dc0f
copy
         lda code,x
         sta $dc02,x
         inx
         cpx #6
         bcc copy
         jsr waitborder
         lda #$01
         jsr $dc02
         jsr restorebrk
         cmp #2
         beq ok
         jsr print
         .byte 13,13
         .text "error 00 01 cycle 3"
         .byte 0
         jsr waitkey
ok
         .bend





         .block
         jmp start
code
         nop
         sta $dc0f
         nop
         rts
start
         jsr setbrk
         ldx #0
         stx $dc0f
copy
         lda code,x
         sta $dc02,x
         inx
         cpx #6
         bcc copy
         lda #$0a
         sta $dc06
         jsr waitborder
         lda #$10
         jsr $dc02
         jsr restorebrk
         cmp #$10
         beq ok
         jsr print
         .byte 13,13
         .text "error 00 10 cycle 1"
         .byte 0
         jsr waitkey
ok
         .bend


         .block
         jmp start
code
         sta $dc0f
         lda #$ea
         rts
start
         jsr setbrk
         ldx #0
         stx $dc0f
copy
         lda code,x
         sta $dc02,x
         inx
         cpx #6
         bcc copy
         lda #$0a
         sta $dc06
         jsr waitborder
         lda #$10
         jsr $dc02
         jsr restorebrk
         cmp #$0a
         beq ok
         jsr print
         .byte 13,13
         .text "error 00 10 cycle 2"
         .byte 0
         jsr waitkey
ok
         .bend


         .block
         jmp start
code
         sta $dc0f
         nop
         nop
         rts
start
         jsr setbrk
         ldx #0
         stx $dc0f
copy
         lda code,x
         sta $dc02,x
         inx
         cpx #6
         bcc copy
         lda #$0a
         sta $dc06
         jsr waitborder
         lda #$10
         jsr $dc02
         jsr restorebrk
         cmp #$20
         beq ok
         jsr print
         .byte 13,13
         .text "error 00 10 cycle 3"
         .byte 0
         jsr waitkey
ok
         .bend





         .block
         jmp start
code
         nop
         sta $dc0f
         nop
         rts
start
         jsr setbrk
         ldx #0
         stx $dc0f
copy
         lda code,x
         sta $dc02,x
         inx
         cpx #6
         bcc copy
         lda #$0a
         sta $dc06
         jsr waitborder
         lda #$11
         jsr $dc02
         jsr restorebrk
         cmp #$11
         beq ok
         jsr print
         .byte 13,13
         .text "error 00 11 cycle 1"
         .byte 0
         jsr waitkey
ok
         .bend


         .block
         jmp start
code
         sta $dc0f
         lda #$ea
         rts
start
         jsr setbrk
         ldx #0
         stx $dc0f
copy
         lda code,x
         sta $dc02,x
         inx
         cpx #6
         bcc copy
         lda #$0a
         sta $dc06
         jsr waitborder
         lda #$11
         jsr $dc02
         jsr restorebrk
         cmp #$0a
         beq ok
         jsr print
         .byte 13,13
         .text "error 00 11 cycle 2"
         .byte 0
         jsr waitkey
ok
         .bend


         .block
         jmp start
code
         sta $dc0f
         nop
         nop
         rts
start
         jsr setbrk
         ldx #0
         stx $dc0f
copy
         lda code,x
         sta $dc02,x
         inx
         cpx #6
         bcc copy
         lda #$0a
         sta $dc06
         jsr waitborder
         lda #$11
         jsr $dc02
         jsr restorebrk
         cmp #$22
         beq ok
         jsr print
         .byte 13,13
         .text "error 00 11 cycle 3"
         .byte 0
         jsr waitkey
ok
         .bend





         .block
         jmp start
code
         nop
         stx $dc0f
         .byte $15
         rts
start
         jsr setbrk
         ldx #0
         stx $dc0f
copy
         lda code,x
         sta $dc02,x
         inx
         cpx #6
         bcc copy
         jsr waitborder
         lda #$01
         ldx #$11
         sta $dc0f
         jsr $dc02
         jsr restorebrk
         cmp #$02
         beq ok
         jsr print
         .byte 13,13
         .text "error 01 11 cycle 1"
         .byte 0
         jsr waitkey
ok
         .bend


         .block
         jmp start
code
         stx $dc0f
         lda #$0a
         rts
start
         jsr setbrk
         ldx #0
         stx $dc0f
copy
         lda code,x
         sta $dc02,x
         inx
         cpx #6
         bcc copy
         jsr waitborder
         lda #$01
         ldx #$11
         sta $dc0f
         jsr $dc02
         jsr restorebrk
         cmp #$0a
         beq ok
         jsr print
         .byte 13,13
         .text "error 01 11 cycle 2"
         .byte 0
         jsr waitkey
ok
         .bend


         .block
         jmp start
code
         stx $dc0f
         nop
         .byte $0a
         rts
start
         jsr setbrk
         ldx #0
         stx $dc0f
copy
         lda code,x
         sta $dc02,x
         inx
         cpx #6
         bcc copy
         jsr waitborder
         lda #$01
         ldx #$11
         sta $dc0f
         jsr $dc02
         jsr restorebrk
         cmp #$02
         beq ok
         jsr print
         .byte 13,13
         .text "error 01 11 cycle 3"
         .byte 0
         jsr waitkey
ok
         .bend





         .block
         jmp start
code
         nop
         stx $dc0f
         .byte $15
         rts
start
         jsr setbrk
         ldx #0
         stx $dc0f
copy
         lda code,x
         sta $dc02,x
         inx
         cpx #6
         bcc copy
         jsr waitborder
         lda #$01
         ldx #$10
         sta $dc0f
         jsr $dc02
         jsr restorebrk
         cmp #$02
         beq ok
         jsr print
         .byte 13,13
         .text "error 01 10 cycle 1"
         .byte 0
         jsr waitkey
ok
         .bend


         .block
         jmp start
code
         stx $dc0f
         lda #$0a
         rts
start
         jsr setbrk
         ldx #0
         stx $dc0f
copy
         lda code,x
         sta $dc02,x
         inx
         cpx #6
         bcc copy
         jsr waitborder
         lda #$01
         ldx #$10
         sta $dc0f
         jsr $dc02
         jsr restorebrk
         cmp #$0a
         beq ok
         jsr print
         .byte 13,13
         .text "error 01 10 cycle 2"
         .byte 0
         jsr waitkey
ok
         .bend


         .block
         jmp start
code
         stx $dc0f
         nop
         asl a
         rts
start
         jsr setbrk
         ldx #0
         stx $dc0f
copy
         lda code,x
         sta $dc02,x
         inx
         cpx #6
         bcc copy
         jsr waitborder
         lda #$01
         ldx #$10
         sta $dc0f
         jsr $dc02
         jsr restorebrk
         cmp #$02
         beq ok
         jsr print
         .byte 13,13
         .text "error 01 10 cycle 3"
         .byte 0
         jsr waitkey
ok
         .bend





         .block
         jmp start
code
         nop
         stx $dc0f
         .byte $15
         rts
start
         jsr setbrk
         ldx #0
         stx $dc0f
copy
         lda code,x
         sta $dc02,x
         inx
         cpx #6
         bcc copy
         jsr waitborder
         lda #$01
         ldx #$00
         sta $dc0f
         jsr $dc02
         jsr restorebrk
         cmp #$02
         beq ok
         jsr print
         .byte 13,13
         .text "error 01 00 cycle 1"
         .byte 0
         jsr waitkey
ok
         .bend


         .block
         jmp start
code
         stx $dc0f
         lda #$0a
         rts
start
         jsr setbrk
         ldx #0
         stx $dc0f
copy
         lda code,x
         sta $dc02,x
         inx
         cpx #6
         bcc copy
         jsr waitborder
         lda #$01
         ldx #$00
         sta $dc0f
         jsr $dc02
         jsr restorebrk
         cmp #$00
         beq ok
         jsr print
         .byte 13,13
         .text "error 01 00 cycle 2"
         .byte 0
         jsr waitkey
ok
         .bend


         .block
         jmp start
code
         stx $dc0f
         nop
         .byte $14
         rts
start
         jsr setbrk
         ldx #0
         stx $dc0f
copy
         lda code,x
         sta $dc02,x
         inx
         cpx #6
         bcc copy
         jsr waitborder
         lda #$01
         ldx #$00
         sta $dc0f
         jsr $dc02
         jsr restorebrk
         cmp #$02
         beq ok
         jsr print
         .byte 13,13
         .text "error 01 00 cycle 3"
         .byte 0
         jsr waitkey
ok
         .bend


        rts ; SUCCESS

