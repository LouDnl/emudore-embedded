*=$c000
!to "interrupts.prg",cbm
!cpu 6510
type = 64

*= $0810

  sei
  lda #%01010010
  sta $dc0f

  lda #$fc
  sta $dc04
  lda #$03
  sta $dc05

  lda #$ff
  sta $dc06
  sta $dc07

  lda #<start
  sta $0314
  lda #>start
  sta $0315

  lda #%00010001
  sta $dc0e

  cli
  jmp main


start:
  ; jsr $e554 ; clears screen except commodore!?
  jsr $e544 ; clear screen
  ldx #$00
helloworld:
  lda text,x
  sta $0400,x ; row 0
  beq ciatimers
  inx
  jmp helloworld

ciatimers:
  ldx #$00
textloop2:
  lda timertext,x
  sta $0428,x ; row 1
  beq timer
  inx
  jmp textloop2

; tod:

timer:
  ldx #0
  ldy #9
loop:
  lda #1
  sta $d800,x
  lda $dc04,x
  sta $0428,y ; row 1
  clc
  inx
  iny
  cpx #2
  bne loop

  ldy #21
loop2:
  lda #1
  sta $d800,x
  lda $dc04,x
  !pet "",a
  sta $0428,y ; row 1
  clc
  inx
  iny
  cpx #4
  bne loop2

  ldy #0 ; start with 0 in x
loop3:
  lda #1 ; load accu with 1
  sta $d800,y ; accu to $d800 + y
  lda $dc08,y ; load accu with $dc08 + y
  ; sta $0400,x ; store accu in screen memory
  sta $0450,y ; store accu in screen memory on row 2
  clc ; clear carry
  ; inx ; increase x
  iny ; inxrease y
  cpy #4 ; compare y with #4
  bne loop3 ; got to loop2 if not equal to 4

  lda #%01111111
  sta $dc0d
  lda $dc0d
  jmp $ea31

main:
  lda #1
  sta $d800

; loop:
  jmp timer
  rts

text:
		!scr "              hello world!              "		;40 cols of text
  ; !text "Hello World!"

timertext:
    !scr "timer a: 00 timer b: 00",0
todtext:
    !scr "timeofday 00:00:00:00",0
