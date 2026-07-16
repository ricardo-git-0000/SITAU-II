// -----------------------------------------------------------------------------
/**
 @file c_virtual_channel.cpp

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

#include "c_vch_tgc.h"
#include "c_virtual_channel.h"
#include "vch_error_code.h"
#include "c_log_file.h"
#include "stlib_dev.h"
#include "stlib2.h"

// _____________________________________________________________________________
// ------ V A R I A B L E S   G L O B A L E S
// =============================================================================

// _____________________________________________________________________________
// ------ CVirtualChannel class Methods
// _____________________________________________________________________________
// =============================================================================
//------------------------------------------------------------------------------
/**

*/
//------------------------------------------------------------------------------
int CVirtualChannel::LOAD_DigitalTGC_Attenuator(int data)
{
int result;

   if (sys_.afe == NULL) return ELOG(charEVCH_BASE_NULL, EVCH_BASE_NULL);
   if ((result = sys_.afe->LoadRegisterAFE_DIG_TGC_ATT_GAIN(data)) < 0) return RLOG(result);
   if ((result = sys_.afe->LoadRegisterAFE_DIG_TGC_ATT(1)) < 0) return RLOG(result);
   update_register_ = update_register_ ? true : sys_.afe->UpdateRegister();
   cfg_.digital_tgc_attenuator = data;
   return EVCH_NONE;
}
//------------------------------------------------------------------------------
/**

*/
//------------------------------------------------------------------------------
int CVirtualChannel::LOAD_LOG2_AVERAGE(int data)
{
int result;

   if (sys_.afe == NULL) return ELOG(charEVCH_BASE_NULL, EVCH_BASE_NULL);
   if ((result = sys_.afe->LoadRegisterBUSSAR_LOG2_AVERAGE(data)) < 0) return RLOG(result);
   update_register_ = update_register_ ? true : sys_.afe->UpdateRegister();
   return EVCH_NONE;
}
//------------------------------------------------------------------------------
/**

*/
//------------------------------------------------------------------------------
int CVirtualChannel::LOAD_LNA_GAIN(int data)
{
int result;

   if (sys_.afe == NULL) return ELOG(charEVCH_BASE_NULL, EVCH_BASE_NULL);
   if ((result = sys_.afe->LoadRegisterAFE_LNA_GAIN(data)) < 0) return RLOG(result);
   update_register_ = update_register_ ? true : sys_.afe->UpdateRegister();
   return EVCH_NONE;
}
//------------------------------------------------------------------------------
/**

*/
//------------------------------------------------------------------------------
int CVirtualChannel::LOAD_LNA_GAIN(float data)
{
	int result, reg;
	if (data < 18) reg = 0;
	if (sys_.afe == NULL) return ELOG(charEVCH_BASE_NULL, EVCH_BASE_NULL);
	if ((result = sys_.afe->LoadRegisterAFE_LNA_GAIN(data)) < 0) return RLOG(result);
	update_register_ = update_register_ ? true : sys_.afe->UpdateRegister();
	return EVCH_NONE;
}
//------------------------------------------------------------------------------
/**

*/
//------------------------------------------------------------------------------
int CVirtualChannel::LOAD_PGA_GAIN(int data)
{
int result;

   if (sys_.afe == NULL) return ELOG(charEVCH_BASE_NULL, EVCH_BASE_NULL);
   if ((result = sys_.afe->LoadRegisterAFE_PGA_GAIN_CONTROL(data)) < 0) return RLOG(result);
   update_register_ = update_register_ ? true : sys_.afe->UpdateRegister();
   return EVCH_NONE;
}
//------------------------------------------------------------------------------
/**

*/
//------------------------------------------------------------------------------
int CVirtualChannel::GET_DigitalTGC_Attenuator(void)
{
   int result;

   if (sys_.afe == NULL) return ELOG(charEVCH_BASE_NULL, EVCH_BASE_NULL);
   result = sys_.afe->GetRegisterAFE_DIG_TGC_ATT_GAIN();
   return result;
}
//------------------------------------------------------------------------------
/**

*/
//------------------------------------------------------------------------------
int CVirtualChannel::GET_LOG2_AVERAGE(void)
{
   int result;

   if (sys_.afe == NULL) return ELOG(charEVCH_BASE_NULL, EVCH_BASE_NULL);
   result = sys_.afe->GetRegisterBUSSAR_LOG2_AVERAGE();
   return result;
}
//------------------------------------------------------------------------------
/**

*/
//------------------------------------------------------------------------------
int CVirtualChannel::GET_LNA_GAIN(void)
{
   int result;

   if (sys_.afe == NULL) return ELOG(charEVCH_BASE_NULL, EVCH_BASE_NULL);
   result = sys_.afe->GetRegisterAFE_LNA_GAIN();
   return result;
}
//------------------------------------------------------------------------------
/**

*/
//------------------------------------------------------------------------------
int CVirtualChannel::GET_PGA_GAIN(void)
{
   int result;

   if (sys_.afe == NULL) return ELOG(charEVCH_BASE_NULL, EVCH_BASE_NULL);
   result = sys_.afe->GetRegisterAFE_PGA_GAIN_CONTROL();
   return result;
}
//------------------------------------------------------------------------------
/**

*/
//------------------------------------------------------------------------------
int CVirtualChannel::LOAD_TGC_Enabled(int data)
{
int result, dig_tgc_att;
int acquiring = ST_Acquiring();

   data = (data == 0) ? 0 : 1; 
   if (sys_.base.size() == 0 || sys_.base[0] == NULL) return ELOG(charEVCH_BASE_NULL, EVCH_BASE_NULL);
   update_tgc_ = true;
   dig_tgc_att = data == 1 ? 0 : 1;
   if ((result = sys_.afe->LoadRegisterAFE_DIG_TGC_ATT(dig_tgc_att)) < 0) return RLOG(result);
   update_register_ = update_register_ ? true : sys_.afe->UpdateRegister();
   cfg_.tgc_enabled = data;
   return EVCH_NONE;
}
//------------------------------------------------------------------------------
/**

*/
//------------------------------------------------------------------------------
int CVirtualChannel::SET_DigitalTGC_Attenuator(int data)
{
	int result;// , acquiring = ST_Acquiring();

	//if (acquiring == 1 && (result = ST_Stop()) < 0) { RLOG(result); }
	if ((result = LOAD_DigitalTGC_Attenuator(data)) < 0) return RLOG(result);
	if ((result = Set()) < 0) return RLOG(result);
	//if (acquiring == 1) ST_Start();
	return EVCH_NONE;
}
//------------------------------------------------------------------------------
/**

*/
//------------------------------------------------------------------------------
int CVirtualChannel::SET_LOG2_AVERAGE(int data)
{
	int result;// , acquiring = ST_Acquiring();

	//if (acquiring == 1 && (result = ST_Stop()) < 0) { RLOG(result); }
	if ((result = LOAD_LOG2_AVERAGE(data)) < 0) return RLOG(result);
	if ((result = Set()) < 0) return RLOG(result);
	//if (acquiring == 1) ST_Start();
	return EVCH_NONE;
}
//------------------------------------------------------------------------------
/**

*/
//------------------------------------------------------------------------------
int CVirtualChannel::SET_LNA_GAIN(int data)
{
	int result;// , acquiring = ST_Acquiring();

	//if (acquiring == 1 && (result = ST_Stop()) < 0) { RLOG(result); }
	if ((result = LOAD_LNA_GAIN(data)) < 0) return RLOG(result);
	if ((result = Set()) < 0) return RLOG(result);
	//if (acquiring == 1) ST_Start();
	return EVCH_NONE;
}
//------------------------------------------------------------------------------
/**

*/
//------------------------------------------------------------------------------
int CVirtualChannel::SET_PGA_GAIN(int data)
{
	int result;// , acquiring = ST_Acquiring();

	//if (acquiring == 1 && (result = ST_Stop()) < 0) { RLOG(result); }
	if ((result = LOAD_PGA_GAIN(data)) < 0) return RLOG(result);
	if ((result = Set()) < 0) return RLOG(result);
	//if (acquiring == 1) ST_Start();
	return EVCH_NONE;
}
//------------------------------------------------------------------------------
/**

*/
//------------------------------------------------------------------------------
int CVirtualChannel::SET_TGC_Enabled(int data)
{
	int result;// , acquiring = ST_Acquiring();

	//if (acquiring == 1 && (result = ST_Stop()) < 0) { RLOG(result); }
	if ((result = LOAD_TGC_Enabled(data)) < 0) return RLOG(result);
	if ((result = Set()) < 0) return RLOG(result);
	cfg_.enabled = data;
	//if (acquiring == 1) ST_Start();
	return EVCH_NONE;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int CVirtualChannel::LOAD_TGC_AttenuationCoefficient(float attn_coefficient)
{
   cfg_.tgc_attn_coefficient = attn_coefficient;
   return EVCH_NONE;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int CVirtualChannel::SET_TGC_AttenuationCoefficient(float data)
{
   int result;
   //int acquiring = ST_Acquiring();

   //if (acquiring == 1 && (result = ST_Stop()) < 0) { RLOG(result); }
   if ((result = LOAD_TGC_AttenuationCoefficient(data)) < 0) return RLOG(result);
   if ((result = Set()) < 0) return RLOG(result);
   //if (acquiring == 1) ST_Start();
   return EVCH_NONE;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
float CVirtualChannel::GET_TGC_AttenuationCoefficient(void)
{
   return EVCH_NONE;
}
