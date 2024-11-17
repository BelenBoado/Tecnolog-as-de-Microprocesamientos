#include <avr/io.h>
#include <avr/interrupt.h>

volatile char DatoRecibido = 0;

void SPI_InicializarEsclavo(void) {
    DDRB = (1 << DDB4);
    
    SPCR = (1 << SPE);
    
    SPCR |= (1 << SPIE);
    sei();  
}

ISR(SPI_STC_vect) {
    DatoRecibido = SPDR;
}

void EnviarUART(unsigned char dato) {
    while (!(UCSR0A & (1 << UDRE0))); 
    UDR0 = dato;
}

void EnviarCadenaUART(const char *str) {
    while (*str) {
        EnviarUART(*str++);
    }
}

int main(void) {
    SPI_InicializarEsclavo();  
    
    UBRR0H = 0;
    UBRR0L = 103;  
    UCSR0B = (1 << TXEN0);  
    UCSR0C = (1 << UCSZ01) | (1 << UCSZ00);  

    char cadena[50];
    while (1) {
        snprintf(cadena, sizeof(cadena), "%d\r\n", DatoRecibido);
        EnviarCadenaUART(cadena);
        _delay_ms(1000);  
    }
}
