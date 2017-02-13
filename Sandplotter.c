/******************************************************************************\
 * File:        Motor.c                                                       *
 * Author:      Claudio Pascale, Deniz Ali                                    *
 * Target:      Explorer16-Board, dsPIC33FJ64GS610                            *
 * Date:        18. Februar 2016                                              *
 * Description: Schrittmotor über das Explorerboard16 unter MPLAB X ansteuern *
 \*****************************************************************************/

/******************************************************************************\
 * Beschreibung:                                                              *
 * Auf dem Explorer16 Board wird über die Tasten S3, S4, S5 und S6 der        *
 * Sandplotter gesteuert. Mit der Taste S3 wird das jeweils ausgewählte       *
 * Programm (HsKa-Logo, Spirale und Quadratische Spirale) gestartet oder      *
 * gestoppt. Zusätzlich sind noch Sensoren angebracht (Öffner), die bei       *
 * Betätigung ebenfalls das Programm abbrechen, um Motor und Schiene zu       *
 * schützen. Mit S6 wird das auszuführende Programm gewählt, bei Betätigung   *
 * von S5 wird die Fläche "gelöscht" und bei S4 der Plotter zurück gesetzt.   *                                                    *                                                                 
\******************************************************************************/

/******************************************************************************\
 * Beschreibung für HsKa_Logo, QUADRATISCHE_SPIRALE, SPIRALE und radieren/löschen:          *
 *                                                                                          *
 * int x[] gibt an wie viele Werte in dem Array Positionsanfahrt_y enthalten sind.          *
 * Ist zum Beispiel x[5] = 133, so sind in Positionsanfahrt_y [5][133] Werte enthalten.     *
 *                                                                                          *
 * x_richtung[...] gibt an, alle wie viele Schritte ein neuer y-Wert berechnet wurde.       *
 * x_richtung[...] entspricht delta X (wobei das Vorzeichen die Richtung angibt).           *
 * Positionsanfahrt_y[][...] entspricht delta Y (wobei das Vorzeichen die Richtung angibt). *
 *                                                                                          *
 * Für den Fall das delta X == delta Y sind beide Motoren gleich schnell.                   *
 * Ist jedoch delta X größer als delta Y ist die Differenz bei delta X doppelt so           *
 * schnell wie bei delta Y.                                                                 *
 * z.B. delta X = 50; delta Y = 40                                                          *
 * delta X ist 10 Durchläufe doppelt so schnell, die restlichen 30 Durchläufe sind beide    *
 * gleich schnell, um einen dynamischen Ablauf zu gewärleisten.                             *
 *                                                                                          *
 * DIR gibt die Richtung an in die sich der Motor drehen soll ( 1 = Rechtsbewegung;         *
 * 0 = Linksbewegung).                                                                      *
 * PUL = 1 (ist ein Schritt des Motors).                                                    *
 * Schnell (bei 1 doppelte Geschwindigkeit).                                                *
 *                                                                                          *
 * (X_PUL, X_DIR, Y_PUL, Y_DIR, X_Schnell, Y_Schnell) diese Werte werden an die             *
 * Funktion FAHREN übergeben.                                                               *
\******************************************************************************/


/***Header-Dateien*************************************************************/
#include "xc.h"
#include <stdio.h>
/******************************************************************************/

/***CONFIG*********************************************************************/
// CONFIG2
#pragma config POSCMOD = XT             // Primary Oscillator Select (XT Oscillator mode selected)
#pragma config OSCIOFNC = OFF           // Primary Oscillator Output Function (OSC2/CLKO/RC15 functions as CLKO (FOSC/2))
#pragma config FCKSM = CSDCMD           // Clock Switching and Monitor (Clock switching and Fail-Safe Clock Monitor are disabled)
#pragma config FNOSC = PRI              // Oscillator Select (Primary Oscillator (XT, HS, EC))
#pragma config IESO = OFF               // Internal External Switch Over Mode (IESO mode (Two-Speed Start-up) disabled
// CONFIG1
#pragma config WDTPS = PS32768          // Watchdog Timer Postscaler (1:32,768)
#pragma config FWPSA = PR128            // WDT Prescaler (Prescaler ratio of 1:128)
#pragma config WINDIS = OFF             // Watchdog Timer Window (Windowed Watchdog Timer enabled; FWDTEN must be 1)
#pragma config FWDTEN = OFF             // Watchdog Timer Enable (Watchdog Timer is disabled)
#pragma config ICS = PGx2               // Comm Channel Select (Emulator/debugger uses EMUC2/EMUD2)
#pragma config GWRP = OFF               // General Code Segment Write Protect (Writes to program memory are allowed)
#pragma config GCP = OFF                // General Code Segment Code Protect (Code protection is disabled)
#pragma config JTAGEN = OFF             // JTAG Port Enable (JTAG port is disabled)
/******************************************************************************/

/***Prototyp*******************************************************************/ 
int RESET_X_RICHTUNG(void);
int RESET_Y_RICHTUNG(void);
int OFFSET(void);
int RESET (void);
int FAHREN(int Delay, char LED, char X_DIR, char X_PUL, char Y_DIR, char Y_PUL, unsigned int X_Schnell, unsigned int Y_Schnell);
int Radieren(void);
int PROGRAMM(int Muster); // hier werden die programme gestartet 
int HsKa_Logo(void);
int SPIRALE(void); 
int QUADRATISCHE_SPIRALE(void);
void ABSCHALTEN (void);
/******************************************************************************/

/***Programm*******************************************************************/
int main(void) 
{      
    LATA  = 0x0000;             // LEDs auswählen
    TRISA = 0x0000;             // LEDs beim start des Programm´s 
    PORTA = 0x0000;             // auf 0 setzen  
    TRISD = 0x0040;             // Taster auf lesen stellen
    
    ABSCHALTEN();
    while(1)
    {   
        ////////////////////////////////////////////////////////////////////////
        // Tasten Auslesen und Programm Starten
        
        if((PORTD & 0x10) == 0x10) // Reset-Taste   RD4 
        {    
            RESET();
            ABSCHALTEN();
        }
        
/*        if((PORTD & 0x20) == 0x20) // Stopp-Taste   RD5
        {
      
        }
*/       
        if((PORTD & 0x01) == 0x01) // HsKa-Logo    RD0  
        {
            HsKa_Logo();
            ABSCHALTEN();
        }
        
        if((PORTD & 0x02) == 0x02) // Spirale   RD1 
        {
            SPIRALE();
            ABSCHALTEN();
        }
        
        if((PORTD & 0x04) == 0x04) // Quadratische-Spirale      RD2
        {
            QUADRATISCHE_SPIRALE();
            ABSCHALTEN();
        }
        
        if((PORTD & 0x08) == 0x08) // Löschen      RD3
        {
            Radieren();
            ABSCHALTEN();
        }
    } 
    return 0;
}

int RESET(void)
{
    RESET_X_RICHTUNG(); 
    if((PORTD & 0x20) == 0x20) // Stopp-Taste   RD5
    {
        return 0;
    }
    
    RESET_Y_RICHTUNG();
    if((PORTD & 0x20) == 0x20) // Stopp-Taste   RD5
    {
        return 0;
    }
    
    OFFSET();
    return 0;
}
int RESET_X_RICHTUNG(void)
{
    TRISE = 0x0008;
    TRISG = 0x0000;                         // PORTG auf schreiben stellen
    TRISA = 0x0000;                         // PORTA auf schreiben stellen
    LATA  = 0x0000;                         // LED's ansteuern
    TRISB = 0x0000;                         // PORTB auf schreiben stellen
    T1CON = 0x8010;                         // Einstellung Timer 1
    
    while(1)
    {   
        if((PORTE & 0x08) == 0x08)               // Taste Maskieren
        {                                        // Stop Bedingung
            ABSCHALTEN();
            return 0;
        }
        
        if(((PORTD & 0x20) == 0x20))        //Stop mit Taster
        {
            return 0;
        }
        
        PORTG = 0x4000;                     // PUL = 1;
        //PORTA = 0x00FF;                     // LED's an
        PORTB = 0x8080;                     // ENA = 1; DIR = 0;
        TMR1 = 0;                           // Reset Timer 1
        
        while (TMR1 < 500)
        {
            //warten
        }
        
        if((PORTE & 0x08) == 0x08)                 // Taste Maskieren          
        {                                           // Stop Bedingung
            ABSCHALTEN();
            return 0;
        }
        
        if(((PORTD & 0x20) == 0x20))        //Stop mit Taster
        {
            return 0;
        }
        
        PORTG = 0x0000;                     // PUL = 0;
        //PORTA = 0x0000;                     // LED's aus
        PORTB = 0x8000;                     // ENA = 1; DIR = 0;
        TMR1 = 0;                           // Reset Timer 1
        
        while (TMR1 < 500)
        {
            //warten
        } 
    }
    return 0;
}

int RESET_Y_RICHTUNG(void)
{
    TRISE = 0x0004;
    TRISG = 0x0000;                         // PORTG auf schreiben stellen
    TRISA = 0x0000;                         // PORTA auf schreiben stellen
    LATA  = 0x0000;                         // LED's ansteuern
    TRISB = 0x0000;                         // PORTB auf schreiben stellen
    T1CON = 0x8010;                         // Einstellung Timer 1
    
    while(1)
    {   
        if((PORTE & 0x04) == 0x04)                  // Taste Maskieren
        {                                           // Stop Bedingung
            ABSCHALTEN();
            return 0;
        }
        
        if(((PORTD & 0x20) == 0x20))        //Stop mit Taster
        {
            return 0;
        }
        
        PORTG = 0x1000;                     // PUL = 1;
        //PORTA = 0x00FF;                     // LED's an
        PORTB = 0x4040;                     // ENA = 1; ; DIR = 0;
        TMR1 = 0;                           // Reset Timer 1
        
        while (TMR1 < 500)
        {
            //warten
        }
        
        if((PORTE & 0x04) == 0x04)                  // Taste Maskieren
        {                                           // Stop Bedingung
            ABSCHALTEN();
            return 0;
        }
        
        if(((PORTD & 0x20) == 0x20))        //Stop mit Taster
        {
            return 0;
        }
        
        PORTG = 0x0000;                     // PUL = 0;
        //PORTA = 0x0000;                     // LED's aus
        PORTB = 0x4000;                     // ENA = 1; DIR = 0;
        TMR1 = 0;                           // Reset Timer 1
        
        while (TMR1 < 500)
        {
            //warten
        } 
    }   
    return 0;
}

int OFFSET()
{
    int Delay = 1000;
    int i;
    char X_DIR = 0x08;
    char X_PUL = 0x40;
    char Y_DIR = 0x04;
    char Y_PUL = 0x10;
    char LED = 0x00;
    
    unsigned int X_Schnell = 0;
    unsigned int Y_Schnell = 0;
    
    for(i=0; i < 100;i++) 
    {
        FAHREN(Delay, LED, X_DIR, X_PUL, Y_DIR, Y_PUL, X_Schnell, Y_Schnell);
    }
    return 0;
}

int FAHREN(int Delay, char LED, char X_DIR, char X_PUL, char Y_DIR, char Y_PUL, unsigned int X_Schnell, unsigned int Y_Schnell)
{
    int x;
    
    if((X_Schnell || Y_Schnell)==1)
    {
        x = 2;
    }
    else
    {
        x = 1;
    }
    
    T1CON = 0x8010;                         // Einstellung Timer 1
    
    PORTG = 0x0000 | (X_PUL<<8) | (Y_PUL<<8);           // PUL = 1 oder 0
    PORTB = 0xc000 | (X_DIR<<8) | (Y_DIR<<8);           // ENA ist immer 1, DIR = 1 oder 0
    PORTA = LED;                                        // LED ansteuern
    TMR1 = 0;                                           // Reset Timer 1  
    while (TMR1 < (Delay/x))
    {
        //warten
    }
    
    if(X_Schnell == 1)
    {
        PORTG = 0x0000 | (Y_PUL<<8);                    // Y_PUL = 1 oder 0, X_PUL = 0
        PORTB = 0xc000 | (X_DIR<<8) | (Y_DIR<<8);       // ENA ist immer 1, DIR = 1 oder 0
        PORTA = LED;                                        // LED ansteuern
        TMR1 = 0;                                       // Reset Timer 1  
        while (TMR1 < (Delay/2))
        {
            //warten
        }
    }
    if(Y_Schnell == 1)
    {
        PORTG = 0x0000 | (X_PUL<<8);                    // X_PUL = 1 oder 0, Y_PUL = 0
        PORTB = 0xc000 | (X_DIR<<8) | (Y_DIR<<8);       // ENA ist immer 1, DIR = 1 oder 0
        PORTA = LED;                                        // LED ansteuern
        TMR1 = 0;
        while (TMR1 < (Delay/2))
        {
            //warten
        }
    }
    if((X_Schnell == 1)|| (Y_Schnell == 1))
    {
        PORTG = 0x0000 | (X_PUL<<8) | (Y_PUL<<8);       // PUL = 1 oder 0
        PORTB = 0xc000 | (X_DIR<<8) | (Y_DIR<<8);       // ENA ist immer 1, DIR = 1 oder 0
        PORTA = LED;                                        // LED ansteuern
        TMR1 = 0;                                       // Reset Timer 1  
        while (TMR1 < (Delay/2))
        {
            //warten
        }
    }
    
    PORTG = 0x0000;                                     // PUL = 0
    PORTB = 0xc000 | (X_DIR<<8) | (Y_DIR<<8);           // ENA ist immer 1, DIR = 1 oder 0
    PORTA = LED;                                        // LED ansteuern
    TMR1 = 0;                                           // Reset Timer 1 
    while (TMR1 < (Delay/x))
    {
        //warten
    }
    return 0;
}

int Radieren(void)
{
    char X_PUL,X_DIR,Y_PUL,Y_DIR;
    char LED = 0x00;

    int k,i,j;  
    int Delay = 0, Delay_1 = 80, Delay_2 = 1000;
    int Schleife, X_Schleife=0, Y_Schleife=0, X_Schnell=0, Y_Schnell=0;
    int Positionsanfahrt_x;  // ist der Puffer für x_richtung
    int x = 1;
    int x_richtung[113] = { 0,200,0,200,0,200,0,200,0,200,
                            0,200,0,200,0,200,0,200,0,200,
                            0,200,0,200,0,200,0,200,0,200,
                            0,200,0,200,0,200,0,200,0,200,
                            0,200,0,200,0,200,0,200,0,200,
                            0,200,0,200,0,200,0,200,0,200,
                            0,200,0,200,0,200,0,200,0,200,
                            0,200,0,200,0,200,0,200,0,200,
                            0,200,0,200,0,200,0,200,0,200,
                            0,200,0,200,0,200,0,200,0,200,
                            0,200,0,200,0,200,0,200,0,200,
                            0,200,0};
    
    int Positionsanfahrt_y[113][1]={{ 11200},{0},{-11200},{0},{ 11200},{0},{-11200},{0},{ 11200},{0},
                                    {-11200},{0},{ 11200},{0},{-11200},{0},{ 11200},{0},{-11200},{0},
                                    { 11200},{0},{-11200},{0},{ 11200},{0},{-11200},{0},{ 11200},{0},
                                    {-11200},{0},{ 11200},{0},{-11200},{0},{ 11200},{0},{-11200},{0},
                                    { 11200},{0},{-11200},{0},{ 11200},{0},{-11200},{0},{ 11200},{0},
                                    {-11200},{0},{ 11200},{0},{-11200},{0},{ 11200},{0},{-11200},{0},
                                    { 11200},{0},{-11200},{0},{ 11200},{0},{-11200},{0},{ 11200},{0},
                                    {-11200},{0},{ 11200},{0},{-11200},{0},{ 11200},{0},{-11200},{0},
                                    { 11200},{0},{-11200},{0},{ 11200},{0},{-11200},{0},{ 11200},{0},
                                    {-11200},{0},{ 11200},{0},{-11200},{0},{ 11200},{0},{-11200},{0},
                                    { 11200},{0},{-11200},{0},{ 11200},{0},{-11200},{0},{ 11200},{0},
                                    {-11200},{0},{ 11200}}; 
    
    TRISG = 0x0000;                         // PORTG auf schreiben stellen
    TRISA = 0x0000;                         // PORTA auf schreiben stellen
    LATA  = 0x0000;                         // LED's ansteuern
    TRISB = 0x0000;                         // PORTB auf schreiben stellen
    TRISE = 0x00cc;                         // Sensoren
   
    for(k=0;k < 113;k++)
    {
        for(i=0;i < x;i++)
        {
            
            if(x_richtung[k] == 0)  // y-richtung soll schneller fahren als x-richtung
            {
                Delay = Delay_1;    //schneller    
            }
            else
            {
                Delay = Delay_2;    //langsamer
            }
            ///////////////////////////////////////////////////////Bestimmung der Richtung
            if(x_richtung[k]< 0)
            {
                X_DIR = 0x00;
                Positionsanfahrt_x = (-1)*x_richtung[k];
            }
            else
            {
                X_DIR = 0x08;
                Positionsanfahrt_x = x_richtung[k];
            }
            if(Positionsanfahrt_y[k][i] < 0)
            {
                Y_DIR = 0x00; 
                Positionsanfahrt_y[k][i] = (-1)*Positionsanfahrt_y[k][i];
            }
            else
            {
                Y_DIR = 0x04;
            }
            //////////////////////////////////////////////Bestimmung der Wiederholungszahl
            if(Positionsanfahrt_y[k][i] >= Positionsanfahrt_x)
            {
                X_Schleife = 0;
                Y_Schleife = Positionsanfahrt_y[k][i] - Positionsanfahrt_x;
                if(Y_Schleife > Positionsanfahrt_x)
                {
                    Y_Schleife = Positionsanfahrt_x;
                }
                Schleife = Positionsanfahrt_y[k][i] - Y_Schleife;
            }
            else
            {
                Y_Schleife = 0;
                X_Schleife = Positionsanfahrt_x - Positionsanfahrt_y[k][i];
                if(X_Schleife > Positionsanfahrt_x/2)
                {
                    X_Schleife = Positionsanfahrt_x/2;
                    Schleife = Positionsanfahrt_x/2;
                }
                else
                {
                    Schleife = Positionsanfahrt_y[k][i];
                }
            }
        
            for(j=0;j<Schleife;j++)
            {
                if((PORTD & 0x20) == 0x20)                  // Taste Maskieren
                {                                           // Stop Bedingung
                    return 0;
                }
            
                if(j<Positionsanfahrt_x)
                {
                    X_PUL = 0x40;
                }
                else
                {
                    X_PUL = 0x00;
                }
            
                if(j<Positionsanfahrt_y[k][i])
                {
                    Y_PUL = 0x10;
                }
                else
                {
                    Y_PUL = 0x00;
                }
            
                if(j < X_Schleife)
                {
                    X_Schnell = 1;
                }
                else
                {
                    X_Schnell = 0;
                }
            
                if(j < Y_Schleife)
                {
                    Y_Schnell = 1;
                }
                else
                {
                    Y_Schnell = 0;
                }
                if((((PORTE) & 0x40) == 0x40) || (((PORTE) & 0x80) == 0x80) || (((PORTE) & 0x08) == 0x08) || (((PORTE) & 0x04) == 0x04) || ((PORTD & 0x20) == 0x20))              // Taste Maskieren
                {                                           // Stop Bedingung
                    return 0;
                }
                FAHREN(Delay, LED, X_DIR, X_PUL, Y_DIR, Y_PUL, X_Schnell, Y_Schnell);
            }
        } 
    }
    return 0;
}

int PROGRAMM(int Muster) // wird nicht benötigt, ist vom alten programm
{   
    ABSCHALTEN();
    switch(Muster)
    {
        case 0:     HsKa_Logo();
                    break;
                        
        case 1:     SPIRALE();
                    break;
                        
        case 2:     QUADRATISCHE_SPIRALE();
                    break;
                        
        default:    break;  
    }
    ABSCHALTEN();
    return 0;
}

int HsKa_Logo(void)
{
    char X_PUL,X_DIR,Y_PUL,Y_DIR;
    char LED = 0x03;

    int k,i,j;  
    int Delay = 500;
    int Schleife,X_Schleife=0, Y_Schleife=0, X_Schnell=0, Y_Schnell=0;
    int Positionsanfahrt_x;  // ist der Puffer für x_richtung
    int x_richtung[14] = {0,5587,-50,7314,0,-20,-20,50,-10,-15,-5,40,-20,-16};//Schritte für x-richtung
    int x[14] = {1,1,97,1,1,133,150,140,134,164,164,141,118,154};    //Schleifendurchlauf(ist für die Anzahl der Werte im Array Positionsanfahrt_y)
    int Positionsanfahrt_y[14][164] =   {{11200},
                                        {0},
                                        {-85	,	-85	,	-85	,	-85	,	-85	,	-85	,	-85	,	-85	,	-85	,	-85	,	-85	,	-85	,	-85	,	-85	,	-85	,	-85	,	-85	,	-85	,	-85	,	-85	,	-85	,	-85	,	-85	,	-85	,	-85	,	-85	,	-85	,	-85	,	-85	,	-85	,	-85	,	-85	,	-85	,	-85	,	-85	,	-85	,	-85	,	-85	,	-85	,	-85	,	-85	,	-85	,	-85	,	-85	,	-85	,	-85	,	-85	,	-85	,	-85	,	-85	,	-85	,	-85	,	-85	,	-85	,	-85	,	-85	,	-85	,	-85	,	-85	,	-85	,	-85	,	-85	,	-85	,	-85	,	-85	,	-85	,	-85	,	-85	,	-85	,	-85	,	-85	,	-85	,	-85	,	-85	,	-85	,	-85	,	-85	,	-85	,	-85	,	-85	,	-85	,	-85	,	-85	,	-85	,	-85	,	-85	,	-85	,	-85	,	-85	,	-85	,	-85	,	-85	,	-85	,	-85	,	-85	,	-85	,	-85},
                                        {0},
                                        {5227},
                                        {0	,	0	,	0	,	0	,	1	,	1	,	1	,	1	,	1	,	1	,	1	,	1	,	2	,	2	,	2	,	2	,	2	,	2	,	2	,	2	,	3	,	3	,	3	,	3	,	3	,	3	,	3	,	3	,	4	,	4	,	4	,	4	,	4	,	4	,	4	,	5	,	5	,	5	,	5	,	5	,	5	,	5	,	5	,	6	,	6	,	6	,	6	,	6	,	6	,	7	,	7	,	7	,	7	,	7	,	7	,	7	,	8	,	8	,	8	,	8	,	8	,	8	,	9	,	9	,	9	,	9	,	9	,	10	,	10	,	10	,	10	,	10	,	11	,	11	,	11	,	11	,	11	,	12	,	12	,	12	,	12	,	13	,	13	,	13	,	13	,	14	,	14	,	14	,	15	,	15	,	15	,	15	,	16	,	16	,	16	,	17	,	17	,	18	,	18	,	18	,	19	,	19	,	20	,	20	,	21	,	21	,	22	,	22	,	23	,	24	,	24	,	25	,	26	,	27	,	28	,	29	,	30	,	31	,	32	,	33	,	35	,	37	,	39	,	41	,	43	,	46	,	50	,	55	,	61	,	70	,	83	,	109	,	265},
                                        {-60,	-60	,	-60	,	-60	,	-60	,	-60	,	-60	,	-60	,	-60	,	-60	,	-60	,	-60	,	-60	,	-60	,	-60	,	-60	,	-60	,	-60	,	-60	,	-60	,	-60	,	-60	,	-60	,	-60	,	-60	,	-60	,	-60	,	-60	,	-60	,	-60	,	-60	,	-60	,	-60	,	-60	,	-60	,	-60	,	-60	,	-60	,	-60	,	-60	,	-60	,	-60	,	-60	,	-60	,	-60	,	-60	,	-60	,	-60	,	-60	,	-60	,	-60	,	-60	,	-60	,	-60	,	-60	,	-60	,	-60	,	-60	,	-60	,	-60	,	-60	,	-60	,	-60	,	-60	,	-60	,	-60	,	-60	,	-60	,	-60	,	-60	,	-60	,	-60	,	-60	,	-60	,	-60	,	-60	,	-60	,	-60	,	-60	,	-60	,	-60	,	-60	,	-60	,	-60	,	-60	,	-60	,	-60	,	-60	,	-60	,	-60	,	-60	,	-60	,	-60	,	-60	,	-60	,	-60	,	-60	,	-60	,	-60	,	-60	,	-60	,	-60	,	-60	,	-60	,	-60	,	-60	,	-60	,	-60	,	-60	,	-60	,	-60	,	-60	,	-60	,	-60	,	-60	,	-60	,	-60	,	-60	,	-60	,	-60	,	-60	,	-60	,	-60	,	-60	,	-60	,	-60	,	-60	,	-60	,	-60	,	-60	,	-60	,	-60	,	-60	,	-60	,	-60	,	-60	,	-60	,	-60	,	-60	,	-60	,	-60	,	-60	,	-60	,	-60	,	-60	,	-60	,	-60	,	-60	,	-60	,	-60},
                                        {14	,	14	,	14	,	14	,	14	,	14	,	14	,	14	,	14	,	14	,	14	,	14	,	14	,	14	,	14	,	14	,	14	,	14	,	14	,	14	,	14	,	14	,	14	,	14	,	14	,	14	,	14	,	14	,	14	,	14	,	14	,	14	,	14	,	14	,	14	,	14	,	14	,	14	,	14	,	14	,	14	,	14	,	14	,	14	,	14	,	14	,	14	,	14	,	14	,	14	,	14	,	14	,	14	,	14	,	14	,	14	,	14	,	14	,	14	,	14	,	14	,	14	,	14	,	14	,	14	,	14	,	14	,	14	,	14	,	14	,	14	,	14	,	14	,	14	,	14	,	14	,	14	,	14	,	14	,	14	,	14	,	14	,	14	,	14	,	14	,	14	,	14	,	14	,	14	,	14	,	14	,	14	,	14	,	14	,	14	,	14	,	14	,	14	,	14	,	14	,	14	,	14	,	14	,	14	,	14	,	14	,	14	,	14	,	14	,	14	,	14	,	14	,	14	,	14	,	14	,	14	,	14	,	14	,	14	,	14	,	14	,	14	,	14	,	14	,	14	,	14	,	14	,	14	,	14	,	14	,	14	,	14	,	14	,	14	,	14	,	14	,	14	,	14	,	14	,	14},
                                        {37	,	37	,	37	,	37	,	37	,	37	,	37	,	37	,	37	,	37	,	37	,	37	,	37	,	37	,	37	,	37	,	37	,	37	,	37	,	37	,	37	,	37	,	37	,	37	,	37	,	37	,	37	,	37	,	37	,	37	,	37	,	37	,	37	,	37	,	37	,	37	,	37	,	37	,	37	,	37	,	37	,	37	,	37	,	37	,	37	,	37	,	37	,	37	,	37	,	37	,	37	,	37	,	37	,	37	,	37	,	37	,	37	,	37	,	37	,	37	,	37	,	37	,	37	,	37	,	37	,	37	,	37	,	37	,	37	,	37	,	37	,	37	,	37	,	37	,	37	,	37	,	37	,	37	,	37	,	37	,	37	,	37	,	37	,	37	,	37	,	37	,	37	,	37	,	37	,	37	,	37	,	37	,	37	,	37	,	37	,	37	,	37	,	37	,	37	,	37	,	37	,	37	,	37	,	37	,	37	,	37	,	37	,	37	,	37	,	37	,	37	,	37	,	37	,	37	,	37	,	37	,	37	,	37	,	37	,	37	,	37	,	37	,	37	,	37	,	37	,	37	,	37	,	37	,	37	,	37	,	37	,	37	,	37	,	37},
                                        {0	,	0	,	0	,	0	,	0	,	0	,	0	,	0	,	0	,	0	,	0	,	0	,	0	,	0	,	0	,	0	,	1	,	1	,	1	,	1	,	1	,	1	,	1	,	1	,	1	,	1	,	1	,	1	,	1	,	1	,	1	,	1	,	1	,	1	,	1	,	1	,	1	,	1	,	1	,	1	,	1	,	1	,	1	,	1	,	1	,	1	,	1	,	2	,	2	,	2	,	2	,	2	,	2	,	2	,	2	,	2	,	2	,	2	,	2	,	2	,	2	,	2	,	2	,	2	,	2	,	2	,	2	,	2	,	2	,	2	,	2	,	2	,	2	,	3	,	3	,	3	,	3	,	3	,	3	,	3	,	3	,	3	,	3	,	3	,	3	,	3	,	3	,	3	,	3	,	3	,	3	,	3	,	3	,	3	,	4	,	4	,	4	,	4	,	4	,	4	,	4	,	4	,	4	,	4	,	4	,	4	,	4	,	4	,	4	,	4	,	5	,	5	,	5	,	5	,	5	,	5	,	5	,	5	,	5	,	5	,	5	,	6	,	6	,	6	,	6	,	6	,	6	,	6	,	6	,	6	,	7	,	7	,	7	,	7	,	7	,	7	,	7	,	8	,	8	,	8	,	8	,	9	,	9	,	9	,	9	,	10	,	10	,	10	,	11	,	11	,	12	,	12	,	13	,	13	,	14	,	15	,	16	,	17	,	19	,	21	,	24	,	28	,	37	,	90},
                                        {-55,	-55	,	-55	,	-55	,	-55	,	-55	,	-55	,	-55	,	-55	,	-55	,	-55	,	-55	,	-55	,	-55	,	-55	,	-55	,	-55	,	-55	,	-55	,	-55	,	-55	,	-55	,	-55	,	-55	,	-55	,	-55	,	-55	,	-55	,	-55	,	-55	,	-55	,	-55	,	-55	,	-55	,	-55	,	-55	,	-55	,	-55	,	-55	,	-55	,	-55	,	-55	,	-55	,	-55	,	-55	,	-55	,	-55	,	-55	,	-55	,	-55	,	-55	,	-55	,	-55	,	-55	,	-55	,	-55	,	-55	,	-55	,	-55	,	-55	,	-55	,	-55	,	-55	,	-55	,	-55	,	-55	,	-55	,	-55	,	-55	,	-55	,	-55	,	-55	,	-55	,	-55	,	-55	,	-55	,	-55	,	-55	,	-55	,	-55	,	-55	,	-55	,	-55	,	-55	,	-55	,	-55	,	-55	,	-55	,	-55	,	-55	,	-55	,	-55	,	-55	,	-55	,	-55	,	-55	,	-55	,	-55	,	-55	,	-55	,	-55	,	-55	,	-55	,	-55	,	-55	,	-55	,	-55	,	-55	,	-55	,	-55	,	-55	,	-55	,	-55	,	-55	,	-55	,	-55	,	-55	,	-55	,	-55	,	-55	,	-55	,	-55	,	-55	,	-55	,	-55	,	-55	,	-55	,	-55	,	-55	,	-55	,	-55	,	-55	,	-55	,	-55	,	-55	,	-55	,	-55	,	-55	,	-55	,	-55	,	-55	,	-55	,	-55	,	-55	,	-55	,	-55	,	-55	,	-55	,	-55	,	-55	,	-55	,	-55	,	-55	,	-55	,	-55	,	-55	,	-55	,	-55	,	-55	,	-55	,	-55	,	-55	,	-55	,	-55},
                                        {25	,	25	,	25	,	25	,	25	,	25	,	25	,	25	,	25	,	25	,	25	,	25	,	25	,	25	,	25	,	25	,	25	,	25	,	25	,	25	,	25	,	25	,	25	,	25	,	25	,	25	,	25	,	25	,	25	,	25	,	25	,	25	,	25	,	25	,	25	,	25	,	25	,	25	,	25	,	25	,	25	,	25	,	25	,	25	,	25	,	25	,	25	,	25	,	25	,	25	,	25	,	25	,	25	,	25	,	25	,	25	,	25	,	25	,	25	,	25	,	25	,	25	,	25	,	25	,	25	,	25	,	25	,	25	,	25	,	25	,	25	,	25	,	25	,	25	,	25	,	25	,	25	,	25	,	25	,	25	,	25	,	25	,	25	,	25	,	25	,	25	,	25	,	25	,	25	,	25	,	25	,	25	,	25	,	25	,	25	,	25	,	25	,	25	,	25	,	25	,	25	,	25	,	25	,	25	,	25	,	25	,	25	,	25	,	25	,	25	,	25	,	25	,	25	,	25	,	25	,	25	,	25	,	25	,	25	,	25	,	25	,	25	,	25	,	25	,	25	,	25	,	25	,	25	,	25	,	25	,	25	,	25	,	25	,	25	,	25	,	25	,	25	,	25	,	25	,	25	,	25},
                                        {39	,	39	,	39	,	39	,	39	,	39	,	39	,	39	,	39	,	39	,	39	,	39	,	39	,	39	,	39	,	39	,	39	,	39	,	39	,	39	,	39	,	39	,	39	,	39	,	39	,	39	,	39	,	39	,	39	,	39	,	39	,	39	,	39	,	39	,	39	,	39	,	39	,	39	,	39	,	39	,	39	,	39	,	39	,	39	,	39	,	39	,	39	,	39	,	39	,	39	,	39	,	39	,	39	,	39	,	39	,	39	,	39	,	39	,	39	,	39	,	39	,	39	,	39	,	39	,	39	,	39	,	39	,	39	,	39	,	39	,	39	,	39	,	39	,	39	,	39	,	39	,	39	,	39	,	39	,	39	,	39	,	39	,	39	,	39	,	39	,	39	,	39	,	39	,	39	,	39	,	39	,	39	,	39	,	39	,	39	,	39	,	39	,	39	,	39	,	39	,	39	,	39	,	39	,	39	,	39	,	39	,	39	,	39	,	39	,	39	,	39	,	39	,	39	,	39	,	39	,	39	,	39	,	39},
                                        {0	,	0	,	0	,	0	,	1	,	1	,	1	,	1	,	1	,	1	,	1	,	1	,	2	,	2	,	2	,	2	,	2	,	2	,	2	,	2	,	3	,	3	,	3	,	3	,	3	,	3	,	3	,	4	,	4	,	4	,	4	,	4	,	4	,	4	,	4	,	5	,	5	,	5	,	5	,	5	,	5	,	5	,	6	,	6	,	6	,	6	,	6	,	6	,	6	,	7	,	7	,	7	,	7	,	7	,	7	,	7	,	8	,	8	,	8	,	8	,	8	,	8	,	9	,	9	,	9	,	9	,	9	,	9	,	10	,	10	,	10	,	10	,	10	,	11	,	11	,	11	,	11	,	11	,	11	,	12	,	12	,	12	,	12	,	13	,	13	,	13	,	13	,	13	,	14	,	14	,	14	,	14	,	15	,	15	,	15	,	15	,	16	,	16	,	16	,	16	,	17	,	17	,	17	,	18	,	18	,	18	,	19	,	19	,	19	,	20	,	20	,	20	,	21	,	21	,	22	,	22	,	22	,	23	,	23	,	24	,	24	,	25	,	25	,	26	,	27	,	27	,	28	,	29	,	29	,	30	,	31	,	32	,	33	,	34	,	35	,	36	,	37	,	38	,	40	,	41	,	43	,	45	,	47	,	50	,	53	,	56	,	60	,	65	,	71	,	79	,	90	,	107	,	140	,	340},
                                        };
    
    TRISG = 0x0000;                         // PORTG auf schreiben stellen
    TRISA = 0x0000;                         // PORTA auf schreiben stellen
    LATA  = 0x0000;                         // LED's ansteuern
    TRISB = 0x0000;                         // PORTB auf schreiben stellen
    TRISE = 0x00cc;                         // Sensoren
   
    for(k=0;k < 14;k++)
    {
        for(i=0;i < x[k];i++)
        {
            ///////////////////////////////////////////////////////Bestimmung der Richtung
            if(x_richtung[k]< 0)
            {
                X_DIR = 0x00;
                Positionsanfahrt_x = (-1)*x_richtung[k];
            }
            else
            {
                X_DIR = 0x08;
                Positionsanfahrt_x = x_richtung[k];
            }
            if(Positionsanfahrt_y[k][i] < 0)
            {
                Y_DIR = 0x00; 
                Positionsanfahrt_y[k][i] = (-1)*Positionsanfahrt_y[k][i];
            }
            else
            {
                Y_DIR = 0x04;
            }
            //////////////////////////////////////////////Bestimmung der Wiederholungszahl
            if(Positionsanfahrt_y[k][i] >= Positionsanfahrt_x)
            {
                X_Schleife = 0;
                Y_Schleife = Positionsanfahrt_y[k][i] - Positionsanfahrt_x;
                if(Y_Schleife > Positionsanfahrt_x)
                {
                    Y_Schleife = Positionsanfahrt_x;
                }
                Schleife = Positionsanfahrt_y[k][i] - Y_Schleife;
            }
            else
            {
                Y_Schleife = 0;
                X_Schleife = Positionsanfahrt_x - Positionsanfahrt_y[k][i];
                if(X_Schleife > Positionsanfahrt_x/2)
                {
                    X_Schleife = Positionsanfahrt_x/2;
                    Schleife = Positionsanfahrt_x/2;
                }
                else
                {
                    Schleife = Positionsanfahrt_y[k][i];
                }
            }
        
            for(j=0;j<Schleife;j++)
            {
                if(j<Positionsanfahrt_x)
                {
                    X_PUL = 0x40;
                }
                else
                {
                    X_PUL = 0x00;
                }
            
                if(j<Positionsanfahrt_y[k][i])
                {
                    Y_PUL = 0x10;
                }
                else
                {
                    Y_PUL = 0x00;
                }
            
                if(j < X_Schleife)
                {
                    X_Schnell = 1;
                }
                else
                {
                    X_Schnell = 0;
                }
            
                if(j < Y_Schleife)
                {
                    Y_Schnell = 1;
                }
                else
                {
                    Y_Schnell = 0;
                }
                if((((PORTE) & 0x40) == 0x40) || (((PORTE) & 0x80) == 0x80) || (((PORTE) & 0x08) == 0x08) || (((PORTE) & 0x04) == 0x04) || ((PORTD & 0x20) == 0x20))              // Taste Maskieren
                {                                           // Stop Bedingung
                    return 0;
                }
                FAHREN(Delay, LED, X_DIR, X_PUL, Y_DIR, Y_PUL, X_Schnell, Y_Schnell);
            }
        } 
    }
    return 0;
}

int SPIRALE(void)
{
    char X_PUL,X_DIR,Y_PUL,Y_DIR;
    char LED = 0x0c;

    int k,i,j; 
    int Delay = 300;
    int Schleife,X_Schleife=0, Y_Schleife=0, X_Schnell=0, Y_Schnell=0;
    int Positionsanfahrt_x;  // ist der Puffer für x_richtung
    int x_richtung[10] = {0,28,-25,30,-20,20,-20,20,-10,5};
    int x[10] = {1,400,396,288,372,313,250,183,244,248};
    int Positionsanfahrt_y[10][400] =   {{5600},
                                        {559	,	231	,	176	,	148	,	130	,	117	,	107	,	99	,	93	,	88	,	83	,	79	,	75	,	72	,	69	,	67	,	65	,	62	,	60	,	59	,	57	,	55	,	54	,	53	,	51	,	50	,	49	,	48	,	47	,	46	,	45	,	44	,	43	,	42	,	41	,	40	,	40	,	39	,	38	,	38	,	37	,	36	,	36	,	35	,	35	,	34	,	34	,	33	,	32	,	32	,	32	,	31	,	31	,	30	,	30	,	29	,	29	,	28	,	28	,	28	,	27	,	27	,	27	,	26	,	26	,	25	,	25	,	25	,	24	,	24	,	24	,	23	,	23	,	23	,	23	,	22	,	22	,	22	,	21	,	21	,	21	,	21	,	20	,	20	,	20	,	20	,	19	,	19	,	19	,	19	,	18	,	18	,	18	,	18	,	17	,	17	,	17	,	17	,	16	,	16	,	16	,	16	,	16	,	15	,	15	,	15	,	15	,	15	,	14	,	14	,	14	,	14	,	14	,	13	,	13	,	13	,	13	,	13	,	12	,	12	,	12	,	12	,	12	,	12	,	11	,	11	,	11	,	11	,	11	,	11	,	10	,	10	,	10	,	10	,	10	,	10	,	9	,	9	,	9	,	9	,	9	,	9	,	8	,	8	,	8	,	8	,	8	,	8	,	7	,	7	,	7	,	7	,	7	,	7	,	7	,	6	,	6	,	6	,	6	,	6	,	6	,	5	,	5	,	5	,	5	,	5	,	5	,	5	,	4	,	4	,	4	,	4	,	4	,	4	,	4	,	3	,	3	,	3	,	3	,	3	,	3	,	3	,	2	,	2	,	2	,	2	,	2	,	2	,	2	,	1	,	1	,	1	,	1	,	1	,	1	,	1	,	0	,	0	,	0	,	0	,	0	,	0	,	0	,	0	,	-1	,	-1	,	-1	,	-1	,	-1	,	-1	,	-1	,	-2	,	-2	,	-2	,	-2	,	-2	,	-2	,	-2	,	-3	,	-3	,	-3	,	-3	,	-3	,	-3	,	-3	,	-4	,	-4	,	-4	,	-4	,	-4	,	-4	,	-4	,	-5	,	-5	,	-5	,	-5	,	-5	,	-5	,	-5	,	-6	,	-6	,	-6	,	-6	,	-6	,	-6	,	-7	,	-7	,	-7	,	-7	,	-7	,	-7	,	-7	,	-8	,	-8	,	-8	,	-8	,	-8	,	-8	,	-9	,	-9	,	-9	,	-9	,	-9	,	-9	,	-10	,	-10	,	-10	,	-10	,	-10	,	-10	,	-11	,	-11	,	-11	,	-11	,	-11	,	-11	,	-12	,	-12	,	-12	,	-12	,	-12	,	-12	,	-13	,	-13	,	-13	,	-13	,	-13	,	-14	,	-14	,	-14	,	-14	,	-14	,	-15	,	-15	,	-15	,	-15	,	-15	,	-16	,	-16	,	-16	,	-16	,	-16	,	-17	,	-17	,	-17	,	-17	,	-18	,	-18	,	-18	,	-18	,	-19	,	-19	,	-19	,	-19	,	-20	,	-20	,	-20	,	-20	,	-21	,	-21	,	-21	,	-21	,	-22	,	-22	,	-22	,	-23	,	-23	,	-23	,	-23	,	-24	,	-24	,	-24	,	-25	,	-25	,	-25	,	-26	,	-26	,	-27	,	-27	,	-27	,	-28	,	-28	,	-28	,	-29	,	-29	,	-30	,	-30	,	-31	,	-31	,	-32	,	-32	,	-32	,	-33	,	-34	,	-34	,	-35	,	-35	,	-36	,	-36	,	-37	,	-38	,	-38	,	-39	,	-40	,	-40	,	-41	,	-42	,	-43	,	-44	,	-45	,	-46	,	-47	,	-48	,	-49	,	-50	,	-51	,	-53	,	-54	,	-55	,	-57	,	-59	,	-60	,	-62	,	-65	,	-67	,	-69	,	-72	,	-75	,	-79	,	-83	,	-88	,	-93	,	-99	,	-107	,	-117	,	-130	,	-148	,	-176	,	-231	,	-559},
                                        {-497	,-205	,	-157,	-132	,	-115	,	-104	,	-95	,	-88	,	-83	,	-78	,	-74	,	-70	,	-67	,	-64	,	-62	,	-59	,	-57	,	-55	,	-54	,	-52	,	-51	,	-49	,	-48	,	-47	,	-45	,	-44	,	-43	,	-42	,	-41	,	-41	,	-40	,	-39	,	-38	,	-37	,	-37	,	-36	,	-35	,	-35	,	-34	,	-33	,	-33	,	-32	,	-32	,	-31	,	-31	,	-30	,	-30	,	-29	,	-29	,	-28	,	-28	,	-27	,	-27	,	-27	,	-26	,	-26	,	-26	,	-25	,	-25	,	-24	,	-24	,	-24	,	-23	,	-23	,	-23	,	-23	,	-22	,	-22	,	-22	,	-21	,	-21	,	-21	,	-20	,	-20	,	-20	,	-20	,	-19	,	-19	,	-19	,	-19	,	-18	,	-18	,	-18	,	-18	,	-17	,	-17	,	-17	,	-17	,	-17	,	-16	,	-16	,	-16	,	-16	,	-16	,	-15	,	-15	,	-15	,	-15	,	-15	,	-14	,	-14	,	-14	,	-14	,	-14	,	-13	,	-13	,	-13	,	-13	,	-13	,	-12	,	-12	,	-12	,	-12	,	-12	,	-12	,	-11	,	-11	,	-11	,	-11	,	-11	,	-11	,	-10	,	-10	,	-10	,	-10	,	-10	,	-10	,	-10	,	-9	,	-9	,	-9	,	-9	,	-9	,	-9	,	-8	,	-8	,	-8	,	-8	,	-8	,	-8	,	-8	,	-7	,	-7	,	-7	,	-7	,	-7	,	-7	,	-7	,	-6	,	-6	,	-6	,	-6	,	-6	,	-6	,	-6	,	-5	,	-5	,	-5	,	-5	,	-5	,	-5	,	-5	,	-5	,	-4	,	-4	,	-4	,	-4	,	-4	,	-4	,	-4	,	-4	,	-3	,	-3	,	-3	,	-3	,	-3	,	-3	,	-3	,	-2	,	-2	,	-2	,	-2	,	-2	,	-2	,	-2	,	-2	,	-1	,	-1	,	-1	,	-1	,	-1	,	-1	,	-1	,	-1	,	0	,	0	,	0	,	0	,	0	,	0	,	0	,	0	,	1	,	1	,	1	,	1	,	1	,	1	,	1	,	1	,	2	,	2	,	2	,	2	,	2	,	2	,	2	,	2	,	3	,	3	,	3	,	3	,	3	,	3	,	3	,	4	,	4	,	4	,	4	,	4	,	4	,	4	,	4	,	5	,	5	,	5	,	5	,	5	,	5	,	5	,	5	,	6	,	6	,	6	,	6	,	6	,	6	,	6	,	7	,	7	,	7	,	7	,	7	,	7	,	7	,	8	,	8	,	8	,	8	,	8	,	8	,	8	,	9	,	9	,	9	,	9	,	9	,	9	,	10	,	10	,	10	,	10	,	10	,	10	,	10	,	11	,	11	,	11	,	11	,	11	,	11	,	12	,	12	,	12	,	12	,	12	,	12	,	13	,	13	,	13	,	13	,	13	,	14	,	14	,	14	,	14	,	14	,	15	,	15	,	15	,	15	,	15	,	16	,	16	,	16	,	16	,	16	,	17	,	17	,	17	,	17	,	17	,	18	,	18	,	18	,	18	,	19	,	19	,	19	,	19	,	20	,	20	,	20	,	20	,	21	,	21	,	21	,	22	,	22	,	22	,	23	,	23	,	23	,	23	,	24	,	24	,	24	,	25	,	25	,	26	,	26	,	26	,	27	,	27	,	27	,	28	,	28	,	29	,	29	,	30	,	30	,	31	,	31	,	32	,	32	,	33	,	33	,	34	,	35	,	35	,	36	,	37	,	37	,	38	,	39	,	40	,	41	,	41	,	42	,	43	,	44	,	45	,	47	,	48	,	49	,	51	,	52	,	54	,	55	,	57	,	59	,	62	,	64	,	67	,	70	,	74	,	78	,	83	,	88	,	95	,	104	,	115	,	132	,	157	,	205	,	497},
                                        {508	,	209	,	160	,	134	,	117	,	106	,	97	,	89	,	83	,	78	,	74	,	71	,	67	,	64	,	62	,	59	,	57	,	55	,	53	,	52	,	50	,	49	,	47	,	46	,	45	,	43	,	42	,	41	,	40	,	39	,	38	,	38	,	37	,	36	,	35	,	34	,	34	,	33	,	32	,	32	,	31	,	30	,	30	,	29	,	29	,	28	,	28	,	27	,	27	,	26	,	26	,	25	,	25	,	24	,	24	,	23	,	23	,	23	,	22	,	22	,	21	,	21	,	21	,	20	,	20	,	20	,	19	,	19	,	18	,	18	,	18	,	17	,	17	,	17	,	17	,	16	,	16	,	16	,	15	,	15	,	15	,	14	,	14	,	14	,	14	,	13	,	13	,	13	,	13	,	12	,	12	,	12	,	11	,	11	,	11	,	11	,	10	,	10	,	10	,	10	,	10	,	9	,	9	,	9	,	9	,	8	,	8	,	8	,	8	,	7	,	7	,	7	,	7	,	7	,	6	,	6	,	6	,	6	,	5	,	5	,	5	,	5	,	5	,	4	,	4	,	4	,	4	,	3	,	3	,	3	,	3	,	3	,	2	,	2	,	2	,	2	,	2	,	1	,	1	,	1	,	1	,	1	,	0	,	0	,	0	,	0	,	-1	,	-1	,	-1	,	-1	,	-1	,	-2	,	-2	,	-2	,	-2	,	-2	,	-3	,	-3	,	-3	,	-3	,	-3	,	-4	,	-4	,	-4	,	-4	,	-5	,	-5	,	-5	,	-5	,	-5	,	-6	,	-6	,	-6	,	-6	,	-7	,	-7	,	-7	,	-7	,	-7	,	-8	,	-8	,	-8	,	-8	,	-9	,	-9	,	-9	,	-9	,	-10	,	-10	,	-10	,	-10	,	-10	,	-11	,	-11	,	-11	,	-11	,	-12	,	-12	,	-12	,	-13	,	-13	,	-13	,	-13	,	-14	,	-14	,	-14	,	-14	,	-15	,	-15	,	-15	,	-16	,	-16	,	-16	,	-17	,	-17	,	-17	,	-17	,	-18	,	-18	,	-18	,	-19	,	-19	,	-20	,	-20	,	-20	,	-21	,	-21	,	-21	,	-22	,	-22	,	-23	,	-23	,	-23	,	-24	,	-24	,	-25	,	-25	,	-26	,	-26	,	-27	,	-27	,	-28	,	-28	,	-29	,	-29	,	-30	,	-30	,	-31	,	-32	,	-32	,	-33	,	-34	,	-34	,	-35	,	-36	,	-37	,	-38	,	-38	,	-39	,	-40	,	-41	,	-42	,	-43	,	-45	,	-46	,	-47	,	-49	,	-50	,	-52	,	-53	,	-55	,	-57	,	-59	,	-62	,	-64	,	-67	,	-71	,	-74	,	-78	,	-83	,	-89	,	-97	,	-106	,	-117	,	-134	,	-160	,	-209	,	-508},
                                        {-385	,	-159	,	-121	,	-102	,	-89	,	-80	,	-74	,	-68	,	-64	,	-60	,	-57	,	-54	,	-52	,	-50	,	-48	,	-46	,	-44	,	-43	,	-41	,	-40	,	-39	,	-38	,	-37	,	-36	,	-35	,	-34	,	-33	,	-33	,	-32	,	-31	,	-30	,	-30	,	-29	,	-29	,	-28	,	-28	,	-27	,	-27	,	-26	,	-26	,	-25	,	-25	,	-24	,	-24	,	-23	,	-23	,	-23	,	-22	,	-22	,	-22	,	-21	,	-21	,	-21	,	-20	,	-20	,	-20	,	-19	,	-19	,	-19	,	-19	,	-18	,	-18	,	-18	,	-18	,	-17	,	-17	,	-17	,	-17	,	-16	,	-16	,	-16	,	-16	,	-15	,	-15	,	-15	,	-15	,	-15	,	-14	,	-14	,	-14	,	-14	,	-14	,	-13	,	-13	,	-13	,	-13	,	-13	,	-12	,	-12	,	-12	,	-12	,	-12	,	-12	,	-11	,	-11	,	-11	,	-11	,	-11	,	-11	,	-11	,	-10	,	-10	,	-10	,	-10	,	-10	,	-10	,	-9	,	-9	,	-9	,	-9	,	-9	,	-9	,	-9	,	-8	,	-8	,	-8	,	-8	,	-8	,	-8	,	-8	,	-8	,	-7	,	-7	,	-7	,	-7	,	-7	,	-7	,	-7	,	-7	,	-6	,	-6	,	-6	,	-6	,	-6	,	-6	,	-6	,	-6	,	-5	,	-5	,	-5	,	-5	,	-5	,	-5	,	-5	,	-5	,	-4	,	-4	,	-4	,	-4	,	-4	,	-4	,	-4	,	-4	,	-4	,	-3	,	-3	,	-3	,	-3	,	-3	,	-3	,	-3	,	-3	,	-3	,	-2	,	-2	,	-2	,	-2	,	-2	,	-2	,	-2	,	-2	,	-2	,	-1	,	-1	,	-1	,	-1	,	-1	,	-1	,	-1	,	-1	,	-1	,	0	,	0	,	0	,	0	,	0	,	0	,	0	,	0	,	0	,	0	,	1	,	1	,	1	,	1	,	1	,	1	,	1	,	1	,	1	,	2	,	2	,	2	,	2	,	2	,	2	,	2	,	2	,	2	,	3	,	3	,	3	,	3	,	3	,	3	,	3	,	3	,	3	,	4	,	4	,	4	,	4	,	4	,	4	,	4	,	4	,	4	,	5	,	5	,	5	,	5	,	5	,	5	,	5	,	5	,	6	,	6	,	6	,	6	,	6	,	6	,	6	,	6	,	7	,	7	,	7	,	7	,	7	,	7	,	7	,	7	,	8	,	8	,	8	,	8	,	8	,	8	,	8	,	8	,	9	,	9	,	9	,	9	,	9	,	9	,	9	,	10	,	10	,	10	,	10	,	10	,	10	,	11	,	11	,	11	,	11	,	11	,	11	,	11	,	12	,	12	,	12	,	12	,	12	,	12	,	13	,	13	,	13	,	13	,	13	,	14	,	14	,	14	,	14	,	14	,	15	,	15	,	15	,	15	,	15	,	16	,	16	,	16	,	16	,	17	,	17	,	17	,	17	,	18	,	18	,	18	,	18	,	19	,	19	,	19	,	19	,	20	,	20	,	20	,	21	,	21	,	21	,	22	,	22	,	22	,	23	,	23	,	23	,	24	,	24	,	25	,	25	,	26	,	26	,	27	,	27	,	28	,	28	,	29	,	29	,	30	,	30	,	31	,	32	,	33	,	33	,	34	,	35	,	36	,	37	,	38	,	39	,	40	,	41	,	43	,	44	,	46	,	48	,	50	,	52	,	54	,	57	,	60	,	64	,	68	,	74	,	80	,	89	,	102	,	121	,	159	,	385},
                                        {353	,	146	,	111	,	93	,	82	,	74	,	67	,	62	,	58	,	55	,	52	,	49	,	47	,	45	,	43	,	42	,	40	,	39	,	37	,	36	,	35	,	34	,	33	,	32	,	31	,	31	,	30	,	29	,	28	,	28	,	27	,	27	,	26	,	25	,	25	,	24	,	24	,	23	,	23	,	23	,	22	,	22	,	21	,	21	,	20	,	20	,	20	,	19	,	19	,	19	,	18	,	18	,	18	,	17	,	17	,	17	,	17	,	16	,	16	,	16	,	16	,	15	,	15	,	15	,	15	,	14	,	14	,	14	,	14	,	13	,	13	,	13	,	13	,	13	,	12	,	12	,	12	,	12	,	11	,	11	,	11	,	11	,	11	,	11	,	10	,	10	,	10	,	10	,	10	,	9	,	9	,	9	,	9	,	9	,	9	,	8	,	8	,	8	,	8	,	8	,	8	,	8	,	7	,	7	,	7	,	7	,	7	,	7	,	6	,	6	,	6	,	6	,	6	,	6	,	6	,	5	,	5	,	5	,	5	,	5	,	5	,	5	,	4	,	4	,	4	,	4	,	4	,	4	,	4	,	4	,	3	,	3	,	3	,	3	,	3	,	3	,	3	,	2	,	2	,	2	,	2	,	2	,	2	,	2	,	2	,	1	,	1	,	1	,	1	,	1	,	1	,	1	,	1	,	0	,	0	,	0	,	0	,	0	,	0	,	0	,	-1	,	-1	,	-1	,	-1	,	-1	,	-1	,	-1	,	-1	,	-2	,	-2	,	-2	,	-2	,	-2	,	-2	,	-2	,	-2	,	-3	,	-3	,	-3	,	-3	,	-3	,	-3	,	-3	,	-4	,	-4	,	-4	,	-4	,	-4	,	-4	,	-4	,	-4	,	-5	,	-5	,	-5	,	-5	,	-5	,	-5	,	-5	,	-6	,	-6	,	-6	,	-6	,	-6	,	-6	,	-6	,	-7	,	-7	,	-7	,	-7	,	-7	,	-7	,	-8	,	-8	,	-8	,	-8	,	-8	,	-8	,	-8	,	-9	,	-9	,	-9	,	-9	,	-9	,	-9	,	-10	,	-10	,	-10	,	-10	,	-10	,	-11	,	-11	,	-11	,	-11	,	-11	,	-11	,	-12	,	-12	,	-12	,	-12	,	-13	,	-13	,	-13	,	-13	,	-13	,	-14	,	-14	,	-14	,	-14	,	-15	,	-15	,	-15	,	-15	,	-16	,	-16	,	-16	,	-16	,	-17	,	-17	,	-17	,	-17	,	-18	,	-18	,	-18	,	-19	,	-19	,	-19	,	-20	,	-20	,	-20	,	-21	,	-21	,	-22	,	-22	,	-23	,	-23	,	-23	,	-24	,	-24	,	-25	,	-25	,	-26	,	-27	,	-27	,	-28	,	-28	,	-29	,	-30	,	-31	,	-31	,	-32	,	-33	,	-34	,	-35	,	-36	,	-37	,	-39	,	-40	,	-42	,	-43	,	-45	,	-47	,	-49	,	-52	,	-55	,	-58	,	-62	,	-67	,	-74	,	-82	,	-93	,	-111	,	-146	,	-353},
                                        {-316	,	-130	,	-99	,	-83	,	-73	,	-65	,	-60	,	-55	,	-51	,	-48	,	-46	,	-43	,	-41	,	-39	,	-38	,	-36	,	-35	,	-34	,	-33	,	-31	,	-30	,	-30	,	-29	,	-28	,	-27	,	-26	,	-26	,	-25	,	-24	,	-24	,	-23	,	-23	,	-22	,	-21	,	-21	,	-21	,	-20	,	-20	,	-19	,	-19	,	-18	,	-18	,	-18	,	-17	,	-17	,	-16	,	-16	,	-16	,	-15	,	-15	,	-15	,	-15	,	-14	,	-14	,	-14	,	-13	,	-13	,	-13	,	-13	,	-12	,	-12	,	-12	,	-12	,	-11	,	-11	,	-11	,	-11	,	-10	,	-10	,	-10	,	-10	,	-9	,	-9	,	-9	,	-9	,	-9	,	-8	,	-8	,	-8	,	-8	,	-8	,	-7	,	-7	,	-7	,	-7	,	-7	,	-6	,	-6	,	-6	,	-6	,	-6	,	-6	,	-5	,	-5	,	-5	,	-5	,	-5	,	-5	,	-4	,	-4	,	-4	,	-4	,	-4	,	-3	,	-3	,	-3	,	-3	,	-3	,	-3	,	-2	,	-2	,	-2	,	-2	,	-2	,	-2	,	-2	,	-1	,	-1	,	-1	,	-1	,	-1	,	-1	,	0	,	0	,	0	,	0	,	0	,	0	,	1	,	1	,	1	,	1	,	1	,	1	,	2	,	2	,	2	,	2	,	2	,	2	,	2	,	3	,	3	,	3	,	3	,	3	,	3	,	4	,	4	,	4	,	4	,	4	,	5	,	5	,	5	,	5	,	5	,	5	,	6	,	6	,	6	,	6	,	6	,	6	,	7	,	7	,	7	,	7	,	7	,	8	,	8	,	8	,	8	,	8	,	9	,	9	,	9	,	9	,	9	,	10	,	10	,	10	,	10	,	11	,	11	,	11	,	11	,	12	,	12	,	12	,	12	,	13	,	13	,	13	,	13	,	14	,	14	,	14	,	15	,	15	,	15	,	15	,	16	,	16	,	16	,	17	,	17	,	18	,	18	,	18	,	19	,	19	,	20	,	20	,	21	,	21	,	21	,	22	,	23	,	23	,	24	,	24	,	25	,	26	,	26	,	27	,	28	,	29	,	30	,	30	,	31	,	33	,	34	,	35	,	36	,	38	,	39	,	41	,	43	,	46	,	48	,	51	,	55	,	60	,	65	,	73	,	83	,	99	,	130	,	316},
                                        {270	,	111	,	84	,	70	,	61	,	55	,	50	,	46	,	43	,	40	,	38	,	36	,	34	,	33	,	31	,	30	,	29	,	28	,	26	,	26	,	25	,	24	,	23	,	22	,	22	,	21	,	20	,	20	,	19	,	18	,	18	,	17	,	17	,	16	,	16	,	15	,	15	,	15	,	14	,	14	,	13	,	13	,	13	,	12	,	12	,	12	,	11	,	11	,	11	,	10	,	10	,	10	,	9	,	9	,	9	,	9	,	8	,	8	,	8	,	7	,	7	,	7	,	7	,	6	,	6	,	6	,	6	,	5	,	5	,	5	,	5	,	4	,	4	,	4	,	4	,	4	,	3	,	3	,	3	,	3	,	2	,	2	,	2	,	2	,	2	,	1	,	1	,	1	,	1	,	0	,	0	,	0	,	0	,	0	,	-1	,	-1	,	-1	,	-1	,	-2	,	-2	,	-2	,	-2	,	-2	,	-3	,	-3	,	-3	,	-3	,	-4	,	-4	,	-4	,	-4	,	-4	,	-5	,	-5	,	-5	,	-5	,	-6	,	-6	,	-6	,	-6	,	-7	,	-7	,	-7	,	-7	,	-8	,	-8	,	-8	,	-9	,	-9	,	-9	,	-9	,	-10	,	-10	,	-10	,	-11	,	-11	,	-11	,	-12	,	-12	,	-12	,	-13	,	-13	,	-13	,	-14	,	-14	,	-15	,	-15	,	-15	,	-16	,	-16	,	-17	,	-17	,	-18	,	-18	,	-19	,	-20	,	-20	,	-21	,	-22	,	-22	,	-23	,	-24	,	-25	,	-26	,	-26	,	-28	,	-29	,	-30	,	-31	,	-33	,	-34	,	-36	,	-38	,	-40	,	-43	,	-46	,	-50	,	-55	,	-61	,	-70	,	-84	,	-111	,	-270},
                                        {-156	,	-64	,	-49	,	-41	,	-36	,	-32	,	-29	,	-27	,	-25	,	-24	,	-23	,	-21	,	-20	,	-19	,	-19	,	-18	,	-17	,	-17	,	-16	,	-15	,	-15	,	-15	,	-14	,	-14	,	-13	,	-13	,	-13	,	-12	,	-12	,	-12	,	-11	,	-11	,	-11	,	-11	,	-10	,	-10	,	-10	,	-10	,	-9	,	-9	,	-9	,	-9	,	-9	,	-8	,	-8	,	-8	,	-8	,	-8	,	-8	,	-7	,	-7	,	-7	,	-7	,	-7	,	-7	,	-7	,	-6	,	-6	,	-6	,	-6	,	-6	,	-6	,	-6	,	-5	,	-5	,	-5	,	-5	,	-5	,	-5	,	-5	,	-5	,	-5	,	-4	,	-4	,	-4	,	-4	,	-4	,	-4	,	-4	,	-4	,	-4	,	-4	,	-3	,	-3	,	-3	,	-3	,	-3	,	-3	,	-3	,	-3	,	-3	,	-3	,	-2	,	-2	,	-2	,	-2	,	-2	,	-2	,	-2	,	-2	,	-2	,	-2	,	-2	,	-2	,	-1	,	-1	,	-1	,	-1	,	-1	,	-1	,	-1	,	-1	,	-1	,	-1	,	-1	,	-1	,	0	,	0	,	0	,	0	,	0	,	0	,	0	,	0	,	0	,	0	,	0	,	0	,	1	,	1	,	1	,	1	,	1	,	1	,	1	,	1	,	1	,	1	,	1	,	1	,	2	,	2	,	2	,	2	,	2	,	2	,	2	,	2	,	2	,	2	,	2	,	2	,	3	,	3	,	3	,	3	,	3	,	3	,	3	,	3	,	3	,	3	,	4	,	4	,	4	,	4	,	4	,	4	,	4	,	4	,	4	,	4	,	5	,	5	,	5	,	5	,	5	,	5	,	5	,	5	,	5	,	6	,	6	,	6	,	6	,	6	,	6	,	6	,	7	,	7	,	7	,	7	,	7	,	7	,	7	,	8	,	8	,	8	,	8	,	8	,	8	,	9	,	9	,	9	,	9	,	9	,	10	,	10	,	10	,	10	,	11	,	11	,	11	,	11	,	12	,	12	,	12	,	13	,	13	,	13	,	14	,	14	,	15	,	15	,	15	,	16	,	17	,	17	,	18	,	19	,	19	,	20	,	21	,	23	,	24	,	25	,	27	,	29	,	32	,	36	,	41	,	49	,	64	,	156},
                                        {79	,	32	,	25	,	21	,	18	,	16	,	15	,	14	,	13	,	12	,	11	,	11	,	10	,	10	,	9	,	9	,	9	,	8	,	8	,	8	,	8	,	7	,	7	,	7	,	7	,	7	,	6	,	6	,	6	,	6	,	6	,	6	,	5	,	5	,	5	,	5	,	5	,	5	,	5	,	5	,	5	,	4	,	4	,	4	,	4	,	4	,	4	,	4	,	4	,	4	,	4	,	4	,	4	,	3	,	3	,	3	,	3	,	3	,	3	,	3	,	3	,	3	,	3	,	3	,	3	,	3	,	3	,	3	,	3	,	2	,	2	,	2	,	2	,	2	,	2	,	2	,	2	,	2	,	2	,	2	,	2	,	2	,	2	,	2	,	2	,	2	,	2	,	2	,	1	,	1	,	1	,	1	,	1	,	1	,	1	,	1	,	1	,	1	,	1	,	1	,	1	,	1	,	1	,	1	,	1	,	1	,	1	,	1	,	1	,	1	,	1	,	1	,	0	,	0	,	0	,	0	,	0	,	0	,	0	,	0	,	0	,	0	,	0	,	0	,	0	,	0	,	0	,	0	,	0	,	0	,	0	,	0	,	0	,	0	,	0	,	0	,	-1	,	-1	,	-1	,	-1	,	-1	,	-1	,	-1	,	-1	,	-1	,	-1	,	-1	,	-1	,	-1	,	-1	,	-1	,	-1	,	-1	,	-1	,	-1	,	-1	,	-1	,	-1	,	-1	,	-1	,	-2	,	-2	,	-2	,	-2	,	-2	,	-2	,	-2	,	-2	,	-2	,	-2	,	-2	,	-2	,	-2	,	-2	,	-2	,	-2	,	-2	,	-2	,	-2	,	-3	,	-3	,	-3	,	-3	,	-3	,	-3	,	-3	,	-3	,	-3	,	-3	,	-3	,	-3	,	-3	,	-3	,	-3	,	-3	,	-4	,	-4	,	-4	,	-4	,	-4	,	-4	,	-4	,	-4	,	-4	,	-4	,	-4	,	-4	,	-5	,	-5	,	-5	,	-5	,	-5	,	-5	,	-5	,	-5	,	-5	,	-6	,	-6	,	-6	,	-6	,	-6	,	-6	,	-7	,	-7	,	-7	,	-7	,	-7	,	-8	,	-8	,	-8	,	-8	,	-9	,	-9	,	-9	,	-10	,	-10	,	-11	,	-11	,	-12	,	-13	,	-14	,	-15	,	-16	,	-18	,	-21	,	-25	,	-32	,	-79}};
  
    
    TRISG = 0x0000;                         // PORTG auf schreiben stellen
    TRISA = 0x0000;                         // PORTA auf schreiben stellen
    LATA  = 0x0000;                         // LED's ansteuern
    TRISB = 0x0000;                         // PORTB auf schreiben stellen
    TRISE = 0x00cc;
   
    for(k=0;k < 10;k++)
    {
        for(i=0;i < x[k];i++)
        {
            ///////////////////////////////////////////////////////Bestimmung der Richtung
            if(x_richtung[k]< 0)
            {
                X_DIR = 0x00;
                Positionsanfahrt_x = (-1)*x_richtung[k];
            }
            else
            {
                X_DIR = 0x08;
                Positionsanfahrt_x = x_richtung[k];
            }
            if(Positionsanfahrt_y[k][i] < 0)
            {
                Y_DIR = 0x00; 
                Positionsanfahrt_y[k][i] = (-1)*Positionsanfahrt_y[k][i];
            }
            else
            {
                Y_DIR = 0x04;
            }
            //////////////////////////////////////////////Bestimmung der Wiederholungszahl
            if(Positionsanfahrt_y[k][i] >= Positionsanfahrt_x)
            {
                X_Schleife = 0;
                Y_Schleife = Positionsanfahrt_y[k][i] - Positionsanfahrt_x;
                if(Y_Schleife > Positionsanfahrt_x)
                {
                    Y_Schleife = Positionsanfahrt_x;
                }
                Schleife = Positionsanfahrt_y[k][i] - Y_Schleife;
            }
            else
            {
                Y_Schleife = 0;
                X_Schleife = Positionsanfahrt_x - Positionsanfahrt_y[k][i];
                if(X_Schleife > Positionsanfahrt_x/2)
                {
                    X_Schleife = Positionsanfahrt_x/2;
                    Schleife = Positionsanfahrt_x/2;
                }
                else
                {
                    Schleife = Positionsanfahrt_y[k][i];
                }
            }
        
            for(j=0;j<Schleife;j++)
            {
                if(j<Positionsanfahrt_x)
                {
                    X_PUL = 0x40;
                }
                else
                {
                    X_PUL = 0x00;
                }
            
                if(j<Positionsanfahrt_y[k][i])
                {
                    Y_PUL = 0x10;
                }
                else
                {
                    Y_PUL = 0x00;
                }
            
                if(j < X_Schleife)
                {
                    X_Schnell = 1;
                }
                else
                {
                    X_Schnell = 0;
                }
            
                if(j < Y_Schleife)
                {
                    Y_Schnell = 1;
                }
                else
                {
                    Y_Schnell = 0;
                }
                if((((PORTE) & 0x40) == 0x40) || (((PORTE) & 0x80) == 0x80) || (((PORTE) & 0x08) == 0x08) || (((PORTE) & 0x04) == 0x04) || ((PORTD & 0x20) == 0x20))              // Taste Maskieren
                {                                           // Stop Bedingung
                    return 0;
                }
                FAHREN(Delay, LED, X_DIR, X_PUL, Y_DIR, Y_PUL, X_Schnell, Y_Schnell);
            }
        } 
    }
    return 0;
}

int QUADRATISCHE_SPIRALE(void)
{
    TRISG = 0x0000;                         // PORTG auf schreiben stellen
    TRISA = 0x0000;                         // PORTA auf schreiben stellen
    LATA  = 0x0000;                         // LED's ansteuern
    TRISB = 0x0000;                         // PORTB auf schreiben stellen
    TRISE = 0x00cc;                         // Sensoren
    
    char X_PUL, X_DIR, Y_PUL, Y_DIR;
    char LED = 0x30; 
    
    int i,j;
    int Delay = 200;
    // x-richtung = 1; y-richtung = 0;
    int xy_richtung[21] = {1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1}; 
    // vorwaerts = 1; rueckwearts = 0;
    int richtung[21] = {1,1,0,0,1,1,0,0,1,1,0,0,1,1,0,0,1,1,0,0,1};
    unsigned int X_Schnell=0, Y_Schnell=0;
    unsigned int Positionsanfahrt[21] = {11200,11200,11200,10080,10080,8960,8960,7840,7840,6720,6720,5600,5600,4480,4480,3360,3360,2240,2240,1120,1120};
    
    
    for(i=0;i<21;i++)
    {
        if(xy_richtung[i]== 1)
        {
            X_PUL = 0x40;
            Y_PUL = 0x00;
        }
        else
        {
            X_PUL = 0x00;
            Y_PUL = 0x10;
        }
        if(richtung[i] == 1)
        {
            X_DIR = 0x08;
            Y_DIR = 0x04;
        }
        else
        {
            X_DIR = 0x00;
            Y_DIR = 0x00;
        }
        
        for(j=0; j < Positionsanfahrt[i];j++) 
        {
            if((((PORTE) & 0x40) == 0x40) || (((PORTE) & 0x80) == 0x80) || (((PORTE) & 0x08) == 0x08) || (((PORTE) & 0x04) == 0x04) || ((PORTD & 0x20) == 0x20))              // Taste Maskieren
            {                                           // Stop Bedingung
                return 0;
            }
            FAHREN(Delay, LED, X_DIR, X_PUL, Y_DIR, Y_PUL, X_Schnell, Y_Schnell);
        }
    }
    return 0;
}

void ABSCHALTEN(void)
{   
    TRISG = 0x0000;                         // PORTG auf schreiben stellen
    TRISB = 0x0000;                         // PORTB auf schreiben stellen
    PORTB = 0x0000;                         // ENA = 0; DIR = 0;
    PORTG = 0x0000;                         // PUL = 0;
    T1CON = 0x8030;                         // Einstellung Timer 1
    
    TMR1 = 0;                               // Reset Timer 1
    while (TMR1 < 15625)
    {
        //warten
    }
}

