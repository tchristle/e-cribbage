//******************************************************************************
//  MSP430F2012 LED cribbage board - software
//
//               MSP430F2012
//            -----------------
//        /|\|              XIN|-
//         | |                 |
//         --|RST          XOUT|-
//           |                 |
//           |             P1.0|-->LED
//           |                 |
//           |                 |     ^   LED TRACKS
//           |                 |     |  -------------
//           |                 |     |-|VDD          |
//           |          (8)P1.6|------>|DATA(18)     |
//           |          (7)P1.5|------>|CLK(22)      |
//           |                 |       |             |
//           |          (3)P1.1|------>|LE(20)       |
//           |                 |       |             |
//           |                 |
//
// 1 - VCC
// 2 - LED 			(P1.0)
// 3 - TX 			(P1.1)
// 4 - RX 			(P1.2)
// 5 - SPI_LE 		(P1.3)
// 6 - FPGA_RST		(P1.4)
// 7 - SPI_CLK 		(P1.5)
// 8 - SPI_DOUT 	(P1.6)
// 9 - SPI_DIN  	(P1.7)
// 10 - JTAG - RST
// 11 - JTAG - TCK
// 12 - NC			(P2.7)
// 13 - RF_BUF_EN	(P2.6)
// 14 - GND
//
//
//  Travis Christle
//  JUNE 2020
//  Built with CCS Version: 6.1.1
//******************************************************************************
//#include "msp430f2012.h"
#include "msp430.h"
#include "stdbool.h"

#define TXD  BIT1                   // TXD on P1.5 to uart module
#define RXD  BIT2                   // RXD on P1.1 from uart module
#define LED  BIT0					// LED
#define LE   BIT3					// Latch Enable
#define FRST BIT4					// FPGA Reset
#define RF_BUF_EN BIT6				// ADC CLOCK Enable

#define Sync 0x55                   // Start byte for BT app message
#define Rx88 0x01
#define Rx89 0x04

#define Bit_time 104                // 115200 Baud, SMCLK=12MHz (12MHz/115200)=104+1
#define Bit_time_5 52               // Time for half a bit.

unsigned int i=0, j=0, k=0, l=0;    // General purpose counters
unsigned int pause = 20000;         // General purpose delay
unsigned int BitCnt;                // Bit count, used when transmitting byte
unsigned int TXByte = 0x0055;       // Value sent over UART when Transmit() is called
unsigned int RXByte = 0x0000;       // Value recieved once hasRecieved is set
unsigned int RXBuff[5] = {0, 0, 0, 0, 0};
unsigned int SPI_readback = 0;

bool isReceiving = false;           // Status for when the device is receiving
bool hasReceived = false;           // Lets the program know when a byte is received
bool echo = true;
bool temp = true;

// Function Definitions
void Transmit(void);
void Send_SPI(unsigned int reg, unsigned int data);
void burst_read(void);

// ******************************************************************************************

void main(void)
{

WDTCTL = WDTPW + WDTHOLD;   		        // Stop WDT

//Configure Basic Clock
BCSCTL1 = CALBC1_12MHZ;               		// Set range
DCOCTL = CALDCO_12MHZ;              		// SMCLK = DCO = 8MHz

//UART Setup
P1SEL = TXD;
P1OUT = TXD + LE + FRST;
P1DIR |= TXD + LED;
P1IES |= RXD;                      			 // RXD Hi/lo edge interrupt
P1IFG &= ~RXD;                		         // Clear RXD (flag) before enabling interrupt
P1IE |= RXD;                 			     // Enable RXD interrupt
P2OUT = RF_BUF_EN;
P2DIR = RF_BUF_EN;
P2SEL = 0;

//SPI Setup
P1DIR |= LE + FRST;                           		    // Set P1.1 for sync output
USICTL0 |= USIPE7 + USIPE6 + USIPE5 + USIMST + USIOE;   // Port, SPI Master
//USICTL1 |= USIIE; //USICKPH + USIIE;      		    // Counter interrupt, flag remains set
USICKCTL = USIDIV_2 + USISSEL_2; 				        // /2 SMCLK
USICTL0 &= ~USISWRST;                     		    	// USI released for operation

//Startup Blinky
for(j=0; j<4; j++)                	      	 	        // Blink green LED for power up & ready
{
	P1OUT |= LED;
	for(i=1; i<=pause; i++);
	P1OUT ^= LED;
	for(i=1; i<=pause; i++);
}

__bis_SR_register(GIE);

while(1)                          			 // Begin loop
{

	if(hasReceived)
	{
		TXByte = RXByte;
		hasReceived = false;
		Transmit();
		RXBuff[4]=RXBuff[3];
		RXBuff[3]=RXBuff[2];
		RXBuff[2]=RXBuff[1];
		RXBuff[1]=RXBuff[0];
		if(RXByte < 58 && RXByte > 47) RXBuff[0]=RXByte-48;									// char 0 - 9
		else if(RXByte < 103 && RXByte > 96) RXBuff[0]=RXByte-87;							// char a - f
		else if(RXByte == 82) {P1OUT ^= FRST; TXByte = 00; Transmit(); P1OUT ^= FRST;}		// char R RESET FPGA
		else if(RXByte == 81) {P2OUT ^= RF_BUF_EN; }										// char Q RF_BUF_EN
		else if(RXByte == 87) {echo = false; }												// char W echo off
		else if(RXByte == 119) {echo = true; }												// char w echo on
		else if(RXByte == 66) {burst_read();}												// char B BURST READ
		else if(RXByte == 13) RXBuff[0]=16;													// char ENTER
		else RXBuff[0]=17;																	// invalid char

		if(RXBuff[0]==16 && (RXBuff[1]==16 || RXBuff[2]==16 || RXBuff[3]==16 || RXBuff[4]==16))	//clear RXBuff on early ENTER press
		{
			RXBuff[4]=16;
			RXBuff[3]=16;
			RXBuff[2]=16;
			RXBuff[1]=16;
			RXBuff[0]=16;
		}
		else if(RXBuff[0]==17)										// trap invalid characters with error message
		{
			RXBuff[4]=16;
			RXBuff[3]=16;
			RXBuff[2]=16;
			RXBuff[1]=16;
			RXBuff[0]=16;
			TXByte = 10; Transmit(); //nl
			TXByte = 13; Transmit(); //cr
			TXByte = 63; Transmit(); //?
			TXByte = 10; Transmit(); //nl
			TXByte = 13; Transmit(); //cr
		}
		else if(RXBuff[0]==16 && RXBuff[1]!=16 && RXBuff[2]!=16 && RXBuff[3]!=16 && RXBuff[4]!=16)	// write SPI when input buffer is valid
		{
			Send_SPI((RXBuff[4] << 4) + RXBuff[3], (RXBuff[2] << 4) + RXBuff[1]);
			TXByte = 10; Transmit();
			TXByte = 13; Transmit();
			if ((RXBuff[4] & 0x8) == 0)																//if readback is set
			{
				if ((SPI_readback >> 4) <= 9)
					TXByte = (SPI_readback >> 4) + 48;
				else
					TXByte = (SPI_readback >> 4) + 87;
				temp = echo;
				echo = true;
				Transmit();
				if ((SPI_readback & 0xF) <= 9)
					TXByte = (SPI_readback & 0xF) + 48;
				else
					TXByte = (SPI_readback & 0xF) + 87;
				Transmit();
				echo = temp;
				TXByte = 10; Transmit();
				TXByte = 13; Transmit();
			}
		}

	}
	else
	{
		TXByte = 0x55; //Transmit();
	}

}
}

// ****************************************************************************************
void Send_SPI(unsigned int reg, unsigned int data)
{

	P1OUT &= ~LE;                         // Latch EN

	USISRL = reg;			   			    // set register 1st byte
	USICNT = 8;                             // re-load counter
	while (!(USIIFG & USICTL1));            // Counter clear?
	USISRL = data;			 			    // set register 2nd byte
	USICNT = 8;                             // re-load counter
	while (!(USIIFG & USICTL1));            // Counter clear?

	P1OUT |= LE;

	SPI_readback = USISRL;

}
//*****************************************************************************************
void Transmit()
{
	if(echo) {
		while(isReceiving);                // Wait for RX completion
		CCTL0 = OUT;                       // TXD Idle as Mark
		TACTL = TASSEL_2 + MC_2;           // SMCLK, continuous mode

		BitCnt = 0xA;                      // Load Bit counter, 8 bits + ST/SP
		CCR0 = TAR;                        // Initialize compare register

		CCR0 += Bit_time;                  // Set time till first bit
		TXByte |= 0x100;                   // Add stop bit to TXByte (which is logical 1)
		TXByte = TXByte << 1;              // Add start bit (which is logical 0)

		CCTL0 = CCIS0 + OUTMOD0 + CCIE;    // Set signal, intial value, enable interrupts
		while ( CCTL0 & CCIE );            // Wait for previous TX completion
	}
}
//*****************************************************************************************
void burst_read()
{
	unsigned int reg1, reg2, reg3, reg4, reg5;

	P1OUT &= ~LE;                         // Latch EN

	USISRL = 0x21;			   			    // set register 1st byte
	USICNT = 8;                             // re-load counter
	while (!(USIIFG & USICTL1));            // Counter clear?

	USISRL = 0x00;			 			    // set register 2nd byte
	USICNT = 8;                             // re-load counter
	while (!(USIIFG & USICTL1));            // Counter clear?
	reg1 = USISRL;

	USISRL = 0x00;			 			    // set register 2nd byte
	USICNT = 8;                             // re-load counter
	while (!(USIIFG & USICTL1));            // Counter clear?
	reg2 = USISRL;

	USISRL = 0x00;			 			    // set register 2nd byte
	USICNT = 8;                             // re-load counter
	while (!(USIIFG & USICTL1));            // Counter clear?
	reg3 = USISRL;

	USISRL = 0x00;			 			    // set register 2nd byte
	USICNT = 8;                             // re-load counter
	while (!(USIIFG & USICTL1));            // Counter clear?
	reg4 = USISRL;

	USISRL = 0x00;			 			    // set register 2nd byte
	USICNT = 8;                             // re-load counter
	while (!(USIIFG & USICTL1));            // Counter clear?
	reg5 = USISRL;

	P1OUT |= LE;

// reg1 print

	if ((reg1 >> 4) <= 9)
		TXByte = (reg1 >> 4) + 48;
	else
		TXByte = (reg1 >> 4) + 87;
	Transmit();
	if ((reg1 & 0xF) <= 9)
		TXByte = (reg1 & 0xF) + 48;
	else
		TXByte = (reg1 & 0xF) + 87;
	Transmit();
// reg2 print
	if ((reg2 >> 4) <= 9)
		TXByte = (reg2 >> 4) + 48;
	else
		TXByte = (reg2 >> 4) + 87;
	Transmit();
	if ((reg2 & 0xF) <= 9)
		TXByte = (reg2 & 0xF) + 48;
	else
		TXByte = (reg2 & 0xF) + 87;
	Transmit();
// reg3 print
	if ((reg3 >> 4) <= 9)
		TXByte = (reg3 >> 4) + 48;
	else
		TXByte = (reg3 >> 4) + 87;
	Transmit();
	if ((reg3 & 0xF) <= 9)
		TXByte = (reg3 & 0xF) + 48;
	else
		TXByte = (reg3 & 0xF) + 87;
	Transmit();
// reg4 print
	if ((reg4 >> 4) <= 9)
		TXByte = (reg4 >> 4) + 48;
	else
		TXByte = (reg4 >> 4) + 87;
	Transmit();
	if ((reg4 & 0xF) <= 9)
		TXByte = (reg4 & 0xF) + 48;
	else
		TXByte = (reg4 & 0xF) + 87;
	Transmit();
//	reg5 print
	if ((reg5 >> 4) <= 9)
		TXByte = (reg5 >> 4) + 48;
	else
		TXByte = (reg5 >> 4) + 87;
	Transmit();
	if ((reg5 & 0xF) <= 9)
		TXByte = (reg5 & 0xF) + 48;
	else
		TXByte = (reg5 & 0xF) + 87;
	Transmit();
//
	TXByte = 10; Transmit();
	TXByte = 13; Transmit();

}

// ****************************************************************************************
// Port 1 interrupt service routine
#pragma vector=PORT1_VECTOR
__interrupt void Port_1(void)
{
	isReceiving = true;

	P1IE &= ~RXD;                       // Disable RXD interrupt
	P1IFG &= ~RXD;                      // Clear RXD IFG (interrupt flag)

	TACTL = TASSEL_2 + MC_2;            // SMCLK, continuous mode
	CCR0 = TAR;                         // Initialize compare register
	CCR0 += Bit_time_5;                 // Set time till first bit
	CCTL0 = OUTMOD1 + CCIE;             // Dissable TX and enable interrupts
	//CCTL0 = OUTMOD2 + CCIE;             // RX debug setting

	RXByte = 0;                         // Initialize RXByte
	BitCnt = 0x9;                       // Load Bit counter, 8 bits + ST
}  

// ******************************************************************************************
// Timer A0 interrupt service routine
#pragma vector=TIMERA0_VECTOR
__interrupt void Timer_A (void)
{
	CCR0 += Bit_time;                   // Add Offset to CCR0

	if(!isReceiving)                    // Transmitting ---------------------------------------
		{
		if ( BitCnt == 0)               // If all bits TXed
			{
			TACTL = TASSEL_2;           // SMCLK, timer off (for power consumption)
			CCTL0 &= ~ CCIE ;           // Disable interrupt
			}
		else
			{
			CCTL0 |= OUTMOD2;           // Set TX bit to 0
			if (TXByte & 0x01)
				CCTL0 &= ~ OUTMOD2;     // If it should be 1, set it to 1
			TXByte = TXByte >> 1;
			BitCnt --;
			}
		}                                // -----------------------------------------------------
	else                                 // Receiving -------------------------------------------
		{
		if ( BitCnt == 1)
			{
			TACTL = TASSEL_2;            // SMCLK, timer off (for power consumption)
			CCTL0 &= ~ CCIE ;            // Disable interrupt

			RXByte = RXByte >> 2;        // Remove start bit

			isReceiving = false;

			P1IFG &= ~RXD;               // clear RXD IFG (interrupt flag)
			P1IE |= RXD;                 // enabled RXD interrupt

	//        if ((RXByte & 0x201) == 0x200) // Validate the start and stop bits are correct
	//            {
	//            RXByte = RXByte >> 1;     // Remove start bit
	//            RXByte &= 0xFF;           // Remove stop bit
				hasReceived = true;
	//            }
	//         __bic_SR_register_on_exit(CPUOFF);     // Enable CPU so the main while loop continues
			}
	else
		{
		if ( (P1IN & RXD) == RXD)        // If bit is set?
			RXByte |= 0x400;             // Set the value in the RXByte
		RXByte = RXByte >> 1;            // Shift the bits down
		BitCnt --;
		}
		}                                // ------------------------------------------------------
}

