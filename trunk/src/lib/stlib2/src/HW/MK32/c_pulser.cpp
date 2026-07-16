// ______________________________________________________________________
// ------ L I B R E R I A S   P R O P I A S   D E L   P R O Y E C T O ---
// ======================================================================

#include "c_pulser.h"
#include "base_error_code.h"
#include "c_log_file.h"
#include "utils.h"
#include "st_reg.h"

// ______________________________________________________________________
// ------ L I B R E R I A S   P R O P I A S   D E L   C O M P I L A D O R
// ======================================================================

// _____________________________________________________________________________
// ------ V A R I A B L E S   G L O B A L E S
// =============================================================================

// _____________________________________________________________________________
// ------ C_PULSER class Methods
// _____________________________________________________________________________
// =============================================================================
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
C_PULSER::C_PULSER()
{
   update_register_ = false;
   ResetRegister();
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
C_PULSER::~C_PULSER()
{
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int C_PULSER::ResetRegister(void)
{
   reg_.enabled = 1;
   reg_.high_z = 0;
   reg_.current = 0;
   reg_.n_pulses = 1;
   reg_.pulse_width = 20;
   reg_.pulse_width_ns = reg_.pulse_width * (float)PCLK4_ns + 4;
   reg_.main_delay = 0;
   return EBASE_NONE;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int C_PULSER::CONFIG_Copy(C_PULSER* src)
{
   if (src == NULL) return ELOG(charEBASE_PtrAccess_NULL, EBASE_PtrAccess_NULL);
   reg_.enabled = src->reg_.enabled;
   reg_.high_z = src->reg_.high_z;
   reg_.current = src->reg_.current;
   reg_.n_pulses = src->reg_.n_pulses;
   reg_.pulse_width = src->reg_.pulse_width;
   reg_.pulse_width_ns = src->reg_.pulse_width_ns;// *(float)PCLK4_ns;;
   reg_.main_delay = src->reg_.main_delay;
   return EBASE_NONE;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
void C_PULSER::PrintFileConfig(FILE* f)
{
   if (f == NULL) return;

   fprintf(f, "\n    __________________");
   fprintf(f, "\n    PULSER Registers.-\n");

   fprintf(f, "\n        PULSER.pulse_width = %d (%.1f ns)", reg_.pulse_width, reg_.pulse_width_ns);
   fprintf(f, "\n        PULSER.pulser_main_delay = %d", reg_.main_delay);

   fprintf(f, "\n        PULSER.CONTROL = (%d) enabled", reg_.enabled);
   fprintf(f, "\n                         (%d) high_z", reg_.high_z);
   fprintf(f, "\n                         (%d) mode", reg_.current);
   fprintf(f, "\n                         (%d) n_pulses", reg_.n_pulses);
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int C_PULSER::LoadRegisterPULSER_N_PULSES (unsigned char data)
{
   update_register_ = update_register_ ? true : (data != reg_.n_pulses);
   reg_.n_pulses = data;
	return EBASE_NONE;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int C_PULSER::LoadRegisterPULSER_HIGH_Z (unsigned char data)
{
   data = (data == 1) ? 1 : 0;
   update_register_ = update_register_ ? true : (data != reg_.high_z);
   reg_.high_z = data;
	return EBASE_NONE;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int C_PULSER::LoadRegisterPULSER_CURRENT (unsigned char data)
{
   data = (data > BASE_MAX_PULSE_MODE) ? BASE_MAX_PULSE_MODE : data;
   update_register_ = update_register_ ? true : (data != reg_.current);
   reg_.current = data;
	return EBASE_NONE;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int C_PULSER::LoadRegisterPULSER_ENABLED (unsigned char data)
{
   data = (data > BASE_MAX_PULSE_MODE) ? BASE_MAX_PULSE_MODE : data;
   update_register_ = update_register_ ? true : (data != reg_.enabled);
   reg_.enabled = data;
	return EBASE_NONE;
}  
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int C_PULSER::LoadRegisterPULSER_WIDTH (unsigned long data)
{
   update_register_ = update_register_ ? true : (data != reg_.pulse_width);
   reg_.pulse_width = data;
   reg_.pulse_width_ns = data * (float)PCLK4_ns + 4;
	return EBASE_NONE;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int C_PULSER::LoadValuePULSER_WIDTH (float data)
{
unsigned long reg_value = Round32u((data - 4)/ (float)PCLK4_ns); 

   update_register_ = update_register_ ? true : (reg_value != reg_.pulse_width);
   reg_.pulse_width = reg_value;
   reg_.pulse_width_ns = data;
	return EBASE_NONE;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int C_PULSER::LoadRegisterPULSER_MAIN_DELAY (unsigned long data)
{
   update_register_ = update_register_ ? true : (data != reg_.main_delay);
   reg_.main_delay = data;
	return EBASE_NONE;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int C_PULSER::GetRegisterPULSER_N_PULSES(void)
{
   return reg_.n_pulses;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int C_PULSER::GetRegisterPULSER_HIGH_Z(void)
{
   return reg_.high_z;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int C_PULSER::GetRegisterPULSER_CURRENT(void)
{
   return reg_.current;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int C_PULSER::GetRegisterPULSER_ENABLED(void)
{
   return reg_.enabled;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int C_PULSER::GetRegisterPULSER_WIDTH(void)
{
   return reg_.pulse_width;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int C_PULSER::GetRegisterPULSER_MAIN_DELAY(void)
{
   return reg_.main_delay;
}
