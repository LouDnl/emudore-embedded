; this file is part of the C64 Emulator Test Suite. public domain, no copyright

        ; x/y - pointer to register dump
showregs 
        .block
         stx 172
         sty 173
         ldy #0
         lda (172),y
         jsr printhb
         lda #32
         jsr cbmk_bsout
         lda #32
         jsr cbmk_bsout
         iny
         lda (172),y
         jsr printhb
         lda #32
         jsr cbmk_bsout
         iny
         lda (172),y
         jsr printhb
         lda #32
         jsr cbmk_bsout
         iny
         lda (172),y
         jsr printhb
         lda #32
         jsr cbmk_bsout
         iny
         lda (172),y
         ldx #"n"
         asl a
         bcc ok7
         ldx #"N"
ok7      pha
         txa
         jsr cbmk_bsout
         pla
         ldx #"v"
         asl a
         bcc ok6
         ldx #"V"
ok6      pha
         txa
         jsr cbmk_bsout
         pla
         ldx #"0"
         asl a
         bcc ok5
         ldx #"1"
ok5      pha
         txa
         jsr cbmk_bsout
         pla
         ldx #"b"
         asl a
         bcc ok4
         ldx #"B"
ok4      pha
         txa
         jsr cbmk_bsout
         pla
         ldx #"d"
         asl a
         bcc ok3
         ldx #"D"
ok3      pha
         txa
         jsr cbmk_bsout
         pla
         ldx #"i"
         asl a
         bcc ok2
         ldx #"I"
ok2      pha
         txa
         jsr cbmk_bsout
         pla
         ldx #"z"
         asl a
         bcc ok1
         ldx #"Z"
ok1      pha
         txa
         jsr cbmk_bsout
         pla
         ldx #"c"
         asl a
         bcc ok0
         ldx #"C"
ok0      pha
         txa
         jsr cbmk_bsout
         pla
         lda #32
         jsr cbmk_bsout
         iny
         lda (172),y
        jmp printhb
         .bend
 
