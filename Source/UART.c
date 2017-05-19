/****************************************************************************
 Module
   UART.c

 Description
   UART Initialization and ISR functions

 Notes

 History
 When           Who     What/Why
 -------------- ---     --------
05/13/2017			SC	
****************************************************************************/

/*----------------------------- Include Files -----------------------------*/
#include "ES_Configure.h"
#include "ES_Framework.h"
#include "ES_DeferRecall.h"

#include "inc/hw_memmap.h"
#include "inc/hw_types.h"
#include "inc/hw_gpio.h"
#include "inc/hw_sysctl.h"
#include "inc/hw_nvic.h"
#include "inc/hw_uart.h"
#include "driverlib/sysctl.h"
#include "driverlib/pin_map.h"	// Define PART_TM4C123GH6PM in project
#include "driverlib/gpio.h"

#include "Constants.h"
#include "Hardware.h"
#include "Transmit_SM.h"
#include "Receive_SM.h"

/*----------------------------- Module Defines ----------------------------*/
//#define RX_PIN	BIT0HI 	// UART7 Rx: PE0
//#define TX_PIN	BIT1HI	// UART7 Tx: PE1

#define ALL_BITS (0xff<<2)

/*---------------------------- Module Variables ---------------------------*/
static uint8_t DataByte; 

/*------------------------------ Module Code ------------------------------*/
/****************************************************************************
 Function
     InitUART

 Description
     Initializes UART7 pins, GPIO pins, and UART interrupts

 Author
     Sarah Cabreros
****************************************************************************/
void InitUART(void) {

	// 1. Enable clock to UART module using RCGCUART register
	HWREG(SYSCTL_RCGCUART) |= SYSCTL_RCGCUART_R5;
	
	// 2. Wait for the UART to be ready (PRUART)
	while ((HWREG(SYSCTL_PRUART) & SYSCTL_PRUART_R5 ) != SYSCTL_PRUART_R5 )
		;
	
	// 3. Enable the clock to GPIO Port E (RCGCGPIO)
	HWREG(SYSCTL_RCGCGPIO) |= SYSCTL_RCGCGPIO_R4;
	
	// 4. Wait for GPIO module to be ready (PRGPIO)
	while ((HWREG(SYSCTL_PRGPIO) & SYSCTL_PRGPIO_R4 ) != SYSCTL_PRGPIO_R4 )
		;
	
	// 5. Configure GPIO pins: PE4 = digital input, PE5 = digital output
	HWREG(GPIO_PORTE_BASE + GPIO_O_DEN) |= (RX_PIN | TX_PIN);
	HWREG(GPIO_PORTE_BASE + GPIO_O_DIR) |= TX_PIN;
	HWREG(GPIO_PORTE_BASE + GPIO_O_DIR) &= ~RX_PIN;
	
	// 6. Select alternate function for UART pins (AFSEL)
	HWREG(GPIO_PORTE_BASE + GPIO_O_AFSEL) |= (RX_PIN | TX_PIN);
	
	// 7. Configure PMC0-1 fields in GPIOPCTL to assign the UART to PE0-1
	//HWREG(GPIO_PORTE_BASE+GPIO_O_PCTL) =
	//	(HWREG(GPIO_PORTE_BASE+GPIO_O_PCTL) & 0xffffff00) + (1<<0*4) + (1<<1*4);
	HWREG(GPIO_PORTE_BASE+GPIO_O_PCTL) = 
		(HWREG(GPIO_PORTE_BASE+GPIO_O_PCTL) & 0xfffffff0) + (1<<4*4); //Rx pin 
	HWREG(GPIO_PORTE_BASE+GPIO_O_PCTL) =
		(HWREG(GPIO_PORTE_BASE+GPIO_O_PCTL) & 0xffffff0f) + (1<<5*4); //Tx pin
	
	// 8. Disable UART (clear UARTEN bit in UARTCTL)
	HWREG(UART5_BASE + UART_O_CTL) &= ~UART_CTL_UARTEN;
	
	// 9. Write integer portion of BRD to UARTIBRD (Baud rate 9600, Integer = 260, Fraction = 27)
	HWREG(UART5_BASE + UART_O_IBRD) = 260; //0x0104;

	// 10. Write fractional portion of BRD to UARTFBRD
	HWREG(UART5_BASE + UART_O_FBRD) = 27; //0x1B;
	
	// 11. Write desired serial parameters to UARTLCRH
	HWREG(UART5_BASE + UART_O_LCRH) |= (BIT5HI | BIT6HI); // 8-bit word length, all other bits zero
	
	// 12. Configure UART operation using UARTCTL register 
	HWREG(UART5_BASE + UART_O_CTL) |= (UART_CTL_RXE | UART_CTL_TXE);// | UART_CTL_EOT); // Enable Receive and Transmit
	
	// 13. Enable UART by setting UARTEN bit in UARTCTL 
	HWREG(UART5_BASE + UART_O_CTL) |= UART_CTL_UARTEN;

	// locally enable RX interrupts
	HWREG(UART5_BASE + UART_O_IM) |= UART_IM_RXIM; 
	
	// set NVIC enable for UART5
	HWREG(NVIC_EN1) |= BIT29HI;
	
	// globally enable interrupts 
	__enable_irq();

} 

/****************************************************************************
 Function
     UART_ISR

 Description
     Responds to RXIM or TXIM interrupts

 Author
     Sarah Cabreros
****************************************************************************/

 void UART_ISR(void) {

  // if RXMIS set:
 	if ((HWREG(UART5_BASE+UART_O_MIS) & UART_MIS_RXMIS) == UART_MIS_RXMIS) {
		//printf("r\r\n");
		// save new data byte
		DataByte = HWREG(UART5_BASE + UART_O_DR); 

		// clear interrupt flag (set RXIC in UARTICR)
		HWREG(UART5_BASE + UART_O_ICR) |= UART_ICR_RXIC;

		// post ByteReceived event to Receive_SM (event param is byte in UARTDR) 
		ES_Event ThisEvent;
		ThisEvent.EventType = ES_BYTE_RECEIVED;
		ThisEvent.EventParam = DataByte;
		PostReceive_SM(ThisEvent); 
 	}

	// else if TXMIS set (FIFO open): // where do we enable TXIM interrupts??? 
	else if ((HWREG(UART5_BASE+UART_O_MIS) & UART_MIS_TXMIS) == UART_MIS_TXMIS) {
	// should get this interrupt for all bytes AFTER the start byte (0x7E) 
		//printf("t\r\n");
		
		// clear interrupt flag 
		HWREG(UART5_BASE + UART_O_ICR) |= UART_ICR_TXIC;

		// post ByteSent event 
		ES_Event ThisEvent;
		ThisEvent.EventType = ES_BYTE_SENT;
		PostTransmit_SM(ThisEvent);
		
		// if this was last byte in message block
		if (IsLastByte()) { // IsLastByte from Transmit_SM
			// disable TXIM 
			HWREG(UART5_BASE + UART_O_IM) &= ~UART_IM_TXIM;
		}
		
	}
}






