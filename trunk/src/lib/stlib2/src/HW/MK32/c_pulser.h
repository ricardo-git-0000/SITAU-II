#ifndef c_pulserH
#define c_pulserH

// _____________________________________________________________________________
// ------ L I B R E R I A S   P R O P I A S   D E L   C O M P I L A D O R
// =============================================================================

#include <stdio.h>

// _____________________________________________________________________________
// ------ L I B R E R I A S   P R O P I A S   D E L   P R O Y E C T O
// =============================================================================

// _____________________________________________________________________________
// ------ D E F I N I C I O N   D E   C O N S T A N T E S
// =============================================================================

#define BASE_MAX_PULSES_NUMBER   0xFF
#define BASE_MAX_PULSE_MODE      0xF

// _____________________________________________________________________________
// ------ T I P O S   D E   D A T O S
// =============================================================================

typedef struct TFP_VCH_PULSER
{
   unsigned char enabled;
   unsigned char n_pulses;
   float pulse_width_ns;
   unsigned long pulse_width;
   unsigned long main_delay;
   unsigned char high_z;
   unsigned char current;
} TFP_VCH_PULSER;

// _____________________________________________________________________________
// ------ C_PULSER class
// _____________________________________________________________________________
// =============================================================================

class C_PULSER
{
private:

   bool update_register_;
   
protected:

public:

   TFP_VCH_PULSER reg_;

   C_PULSER();

   ~C_PULSER();

   int ResetRegister(void);

   bool UpdateRegister(void) { return update_register_; }

   void PrintFileConfig(FILE* f);

   int CONFIG_Copy(C_PULSER* src);

   int LoadRegisterPULSER_N_PULSES (unsigned char data);

   int LoadRegisterPULSER_HIGH_Z (unsigned char data);

   int LoadRegisterPULSER_CURRENT(unsigned char data);

   int LoadRegisterPULSER_ENABLED (unsigned char data);

   int LoadRegisterPULSER_WIDTH (unsigned long data);

   int LoadValuePULSER_WIDTH (float data);

   int LoadRegisterPULSER_MAIN_DELAY (unsigned long data);

   int GetRegisterPULSER_N_PULSES(void);

   int GetRegisterPULSER_HIGH_Z(void);

   int GetRegisterPULSER_CURRENT(void);

   int GetRegisterPULSER_ENABLED(void);

   int GetRegisterPULSER_WIDTH(void);

   int GetRegisterPULSER_MAIN_DELAY(void);

};

// _____________________________________________________________________________
// ------ V A R I A B L E S   G L O B A L E S
// =============================================================================

// _____________________________________________________________________________
// ------ P R O T O T I P O   D E   F U N C I O N E S
// =============================================================================

#endif
