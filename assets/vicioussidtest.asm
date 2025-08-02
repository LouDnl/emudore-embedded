*=0801
IRQ1:
STA $FE
STY $FF

LDA #$00 ;step 5(voice 3)
STA $D40F
LDA #$11 ;step 6(voice 3)
STA $D412

LDA #$09 ;step 1(voice 2)
STA $D40B
LDY #$00 ;step 2(voice 2)
LDA ($10),Y
STA $D408
LDA #$00 ;step 3(voice 2)
STA $D40B

;step 4 until next irq (voice 2)

INC $10
BNE *+4
INC $11
LDA $DC0D

LDA #<IRQ2
STA $FFFE
LDA #>IRQ2
STA $FFFF

LDA $FE
LDY $FF
RTI

IRQ2:
STA $FE
STY $FF

LDA #$00 ;step 5(voice 2)
STA $D408
LDA #$11 ;step 6(voice 2)
STA $D40B

LDA #$09 ;step 1(voice 3)
STA $D412
LDY #$00 ;step 2(voice 3)
LDA ($10),Y
STA $D40F
LDA #$00 ;step 3(voice 3)
STA $D412

;step 4 until next irq (voice 3)

INC $10
BNE *+4
INC $11
LDA $DC0D

LDA #<IRQ1
STA $FFFE
LDA #>IRQ1
STA $FFFF

LDA $FE
LDY $FF
RTI
