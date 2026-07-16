// -----------------------------------------------------------------------------
/**
 @file fl_pa.cpp

 @brief Este archivo contiene la implementación de las funciones de la clase \c CFL_PA.<br>

 Esta clase tienen la funcionalidad necesaria para definir los transductores PA utilizados
 en los diferebnte barridos.

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

#include "fl_pa.h"
#include "fl_error_code.h"
#include "c_log_file.h"

using namespace std;

// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
CFL_PA::CFL_PA (int n_elements, float frequency)
{
   if (  n_elements <= 1 ||
         frequency < 0.05)
   	throw ELOG(charEFL_PARAMETER,EFL_PARAMETER);
   n_elements_ = n_elements;
	frequency_ = frequency;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
void CFL_PA::SetElementNumber (int n_element)
{
	n_elements_ = n_element;
}

