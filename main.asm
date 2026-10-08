            .cdecls C,LIST,"msp430.h"


            .text
            .retain
            .retainrefs

            .global RESET
            .global main
            .global echo_start
            .global echo_end
            .global echo_captured
            .global distance_cm ;deklaracija

; TA0 Capture ISR TimerA0 generise capture interrupt
TA0_Capture_ISR:

    push    r15
    push    r14 ;cuvanje registri koje se koriste

    mov.w   &TA0IV,r15
    cmp.w   #2,r15
    jne     T0_END ;ako nije prekid iz CCR1

    bit.b   #CCI,&TA0CCTL1 ;edge
    jnz     T0_RISE

    mov.w   &TA0CCR1,r14 ;preuzmi vrednost tajmera
    mov.w   r14,&echo_end ;kraj pulsa
    mov.b   #1,&echo_captured

    jmp     T0_END

T0_RISE:
    mov.w   &TA0CCR1,r14 ;rising edge
    mov.w   r14,&echo_start ;pocetak pulsa

T0_END:
    pop r14
    pop r15
    reti

; TA1 LED ISR
TA1_LED_ISR:
    push r15

    mov.w &distance_cm,r15
    cmp.w #60,r15
    jge LED_OFF

    xor.b #BIT4,&P2OUT   ; LD3
    xor.b #BIT5,&P2OUT   ; LD4
    jmp LED_DONE

LED_OFF:
    bic.b #BIT4,&P2OUT
    bic.b #BIT5,&P2OUT

LED_DONE:
    pop r15
    reti

; Vektori
    .sect ".int55"  ; TA0 capture
    .short TA0_Capture_ISR

    .sect ".int56"  ; TA1 LED
    .short TA1_LED_ISR
