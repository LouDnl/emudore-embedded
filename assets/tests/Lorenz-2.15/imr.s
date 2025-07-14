; this file is part of the C64 Emulator Test Suite. public domain, no copyright

; original file was: imr.asm
;-------------------------------------------------------------------------------

            .include "common.asm"
            .include "waitkey.asm"
            .include "waitborder.asm"

;------------------------------------------------------------------------------
thisname   .null "imr"      ; name of this test
nextname   .null "flipos"   ; name of next test, "-" means no more tests
;------------------------------------------------------------------------------
main:
; FIXME: save/restore vectors shouldnt be necessary here
         sec
         jsr vector
         jsr oldmain
         clc
         jsr vector

         rts    ; SUCCESS

;---------------------------------------
; vector save/restore

; FIXME: shouldnt complete vector reset be good enough?
; used before/after main, and in waitkey
vector
;           ldx #<$334
;           ldy #>$334
;           jmp $ff8d
            rts

;---------------------------------------

irqwait   .byte 0

onirq
         lda #1
         sta irqwait

         lda $dc0d      ; ACK CIA1 IRQs
         pla
         tay
         pla
         tax
         pla
         rti

;---------------------------------------

oldmain:
         sei
         lda #<onirq
         sta $0314
         lda #>onirq
         sta $0315

;---------------------------------------
;set imr clock 2

         .block
         sei            ; disable IRQs
         jsr waitborder
         lda #0
         sta irqwait
         sta $dc0e      ; CIA1 TimerA stop
         sta $dc05      ; CIA1 TimerA Hi
.ifeq NEWCIA - 1
         lda #7
.endif
         sta $dc04      ; CIA1 TimerA Lo
         lda #$7f
         sta $dc0d      ; disable all CIA1 IRQs
         bit $dc0d      ; ACK pending CIA1 IRQs
         lda #%00011001
         sta $dc0e      ; CIA1 TimerA start, oneshot, force load
         cli            ; 2 cycles enable IRQs
         lda #$81       ; 2 cycles
         sta $dc0d      ; 4 cycles enable CIA1 TimerA IRQ
         sei            ; 2 cycles disable IRQs
         lda irqwait
         beq ok
         jsr print
         .byte 13
         .text "imr=$81 irq in clock 2"
         .byte 0
         jsr waitkey
ok
         cli
         .bend

;---------------------------------------
;set imr clock 3

         .block
         sei            ; disable IRQs
         jsr waitborder
         lda #0
         sta irqwait
         sta $dc0e      ; CIA1 TimerA stop
         sta $dc04      ; CIA1 TimerA Lo
         sta $dc05      ; CIA1 TimerA Hi
         lda #$7f
         sta $dc0d      ; disable all CIA1 IRQs
         bit $dc0d      ; ACK pending CIA1 IRQs
         lda #%00011001
         sta $dc0e      ; CIA1 TimerA start, oneshot, force load
         cli            ; 2 cycles enable CIA1 TimerA IRQ
         lda #$81       ; 2 cycles
         sta $dc0d      ; 4 cycles enable CIA1 TimerA IRQ
.ifeq NEWCIA - 1
         lda #2 ; 2 cycles
.else
         lda 2 ; 3 cycles
.endif
         lda irqwait
         bne ok
         jsr print
         .byte 13
         .text "imr=$81 no irq in clock 3"
         .byte 0
         jsr waitkey
ok
         .bend

;---------------------------------------
;clear imr

         .block
         sei            ; disable IRQs
         jsr waitborder
         lda #0
         sta irqwait
         sta $dc0e      ; CIA1 TimerA stop
         sta $dc04      ; CIA1 TimerA Lo
         sta $dc05      ; CIA1 TimerA Hi
         ; TimerA value = 0000
         lda #$7f
         sta $dc0d      ; disable all CIA1 IRQs
         bit $dc0d      ; ACK pending CIA1 IRQs
         lda #%00011001
         sta $dc0e      ; (4) CIA1 TimerA start, oneshot, force load
         lda #$81       ; (2)
         sta $dc0d      ; (4) enable CIA1 TimerA IRQ
         ; IRQs should happen now. if we would read from $dc0d, we would read $81
         lda #$7f       ; (2)
         sta $dc0d      ; (4)disable all CIA1 IRQs
         ; although IMR was cleared (and no more IRQs can occur), the value that
         ; will get read from $dc0d still has the respective bits set, it must
         ; be read once to ACK pending IRQs
         lda $dc0d      ; read IRQ mask and ACK IRQs

         ; we expect to read $81 here, indicate failure if not so
         cmp #$81
         beq ok
         jsr print
         .byte 13
         .text "imr=$7f may not clear int"
         .byte 0
         jsr waitkey
ok
         .bend

;---------------------------------------
end
         lda #<$ea31
         sta $0314
         lda #>$ea31
         sta $0315
         cli
         rts
