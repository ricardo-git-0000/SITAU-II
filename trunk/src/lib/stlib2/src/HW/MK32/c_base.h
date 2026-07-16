#ifndef c_baseH
#define c_baseH

// _____________________________________________________________________________
// ------ L I B R E R I A S   P R O P I A S   D E L   C O M P I L A D O R
// =============================================================================

#if defined(_MSC_VER)
   #include <vector>
#else
   #include <vector.h>
   #include <system.hpp>
#endif

#include "tad.h"

using namespace std;

// _____________________________________________________________________________
// ------ L I B R E R I A S   P R O P I A S   D E L   P R O Y E C T O
// =============================================================================

#include "t_base_tad.h"
#include "c_base_load.h"
#include "c_base_get.h"
#include "c_base_config.h"
#include "protocol_base.h"

// _____________________________________________________________________________
// ------ D E F I N I C I O N   D E   C O N S T A N T E S
// =============================================================================

// _____________________________________________________________________________
// ------ T I P O S   D E   D A T O S
// =============================================================================

// _____________________________________________________________________________
// ------ CBASE_VirtualChannel class
// _____________________________________________________________________________
// =============================================================================

class CBASE_VirtualChannel
   :  public CBASE_Load,
      public CBASE_Get,
      public CBASE_Config
{
private:

   bool update_register_;

protected:

public:

   TFP_VCH_BASESysConfig sys_;
   TFP_VCH_BASE reg_;

   CBASE_VirtualChannel (int id_base_sitau, bool multiplexed);

   ~CBASE_VirtualChannel();

   int SetIdBase (int id_base_sitau, bool multiplexed);

   int CopyConfig (CBASE_VirtualChannel *base);

   int ResetRegister (void);

   bool UpdateRegister(void) {return update_register_;}

   int GetRegisters (TMSG_BASE *base_reg);
};

// _____________________________________________________________________________
// ------ V A R I A B L E S   G L O B A L E S
// =============================================================================

// _____________________________________________________________________________
// ------ P R O T O T I P O   D E   F U N C I O N E S
// =============================================================================

#endif
