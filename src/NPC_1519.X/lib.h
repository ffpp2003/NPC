#ifndef LIB_H
#define LIB_H

#include <xc.h>

#define UART_TRIS_RX     TRISCbits.TRISC7
#define UART_TRIS_TX     TRISCbits.TRISC6
#define OVRPSS_V_RNGE 175 // distance from car to end of mirror view at rear bumper alignment

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

char *strtok_single(char *, char const *);
char UARTreadString(char *, unsigned char);
void UARTinit(const long, const unsigned char);
void UARTsendString(const char *, const unsigned char);
unsigned char us_to_cm(unsigned int);
unsigned char prom_us(unsigned int, unsigned int);
char updateSpeed(unsigned char *, float *, char *);
long map(long, long, long, long, long);
void beginSecuence();
void debugSS(float *, unsigned long *, unsigned char *);

#endif