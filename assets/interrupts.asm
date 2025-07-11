*=$0810

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

  lda #<timer
  sta $0314
  lda #>timer
  sta $0315

  lda #%00010001
  sta $dc0e

  cli
  jmp main

timer:
  ; inc $0400
  ldx #0
loop:
  lda #1
  sta $d800,x
  lda $dc04,x
  sta $0400,x
  clc
  inx
  cpx #4
  bne loop

  lda #%01111111
  sta $dc0d
  lda $dc0d
  jmp $ea31

main:
  lda #1
  sta $d800

; loop:
  ; jmp loop
  rts
