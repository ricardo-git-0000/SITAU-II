#ifndef c_pa_systemH
#define c_pa_systemH

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

//#include "stinfo.h"
#include "t_pa_system_tad.h"
#include "t_base_tad.h"
#include "c_pa_system_config.h"

// _____________________________________________________________________________
// ------ D E F I N I C I O N   D E   C O N S T A N T E S
// =============================================================================

// _____________________________________________________________________________
// ------ T I P O S   D E   D A T O S
// =============================================================================

// _____________________________________________________________________________
// ------ CPASystem class
// _____________________________________________________________________________
// =============================================================================

class CPASystem : public CPASystem_Config
{
private:

   bool update_;
   bool update_pa_system_wmpar_;
   vector<T_PASystem_BASE_ShareVCH> base_vch_;

protected:

public:

   T_PASystemConfig sys_;
   vector<T_PASystem_VCH> total_vch_;

   CPASystem ( AnsiString name,
               int id,
               int id_base_first,
               int n_bases,
               int n_multiplexed_channels,
               int start_channel,
               TXMLHW_FRONTEND* frontend);

   ~CPASystem();

   int Setup ( AnsiString name,
               int id,
               int id_base_first,
               int n_bases,
               int n_multiplexed_channels,
               int start_channel);

   int SetElementToChannel (vector<TInfoChannel> &element_to_channel);
   int SetPinToChannel (vector<unsigned short> &pin_to_channel);
   int SetDisabledChannel (vector<unsigned short> &disabled_channel);

   bool Enabled(void) {return sys_.enabled;}
   bool Update(void) {return update_;}
   void Update(bool data) {update_ = data;}
   bool Multiplexed(void) {return sys_.multiplexed;}

   int GetChannelsNumber(void) {return sys_.n_total_channels;}
   int GetActiveChannelsNumber(void) {return sys_.n_active_channels;}
   int GetMultiplexedChannelsNumber(void) {return sys_.n_multiplexed_channels;}

   unsigned char GetBASEFirst(void) {return sys_.id_base_first;}
   unsigned char GetBASELast(void) {return sys_.id_base_last;}
   int GetBASENumber(void) {return sys_.n_bases;}

   unsigned char GetMODULEFirst(void) {return sys_.id_mod_first;}
   unsigned char GetMODULELast(void) {return sys_.id_mod_last;}
   int GetMODULENumber(void) {return sys_.n_modules;}
   int GetTotalMODULENumber(void);

   unsigned char GetFRONTENDId(void) {return sys_.id_frontend;}

   unsigned short GetFirstFocalLaw(void) {return sys_.id_fl_first;}
   unsigned short GetLastFocalLaw(void) {return sys_.id_fl_last;}

   unsigned char GetStartElement(void) {return sys_.start_element;}
   unsigned short GetActualFocalLaw(void) {return sys_.actual_focal_law;}

   bool GetEnabled(void) {return sys_.enabled;}
   AnsiString GetName(void) {return sys_.name;}
   int GetId(void) {return sys_.id;}

   int GetPinToChannel (vector<unsigned short> &pin_to_channel);
   int GetDisabledChannel (vector<unsigned short> &disabled_channel);

   int GetConnectorStartChannel (int id_connector);

   int CPASystem::GetConnectorsNumber(void);
   AnsiString CPASystem::GetConnectorName(int id_connector);
   int CPASystem::GetFrontendChannelsNumber(void);
   int GetConnectorChannelsNumber(int id_connector);


};

// _____________________________________________________________________________
// ------ V A R I A B L E S   G L O B A L E S
// =============================================================================

// _____________________________________________________________________________
// ------ P R O T O T I P O   D E   F U N C I O N E S
// =============================================================================

int ST_SetPASystemElementToChannel (vector<TInfoChannel> &element_to_channel);

int ST_SetPASystemDisabledChannel (vector<unsigned short> &disabled_channel);

#endif
