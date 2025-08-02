COLOR0= $CE00  ; Place for color bar 0
COLOR1= $CF00  ; Place for color bar 1
RASTER= $FA    ; Line for the raster interrupt
DUMMY= $CFFF   ; Timing variable

*= $C000
        SEI             ; Disable interrupts
        LDA #$7F        ; Disable timer interrupts
        STA $DC0D
        LDA #$01        ; Enable raster interrupts
        STA $D01A
        STA $D015       ; Enable Sprite 0
        LDA #<IRQ       ; Init interrupt vector
        STA $0314
        LDA #>IRQ
        STA $0315
        LDA #$1B
        STA $D011
        LDA #RASTER     ; Set interrupt position (inc. 9th bit)
        STA $D012
        LDA #RASTER-20  ; Sprite will just reach the interrupt position
        STA $D001       ;  when it is positioned 20 lines earlier

        LDX #51
        LDY #0
        STA $D017       ; No Y-enlargement
LOOP0   LDA COL,X       ; Create color bars
        PHA
        AND #15
        STA COLOR0,X
        STA COLOR0+52,Y
        STA COLOR0+104,X
        STA COLOR0+156,Y
        PLA
        LSR
        LSR
        LSR
        LSR
        STA COLOR1,X
        STA COLOR1+52,Y
        STA COLOR1+104,X
        STA COLOR1+156,Y
        INY
        DEX
        BPL LOOP0
        CLI             ; Enable interrupts
        RTS             ; Return


IRQ     NOP             ; Wait a bit
        NOP
        NOP
        NOP
        LDY #103        ; 104 lines of colors (some of them not visible)
			; Reduce for NTSC, 55 ?
        INC DUMMY       ; Handles the synchronization with the help of the
        DEC DUMMY       ;  sprite and the 6-clock instructions
			; Add a NOP for NTSC

FIRST   LDX COLOR0,Y    ; Do the color effects
SECOND  LDA COLOR1,Y
        STA $D020
        STX $D020
        STA $D020
        STX $D020
        STA $D020
        STX $D020
        STA $D020
        STX $D020
        STA $D020
        STX $D020
        STA $D020
        STX $D020
			; Add a NOP for NTSC (one line = 65 cycles)
        LDA #0          ; Throw away 2 cycles (total loop = 63 cycles)
        DEY
        BPL FIRST       ; Loop for 104 lines

        STA $D020
        LDA #103        ; For subtraction
        DEC FIRST+1     ; Move the bars
        BPL OVER
        STA FIRST+1
OVER    SEC
        SBC FIRST+1
        STA SECOND+1

        LDA #1          ; Ack the raster interrupt
        STA $D019
        JMP $EA31       ; Jump to the standard irq handler

COL     BYT $09,$90,$09,$9B,$00,$99,$2B,$08,$90,$29,$8B,$08,$9C,$20,$89,$AB
        BYT $08,$9C,$2F,$80,$A9,$FB,$08,$9C,$2F,$87,$A0,$F9,$7B,$18,$0C,$6F
        BYT $07,$61,$40,$09,$6B,$48,$EC,$0F,$67,$41,$E1,$30,$09,$6B,$48,$EC
        BYT $3F,$77,$11,$11
                        ; Two color bars

