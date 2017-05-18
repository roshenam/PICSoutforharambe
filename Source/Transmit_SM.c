/****************************************************************************
 Module
   Transmit_SM.c

 Description
   Transmit state machine 

 History
 When           Who     What/Why
 -------------- ---     --------
 05/14/2017			SC
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

#include "Comm_Service.h"
#include "Transmit_SM.h"
#include "Receive_SM.h"
#include "UART.h"
#include "Constants.h"

/*----------------------------- Module Defines ----------------------------*/
#define TRANSMIT_TIMER_LENGTH 10 // based off of 9600 baud rate (each character takes ~1.04ms to send)

#define MAX_FRAME_LENGTH 40 // max number of bytes we expect to receive for any data type (including frame overhead)


/*---------------------------- Module Functions ---------------------------*/
bool IsLastByte(void);
static void SendByte(uint8_t DataByte);

/*---------------------------- Module Variables ---------------------------*/
static TransmitState_t CurrentState;

static uint8_t MyPriority;


static uint8_t* DataToSend; // pointer to array containing all bytes in data packet
static uint8_t DataPacketLength = 0;
static uint8_t index = 0;
static bool LastByteFlag = 0;


/*------------------------------ Module Code ------------------------------*/
/****************************************************************************
 Function
     InitTransmit_SM

 Parameters
     uint8_t : the priorty of this service

 Returns
     bool, false if error in initialization, true otherwise

 Description
     Saves away the priority, sets up the initial transition and does any
     other required initialization for this state machine
 Notes

 Author
     Sarah Cabreros
****************************************************************************/
bool InitTransmit_SM ( uint8_t Priority )
{
  ES_Event ThisEvent;

  MyPriority = Priority;
  // put us into the Initial PseudoState
  CurrentState = InitTransmit;
	
  // post the initial transition event
  ThisEvent.EventType = ES_INIT;
	printf("Initialized in Transmit_SM\r\n");
  if (ES_PostToService( MyPriority, ThisEvent) == true)
  {
      return true;
  }else
  {
      return false;
  }
}

/****************************************************************************
 Function
     PostTransmit_SM

 Parameters
     EF_Event ThisEvent , the event to post to the queue

 Returns
     boolean False if the Enqueue operation failed, True otherwise

 Description
     Posts an event to this state machine's queue
 Notes

 Author
     Sarah Cabreros
****************************************************************************/
bool PostTransmit_SM( ES_Event ThisEvent )
{
  return ES_PostToService( MyPriority, ThisEvent);
}

/****************************************************************************
 Function
    RunTransmit_SM

 Parameters
   ES_Event : the event to process

 Returns
   ES_Event, ES_NO_EVENT if no error ES_ERROR otherwise

 Description
   add your description here
 Notes
   uses nested switch/case to implement the machine.
 Author
   Sarah Cabreros
****************************************************************************/
ES_Event RunTransmit_SM( ES_Event ThisEvent )
{
  ES_Event ReturnEvent;
  ReturnEvent.EventType = ES_NO_EVENT; // assume no errors

  switch ( CurrentState )
  {
    case InitTransmit :       
        if ( ThisEvent.EventType == ES_INIT )// only respond to ES_Init
        {
            // set current state to Idle
            CurrentState = Idle;

            // set index to zero
            index = 0; 

            // get pointer to array of message
            DataToSend = GetDataPacket_Tx(); // function from Comm_Service
         }
    break;

    case Idle:      
			// waiting to receive Start_Xmit event from Comm_Service
			if ( ThisEvent.EventType == ES_START_XMIT ) {
				
				// clear LastByteFlag
        LastByteFlag = 0;
				
				// get length of array 
				DataPacketLength = ThisEvent.EventParam /*framelength*/ + HEADER_LENGTH + 1 /*checksum bit*/; 
				
				// send first byte of array 
				uint8_t CurrentByte = *(DataToSend+index);
				SendByte(CurrentByte);
				printf("start xmit: %i\n\r ", CurrentByte);

				// increment index 
				index++;

				// enable TXIM interrupts
				HWREG(UART7_BASE + UART_O_IM) |= UART_IM_TXIM; 

				// start timer 
				ES_Timer_InitTimer(TRANSMIT_TIMER, TRANSMIT_TIMER_LENGTH);
					
				// set current state to SendingData
				CurrentState = SendingData;
			}
			
    break;

		case SendingData:      
			if ( ThisEvent.EventType == ES_TIMEOUT && ThisEvent.EventParam == TRANSMIT_TIMER ) {
				// go back to Idle
				CurrentState = Idle;
			}
			
			if ( ThisEvent.EventType == ES_BYTE_SENT) { // from UART ISR
				//printf("woohoo byte sent\r\n");
				
				// if index = length of array, we are done sending data
				if (index == DataPacketLength) {
					printf("sent all bytes \n\r");
					// set LastByteFlag
					LastByteFlag = 1;

					// set index back to 0
					index = 0;
					
					// go back to Idle 
					CurrentState = Idle;
				} else {
					// send next byte of array 
					uint8_t CurrentByte = *(DataToSend+index);
					SendByte(CurrentByte);
					printf("xmit data: %i\n\r", CurrentByte);
					
					// increment index 
					index++;

					// start transmit timer
					ES_Timer_InitTimer(TRANSMIT_TIMER, TRANSMIT_TIMER_LENGTH);

				}	
			}
		
    break;
		
    default :
      ;
  }                                   // end switch on Current State
  return ReturnEvent;
}

static void SendByte(uint8_t DataByte) {
	// Check if room in FIFO
	if((HWREG(UART7_BASE + UART_O_FR)&UART_FR_TXFE) != 0){
		// write data to DR 
		HWREG(UART7_BASE + UART_O_DR) = DataByte; 
		printf("Sending byte: %i\r\n", DataByte);
	}else{
		printf("Fifo not empty\r\n");
	}	
}

bool IsLastByte(void) {
	if (LastByteFlag == 1) return true;
	else return false;
}
