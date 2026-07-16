// -----------------------------------------------------------------------------
/**
 @file c_base.cpp

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

//#include <math.h>

// _____________________________________________________________________________
// ------ L I B R E R I A S   P R O P I A S   D E L   P R O Y E C T O
// =============================================================================

#include "c_base.h"
#include "base_error_code.h"
#include "c_log_file.h"
#include "protocol_base.h"
//#include "c_sitau_hw.h"

// _____________________________________________________________________________
// ------ V A R I A B L E S   G L O B A L E S
// =============================================================================

// _____________________________________________________________________________
// ------ CBASE_VirtualChannel class Methods
// _____________________________________________________________________________
// =============================================================================
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
CBASE_VirtualChannel::CBASE_VirtualChannel (int id_base_sitau, bool multiplexed)
   :  CBASE_Load(&reg_, &sys_, &update_register_),
      CBASE_Get(&reg_, &sys_, &update_register_),
      CBASE_Config(&reg_, &sys_, &update_register_)
{
   sys_.id_base_sitau = id_base_sitau;
   sys_.base_amplia_address = sys_.id_base_sitau * 8;
   sys_.multiplexed = multiplexed;
   update_register_ = false;
   ResetRegister();
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
CBASE_VirtualChannel::~CBASE_VirtualChannel()
{
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int CBASE_VirtualChannel::SetIdBase (int id_base_sitau, bool multiplexed)
{
   sys_.id_base_sitau = id_base_sitau;
   sys_.base_amplia_address = sys_.id_base_sitau * 8;
   sys_.multiplexed = multiplexed;
   return EBASE_NONE;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int CBASE_VirtualChannel::CopyConfig (CBASE_VirtualChannel *base)
{
   if (base == NULL) return ELOG(charEBASE_Parameter, EBASE_Parameter);
   sys_.id_base_sitau = base->sys_.id_base_sitau;
   sys_.base_amplia_address = base->sys_.base_amplia_address;
   sys_.multiplexed = base->sys_.multiplexed;
   return EBASE_NONE;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int CBASE_VirtualChannel::ResetRegister (void)
{
   reg_.beamformer.n_fl = 0;
   reg_.beamformer.n_fl_parallel = 8;
   reg_.beamformer.t0_max = 0;
	return EBASE_NONE;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int CBASE_VirtualChannel::GetRegisters (TMSG_BASE *base_reg)
{
   if (base_reg != NULL)
   {
      base_reg->enabled = 1;
      base_reg->id_base = sys_.id_base_sitau;
   }
   return EBASE_NONE;
}
