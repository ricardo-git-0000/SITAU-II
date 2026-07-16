#ifndef stfp_getH
#define stfp_getH

#if defined(_MSC_VER)
   #include <vector>
#else
   #include <vector.h>
#endif
#include "tad.h"

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

float ST_GetTimeResolution(void);

float ST_GetDistanciaEntreMuestras(void);

bool ST_GetSelectLogData_HWCircularBuffer (void);

bool ST_GetSelectLogData_AcqBufferCounter (void);

bool ST_GetSelectLogData_AcqBufferSize (void);

bool ST_GetSelectLogData_AcqBufferNumber (void);

int ST_AcquiringError (void);

#endif
