/****************************************************************************
 
  Header file of Hardware created for ME218C Final Project

 ****************************************************************************/

#ifndef Hardware_H
#define Hardware_H

#include "ES_Types.h"
#include <stdint.h>
#include <stdbool.h>

// the headers to access the GPIO subsystem
#include "inc/hw_memmap.h"
#include "inc/hw_types.h"
#include "inc/hw_gpio.h"
#include "inc/hw_sysctl.h"
#include "inc/hw_pwm.h"
#include "inc/hw_timer.h"
#include "inc/hw_nvic.h"
#include "inc/hw_ssi.h"


// the headers to access the TivaWare Library
#include "driverlib/sysctl.h"
#include "driverlib/pin_map.h"
#include "driverlib/gpio.h"
#include "driverlib/timer.h"
#include "driverlib/interrupt.h"
#include "driverlib/pwm.h"
#include "termio.h"

#include "BITDEFS.H"

//My Modules
#include "UART.h"
#include "Receive_SM.h"
//#include "FARMER_SM.h"
//#include "IMU_Service.h"
//#include "HoverControl_Module.h"
#include "Constants.h"
#include "Comm_Service.h"

/********************Module Defines*******************************************/
//Port A
#define LIFT_FAN_PORT	GPIO_PORTA_BASE
#define LIFT_FAN_PIN	GPIO_PIN_2

//Port B


//Port C


//Port D


//Port E

#define RX_PIN	BIT4HI 	// UART5 Rx: PE4
#define TX_PIN	BIT5HI	// UART5 Tx: PE5
//Port F


//Port G


// Public Function Prototypes
uint8_t ReadInput (int Port, uint8_t Pin);
void SetOutput (int Port, uint8_t Pin, uint8_t Value);
void InitPin (int Port, uint8_t Pin, uint8_t Direction);
void InitAll (void);

#endif /* Hardware_H */

