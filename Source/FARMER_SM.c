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
#include "Accelerometers.h"

/*----------------------------- Module Defines ----------------------------*/
#define NUM_ENCRYPTION_BYTES 32
#define PERIPHERAL_PIN		BIT5HI
#define BRAKE_PIN					BIT2HI

/*---------------------------- Module Functions ---------------------------*/
static void CreateEncryptionKey(void);
uint8_t* GetEncryptionKey(void);
uint8_t* GetSensorData(void); // placeholder

/*---------------------------- Module Variables ---------------------------*/
static FARMERState_t CurrentState;

static uint8_t MyPriority;

static uint8_t EncryptionKey[NUM_ENCRYPTION_BYTES]; // array of 32 encryption keys 

static uint8_t DogTag = 0x01;

static uint16_t GameTimerLength;


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
  MyPriority = Priority;
  
	CurrentState = Wait2Pair;
	//CurrentState = Debug;

	printf("Initialized in FARMER_SM\r\n");
	
	Init_Accel();
	
	// Initialize LED/eyes pin
	HWREG(SYSCTL_RCGCGPIO) |= SYSCTL_RCGCGPIO_R1;
	while( (HWREG(SYSCTL_PRGPIO) & SYSCTL_PRGPIO_R1 ) != SYSCTL_PRGPIO_R1);
	// Congifure the button input line as digital
	HWREG( GPIO_PORTB_BASE + GPIO_O_DEN ) |= ( GPIO_PIN_7 );
	// Configure the button line as an output line
	HWREG( GPIO_PORTB_BASE + GPIO_O_DIR ) |= GPIO_PIN_7 ;
	HWREG( GPIO_PORTB_BASE + ( GPIO_O_DATA + ALL_BITS) ) &= ~(GPIO_PIN_7) ; // start low
  
	// Initialize peripheral pin
	HWREG( GPIO_PORTB_BASE + GPIO_O_DEN ) |= ( PERIPHERAL_PIN );
	HWREG( GPIO_PORTB_BASE + GPIO_O_DIR ) &= ~PERIPHERAL_PIN ;
	
	// Initialize brake pin
	HWREG(SYSCTL_RCGCGPIO) |= SYSCTL_RCGCGPIO_R0;
	while( (HWREG(SYSCTL_PRGPIO) & SYSCTL_PRGPIO_R0 ) != SYSCTL_PRGPIO_R0);
	HWREG( GPIO_PORTA_BASE + GPIO_O_DEN ) |= ( BRAKE_PIN );
	HWREG( GPIO_PORTA_BASE + GPIO_O_DIR ) &= ~BRAKE_PIN ;
	
	return true;
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

		case Debug:
			if (ThisEvent.EventType == ES_NEW_KEY){
				if (ThisEvent.EventParam == 'f'){
					uint8_t FB_Accel = Get_AccelFB();
				}
				else if (ThisEvent.EventParam == 'r'){
					uint8_t RL_Accel = Get_AccelRL();
				}
			}
		break;
			
    case Wait2Pair: 
			printf("wait2pair \r\n");
 
			if (ThisEvent.EventType == ES_UNPAIR) {
				printf("UNPAIRED\r\n");
				//if there is ever a place where we want to unpair, send this event to farmer_sm
				//most likeley for debugging - add in a key-press event that sends this event
				CurrentState = Wait2Pair;
			}
		
			if ( ThisEvent.EventType == ES_PAIR) {
				// read DOGTAG number
				//DogTag = ;
				
				// send a REQ2PAIR packet 
				ES_Event NewEvent;
				NewEvent.EventType = ES_SENDPACKET;
				NewEvent.EventParam = FARMER_DOG_REQ_2_PAIR; // type of data packet to construct
				PostComm_Service(NewEvent);
				
				// start LOST_COMM timer
				ES_Timer_InitTimer(LOST_COMM_TIMER, LOST_COMM_TIME);
				
				// go to Wait4PairResponse
				CurrentState = Wait4PairResponse;
			}
      
    break;

		case Wait4PairResponse:

			if (ThisEvent.EventType == ES_UNPAIR) {
				printf("UNPAIRED\r\n");
				//if there is ever a place where we want to unpair, send this event to farmer_sm
				//most likeley for debugging - add in a key-press event that sends this event
				CurrentState = Wait2Pair;
			}
			
			if ( ThisEvent.EventType == ES_TIMEOUT && ThisEvent.EventParam == LOST_COMM_TIMER ) {
				printf("Lost communication, No ACK\r\n");
				// turn off eyes
				HWREG( GPIO_PORTB_BASE + ( GPIO_O_DATA + ALL_BITS) ) &= ~(GPIO_PIN_7) ;
				
				// go back to Wait2Pair state
				CurrentState = Wait2Pair;
			}
			
			if (ThisEvent.EventType == ES_DOG_ACK_RECEIVED ) {
				printf("PAIRED\r\n");
				
				//light up eyes
				HWREG( GPIO_PORTB_BASE + ( GPIO_O_DATA + ALL_BITS) ) |= (GPIO_PIN_7) ;
				
				// generate ecryption key 
				CreateEncryptionKey();
				
				// send an ENCR_KEY packet 
				ES_Event NewEvent;
				NewEvent.EventType = ES_SENDPACKET;
				NewEvent.EventParam = FARMER_DOG_ENCR_KEY;
				PostComm_Service(NewEvent);
				
				// set encryption index to zero
				ResetEncryptionIndex();
				
				// start LOST_COMM timer
				ES_Timer_InitTimer(LOST_COMM_TIMER, LOST_COMM_TIME);
				
				// start INTER_MESSAGE timer
				ES_Timer_InitTimer(INTER_MESSAGE_TIMER, INTER_MESSAGE_TIME);
				
				// start GameTimer
				ES_Timer_InitTimer(GAME_TIMER, 10000);
				
				// go to Paired state
				CurrentState = Paired;
			}
		
    break;

		case Paired:      
			
			if (ThisEvent.EventType == ES_UNPAIR) {
				printf("UNPAIRED\r\n");
				// turn off eyes
				HWREG( GPIO_PORTB_BASE + ( GPIO_O_DATA + ALL_BITS) ) &= ~(GPIO_PIN_7) ;
				
				//if there is ever a place where we want to unpair, send this event to farmer_sm
				//most likeley for debugging - add in a key-press event that sends this event
				CurrentState = Wait2Pair;
			}
			
			if ( ThisEvent.EventType == ES_TIMEOUT && ThisEvent.EventParam == GAME_TIMER ) {
				printf("GAME OVER\r\n");
				// turn off eyes
				HWREG( GPIO_PORTB_BASE + ( GPIO_O_DATA + ALL_BITS) ) &= ~(GPIO_PIN_7) ;
				
				// go back to Wait2Pair state
				CurrentState = Wait2Pair;
			}
		
			if ( ThisEvent.EventType == ES_TIMEOUT && ThisEvent.EventParam == LOST_COMM_TIMER ) {
				printf("Lost communication\r\n");
				// turn off eyes
				HWREG( GPIO_PORTB_BASE + ( GPIO_O_DATA + ALL_BITS) ) &= ~(GPIO_PIN_7) ;
				
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
				ResetEncryptionIndex();
				
				// start LOST_COMM timer
				ES_Timer_InitTimer(LOST_COMM_TIMER, LOST_COMM_TIME);
			}
		
    break;		
		
    default :
      ;
  }                                   // end switch on Current State
  return ReturnEvent;
}

/****************************************************************************
 Function
     CreateEncryptionKey

 Parameters
     none

 Returns
     none

 Description
     Populates EncryptionKey array with 32 randomly-generated encryption bytes

 Author
     Sarah Cabreros
****************************************************************************/
static void CreateEncryptionKey(void) {
	for (int i = 0; i < NUM_ENCRYPTION_BYTES; i++) {
		EncryptionKey[i] = rand() % 255; // generate a random integer between 0 and 255
	}
}

/****************************************************************************
 Function
     GetEncryptionKey

 Parameters
     none

 Returns
     Pointer to first element in EncryptionKey array

 Description
     Getter function to access EncryptionKey array

 Author
     Sarah Cabreros
****************************************************************************/
uint8_t* GetEncryptionKey(void) {
	return &EncryptionKey[0];
}

/****************************************************************************
 Function
     GetDogTag

 Parameters
     none

 Returns
     DogTag of paired DOG

 Description
     Getter function for paired DogTag

 Author
     Sarah Cabreros
****************************************************************************/
uint8_t GetDogTag(void) {
	return DogTag;
}

uint8_t* GetSensorData(void) {
	static uint8_t Data[3];
	Data[0] = Get_AccelFB();
	Data[1] = Get_AccelRL();
	
	// get input from brake and peripheral
	uint8_t DigitalByte = 0;
	
	// read peripheral pin (tongue switch, PB5)
	if ( (HWREG(GPIO_PORTB_BASE + ( GPIO_O_DATA + ALL_BITS) ) & PERIPHERAL_PIN) == 0) { // if pin is LOW (button pressed)
		DigitalByte |= BIT0HI;
	} // else leave bit 0 low
	
	// read brake pin (green button, PA2)
	if ( (HWREG( GPIO_PORTA_BASE + ( GPIO_O_DATA + ALL_BITS) ) & BRAKE_PIN) == 0) { // if pin is LOW (switch pressed)
		DigitalByte |= BIT1HI;
	} // else leave bit 1 low	
	
	Data[2] = DigitalByte;
	
	return &Data[0];
}
