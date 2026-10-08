/**
 * @file main.c
 *
 * Developed an HC-SR04-based parking sensor using timer
 * Capture for distance measurement, UART for PC communication
 * LED indicators with distance-dependent blinking frequency.
 *
 * @date 2025
 * @author Ivona Radojicic
 */
#include <msp430.h>
#include <stdint.h>

//Pins
#define TRIG_PIN    BIT0    // P2.0
#define ECHO_PIN    BIT6    // P1.6
#define LD3         BIT4    // P2.4
#define LD4         BIT5    // P2.5

volatile unsigned int echo_start = 0;
volatile unsigned int echo_end = 0;
volatile uint8_t echo_captured = 0;
volatile uint16_t distance_cm = 0;


//UART
void uart_init(void) {
    P4SEL |= BIT4 | BIT5;       // P4.4=TX, P4.5=RX
    UCA1CTL1 |= UCSWRST;
    UCA1CTL1 |= UCSSEL_2;       // SMCLK
    UCA1BR0 = 104;              // 9600 baud @1MHz
    UCA1BR1 = 0;
    UCA1MCTL = UCBRS0;
    UCA1CTL1 &= ~UCSWRST;
}

void uart_puts(const char *s) {
    while(*s) {
        while(!(UCA1IFG & UCTXIFG));
        UCA1TXBUF = *s++;
    }
}

//Converts number to string
void itoa_simple(uint16_t val, char *buf) {
    unsigned int i = 0, j = 0;
    char temp[6];
    if(val == 0){
        buf[0]='0'; buf[1]=0; return;
    }
    while(val>0){
        temp[i++] = (val%10)+'0';
        val/=10;
    }
    for(j=0;j<i;j++) buf[j]=temp[i-j-1];
    buf[i]=0;
}

//TRIG impulse
void trig_pulse(void) {
    P2OUT |= TRIG_PIN;
    __delay_cycles(20);
    P2OUT &= ~TRIG_PIN;

//HCSR04 measurement
uint16_t measure_distance(void) {
    uint32_t count = 0;

    trig_pulse();


    uint32_t timeout = 0;
    while(!(P1IN & ECHO_PIN)) {
        timeout++;
        __delay_cycles(10);
        if(timeout > 50000) {
            return 0;
        }
    }


    while(P1IN & ECHO_PIN){
        count++;
        __delay_cycles(10);
        if(count>60000) break; // timeout
    }

    uint16_t distance = (uint16_t)(count * 10 / 58);
    return distance;
}

int main(void){
    WDTCTL = WDTPW | WDTHOLD; // stop WDT

    // GPIO
    P2DIR |= TRIG_PIN | LD3 | LD4;
    P2OUT &= ~(TRIG_PIN | LD3 | LD4);
    P1DIR &= ~ECHO_PIN;       // ECHO input

    uart_init();

    char num_str[6];

    while(1){
        distance_cm = measure_distance();

        // UART
        itoa_simple(distance_cm,num_str);
        uart_puts("Distance = ");
        uart_puts(num_str);
        uart_puts(" cm\r\n");

        // LED indicators with distance-dependent blinking frequency
        if(distance_cm < 10){
            P2OUT ^= LD3 | LD4;
            __delay_cycles(20000);
        } else if(distance_cm < 30){
            P2OUT ^= LD3 | LD4;
            __delay_cycles(50000);
        } else if(distance_cm < 60){
            P2OUT ^= LD3 | LD4;
            __delay_cycles(100000);
        } else {
            P2OUT &= ~(LD3 | LD4); // LED off
            __delay_cycles(100000);
        }
    }
}
