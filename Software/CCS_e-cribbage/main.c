//******************************************************************************
//  MSP430F2012 LED cribbage board - software
//
//               MSP430F2012
//            -----------------
//        /|\|                 |
//         | |                 |
//         --|RST              |
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
//           |                 |
//           |                 |        -------------
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

//port1 bits
#define P1REV   BIT2                // S3
#define P1FWD   BIT3                // S4
#define P2REV   BIT0                // S1
#define P2FWD   BIT1                // S2
#define LE      BIT4                // RCLK
#define CLK     BIT5                // Clock
#define DATA    BIT6                // Data
#define VSW     BIT7                // 3V SW
//port2 bits
#define LE1     BIT6                // P1_EN
#define LE2     BIT7                // P2_EN

#define delay10us       1
#define delay100us      9
#define delay20ms       1850
#define five_minutes    20000
#define five_seconds    400
#define longpress       11000

//global definitions
unsigned int sleep_time=0;
unsigned int peg_time=0;
unsigned int button_time=0;
unsigned int playtime0=0;
unsigned int playtime1=0;
unsigned int P1pts = 0;
unsigned int P1pegA = 0;
unsigned int P1pegB = 0;
unsigned int P2pts = 0;
unsigned int P2pegA = 0;
unsigned int P2pegB = 0;
unsigned int P1gameswon = 0;
unsigned int P2gameswon = 0;
bool cur1Peg = false;
bool cur2Peg = false;
bool P1button_pressed = false;
bool P2button_pressed = false;
bool current_player1 = false;

//function declarations
void show_pegs(void);
void restart_game(void);
void restore_game(void);
void save_game(void);
void wait_for_input(void);
void show_winner_pattern(void);
void delay(unsigned int dcount);
void show_win_totals(void);
void show_playtimer(void);

// ******************************************************************************************
int main(void){
    // stop watchdog timer
    WDTCTL = WDTPW | WDTHOLD;

    //Configure Basic Clock
    BCSCTL1 = CALBC1_1MHZ;
    DCOCTL = CALDCO_1MHZ;

    //Peripheral Setup
    P1DIR |= LE + VSW + CLK + DATA;
    P2DIR |= LE1 + LE2;
    P2SEL = 0;
    P1IES |= P1REV + P1FWD + P2REV + P2FWD;

    //Enter Sleep
    P1OUT &= ~VSW;                            //disable VSW
    P1IE |= P1REV + P1FWD + P2REV + P2FWD;    //enable interupts for wake up
    _BIS_SR(LPM4_bits + GIE);                 // Enter LPM4 w/interrupt

    return 0;
}

// ******************************************************************************************
void wait_for_input(void){
    //enable VSW
    P1OUT |= VSW;
    delay(delay20ms);
    show_pegs();
    //start sleep timer
    sleep_time=0;

    while(sleep_time<five_minutes){
        //reset button timer
        button_time=0;

        //Detect Button Press, Port1_input=P1IN
        while(P1REV & ~P1IN){
            P1button_pressed = true;
            if(button_time==0 && P1pts>0) P1pts--;
            button_time++;
            if(button_time==longpress) break;
            if(P1REV & ~P1IN && P2REV & ~P1IN) restart_game();
        }
        while(P1FWD & ~P1IN){
            P1button_pressed = true;
            if(button_time==0) P1pts++;
            //wait for release and count for long press (+5)
            if(button_time==longpress)
            {
                P1pts+=3;
                break;
            }
            button_time++;
            if(P1FWD & ~P1IN && P2FWD & ~P1IN) save_game();
            if(P1FWD & ~P1IN && P1REV & ~P1IN) show_win_totals();
        }
        while(P2REV & ~P1IN){
            P2button_pressed = true;
            if(button_time==0 && P2pts>0) P2pts--;
            button_time++;
            if(button_time==longpress) break;
            if(P1REV & ~P1IN && P2REV & ~P1IN) restart_game();
        }
        while(P2FWD & ~P1IN){
            P2button_pressed = true;
            if(button_time==0) P2pts++;
            //wait for release and count for long press (+5)
            if(button_time==longpress){
                P2pts+=3;
                break;
            }
            button_time++;
            if(P1FWD & ~P1IN && P2FWD & ~P1IN) { restore_game(); P2pts--;}
            if(P2FWD & ~P1IN && P2REV & ~P1IN) show_playtimer();
        }
        //update pegs
        if(P1button_pressed || P2button_pressed){
            //alternate pegs and update peg values if points changed
            if(P1button_pressed){
                //switch pegs if player switched or peg_time > five_seconds
                if(current_player1==false || peg_time>five_seconds) cur1Peg ^= 1;
                if(cur1Peg) P1pegA = P1pts; else P1pegB = P1pts;
                if(P1pegA > P1pts) P1pegA = P1pts - 1;
                if(P1pegB > P1pts) P1pegB = P1pts - 1;
                if(P1pegA==P1pegB && cur1Peg) P1pegB--;
                if(P1pegA==P1pegB && ~cur1Peg) P1pegA--;
                current_player1 = true;
                }
            if(P2button_pressed){
                //switch pegs if player switched or peg_time > five_seconds
                if(current_player1==true  || peg_time>five_seconds) cur2Peg ^= 1;
                if(cur2Peg) P2pegA = P2pts; else P2pegB = P2pts;
                if(P2pegA > P2pts) P2pegA = P2pts - 1;
                if(P2pegB > P2pts) P2pegB = P2pts - 1;
                if(P2pegA==P2pegB && cur2Peg) P2pegB--;
                if(P2pegA==P2pegB && ~cur2Peg) P2pegA--;
                current_player1 = false;
                }
            P1button_pressed = false;
            P2button_pressed = false;
            sleep_time=0;
            peg_time=0;
            show_pegs();
        }
        //increment timers
        sleep_time++;
        peg_time++;
        playtime0++;
        if(playtime0==0xFFFF) playtime1++;
        delay(delay20ms);

        //check for winner
        if(P1pts >= 121 || P2pts >= 121) show_winner_pattern();
    }
    save_game();
    P1OUT &= ~VSW;                            //disable VSW
    P1IFG &= ~P1REV & ~P1FWD & ~P2REV & ~P2FWD;
    _BIS_SR(LPM4_bits + GIE);                 // Enter LPM4 w/interrupt
}

// ******************************************************************************************
void show_pegs(void){
    unsigned int i=0;

    //Show Player 1 Pegs
    P2OUT = LE1;
    for(i=120; i>0; i--){
        if(i==P1pegA || i==P1pegB){
            P1OUT |= DATA;
            delay(delay10us);
            P1OUT |= CLK;
            P1OUT &= ~DATA;
            P1OUT &= ~CLK;
        }
        else{
            P1OUT |= CLK;   //~5us
            P1OUT &= ~CLK;
        }
    }
    P1OUT |= LE;
    delay(delay10us);
    P1OUT &= ~LE;
    P2OUT = 0;

    //Show Player 2 Pegs
    P2OUT = LE2;
    for(i=120; i>0; i--){
        if(i==P2pegA || i==P2pegB){
            P1OUT |= DATA;
            delay(delay10us);
            P1OUT |= CLK;
            P1OUT &= ~DATA;
            P1OUT &= ~CLK;
        }
        else{
            P1OUT |= CLK;
            P1OUT &= ~CLK;
        }
    }
    P1OUT |= LE;
    delay(delay10us);
    P1OUT &= ~LE;
    P2OUT = 0;
}

// ******************************************************************************************
void restart_game(void){
    //prevent unsigned overflow
    if(P1pegA > 121) P1pegA=1;
    if(P1pegB > 121) P1pegB=1;
    if(P2pegA > 121) P2pegA=1;
    if(P2pegB > 121) P2pegB=1;
    //show reverse LED walk pattern
    while((P1pegA+P1pegB+P2pegA+P2pegB) > 0){
        if(P1pegA > 0) P1pegA--;
        if(P1pegB > 0) P1pegB--;
        if(P2pegA > 0) P2pegA--;
        if(P2pegB > 0) P2pegB--;
        delay(delay20ms);
        show_pegs();
    }
    //Reset Peg Values
    P1pts = 0;
    P2pts = 0;
}

// ******************************************************************************************

void restore_game(void){
    //read points and peg values from FLASH information memory section C
    //char  value;                // 8-bit value to write to segment C
    char *Flash_ptr;            // Flash pointer

    FCTL2 = FWKEY + FSSEL0 + FN1;             // MCLK/3 for Flash Timing Generator
    //value = 0;                                // initialize value
    Flash_ptr = (char *) 0x1040;              // Initialize Flash pointer

    // *** ints need to be cast to char ***
    P1pegA = *Flash_ptr++;
    P1pegB = *Flash_ptr++;
    P1pts  = *Flash_ptr++;
    P2pegA = *Flash_ptr++;
    P2pegB = *Flash_ptr++;
    P2pts  = *Flash_ptr++;
    show_pegs();
}

// ******************************************************************************************
void save_game(void){
    //store points and peg values to FLASH information memory section C
    //char  value;                // 8-bit value to write to segment C
    char *Flash_ptr;            // Flash pointer

    FCTL2 = FWKEY + FSSEL0 + FN1;             // MCLK/3 for Flash Timing Generator
    //value = 0;                                // initialize value
    Flash_ptr = (char *) 0x1040;              // Initialize Flash pointer
    FCTL1 = FWKEY + ERASE;                    // Set Erase bit
    FCTL3 = FWKEY;                            // Clear Lock bit
    *Flash_ptr = 0;                           // Dummy write to erase Flash segment

    FCTL1 = FWKEY + WRT;                      // Set WRT bit for write operation

    // *** ints need to be cast to char ***
    *Flash_ptr++ = P1pegA;
    *Flash_ptr++ = P1pegB;
    *Flash_ptr++ = P1pts;
    *Flash_ptr++ = P2pegA;
    *Flash_ptr++ = P2pegB;
    *Flash_ptr++ = P2pts;
    *Flash_ptr++ = playtime1;

    FCTL1 = FWKEY;                            // Clear WRT bit
    FCTL3 = FWKEY + LOCK;                     // Set LOCK bit
}

// ******************************************************************************************
void show_winner_pattern(){
    //display pattern for winner
    if(P1pts >= 121){
        P1gameswon++;
        if(P2pts<91) P1gameswon++; //skunk
        P1pegA = 1;
        P1pegB = 2;
        while(P1pegB <= 120){
            P1pegA++;
            P1pegB++;
            delay(delay20ms);
            show_pegs();
        }
    }
    if(P2pts >= 121){
        P2gameswon++;
        if(P1pts<91) P2gameswon++; //skunk
        P2pegA = 1;
        P2pegB = 2;
        while(P2pegB <= 120){
            P2pegA++;
            P2pegB++;
            delay(delay20ms);
            show_pegs();
        }
    }
    //reset points
    P1pts = 0;
    P2pts = 0;
    P1pegA = 121-P1gameswon;
    P1pegB = 0;
    P2pegA = 121-P2gameswon;
    P2pegB = 0;
    show_pegs();
}

// ******************************************************************************************
void show_win_totals(void){
    unsigned int tempA = P1pegA;
    unsigned int tempB = P1pegB;
    P1pegA = 121-P1gameswon;
    P2pegA = 121-P2gameswon;
    show_pegs();
    //wait for release
    while (P1FWD & ~P1IN || P1REV & ~P1IN);// _NOP();
    P1pegA = tempA;
    P2pegA = tempB;
    show_pegs();
}

// ******************************************************************************************
void show_playtimer(void){
    unsigned int tempA = P1pegA;
    unsigned int tempB = P1pegB;
    P1pegA = 121-playtime1/8;
    P2pegA = 121-playtime1%8;
    show_pegs();
    //wait for release
    while (P1FWD & ~P1IN || P1REV & ~P1IN);// _NOP();
    P1pegA = tempA;
    P2pegA = tempB;
    show_pegs();
}

// ******************************************************************************************
void delay(unsigned int dcount){
    unsigned int d;
    for(d=dcount; d>0; d--) _NOP();
}

// ******************************************************************************************
// Port 1 interrupt service routine
#pragma vector=PORT1_VECTOR
__interrupt void Port_1(void){
    //wait until button is released after wake up
    while(P1REV & ~P1IN || P1FWD & ~P1IN || P2REV & ~P1IN || P2FWD & ~P1IN);
    wait_for_input();
}
