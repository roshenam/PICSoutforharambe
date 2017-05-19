/****************************************************************************
 Module
   Receive_SM.c

 Description
   Receiving state machine 

 History
 When           Who     What/Why
 -------------- ---     --------
 05/13/2017			SC
****************************************************************************/
/*----------------------------- Include Files -----------------------------*/
#include "ES_Configure.h"
#include "ES_Framework.h"
#include "ES_DeferRecall.h"

#include "Constants.h"
#include "Hardware.h"

/*----------------------------- Module Defines ----------------------------*/

#define RECEIVE_TIMER_LENGTH 10 // based off of 9600 baud rate (each character takes ~1.04ms to send)

#define MAX_FRAME_LENGTH 40 // max number of bytes we expect to receive for any data type 


/*---------------------------- Module Functions ---------------------------*/


/*---------------------------- Module Variables ---------------------------*/
static ReceiveState_t CurrentState;

static uint8_t MyPriority;

static uint8_t MSBLength = 0;
static uint8_t LSBLength = 0;
static uint8_t FrameLength = 0; // num bytes in data frame
static uint8_t BytesLeft = 0;
static uint8_t CheckSum = 0;

static uint8_t DataPacket[MAX_FRAME_LENGTH]; // array containing all bytes in data packet
static uint8_t ArrayIndex = 0;

static uint8_t *outgoingDataPacket; //pointer to data packet



/*------------------------------ Module Code ------------------------------*/
/****************************************************************************
 Function
     InitReceive_SM

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
bool InitReceive_SM ( uint8_t Priority )
{
  ES_Event ThisEvent;

  MyPriority = Priority;
  // put us into the Initial PseudoState
  CurrentState = InitReceive;
	
	outgoingDataPacket = &DataPacket[0]; // address of first entry in data packet array
	
  // post the initial transition event
  ThisEvent.EventType = ES_INIT;
	printf("Initialized in Receive_SM\r\n");
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
     PostReceive_SM

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
bool PostReceive_SM( ES_Event ThisEvent )
{
  return ES_PostToService( MyPriority, ThisEvent);
}

/****************************************************************************
 Function
    RunReceive_SM

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
ES_Event RunReceive_SM( ES_Event ThisEvent )
{
  ES_Event ReturnEvent;
  ReturnEvent.EventType = ES_NO_EVENT; // assume no errors

  switch ( CurrentState )
  {
    case InitReceive :       
        if ( ThisEvent.EventType == ES_INIT )// only respond to ES_Init
        {
            // initialize UART
						InitUART();

						outgoingDataPacket = &DataPacket[0];
					
            // set current state to Wait4Start 
            CurrentState = Wait4Start;
         }
    break;

    case Wait4Start:      
			// waiting to receive 0x7E 
			if ( ThisEvent.EventType == ES_BYTE_RECEIVED ) {
				//printf("wait4start : %i\r\n", ThisEvent.EventParam);
				// check if byte received is 0x7E 
				if ( ThisEvent.EventParam == START_DELIMITER ) {
					printf("------------RECEIVING-------------------\n\r");
					printf("Start: %i\n\r", ThisEvent.EventParam);
					// start timer 
					ES_Timer_InitTimer(RECEIVE_TIMER, RECEIVE_TIMER_LENGTH);
					
					// set current state to Wait4MSB
					CurrentState = Wait4MSBLength;
				}
			}
      
    break;

		case Wait4MSBLength:      
			if ( ThisEvent.EventType == ES_TIMEOUT && ThisEvent.EventParam == RECEIVE_TIMER ) {
				// go back to Wait4Start
				CurrentState = Wait4Start;
			}
			
			if ( ThisEvent.EventType == ES_BYTE_RECEIVED ) {
				printf("wait4msb : %i\r\n", ThisEvent.EventParam);
				// store MSB in data packet 
				MSBLength = ThisEvent.EventParam; 
				// start receive timer
				ES_Timer_InitTimer(RECEIVE_TIMER, RECEIVE_TIMER_LENGTH);
				// set current state to Wait4LSB
				CurrentState = Wait4LSBLength;
			}
		
    break;
		
		case Wait4LSBLength: 
			if ( ThisEvent.EventType == ES_TIMEOUT && ThisEvent.EventParam == RECEIVE_TIMER ) {
				// go back to Wait4Start
				CurrentState = Wait4Start;
			}		

			if ( ThisEvent.EventType == ES_BYTE_RECEIVED ) {
				printf("wait4lsb : %i\r\n", ThisEvent.EventParam);
				// store LSB in data packet 
				LSBLength = ThisEvent.EventParam; 
				
				// update FrameLength and BytesLeft
				FrameLength = MSBLength + LSBLength;
				BytesLeft = FrameLength;
				
				// start receive timer
				ES_Timer_InitTimer(RECEIVE_TIMER, RECEIVE_TIMER_LENGTH);
				
				// set ArrayIndex to 0 
				ArrayIndex = 0;
				
				// initialize CheckSum to 0
				CheckSum = 0;
				
				// set current state to ReceivingData
				CurrentState = ReceivingData;
			}			
      
    break;
		
		case ReceivingData: 
			if ( ThisEvent.EventType == ES_TIMEOUT && ThisEvent.EventParam == RECEIVE_TIMER ) {
				// go back to Wait4Start
				CurrentState = Wait4Start;
			}		
			
			if ( ThisEvent.EventType == ES_BYTE_RECEIVED ) {
				printf("receivingdata : %i\r\n", ThisEvent.EventParam);
				// if BytesLeft = 0, then we just received the checksum 
				if (BytesLeft == 0) {
					printf("CheckSum: %i\n\r", ThisEvent.EventParam);
					if (ThisEvent.EventParam == (0xFF - CheckSum)) {
						// if good checksum, post PacketReceived event to Comm_Service
						ES_Event ThisEvent;         
						ThisEvent.EventType = ES_DATAPACKET_RECEIVED;
						ThisEvent.EventParam = FrameLength; 
						PostComm_Service(ThisEvent);
					} else {
						// if bad checksum, don't do anything? 
					}
					
					// go back to Wait4Start
					CurrentState = Wait4Start;
				}

				// else if BytesLeft > 0, then we're still receiving data bytes
				// store current data byte 
				uint8_t CurrentByte = ThisEvent.EventParam;
				
				// add byte to DataPacket
				DataPacket[ArrayIndex] = CurrentByte;
				
				// increment ArrayIndex
				ArrayIndex++;
				
				// update check sum
				CheckSum += CurrentByte;
				
				// decrement Bytes left 
				BytesLeft--;
				
				// start receive timer 
				ES_Timer_InitTimer(RECEIVE_TIMER, RECEIVE_TIMER_LENGTH);
				
			}
      
    break;
		
    default :
      ;
  }                                   // end switch on Current State
  return ReturnEvent;
}

/****************************************************************************
 Function
    GetMessage

 Parameters
   none

 Returns
   pointer to array that has data packet received from Xbee

 Author
   Sarah Cabreros
****************************************************************************/
uint8_t* GetDataPacket(void) {
	return outgoingDataPacket;
}


