/****************************************************************************
 
  Header file for Touch_SM
  based on the Gen 2 Events and Services Framework

 ****************************************************************************/

#ifndef TouchSM_H
#define TouchSM_H

// Event Definitions
#include "ES_Configure.h" /* gets us event definitions */
#include "ES_Types.h"     /* gets bool type for returns */
#include "ES_PostList.h"
#include "ES_ServiceHeaders.h"
#include "ES_Port.h"

// typedefs for the states
// State definitions for use with the query function
typedef enum { Debouncing, Ready2Sample, InitButtonDebounce  } ButtonDebounce_t ;

// Public Function Prototypes

bool InitTouch_SM ( uint8_t Priority );
bool PostTouch_SM( ES_Event ThisEvent );
ES_Event RunTouch_SM( ES_Event ThisEvent );

#endif /* TouchButtonDebounce_H */

