#ifndef stset_v2H
#define stset_v2H

#if defined(_MSC_VER)
   #include <vector>
#else
   #include <vector.h>
#endif

using namespace std;

// ______________________________________________________________________
// ------ D E F I N I C I O N   D E   C O N S T A N T E S
// ======================================================================

// ______________________________________________________________________
// ------ D E F I N I C I O N   D E   C O N S T A N T E S
// ======================================================================

// ______________________________________________________________________
// ------ P R O T O T I P O S   D E   F U N C I O N E S -----------------
// ======================================================================

int ST_SetAcqCounterZero (void);

int ST_SetSelectLogData_HWCircularBuffer (bool data);

int ST_SetSelectLogData_AcqBufferCounter (bool data);

int ST_SetSelectLogData_AcqBufferSize (bool data);

int ST_SetSelectLogData_AcqBufferNumber (bool data);

int ST_FP_SetSelectLogData_GetBufferCounter (bool data);

int ST_PA_SetSelectLogData_GetBufferCounter(bool data);

int ST_StartPRFScan(void);

int ST_StartExternalTriggerScan(void);

#endif
