; this file is part of the C64 Emulator Test Suite. public domain, no copyright

; original source file: n/a (broken), this was recreated from a disassembly
; slightly updated so the test does actually fail when it behaves wrong
;-------------------------------------------------------------------------------

            .include "common.asm"

;------------------------------------------------------------------------------           
thisname    .null "branchwrap" ; name of this test
nextname    .null "mmufetch"   ; name of next test, "-" means no more tests
;-------------------------------------------------------------------------------           

irqdisable
        ; disable timer irq
        lda #$7f
        sta $dc0d
        ; disable ROMs
        lda #$e3
        sta $00
        lda #$34
        sta $01
        rts

irqenable
        ; enable ROMs
        lda #$2f
        sta $00
        lda #$37
        sta $01
        ; enable timer irq
        lda #$81
        sta $dc0d
        rts

main
        jsr irqdisable

        lda #$10  ; bpl
        sta $ffbe
        lda #$42  ; $0002
        sta $ffbf
        lda #$a9  ; lda #
        sta $ffc0
        lda #$00  ; $00
        sta $ffc1
        lda #$60  ; rts
        sta $ffc2

        lda #$a9  ; lda #
        sta $02
        lda #$01  ; $01
        sta $03
        lda #$60  ; rts
        sta $04

        lda #$4c  ; jmp
        sta $ff02
        lda #<failed
        sta $ff03
        lda #>failed
        sta $ff04
lp1
        ; flag cleared, branch taken
        ;
        ; NV-BDIZC 
        ; 00110000
        lda #$30
        pha
        plp
        jsr $ffbe
        ; $ffbe bpl, bvc, bcc, bne $0002
        ; $ffc0 lda #$00
        ; $ffc2 rts
        ; $0002 lda #$01    <-
        ; $0004 rts
        bne ok1
        jmp failed
ok1:        
        ; flag set, branch not taken
        ;
        ; NV-BDIZC 
        ; 11110011
        lda #$f3
        pha
        plp
        jsr $ffbe
        ; $ffbe bmi, bvs, bcs, beq $0002
        ; $ffc0 lda #$00    <-
        ; $ffc2 rts
        ; $0002 lda #$01
        ; $0004 rts
        beq ok2
        jmp failed
ok2:        
        
        clc
        lda $ffbe
        adc #$40
        sta $ffbe
        bcc lp1

        lda #$30  ; bmi
        sta $ffbe
        
lp2
        ; flag cleared, branch not taken
        ;
        ; NV-BDIZC 
        ; 00110000
        lda #$30
        pha
        plp
        jsr $ffbe
        ; $ffbe bpl, bvc, bcc, bne $0002
        ; $ffc0 lda #$00   <-
        ; $ffc2 rts
        ; $0002 lda #$01    
        ; $0004 rts
        beq ok3
        jmp failed
ok3:        
        ; flag set, branch taken
        ;
        ; NV-BDIZC 
        ; 11110011
        lda #$f3
        pha
        plp
        jsr $ffbe
        ; $ffbe bmi, bvs, bcs, beq $0002
        ; $ffc0 lda #$00    
        ; $ffc2 rts
        ; $0002 lda #$01    <-
        ; $0004 rts
        bne ok4
        jmp failed
ok4:        
        
        clc
        lda $ffbe
        adc #$40
        sta $ffbe
        bcc lp2

        jsr irqenable

        rts ; SUCCESS

failed:
        jsr print
        .text " - error"
        .byte $0d, 0

        lda #$ff       ; failure
        sta $d7ff
        lda #10
        sta $d020
        jmp *
