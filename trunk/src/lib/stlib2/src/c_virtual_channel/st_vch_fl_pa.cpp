// -----------------------------------------------------------------------------
/**
 @file st_vch_fl_pa.cpp

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

#include <math.h>

// _____________________________________________________________________________
// ------ L I B R E R I A S   P R O P I A S   D E L   P R O Y E C T O
// =============================================================================

#include "st_vch_fl_pa.h"
#include "stlib2.h"
#include "c_virtual_channel.h"
#include "st_error_code.h"
#include "c_log_file.h"

// _____________________________________________________________________________
// ------ V A R I A B L E S   G L O B A L E S
// =============================================================================

// _____________________________________________________________________________
// ------ D E F I N I C I O N   D E   F U N C I O N E S
// =============================================================================
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int ST_PA_SetLinearScanConfig (int step, int aperture, int startelement, int endelement, float angle)
{
int result;
int id_vch = 0;

   if (id_vch < 0) id_vch = gb_selected_virtual_channel;
   if (id_vch < 0 || id_vch >= gb_virtual_channel.size() || gb_virtual_channel[id_vch] == NULL)
      return ELOG(charES2_IdVirtualChannel, ES2_IdVirtualChannel);

   if ((result = gb_virtual_channel[id_vch]->SET_PA_LinearScanConfig (step, aperture, startelement, endelement, angle)) < 0)
      return RLOG(result);
   return ES2_NONE;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int ST_PA_SetAngularScanConfig(float step, unsigned short aperture, unsigned short startelement, float startangle, float endangle)//, int id_vch)
{
int result;
int id_vch = 0;

   if (id_vch < 0) id_vch = gb_selected_virtual_channel;
   if (id_vch < 0 || id_vch >= gb_virtual_channel.size() || gb_virtual_channel[id_vch] == NULL)
      return ELOG(charES2_IdVirtualChannel, ES2_IdVirtualChannel);

   if ((result = gb_virtual_channel[id_vch]->SET_PA_AngularScanConfig (step, aperture, startelement, startangle, endangle)) < 0)
      return RLOG(result);
   return ES2_NONE;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
//int ST_SetDynamicFocusingEnable (int data)//, int id_vch)
//{
//int result;
//int id_vch = 0;
//
//   if (id_vch < 0) id_vch = gb_selected_virtual_channel;
//   if (id_vch < 0 || id_vch >= gb_virtual_channel.size() || gb_virtual_channel[id_vch] == NULL)
//      return ELOG(charES2_IdVirtualChannel, ES2_IdVirtualChannel);
//
//   if ((result = gb_virtual_channel[id_vch]->SET_PA_DynamicFocusingEnable(data)) < 0)
//      return RLOG(result);
//   return ES2_NONE;
//}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
//int ST_GetDynamicFocusingEnable (void)//int id_vch)
//{
//int id_vch = 0;
//
//   if (id_vch < 0) id_vch = gb_selected_virtual_channel;
//   if (id_vch < 0 || id_vch >= gb_virtual_channel.size() || gb_virtual_channel[id_vch] == NULL)
//      return ELOG(charES2_IdVirtualChannel, ES2_IdVirtualChannel);
//   return gb_virtual_channel[id_vch]->GET_PA_DynamicFocusingEnable();
//}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int ST_PA_GetAperture (void)
{
int id_vch = 0;

   if (id_vch < 0) id_vch = gb_selected_virtual_channel;
   if (id_vch < 0 || id_vch >= gb_virtual_channel.size() || gb_virtual_channel[id_vch] == NULL)
      return ELOG(charES2_IdVirtualChannel, ES2_IdVirtualChannel);
   return gb_virtual_channel[id_vch]->GET_PA_ReceptionAperture();
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int ST_PA_GetScanType (void)
{
int id_vch = 0;

   if (id_vch < 0) id_vch = gb_selected_virtual_channel;
   if (id_vch < 0 || id_vch >= gb_virtual_channel.size() || gb_virtual_channel[id_vch] == NULL)
      return ELOG(charES2_IdVirtualChannel, ES2_IdVirtualChannel);
   return gb_virtual_channel[id_vch]->GET_PA_ScanType();
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int ST_PA_GetLinearScanStep(void)
{
int id_vch = 0;

   if (id_vch < 0) id_vch = gb_selected_virtual_channel;
   if (id_vch < 0 || id_vch >= gb_virtual_channel.size() || gb_virtual_channel[id_vch] == NULL)
      return ELOG(charES2_IdVirtualChannel, ES2_IdVirtualChannel);
   return gb_virtual_channel[id_vch]->GET_PA_LinearStep();
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int ST_PA_GetScanStartElement (void)
{
int id_vch = 0;

   if (id_vch < 0) id_vch = gb_selected_virtual_channel;
   if (id_vch < 0 || id_vch >= gb_virtual_channel.size() || gb_virtual_channel[id_vch] == NULL)
      return ELOG(charES2_IdVirtualChannel, ES2_IdVirtualChannel);
   return gb_virtual_channel[id_vch]->GET_PA_ReceptionStartElement();
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int ST_GetScanEmissionStartElement (void)//int id_vch)
{
int id_vch = 0;

   if (id_vch < 0) id_vch = gb_selected_virtual_channel;
   if (id_vch < 0 || id_vch >= gb_virtual_channel.size() || gb_virtual_channel[id_vch] == NULL)
      return ELOG(charES2_IdVirtualChannel, ES2_IdVirtualChannel);
   return gb_virtual_channel[id_vch]->GET_PA_EmissionStartElement();
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int ST_PA_GetLinearScanEndElement(void)
{
int id_vch = 0;

   if (id_vch < 0) id_vch = gb_selected_virtual_channel;
   if (id_vch < 0 || id_vch >= gb_virtual_channel.size() || gb_virtual_channel[id_vch] == NULL)
      return ELOG(charES2_IdVirtualChannel, ES2_IdVirtualChannel);
   return gb_virtual_channel[id_vch]->GET_PA_LinearEndElement();
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
float ST_PA_GetLinearScanAngle(void)
{
int id_vch = 0;

   if (id_vch < 0) id_vch = gb_selected_virtual_channel;
   if (id_vch < 0 || id_vch >= gb_virtual_channel.size() || gb_virtual_channel[id_vch] == NULL)
      return ELOG(charES2_IdVirtualChannel, ES2_IdVirtualChannel);
   return gb_virtual_channel[id_vch]->GET_PA_LinearAngle();
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
float ST_PA_GetAngularScanStartAngle(void)
{
int id_vch = 0;

   if (id_vch < 0) id_vch = gb_selected_virtual_channel;
   if (id_vch < 0 || id_vch >= gb_virtual_channel.size() || gb_virtual_channel[id_vch] == NULL)
      return ELOG(charES2_IdVirtualChannel, ES2_IdVirtualChannel);
   return gb_virtual_channel[id_vch]->GET_PA_AngularStartAngle();
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
float ST_PA_GetAngularScanEndAngle(void)//int id_vch)
{
int id_vch = 0;

   if (id_vch < 0) id_vch = gb_selected_virtual_channel;
   if (id_vch < 0 || id_vch >= gb_virtual_channel.size() || gb_virtual_channel[id_vch] == NULL)
      return ELOG(charES2_IdVirtualChannel, ES2_IdVirtualChannel);
   return gb_virtual_channel[id_vch]->GET_PA_AngularEndAngle();
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
float ST_PA_GetAngularScanStepAngle(void)
{
int id_vch = 0;

   if (id_vch < 0) id_vch = gb_selected_virtual_channel;
   if (id_vch < 0 || id_vch >= gb_virtual_channel.size() || gb_virtual_channel[id_vch] == NULL)
      return (float)ELOG(charES2_IdVirtualChannel, ES2_IdVirtualChannel);
   return gb_virtual_channel[id_vch]->GET_PA_AngularStep();
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int ST_PA_GetFocusingEnabled(void)
{
int id_vch = 0;

   if (id_vch < 0) id_vch = gb_selected_virtual_channel;
   if (id_vch < 0 || id_vch >= gb_virtual_channel.size() || gb_virtual_channel[id_vch] == NULL)
      return (float)ELOG(charES2_IdVirtualChannel, ES2_IdVirtualChannel);
   return gb_virtual_channel[id_vch]->GET_PA_FocusingEnabled();
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
float ST_PA_GetEmissionFocalPoint(void)
{
int id_vch = 0;

   if (id_vch < 0) id_vch = gb_selected_virtual_channel;
   if (id_vch < 0 || id_vch >= gb_virtual_channel.size() || gb_virtual_channel[id_vch] == NULL)
      return (float)ELOG(charES2_IdVirtualChannel, ES2_IdVirtualChannel);
   return gb_virtual_channel[id_vch]->GET_PA_EmissionFocalPoint();
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
float ST_GetReceptionFocalPoint (void)//int id_vch)
{
int id_vch = 0;

   if (id_vch < 0) id_vch = gb_selected_virtual_channel;
   if (id_vch < 0 || id_vch >= gb_virtual_channel.size() || gb_virtual_channel[id_vch] == NULL)
      return (float)ELOG(charES2_IdVirtualChannel, ES2_IdVirtualChannel);
   return gb_virtual_channel[id_vch]->GET_PA_ReceptionFocalPoint();
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int ST_GetImageEntryPoint_X(float *x_coordinate, int size_data)//int id_vch)
{
int id_vch = 0;

   if (id_vch < 0) id_vch = gb_selected_virtual_channel;
   if (id_vch < 0 || id_vch >= gb_virtual_channel.size() || gb_virtual_channel[id_vch] == NULL)
      return (float)ELOG(charES2_IdVirtualChannel, ES2_IdVirtualChannel);
   return gb_virtual_channel[id_vch]->GET_ImageEntryPoint_X(x_coordinate, size_data);
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int ST_GetImageEntryPoint_Z(float *z_coordinate, int size_data)
{
int id_vch = 0;

   if (id_vch < 0) id_vch = gb_selected_virtual_channel;
   if (id_vch < 0 || id_vch >= gb_virtual_channel.size() || gb_virtual_channel[id_vch] == NULL)
      return (float)ELOG(charES2_IdVirtualChannel, ES2_IdVirtualChannel);
   return gb_virtual_channel[id_vch]->GET_ImageEntryPoint_Z(z_coordinate, size_data);
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int ST_GetImageEntryPoint_Angle(float *angle, int size_data)
{
int id_vch = 0;

   if (id_vch < 0) id_vch = gb_selected_virtual_channel;
   if (id_vch < 0 || id_vch >= gb_virtual_channel.size() || gb_virtual_channel[id_vch] == NULL)
      return (float)ELOG(charES2_IdVirtualChannel, ES2_IdVirtualChannel);
   return gb_virtual_channel[id_vch]->GET_ImageEntryPoint_Angle(angle, size_data);
}

