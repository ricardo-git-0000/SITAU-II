// -----------------------------------------------------------------------------
/**
 @file c_vch_fl_pa.cpp

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

#include "c_vch_fl_pa.h"
#include "c_virtual_channel.h"
#include "vch_error_code.h"
#include "c_log_file.h"
#include "stlib_dev.h"
#include "stlib2.h"
#include "protocol_uci.h"
#include "foc_params.h"
#include "sitau_hw.h"
#include "t_base_tad.h"

// _____________________________________________________________________________
// ------ V A R I A B L E S   G L O B A L E S
// =============================================================================

// _____________________________________________________________________________
// ------ CVirtualChannel class Methods
// _____________________________________________________________________________
// =============================================================================
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int CVirtualChannel::PRINT_Beamformer(void)
{
   int i, i_fl, i_ch, b, n_fl, n_ch_sitau;
   FILE* f_D0, * f_A0, * f_R0, * f_Z0, * f_T0, * f_E0;
   char str_char[255];
   time_t t;

#if !defined(_PA)
   return 0;
#endif

   if ((n_fl = cfg_.pa_focal_law.beamformer.size()) > 0)
   {
      t = time(NULL);
      strftime(str_char, 255, "%d/%m/%Y %H:%M:%S", localtime(&t));
      if ((f_D0 = fopen("VCH_D0.TXT", "wb")) != NULL)
      {
         fprintf(f_D0, "BEAMFORMER PARAMETERS (D_0) - %s", str_char);
         fprintf(f_D0, "\n-----------------------------------------------------------------\n      ");
      }
      if ((f_A0 = fopen("VCH_A0.TXT", "wb")) != NULL)
      {
         fprintf(f_A0, "BEAMFORMER PARAMETERS (A_0) - %s", str_char);
         fprintf(f_A0, "\n-----------------------------------------------------------------\n      ");
      }
      if ((f_R0 = fopen("VCH_R0.TXT", "wb")) != NULL)
      {
         fprintf(f_R0, "BEAMFORMER PARAMETERS (R_0) - %s", str_char);
         fprintf(f_R0, "\n-----------------------------------------------------------------\n      ");
      }
      if ((f_Z0 = fopen("VCH_Z0.TXT", "wb")) != NULL)
      {
         fprintf(f_Z0, "BEAMFORMER PARAMETERS (Z_0) - %s", str_char);
         fprintf(f_Z0, "\n-----------------------------------------------------------------\n      ");
      }
      if ((f_T0 = fopen("VCH_T0.TXT", "wb")) != NULL)
      {
         fprintf(f_T0, "BEAMFORMER PARAMETERS (T_0) - %s", str_char);
         fprintf(f_T0, "\n-----------------------------------------------------------------\n      ");
      }
      if ((f_E0 = fopen("VCH_E0.TXT", "wb")) != NULL)
      {
         fprintf(f_E0, "BEAMFORMER PARAMETERS (E_0) - %s", str_char);
         fprintf(f_E0, "\n-----------------------------------------------------------------\n      ");
      }
      n_ch_sitau = cfg_.pa_focal_law.beamformer[0].size();
      for (i_fl = 0; i_fl < n_ch_sitau; i_fl++)
      {
         if (f_D0 != NULL) fprintf(f_D0, "(%7d) ", i_fl + 1);
         if (f_A0 != NULL) fprintf(f_A0, "(%7d) ", i_fl + 1);
         if (f_R0 != NULL) fprintf(f_R0, "(%5d) ", i_fl + 1);
         if (f_Z0 != NULL) fprintf(f_Z0, "(%5d) ", i_fl + 1);
         if (f_T0 != NULL) fprintf(f_T0, "(%5d) ", i_fl + 1);
         if (f_E0 != NULL) fprintf(f_E0, "(%5d) ", i_fl + 1);
      }
      for (i_fl = 0; i_fl < n_fl; i_fl++)
      {
         if (f_D0 != NULL) fprintf(f_D0, "\n(%3d)  ", i_fl + 1);
         if (f_A0 != NULL) fprintf(f_A0, "\n(%3d)  ", i_fl + 1);
         if (f_R0 != NULL) fprintf(f_R0, "\n(%3d)  ", i_fl + 1);
         if (f_Z0 != NULL) fprintf(f_Z0, "\n(%3d)  ", i_fl + 1);
         if (f_T0 != NULL) fprintf(f_T0, "\n(%3d)  ", i_fl + 1);
         if (f_E0 != NULL) fprintf(f_E0, "\n(%3d)  ", i_fl + 1);
         n_ch_sitau = cfg_.pa_focal_law.beamformer[i_fl].size();
         for (i_ch = 0; i_ch < n_ch_sitau; i_ch++)
         {
            if (f_D0 != NULL) fprintf(f_D0, "%7d   ", cfg_.pa_focal_law.beamformer[i_fl][i_ch].D_0);
            if (f_A0 != NULL) fprintf(f_A0, "%7d   ", cfg_.pa_focal_law.beamformer[i_fl][i_ch].A_0);
            if (f_R0 != NULL) fprintf(f_R0, "%5d   ", cfg_.pa_focal_law.beamformer[i_fl][i_ch].R_0);
            if (f_Z0 != NULL) fprintf(f_Z0, "%5d   ", cfg_.pa_focal_law.beamformer[i_fl][i_ch].Z_0);
            if (f_T0 != NULL) fprintf(f_T0, "%5d   ", cfg_.pa_focal_law.beamformer[i_fl][i_ch].T_0);
            if (f_E0 != NULL) fprintf(f_E0, "%5d   ", cfg_.pa_focal_law.beamformer[i_fl][i_ch].E_0);
         }
      }
      if (f_D0 != NULL) fclose(f_D0);
      if (f_A0 != NULL) fclose(f_A0);
      if (f_R0 != NULL) fclose(f_R0);
      if (f_Z0 != NULL) fclose(f_Z0);
      if (f_T0 != NULL) fclose(f_T0);
      if (f_E0 != NULL) fclose(f_E0);
   }

   for (n_fl = b = 0; b < sys_.base.size(); b++)
   {
      if (sys_.base[b] != NULL)
      {
         if (b == 0) n_fl = sys_.base[b]->reg_.beamformer.bf_parameters.size();
         else if (n_fl != sys_.base[b]->reg_.beamformer.bf_parameters.size()) return ELOG(charEVCH_DataInconsistency, EVCH_DataInconsistency);
      }
   }

   if (n_fl > 0)
   {
      if ((f_D0 = fopen("HW_D0.TXT", "wb")) != NULL)
      {
         fprintf(f_D0, "BEAMFORMER PARAMETERS HARDWARE (D_0) - %s", str_char);
         fprintf(f_D0, "\n-----------------------------------------------------------------\n      ");
         fprintf(f_D0, "\n        ");
         for (i_ch = b = 0; b < sys_.base.size(); b++)
         {
            if (sys_.base[b] != NULL)
            {
               for (i_ch = 0; i_ch < MOD_MAX_CH; i_ch++) fprintf(f_D0, "Base (%d) ", b);
            }
         }
         fprintf(f_D0, "\n        ");
         for (i_ch = b = 0; b < sys_.base.size(); b++)
         {
            if (sys_.base[b] != NULL)
            {
               for (i = 0; i < MOD_MAX_CH; i++, i_ch++) fprintf(f_D0, "(%7d) ", i_ch);
            }
         }
         for (i_fl = 0; i_fl < n_fl; i_fl++)
         {
            fprintf(f_D0, "\n(%3d)    ", i_fl + 1);
            for (i = 0; i < sys_.base.size(); i++)
            {
               for (i_ch = 0; i_ch < MOD_MAX_CH && sys_.base[i]->reg_.beamformer.bf_parameters[i_fl].size(); i_ch++)
                  fprintf(f_D0, "%7d   ", sys_.base[i]->reg_.beamformer.bf_parameters[i_fl][i_ch].D_0);
            }
         }
      }
      if ((f_A0 = fopen("HW_A0.TXT", "wb")) != NULL)
      {
         fprintf(f_A0, "BEAMFORMER PARAMETERS HARDWARE (A_0) - %s", str_char);
         fprintf(f_A0, "\n-----------------------------------------------------------------\n      ");
         fprintf(f_A0, "\n        ");
         for (i_ch = b = 0; b < sys_.base.size(); b++)
         {
            if (sys_.base[b] != NULL)
            {
               for (i_ch = 0; i_ch < MOD_MAX_CH; i_ch++) fprintf(f_A0, "Base (%d) ", b);
            }
         }
         fprintf(f_A0, "\n        ");
         for (i_ch = b = 0; b < sys_.base.size(); b++)
         {
            if (sys_.base[b] != NULL)
            {
               for (i = 0; i < MOD_MAX_CH; i++, i_ch++) fprintf(f_A0, "(%7d) ", i_ch);
            }
         }
         for (i_fl = 0; i_fl < n_fl; i_fl++)
         {
            fprintf(f_A0, "\n(%3d)    ", i_fl + 1);
            for (i = 0; i < sys_.base.size(); i++)
            {
               for (i_ch = 0; i_ch < MOD_MAX_CH && sys_.base[i]->reg_.beamformer.bf_parameters[i_fl].size(); i_ch++)
                  fprintf(f_A0, "%7d   ", sys_.base[i]->reg_.beamformer.bf_parameters[i_fl][i_ch].A_0);
            }
         }
      }
      if ((f_R0 = fopen("HW_R0.TXT", "wb")) != NULL)
      {
         fprintf(f_R0, "BEAMFORMER PARAMETERS HARDWARE (R_0) - %s", str_char);
         fprintf(f_R0, "\n-----------------------------------------------------------------\n      ");
         fprintf(f_R0, "\n        ");
         for (i_ch = b = 0; b < sys_.base.size(); b++)
         {
            if (sys_.base[b] != NULL)
            {
               for (i_ch = 0; i_ch < MOD_MAX_CH; i_ch++) fprintf(f_R0, "Base(%d) ", b);
            }
         }
         fprintf(f_R0, "\n        ");
         for (i_ch = b = 0; b < sys_.base.size(); b++)
         {
            if (sys_.base[b] != NULL)
            {
               for (i = 0; i < MOD_MAX_CH; i++, i_ch++) fprintf(f_R0, "(%5d) ", i_ch);
            }
         }
         for (i_fl = 0; i_fl < n_fl; i_fl++)
         {
            fprintf(f_R0, "\n(%3d)    ", i_fl + 1);
            for (i = 0; i < sys_.base.size(); i++)
            {
               for (i_ch = 0; i_ch < MOD_MAX_CH && sys_.base[i]->reg_.beamformer.bf_parameters[i_fl].size(); i_ch++)
                  fprintf(f_R0, "%5d   ", sys_.base[i]->reg_.beamformer.bf_parameters[i_fl][i_ch].R_0);
            }
         }
      }
      if ((f_Z0 = fopen("HW_Z0.TXT", "wb")) != NULL)
      {
         fprintf(f_Z0, "BEAMFORMER PARAMETERS HARDWARE (Z_0) - %s", str_char);
         fprintf(f_Z0, "\n-----------------------------------------------------------------\n      ");
         fprintf(f_Z0, "\n        ");
         for (i_ch = b = 0; b < sys_.base.size(); b++)
         {
            if (sys_.base[b] != NULL)
            {
               for (i_ch = 0; i_ch < MOD_MAX_CH; i_ch++) fprintf(f_Z0, "Base(%d) ", b);
            }
         }
         fprintf(f_Z0, "\n        ");
         for (i_ch = b = 0; b < sys_.base.size(); b++)
         {
            if (sys_.base[b] != NULL)
            {
               for (i = 0; i < MOD_MAX_CH; i++, i_ch++) fprintf(f_Z0, "(%5d) ", i_ch);
            }
         }
         for (i_fl = 0; i_fl < n_fl; i_fl++)
         {
            fprintf(f_Z0, "\n(%3d)    ", i_fl + 1);
            for (i = 0; i < sys_.base.size(); i++)
            {
               for (i_ch = 0; i_ch < MOD_MAX_CH && sys_.base[i]->reg_.beamformer.bf_parameters[i_fl].size(); i_ch++)
                  fprintf(f_Z0, "%5d   ", sys_.base[i]->reg_.beamformer.bf_parameters[i_fl][i_ch].Z_0);
            }
         }
      }
      if ((f_T0 = fopen("HW_T0.TXT", "wb")) != NULL)
      {
         fprintf(f_T0, "BEAMFORMER PARAMETERS HARDWARE (T_0) - %s", str_char);
         fprintf(f_T0, "\n-----------------------------------------------------------------\n      ");
         fprintf(f_T0, "\n        ");
         for (i_ch = b = 0; b < sys_.base.size(); b++)
         {
            if (sys_.base[b] != NULL)
            {
               for (i_ch = 0; i_ch < MOD_MAX_CH; i_ch++) fprintf(f_T0, "Base(%d) ", b);
            }
         }
         fprintf(f_T0, "\n        ");
         for (i_ch = b = 0; b < sys_.base.size(); b++)
         {
            if (sys_.base[b] != NULL)
            {
               for (i = 0; i < MOD_MAX_CH; i++, i_ch++) fprintf(f_T0, "(%5d) ", i_ch);
            }
         }
         for (i_fl = 0; i_fl < n_fl; i_fl++)
         {
            fprintf(f_T0, "\n(%3d)    ", i_fl + 1);
            for (i = 0; i < sys_.base.size(); i++)
            {
               for (i_ch = 0; i_ch < MOD_MAX_CH && sys_.base[i]->reg_.beamformer.bf_parameters[i_fl].size(); i_ch++)
                  fprintf(f_T0, "%5d   ", sys_.base[i]->reg_.beamformer.bf_parameters[i_fl][i_ch].T_0);
            }
         }
      }
      if ((f_E0 = fopen("HW_E0.TXT", "wb")) != NULL)
      {
         fprintf(f_E0, "BEAMFORMER PARAMETERS HARDWARE (E_0) - %s", str_char);
         fprintf(f_E0, "\n-----------------------------------------------------------------\n      ");
         fprintf(f_E0, "\n        ");
         for (i_ch = b = 0; b < sys_.base.size(); b++)
         {
            if (sys_.base[b] != NULL)
            {
               for (i_ch = 0; i_ch < MOD_MAX_CH; i_ch++) fprintf(f_E0, "Base(%d) ", b);
            }
         }
         fprintf(f_E0, "\n        ");
         for (i_ch = b = 0; b < sys_.base.size(); b++)
         {
            if (sys_.base[b] != NULL)
            {
               for (i = 0; i < MOD_MAX_CH; i++, i_ch++) fprintf(f_E0, "(%5d) ", i_ch);
            }
         }
         for (i_fl = 0; i_fl < n_fl; i_fl++)
         {
            fprintf(f_E0, "\n(%3d)    ", i_fl + 1);
            for (i = 0; i < sys_.base.size(); i++)
            {
               for (i_ch = 0; i_ch < MOD_MAX_CH && sys_.base[i]->reg_.beamformer.bf_parameters[i_fl].size(); i_ch++)
                  fprintf(f_E0, "%5d   ", sys_.base[i]->reg_.beamformer.bf_parameters[i_fl][i_ch].E_0);
            }
         }
      }
      if (f_D0 != NULL) fclose(f_D0);
      if (f_A0 != NULL) fclose(f_A0);
      if (f_R0 != NULL) fclose(f_R0);
      if (f_Z0 != NULL) fclose(f_Z0);
      if (f_T0 != NULL) fclose(f_T0);
      if (f_E0 != NULL) fclose(f_E0);
   }
   return EVCH_NONE;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int CVirtualChannel::PRINT_TFM_Backward(void)
{
    int i, i_fl, i_ch, b, n_fl, n_ch_sitau;
    FILE* f_D0, * f_A0, * f_R0, * f_Z0, * f_T0, * f_E0;
    char str_char[255];
    time_t t;

#if !defined(_TFM)
    return 0;
#endif

    if ((n_fl = cfg_.tfm_backward.beamformer.size()) > 0)
    {
        t = time(NULL);
        strftime(str_char, 255, "%d/%m/%Y %H:%M:%S", localtime(&t));
        if ((f_D0 = fopen("VCH_TFM_BACKWARD_D0.TXT", "wb")) != NULL)
        {
            fprintf(f_D0, "TFM BACKWARD PARAMETERS (D_0) - %s", str_char);
            fprintf(f_D0, "\n-----------------------------------------------------------------\n      ");
        }
        if ((f_A0 = fopen("VCH_TFM_BACKWARD_A0.TXT", "wb")) != NULL)
        {
            fprintf(f_A0, "TFM BACKWARD PARAMETERS (A_0) - %s", str_char);
            fprintf(f_A0, "\n-----------------------------------------------------------------\n      ");
        }
        if ((f_R0 = fopen("VCH_TFM_BACKWARD_R0.TXT", "wb")) != NULL)
        {
            fprintf(f_R0, "TFM BACKWARD PARAMETERS (R_0) - %s", str_char);
            fprintf(f_R0, "\n-----------------------------------------------------------------\n      ");
        }
        if ((f_Z0 = fopen("VCH_TFM_BACKWARD_Z0.TXT", "wb")) != NULL)
        {
            fprintf(f_Z0, "TFM BACKWARD PARAMETERS (Z_0) - %s", str_char);
            fprintf(f_Z0, "\n-----------------------------------------------------------------\n      ");
        }
        if ((f_T0 = fopen("VCH_TFM_BACKWARD_T0.TXT", "wb")) != NULL)
        {
            fprintf(f_T0, "TFM BACKWARD PARAMETERS (T_0) - %s", str_char);
            fprintf(f_T0, "\n-----------------------------------------------------------------\n      ");
        }
        if ((f_E0 = fopen("VCH_TFM_BACKWARD_E0.TXT", "wb")) != NULL)
        {
            fprintf(f_E0, "TFM BACKWARD PARAMETERS (E_0) - %s", str_char);
            fprintf(f_E0, "\n-----------------------------------------------------------------\n      ");
        }
        n_ch_sitau = cfg_.tfm_backward.beamformer[0].size();
        for (i_fl = 0; i_fl < n_ch_sitau; i_fl++)
        {
            if (f_D0 != NULL) fprintf(f_D0, "(%7d) ", i_fl + 1);
            if (f_A0 != NULL) fprintf(f_A0, "(%7d) ", i_fl + 1);
            if (f_R0 != NULL) fprintf(f_R0, "(%5d) ", i_fl + 1);
            if (f_Z0 != NULL) fprintf(f_Z0, "(%5d) ", i_fl + 1);
            if (f_T0 != NULL) fprintf(f_T0, "(%5d) ", i_fl + 1);
            if (f_E0 != NULL) fprintf(f_E0, "(%5d) ", i_fl + 1);
        }
        for (i_fl = 0; i_fl < n_fl; i_fl++)
        {
            if (f_D0 != NULL) fprintf(f_D0, "\n(%3d)  ", i_fl + 1);
            if (f_A0 != NULL) fprintf(f_A0, "\n(%3d)  ", i_fl + 1);
            if (f_R0 != NULL) fprintf(f_R0, "\n(%3d)  ", i_fl + 1);
            if (f_Z0 != NULL) fprintf(f_Z0, "\n(%3d)  ", i_fl + 1);
            if (f_T0 != NULL) fprintf(f_T0, "\n(%3d)  ", i_fl + 1);
            if (f_E0 != NULL) fprintf(f_E0, "\n(%3d)  ", i_fl + 1);
            n_ch_sitau = cfg_.tfm_backward.beamformer[i_fl].size();
            for (i_ch = 0; i_ch < n_ch_sitau; i_ch++)
            {
                if (f_D0 != NULL) fprintf(f_D0, "%7d   ", cfg_.tfm_backward.beamformer[i_fl][i_ch].D_0);
                if (f_A0 != NULL) fprintf(f_A0, "%7d   ", cfg_.tfm_backward.beamformer[i_fl][i_ch].A_0);
                if (f_R0 != NULL) fprintf(f_R0, "%5d   ", cfg_.tfm_backward.beamformer[i_fl][i_ch].R_0);
                if (f_Z0 != NULL) fprintf(f_Z0, "%5d   ", cfg_.tfm_backward.beamformer[i_fl][i_ch].Z_0);
                if (f_T0 != NULL) fprintf(f_T0, "%5d   ", cfg_.tfm_backward.beamformer[i_fl][i_ch].T_0);
                if (f_E0 != NULL) fprintf(f_E0, "%5d   ", cfg_.tfm_backward.beamformer[i_fl][i_ch].E_0);
            }
        }
        if (f_D0 != NULL) fclose(f_D0);
        if (f_A0 != NULL) fclose(f_A0);
        if (f_R0 != NULL) fclose(f_R0);
        if (f_Z0 != NULL) fclose(f_Z0);
        if (f_T0 != NULL) fclose(f_T0);
        if (f_E0 != NULL) fclose(f_E0);
    }

    for (n_fl = b = 0; b < sys_.base.size(); b++)
    {
        if (sys_.base[b] != NULL)
        {
            if (b == 0) n_fl = sys_.base[b]->reg_.tfm_backward.bf_parameters.size();
            else if (n_fl != sys_.base[b]->reg_.tfm_backward.bf_parameters.size()) return ELOG(charEVCH_DataInconsistency, EVCH_DataInconsistency);
        }
    }

    if (n_fl > 0)
    {
        if ((f_D0 = fopen("HW_TFM_BACKWARD_D0.TXT", "wb")) != NULL)
        {
            fprintf(f_D0, "TFM BACKWARD PARAMETERS HARDWARE (D_0) - %s", str_char);
            fprintf(f_D0, "\n-----------------------------------------------------------------\n      ");
            fprintf(f_D0, "\n         ");
            for (i_ch = b = 0; b < sys_.base.size(); b++)
            {
                if (sys_.base[b] != NULL)
                {
                    for (i_ch = 0; i_ch < MOD_MAX_CH; i_ch++) fprintf(f_D0, "Base(%d)   ", b);
                }
            }
            fprintf(f_D0, "\n        ");
            for (i_ch = b = 0; b < sys_.base.size(); b++)
            {
                if (sys_.base[b] != NULL)
                {
                    for (i = 0; i < MOD_MAX_CH; i++, i_ch++) fprintf(f_D0, "(%7d) ", i_ch);
                }
            }
            for (i_fl = 0; i_fl < n_fl; i_fl++)
            {
                fprintf(f_D0, "\n(%3d)    ", i_fl + 1);
                for (i = 0; i < sys_.base.size(); i++)
                {
                    for (i_ch = 0; i_ch < MOD_MAX_CH && sys_.base[i]->reg_.tfm_backward.bf_parameters[i_fl].size(); i_ch++)
                        fprintf(f_D0, "%7d   ", sys_.base[i]->reg_.tfm_backward.bf_parameters[i_fl][i_ch].D_0);
                }
            }
        }
        if ((f_A0 = fopen("HW_TFM_BACKWARD_A0.TXT", "wb")) != NULL)
        {
            fprintf(f_A0, "TFM BACKWARD PARAMETERS HARDWARE (A_0) - %s", str_char);
            fprintf(f_A0, "\n-----------------------------------------------------------------\n      ");
            fprintf(f_A0, "\n         ");
            for (i_ch = b = 0; b < sys_.base.size(); b++)
            {
                if (sys_.base[b] != NULL)
                {
                    for (i_ch = 0; i_ch < MOD_MAX_CH; i_ch++) fprintf(f_A0, "Base(%d)   ", b);
                }
            }
            fprintf(f_A0, "\n        ");
            for (i_ch = b = 0; b < sys_.base.size(); b++)
            {
                if (sys_.base[b] != NULL)
                {
                    for (i = 0; i < MOD_MAX_CH; i++, i_ch++) fprintf(f_A0, "(%7d) ", i_ch);
                }
            }
            for (i_fl = 0; i_fl < n_fl; i_fl++)
            {
                fprintf(f_A0, "\n(%3d)    ", i_fl + 1);
                for (i = 0; i < sys_.base.size(); i++)
                {
                    for (i_ch = 0; i_ch < MOD_MAX_CH && sys_.base[i]->reg_.tfm_backward.bf_parameters[i_fl].size(); i_ch++)
                        fprintf(f_A0, "%7d   ", sys_.base[i]->reg_.tfm_backward.bf_parameters[i_fl][i_ch].A_0);
                }
            }
        }
        if ((f_R0 = fopen("HW_TFM_BACKWARD_R0.TXT", "wb")) != NULL)
        {
            fprintf(f_R0, "TFM BACKWARD PARAMETERS HARDWARE (R_0) - %s", str_char);
            fprintf(f_R0, "\n-----------------------------------------------------------------\n      ");
            fprintf(f_R0, "\n        ");
            for (i_ch = b = 0; b < sys_.base.size(); b++)
            {
                if (sys_.base[b] != NULL)
                {
                    for (i_ch = 0; i_ch < MOD_MAX_CH; i_ch++) fprintf(f_R0, "Base(%d) ", b);
                }
            }
            fprintf(f_R0, "\n        ");
            for (i_ch = b = 0; b < sys_.base.size(); b++)
            {
                if (sys_.base[b] != NULL)
                {
                    for (i = 0; i < MOD_MAX_CH; i++, i_ch++) fprintf(f_R0, "(%5d) ", i_ch);
                }
            }
            for (i_fl = 0; i_fl < n_fl; i_fl++)
            {
                fprintf(f_R0, "\n(%3d)    ", i_fl + 1);
                for (i = 0; i < sys_.base.size(); i++)
                {
                    for (i_ch = 0; i_ch < MOD_MAX_CH && sys_.base[i]->reg_.tfm_backward.bf_parameters[i_fl].size(); i_ch++)
                        fprintf(f_R0, "%5d   ", sys_.base[i]->reg_.tfm_backward.bf_parameters[i_fl][i_ch].R_0);
                }
            }
        }
        if ((f_Z0 = fopen("HW_TFM_BACKWARD_Z0.TXT", "wb")) != NULL)
        {
            fprintf(f_Z0, "TFM BACKWARD PARAMETERS HARDWARE (Z_0) - %s", str_char);
            fprintf(f_Z0, "\n-----------------------------------------------------------------\n      ");
            fprintf(f_Z0, "\n        ");
            for (i_ch = b = 0; b < sys_.base.size(); b++)
            {
                if (sys_.base[b] != NULL)
                {
                    for (i_ch = 0; i_ch < MOD_MAX_CH; i_ch++) fprintf(f_Z0, "Base(%d)  ", b);
                }
            }
            fprintf(f_Z0, "\n        ");
            for (i_ch = b = 0; b < sys_.base.size(); b++)
            {
                if (sys_.base[b] != NULL)
                {
                    for (i = 0; i < MOD_MAX_CH; i++, i_ch++) fprintf(f_Z0, "(%5d) ", i_ch);
                }
            }
            for (i_fl = 0; i_fl < n_fl; i_fl++)
            {
                fprintf(f_Z0, "\n(%3d)    ", i_fl + 1);
                for (i = 0; i < sys_.base.size(); i++)
                {
                    for (i_ch = 0; i_ch < MOD_MAX_CH && sys_.base[i]->reg_.tfm_backward.bf_parameters[i_fl].size(); i_ch++)
                        fprintf(f_Z0, "%5d   ", sys_.base[i]->reg_.tfm_backward.bf_parameters[i_fl][i_ch].Z_0);
                }
            }
        }
        if ((f_T0 = fopen("HW_TFM_BACKWARD_T0.TXT", "wb")) != NULL)
        {
            fprintf(f_T0, "TFM BACKWARD PARAMETERS HARDWARE (T_0) - %s", str_char);
            fprintf(f_T0, "\n-----------------------------------------------------------------\n      ");
            fprintf(f_T0, "\n        ");
            for (i_ch = b = 0; b < sys_.base.size(); b++)
            {
                if (sys_.base[b] != NULL)
                {
                    for (i_ch = 0; i_ch < MOD_MAX_CH; i_ch++) fprintf(f_T0, "Base(%d) ", b);
                }
            }
            fprintf(f_T0, "\n        ");
            for (i_ch = b = 0; b < sys_.base.size(); b++)
            {
                if (sys_.base[b] != NULL)
                {
                    for (i = 0; i < MOD_MAX_CH; i++, i_ch++) fprintf(f_T0, "(%5d) ", i_ch);
                }
            }
            for (i_fl = 0; i_fl < n_fl; i_fl++)
            {
                fprintf(f_T0, "\n(%3d)    ", i_fl + 1);
                for (i = 0; i < sys_.base.size(); i++)
                {
                    for (i_ch = 0; i_ch < MOD_MAX_CH && sys_.base[i]->reg_.tfm_backward.bf_parameters[i_fl].size(); i_ch++)
                        fprintf(f_T0, "%5d   ", sys_.base[i]->reg_.tfm_backward.bf_parameters[i_fl][i_ch].T_0);
                }
            }
        }
        if ((f_E0 = fopen("HW_TFM_BACKWARD_E0.TXT", "wb")) != NULL)
        {
            fprintf(f_E0, "TFM BACKWARD PARAMETERS HARDWARE (E_0) - %s", str_char);
            fprintf(f_E0, "\n-----------------------------------------------------------------\n      ");
            fprintf(f_E0, "\n        ");
            for (i_ch = b = 0; b < sys_.base.size(); b++)
            {
                if (sys_.base[b] != NULL)
                {
                    for (i_ch = 0; i_ch < MOD_MAX_CH; i_ch++) fprintf(f_E0, "Base(%d)  ", b);
                }
            }
            fprintf(f_E0, "\n        ");
            for (i_ch = b = 0; b < sys_.base.size(); b++)
            {
                if (sys_.base[b] != NULL)
                {
                    for (i = 0; i < MOD_MAX_CH; i++, i_ch++) fprintf(f_E0, "(%5d) ", i_ch);
                }
            }
            for (i_fl = 0; i_fl < n_fl; i_fl++)
            {
                fprintf(f_E0, "\n(%3d)    ", i_fl + 1);
                for (i = 0; i < sys_.base.size(); i++)
                {
                    for (i_ch = 0; i_ch < MOD_MAX_CH && sys_.base[i]->reg_.tfm_backward.bf_parameters[i_fl].size(); i_ch++)
                        fprintf(f_E0, "%5d   ", sys_.base[i]->reg_.tfm_backward.bf_parameters[i_fl][i_ch].E_0);
                }
            }
        }
        if (f_D0 != NULL) fclose(f_D0);
        if (f_A0 != NULL) fclose(f_A0);
        if (f_R0 != NULL) fclose(f_R0);
        if (f_Z0 != NULL) fclose(f_Z0);
        if (f_T0 != NULL) fclose(f_T0);
        if (f_E0 != NULL) fclose(f_E0);
    }
    return EVCH_NONE;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int CVirtualChannel::PRINT_TFM_FMC_Forward(void)
{
    int i_fl, i_ch, n_fl, n_lines;
    FILE* f_D0, * f_A0, * f_R0, * f_Z0, * f_T0, * f_E0;
    char str_char[255];
    time_t t;

#if !defined(_TFM)
    return 0;
#endif

    if ((n_fl = sys_.tfm_fmc_forward.beamformer.size()) > 0)
    {
        t = time(NULL);
        strftime(str_char, 255, "%d/%m/%Y %H:%M:%S", localtime(&t));
        if ((f_D0 = fopen("VCH_TFM_FORWARD_D0.TXT", "wb")) != NULL)
        {
            fprintf(f_D0, "TFM BACKWARD PARAMETERS (D_0) - %s", str_char);
            fprintf(f_D0, "\n-----------------------------------------------------------------\n      ");
        }
        if ((f_A0 = fopen("VCH_TFM_FORWARD_A0.TXT", "wb")) != NULL)
        {
            fprintf(f_A0, "BEAMFORMER PARAMETERS (A_0) - %s", str_char);
            fprintf(f_A0, "\n-----------------------------------------------------------------\n      ");
        }
        if ((f_R0 = fopen("VCH_TFM_FORWARD_R0.TXT", "wb")) != NULL)
        {
            fprintf(f_R0, "TFM FORWARD PARAMETERS (R_0) - %s", str_char);
            fprintf(f_R0, "\n-----------------------------------------------------------------\n      ");
        }
        if ((f_Z0 = fopen("VCH_TFM_FORWARD_Z0.TXT", "wb")) != NULL)
        {
            fprintf(f_Z0, "TFM FORWARD PARAMETERS (Z_0) - %s", str_char);
            fprintf(f_Z0, "\n-----------------------------------------------------------------\n      ");
        }
        if ((f_T0 = fopen("VCH_TFM_FORWARD_T0.TXT", "wb")) != NULL)
        {
            fprintf(f_T0, "TFM FORWARD PARAMETERS (T_0) - %s", str_char);
            fprintf(f_T0, "\n-----------------------------------------------------------------\n      ");
        }
        if ((f_E0 = fopen("VCH_TFM_FORWARD_E0.TXT", "wb")) != NULL)
        {
            fprintf(f_E0, "TFM FORWARD PARAMETERS (E_0) - %s", str_char);
            fprintf(f_E0, "\n-----------------------------------------------------------------\n      ");
        }
        n_lines = sys_.tfm_fmc_forward.beamformer[0].size();
        for (i_fl = 0; i_fl < n_lines; i_fl++)
        {
            if (f_D0 != NULL) fprintf(f_D0, "(%7d) ", i_fl + 1);
            if (f_A0 != NULL) fprintf(f_A0, "(%7d) ", i_fl + 1);
            if (f_R0 != NULL) fprintf(f_R0, "(%5d) ", i_fl + 1);
            if (f_Z0 != NULL) fprintf(f_Z0, "(%5d) ", i_fl + 1);
            if (f_T0 != NULL) fprintf(f_T0, "(%5d) ", i_fl + 1);
            if (f_E0 != NULL) fprintf(f_E0, "(%5d) ", i_fl + 1);
        }
        for (i_fl = 0; i_fl < n_fl; i_fl++)
        {
            if (f_D0 != NULL) fprintf(f_D0, "\n(%3d)  ", i_fl + 1);
            if (f_A0 != NULL) fprintf(f_A0, "\n(%3d)  ", i_fl + 1);
            if (f_R0 != NULL) fprintf(f_R0, "\n(%3d)  ", i_fl + 1);
            if (f_Z0 != NULL) fprintf(f_Z0, "\n(%3d)  ", i_fl + 1);
            if (f_T0 != NULL) fprintf(f_T0, "\n(%3d)  ", i_fl + 1);
            if (f_E0 != NULL) fprintf(f_E0, "\n(%3d)  ", i_fl + 1);
            n_lines = sys_.tfm_fmc_forward.beamformer[i_fl].size();
            for (i_ch = 0; i_ch < n_lines; i_ch++)
            {
                if (f_D0 != NULL) fprintf(f_D0, "%7d   ", sys_.tfm_fmc_forward.beamformer[i_fl][i_ch].D_0);
                if (f_A0 != NULL) fprintf(f_A0, "%7d   ", sys_.tfm_fmc_forward.beamformer[i_fl][i_ch].A_0);
                if (f_R0 != NULL) fprintf(f_R0, "%5d   ", sys_.tfm_fmc_forward.beamformer[i_fl][i_ch].R_0);
                if (f_Z0 != NULL) fprintf(f_Z0, "%5d   ", sys_.tfm_fmc_forward.beamformer[i_fl][i_ch].Z_0);
                if (f_T0 != NULL) fprintf(f_T0, "%5d   ", sys_.tfm_fmc_forward.beamformer[i_fl][i_ch].T_0);
                if (f_E0 != NULL) fprintf(f_E0, "%5d   ", sys_.tfm_fmc_forward.beamformer[i_fl][i_ch].E_0);
            }
        }
        if (f_D0 != NULL) fclose(f_D0);
        if (f_A0 != NULL) fclose(f_A0);
        if (f_R0 != NULL) fclose(f_R0);
        if (f_Z0 != NULL) fclose(f_Z0);
        if (f_T0 != NULL) fclose(f_T0);
        if (f_E0 != NULL) fclose(f_E0);
    }
    return EVCH_NONE;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int CVirtualChannel::PRINT_TFM_PWI_Forward(void)
{
    int i_fl, i_ch, n_fl, n_lines;
    FILE* f_F, * f_T, * f_Angle;
    char str_char[255];
    time_t t;

#if !defined(_TFM)
    return 0;
#endif

    if ((n_fl = sys_.tfm_pwi_forward.forwardBeamformer.size()) > 0)
    {
        t = time(NULL);
        strftime(str_char, 255, "%d/%m/%Y %H:%M:%S", localtime(&t));
        if ((f_F = fopen("VCH_TFM_PWI_FORWARD_F.TXT", "wb")) != NULL)
        {
            fprintf(f_F, "TFM PWI FORWARD PARAMETERS (F) - %s", str_char);
            fprintf(f_F, "\n-----------------------------------------------------------------\n      ");
        }
        if ((f_T = fopen("VCH_TFM_PWI_FORWARD_T.TXT", "wb")) != NULL)
        {
            fprintf(f_T, "TFM PWI FORWARD PARAMETERS (T) - %s", str_char);
            fprintf(f_T, "\n-----------------------------------------------------------------\n      ");
        }
        if ((f_Angle = fopen("VCH_TFM_FORWARD_ANGLE.TXT", "wb")) != NULL)
        {
            fprintf(f_Angle, "TFM PWI FORWARD PARAMETERS (Angle) - %s", str_char);
            fprintf(f_Angle, "\n-----------------------------------------------------------------\n");
        }
        n_lines = sys_.tfm_pwi_forward.anglesBeamformer.size();
        for (i_fl = 0; i_fl < n_lines; i_fl++)
        {
            if (f_Angle != NULL) fprintf(f_Angle, "(%5d) ", i_fl + 1);
            if (f_Angle != NULL) fprintf(f_Angle, "%7d\n", sys_.tfm_pwi_forward.anglesBeamformer[i_fl].COS_PWI);
        }
        if (f_Angle != NULL) fclose(f_Angle);

        n_lines = sys_.tfm_pwi_forward.forwardBeamformer[0].size();
        for (i_fl = 0; i_fl < n_lines; i_fl++)
        {
            if (f_F != NULL) fprintf(f_F, "(%7d) ", i_fl + 1);
            if (f_T != NULL) fprintf(f_T, "(%7d) ", i_fl + 1);
        }
        for (i_fl = 0; i_fl < n_fl; i_fl++)
        {
            if (f_F != NULL) fprintf(f_F, "\n(%3d)  ", i_fl + 1);
            if (f_T != NULL) fprintf(f_T, "\n(%3d)  ", i_fl + 1);
            n_lines = sys_.tfm_pwi_forward.forwardBeamformer[i_fl].size();
            for (i_ch = 0; i_ch < n_lines; i_ch++)
            {
                if (f_F != NULL) fprintf(f_F, "%7d   ", sys_.tfm_pwi_forward.forwardBeamformer[i_fl][i_ch].F_0);
                if (f_T != NULL) fprintf(f_T, "%7d   ", sys_.tfm_pwi_forward.forwardBeamformer[i_fl][i_ch].T_0);
            }
        }
        if (f_F != NULL) fclose(f_F);
        if (f_T != NULL) fclose(f_T);
    }
    return EVCH_NONE;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int CVirtualChannel::LOAD_PA_FocalLaw (void)
{
int i, id_ap, i_fl, i_ch, result, n_fl, c, n_ch_sitau, n_ch_pa;
vector<vector<TFP_VCH_BASE_BeamformerParameters> > beamformer;
vector<unsigned short> start_element;
TFP_VCH_BASE_BeamformerParameters beamformer_default = {0, 0, 0, 0, 0, 1};
vector<T_VCH_EmitterFocalLaw> efl;
float e_tick;

   if (cfg_.scan == NULL) return ELOG(charEVCH_SCAN_NULL, EVCH_SCAN_NULL);
   //UpdateAcquisitionSize();
   if ((result = PA_UpdateAcquisitionSize()) < 0) return RLOG(result);

   n_ch_sitau = GET_ChannelsNumber();

   // ... EMISSION FOCAL LAW
   if ((e_tick = cfg_.scan->GetEmissionTick()) <= 0) return ELOG(charEVCH_DataInconsistency, EVCH_DataInconsistency);
   n_fl = (int)cfg_.scan->fl_.emission_focal_law_ticks.size();
   if (n_ch_sitau > 0 && n_fl > 0)
   {
      cfg_.pa_emission_focal_law.clear();
      efl.resize(n_ch_sitau);
      for (i = 0; i < n_fl && i < cfg_.scan->fl_.emission_focal_law_ticks.size() && i < cfg_.scan->fl_.start_element.size(); i++)
      {
		   for (c = 0; c < cfg_.scan->fl_.start_element[i] && c < n_ch_sitau; c++)
		   {
            efl[c].emitter_delay = 0;
            efl[c].emitter_delay_ticks = 0;
            efl[c].emitter_enabled = 0;
         }               
		   for (id_ap = 0; id_ap < cfg_.scan->fl_.emission_focal_law_ticks[i].size() && c < n_ch_sitau; c++, id_ap++)
		   {
            efl[c].emitter_delay = cfg_.scan->fl_.emission_focal_law_ticks[i][id_ap] * e_tick;
            efl[c].emitter_delay_ticks = cfg_.scan->fl_.emission_focal_law_ticks[i][id_ap];
            efl[c].emitter_enabled = 1;
		   }
		   for (; c< n_ch_sitau; c++)
		   {
            efl[c].emitter_delay_ticks = 0;
            efl[c].emitter_enabled = 0;
		   }
         cfg_.pa_emission_focal_law.push_back(efl);
      }
   }
   // ... RECEPTION FOCAL LAW

   if ((result = cfg_.scan->GetBeamformerParameters(beamformer)) < 0) return RLOG(result);
   n_fl = beamformer.size();
   cfg_.scan->GetReceptionStartElement(start_element);
   if (start_element.size() != n_fl) return ELOG(charEVCH_DataInconsistency, EVCH_DataInconsistency);

   // Asignación de Pines
   cfg_.pa_focal_law.beamformer.resize(n_fl);
   for (i_fl = 0; i_fl < n_fl; i_fl++)
   {
      cfg_.pa_focal_law.beamformer[i_fl].resize(n_ch_sitau);
      for (i_ch =0 ; i_ch < n_ch_sitau; i_ch++) cfg_.pa_focal_law.beamformer[i_fl][i_ch] = beamformer_default;

      n_ch_pa = beamformer[i_fl].size();
      for (i = 0, i_ch = start_element[i_fl]; i_ch < n_ch_sitau && i < n_ch_pa; i_ch++, i++)
      {
         cfg_.pa_focal_law.beamformer[i_fl][i_ch] = beamformer[i_fl][i];
      }
   }
   
   if ((result = LOAD_Beamformer()) < 0) return RLOG(result);
   if (cfg_.acquisition_type == ACQUISITION_TYPE_PA)
   {
       if ((result = LOAD_EmissionFocalLaw(cfg_.pa_emission_focal_law)) < 0) return RLOG(result);
   }
   return EVCH_NONE;
}
//------------------------------------------------------------------------------
/**

*/
//------------------------------------------------------------------------------
int CVirtualChannel::SET_Beamformer(void)
{
int b, c, result, connector_start_channel;
int id_base_first = 0, id_base_last = 0;
TMSG_Beamformer *msg = NULL;
TMSG_BeamformerExtended* msg_ext = NULL;

#if !defined(_PA)
    return EVCH_NONE;
#endif

try
{
   if (update_focal_law_pa_)
   {
      if (sys_.pa_sys != NULL)
      {
         if ((connector_start_channel = sys_.pa_sys->GetConnectorStartChannel(sys_.id_connector)) < 0)
            return RLOG(connector_start_channel);
      }
      else return ELOG(charEVCH_SYSTEM_NULL, EVCH_SYSTEM_NULL);
     
      if ((msg = (TMSG_Beamformer *)malloc(sizeof(TMSG_Beamformer))) == NULL)
         return ELOG(charEVCH_NoMemory, EVCH_NoMemory);

      msg->id_vch = cfg_.id;
      msg->n_acq_samples = cfg_.acq.n_acq_samples;
      msg->n_beamformed_samples = cfg_.acq.n_beamformed_samples;
      msg->beamforming_type = BEAMFORMING_PA;

      msg->n_fl = sys_.base[0]->reg_.beamformer.n_fl;
      msg->n_fl_parallel = sys_.base[0]->reg_.beamformer.n_fl_parallel;// == 2 ? 1 : sys_.base[b]->reg_.beamformer.n_fl_parallel;
      for (msg->t0_max = b = 0; b < sys_.base.size(); b++)
      {
         if (sys_.base[b]->reg_.beamformer.t0_max > msg->t0_max) msg->t0_max = sys_.base[b]->reg_.beamformer.t0_max;
      }
      msg->n_lword = sys_.base[0]->reg_.beamformer.n_lword;
      msg->shift_bits = sys_.base[0]->reg_.beamformer.shift_bits;

      for (c=b=0; b<sys_.base.size() && b < MAX_N_BASES_MSG; b++)
      {
         if (sys_.base[b] != NULL)
         {
            memcpy(msg->base[b].bf_memory, sys_.base[b]->reg_.beamformer.bf_memory, FP_BASE_BEAMFORMER_MEMORY_SIZE * sizeof(u32));
         }
      }
      if (b < sys_.base.size())
      {
          if ((msg_ext = (TMSG_BeamformerExtended*)malloc(sizeof(TMSG_BeamformerExtended))) == NULL)
              return ELOG(charEVCH_NoMemory, EVCH_NoMemory);
          
          msg_ext->id_first_base = b;
          msg_ext->id_vch = cfg_.id;
          msg_ext->n_acq_samples = cfg_.acq.n_acq_samples;
          msg_ext->n_beamformed_samples = cfg_.acq.n_beamformed_samples;
          msg_ext->beamforming_type = BEAMFORMING_PA;

          msg_ext->n_fl = sys_.base[0]->reg_.beamformer.n_fl;
          msg_ext->n_fl_parallel = sys_.base[0]->reg_.beamformer.n_fl_parallel;// == 2 ? 1 : sys_.base[b]->reg_.beamformer.n_fl_parallel;
          // El segundo mensaje comienza en id_first_base y usa sus posiciones 0..3.
          for (msg_ext->t0_max = b = 0; b < sys_.base.size() - msg_ext->id_first_base && b < MAX_N_BASES_MSG; b++)
          {
              if (sys_.base[msg_ext->id_first_base + b]->reg_.beamformer.t0_max > msg_ext->t0_max) msg_ext->t0_max = sys_.base[msg_ext->id_first_base + b]->reg_.beamformer.t0_max;
          }
          msg_ext->n_lword = sys_.base[0]->reg_.beamformer.n_lword;
          msg_ext->shift_bits = sys_.base[0]->reg_.beamformer.shift_bits;

          for (c = b = 0; b < sys_.base.size() - msg_ext->id_first_base && b < MAX_N_BASES_MSG; b++)
          {
              if (sys_.base[msg_ext->id_first_base + b] != NULL)
              {
                  memcpy(msg_ext->base[b].bf_memory, sys_.base[msg_ext->id_first_base + b]->reg_.beamformer.bf_memory, FP_BASE_BEAMFORMER_MEMORY_SIZE * sizeof(u32));
              }
          }
      }

      //if (acquiring == 1 && (result = ST_Stop()) < 0) { RLOG(result); }
      if ((result = TCPSend_MSG_VirtualChannel_Beamformer(msg)) < 0) return RLOG(result);
      if (msg_ext != NULL && (result = TCPSend_MSG_VirtualChannel_BeamformerExtended(msg_ext)) < 0) return RLOG(result);
      //if (acquiring == 1) ST_Start();
      update_focal_law_pa_ = false;
   }
}
__finally
{
   if (msg != NULL) free(msg);
}
   return EVCH_NONE;
}
//------------------------------------------------------------------------------
/**

*/
//------------------------------------------------------------------------------
int CVirtualChannel::SET_TFMBackward(void)
{
    int b, c, result, connector_start_channel;
    int id_base_first = 0, id_base_last = 0;
    TMSG_Beamformer* msg = NULL;
    TMSG_BeamformerExtended* msg_ext = NULL;
    //int acquiring = ST_Acquiring();

#if !defined(_PA)
    return EVCH_NONE;
#endif

    try
    {
        if (update_tfm_backward_)
        {
            if (sys_.pa_sys != NULL)
            {
                if ((connector_start_channel = sys_.pa_sys->GetConnectorStartChannel(sys_.id_connector)) < 0)
                    return RLOG(connector_start_channel);
            }
            else return ELOG(charEVCH_SYSTEM_NULL, EVCH_SYSTEM_NULL);

            if ((msg = (TMSG_Beamformer*)malloc(sizeof(TMSG_Beamformer))) == NULL)
                return ELOG(charEVCH_NoMemory, EVCH_NoMemory);

            msg->id_vch = cfg_.id;
            msg->n_acq_samples = cfg_.acq.n_acq_samples;
            msg->n_beamformed_samples = cfg_.acq.n_beamformed_samples;
            if (cfg_.tfm.focal_laws_type == VCH_FP_FL_TYPE_PWI) msg->beamforming_type = BEAMFORMING_PWI;
            else msg->beamforming_type = BEAMFORMING_TFM;// BEAMFORMING_TFM; RIC Cambiar

            msg->n_fl = sys_.base[0]->reg_.tfm_backward.n_fl;
            msg->n_fl_parallel = cfg_.tfm_backward.parallel_lines;// sys_.base[0]->reg_.tfm_backward.n_fl_parallel;// == 2 ? 1 : sys_.base[b]->reg_.beamformer.n_fl_parallel;
            for (msg->t0_max = b = 0; b < sys_.base.size(); b++)
            {
                if (sys_.base[b]->reg_.tfm_backward.t0_max > msg->t0_max) msg->t0_max = sys_.base[b]->reg_.tfm_backward.t0_max;
            }
            msg->n_lword = sys_.base[0]->reg_.tfm_backward.n_lword;
            msg->shift_bits = sys_.base[0]->reg_.tfm_backward.shift_bits;

            for (c = b = 0; b < sys_.base.size() && b < MAX_N_BASES_MSG; b++)
            {
                if (sys_.base[b] != NULL)
                {
                    memcpy(msg->base[b].bf_memory, sys_.base[b]->reg_.tfm_backward.bf_memory, FP_BASE_BEAMFORMER_MEMORY_SIZE * sizeof(u32));
                }
            }

            if (b < sys_.base.size())
            {
                if ((msg_ext = (TMSG_BeamformerExtended*)malloc(sizeof(TMSG_BeamformerExtended))) == NULL)
                    return ELOG(charEVCH_NoMemory, EVCH_NoMemory);

                msg_ext->id_first_base = b;
                msg_ext->id_vch = cfg_.id;
                msg_ext->n_acq_samples = cfg_.acq.n_acq_samples;
                msg_ext->n_beamformed_samples = cfg_.acq.n_beamformed_samples;
                msg_ext->beamforming_type = msg->beamforming_type;
                msg_ext->n_fl = sys_.base[0]->reg_.tfm_backward.n_fl;
                msg_ext->n_fl_parallel = cfg_.tfm_backward.parallel_lines;

                // El mensaje extendido transporta las bases restantes en posiciones 0..3.
                for (msg_ext->t0_max = b = 0; b < sys_.base.size() - msg_ext->id_first_base && b < MAX_N_BASES_MSG; b++)
                {
                    if (sys_.base[msg_ext->id_first_base + b]->reg_.tfm_backward.t0_max > msg_ext->t0_max)
                        msg_ext->t0_max = sys_.base[msg_ext->id_first_base + b]->reg_.tfm_backward.t0_max;
                }
                msg_ext->n_lword = sys_.base[0]->reg_.tfm_backward.n_lword;
                msg_ext->shift_bits = sys_.base[0]->reg_.tfm_backward.shift_bits;

                for (c = b = 0; b < sys_.base.size() - msg_ext->id_first_base && b < MAX_N_BASES_MSG; b++)
                {
                    if (sys_.base[msg_ext->id_first_base + b] != NULL)
                    {
                        memcpy(msg_ext->base[b].bf_memory,
                            sys_.base[msg_ext->id_first_base + b]->reg_.tfm_backward.bf_memory,
                            FP_BASE_BEAMFORMER_MEMORY_SIZE * sizeof(u32));
                    }
                }
            }

            //if (acquiring == 1 && (result = ST_Stop()) < 0) { RLOG(result); }
            if ((result = TCPSend_MSG_VirtualChannel_Beamformer(msg)) < 0) return RLOG(result);
            if (msg_ext != NULL && (result = TCPSend_MSG_VirtualChannel_BeamformerExtended(msg_ext)) < 0) return RLOG(result);
            //if (acquiring == 1) ST_Start();
            update_tfm_backward_ = false;
        }
    }
    __finally
    {
        if (msg != NULL) free(msg);
        if (msg_ext != NULL) free(msg_ext);
    }
    return EVCH_NONE;
}
//------------------------------------------------------------------------------
/**

*/
//------------------------------------------------------------------------------
int CVirtualChannel::TFM_SET_FMC_Forward(void)
{
    int result;
    TMSG_TFM_FMC_Forward* msg = NULL;
    //int acquiring = ST_Acquiring();

#if !defined(_TFM)
    return EVCH_NONE;
#endif

    try
    {
        if (update_tfm_forward_)
        {
            if ((msg = (TMSG_TFM_FMC_Forward*)malloc(sizeof(TMSG_TFM_FMC_Forward))) == NULL) return ELOG(charEVCH_NoMemory, EVCH_NoMemory);
            msg->id_vch = cfg_.id;
            msg->size32_memory = sys_.tfm_fmc_forward.n_lword;
            msg->size32_forward_focal_law = 0;
            msg->n_forward_focal_law = sys_.tfm_fmc_forward.beamformer.size();
            memcpy(msg->memory, sys_.tfm_fmc_forward.memory, TFM_FMC_FORWARD_MEMORY_SIZE_MSG * sizeof(u32));

            //if (acquiring == 1 && (result = ST_Stop()) < 0) { RLOG(result); }
            if ((result = TCPSend_MSG_TFM_FMC_Forward(msg)) < 0) return RLOG(result);
            //if (acquiring == 1) ST_Start();
            update_tfm_forward_ = false;
        }
    }
    __finally
    {
        if (msg != NULL) free(msg);
    }
    return EVCH_NONE;
}
//------------------------------------------------------------------------------
/**

*/
//------------------------------------------------------------------------------
int CVirtualChannel::TFM_SET_PWI_Forward(void)
{
    int result;
    TMSG_TFM_PWI_Forward* msg = NULL;

#if !defined(_TFM)
    return EVCH_NONE;
#endif

    try
    {
        if (update_tfm_forward_)
        {
            if ((msg = (TMSG_TFM_PWI_Forward*)malloc(sizeof(TMSG_TFM_PWI_Forward))) == NULL) return ELOG(charEVCH_NoMemory, EVCH_NoMemory);
            msg->id_vch = cfg_.id;
            msg->size32_memory = sys_.tfm_pwi_forward.n_lword;
            msg->size32_forward_focal_law = 0;
            msg->n_forward_focal_law = sys_.tfm_pwi_forward.forwardBeamformer.size();
            memcpy(msg->forwardMemory, sys_.tfm_pwi_forward.forwardMemory, TFM_PWI_FORWARD_MEMORY_SIZE_MSG * sizeof(u32));
            memcpy(msg->anglesMemory, sys_.tfm_pwi_forward.anglesMemory, MAX_PWI_FL * sizeof(u16));

            if ((result = TCPSend_MSG_TFM_PWI_Forward(msg)) < 0) return RLOG(result);
            update_tfm_forward_ = false;
        }
    }
    __finally
    {
        if (msg != NULL) free(msg);
    }
    return EVCH_NONE;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int CVirtualChannel::LOAD_PA_LinearScanConfig (int step, int aperture, int startelement, int endelement, float angle)
{
int result, n_fl;
vector<float> v_aux_float;
vector<int> v_aux_int;

    if (cfg_.scan == NULL) return ELOG(charEVCH_SCAN_NULL, EVCH_SCAN_NULL);
    if (aperture > GET_Probe_ElementNumber()) aperture = ST_GetProbeElementNumber();
    if (startelement > GET_Probe_ElementNumber()) startelement = ST_GetProbeElementNumber();
	if ((n_fl = cfg_.scan->ConfigLinearScanStd (startelement, aperture, startelement, aperture, 1, endelement, step, angle)) < 0)
        return RLOG(n_fl);

    if ((result = RELOAD_Gain()) < 0) return RLOG(result);
    if ((result = LOAD_PA_FocalLaw()) < 0) return RLOG(result); 
    return EVCH_NONE;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int CVirtualChannel::LOAD_PA_AngularScanConfig (float step, unsigned short aperture, unsigned short startelement, float startangle, float endangle)
{
int n_fl, result;

   if (cfg_.scan == NULL) return ELOG(charEVCH_SCAN_NULL, EVCH_SCAN_NULL);
   if (aperture >= GET_Probe_ElementNumber()) aperture = ST_GetProbeElementNumber();
   if (startelement >= GET_Probe_ElementNumber()) startelement = ST_GetProbeElementNumber();

	if ((n_fl = cfg_.scan->ConfigSectorialScanStd (	startelement,
                                       				aperture,
                                       				startelement,
																	aperture,
                                                   1,
                                       				startangle,
                                       				endangle,
                                       				step)) < 0)
        return RLOG(n_fl);

   if ((result = LOAD_PA_FocalLaw()) < 0) return RLOG(result);
	return EVCH_NONE;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int CVirtualChannel::SET_PA_LinearScanConfig (int step, int aperture, int startelement, int endelement, float angle)
{
    int result, acquiring = ST_Acquiring();

    if (acquiring == 1 && (result = ST_Stop()) < 0) { RLOG(result); }
    BufferPA_HWUT_Clear();
    if ((result = LOAD_PA_LinearScanConfig (step, aperture, startelement, endelement, angle)) < 0) 
        return RLOG(result);
    if ((result = Set()) < 0) return RLOG(result);
    if (acquiring == 1) ST_Start();
    return EVCH_NONE;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int CVirtualChannel::SET_PA_AngularScanConfig (float step, unsigned short aperture, unsigned short startelement, float startangle, float endangle)
{
    int result, acquiring = ST_Acquiring();

    if (acquiring == 1 && (result = ST_Stop()) < 0) { RLOG(result); }
    BufferPA_HWUT_Clear();
    if ((result = LOAD_PA_AngularScanConfig (step, aperture, startelement, startangle, endangle)) < 0) 
        return RLOG(result);
    if ((result = Set()) < 0) return RLOG(result);
    if (acquiring == 1) ST_Start();
    return EVCH_NONE;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int CVirtualChannel::GET_PA_ReceptionAperture (void)
{
   if (cfg_.scan == NULL) return ELOG(charEVCH_SCAN_NULL,EVCH_SCAN_NULL);
   return cfg_.scan->GetScanReceptionAperture();
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int CVirtualChannel::GET_PA_EmissionAperture (void)
{
   if (cfg_.scan == NULL) return ELOG(charEVCH_SCAN_NULL,EVCH_SCAN_NULL);
   return cfg_.scan->GetScanEmissionAperture();
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int CVirtualChannel::GET_PA_ScanType (void)
{
   if (cfg_.scan == NULL) return ELOG(charEVCH_SCAN_NULL,EVCH_SCAN_NULL);
   return cfg_.scan->GetScanType();
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int CVirtualChannel::GET_PA_LinearStep (void)
{
   if (cfg_.scan == NULL) return ELOG(charEVCH_SCAN_NULL,EVCH_SCAN_NULL);
   return cfg_.scan->GetLinearScanStep();
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int CVirtualChannel::GET_PA_ReceptionStartElement (void)
{
   if (cfg_.scan == NULL) return ELOG(charEVCH_SCAN_NULL,EVCH_SCAN_NULL);
   return cfg_.scan->GetScanReceptionStartElement();
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int CVirtualChannel::GET_PA_EmissionStartElement (void)
{
   if (cfg_.scan == NULL) return ELOG(charEVCH_SCAN_NULL,EVCH_SCAN_NULL);
   return cfg_.scan->GetScanEmissionStartElement();
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int CVirtualChannel::GET_PA_LinearEndElement (void)
{
   if (cfg_.scan == NULL) return ELOG(charEVCH_SCAN_NULL,EVCH_SCAN_NULL);
   return cfg_.scan->GetLinearScanEndElement();
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
float CVirtualChannel::GET_PA_LinearAngle (void)
{
   if (cfg_.scan == NULL) return (float)ELOG(charEVCH_SCAN_NULL,EVCH_SCAN_NULL);
   return cfg_.scan->GetLinearScanAngle();
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
float CVirtualChannel::GET_PA_AngularStartAngle (void)
{
   if (cfg_.scan == NULL) return (float)ELOG(charEVCH_SCAN_NULL,EVCH_SCAN_NULL);
   return cfg_.scan->GetAngularScanStartAngle();
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
float CVirtualChannel::GET_PA_AngularEndAngle (void)
{
   if (cfg_.scan == NULL) return (float)ELOG(charEVCH_SCAN_NULL,EVCH_SCAN_NULL);
   return cfg_.scan->GetAngularScanEndAngle();
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
float CVirtualChannel::GET_PA_AngularStep (void)
{
   if (cfg_.scan == NULL) return (float)ELOG(charEVCH_SCAN_NULL,EVCH_SCAN_NULL);
   return cfg_.scan->GetAngularScanStepAngle();
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int CVirtualChannel::GET_PA_FocusingEnabled (void)
{
   if (cfg_.scan == NULL) return (float)ELOG(charEVCH_SCAN_NULL,EVCH_SCAN_NULL);
   return cfg_.scan->GetFocusEnable();
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
float CVirtualChannel::GET_PA_EmissionFocalPoint (void)
{
   if (cfg_.scan == NULL) return (float)ELOG(charEVCH_SCAN_NULL,EVCH_SCAN_NULL);
   return cfg_.scan->GetEmissionFocus();
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
float CVirtualChannel::GET_PA_ReceptionFocalPoint (void)
{
   if (cfg_.scan == NULL) return (float)ELOG(charEVCH_SCAN_NULL,EVCH_SCAN_NULL);
   return cfg_.scan->GetReceptionFocus();
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
//int CVirtualChannel::GET_PA_ReceptionAutoFocusEnabled (void)
//{
//   if (cfg_.scan == NULL) return (float)ELOG(charEVCH_SCAN_NULL,EVCH_SCAN_NULL);
//   return cfg_.scan->GetDynamicFocusingEnable();
//}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
//int CVirtualChannel::GET_PA_EmissionAutoFocusEnabled (void)
//{
//   if (cfg_.scan == NULL) return (float)ELOG(charEVCH_SCAN_NULL,EVCH_SCAN_NULL);
//   return 0;
//}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int CVirtualChannel::GET_ImageEntryPoint_X (float *x_coordinate, int size_data)
{
    int i, n_ch, result = EVCH_NONE;

    switch (cfg_.acquisition_type)
    {
        case ACQUISITION_TYPE_MC:
            if ((n_ch = GET_ChannelsNumber()) < 0) result = n_ch;
            else
            {
                for (i = 0; i < size_data && i < n_ch; i++)
                    x_coordinate[i] = i * cfg_.pa_probe->GetPitch();
            }
            break;
        case ACQUISITION_TYPE_PA:
            if (cfg_.scan == NULL) return ELOG(charEVCH_SCAN_NULL, EVCH_SCAN_NULL);
            return cfg_.scan->GetReceptionEntryPoint_X(x_coordinate, size_data);
            break;
        case ACQUISITION_TYPE_TFM:
            break;
        default:
            result = ELOG(charEVCH_DataInconsistency, EVCH_DataInconsistency);
            break;
    }
    return result;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int CVirtualChannel::GET_ImageEntryPoint_Z (float *z_coordinate, int size_data)
{
    int i, n_ch, result = EVCH_NONE;

    switch (cfg_.acquisition_type)
    {
    case ACQUISITION_TYPE_MC:
        if ((n_ch = GET_ChannelsNumber()) < 0) result = n_ch;
        else
        {
            for (i = 0; i < size_data && i < n_ch; i++) z_coordinate[i] = 0;
        }
        break;
    case ACQUISITION_TYPE_PA:
        if (cfg_.scan == NULL) return ELOG(charEVCH_SCAN_NULL, EVCH_SCAN_NULL);
        return cfg_.scan->GetReceptionEntryPoint_Z(z_coordinate, size_data);
        break;
    case ACQUISITION_TYPE_TFM:
        break;
    default:
        result = ELOG(charEVCH_DataInconsistency, EVCH_DataInconsistency);
        break;
    }
    return result;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int CVirtualChannel::GET_ImageEntryPoint_Angle (float *angle, int size_data)
{
    int i, n_ch, result = EVCH_NONE;

    switch (cfg_.acquisition_type)
    {
    case ACQUISITION_TYPE_MC:
        if ((n_ch = GET_ChannelsNumber()) < 0) result = n_ch;
        else
        {
            for (i = 0; i < size_data && i < n_ch; i++) angle[i] = 0;
        }
        break;
    case ACQUISITION_TYPE_PA:
        if (cfg_.scan == NULL) return ELOG(charEVCH_SCAN_NULL, EVCH_SCAN_NULL);
        return cfg_.scan->GetReceptionEntryPoint_Angle(angle, size_data);
        break;
    case ACQUISITION_TYPE_TFM:
        break;
    default:
        result = ELOG(charEVCH_DataInconsistency, EVCH_DataInconsistency);
        break;
    }
    return result;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int CVirtualChannel::GET_PA_ReceptionFocalLawNumber(void)
{
   if (cfg_.scan == NULL) return ELOG(charEVCH_SCAN_NULL, EVCH_SCAN_NULL);
   return cfg_.scan->GetReceptionFocalLawNumber();
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int CVirtualChannel::GET_PA_EmissionFocalLawNumber (void)
{
   if (cfg_.scan == NULL) return ELOG(charEVCH_SCAN_NULL, EVCH_SCAN_NULL);
   return cfg_.scan->GetEmissionFocalLawNumber();
}
//------------------------------------------------------------------------------
/**

*/
//------------------------------------------------------------------------------
int CVirtualChannel::LOAD_PA_FocalPoint(float emission_focus, float reception_focus)
{
   int result;

   if (cfg_.scan == NULL) return ELOG(charEVCH_SCAN_NULL, EVCH_SCAN_NULL);
   if (cfg_.scan->GetEmissionFocus() != emission_focus || cfg_.scan->GetReceptionFocus() != reception_focus)
   {
      if ((result = cfg_.scan->ConfigFocus(emission_focus, reception_focus)) < 0)
         return RLOG(result);
      update_focal_law_fp_ = true;
      update_focal_law_pa_ = true;
   }

   if ((result = RELOAD_FocalLaw()) < 0) return RLOG(result);
   if ((result = LOAD_PA_FocalLaw()) < 0) return RLOG(result);
   return EVCH_NONE;
}
//------------------------------------------------------------------------------
/**

*/
//------------------------------------------------------------------------------
int CVirtualChannel::LOAD_PA_EmissionFocalPoint(float emission_focus)
{
    int result;

    if (cfg_.scan == NULL) return ELOG(charEVCH_SCAN_NULL, EVCH_SCAN_NULL);
    if (cfg_.scan->GetEmissionFocus() != emission_focus)
    {
        if ((result = cfg_.scan->ConfigFocus(emission_focus, cfg_.scan->GetReceptionFocus())) < 0)
            return RLOG(result);
        update_focal_law_fp_ = true;
        update_focal_law_pa_ = true;
    }

    if ((result = RELOAD_FocalLaw()) < 0) return RLOG(result);
    if ((result = LOAD_PA_FocalLaw()) < 0) return RLOG(result);
    return EVCH_NONE;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int CVirtualChannel::SET_PA_FocalPoint(float emission_focus, float reception_focus)
{
   int result;

   if ((result = LOAD_PA_FocalPoint(emission_focus, reception_focus)) < 0)
      return RLOG(result);
   if ((result = Set()) < 0) return RLOG(result);
   return EVCH_NONE;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int CVirtualChannel::SET_PA_EmissionFocalPoint(float emission_focus)
{
    int result;

    if ((result = LOAD_PA_EmissionFocalPoint(emission_focus)) < 0)
        return RLOG(result);
    if ((result = Set()) < 0) return RLOG(result);
    return EVCH_NONE;
}







