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

  switch ( CurrentState )
  {
    case InitFARMER :       
        if ( ThisEvent.EventType == ES_INIT )// only respond to ES_Init
        {
            // set current state to FarmerUnpaired
            CurrentState = FarmerUnpaired;
         }
    break;

    case FarmerUnpaired:      
 
			if ( ThisEvent.EventType == ES_NEW_KEY && ThisEvent.EventParam == 'd') {
				printf("d\r\n");
				
				// send some shit 
				ES_Event NewEvent;
				NewEvent.EventType = ES_SENDPACKET;
				NewEvent.EventParam = 11; // size of data packet
				PostComm_Service(NewEvent);
			}
      
    break;

		case FarmerPaired:      

		
    break;
		
		
    default :
      ;
  }                                   // end switch on Current State
  return ReturnEvent;
}
