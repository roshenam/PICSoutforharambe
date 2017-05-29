/****************************************************************************
 
  Header file for Nose_SM
  based on the Gen 2 Events and Services Framework

 ****************************************************************************/

#ifndef NoseSM_H
#define NoseSM_H

// Event Definitions
#include "ES_Configure.h" /* gets us event definitions */
#include "ES_Types.h"     /* gets bool type for returns */
#include "ES_PostList.h"
#include "ES_ServiceHeaders.h"
#include "ES_Port.h"

// typedefs for the states
// State definitions for use with the query function
typedef enum { DebouncingNose, Ready2SampleNose, InitNoseDebounce  } NoseDebounce_t ;

// Public Function Prototypes

bool InitNose_SM ( uint8_t Priority );
bool PostNose_SM( ES_Event ThisEvent );
ES_Event RunNose_SM( ES_Event ThisEvent );

#endif /* NoseDebounce_H */

