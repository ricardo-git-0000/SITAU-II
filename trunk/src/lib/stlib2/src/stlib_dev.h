#ifndef stlib_devH
#define stlib_devH

#include "tad.h"

// ______________________________________________________________________
// ------ C O N S T A N T   D E F I N I T I O N
// ======================================================================

// ______________________________________________________________________
// ------ F U N T I O N S   P R O T O T Y P E
// ======================================================================

// _____________________________________________________________________________
// ------ System Functions
// =============================================================================

int   ST_LoadFileConfig_Progress (void);

void  ST_SetFileLogProtocol (int data);
void  ST_SetFileLogHWRegisters (int data);
void  ST_SetFileLogSystemInfo (int data);

// _____________________________________________________________________________
// ------ Acqusition Data Functions
// =============================================================================

// ------ Control Acquisition Functions ----------------------------------------

int            ST_GetFP_ImageDataNumber(void);

int  ST_LoadFileAcquisition_Progress (char* str);
int  ST_SaveFileAcquisition_Progress (char* str);

void  ST_MemoryClear(void);
int  ST_Run (int wait);

// ------ Start / Stop Automatic Acquisition Functions -------------------------

int ST_StopProcess (void);
int ST_Acquire();// unsigned long* data);

                                       

// ------ Manual User Trigger Acquisition Functions ----------------------------

// _____________________________________________________________________________
// ------ Virtual Channels Configuration
// =============================================================================

int  ST_GetVirtualChannelsNumber (void);
int  ST_SetSelectedVirtualChannel (int id_virtual_channel);
int  ST_GetSelectedVirtualChannel (void);
int  ST_GetVirtualChannelName (char *name, int id_virtual_channel);
int  ST_SetVirtualChannelName(char *data, int id_virtual_channel);

// _____________________________________________________________________________
// ------ Range Functions
// =============================================================================

float  ST_RangeMaxPRF (void);
float  ST_RangeMinPRF (void);

// _____________________________________________________________________________
// ------ Conversion Functions -------------------------------------------------
// =============================================================================

float ST_SamplesToDistance (int data);
float ST_SamplesToTime (int data);
int   ST_TimeToSamples (float data);
float ST_TimeToDistance (float data);
float ST_DistanceToTime (float data);
float STPA_ADDataToPercentage (int data, int id_vch);
int   STFP_PercentageToADData (float data);

#endif


