#ifndef TRANSMIT_H
#define TRANSMIT_H

#include "ES_Configure.h"
#include "ES_Types.h"

typedef enum { Idle, SendingData } TransmitState_t ;

bool IsLastByte(void);

bool InitTransmit_SM ( uint8_t Priority );
bool PostTransmit_SM( ES_Event ThisEvent );
ES_Event RunTransmit_SM( ES_Event ThisEvent );

#endif 
