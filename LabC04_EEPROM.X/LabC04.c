/* Archivo: LabC04.c
 * Dispositivo: PIC16F887
 * Autor: Kevin Alarcón
 * Compilador: XC8(v2.40), MPLABX V6.05
 * 
 * 
 * Programa: Crear un menú en la terminal que pueda presentar el valor de un potenciometro 
 * Hardware: Potenciómetro en RA0 y una terminal conectada
 * 
 * Creado: 21 de abril, 2023
 * Última modificación: 25 de abril, 2023
 */

// CONFIG1
#pragma config FOSC = INTRC_NOCLKOUT// Oscillator Selection bits (INTOSCIO oscillator: I/O function on RA6/OSC2/CLKOUT pin, I/O function on RA7/OSC1/CLKIN)
#pragma config WDTE = OFF       // Watchdog Timer Enable bit (WDT disabled and can be enabled by SWDTEN bit of the WDTCON register)
#pragma config PWRTE = OFF      // Power-up Timer Enable bit (PWRT disabled)
#pragma config MCLRE = OFF      // RE3/MCLR pin function select bit (RE3/MCLR pin function is digital input, MCLR internally tied to VDD)
#pragma config CP = OFF         // Code Protection bit (Program memory code protection is disabled)
#pragma config CPD = OFF        // Data Code Protection bit (Data memory code protection is disabled)
#pragma config BOREN = OFF      // Brown Out Reset Selection bits (BOR disabled)
#pragma config IESO = OFF       // Internal External Switchover bit (Internal/External Switchover mode is disabled)
#pragma config FCMEN = OFF      // Fail-Safe Clock Monitor Enabled bit (Fail-Safe Clock Monitor is disabled)
#pragma config LVP = OFF        // Low Voltage Programming Enable bit (RB3 pin has digital I/O, HV on MCLR must be used for programming)

// CONFIG2
#pragma config BOR4V = BOR40V   // Brown-out Reset Selection bit (Brown-out Reset set to 4.0V)
#pragma config WRT = OFF        // Flash Program Memory Self Write Enable bits (Write protection off)

/*******************************LIBRERIAS***********************************/
#include <stdio.h>
#include <xc.h>
#include <stdint.h>
#include <stdlib.h>
#include <pic16f887.h>

/******************************DEFINIR CONSTANTES***************************/
#define _XTAL_FREQ 1000000

//---------------------Variables---------------------------------
int contador = 0;
uint8_t  address = 0, data = 0, Pot = 0;

//-------------------Prototipos de funciones------------------
void setup(void);
void write_EEPROM (uint8_t  address, uint8_t  data);
uint8_t  read_EEPROM(uint8_t  address);

//----------------------Interrupciones---------------------------
void __interrupt() isr(void) {
    if (PIR1bits.ADIF) //Revisamos si la bandera de interrupción del ADC se enciende 
    {    
        Pot = ADRESH; //Ingresamos a la variable Pot el valor del potenciometro en RE0
        PORTC = Pot; //Ingresamos el puerto C el valor de la variable Pot
        contador = 0; //Colocamos nuestra bandera de Sleep en 0
        PIR1bits.ADIF = 0; //Limpiar la bandera de la interrupcion del ADC
    } 
    if (INTCONbits.RBIF) //Revisamos si la bandera de interrupción del Puerto B se enciende
    {
        //RB0 -> Aumentar la dirección de memoria
        if (PORTBbits.RB0 == 0){
            address++; //Aumentamos nuestra variable de localidad
            PORTA = 0; //Limpiamos el puerto A
            contador = 0; //Colocamos nuestra bandera de Sleep en 0
        }
        //RB1 -> Disminuir la dirección de memoria
        else if (PORTBbits.RB1 == 0){
            address--; //Disminuimos nuestra variable de localidad
            contador = 0; //Colocamos nuestra bandera de Sleep en 0
        }
        //RB2 -> Escribir en la EEPROM
        else if (PORTBbits.RB2 == 0){
            write_EEPROM (address, Pot); //Mandamos a llamar a nuestra función de escritra en la EEPROM
            contador = 0; //Colocamos nuestra bandera de Sleep en 0
        }
        //RB3 -> Leer le EEPROM en el Puerto D
        else if (PORTBbits.RB3 == 0){
            PORTD = 0; //Limpiamos el puerto D
            data = read_EEPROM(address); //Mandamos a llamar a nuestra función de lectura en la EEPROM
            PORTD = data; //Ingresamos el valor tomado de la EEPROM en el puerto D
            contador = 0; //Colocamos nuestra bandera de Sleep en 0
        }
        //RB4 -> Sleep
        else if (PORTBbits.RB4 == 0)
             contador = 1; //Colocamos nuestra bandera de Sleep en 1
             PORTA = address; //Ingresamos en el puerto A el valor de la localidad en la que estamos
             PORTC = Pot; //Ingresamos en el puerto C el valor del potenciómetro
         
        
        INTCONbits.RBIF = 0; //Apagamos la bandera del puerto B
    }
    return;
}

void main(void) {
    setup (); //Mandamos a llamar a nuestra función que configura los puertos e interrupciones
    ADCON0bits.GO = 1; //Activamos la lectura del ADC
    while(1){ //loop forever  
        if (contador == 1){ //Revisamos si nuestra bandera de Sleep está encendida
            SLEEP(); //Mientras esté encendida pondremos al Pic en modo de reposo
        }
        
        if (ADCON0bits.GO == 0) { // Si la lectura del ADC se desactiva
            if(ADCON0bits.CHS == 0b0101) //Revisamos si el canal esta en el AN5
            {
                ADCON0bits.CHS = 0b0101; //Si, sí está cambiamos el ADC al canal AN5 ya que solo tenemos un canal
            }
            __delay_us(1000); //Este es el tiempo que se dará cada vez que se desactiva la lectura
            ADCON0bits.GO = 1; //Activamos la lectura del ADC
        }
    }
    return;
}

void setup(void){
    //definir digitales
    ANSELbits.ANS5 = 1;//Seleccionamos solo los dos pines que utilizaremos como analógicos
    ANSELH = 0; 
    
    //Definimos puertos que serán entradas
    TRISE = 1;
    TRISB = 0b11111111;
    
    //Definimos puertos que serán salidas
    TRISC = 0;
    TRISD = 0;
    TRISA = 0;
    
    //Limpiamos los puertos
    PORTA = 0;
    PORTB = 0;
    PORTC = 0;
    PORTD = 0;
    PORTE = 0;

    OPTION_REGbits.nRBPU = 0; //Habilitamos los PULLUPS
    IOCB = 0b01111111; //Habilitamos las interrupciones al cambiar de estaso el puerto B
    
    /////////Banderas e interrupciones
    INTCONbits.RBIF = 0; //Apagamos la bandera del puerto B
    INTCONbits.RBIE = 1; //Habilitamos la interrupción en el puerto B
    PIR1bits.ADIF = 0; //Apagamos la bandera del ADC
    INTCONbits.PEIE = 1; //Habilitar interrupciones periféricas
    PIE1bits.ADIE = 1; //Habilitar interrupciones del ADC
    INTCONbits.GIE = 1; //Habilitar interrupciones globales
    ADCON0bits.GO = 1; //Activamos la lectura del ADC
    
        //Configuración ADC
    ADCON0bits.CHS = 0b0101; //Elegimos canal RA0 como inicial
    ADCON1bits.VCFG0 = 0; //Voltaje referenciado de 0V
    ADCON1bits.VCFG1 = 0; //Voltaje referenciado de 5V
    ADCON0bits.ADCS = 0b10; // Fosc/32
     
    ADCON1bits.ADFM = 0; //Justificado a la izquierda
    ADCON0bits.ADON = 1;//Encendemos el módulo del ADC
     __delay_ms(1); 
    
    return;
}

uint8_t read_EEPROM(uint8_t address){
    while(WR||RD); //Verifificar si hay algún proceso de escritura
                   //o lectura en proceso
    EEADR = address; //Accedemos a la dirección que queremos leer
    EECON1bits.EEPGD = 0; //Lectura de la EEPROM
    EECON1bits.RD = 1; //Obtenemos el dato correspondiente a la dirección 
    return EEDAT;
}

void write_EEPROM (uint8_t address, uint8_t data){
    uint8_t gieStatus;
    while (WR); //Verificar WR para saber si hay un proceso
                // de escritura en proceso
    EEADR = address; //Dirección de memoria a escribir
    EEDAT = data; //Ingresamos el datos que queremos escribir
    EECON1bits.EEPGD = 0; //Acceso a memoria de datos en la EEPROM
    EECON1bits.WREN = 1; //Habilitamos la escritura en la EEPROM
    gieStatus = GIE;
    INTCONbits.GIE = 0; //Deshabilitamos interrupciones
    EECON2 = 0x55;
    EECON2 = 0xAA;
    EECON1bits.WR = 1; //Iniciamos escritura
    EECON1bits.WREN = 0; //Deshabilitamos la escritura en la EEPROM
    
    INTCONbits.GIE = gieStatus; //Habilitamos las interrupciones
}  