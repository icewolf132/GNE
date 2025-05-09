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

#define    V_SUP        RB6 //input modulo di temperatura tensione di soglia superiore
#define    V_INF        RB5 //input modulo di temperatura tensione di soglia inferiore

#define    T_STOP       RB0 //pulsante di stop per irrigazione con interrupt


//USCITE

//controllo motore
#define    POWER_MOT    RD4 //ingresso inverter per accensione e spegnimento del motore
#define    VEL_MOT      RD3 //ingresso inverter per impostare la velocitÃ 
#define    DIR_MOT      RD2 //ingresso inverter per impostare la direzione

 //tabella di input dell'inverter
#define    START        1
#define    STOP         0
#define    AVANTI       1
#define    DIETRO       0
#define    LENTO        0
#define    VELOCE       1

//controllo relÃ¨
#define    RELE_ACQUA      RD7
#define    VENTILATORI     RD6
#define    FINESTRE        RD5

#define    ECCITATO        1
#define    DISECCITATO     0

//visualizzazioni display 7 segmenti 
#define    ON    0
#define    OFF   1
#define    FC    2
#define    EN    3 
#define    NULLA 4 

//variabili globali
int stato=0;
int stop=0;

//funzioni
void Irrigazione(int power, int direzione, int velocita, int irrigazione)
{
    //controllo direzione del motore fino a finecorsa
    if(power == START) POWER_MOT = START;
    if(power == STOP) POWER_MOT = STOP;
    
    //controllo direzione del motore fino a finecorsa
    if(direzione == AVANTI) DIR_MOT = AVANTI;
    if(direzione == DIETRO) DIR_MOT = DIETRO;
        
    //controllo velocitÃ  motore
    if(velocita == LENTO) VEL_MOT = LENTO;
    if(velocita == VELOCE) VEL_MOT = VELOCE;
    
    //controllo relÃ¨ di irrigazione
    if(irrigazione == ECCITATO ) RELE_ACQUA = ECCITATO;
    if(irrigazione == DISECCITATO) RELE_ACQUA = DISECCITATO;
}

void Visualizza (int display)
{
    PORTC =0b11111111;
    unsigned int lettera[5] = {0b00111111, 0b01110001, 0b01111001, 0b01010100, 0b01011000}; // O, F, E, n, c
    
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
        PORTC =lettera[1];// lettera[1];    //visualizzo la lettera F   
        PORTA =lettera[4];// lettera[4];    //visualizzo la lettera c
    }
    
    if(display == EN )
    {
        PORTC = lettera[2];     //visualizzo la lettera E
        PORTA = lettera[3];    //visualizzo la lettera n
    }
    
    if(display == NULLA )
    {
        PORTC = 0b00111111;     //visualizzo la lettera 0
        PORTA = lettera[0];     //visualizzo la lettera 0
    }
     
}

void Stop (void)
{
    Irrigazione(STOP, DIETRO, LENTO, DISECCITATO); // Ferma tutto
    Visualizza(OFF); //visualizza of
    stop=1;
    __delay_ms(5000); // Aspetta 5 secondi
    if (RB0 == 0) // Se ancora premuto
    {
        while(FC1==1)
        {
            Irrigazione(START, DIETRO, VELOCE, DISECCITATO); // Torna indietro 
        }
        Irrigazione(STOP, DIETRO, LENTO, DISECCITATO);
        stato = 0;
    }
    else stato = 0;
}

void main(void) 
{
    OSCCON |=0x60;       //Clock 4 Mhz
    PORTA = 0x00;   //7SEG 2
    PORTB = 0x00;   //IN
    PORTC = 0x00;   //7SEG 1
    PORTD = 0x00;   //OUT
    //PORTE = 0x00;   
    
    TRISA = 0b00000000;             //Imposto i pin di PORTA 
    TRISB = 0b11111111;             //Imposto i pin di PORTB
    TRISC = 0b00000000;             //Imposto i pin di PORTC 
    TRISD = 0b00000000;             //Imposto i pin di PORTD 
    
    INTCON = 0b10010000;

    ANSEL = 0x00;                   //Imposto tutti i pin come ingressi digitali
    ANSELH = 0x00;                  //Imposto tutti i pin come ingressi digitali
    
    OPTION_REG = 0b00000011;    //  pull-up PORTB,  Fosc/4,  PS=32
    //INTCON = 0b10100000;        // Impostazione Interrupt su TMR0  GIE=T0IE=1
    //TMR0 = 6;                   // Imposto Timer0 per un conteggio pari a 250
   
    
    Visualizza(NULLA); //visualizza of
    Irrigazione(STOP, DIETRO, LENTO, DISECCITATO); // Fermo tutto
    stop =0;
    
    while (1) //Ciclo infinito
    {      
         if (FC1 == 0 && FC2 == 0)
        {
            Visualizza(FC);  // Allarme finecorsa
            Irrigazione(STOP, DIETRO, LENTO, DISECCITATO); // Fermo tutto
            stato=0;
        }

        switch(stato)
        {
            case 0: //non sto fecendo nulla 
                if(stop==1)Visualizza(OFF);
                Visualizza(NULLA);
                if(T_START == 0) //nel caso il tasto start Ã¨ premuto
                {
                    stop=0;
                    Visualizza(ON); //visualizza on
                    stato=1; //vai al primo stato
                }
                
                if (V_SUP ==1) //se la temperatura Ã¨ sopra i 30Â°C
                { 
                    //stato=10; //vai allo stato 10
                }
                break;
                
            case 1: //irrigatore non si sa dove sta 
                if(FC1==1) //se l'irrigatore non si trova a inizio corsa
                {
                    Irrigazione(START, DIETRO, VELOCE, DISECCITATO); //fai andare il motore alla massima velocitÃ  indietro fino aquando
                    stato=2; //passa allo stato due
                }
                else if(FC1==0)stato=3; //se l'irrigatore si trova a inizio corsa vai allo stato 3
                break;
                
            case 2: //irrigatore sta tornando indietro
                if(FC1==0)stato=3; //se l'irrigatore si trova a inizio corsa vai allo stato 3
                break;
            
            case 3: //irrigatore sta a inizio corsa
                Irrigazione(START, AVANTI, LENTO, ECCITATO);
                stato=4;
                break;
                
            case 4: //irrigatore sta andando avanti alla minima velocitÃ  irrigando
                if (FC2==0) stato=5;
                break;
                
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
                break;
                
            case 6: //irrigatore sta tornando indietro alla massima velocitÃ  non irrigando
                if (FC1==0)
                {  
                    Irrigazione(STOP,DIETRO, LENTO, DISECCITATO);
                    stato=8;
                }
                break;
                
            case 7: //irrigatore sta tornando indietro alla minima velocitÃ  irrigando
                if (FC1==0)
                {  
                    Irrigazione(STOP,DIETRO, LENTO, DISECCITATO);
                    stato=8;
                }
                break;
                
            case 8: //irrigatore sta ad inizio corsa
                stato=0;
                break;
                    
            case 10: //temperatura sopra i 30Â°C
                FINESTRE    = 1;
                stato=20;
                break;
                
            case 20: //temperatura sopra i 30Â°C
                __delay_ms(3000);
                VENTILATORI = 1;
                stato=30;
                break;
                
            case 30:
                if (V_SUP == 0) //se la temperatura e'sotto i 27C
                { 
                    VENTILATORI = 0;
                    FINESTRE    = 0;
                    stato=40; //vai allo stato 11
                }
                
            case 40: //temperatura sotto i 27C
                stato=0;
                break;
          
        }
    }
}

void __interrupt() ISR(void)
{
    if (INTCONbits.INTE && INTCONbits.INTF)
    {
        INTCONbits.INTF = 0; // reset flag
        Stop();
    
        
    }
}