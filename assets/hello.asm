; Message: hello
*=$033C        
    BYTE $4F,$49,$49,$45,$48
*=$1000
START
    JSR            PRINT_MESSAGE
EXIT
    RTS
PRINT_MESSAGE
    LDX #$04        ; initialize x to message length
GETCHAR
    LDA $033C,X     ; grab byte
    JSR $FFD2       ; render text in A with Subroutine:CLRCHN
    DEX             ; decrement X
    BPL GETCHAR     ; loop until X goes negative
    RTS