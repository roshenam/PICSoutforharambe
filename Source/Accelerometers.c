/****************************************************************************
 Module
   Accelerometers

 Description
   Interacts with R/L and F/B accelerometers
 History
 When           Who     What/Why
 -------------- ---     --------
 05/16/2017			RM
****************************************************************************/
#include "inc/hw_gpio.h"
#include "inc/hw_types.h"
#include "termio.h"
#include "ADMulti.h"

#include "Accelerometers.h"

/*----------------------------- Module Defines ----------------------------*/
#define ACCELRL_PIN 0
#define ACCELFB_PIN 2

#define MAX_EFFORT 255
#define MIN_EFFORT 0
#define MAX_ANALOG 4095
#define MIN_ANALOG 0
#define MAX_ACCEL_HEAD 2300
#define MAX_ACCEL_TAIL 1800
#define MIN_ACCEL_HEAD 840
#define MIN_ACCEL_TAIL 840
#define TAIL 1
#define HEAD 0
#define MIDDLE 127
/*---------------------------- Module Functions ---------------------------*/
static uint8_t Scale_Accel( uint32_t AccelToScale, uint8_t Tail );


/*---------------------------- Module Variables ---------------------------*/
static uint8_t Head_Effort;
static uint8_t Tail_Effort;
static uint32_t ADResults[4];



/*------------------------------ Module Code ------------------------------*/
/****************************************************************************
 Function
     Init_Accel

 Parameters
     None

 Returns
    None

 Description
     Initializes pins and AD 
 Notes

 Author
     Roshena MacPherson
****************************************************************************/
void Init_Accel ( void )
{
	ADC_MultiInit(3);
}

/****************************************************************************
 Function
     Get_AccelTail

 Parameters
     none

 Returns
     Value of RL accelerometer

 Description
     Reads the state of the RL accelerometer input
 Notes

 Author
     Roshena MacPherson
****************************************************************************/
uint8_t Get_AccelTail( void )
{
  ADC_MultiRead(ADResults);
	
	// Do scaling fro 0-255
	Tail_Effort = Scale_Accel(ADResults[ACCELRL_PIN], TAIL);
	printf("RL value is %d, scaled is %d\r\n",Tail_Effort, ADResults[ACCELRL_PIN]);
	return Tail_Effort;
}

/****************************************************************************
 Function
     Get_AccelHead

 Parameters
     none

 Returns
     Value of FB accelerometer

 Description
     Reads the state of the FB accelerometer input
 Notes

 Author
     Roshena MacPherson
****************************************************************************/
uint8_t Get_AccelHead( void )
{
  ADC_MultiRead(ADResults);
	
	// Do scaling from 0-255
	Head_Effort = Scale_Accel(ADResults[ACCELFB_PIN], HEAD);
	printf("FB value is %d, unscaled is %d\r\n",Head_Effort, ADResults[ACCELFB_PIN]);
	return Head_Effort;
}

/****************************************************************************
 Function
     Get_FB

 Parameters
     none

 Returns
     FB value 

 Description
     
 Notes

 Author
     Roshena MacPherson
****************************************************************************/
uint8_t Get_FB( void )
{
	uint8_t Left = Get_AccelHead();
	uint8_t Right = Get_AccelTail();

	printf("Mean is %d\r\n",((Left/2) + (Right/2)));
	return ((Left/2) + (Right/2));
}

/****************************************************************************
 Function
     Get_RL

 Parameters
     none

 Returns
     FB value 

 Description
     
 Notes

 Author
     Roshena MacPherson
****************************************************************************/
uint8_t Get_RL( void )
{
	uint8_t Left = Get_AccelHead();
	uint8_t Right = Get_AccelTail();

	int Delta = Left-Right;
	
	printf("LR is %d\r\n",(Delta*MAX_EFFORT/(2*MAX_EFFORT) + MIDDLE));
	return (Delta*MAX_EFFORT/(2*MAX_EFFORT) + MIDDLE);
}

/****************************************************************************
 Function
     Scale_Accel

 Parameters
		AccelToScale: The raw 0-4095 value we want to scale

 Returns
     Scaled value of FB accelerometer from 0-255

 Description
     Scales the accelerometer raw AD values
 Notes

 Author
     Roshena MacPherson
****************************************************************************/
static uint8_t Scale_Accel( uint32_t AccelToScale, uint8_t Tail )
{
	uint16_t Max_Accel;
	uint16_t Min_Accel;
	
	if( Tail ){
		Max_Accel = MAX_ACCEL_TAIL;
		Min_Accel = MIN_ACCEL_TAIL;
	}
	else{
		Max_Accel = MAX_ACCEL_HEAD;
		Min_Accel = MIN_ACCEL_HEAD;
	}
		
  if( AccelToScale > Max_Accel ){
		return MIN_EFFORT;
	}
	else if( AccelToScale < Min_Accel ){
		return MAX_EFFORT;
	}
	else{
		return (MAX_EFFORT*Max_Accel)/(Max_Accel-Min_Accel) - (MAX_EFFORT*AccelToScale)/(Max_Accel-Min_Accel);
	}
}

