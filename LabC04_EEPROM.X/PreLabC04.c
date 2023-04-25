/* Archivo: PostLab04.c
 * Dispositivo: PIC16F887
 * Autor: Kevin Alarcón
 * Compilador: XC8(v2.40), MPLABX V6.05
 * 
 * 
 * Programa: Crear un menú en la terminal que pueda presentar el valor de un potenciometro 
 * Hardware: Potenciómetro en RA0 y una terminal conectada
 * 
 * Creado: 17 de abril, 2023
 * Última modificación: 20 de abril, 2023
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
#define _XTAL_FREQ 8000000

//---------------------Variables---------------------------------
int contador = 0;
uint8_t  address = 0, data = 0, Pot = 0;



//-------------------Prototipos de funciones------------------
void setup(void);

//----------------------Interrupciones---------------------------
void __interrupt() isr(void) {
    if (PIR1bits.ADIF)
    {    
        Pot = ADRESH;
        PORTC = Pot;
        PIR1bits.ADIF = 0; //Limpiar la bandera de la interrupcion del ADC
    } 
    if (INTCONbits.RBIF)
    {
        //RB4 -> Sleep
        if (PORTBbits.RB4 == 0){
             SLEEP();
             PORTC = Pot;
         
        }
        INTCONbits.RBIF = 0; //Apagamos la bandera del puerto B
    }
    return;
}

void main(void) {
    setup (); 
    ADCON0bits.GO = 1; //Activamos la lectura del ADC
    while(1){ //loop forever  
        PORTE = contador;
        contador++;
        /*if (contador > 7){
            contador = 0;
            __delay_us(1000); //Este es el tiempo que se dará cada vez que se desactiva la lectura
        }*/
        //PORTC = ADRESH;
        if (ADCON0bits.GO == 0) { // Si la lectura del ADC se desactiva
            if(ADCON0bits.CHS == 0b0101) //Revisamos si el canal esta en el AN0
            {
                ADCON0bits.CHS = 0b0101; //Si, sí está cambiamos el ADC al canal AN0
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
    
    //////////////oscilador
    OSCCONbits.IRCF = 0b111 ; ///8Mhz
    OSCCONbits.SCS = 1; //Utilizar oscilados interno
    

    /*/////////////// tmr0
    OPTION_REGbits.T0CS = 0; //Usar Timer0 con Fosc/4
    OPTION_REGbits.PSA = 0; //Prescaler con el Timer0
    OPTION_REGbits.PS2 = 1; //Prescaler de 256
    OPTION_REGbits.PS1 = 1;
    OPTION_REGbits.PS0 = 1;  */
    
    //INTCONbits.T0IF = 0; //Limpiamos la bandera del TMR0
    //TMR0 = _tmr0_value; //Restablecemos el valor del TMR0
    
    OPTION_REGbits.nRBPU = 0;
    //WPUB = 1;
    IOCB = 0b01111111;
    
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
