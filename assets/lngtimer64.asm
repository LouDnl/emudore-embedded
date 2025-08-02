;LNG-TIMER 64, 6502tass version. Original idea by M. Abrash. Extended
;version for extra-slow routine evaluation. Doesn't like timer interrupts 
;& output is in hex for simplicity's sake. Usage:

;  jsr measure    or <start adress>+0 if precompiled   to start cycle counting
;  jsr evaluate   or <start adress>+3 if precompiled   to stop counting & print result


overhead        = 19            ;cycles wasted by the timer itself during measurement
dma_off         = 1             ;0 to allow badlines (dito)
sprites_off     = 1             ;0 to allow sprites (dito)
printout        = $400          ;where to write the result 

;* = $1000      ;uncomment to precompile to wanted address

                jsr measure    ;start cycle counting
                jsr evaluate   ;stop count & print out cycle count

;                jmp measure

evaluate        sei
                lda #0
                sta $dc0e
                sta $dc0f
                lda vald011
                sta $d011
                lda vald015
                sta $d015
                cld
                sec
                lda #<($ffff-overhead)
                sbc $dc04
                sta cycles
                lda #>($ffff-overhead)
                sbc $dc05
                sta cycles+1
                lda #$ff
                sbc $dc06
                sta cycles+2
                lda #$ff
                sbc $dc07
                sta cycles+3
                ldx #3
                ldy #0
 showresult     lda cycles,x
                lsr
                lsr
                lsr
                lsr
                jsr toscreen
                lda cycles,x
                and #$0f
                jsr toscreen
                dex
                bpl showresult
                                                
                lda statusreg   ;restore (most of) st
                pha
                plp
                rts

toscreen        sed             ;simple hex to hexpetscii conversion,
                cmp #$0a        ;courtesy of Frank Kontros
                adc #$30
                cld
                sta printout,y
                lda $d021
                eor #$08
                sta (printout%$400)+$d800,y
                iny
                rts
                

cycles         .byte 0,0,0,0,0
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
                stx $dc0e       ;stop timers
                stx $dc0f       
.if dma_off
                stx $d011
.fi
.if sprites_off
                stx $d015
.fi

                dex
                cpx $d012
                bne *-3         ;wait for vblank area
                stx $dc04       ;set timers to $ffffffff
                stx $dc05
                stx $dc06       
                stx $dc07
                lda #$59
                sta $dc0f       ;reload and set timer b to count timer a underflow
                lda #$11
                sta $dc0e       ;reload and start timer a, continuous mode
                rts