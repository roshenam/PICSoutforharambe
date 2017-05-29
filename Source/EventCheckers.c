/****************************************************************************
 Module
   EventCheckers.c

 Revision
   1.0.1 

 Description
   This is the sample for writing event checkers along with the event 
   checkers used in the basic framework test harness.

 Notes
   Note the use of static variables in sample event checker to detect
   ONLY transitions.
   
 History
 When           Who     What/Why
 -------------- ---     --------
 08/06/13 13:36 jec     initial version
****************************************************************************/

// this will pull in the symbolic definitions for events, which we will want
// to post in response to detecting events
#include "ES_Configure.h"
// this will get us the structure definition for events, which we will need
// in order to post events in response to detecting events
#include "ES_Events.h"
// This include will pull in all of the headers from the service modules
// providing the prototypes for all of the post functions
#include "ES_ServiceHeaders.h"
// this test harness for the framework references the serial routines that
// are defined in ES_Port.c
#include "ES_Port.h"
// include our own prototypes to insure consistency between header & 
// actual functionsdefinition
#include "EventCheckers.h"

#include "FARMER_SM.h"

#include "inc/hw_memmap.h"
#include "inc/hw_types.h"
#include "inc/hw_gpio.h"
#include "inc/hw_sysctl.h"
#include "driverlib/sysctl.h"
#include "driverlib/pin_map.h"	// Define PART_TM4C123GH6PM in project
#include "driverlib/gpio.h"

// Pair button on PB4
#define TOUCHBUTTON GPIO_PIN_4
#define TOUCHBUTTON_LO BIT4LO
#define TOUCHBUTTON_HI BIT4HI

// Peripheral button on PB5
#define NOSEBUTTON GPIO_PIN_5
#define NOSEBUTTON_LO BIT5LO
#define NOSEBUTTON_HI BIT5HI

#define ALL_BITS (0xff<<2)

static uint8_t LastButtonState = 0;
static uint8_t LastNoseState = 0;

/****************************************************************************
 Function
   Check4Keystroke
 Parameters
   None
 Returns
   bool: true if a new key was detected & posted
 Description
   checks to see if a new key from the keyboard is detected and, if so, 
   retrieves the key and posts an ES_NewKey event to TestHarnessService0
 Notes
   The functions that actually check the serial hardware for characters
   and retrieve them are assumed to be in ES_Port.c
   Since we always retrieve the keystroke when we detect it, thus clearing the
   hardware flag that indicates that a new key is ready this event checker 
   will only generate events on the arrival of new characters, even though we
   do not internally keep track of the last keystroke that we retrieved.
 Author
   J. Edward Carryer, 08/06/13, 13:48
****************************************************************************/
bool Check4Keystroke(void)
{
  if ( IsNewKeyReady() ) // new key waiting?
  {
    
		ES_Event ThisEvent;
    //ThisEvent.EventType = ES_NEW_KEY;
    ThisEvent.EventParam = GetNewKey();
    //PostFARMER_SM( ThisEvent );
		
		if (ThisEvent.EventParam == 'd' || ThisEvent.EventParam == 'D') {
			printf("d\r\n");
			ThisEvent.EventType = ES_PAIR;
			PostFARMER_SM(ThisEvent);
		}
		
		else if (ThisEvent.EventParam == 'u' || ThisEvent.EventParam == 'U') {
			printf("u\r\n");
			ThisEvent.EventType = ES_UNPAIR;
			PostFARMER_SM(ThisEvent);
		}
		
		else{
			ThisEvent.EventType = ES_NEW_KEY;
			PostFARMER_SM(ThisEvent);
		}
    
    return true;
  }
  return false;
}

/****************************************************************************
 Function
   CheckTouchButton
 Parameters
   None
 Returns
   bool: true if a new event was posted
 Description
   Event checker for the Touch_SM service that detects rises and falls 
	 in the input pin for the touch sensor

 Author
 R. MacPherson, 5/21/17
****************************************************************************/
bool CheckTouchButton (void) {
	
	// Local variables
	bool ReturnVal = false;
	uint8_t CurrentButtonState;
	uint16_t CurrTime = ES_Timer_GetTime();
	// Get the CurrentButtonState from the input line
	CurrentButtonState = ( HWREG(GPIO_PORTB_BASE + ( GPIO_O_DATA + ALL_BITS )) & TOUCHBUTTON_HI );
	//printf("reset button state is %d\r\n",CurrentButtonState);
	// If the state of the Button input line has changed
	if ( CurrentButtonState != LastButtonState ){
		ES_Event ThisEvent;
		ThisEvent.EventParam = CurrTime;
		// If the current state of the input line is high
		if ( CurrentButtonState == TOUCHBUTTON_HI ){
			// PostEvent ResetButtonUp with parameter of the Current Time
    		ThisEvent.EventType = TOUCHBUTTON_UP;}
		else{
			ThisEvent.EventType = TOUCHBUTTON_DOWN;}
		
		PostTouch_SM(ThisEvent); 	
    ReturnVal = true; }
		//printf("reset button event checker\r\n");
		LastButtonState = CurrentButtonState;
		return ReturnVal;
}

/****************************************************************************
 Function
   CheckNoseButton
 Parameters
   None
 Returns
   bool: true if a new event was posted
 Description
   Event checker for the Nose_SM service that detects rises and falls 
	 in the input pin for the nose button

 Author
 R. MacPherson, 5/21/17
****************************************************************************/
bool CheckNoseButton (void) {
	
	// Local variables
	bool ReturnVal = false;
	uint8_t CurrentNoseState;
	uint16_t CurrTime = ES_Timer_GetTime();
	// Get the CurrentButtonState from the input line
	CurrentNoseState = ( HWREG(GPIO_PORTB_BASE + ( GPIO_O_DATA + ALL_BITS )) & NOSEBUTTON_HI );
	//printf("reset button state is %d\r\n",CurrentButtonState);
	// If the state of the Button input line has changed
	if ( CurrentNoseState != LastNoseState ){
		ES_Event ThisEvent;
		ThisEvent.EventParam = CurrTime;
		// If the current state of the input line is high
		if ( CurrentNoseState == NOSEBUTTON_HI ){
			// PostEvent ResetButtonUp with parameter of the Current Time
    		ThisEvent.EventType = NOSEBUTTON_UP;}
		else{
			ThisEvent.EventType = NOSEBUTTON_DOWN;}
		
		PostNose_SM(ThisEvent); 	
    ReturnVal = true; }
		//printf("reset button event checker\r\n");
		LastNoseState = CurrentNoseState;
		return ReturnVal;
}


