// commit per github pls



#pragma config FOSC = INTRC_NOCLKOUT //XT        // Oscillator Selection Bits (XT Oscillator, Crystal/resonator connected between OSC1 and OSC2 pins)
#pragma config WDTE = OFF       // Watchdog Timer Enable bit (WDT disabled and can be enabled by SWDTEN bit of the WDTCON register)
#pragma config PWRTE = OFF      // Power-up Timer Enable bit (PWRT disabled)
#pragma config MCLRE = OFF       // RE3/MCLR pin function select bit (RE3/MCLR pin function is MCLR)
#pragma config CP = OFF         // Code Protection bit (Program memory code protection is disabled)
#pragma config CPD = OFF        // Data Code Protection bit (Data memory code protection is disabled)
#pragma config BOREN = OFF      // Brown Out Reset Selection bits (BOR disabled)
#pragma config IESO = OFF       // Internal External Switchover bit (Internal/External Switchover mode is disabled)
#pragma config FCMEN = OFF      // Fail-Safe Clock Monitor Enabled bit (Fail-Safe Clock Monitor is disabled)
#pragma config LVP = OFF        // Low Voltage Programming Enable bit (RB3 pin has digital I/O, HV on MCLR must be used for programming)

// CONFIG2
#pragma config BOR4V = BOR40V   // Brown-out Reset Selection bit (Brown-out Reset set to 4.0V)
#pragma config WRT = OFF        // Flash Program Memory Self Write Enable bits (Write protection off)

// #pragma config statements should precede project file includes.
// Use project enums instead of #define for ON and OFF.

#include <xc.h>                    //Header file generico

#define _XTAL_FREQ 4000000         //Specifico la frequenza dell'oscillatore



//INGRESSI

#define    FC1          RB3 //pulsante di finecorsa inizio
#define    FC2          RB2 //pulsante di finecorsa fine

#define    T_START      RB1 //pulsante di inizio irigamento
#define    MODO         RB4 //interrutore di selezione di ritorno

#define    TEMP         RB4 //input modulo di temperatura

#define    T_STOP       RB0 //pulsante di stop per irrigazione con interrupt


//USCITE

//controllo motore
#define    POWER_MOT    RD4 //ingresso inverter per accensione e spegnimento del motore
#define    VEL_MOT      RD3 //ingresso inverter per impostare la velocit�
#define    DIR_MOT      RD2 //ingresso inverter per impostare la direzione

 //tabella di input dell'inverter
#define    START        1
#define    STOP         0
#define    AVANTI       1
#define    DIETRO       0
#define    LENTO        0
#define    VELOCE       1

//controllo rel�
#define    RELE_ACQUA      RD7
#define    VENTILATORI     RD6
#define    FINESTRE        RD5

#define    ECCITATO        1
#define    DISECCITATO     0

//visualizzazioni display 7 segmenti 
#define    ON   0
#define    OFF  1
#define    FC   2
#define    EN   3 

//char conta;

void Irrigazione(int power, int direzione, int velocita, int irrigazione)
{
    //controllo direzione del motore fino a finecorsa
    if(power == START) POWER_MOT = START;
    if(power == STOP) POWER_MOT = STOP;
    
    //controllo direzione del motore fino a finecorsa
    if(direzione == AVANTI) DIR_MOT = AVANTI;
    if(direzione == DIETRO) DIR_MOT = DIETRO;
        
    //controllo velocit� motore
    if(velocita == LENTO) VEL_MOT = LENTO;
    if(velocita == VELOCE) VEL_MOT = VELOCE;
    
    //controllo rel� di irrigazione
    if(irrigazione == ECCITATO ) RELE_ACQUA = ECCITATO;
    if(irrigazione == DISECCITATO) RELE_ACQUA = DISECCITATO;
}

void Visualizza (int display)
{
    int lettera[5] = {0b01111110, 0b11100010, 0b11110010, 0b10101000, 0b10110000}; // O, F, E, n, c
    
    if(display == OFF )
    {
        PORTC = lettera[0];    //visualizzo la lettera O
        PORTA = lettera[1];    //visualizzo la lettera f
    }
    
    if(display == ON )
    {
        PORTC = lettera[0];    //visualizzo la lettera O   
        PORTA = lettera[3];    //visualizzo la lettera n
    }
    
    if(display == FC )
    {
        PORTC = lettera[1];    //visualizzo la lettera F   
        PORTA = lettera[4];    //visualizzo la lettera c
    }
    
    if(display == EN )
    {
        PORTC = lettera[2];     //visualizzo la lettera E
        PORTA = lettera[3];    //visualizzo la lettera n
    }
}
  
int stato=0;

void main(void) 
{
//    OSCCON |=0x60;       //Clock 4 Mhz
    PORTA = 0x00;   //7SEG 2
    PORTB = 0x00;   //IN
    PORTC = 0x00;   //7SEG 1
    PORTD = 0x00;   //OUT
    //PORTE = 0x00;   
    
    TRISA = 0b00000000;             //Imposto i pin di PORTA 
    TRISB = 0b11111111;             //Imposto i pin di PORTB
    TRISC = 0b00000000;             //Imposto i pin di PORTC 
    TRISD = 0b00000000;             //Imposto i pin di PORTD 
    //TRISE = 0b00000000;             //Imposto i pin di PORTE 
    
    INTCON = 0b1011100;

    ANSEL = 0x00;                   //Imposto tutti i pin come ingressi digitali
    ANSELH = 0x00;                  //Imposto tutti i pin come ingressi digitali
    
    OPTION_REG = 0b00000011;    //  pull-up PORTB,  Fosc/4,  PS=32
    //INTCON = 0b10100000;        // Impostazione Interrupt su TMR0  GIE=T0IE=1
    //TMR0 = 6;                   // Imposto Timer0 per un conteggio pari a 250
    
    
    while (1) //Ciclo infinito
    {                       
          stato=0;
          
        switch(stato)
        {
            case 0: //non sto fecendo nulla 
                if(T_START == 0) //nel caso il tasto start � premuto
                {
                    PORTA=0b11111111;//Visualizza(ON); //visualizza on
                    PORTC=0b11111111;
                    stato=1; //vai al primo stato
                    while(1);
                }
                
                if (TEMP ==1) //se la temperatura � sopra i 30�C
                { 
                    stato=10; //vai allo stato 10
                }
                
            case 1: //irrigatore non si sa dove sta 
                if(FC1==1) //se l'irrigatore non si trova a inizio corsa
                {
                    Visualizza(FC); //visualizza on
                    Irrigazione(START, DIETRO, VELOCE, DISECCITATO); //fai andare il motore alla massima velocit� indietro fino aquando
                    stato=2; //passa allo stato due
                }
                else if(FC1==0)stato=3; //se l'irrigatore si trova a inizio corsa vai allo stato 3
                break;
                
            case 2: //irrigatore sta tornando indietro
                if(FC1==0)stato=3; //se l'irrigatore si trova a inizio corsa vai allo stato 3
            
            case 3: //irrigatore sta a inizio corsa
                Irrigazione(START, AVANTI, LENTO, ECCITATO);
                stato=0;
                
            case 4: //irrigatore sta andando avanti alla minima velocit� irrigando
                if (FC2==0) stato=0;
                
            case 5: //irrigatore sta  a fine corsa
                if (MODO==0) 
                { 
                    Irrigazione(START, DIETRO, VELOCE, DISECCITATO);
                    stato=6;
                }
                else if (MODO==1) 
                { 
                    Irrigazione(START,DIETRO, LENTO, ECCITATO);
                    stato=7;
                }
                
            case 6: //irrigatore sta tornando indietro alla massima velocit� non irrigando
                if (FC1==0)
                {  
                    //ferma motore;
                    stato=8;
                }
                
            case 7: //irrigatore sta tornando indietro alla minima velocit� irrigando
                if (FC1==0)
                {  
                    //ferma motore;
                    stato=8;
                }
            case 8: //irrigatore sta ad inizio corsa
                    stato=0;
                    
            
        }
    }

}


/*void __interrupt ISR(void) // Interrupt Service Routine
{
    if (INTCONbits.INTE && INTCONbits.INTF) // verifico che l'interrupt � stato causato dal pin esterno
    {
        Irrigazione(STOP,DIETRO, LENTO, ECCITATO); //   fermo l'irrigatore
        __delay_ms(5000); //aspetto 5 secondi
        if(RB0==1) //se il tasto � ancora premuto
        {
            Irrigazione(START, DIETRO, VELOCE, DISECCITATO); //fai andare l'irrigatore alla massima velocit� indietro 
        }
        stato=0;
    }
 } 
*/