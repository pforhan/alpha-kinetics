; Jaguar Startup Module
; Optimized for RMAC / Madmac syntax

    .extern ___main
    .globl  _start
    .globl  start

    .text

_start:
start:
    ; Initial Stack Pointer (Top of DRAM)
    move.l  #$001FFFFC, sp

    ; Disable Interrupts
    move.w  #$2700, sr

    ; Visual debug: Set background to Purple and enable video
    ; This proves the startup code is running.
    move.l  #$F00024, a0
    move.l  #$000000FF, (a0) ; BCOLOR (Blueish)
    move.w  #$0001, $4(a0)    ; VMODE at $F00028

    ; Jump to the C entry point
    jsr     ___main

    ; If __main returns, just loop forever
loop:
    bra.s   loop

    .bss
    .globl _opList
    .globl _frameBuffer
    .phrase
_opList:
    .ds.b 64
    .phrase
_frameBuffer:
    .ds.b 153600
