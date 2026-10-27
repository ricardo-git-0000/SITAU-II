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
#include "st_error_code.h"

// _____________________________________________________________________________
// ------ V A R I A B L E S   G L O B A L E S
// =============================================================================

// _____________________________________________________________________________
// ------ CPASystem class Methods
// _____________________________________________________________________________
// =============================================================================
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
CPASystem::CPASystem (  AnsiString name,
                        int id,
                        int id_base_first,
                        int n_bases,
                        int n_multiplexed_channels,
                        int start_channel,
                        TXMLHW_FRONTEND* frontend)
   : CPASystem_Config(&sys_)
{
int i;

   Setup (  name,
            id,
            id_base_first,
            n_bases,
            n_multiplexed_channels,
            start_channel);

   sys_.pin_to_channel.resize(sys_.n_total_channels);
   for (i=start_channel; i<sys_.pin_to_channel.size(); i++) sys_.pin_to_channel[i] = i;

   sys_.disabled_channel.resize(sys_.n_total_channels);
   sys_.disabled_channel.assign(sys_.disabled_channel.size(), 0);

   if ((sys_.frontend = frontend) != NULL) sys_.multiplexed = sys_.frontend->n_multiplexed_channels > 0;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
CPASystem::~CPASystem()
{
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int CPASystem::Setup (  AnsiString name,
                        int id,
                        int id_base_first,
                        int n_bases,
                        int n_multiplexed_channels,
                        int start_channel)
{
int i;

   sys_.name = name;
   sys_.id = id;

   sys_.enabled = true;

   switch(n_multiplexed_channels)
   {
   case 128:
      sys_.multiplexed = true;
      sys_.n_bases = 1;
      sys_.n_active_channels = MOD_MAX_CH;
      sys_.n_multiplexed_channels = n_multiplexed_channels;
      sys_.n_total_channels = sys_.n_multiplexed_channels;
      sys_.connector.resize(1);
      if (sys_.connector.size() > 0)
         sys_.connector[0].start_channel = 0;
      break;
   case 256:
      sys_.multiplexed = true;
      sys_.n_bases = 1;
      sys_.n_active_channels = MOD_MAX_CH;
      sys_.n_multiplexed_channels = n_multiplexed_channels;
      sys_.n_total_channels = sys_.n_multiplexed_channels;
      sys_.connector.resize(2);
      for (i=0; i<sys_.connector.size(); i++)
         sys_.connector[i].start_channel = i * 128;
      break;
   case 512:
      sys_.multiplexed = true;
      sys_.n_bases = 1;
      sys_.n_active_channels = MOD_MAX_CH;
      sys_.n_multiplexed_channels = n_multiplexed_channels;
      sys_.n_total_channels = sys_.n_multiplexed_channels;
      sys_.connector.resize(4);
      for (i=0; i<sys_.connector.size(); i++)
         sys_.connector[i].start_channel = i * 128;
      break;
   default:
      sys_.multiplexed = false;
//      if (n_bases > BASE_NUM_BASES) n_bases = BASE_NUM_BASES;
//      if (n_bases > ST_GetHWBASENumber())
//      {
//         ELOG(charES2_BASE_Number, ES2_BASE_Number);
//         n_bases = 1;
//      }
      sys_.n_bases = n_bases;
      sys_.n_active_channels = sys_.n_bases * MOD_MAX_CH;
      sys_.n_multiplexed_channels = 0;
      sys_.n_total_channels = sys_.n_active_channels;
      sys_.connector.resize(1);
      if (sys_.connector.size() > 0) sys_.connector[0].start_channel = 0;
      break;
   }

   sys_.id_base_first = id_base_first;
   sys_.id_base_last = sys_.id_base_first + sys_.n_bases - 1;

   // En la arquitectura actual, cada base FPGA representa un módulo de 32 canales.
   // Para el modo multiplexado, el switch anterior deja n_bases a 1.
   sys_.n_modules_base = 1;
   sys_.n_modules = sys_.n_bases * sys_.n_modules_base;
   sys_.id_mod_first = sys_.id_base_first;
   sys_.id_mod_last = sys_.id_mod_first + sys_.n_modules - 1;

   sys_.id_fl_first = 0;
   sys_.id_fl_last = 0;
   sys_.start_element = 0;
   sys_.actual_focal_law = 0;

//   if (sys_.n_total_channels != pin_to_channel.size())
//   {
//      pin_to_channel.resize(sys_.n_total_channels);
//      for (i=0; i<pin_to_channel.size(); i++) pin_to_channel[i] = i;
//   }
//   sys_.pin_to_channel.assign(pin_to_channel.begin(), pin_to_channel.end());

   update_ = true;
   update_pa_system_wmpar_ = true;
   return ES2_NONE;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int CPASystem::SetElementToChannel (vector<TInfoChannel> &element_to_channel)
{
   sys_.element_to_channel.assign(element_to_channel.begin(), element_to_channel.end());
   update_ = true;
//   if ((result = Set()) < 0) return RLOG(result);
   return ES2_NONE;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int CPASystem::SetPinToChannel (vector<unsigned short> &pin_to_channel)
{
   sys_.pin_to_channel.assign(pin_to_channel.begin(), pin_to_channel.end());
   update_ = true;
//   if ((result = Set()) < 0) return RLOG(result);
   return ES2_NONE;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int CPASystem::SetDisabledChannel (vector<unsigned short> &disabled_channel)
{
   sys_.disabled_channel.assign(disabled_channel.begin(), disabled_channel.end());
   update_ = true;
//   if ((result = Set()) < 0) return RLOG(result);
   return ES2_NONE;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int CPASystem::GetConnectorStartChannel (int id_connector)
{
   if (id_connector >= sys_.connector.size()) return ELOG(charES2_RANGO, ES2_RANGO);
   return sys_.connector[id_connector].start_channel;
}
//------------------------------------------------------------------------------
/**

*/
//------------------------------------------------------------------------------
int CPASystem::GetPinToChannel (vector<unsigned short> &pin_to_channel)
{
int result = 0;

   if (sys_.pin_to_channel.size() != sys_.n_total_channels)
      return ELOG(charES2_DataInconsistency, ES2_DataInconsistency);
   if (sys_.pin_to_channel.size() > 0)
   {
      pin_to_channel.assign(sys_.pin_to_channel.begin(), sys_.pin_to_channel.end());
      result = (int)pin_to_channel.size();
   }
   return result;
}
//------------------------------------------------------------------------------
/**

*/
//------------------------------------------------------------------------------
int CPASystem::GetDisabledChannel (vector<unsigned short> &disabled_channel)
{
int result = 0;

   if (sys_.disabled_channel.size() != sys_.n_total_channels)
      return ELOG(charES2_DataInconsistency, ES2_DataInconsistency);
   if (sys_.disabled_channel.size() > 0)
   {
      disabled_channel.assign(sys_.disabled_channel.begin(), sys_.disabled_channel.end());
      result = (int)disabled_channel.size();
   }
   return result;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int CPASystem::GetTotalMODULENumber (void)
{
   return sys_.id_mod_last - sys_.id_mod_first + 1;
}

// _____________________________________________________________________________
// ------ D E F I N I C I O N   D E   F U N C I O N E S
// =============================================================================
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int CPASystem::GetConnectorsNumber(void)
{
   int result = 0;

   if (sys_.frontend != NULL) result = (int)sys_.frontend->connector.size();
   return result;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
AnsiString CPASystem::GetConnectorName(int id_connector)
{
   AnsiString result = "";

   if (sys_.frontend != NULL && id_connector < sys_.frontend->connector.size())
   {
      result = sys_.frontend->connector[id_connector].name;
   }
   return result;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int CPASystem::GetFrontendChannelsNumber(void)
{
   int result = 0;

   if (sys_.frontend != NULL)
   {
      result = sys_.frontend->n_channels;
   }
   return result;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int CPASystem::GetConnectorChannelsNumber(int id_connector)
{
   int result = 0;

   if (sys_.frontend != NULL && id_connector < sys_.frontend->connector.size())
   {
      result = sys_.frontend->connector[id_connector].n_total_channels;
   }
   return result;
}



