#ifndef Comm_Service_H
#define Comm_Service_H

#include "ES_Configure.h"
#include "ES_Types.h"
#include "ES_Events.h"

void ResetEncryptionIndex(void);

bool InitComm_Service ( uint8_t Priority );
bool PostComm_Service( ES_Event ThisEvent );
ES_Event RunComm_Service( ES_Event ThisEvent );

/***getter***/
uint8_t* GetDataPacket_Tx (void);
uint8_t* GetIMUData(void);

#endif 
