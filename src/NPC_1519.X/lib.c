/*
 * File:   lib.c
 * Author: ffpp2
 *
 * Created on 5 de noviembre de 2022, 17:45
 */

#include <xc.h>
#include <string.h>
#include "config.h"
#include "lib.h"

char *strtok_single(char *str, char const *delims) {
    static char *src = NULL;
    char *p, *ret = 0;

    if (str != NULL)
        src = str;

    if (src == NULL)
        return NULL;

    if ((p = strpbrk(src, delims)) != NULL) {
        *p = 0;
        ret = src;
        src = ++p;
    } else if (*src) {
        ret = src;
        src = NULL;
    }

    return ret;
}


// RCREG: EUSART Receive Data Register
// OERR: Receive Overrun error (FIFO out of space). Clearing this bit is mandatory when this error ocurrs (reset USART)
// CREN: Continous Receive enable Bit
// FERR: Framing Error (stop bit missing)(not required for normal operation)


void UARTinit(const long baud_rate, const unsigned char BRGH) {
    // Calculate BRG
    if (BRGH == 0) {
        SPBRG = _XTAL_FREQ / (64 * baud_rate) - 1;
        TXSTAbits.BRGH = 0;
    } else {
        SPBRG = _XTAL_FREQ / (16 * baud_rate) - 1;
        TXSTAbits.BRGH = 1;
    }

    // TXSTA register
    TXSTAbits.TX9 = 0; // 8-bit transmission
    TXSTAbits.TXEN = 1; // Enable transmission
    TXSTAbits.SYNC = 0; // Asynchronous mode

    // RCSTA register
    RCSTAbits.SPEN = 1; // Enable serial port
    RCSTAbits.RX9 = 0; // 8-bit reception
    RCSTAbits.CREN = 1; // Enable continuous reception
    RCSTAbits.FERR = 0; // Disable framing error
    RCSTAbits.OERR = 0; // Disable overrun error

    // Set up direction of RX/TX pins
    UART_TRIS_RX = 1;
    UART_TRIS_TX = 0;
}

void UARTsendString(const char* str, const unsigned char max_length) {
    int i = 0;
    for (i = 0; i < max_length && str[i] != '\0'; i++) {
        while (TXSTAbits.TRMT == 0); // Wait for buffer to be empty
        TXREG = str[i];
    }
}

char UARTreadString(char *buf, unsigned char bufSize){      // Execute ONLY on ISR RCIF!!
    static int i = 0;
    if (RCSTAbits.OERR) {                                   // Overrun error ocurred
        char tmp = RCREG;                                   // Clear last byte of the receive register
        RCSTAbits.CREN = 0;                                 // Reset EUSART
        RCSTAbits.CREN = 1;
        return 0;                                           // Return 0 as no data was read
    }
    char tmp = RCREG;                                       // Read last byte of the receive register

    if (i < bufSize){                                       // Verify that there is still space on the buffer
        if (tmp == '\0' || tmp == '\n' || tmp == '\r') {    // Check if an ending character is present
            buf[i + 1] = '\0';                              // Removes \n or \r and replaces it by NULL
            i = 0;                                          // Reset number of received bytes
            return 1;                                       // Return 1 to tell that the end of the line was reached
        }
        buf[i] = tmp;                                       // Copy the received byte to the buffer
        i++;                                                // Add 1 to keep track of received bytes
        return 0;                                           // Return 0 to tell that the end of line wasn't reached
    }
    return 0;
}

unsigned char us_to_cm(unsigned int us){
    return us/58;
}

unsigned char prom_us(unsigned int us1, unsigned int us2){
    return (us1 + us2)/2;
}

char updateSpeed(char *DRDY, float *v, char *string){
    if (*DRDY){
            *DRDY = 0;
            if (!strncmp(string, "$GPVTG", 6)){
                strtok_single(string, ",");
                for (int i = 0; i < 6; i++){
                    strtok_single(NULL, ",");
                }
                *v = atof(strtok_single(NULL, ","));
                return 1;
            }
        }
    return 0;
}

long map(long x, long in_min, long in_max, long out_min, long out_max) {
  return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}