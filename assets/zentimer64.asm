;ZEN-TIMER 64, 6502tass v1.31 version. Original idea by M. Abrash. Usage:

;  jsr measure    or <start address>+0 if precompiled   to start cycle counting
;  jsr evaluate   or <start address>+3 if precompiled   to stop counting & print result

;Note: max cycle count range is limited to about 65.500 cycles (=roughly 3 frames)


overhead        = 19            ;cycles wasted by the timer itself during measurement
irqs_allowed    = 0             ;1 to allow them (less accurate results)
dma_off         = 0;1             ;0 to allow badlines (dito)
sprites_off     = 0;1             ;0 to allow sprites (dito)
printout        = $400          ;0 to use $bdcd, <address> to write directly to screen
                                ;(or some other location to look it up via ml-mon)


;* = $1000      ;uncomment to precompile to wanted address

;                jsr initdata   ;prepare test case for your sorting algo 
                jsr measure    ;start cycle counting
;                jsr sortalgo   
                jsr evaluate   ;stop count & print out cycle count

;                jmp measure

evaluate        sei
                lda #0
                sta $dc0f
                lda vald011
                sta $d011
                lda vald015
                sta $d015
                cld
                sec
                lda #<($ffff-overhead)
                sbc $dc06
                sta locycles
                lda #>($ffff-overhead)
                sbc $dc07

.if !printout
                ldx locycles
                jsr $bdcd
                lda #13
                jsr $ffd2
                lda statusreg   ;restore (most of) st
                pha
                plp
                rts
.else

                ldy locycles    ;lame hex to petscii conversion
                ldx #$30-1
                stx ten1000s
                stx ten1000s+1
                stx ten1000s+2
                stx ten1000s+3
                stx ten1000s+4

                sec
hploop          sta temp
                inc ten1000s-$30+1,x
                tya
                sbc lo,x
                tay
                lda temp
                sbc hi,x
                bcs hploop

                tya
                adc lo,x
                tay
                inx
                cpx #$34
                sec
                bne hploop+3

                ldx #4
print           lda ten1000s,x
                sta printout,x
                lda $d021
                eor #8
                sta (printout%$400)+$d800,x
                dex
                bpl print

                lda statusreg   ;restore (most of) st
                pha
                plp
                rts

temp            .byte 0           ;needed for hb
ten1000s        .byte 0,0,0,0,0
lo = *-$30+1
.byte <10000,<1000,<100,<10,<1
hi = *-$30+1
.byte >10000,>1000,>100,>10,>1

.fi

locycles        .byte 0
vald015         .byte 0
vald011         .byte 0
statusreg       .byte 0

measure         php             ;save st, just in case
                sei
                pla
                sta statusreg
                lda $d011
                sta vald011
                lda $d015
                sta vald015
                ldx #$00
                stx $dc0f       ;stop timer b (not really necessary, but still)
.if dma_off
                stx $d011
.fi
.if sprites_off
                stx $d015
.fi

                dex
                cpx $d012
                bne *-3         ;wait for vblank area
                stx $dc06       ;set to $ffff
                stx $dc07
                lda #$19

.if irqs_allowed
                cli
.fi

                sta $dc0f       ;start timer b, one shot mode
                rts