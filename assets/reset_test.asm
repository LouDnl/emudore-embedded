; assembled with AS65 from http://www.kingswood-consulting.co.uk/assemblers/
; command line switches: -l -m -s2 -w -h0
;                         |  |  |   |  no page headers in listing
;                         |  |  |   wide listing (133 char/col)
;                         |  |  write intel hex file instead of binary
;                         |  expand macros in listing
;                         generate pass2 listing

* = $0810    ; start with SYS2064 from BASIC[5]
 sei
 lda #$c3     ;the string "CBM80" in PETSCII from $8004
 sta $8004    ;is used by ROM to identify presence of a cartridge
 lda #$c2
 sta $8005
 lda #$cd
 sta $8006
 lda #$38
 sta $8007
 lda #$30
 sta $8008
 lda #<reset  ;point address vector at $8000 to our code
 sta $8000
 lda #>reset
 sta $8001
main
 inc $d020    ;visual stimulation
 jmp main

reset          ;here's where we end up when there's reset
 lda #$2f     ;reset data-direction register, otherwise the system won't start correctly
 sta $00
 jsr $e5a8    ;optional, refresh the VIC
 jmp main     ;go back to main code
