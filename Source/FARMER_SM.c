/****************************************************************************
 Module
   FARMER_SM.c

 Description
   Farmer state machine 

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
#include "FARMER_SM.h"

/*----------------------------- Module Defines ----------------------------*/



/*---------------------------- Module Functions ---------------------------*/


/*---------------------------- Module Variables ---------------------------*/
static FARMERState_t CurrentState;

static uint8_t MyPriority;


/*------------------------------ Module Code ------------------------------*/
/****************************************************************************
 Function
     InitFARMER_SM

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
bool InitFARMER_SM ( uint8_t Priority )
{
  ES_Event ThisEvent;

  MyPriority = Priority;
  // put us into the Initial PseudoState
  CurrentState = InitFARMER;
	
  // post the initial transition event
  ThisEvent.EventType = ES_INIT;
	printf("Initialized in FARMER_SM\r\n");
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
     PostFARMER_SM

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
bool PostFARMER_SM( ES_Event ThisEvent )
{
  return ES_PostToService( MyPriority, ThisEvent);
}

/****************************************************************************
 Function
    RunFARMER_SM

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
ES_Event RunFARMER_SM( ES_Event ThisEvent )
{
  ES_Event ReturnEvent;
  ReturnEvent.EventType = ES_NO_EVENT; // assume no errors
	
	if (ThisEvent.EventType == ES_UNPAIR) {
		printf("UNPAIRED\r\n");
		//if there is ever a place where we want to unpair, send this event to farmer_sm
		//most likeley for debugging - add in a key-press event that sends this event
		CurrentState = Wait2Pair;
	}

  switch ( CurrentState )
  {
    case InitFARMER :       
        if ( ThisEvent.EventType == ES_INIT )// only respond to ES_Init
        {
            // set current state to Wait2Pair
            CurrentState = Wait2Pair;
         }
    break;

    case Wait2Pair:      
 
			if ( ThisEvent.EventType == ES_PAIR) {
				printf("PAIRED\r\n");
				
				// EventParam should be DOG_TAG number? (0x01, 0x02, or 0x03) or do we need to know DOG_TAG here?
				
				// send a REQ2PAIR packet 
				ES_Event NewEvent;
				NewEvent.EventType = ES_SENDPACKET;
				NewEvent.EventParam = FARMER_DOG_REQ_2_PAIR; // type of data packet to construct
				PostComm_Service(NewEvent);
				
				// start LOST_COMM timer
				ES_Timer_InitTimer(LOST_COMM_TIMER, LOST_COMM_TIME);
			}
      
    break;

		case Wait4PairResponse:      
			if ( ThisEvent.EventType == ES_TIMEOUT && ThisEvent.EventParam == LOST_COMM_TIMER ) {
				// go back to Wait2Pair state
				CurrentState = Wait2Pair;
			}
			
			if (ThisEvent.EventType == ES_DOG_ACK_RECEIVED ) {
				// send an ENCR_KEY packet 
				ES_Event NewEvent;
				NewEvent.EventType = ES_SENDPACKET;
				NewEvent.EventParam = FARMER_DOG_ENCR_KEY;
				PostComm_Service(NewEvent);
				
				// start LOST_COMM timer
				ES_Timer_InitTimer(LOST_COMM_TIMER, LOST_COMM_TIME);
				
				// start INTER_MESSAGE timer
				ES_Timer_InitTimer(INTER_MESSAGE_TIMER, INTER_MESSAGE_TIME);
				
				// go to Paired state
				CurrentState = Paired;
			}
		
    break;

		case Paired:      
			if ( ThisEvent.EventType == ES_TIMEOUT && ThisEvent.EventParam == LOST_COMM_TIMER ) {
				// go back to Wait2Pair state
				CurrentState = Wait2Pair;
			}
			
			if ( ThisEvent.EventType == ES_TIMEOUT && ThisEvent.EventParam == INTER_MESSAGE_TIMER ) {
				// send a CTRL packet
				ES_Event NewEvent;
				NewEvent.EventType = ES_SENDPACKET;
				NewEvent.EventParam = FARMER_DOG_CTRL;
				PostComm_Service(NewEvent);		

				// start inter message timer
				ES_Timer_InitTimer(INTER_MESSAGE_TIMER, INTER_MESSAGE_TIME);
			}
			
			if ( ThisEvent.EventType == ES_DOG_REPORT_RECEIVED ) {
				// start LOST_COMM timer
				ES_Timer_InitTimer(LOST_COMM_TIMER, LOST_COMM_TIME);
			}
			
			if ( ThisEvent.EventType == ES_DOG_RESET_ENCR_RECEIVED ) {
				// reset encryption index to zero 
				
				// start LOST_COMM timer
				ES_Timer_InitTimer(LOST_COMM_TIMER, LOST_COMM_TIME);
			}
		
    break;		
		
    default :
      ;
  }                                   // end switch on Current State
  return ReturnEvent;
}
