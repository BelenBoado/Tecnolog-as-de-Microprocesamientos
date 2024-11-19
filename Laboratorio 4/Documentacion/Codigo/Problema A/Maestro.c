#include <avr/io.h>
#include <util/delay.h>
#include <stdio.h>

#define DHT11_PIN PD0
#define BOTON_PIN PD1

void InicializarADC() {
    ADMUX = (1 << REFS0); 
    ADCSRA = (1 << ADEN) | (1 << ADPS2) | (1 << ADPS1); 
}

uint16_t LeerADC(uint8_t channel) {
    ADMUX = (ADMUX & 0xF8) | (channel & 0x07); 
    ADCSRA |= (1 << ADSC); 
    while (ADCSRA & (1 << ADSC)); 
    return ADC;
}

void DHT11_SenalInicio() {
   DDRD |= (1 << DHT11_PIN);
   
   PORTD &= ~(1 << DHT11_PIN);
   
   _delay_ms(18);
   
   PORTD |= (1 << DHT11_PIN);
   
   _delay_us(40);
}

uint8_t DHT11_Respuesta() {
    DDRD &= ~(1 << DHT11_PIN);
    uint8_t respuesta = 0;
    while (PIND & (1 << DHT11_PIN));
    
    if (!(PIND & (1 << DHT11_PIN))) {
       _delay_us(80);
       
       if (PIND & (1 << DHT11_PIN)) {
   respuesta = 1;
       }
       while (PIND & (1 << DHT11_PIN));
    }
    return respuesta;
}

uint8_t DHT11_LeerDatos() {
    uint8_t dato = 0;
    for (int i = 0;i < 8; i++) {
       while (!(PIND & (1 << DHT11_PIN)));
       _delay_us(50);
       if (PIND & (1 << DHT11_PIN)) {
    dato |= (1 << (7 - i));
       } else {
    dato &= ~(1 << (7 - i));
       }
       while (PIND & (1 << DHT11_PIN));
    }
    return dato;
}

void SPI_IniciarMaestro(void) {
    DDRB = (1 << DDB3) | (1 << DDB5) | (1 << DDB2);  
    PORTB |= (1 << PORTB2);  

    SPCR = (1 << SPE) | (1 << MSTR) | (1 << SPR0);
}

void SPI_EnviarAEsclavo(uint16_t dato) {
    SPDR = dato;
    
    while (!(SPSR & (1 << SPIF)));
}

int main(void) {
    SPI_IniciarMaestro();  
    DDRD &= ~(1 << BOTON_PIN);
    PORTD &= ~(1 << BOTON_PIN);
    uint16_t humedad_ent, humedad_dec, temp_ent, temp_dec, suma, estadoBoton;
    InicializarADC();
    _delay_ms(1000);
    uint16_t valorPot;
    uint16_t datos[4];

    while (1) {
        valorPot = LeerADC(0);
        estadoBoton = 0;
        if (PIND & (1 << BOTON_PIN)) {
          estadoBoton = 1;
        }
        DHT11_SenalInicio();
        if (DHT11_Respuesta()) {
          humedad_ent = DHT11_LeerDatos();
          humedad_dec = DHT11_LeerDatos();
          temp_ent = DHT11_LeerDatos();
          temp_dec = DHT11_LeerDatos();
          suma = DHT11_LeerDatos();
        }
        datos[0] = valorPot/4;
        datos[1] = estadoBoton;
        datos[2] = humedad_ent;
        datos[3] = temp_ent;
        
        for (int i = 0;i<4;i++) {
          int dato = datos[i];
          PORTB &= ~(1 << PORTB2);  
          
          SPI_EnviarAEsclavo(dato);  
          
          PORTB |= (1 << PORTB2);  
          
          _delay_ms(100);  
        }
    }
}
