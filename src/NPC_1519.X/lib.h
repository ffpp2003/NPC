#ifndef LIB_H
#define LIB_H

#include <xc.h>

#define UART_TRIS_RX     TRISCbits.TRISC7
#define UART_TRIS_TX     TRISCbits.TRISC6

char *strtok_single(char *, char const *);
char UARTreadString(char *, unsigned char);
void UARTinit(const long, const unsigned char);
void UARTsendString(const char *, const unsigned char);
unsigned char us_to_cm(unsigned int);
unsigned char prom_us(unsigned int, unsigned int);
char updateSpeed(char *, float *, char *);
long map(long, long, long, long, long);

#endif