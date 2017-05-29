#ifndef FARMER_SM_H
#define FARMER_SM_H

#include "ES_Configure.h"
#include "ES_Types.h"
#include "ES_Events.h"

typedef enum { Wait2Pair, Wait4PairResponse, Paired_Wait4Status, Paired, Debug} FARMERState_t ;

uint8_t* GetEncryptionKey(void);
uint8_t GetDogTag(void);
uint8_t* GetSensorData(void); // placeholder

bool InitFARMER_SM ( uint8_t Priority );
bool PostFARMER_SM( ES_Event ThisEvent );
ES_Event RunFARMER_SM( ES_Event ThisEvent );
bool Get_PairCommand( void );

#endif 
