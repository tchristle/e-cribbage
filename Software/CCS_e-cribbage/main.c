//******************************************************************************
//  MSP430F2012 LED cribbage board - software
//
//               MSP430F2012
//            -----------------
//        /|\|              XIN|-
//         | |                 |
//         --|RST          XOUT|-
//           |                 |
//           |             P1.0|<--S1
//           |             P1.1|<--S2
//           |             P1.2|<--S3
//           |             P1.3|<--S4
//           |                 |
//           |                 |     ^   LED TRACKS
//           |                 |     |  -------------
//           |                 |     |-|VDD          |
//           |             P1.6|------>|DATA         |
//           |             P1.5|------>|CLK          |
//           |             P1.4|------>|RCLK         |
//           |                 |       |             |
//           |             P2.6|------>|LE1          |
//           |             P2.7|------>|LE2          |
//           |                 |       |             |
//           |             P1.7|------>|VSW          |
//           |                 |       |             |
//
// 1 - VCC
// 2 - S1           (P1.0)
// 3 - S2           (P1.1)
// 4 - S3           (P1.2)
// 5 - S4           (P1.3)
// 6 - RCLK         (P1.4)
// 7 - CLK          (P1.5)
// 8 - DATA         (P1.6)
// 9 - VSW          (P1.7)
// 10 - JTAG - RST
// 11 - JTAG - TCK
// 12 - LE2         (P2.7)
// 13 - LE1         (P2.6)
// 14 - GND
//
//
//  Travis Christle
//  JUNE 2020
//  Built with CCS Version: 10.1.0
//******************************************************************************
#include "msp430.h"
#include "stdbool.h"

#define P1REV   BIT0                // TXD on P1.5 to uart module
#define P1FWD   BIT1                // RXD on P1.1 from uart module
#define P2REV   BIT2                // LED
#define P2FWD   BIT3                // LED
#define LE      BIT4                // Latch Enable
#define CLK     BIT5                // Clock
#define DATA    BIT6                // Data
#define VSW     BIT7                // 3V SW

#define LE1     BIT6                // P1_EN
#define LE2     BIT7                // P2_EN

#define delay10us       1
#define delay100us      9
#define delay20ms       1850
#define five_minutes    5400
#define five_seconds    400
#define longpress       20000

//global definitions
unsigned int i=0, j=0;
unsigned int main_time=0;
unsigned int peg_time=0;
unsigned int P1pts = 0;
unsigned int P1pegA = 0;
unsigned int P1pegB = 0;
unsigned int P2pts = 0;
unsigned int P2pegA = 0;
unsigned int P2pegB = 0;
bool cur1Peg = false;
bool cur2Peg = false;
bool button_pressed = false;

// Function Definitions
void show_pegs(void);
void restart_game(void);
void save_game(void);
void wait_for_input(void);
void show_winner_pattern(void);
void delay(unsigned int dcount);

// ******************************************************************************************

/**
 * main.c
 */
int main(void)
{
	WDTCTL = WDTPW | WDTHOLD;	// stop watchdog timer
	
	//Configure Basic Clock
	BCSCTL1 = CALBC1_1MHZ;                     // Set range
	DCOCTL = CALDCO_1MHZ;                      // SMCLK = DCO = 8MHz

	//Peripheral Setup
	P1DIR |= LE + VSW + CLK + DATA;
	P1OUT |= VSW;
	P1OUT &= ~LE & ~CLK & ~DATA;
	P2DIR |= LE1 + LE2;
	P2OUT |= LE1 + LE2;
	//P1SEL P2SEL
	P1SEL = 0;
	P2SEL = 0;

	wait_for_input();

	return 0;
}

// ******************************************************************************************
void wait_for_input(void)
{
    //~393us
    //Enable VSW
    P1OUT |= VSW;
    //show_pegs();
    //Start Timer
    main_time=0;

    while(main_time<five_minutes) //~20s at 12MHz
    {
        //Reset Button Press Timer
        j=0;
        //P1OUT |= VSW;
        //P1OUT &= ~VSW;
        //if Timer_j > 5s then switch Current Peg
        //if Timer_i > 5min then save game and power down

        //Detect Button Press
        while(P1REV & ~P1IN)
        {
            button_pressed = true;
            if(j==0 && P1pegA>P1pegB && P1pegA>0) P1pegA--;
            if(j==0 && P1pegB>P1pegA && P1pegB>0) P1pegB--;
            j++;
            if(j==longpress) break;
        }
        while(P1FWD & ~P1IN)
        {
            button_pressed = true;
            if(j==0 && cur1Peg) {P1pegA++; }
            if(j==0 && ~cur1Peg) {P1pegB++; }
            //wait for release and count for long press (+5)
            if(j==longpress)
            {
                if(cur1Peg) {P1pegA+=3; }
                if(~cur1Peg) {P1pegB+=3; }
                break;
            }
            j++;

        }
        while(P2REV & ~P1IN)
        {
            button_pressed = true;
            if(j==0 && P2pegA>P2pegB && P2pegA>0) P2pegA--;
            if(j==0 && P2pegB>P2pegA && P2pegB>0) P2pegB--;
            j++;
            if(j==longpress) break;
        }
        while(P2FWD & ~P1IN)
        {
            button_pressed = true;
            if(j==0 && cur2Peg) {P2pegA++; }
            if(j==0 && ~cur2Peg) {P2pegB++; }
            //wait for release and count for long press (+5)
            if(j==longpress)
            {
                if(cur2Peg) {P2pegA+=3; }
                if(~cur2Peg) {P2pegB+=3; }
                break;
            }
            j++;
            /*
            //debug
            P1OUT |= VSW + DATA;
            P2OUT |= LE1 + LE2;
            delay(delay100us);
            P1OUT &= ~VSW;
            P1OUT &= ~DATA; //10us from VSW to DATA LOW
            P2OUT &= ~LE1;
            P2OUT &= ~LE2;
            */
        }

        //update pegs and increment timer_i
        if(button_pressed)
        {
            show_pegs();
            button_pressed = false;
            main_time=0;
            peg_time=0;
        }
        //for(j=0; j<=delay20ms; j++);
        delay(delay20ms);
        main_time++;
        peg_time++;

        //change active peg after timeout
        if(peg_time == five_seconds)
        {
            P1OUT |= VSW;
            P1OUT &= ~VSW;
            cur1Peg = ~cur1Peg;
            if(P1pegA < P1pegB) P1pegA = P1pegB;
            else P1pegB = P1pegA;
            cur2Peg = ~cur2Peg;
            if(P2pegA < P2pegB) P2pegA = P2pegB;
            else P2pegB = P2pegA;
            peg_time = 0;
        }

        //check for winner

    }
    save_game();

}
// ******************************************************************************************
void show_pegs(void)
{
    //Show Player 1 Pegs
    P2OUT |= LE1;
    for(i=1; i<=120; i++)
    {
        if(i==P1pegA || i==P1pegB)
        {
            P1OUT |= DATA;
            delay(delay10us);
            P1OUT |= CLK;
            P1OUT &= ~DATA;
            P1OUT &= ~CLK;
        }
        else
        {
            P1OUT |= CLK;   //~420ns
            P1OUT &= ~CLK;
        }
    }
    P1OUT |= LE;
    P1OUT &= ~LE;   //~430ns
    P2OUT &= ~ LE1;

    //Show Player 2 Pegs
    P2OUT |= LE2;
    for(i=1; i<=120; i++)
    {
        if(i==P2pegA || i==P2pegB)
        {
            P1OUT |= DATA;
            delay(delay10us);
            P1OUT |= CLK;
            P1OUT &= ~DATA;
            P1OUT &= ~CLK;
        }
        else
        {
            P1OUT |= CLK;
            P1OUT &= ~CLK;
        }
    }
    P1OUT |= LE;
    P1OUT &= ~LE;
    P2OUT &= ~ LE2;

}

// ******************************************************************************************
void restart_game(void)
{

    //show reverse LED sweep pattern
    for(i=0; i<=120; i++)
    {
        P1pegA = 120-i;
        P2pegA = 120-i;
        P1pegB = 119-i;
        P2pegB = 119-i;
        //delay
        //for(j=0; j<=delay20ms; j++);
        delay(delay20ms);
        show_pegs();
    }
    //Reset Peg Values
    P1pts = 0;
    P1pegA = 0;
    P1pegB = 0;
    P2pts = 0;
    P2pegA = 0;
    P2pegB = 0;

}

// ******************************************************************************************
void save_game(void)
{
    //Store Peg Values to EEPROM
    //Disable VSW
    P1OUT &= ~VSW;
    //Enter Low Power Mode

    //temp place holder for debug
    wait_for_input();
}

// ******************************************************************************************
void show_winner_pattern()
{
    //Display Patter for Winner
}

// ******************************************************************************************
void delay(unsigned int dcount)
{
    unsigned int d;
    for(d=dcount; d>0; d--);
}
