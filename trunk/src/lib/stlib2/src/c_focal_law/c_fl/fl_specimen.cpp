// -----------------------------------------------------------------------------
/**
 @file fl_specimen.cpp

 @brief Este archivo contiene la implementación de las funciones de la clase \c CFL_Specimen.<br>

 Esta clase tienen la funcionalidad necesaria para definir los parámetros que definen
 el material por el que se propaga el sonido.

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
// ------ L I B R E R I A S   P R O P I A S   D E L   P R O Y E C T O
// =============================================================================

#include "fl_specimen.h"
#include "fl_error_code.h"
#include "c_log_file.h"

using namespace std;

// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
CFL_Specimen::CFL_Specimen (float velocity)
{
	if (velocity <= 0) velocity = 1;
	velocity_ = velocity;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int CFL_Specimen::SetVelocity (float data)
{
	if (data <= 0) return ELOG(charEFL_PARAMETER,EFL_PARAMETER);
	velocity_ = data;
   return EFL_NONE;
}

