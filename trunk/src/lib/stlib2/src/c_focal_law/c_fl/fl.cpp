// -----------------------------------------------------------------------------
/**
 @file fl.cpp

 @brief Este archivo contiene la implementación de las funciones relacionadas
 con la información de la librería.
 
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

#include <string>

// _____________________________________________________________________________
// ------ L I B R E R I A S   P R O P I A S   D E L   P R O Y E C T O
// =============================================================================

#include "fl.h"
#include "fl_error_code.h"

// -----------------------------------------------------------------------------
/**
 Devuelve la versión de la librería.

 @param *data Puntero a la cadena de caracteres donde se devuelve la versión de
              la librería.<br>
              El número de caracteres de la cadena debe ser 50.

 @return	Ver códigos de error.
*/
// -----------------------------------------------------------------------------
int FL_InfoVersion (char *data)
{
   strcpy(data,_FL_VERSION_MAJOR);
   strcat(data,".");
	strcat(data,_FL_VERSION_MINOR);
   strcat(data,".");
   strcat(data,_FL_VERSION_RELEASE);
   strcat(data,".");
   strcat(data,_FL_VERSION_BUILD);
   strcat(data," ");
   strcat(data,_FL_VERSION_DATE);
   return EFL_NONE;
}

