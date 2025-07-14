; this file is part of the C64 Emulator Test Suite. public domain, no copyright

; original file was: cia2pb7.asm
;-------------------------------------------------------------------------------

            .include "common.asm"
            .include "printhb.asm"
            .include "waitborder.asm"
            .include "waitkey.asm"
           
;-------------------------------------------------------------------------------           
thisname .null "cia2pb7"
nextname .null "cia1tab"
;-------------------------------------------------------------------------------           
         
main:         
;---------------------------------------
;old crb 0 start
;    crb 1 pb7out
;    crb 2 pb7toggle
;new crb 0 start
;    crb 1 pb7out
;    crb 2 pb7toggle
;    crb 4 force load

         .block
         jmp start

i        .byte 0
old      .byte 0
new      .byte 0
or       .byte 0
right    .text "----------------"
         .text "0000000000000000"
         .text "----------------"
         .text "1111111111111111"
         .text "----------------"
         .text "0000000000000000"
         .text "----------------"
         .text "1111111111111111"

start
         lda #0
         sta i
loop
         lda #$80
         sta $dd03
         lda #0
         sta $dd01
         sta $dd0e
         sta $dd0f
         lda #127
         sta $dd0d
         bit $dd0d
         lda #$ff
         sta $dd06
         sta $dd07
         lda i
         and #%00000111
         sta $dd0f
         sta old
         lda i
         lsr a
         lsr a
         pha
         and #%00010000
         sta or
         pla
         lsr a
         and #%00000111
         ora or
         sta $dd0f
         sta new
         lda $dd01
         eor #$80
         sta $dd01
         cmp $dd01
         beq minus
         eor #$80
         asl a
         lda #"0"/2
         rol a
         jmp nominus
minus
         lda #"-"
nominus
         ldx i
         cmp right,x
         beq ok
         pha
         jsr print
         .byte 13
         .text "old new pb7  "
         .byte 0
         lda old
         jsr printhb
         lda #32
         jsr $ffd2
         lda new
         jsr printhb
         lda #32
         jsr $ffd2
         pla
         jsr $ffd2
         jsr waitkey
ok
         inc i
         bmi end
         jmp loop
end
         .bend

;---------------------------------------
;toggle pb7, crb one shot, start timer
;-> pb7 must be high
;wait until crb has stopped
;-> pb7 must be low
;write crb, write ta low/high, force
;load, pb7on, pb7toggle
;-> pb7 must remain low
;start
;-> pb7 must go high

         .block
         lda #0
         sta $dd0f
         ldx #100
         stx $dd06
         sta $dd07
         sei
         jsr waitborder
         lda #$0f
         sta $dd0f
         lda #$80
         bit $dd01
         bne ok1
         jsr print
         .byte 13
         .null "pb7 is not high"
         jsr waitkey
ok1
         lda #$01
wait
         bit $dd0f
         bne wait
         lda #$80
         bit $dd01
         beq ok2
         jsr print
         .byte 13
         .null "pb7 is not low"
         jsr waitkey
ok2
         lda #$0e
         sta $dd0f
         lda #$80
         bit $dd01
         beq ok3
         jsr print
         .byte 13
         .text "writing crb may "
         .text "not set pb7 high"
         .byte 0
         jsr waitkey
ok3
         lda #100
         sta $dd06
         lda #$80
         bit $dd01
         beq ok4
         jsr print
         .byte 13
         .text "writing ta low may "
         .text "not set pb7 high"
         .byte 0
         jsr waitkey
ok4
         lda #0
         sta $dd05
         lda #$80
         bit $dd01
         beq ok5
         jsr print
         .byte 13
         .text "writing ta high may "
         .text "not set pb7 high"
         .byte 0
         jsr waitkey
ok5
         lda #$1e
         sta $dd0f
         lda #$80
         bit $dd01
         beq ok6
         jsr print
         .byte 13
         .text "force load may "
         .text "not set pb7 high"
         .byte 0
         jsr waitkey
ok6
         lda #%00001010
         sta $dd0f
         lda #%00001110
         sta $dd0f
         lda #$80
         bit $dd01
         beq ok7
         jsr print
         .byte 13
         .text "switching toggle "
         .text "may not set pb7 high"
         .byte 0
         jsr waitkey
ok7
         lda #%00001100
         sta $dd0f
         lda #%00001110
         sta $dd0f
         lda #$80
         bit $dd01
         beq ok8
         jsr print
         .byte 13
         .text "switching pb7on "
         .text "may not set pb7 high"
         .byte 0
         jsr waitkey
ok8
         sei
         jsr waitborder
         lda #%00000111
         sta $dd0f
         lda #$80
         bit $dd01
         bne ok9
         jsr print
         .byte 13
         .text "start must set "
         .text "pb7 high"
         .byte 0
         jsr waitkey
ok9
         lda #$80
         ldx #0
waitlow0
         dex
         beq timeout
         bit $dd01
         bne waitlow0
waithigh0
         dex
         beq timeout
         bit $dd01
         beq waithigh0
waitlow1
         dex
         beq timeout
         bit $dd01
         bne waitlow1
waithigh1
         dex
         beq timeout
         bit $dd01
         beq waithigh1
         jmp ok
timeout
         jsr print
         .byte 13
         .null "pb7 toggle timed out"
         jsr waitkey
ok
         .bend

;---------------------------------------
;crb pb7on/toggle 4 combinations
;wait until underflow
;set both pb7on and toggle
;-> pb7 must be independent from
;   pb7on/toggle state at underflow

         .block
         jmp start

i        .byte 0

start
         lda #3
         sta i
loop
         lda #0
         sta $dd0f
         lda #15
         sta $dd06
         lda #0
         sta $dd07
         sei
         jsr waitborder
         lda i
         sec
         rol a
         sta $dd0f
         ldx #$07
         stx $dd0f
         ldy $dd01
         sta $dd0f
         ldx #$07
         stx $dd0f
         lda $dd01
         and #$80
         bne error
         tya
         and #$80
         bne ok
error
         jsr print
         .byte 13
         .text "toggle state is not "
         .null "independent "
         lda i
         jsr printhb
         jsr waitkey
ok
         dec i
         bpl loop
         .bend

;---------------------------------------
;check pb7 timing

         .block
         jmp start

settab   .byte 7,7,7,7,7,7
         .byte 3,3,3,3,3,3,3,3
loadtab  .byte 7,6,3,2,1,0
         .byte 7,6,5,4,3,2,1,0
comptab  .byte 1,0,0,1,0,0
         .byte 0,1,0,0,0,0,0,1

i        .byte 0

start
         lda #loadtab-settab-1
         sta i
loop
         lda #0
         sta $dd0f
         ldx i
         lda loadtab,x
         sta $dd06
         lda #0
         sta $dd07
         sei
         jsr waitborder
         ldx i
         lda settab,x
         sta $dd0f
         nop
         nop
         lda $dd01
         asl a
         lda #0
         rol a
         cmp comptab,x
         beq ok
         jsr print
         .byte 13
         .null "timing error index "
         lda i
         jsr printhb
         jsr waitkey
ok
         dec i
         bpl loop
         .bend

;---------------------------------------

         rts
