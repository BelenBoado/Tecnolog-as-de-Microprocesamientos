#include <avr/io.h>       // Biblioteca para operaciones de entrada/salida en AVR
#include <avr/interrupt.h> // Biblioteca para interrupciones

// Variable global para almacenar el dato recibido vía SPI
volatile uint16_t DatoRecibido = 0;

// Función para inicializar SPI en modo esclavo
void SPI_InicializarEsclavo(void) {
    DDRB = (1 << DDB4);  // Configura MISO (PB4) como salida

    SPCR = (1 << SPE);   // Habilita SPI

    SPCR |= (1 << SPIE); // Habilita interrupciones SPI
    sei();               // Habilita interrupciones globales
}

// Interrupción para recibir datos SPI
ISR(SPI_STC_vect) {
    DatoRecibido = SPDR; // Almacena el dato recibido en la variable global
}

// Función para enviar un dato por UART
void EnviarUART(unsigned char dato) {
    while (!(UCSR0A & (1 << UDRE0))); // Espera a que el registro esté listo para transmitir
    UDR0 = dato;                      // Carga el dato en el registro de transmisión
}

// Función para enviar una cadena por UART
void EnviarCadenaUART(const char *str) {
    while (*str) {
        EnviarUART(*str++); // Envía cada carácter de la cadena
    }
}

// Inicializa el PWM en el Timer0
void inicializarPwm() {
    TCCR0A = (1 << COM0A1) | (1 << WGM00); // Modo PWM, fase correcta, canal OC0A habilitado
    TCCR0B = (1 << CS01);                  // Prescaler de 8
}

int main(void) {
    // Configuración de pines
    DDRD |= (1 << PD7) | (1 << PD6) | (1 << PD5) | (1 << PD4) | (1 << PD3); // Configura PD7-PD3 como salida
    PORTD |= (1 << PD3); // PD3 a nivel alto (sentido horario)

    inicializarPwm();          // Inicializa PWM
    SPI_InicializarEsclavo();  // Inicializa SPI como esclavo

    OCR0A = 0;                 // Inicializa PWM con 0% de duty cycle

    // Configuración UART
    UBRR0H = 0;                // Baud rate alto
    UBRR0L = 103;              // Baud rate bajo para 9600bps (a 16MHz)
    UCSR0B = (1 << TXEN0);     // Habilita transmisión UART
    UCSR0C = (1 << UCSZ01) | (1 << UCSZ00); // Configura UART para 8 bits, sin paridad, 1 bit de parada

    _delay_ms(1000);           // Espera inicial para estabilización

    // Variables auxiliares
    char cadena2[50];           // Buffer para cadenas UART
    int datoAnterior = 0;      // Almacena el último dato recibido para evitar repeticiones
    int datoTemperatura, datoHumedad; // Variables para temperatura y humedad

    while (1) {
        // Bucle principal
        for (int i = 0; i < 4; i++) {
            while (datoAnterior == DatoRecibido); // Espera a que el dato recibido sea nuevo
            datoAnterior = DatoRecibido;         // Actualiza el dato anterior

            // Procesa los datos según el índice
            switch (i) {
                case 0: // Caso: Potenciómetro
                    OCR0A = DatoRecibido; // Ajusta el PWM al valor recibido
                    snprintf(cadena2, sizeof(cadena2), "POT:%d\r\n", DatoRecibido);
                    EnviarCadenaUART(cadena2); // Envía el valor por UART
                    break;

                case 1: // Caso: Botón
                    snprintf(cadena2, sizeof(cadena2), "Boton:%d\r\n", DatoRecibido);
                    EnviarCadenaUART(cadena2); // Envía el estado del botón por UART
                    if (DatoRecibido == 1) {
                        PORTD |= (1 << PD7);  // Enciende PD7 si el botón está presionado
                    } else {
                        PORTD &= ~(1 << PD7); // Apaga PD7 si el botón no está presionado
                    }
                    break;

                case 2: // Caso: Humedad
                    datoHumedad = DatoRecibido; // Almacena el dato de humedad
                    snprintf(cadena2, sizeof(cadena2), "Humedad:%d\r\n", DatoRecibido);
                    EnviarCadenaUART(cadena2); // Envía el valor de humedad por UART
                    break;

                case 3: // Caso: Temperatura
                    datoTemperatura = DatoRecibido; // Almacena el dato de temperatura
                    // Controla los LEDs según los valores de humedad y temperatura
                    if (datoHumedad > datoTemperatura) {
                        PORTD |= (1 << PD4);  // Enciende PD4 
                        PORTD &= ~(1 << PD5); // Apaga PD5 
                    } else if (datoTemperatura > datoHumedad) {
                        PORTD |= (1 << PD5);  // Enciende PD5 
                        PORTD &= ~(1 << PD4); // Apaga PD4 
                    } else {
                        PORTD &= ~(1 << PD4); // Apaga PD4 y PD5
                        PORTD &= ~(1 << PD5);
                    }
                    snprintf(cadena2, sizeof(cadena2), "Temperatura:%d\r\n", DatoRecibido);
                    EnviarCadenaUART(cadena2); // Envía el valor de temperatura por UART
                    break;
            }
        }
    }
}
