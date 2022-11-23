/*
 * File:   main.c
 * Author: Alumno
 *
 * Created on 11 de noviembre de 2022, 13:29
 */


#include <xc.h>
#include <stdio.h>
#include "config.h"
#include "lib.h"

#define OVRPSS_V_RNGE 200 // distance from car to end of mirror view at rear bumper alignment

#define TRIG PORTCbits.RC0
//#define BUZZ PORTAbits.RA6
#define BUZZ PORTCbits.RC1
#define SLOWUSA 0
#define SLOWUSB 1
#define GP_LED  PORTAbits.RA0
#define BP_LED  PORTAbits.RA1
#define RP_LED  PORTAbits.RA2
#define GD_LED  PORTAbits.RA3
#define BD_LED  PORTAbits.RA4
#define RD_LED  PORTAbits.RA5
#define OVRPS_D PORTAbits.RA6
#define OVRPS_P PORTAbits.RA7

// Ultrasonic Echo pin Definitions
//RB0 is Front Grill D
//RB1 is Front Grill P
//RB2 is Rear bumper D
//RB3 is Rear bumper P
//RB4 is Front Right mirror A
//RB5 is Front Right mirror B
//RB6 is Front Left mirror A
//RB7 is Front Left mirror B
//
// The variable us_us stores the respective echo time for every Ultrasonic in the avove mentioned order

// LED LAYOUT
//Green RA0     |
//Blue RA1      |> Passenger side
//Red RA2       |
//Green RA3     |
//Blue RA4      |> Drivers side
//Red RA5       |
//Red PA6       |> Overpass Driver
//Red PA7       |> Overpass Passenger

unsigned long millis = 0, prevMillis = 0;
unsigned int us_us[8] = {0};
char GPSRDY = 0, bzzEn = 0;
float speed = 0;
char NMEASentence[50] = {0};

void main(void) {
    OSCCON = 0x6A;
    TRISA = 0x00;
    TRISB = 0xFF;
    TRISC = 0x00;
    ANSELA = 0x00;
    ANSELB = 0x00;
    ANSELC = 0x00;
    PORTA = 0x00;
    PORTB = 0x00;
    PORTC = 0x00;

    INTCON = 0xE8; //Activa las interrupciones por cambio de estado, timer0 y perifericos
    PIE1 = 0x22; //Activa las interrupciones por Rx del modulo ESUART
    IOCBN = 0b00000011; //Activa las interrupciones por cambio de estado(flanco negativo) 

    OPTION_REG = 0xC6; //Activa el timer0 con preescaler de 1:128, Fosc/4, no WPU
    T1CON = 0x01; //Activa el timer1 con preescaler de 1:1
    T1GCON = 0xE1; //Activa el Gate Control del timer1 y usa como gate el TMR0IF (modo toggle)
    
    T2CON = 0x3C;
    PR2 = 125;
    
    UARTinit(9600, 1);
    UARTsendString("PIC INIT OK!\n\r", 24);
    while (1){
//        asd =  map(us_to_cm(us_cm[0]), 20, 150, 30, 900);
//        if (updateSpeed(&GPSRDY, &speed, NMEASentence)){
//            sprintf(asd, "P:%d,T:%lu\n\r", us_to_cm(us_cm[0]), anashee);
//            UARTsendString(asd, sizeof asd);
//        }
        // BEGIN Front Grill Check BEGIN
        if (speed <= 3){
            long bzzTime = 0;
            bzzTime = map(prom_us(us_to_cm(us_us[0]), us_to_cm(us_us[1])), 30, 2000, 40, 1500);
            if ((millis - prevMillis) >= bzzTime){
                bzzEn = 1;
                prevMillis = millis;
            }
            if (bzzTime < 0){
                BUZZ = 1;
            }
        }
        // END Front Grill Check END
        // BEGIN Overpass Check BEGIN
        if (speed >= 13){ // faster than 50km/h
            // Driver overpass check
            if (us_to_cm(us_us[2]) <= OVRPSS_V_RNGE){
                OVRPS_D = 1;
            } else {
                OVRPS_D = 0;
            }
            // Passenger overpass check
            if (us_to_cm(us_us[3]) <= OVRPSS_V_RNGE){
                OVRPS_P = 1;
            } else {
                OVRPS_P = 0;
            }
        }
        // END Overpass Check END
        // BEGIN Pillar Check BEGIN
        if (speed >= 3 && speed <= 13){ //between 10 and 50km/h
            int ledIntenD = 0, ledIntenP = 0;
            ledIntenD = map(us_to_cm(us_us[4]), 50, 200, 0, 2);
            ledIntenP = map(us_to_cm(us_us[5]), 50, 200, 0, 2);
            switch (ledIntenD){
                case 0:
                    GD_LED = 1;
                    RD_LED = 0;
                    BD_LED = 0;
                    break;
                case 1:
                    GD_LED = 1;
                    RD_LED = 1;
                    BD_LED = 0;
                    break;
                case 2:
                    GD_LED = 0;
                    RD_LED = 1;
                    BD_LED = 0;
                    break;
            }
            switch (ledIntenP){
                case 0:
                    GP_LED = 1;
                    RP_LED = 0;
                    BP_LED = 0;
                    break;
                case 1:
                    GP_LED = 1;
                    RP_LED = 1;
                    BP_LED = 0;
                    break;
                case 2:
                    GP_LED = 0;
                    RP_LED = 1;
                    BP_LED = 0;
                    break;
            }
        }
    }
}

void __interrupt() isr() {
    if (INTCONbits.TMR0IF) {
        static char meas = 1;   // Variable to know when it is time to trigger
        TMR0 = TMR0 + 22;       // Trigger occurs every 60ms, but only reading echo the first 30ms
        if (meas) {
            TMR1H = 0x00;       // Reset Timer0
            TMR1L = 0x00;
            TRIG = 1;           // Trigger all Ultrasonic sensors
            __delay_us(10);
            TRIG = 0;
        }
        meas = !meas;
        INTCONbits.TMR0IF = 0;
    }
    if (INTCONbits.IOCIF) {
        for (int i = 0; i < 8; i++) {
            us_us[i] = (((IOCBF >> i) & 0b1) == 1 ? (((TMR1H << 8) | TMR1L) - 471) : us_us[i]);
            IOCBF = (IOCBF & (~(1 << i))) | (0 << i);
        }
        INTCONbits.IOCIF = 0;
    }
    if (PIR1bits.RCIF) {
        GPSRDY = UARTreadString(NMEASentence, sizeof NMEASentence);
    }
    if (PIR1bits.TMR2IF){
        static int bzzPasses = 0;
        millis += 1;
        if (bzzEn){
            BUZZ = 1;
            bzzPasses++;
            if (bzzPasses >= 40){
                BUZZ = 0;
                bzzEn = 0;
                bzzPasses = 0;
            }
        }
        PIR1bits.TMR2IF = 0;
    }
}