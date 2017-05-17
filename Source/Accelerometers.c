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
#define ACCELFB_PIN 1

/*---------------------------- Module Functions ---------------------------*/
static uint32_t ADResults[4];

/*---------------------------- Module Variables ---------------------------*/


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
	ADC_MultiInit(2);
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
uint32_t Get_AccelRL( void )
{
  ADC_MultiRead(ADResults);
	printf("RL Accelerometer value is %d\r\n",ADResults[ACCELRL_PIN]);
	return ADResults[ACCELRL_PIN];
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
uint32_t Get_AccelFB( void )
{
  ADC_MultiRead(ADResults);
	printf("FB Accelerometer value is %d\r\n",ADResults[ACCELFB_PIN]);
	return ADResults[ACCELFB_PIN];
}
