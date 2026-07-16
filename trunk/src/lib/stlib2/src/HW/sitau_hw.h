#ifndef sitau_hwH
#define sitau_hwH
// -----------------------------------------------------------------------------
/**
 @file c_pa_system.cpp

 @brief Este archivo contiene la implementación de las funciones de conversión de datos de la curva TGC.

 @author (rg) Ricardo González

 <pre>
 MODIFICATION HISTORY:

 Ver   Who  Date     Changes
 ----- ---- -------- -----------------------------------------------------------
 1.00a (rg) 11/29/10 First release
 </pre>
*/
// -----------------------------------------------------------------------------
// _____________________________________________________________________________
// ------ L I B R E R I A S   P R O P I A S   D E L   C O M P I L A D O R
// =============================================================================

// _____________________________________________________________________________
// ------ L I B R E R I A S   P R O P I A S   D E L   P R O Y E C T O
// =============================================================================

#include "c_pa_system.h"

void STHW_DeleteXMLData (void);

int STHW_CreateXMLData (char *path, char *file=NULL);

CPASystem *STHW_GetPtrPASystem(int id_pa_sys);

CPASystem *STHW_GetPtrPASystem_BASEFirst (int id_base_first);

int STHW_GetPASysChannelsNumber (int id_pa_sys);

int STHW_GetPASysActiveChannelsNumber (int id_pa_sys);

int STHW_GetPASysMultiplexedChannelsNumber (int id_pa_sys);

int STHW_GetHWBASENumber (void);

int STHW_GetPASysBASENumber (int id_pa_sys);

int STHW_GetConnectorStartChannel (int id_pa_sys, int id_conector);

int STHW_GetPASysFirstBASE (int id_pa_sys);

int STHW_GetPASysLastBASE (int id_pa_sys);

int STHW_GetEncodersNumber(void);

int STHW_GetBeamformersNumber(void);

AnsiString STHW_GetEncoderName(int id_encoder);

AnsiString STHW_GetSerialNumber(void);

#endif
