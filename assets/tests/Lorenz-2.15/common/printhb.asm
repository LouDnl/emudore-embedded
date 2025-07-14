; this file is part of the C64 Emulator Test Suite. public domain, no copyright

            ; print hex byte in Akku
printhb
            .block
            pha
            lsr a
            lsr a
            lsr a
            lsr a
            jsr printhn
            pla
            and #$0f
printhn
            ora #$30
            cmp #$3a
            bcc printhn0
            adc #6
printhn0
            jsr cbmk_bsout
            rts
            .bend
 
 
