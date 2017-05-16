#ifndef FARMER_SM_H
#define FARMER_SM_H

#include "ES_Configure.h"
#include "ES_Types.h"
#include "ES_Events.h"

typedef enum {InitFARMER, FarmerUnpaired, FarmerPaired} FARMERState_t ;

bool InitFARMER_SM ( uint8_t Priority );
bool PostFARMER_SM( ES_Event ThisEvent );
ES_Event RunFARMER_SM( ES_Event ThisEvent );

#endif 
