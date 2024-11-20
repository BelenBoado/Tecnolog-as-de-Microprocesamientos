#include <avr/io.h>     // Biblioteca para operaciones de entrada/salida en AVR
#include <util/delay.h> // Biblioteca para retardos
#include <stdio.h>      // Biblioteca estándar para funciones de entrada/salida

// Definiciones de pines
#define DHT11_PIN PD0          // Pin conectado al sensor DHT11
#define BOTON_PIN PD1          // Pin para el botón
#define LCD_RS PB0             // Pin de Registro de Selección (RS) del LCD
#define LCD_EN PB1             // Pin de Enable del LCD
#define LCD_DATA_PORT PORTD    // Puerto de datos del LCD (PD4 a PD7)
#define LCD_CTRL_PORT PORTB    // Puerto de control del LCD
#define LCD_DATA_DDR DDRD      // Dirección de datos del LCD
#define LCD_CTRL_DDR DDRB      // Dirección de control del LCD

// Configuración de pines del LCD para comunicación en 4 bits
void lcd_inicializar_pins(void) {
    LCD_DATA_DDR |= (1 << PD4) | (1 << PD5) | (1 << PD6) | (1 << PD7); // Configura pines de datos como salida
    LCD_CTRL_DDR |= (1 << LCD_RS) | (1 << LCD_EN);                    // Configura pines de control como salida
}

// Genera un pulso en el pin Enable (EN) del LCD para que acepte datos o comandos
void lcd_pulso_enable(void) {
    LCD_CTRL_PORT |= (1 << LCD_EN); // Activa Enable
    _delay_us(1);                  // Retardo breve para el pulso
    LCD_CTRL_PORT &= ~(1 << LCD_EN); // Desactiva Enable
    _delay_us(100);                // Retardo para procesar
}

// Envía un nibble (4 bits) al LCD
void lcd_enviar_nibble(uint8_t nibble) {
    LCD_DATA_PORT = (LCD_DATA_PORT & 0x0F) | (nibble & 0xF0); // Envía nibble alto a los pines de datos
    lcd_pulso_enable();                                       // Genera pulso para enviar
}

// Envía un comando al LCD
void lcd_comandos(uint8_t cmd) {
    LCD_CTRL_PORT &= ~(1 << LCD_RS); // RS en bajo para indicar comando
    lcd_enviar_nibble(cmd);          // Envía nibble alto del comando
    lcd_enviar_nibble(cmd << 4);     // Envía nibble bajo
    _delay_ms(2);                    // Retardo para procesar comando
}

// Envía un dato al LCD
void lcd_datos(uint8_t dato) {
    LCD_CTRL_PORT |= (1 << LCD_RS);  // RS en alto para indicar dato
    lcd_enviar_nibble(dato);         // Envía nibble alto del dato
    lcd_enviar_nibble(dato << 4);    // Envía nibble bajo
    _delay_ms(2);                    // Retardo para procesar dato
}

// Inicialización del LCD en modo de 4 bits
void lcd_inicializar(void) {
    _delay_ms(50);            // Retardo inicial para encendido del LCD

    // Secuencia de inicialización en modo 4 bits
    lcd_comandos(0x30);       // Primer intento de inicialización
    _delay_ms(5);
    lcd_comandos(0x30);       // Segundo intento
    _delay_us(100);
    lcd_comandos(0x32);       // Configura modo 4 bits
    _delay_ms(5);

    // Configuración final del LCD
    lcd_comandos(0x28);       // Modo 4 bits, 2 líneas, 5x8 caracteres
    lcd_comandos(0x0C);       // Display encendido, sin cursor
    lcd_comandos(0x01);       // Limpia display
    _delay_ms(2);             // Retardo para limpiar
    lcd_comandos(0x06);       // Incremento automático del cursor
}

// Escribe una línea en la primera fila del LCD
void lcd_escribir_linea1(const char *mensaje) {
    lcd_comandos(0x80);       // Dirección inicial de la primera línea
    while (*mensaje) {
        lcd_datos(*mensaje++); // Escribe cada carácter
    }
}

// Escribe una línea en la segunda fila del LCD
void lcd_escribir_linea2(const char *mensaje) {
    lcd_comandos(0xC0);       // Dirección inicial de la segunda línea
    while (*mensaje) {
        lcd_datos(*mensaje++); // Escribe cada carácter
    }
}

// Inicializa el ADC (Convertidor Analógico-Digital)
void InicializarADC() {
    ADMUX = (1 << REFS0); // Usa Vcc como referencia de voltaje
    ADCSRA = (1 << ADEN) | (1 << ADPS2) | (1 << ADPS1); // Habilita ADC, prescaler de 64
}

// Lee un valor ADC de un canal específico
uint16_t LeerADC(uint8_t channel) {
    ADMUX = (ADMUX & 0xF8) | (channel & 0x07); // Selecciona canal
    ADCSRA |= (1 << ADSC);                    // Inicia conversión
    while (ADCSRA & (1 << ADSC));             // Espera a que termine la conversión
    return ADC;                               // Devuelve valor convertido
}

// Envía señal de inicio al DHT11
void DHT11_SenalInicio() {
    DDRD |= (1 << DHT11_PIN);   // Configura pin como salida
    PORTD &= ~(1 << DHT11_PIN); // Lleva el pin a nivel bajo por 18ms
    _delay_ms(18);
    PORTD |= (1 << DHT11_PIN);  // Sube el pin a nivel alto
    _delay_us(40);              // Espera 40us
}

// Verifica la respuesta del DHT11
uint8_t DHT11_Respuesta() {
    DDRD &= ~(1 << DHT11_PIN); // Configura pin como entrada
    uint8_t respuesta = 0;
    while (PIND & (1 << DHT11_PIN)); // Espera a nivel bajo
    if (!(PIND & (1 << DHT11_PIN))) {
        _delay_us(80);
        if (PIND & (1 << DHT11_PIN)) {
            respuesta = 1; // Verifica nivel alto
        }
        while (PIND & (1 << DHT11_PIN)); // Espera a que termine
    }
    return respuesta;
}

// Lee un byte de datos del DHT11
uint8_t DHT11_LeerDatos() {
    uint8_t dato = 0;
    for (int i = 0; i < 8; i++) {
        while (!(PIND & (1 << DHT11_PIN))); // Espera a nivel alto
        _delay_us(50);                     // Espera 50us
        if (PIND & (1 << DHT11_PIN)) {
            dato |= (1 << (7 - i)); // Lee bit alto
        }
        while (PIND & (1 << DHT11_PIN)); // Espera a nivel bajo
    }
    return dato;
}

// Inicializa SPI en modo maestro
void SPI_IniciarMaestro(void) {
    DDRB = (1 << DDB3) | (1 << DDB5) | (1 << DDB2); // Configura pines MOSI, SCK y SS como salida
    PORTB |= (1 << PORTB2);                         // Lleva SS a nivel alto
    SPCR = (1 << SPE) | (1 << MSTR) | (1 << SPR0);  // Habilita SPI, modo maestro, prescaler 16
}

// Envía un dato al esclavo SPI
void SPI_EnviarAEsclavo(uint16_t dato) {
    SPDR = dato;                     // Carga dato en el registro de datos
    while (!(SPSR & (1 << SPIF)));   // Espera a que la transferencia termine
}

// Programa principal
int main(void) {
    lcd_inicializar_pins();          // Configura pines del LCD
    lcd_inicializar();               // Inicializa el LCD
    lcd_escribir_linea1("Inicializando...");
    SPI_IniciarMaestro();            // Inicializa SPI
    DDRD &= ~(1 << BOTON_PIN);       // Configura pin del botón como entrada
    PORTD &= ~(1 << BOTON_PIN);      // Desactiva resistencia pull-up

    uint16_t humedad_ent, humedad_dec, temp_ent, temp_dec, suma, estadoBoton;
    InicializarADC();                // Inicializa ADC
    _delay_ms(1000);                 // Retardo inicial

    uint16_t valorPot;
    uint16_t datos[4];

    while (1) {
        valorPot = LeerADC(0);       // Lee valor del potenciómetro
        estadoBoton = (PIND & (1 << BOTON_PIN)) ? 1 : 0; // Lee estado del botón

        DHT11_SenalInicio();         // Envía señal de inicio al DHT11
        if (DHT11_Respuesta()) {
            humedad_ent = DHT11_LeerDatos();  // Lee humedad (parte entera)
            humedad_dec = DHT11_LeerDatos();  // Lee humedad (parte decimal)
            temp_ent = DHT11_LeerDatos();     // Lee temperatura (parte entera)
            temp_dec = DHT11_LeerDatos();     // Lee temperatura (parte decimal)
            suma = DHT11_LeerDatos();         // Lee checksum
        }

        datos[0] = valorPot / 4;     // Ajusta escala del potenciómetro
        datos[1] = estadoBoton;      // Guarda estado del botón
        datos[2] = humedad_ent;      // Guarda humedad entera
        datos[3] = temp_ent;         // Guarda temperatura entera

        for (int i = 0; i < 4; i++) {
            int dato = datos[i];
            lcd_inicializar_pins();  // Reinicia configuración de pines
            lcd_inicializar();       // Reinicia LCD

            // Escribe mensajes específicos en el LCD según el dato
            switch (i) {
                case 0:
                    lcd_escribir_linea1("Enviando datos..");
                    lcd_escribir_linea2("Potenciometro");
                    break;
                case 1:
                    lcd_escribir_linea1("Enviando datos..");
                    lcd_escribir_linea2("Estado del boton");
                    break;
                case 2:
                    lcd_escribir_linea1("Enviando datos..");
                    lcd_escribir_linea2("Humedad");
                    break;
                case 3:
                    lcd_escribir_linea1("Enviando datos..");
                    lcd_escribir_linea2("Temperatura");
                    break;
            }

            PORTB &= ~(1 << PORTB2); // Baja SS para habilitar esclavo
            if (i != 1 && dato <= 2) {
                SPI_EnviarAEsclavo(2); // Envía valor mínimo si el dato es crítico
            } else {
                SPI_EnviarAEsclavo(dato); // Envía el dato
            }
            PORTB |= (1 << PORTB2); // Sube SS para deshabilitar esclavo

            _delay_ms(100); // Espera antes de procesar el siguiente dato
        }
    }
}
