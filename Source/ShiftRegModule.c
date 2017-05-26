/****************************************************************************
 Module
   ShiftRegisterWrite.c

 Revision
   1.0.1

 Description
   This module acts as the low level interface to a write only shift register.

 Notes

 History
 When           Who     What/Why
 -------------- ---     --------
 10/11/15 19:55 jec     first pass
 
****************************************************************************/
#define TEST

// the common headers for C99 types 
#include <stdint.h>
#include <stdbool.h>

// the headers to access the GPIO subsystem
#include "inc/hw_memmap.h"
#include "inc/hw_types.h"
#include "inc/hw_gpio.h"
#include "inc/hw_sysctl.h"

// the headers to access the TivaWare Library
#include "driverlib/sysctl.h"
#include "driverlib/pin_map.h"
#include "driverlib/gpio.h"
#include "driverlib/timer.h"
#include "driverlib/interrupt.h"

#include "BITDEFS.H"
#include "termio.h"
#include "EnablePA25_PB23_PD7_PF0.h"

// readability defines
#define DATA GPIO_PIN_6

#define SCLK GPIO_PIN_2
#define SCLK_HI BIT2HI
#define SCLK_LO BIT2LO

#define RCLK GPIO_PIN_0
#define RCLK_LO BIT0LO
#define RCLK_HI BIT0HI

#define GET_MSB_IN_LSB(x) ((x & 0x80)>>7)
#define ALL_BITS (0xff<<2)

// an image of the last 8 bits written to the shift register
static uint8_t LocalRegisterImage=0;

// Create your own function header comment
void SR_Init(void){

  // set up port B by enabling the peripheral clock and setting the direction
  // of PB0, PB1 & PB2 to output
	HWREG(SYSCTL_RCGCGPIO) |= SYSCTL_RCGCGPIO_R1;
	while((HWREG(SYSCTL_PRGPIO) & SYSCTL_PRGPIO_R1) != SYSCTL_PRGPIO_R1) 
		;
	//set bits 0,1,2 on port B to be an output
	HWREG(GPIO_PORTB_BASE + GPIO_O_DEN) |= (DATA | SCLK );
	HWREG(GPIO_PORTB_BASE + GPIO_O_DIR) |= (DATA | SCLK );
	
	HWREG(SYSCTL_RCGCGPIO) |= SYSCTL_RCGCGPIO_R5;
	while((HWREG(SYSCTL_PRGPIO) & SYSCTL_PRGPIO_R5) != SYSCTL_PRGPIO_R5) 
	
	HWREG(GPIO_PORTF_BASE + GPIO_O_DEN) |= ( RCLK );
	HWREG(GPIO_PORTF_BASE + GPIO_O_DIR) |= ( RCLK );
	
  // start with the data & sclk lines low and the RCLK line high
	HWREG(GPIO_PORTF_BASE + (GPIO_O_DATA+ALL_BITS)) |= RCLK_HI;
	HWREG(GPIO_PORTB_BASE + (GPIO_O_DATA+ALL_BITS)) &= SCLK_LO;
	HWREG(GPIO_PORTB_BASE + (GPIO_O_DATA+ALL_BITS)) &= ~DATA;
}

// Create your own function header comment
uint8_t SR_GetCurrentRegister(void){
  return LocalRegisterImage;
}

// Create your own function header comment
void SR_Write(uint8_t NewValue){
  uint8_t BitCounter;
  LocalRegisterImage = NewValue; // save a local copy

	// lower the register clock
	HWREG(GPIO_PORTF_BASE + (GPIO_O_DATA+ALL_BITS)) &= RCLK_LO;
	
	// for loop: shift out the data while pulsing the serial clock  
	for (int i = 0; i<8; i++) {
		printf("%i\r\n",i);
		// Isolate the MSB of NewValue, put it into the LSB position and output
		if((NewValue & BIT7HI) != 0) {//MSB is high
			HWREG(GPIO_PORTB_BASE + GPIO_O_DATA+ALL_BITS) |= DATA; //set DATA line high 
		} else { //MSB is low
			HWREG(GPIO_PORTB_BASE + GPIO_O_DATA+ALL_BITS) &= ~DATA; //set DATA line low
		}
		// raise SCLK
		HWREG(GPIO_PORTB_BASE + (GPIO_O_DATA+ALL_BITS)) |= SCLK_HI;
		// lower SCLK
		HWREG(GPIO_PORTB_BASE + (GPIO_O_DATA+ALL_BITS)) &= SCLK_LO;
		
		//shift byte
		NewValue <<= 1;
	}
	// raise the register clock to latch the new data
  HWREG(GPIO_PORTF_BASE + (GPIO_O_DATA+ALL_BITS)) |= RCLK_HI;
}


// Test harness for shift register functions
/*#ifdef TEST
int main(void){
	TERMIO_Init();
	puts("Testing shift register module\r\n");
	SR_Init();
	uint8_t testByte = (BIT0HI | BIT3HI | BIT5HI);                                  ;
	SR_Write(testByte);
	//uint8_t returnedByte = SR_GetCurrentRegister();
	return 0;
}
#endif*/