// _____________________________________________________________________________
// ------ L I B R E R I A S   P R O P I A S   D E L   C O M P I L A D O R
// =============================================================================

#if defined(_MSC_VER)
   #include <string>  
#endif

// _____________________________________________________________________________
// ------ L I B R E R I A S   P R O P I A S   D E L   P R O Y E C T O
// =============================================================================

#include "c_base_config.h"
#include "fileformat.h"
#include "base_error_code.h"
#include "c_log_file.h"
#include "st_reg.h"

// _____________________________________________________________________________
// ------ V A R I A B L E S   G L O B A L E S
// =============================================================================

// _____________________________________________________________________________
// ------ CBASE_Config class Methods
// _____________________________________________________________________________
// =============================================================================
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
CBASE_Config::CBASE_Config (TFP_VCH_BASE *reg, TFP_VCH_BASESysConfig *sys, bool *update_register)
{
   reg_ = reg;
   sys_ = sys;
   update_register_ = update_register;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
CBASE_Config::~CBASE_Config()
{
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int CBASE_Config::DecoFileVersion ( AnsiString version_file,
                                    unsigned short *field_1,
                                    unsigned short *field_2,
                                    unsigned short *field_3,
                                    unsigned short *field_4)
{
int idx;
AnsiString str;
#if defined(_MSC_VER)
int n_data = (int)version_file.length();

   for (str="", idx=1; idx<=n_data && version_file[idx] != '-'; idx++);

   for (str="", idx=1; idx<=n_data && version_file[idx] != '.'; idx++)
      str.insert(str.length(), 1, version_file[idx]);
   if (field_1 != NULL && str != "") *field_1 = atoi(str.c_str());

   for (idx++, str=""; idx<=n_data && version_file[idx] != '.'; idx++)
      str.insert(str.length(), 1, version_file[idx]);
   if (field_2 != NULL && str != "") *field_2 = atoi(str.c_str());

   for (idx++, str=""; idx<=n_data && version_file[idx] != '.'; idx++)
      str.insert(str.length(), 1, version_file[idx]);
   if (field_3 != NULL && str != "") *field_3 = atoi(str.c_str());

   for (idx++, str=""; idx<=n_data && version_file[idx] != '.'; idx++)
      str.insert(str.length(), 1, version_file[idx]);
   if (field_4 != NULL && str != "") *field_4 = atoi(str.c_str());
#else
int n_data = version_file.Length();

   for (str="", idx=1; idx<=n_data && version_file[idx] != '-'; idx++);

   for (str="", idx=1; idx<=n_data && version_file[idx] != '.'; idx++)
      str.Insert(version_file[idx], str.Length()+1);
   if (field_1 != NULL && str != "") *field_1 = atoi(str.c_str());

   for (idx++, str=""; idx<=n_data && version_file[idx] != '.'; idx++)
      str.Insert(version_file[idx], str.Length()+1);
   if (field_2 != NULL && str != "") *field_2 = atoi(str.c_str());

   for (idx++, str=""; idx<=n_data && version_file[idx] != '.'; idx++)
      str.Insert(version_file[idx], str.Length()+1);
   if (field_3 != NULL && str != "") *field_3 = atoi(str.c_str());

   for (idx++, str=""; idx<=n_data && version_file[idx] != '.'; idx++)
      str.Insert(version_file[idx], str.Length()+1);
   if (field_4 != NULL && str != "") *field_4 = atoi(str.c_str());
#endif
   return EBASE_NONE;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
void CBASE_Config::PrintFileConfig(FILE *f)
{
int idx, l, c, k;

    if (f != NULL && sys_ != NULL && reg_ != NULL)
    {  
        fprintf(f, "\n    ____________________________________________");
        fprintf(f, "\n    BASE (%d) Address: %d - Emitter delay (%d ns).-\n", sys_->id_base_sitau, sys_->base_amplia_address, (int)PCLK4_ns);
        for (idx=l=0; l<reg_->fp_fl.efl.n_fl && idx<reg_->fp_fl.efl.delay.size(); l++)
        {
            fprintf(f, "\n    (%4d):", l);
            for (c=0; idx<reg_->fp_fl.efl.delay.size() && c<MOD_MAX_CH;)
            {
                for (k=0; idx<reg_->fp_fl.efl.delay.size() && k<8; c++, k++, idx++) 
                {
                    if (reg_->fp_fl.efl.delay[idx] == 0xFFFF) fprintf(f, "  --- ");
                    else fprintf(f, " %5d", reg_->fp_fl.efl.delay[idx]);
                }
                fprintf(f, "   *");
            }               
        }
   }
}
