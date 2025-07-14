
; the TARGETXXX constants are referred to by their absolute value in makefiles, 
; so do not change them, only append new ones

TARGETC64       = 0
TARGETC128      = 1
TARGETC128C64   = 2
TARGETVIC20     = 3
TARGETPLUS4     = 4
TARGETDTV       = 5

TARGET1541      = 10
TARGET1571      = 11

; the CPUXXX constants will get defined by this include file

CPU6510     = 0
CPU6502     = 1
CPU65C02    = 2
CPU6502DTV  = 3

;------------------------------------------------------------------------------
.ifeq (TARGET - TARGETC64)
TARGETCPU = CPU6510

VICII = $d000
SID   = $d400
CIA1  = $dc00
CIA2  = $dd00

STARTADDR = $0801

MEMPAGE1000 = $1000
MEMPAGE1100 = $1100
MEMPAGE1200 = $1200
MEMPAGE1300 = $1300

cbmk_ioinit= $ff84
cbmk_restor= $ff8a
cbmk_setlfs= $ffba
cbmk_setnam= $ffbd
cbmk_bsout = $ffd2
cbmk_load  = $ffd5
cbmk_getin = $ffe4

SET_EXIT_CODE_SUCCESS:  .macro
        pha
        lda #$00
        sta $d7ff
        lda #13 ; l.green
        sta $d020
        pla
                        .endm

SET_EXIT_CODE_FAILURE:  .macro
        pha
        lda #$ff
        sta $d7ff
        lda #10 ; l.red
        sta $d020
        pla
                        .endm

RESET_MEMORY_MAP:   .macro
        lda #$37
        sta 1
        lda #$2f
        sta 0
                    .endm

RESET_KERNAL_IO:    .macro
        jsr cbmk_restor            ; Restore Kernal Vectors
        jsr cbmk_ioinit            ; Initialise I/O
                    .endm

RESET_COLORS:       .macro
        lda #0 ; black
        sta $d020
                    .endm
RESET_CURSOR:       .macro
        lda #0
        sta $d3
                    .endm

.endif

;------------------------------------------------------------------------------
.ifeq (TARGET - TARGETC128C64)
TARGETCPU = CPU6510

VICII = $d000
SID   = $d400
CIA1  = $dc00
CIA2  = $dd00

STARTADDR = $0801

MEMPAGE1000 = $1000
MEMPAGE1100 = $1100
MEMPAGE1200 = $1200
MEMPAGE1300 = $1300

cbmk_ioinit= $ff84
cbmk_restor= $ff8a
cbmk_setlfs= $ffba
cbmk_setnam= $ffbd
cbmk_bsout = $ffd2
cbmk_load  = $ffd5
cbmk_getin = $ffe4

SET_EXIT_CODE_SUCCESS:  .macro
        pha
        lda #$00
        sta $d7ff
        lda #13 ; l.green
        sta $d020
        pla
                        .endm

SET_EXIT_CODE_FAILURE:  .macro
        pha
        lda #$ff
        sta $d7ff
        lda #10 ; l.red
        sta $d020
        pla
                        .endm

RESET_MEMORY_MAP:   .macro
        lda #$37
        sta 1
        lda #$2f
        sta 0
                    .endm

RESET_KERNAL_IO:    .macro
        jsr cbmk_restor            ; Restore Kernal Vectors
        jsr cbmk_ioinit            ; Initialise I/O
                    .endm

RESET_COLORS:       .macro
        lda #0 ; black
        sta $d020
                    .endm
RESET_CURSOR:       .macro
        lda #0
        sta $d3
                    .endm

.endif

;------------------------------------------------------------------------------
.ifeq (TARGET - TARGETC128)
TARGETCPU = CPU6510
.endif

;------------------------------------------------------------------------------
.ifeq (TARGET - TARGETVIC20)
TARGETCPU = CPU6502
.endif

;------------------------------------------------------------------------------
.ifeq (TARGET - TARGETDTV)
TARGETCPU = CPU6502DTV

VICII = $d000
SID   = $d400
CIA1  = $dc00
CIA2  = $dd00

STARTADDR = $0801

MEMPAGE1000 = $1000
MEMPAGE1100 = $1100
MEMPAGE1200 = $1200
MEMPAGE1300 = $1300

cbmk_ioinit= $ff84
cbmk_restor= $ff8a
cbmk_setlfs= $ffba
cbmk_setnam= $ffbd
cbmk_bsout = $ffd2
cbmk_load  = $ffd5
cbmk_getin = $ffe4

SET_EXIT_CODE_SUCCESS:  .macro
        pha
        lda #$00
        sta $d7ff
        lda #13 ; l.green
        sta $d020
        pla
                        .endm

SET_EXIT_CODE_FAILURE:  .macro
        pha
        lda #$ff
        sta $d7ff
        lda #10 ; l.red
        sta $d020
        pla
                        .endm

RESET_MEMORY_MAP:   .macro
        lda #$37
        sta 1
        lda #$2f
        sta 0
                    .endm

RESET_KERNAL_IO:    .macro
        jsr cbmk_restor            ; Restore Kernal Vectors
        jsr cbmk_ioinit            ; Initialise I/O
                    .endm

RESET_COLORS:       .macro
        lda #0 ; black
        sta $d020
                    .endm
RESET_CURSOR:       .macro
        lda #0
        sta $d3
                    .endm
.endif

;------------------------------------------------------------------------------
.ifeq (TARGET - TARGETPLUS4)
TARGETCPU = CPU6502

STARTADDR = $1001

MEMPAGE1000 = $1800
MEMPAGE1100 = $1900
MEMPAGE1200 = $1a00
MEMPAGE1300 = $1b00

cbmk_ioinit= $ff84
cbmk_restor= $ff8a
cbmk_setlfs= $ffba
cbmk_setnam= $ffbd
cbmk_bsout = $ffd2
cbmk_load  = $ffd5
cbmk_getin = $ffe4

SET_EXIT_CODE_SUCCESS:  .macro
        pha
        lda #$00
        sta $fdcf
        lda #5+64  ; l.green
        sta $ff19
        pla
                        .endm

SET_EXIT_CODE_FAILURE:  .macro
        pha
        lda #$ff
        sta $fdcf
        lda #2+64  ; l.red
        sta $ff19
        pla
                        .endm

RESET_MEMORY_MAP:   .macro
        ; nothing to do here
                    .endm

RESET_KERNAL_IO:    .macro
        jsr cbmk_restor            ; Restore Kernal Vectors
;        jsr cbmk_ioinit            ; Initialise I/O
                    .endm

RESET_COLORS:       .macro
        lda #0 ; black
        sta $ff19
                    .endm
RESET_CURSOR:       .macro
        lda #0
        sta $ca
                    .endm

.endif

