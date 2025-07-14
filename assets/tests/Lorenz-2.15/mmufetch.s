; this file is part of the C64 Emulator Test Suite. public domain, no copyright

; original file was: mmufetch.asm
;-------------------------------------------------------------------------------

            .include "common.asm"
            .include "printhb.asm"

;------------------------------------------------------------------------------           
thisname    .null "mmufetch" ; name of this test
nextname    .null "mmu"      ; name of next test, "-" means no more tests
;-------------------------------------------------------------------------------           

rom
         lda #$2f
         sta 0
         lda #$37
         sta 1
         cli
         rts

irqhandler:
         pha
         txa
         pha

         tsx
         lda $104,x
         and #$10
         bne breakhandler
         
         inc $d020
         dec $d020
         
         lda $dc0d
         pla
         tax
         pla
         rti

breakhandler:
         jsr rom
         sei
         lda #2
         sta $d020
         lda #$ff
         sta $d7ff  ; failed
         jmp *

main
         lda #<breakhandler
         sta $0316
         lda #>breakhandler
         sta $0317
         lda #<irqhandler
         sta $fffe
         lda #>irqhandler
         sta $ffff

         lda #$30   ; "0"
         sta $0400
         
         jsr rom
         sei

;a000 ram-rom-ram
         ldy #1
         sty $24
         dey
         sty $25

         lda #$36
         sta 1          ; BASIC off

         ; save RAM a4df-a4e3
         lda $a4df
         pha
         lda $a4e0
         pha
         lda $a4e1
         pha
         lda $a4e2
         pha
         lda $a4e3
         pha

         lda #$86   ; stx
         sta $a4df
         lda #1     ; $01
         sta $a4e0
         lda #0     ; brk
         sta $a4e1
         sta $a4e2
         lda #$60   ; rts
         sta $a4e3

         lda #$36
         ldx #$37
         jsr $a4df
         ; a4df stx $01     ; should switch to ROM
         ; RAM a4e1 brk
         ; RAM a4e2 brk
         ; RAM a4e3 rts

         ; restore RAM a4df-a4e3
         pla
         sta $a4e3
         pla
         sta $a4e2
         pla
         sta $a4e1
         pla
         sta $a4e0
         pla
         sta $a4df

         inc $0400  ; 1
         
;b000 ram-rom-ram
         ldy #1
         sty $14
         dey
         sty $15
         
         lda #$36
         sta 1
         
         ; save b828-b82c
         lda $b828
         pha
         lda $b829
         pha
         lda $b82a
         pha
         lda $b82b
         pha
         lda $b82c
         pha
         
         lda #$86   ; stx
         sta $b828
         lda #1     ; $01
         sta $b829
         lda #0     ; brk
         sta $b82a
         sta $b82b
         lda #$60   ; rts
         sta $b82c
         
         lda #$36
         ldx #$37
         jsr $b828
         ; b828 stx $01     ; should switch to ROM
         ; RAM b829 brk
         ; RAM b82a brk
         ; RAM b82b rts
         
         ; restore b828-b82c
         pla
         sta $b82c
         pla
         sta $b82b
         pla
         sta $b82a
         pla
         sta $b829
         pla
         sta $b828

         inc $0400  ; 2
         
;e000 ram-rom-ram
         lda #$86   ; stx
         sta $ea77
         lda #1     ; $01
         sta $ea78
         lda #0     ; brk
         sta $ea79
         sta $ea7a
         lda #$60   ; rts
         sta $ea7b
         
         lda #$35
         ldx #$37
         sta 1
         jsr $ea77
         ; ea77 stx $01     ; should switch to ROM
         ; RAM ea78 brk
         ; RAM ea79 brk
         ; RAM ea7a rts

         inc $0400  ; 3
         
;f000 ram-rom-ram
         ldy #1
         sty $c3
         dey
         sty $c4
         
         lda #$86   ; stx
         sta $fd25
         lda #1     ; $01
         sta $fd26  
         lda #0     ; brk
         sta $fd27
         sta $fd28
         lda #$60   ; rts
         sta $fd29
         
         lda #$35
         ldx #$37
         sta 1
         jsr $fd25
         ; fd25 stx $01     ; should switch to ROM
         ; RAM fd27 brk
         ; RAM fd28 brk
         ; RAM fd29 rts

         inc $0400  ; 4
         
;d000 ram-rom-ram
         lda $91
         pha
         lda $92
         pha

         ldy #1
         sty $91
         dey
         sty $92
         
         lda #$34
         sta 1
         
         lda #$86   ; stx
         sta $d400
         lda #1     ; $01
         sta $d401
         lda #0     ; brk
         sta $d402
         sta $d403
         lda #$60   ; rts
         sta $d404
         
         lda #$34
         ldx #$33
         sta 1              ; switch to RAM
         jsr $d400
         ; RAM d400 stx $01     ; should switch to ROM
         ; ROM d402 STA ($91),Y ; should switch to RAM
         ; RAM d404 rts

         pla
         sta $92
         pla
         sta $91

         inc $0400  ; 5
         
;d000 ram-io-ram
         lda #$37
         sta 1      ; I/O at $d000

         lda #0     ; I/O d000   00 00
         sta $d000
         sta $d001
         lda #$85   ; I/O d002   85 01  STA $01
         sta $d002
         lda #1
         sta $d003
         lda #0     ; I/O d004   00
         sta $d004

         lda #$33
         sta 1      ; chargen at $d000

         lda #$86   ; RAM d000   86 01  STX $01
         sta $d000
         lda #1
         sta $d001
         lda #0     ; RAM d002   00 00
         sta $d002
         sta $d003
         lda #$60   ; RAM d004   60     RTS
         sta $d004

         lda #$34
         ldx #$37
         sta 1      ; RAM at $d000
         jsr $d000
         ; when everything works correctly, execution goes as follows:

         ; RAM d000   86 01  STX $01    <- enables I/O
         ; I/O d002   85 01  STA $01    <- enables RAM
         ; RAM d004   60     RTS

         inc $0400  ; 6

         jsr rom

ok

        rts ; SUCCESS

