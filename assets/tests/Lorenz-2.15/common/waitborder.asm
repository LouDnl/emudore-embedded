;wait until raster line is in border
;to prevent getting disturbed by DMAs

.ifeq (TARGET - TARGETC64)
waitborder:
           .block
        dec $d020
            lda $d011
            bmi ok
wait
            lda $d012
            cmp #30
            bcs wait
ok
        inc $d020
            rts
           .bend 
.endif

.ifeq (TARGET - TARGETDTV)
waitborder:
           .block
        dec $d020
            lda $d011
            bmi ok
wait
            lda $d012
            cmp #30
            bcs wait
ok
        inc $d020
            rts
           .bend 
.endif

.ifeq (TARGET - TARGETPLUS4)
waitborder:
           .block
        dec $ff19

            lda $ff1c
            and #$01
            beq wait0
wait1
            lda $ff1d
            cmp #$30
            bcs wait1
            bcc ok
wait0
            lda $ff1d
            cmp #$d0
            bcc wait0
ok
        inc $ff19
            rts
           .bend 
.endif
