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
#define MAX_ACCEL 2300
#define MIN_ACCEL 700

/*---------------------------- Module Functions ---------------------------*/
static uint8_t Scale_Accel( uint32_t AccelToScale );


/*---------------------------- Module Variables ---------------------------*/
static uint8_t FB_Effort;
static uint8_t RL_Effort;
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
     Get_AccelRL

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
uint8_t Get_AccelRL( void )
{
  ADC_MultiRead(ADResults);
	
	// Do scaling fro 0-255
	RL_Effort = Scale_Accel(ADResults[ACCELRL_PIN]);
	printf("RL Accelerometer value is %d\r\n",RL_Effort);
	return RL_Effort;
}

/****************************************************************************
 Function
     Get_AccelFB

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
uint8_t Get_AccelFB( void )
{
  ADC_MultiRead(ADResults);
	
	// Do scaling from 0-255
	FB_Effort = Scale_Accel(ADResults[ACCELFB_PIN]);
	printf("FB Accelerometer value is %d\r\n",FB_Effort);
	return FB_Effort;
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
static uint8_t Scale_Accel( uint32_t AccelToScale )
{
  if( AccelToScale > MAX_ACCEL ){
		return MAX_EFFORT;
	}
	else if( AccelToScale < MIN_ACCEL ){
		return MIN_EFFORT;
	}
	else{
		return (MAX_EFFORT*MAX_ACCEL)/(MAX_ACCEL-MIN_ACCEL) - (MAX_EFFORT*AccelToScale)/(MAX_ACCEL-MIN_ACCEL);
	}
}

