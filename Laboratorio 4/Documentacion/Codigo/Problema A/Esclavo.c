#include <avr/io.h>
#include <avr/interrupt.h>

volatile uint16_t DatoRecibido = 0;

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
    DDRD = (1 << PD7);
    UBRR0H = 0;
    UBRR0L = 103;  
    UCSR0B = (1 << TXEN0);  
    UCSR0C = (1 << UCSZ01) | (1 << UCSZ00);  
    _delay_ms(1100);
    char cadena[50];
    char cadena2[50];
    int datoAnterior = 0;
    while (1) {
        for (int i = 0; i<4;i++) {
          while (datoAnterior == DatoRecibido);
          datoAnterior = DatoRecibido;
          switch (i)
          {
              case 0:
                snprintf(cadena2, sizeof(cadena2), "POT:%d\r\n", DatoRecibido);
                EnviarCadenaUART(cadena2);            
                break;

              case 1:
                snprintf(cadena2, sizeof(cadena2), "Boton:%d\r\n", DatoRecibido);
                EnviarCadenaUART(cadena2);            
                if (DatoRecibido == 1) {
                    PORTD |= (1 << PD7);
                  } else {
                    PORTD &= ~(1 << PD7);  
                  }
                break;
              
              case 2:
                snprintf(cadena2, sizeof(cadena2), "Humedad:%d\r\n", DatoRecibido);
                EnviarCadenaUART(cadena2);            
                break;

              case 3:
                snprintf(cadena2, sizeof(cadena2), "Temperatura:%d\r\n", DatoRecibido);
                EnviarCadenaUART(cadena2);            
                break;
          }
        }  
    }
}
