/****************************************************************************
 Module
   Nose_SM.c

 Revision
   1.0.0

 Description
   This is the service that deals with capacitive touche sensor presses

 Notes

 History
 When           Who     What/Why
 -------------- ---     -------- 
****************************************************************************/
#include "ES_Configure.h"
#include "ES_Framework.h"
#include "ES_DeferRecall.h"
#include "ES_ShortTimer.h"
#include "Nose_SM.h"

#include "inc/hw_memmap.h"
#include "inc/hw_types.h"
#include "inc/hw_gpio.h"
#include "inc/hw_sysctl.h"
#include "driverlib/sysctl.h"
#include "driverlib/pin_map.h"	// Define PART_TM4C123GH6PM in project
#include "driverlib/gpio.h"

/*----------------------------- Module Defines ----------------------------*/
// these times assume a 1.000mS/tick timing
#define ONE_SEC 976
#define HALF_SEC (ONE_SEC/2)
#define TWO_SEC (ONE_SEC*2)
#define FIVE_SEC (ONE_SEC*5)
#define QUARTER_SEC (ONE_SEC/4)
#define DEBOUNCE_DELAY (ONE_SEC/4)

// Data pins
// Pair button on PB4
#define NOSEBUTTON GPIO_PIN_5
#define NOSEBUTTON_LO BIT5LO
#define NOSEBUTTON_HI BIT5HI

#define ALL_BITS (0xff<<2)

/*---------------------------- Module Functions ---------------------------*/
/* prototypes for private functions for this service.They should be functions
   relevant to the behavior of this service 
*/

/*---------------------------- Module Variables ---------------------------*/
// with the introduction of Gen2, we need a module level Priority variable
static uint8_t MyPriority;
static NoseDebounce_t CurrentState = InitNoseDebounce;

// add a deferral queue for up to 3 pending deferrals +1 to allow for overhead
//static ES_Event DeferralQueue[3+1];

/****************************************************************************
 Function
     InitNose_SM

 Parameters
     uint8_t : the priorty of this service

 Returns
     bool, false if error in initialization, true otherwise

 Description
     Saves away the priority, and does any 
     other required initialization for this service
 Notes

 Author
     R. MacPherson, 5/21/17
****************************************************************************/

bool InitNose_SM ( uint8_t Priority )
{
  ES_Event ThisEvent;
	// Initialize the MyPriority variable with the passed in parameter.
  MyPriority = Priority;
	printf("initialized nose button\r\n");
	// Initialize the port line to monitor the button
	HWREG(SYSCTL_RCGCGPIO) |= SYSCTL_RCGCGPIO_R1;
	while( (HWREG(SYSCTL_PRGPIO) & SYSCTL_PRGPIO_R1 ) != SYSCTL_PRGPIO_R1);
	// Congifure the button input line as digital
	HWREG( GPIO_PORTB_BASE + GPIO_O_DEN ) |= ( NOSEBUTTON );
	// Configure the button line as an input line
	HWREG( GPIO_PORTB_BASE + GPIO_O_DIR ) &= ~NOSEBUTTON;

	// Sample port line and use it to initialize the LastButtonState variable
	//LastButtonState = ( HWREG(GPIO_PORTB_BASE + ( GPIO_O_DATA + ALL_BITS )) & TOUCHBUTTON_HI );
	CurrentState = DebouncingNose;
	
	ES_Timer_InitTimer(NOSEDEBOUNCE_TIMER, DEBOUNCE_DELAY);
	
  // Post Event ES_Init to ButtonDebounce queue (this service)
  ThisEvent.EventType = ES_INIT;
	PostNose_SM( ThisEvent );
	
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
     PostNose_SM

 Parameters
     EF_Event ThisEvent ,the event to post to the queue

 Returns
     bool false if the Enqueue operation failed, true otherwise

 Description
     Posts an event to this state machine's queue
 Notes

 Author
R. MacPherson, 5/21/17
****************************************************************************/
bool PostNose_SM( ES_Event ThisEvent )
{
  return ES_PostToService( MyPriority, ThisEvent);
}

/****************************************************************************
 Function
    RunNose_SM

 Parameters
   ES_Event : the event to process

 Returns
   ES_Event, ES_NO_EVENT if no error ES_ERROR otherwise

 Description
   Implements the service for Button Debounce
 Notes
	Returns ES_NO_EVENT if No Error detected, ES_ERROR otherwise
   
 Author
R. MacPherson, 5/21/17
****************************************************************************/

ES_Event RunNose_SM( ES_Event ThisEvent ) {
	
  ES_Event ReturnEvent;
  ReturnEvent.EventType = ES_NO_EVENT; // assume no errors
	
	switch( CurrentState ){
		case DebouncingNose :
			if( (ThisEvent.EventType == ES_TIMEOUT) && (ThisEvent.EventParam == NOSEDEBOUNCE_TIMER )){
				CurrentState = Ready2SampleNose;}
			break;
		case Ready2SampleNose :
			switch( ThisEvent.EventType ){
				ES_Event Button_Event;
				case NOSEBUTTON_UP :
					printf("nose button up in TBD\r\n");
					ES_Timer_InitTimer( NOSEDEBOUNCE_TIMER, DEBOUNCE_DELAY );
					CurrentState = DebouncingNose;
					//Button_Event.EventType = DB_TOUCHBUTTONUP;
					Button_Event.EventParam = ES_Timer_GetTime();
					Button_Event.EventType = TOGGLE_PERIPHERAL;
					PostFARMER_SM( Button_Event );
					break;
				case NOSEBUTTON_DOWN :
					//printf("touch button down in TBD\r\n");
					ES_Timer_InitTimer( NOSEDEBOUNCE_TIMER, DEBOUNCE_DELAY);
					CurrentState = DebouncingNose;	
					break;
				default :
					break;
			}
				break;
			default:
				break;
	}
	return ReturnEvent;
}
				


