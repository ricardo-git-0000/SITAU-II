// _____________________________________________________________________________
// ------ L I B R E R I A S   P R O P I A S   D E L   C O M P I L A D O R
// =============================================================================

#if defined(_MSC_VER)
   #include <string>  
#endif

// _____________________________________________________________________________
// ------ L I B R E R I A S   P R O P I A S   D E L   P R O Y E C T O
// =============================================================================

#include "c_pa_system_config.h"
#include "c_pa_system.h"
#include "fileformat.h"
#include "st_error_code.h"
#include "c_log_file.h"
#include "st_reg.h"

// _____________________________________________________________________________
// ------ V A R I A B L E S   G L O B A L E S
// =============================================================================

// _____________________________________________________________________________
// ------ CPASystem_Config class Methods
// _____________________________________________________________________________
// =============================================================================
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
CPASystem_Config::CPASystem_Config (T_PASystemConfig *sys)
{
   sys_ = sys;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
CPASystem_Config::~CPASystem_Config()
{
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int CPASystem_Config::DecoFileVersion ( AnsiString version_file,
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
   return ES2_NONE;
}
// ----------------------------------------------------------------------------
/**
	ST_LoadConfig
	==============
Descripcion:
	Carga la configuracion desde un archivo. El archivo debe haber sido creado
	previamente con la funcion 'USC_SaveConfig'.

Parametros:
	(*file): Nombre del archivo.
	(*date): Devuelve la fecha y hora en la que la configuracion fue salvada.
				El formato es una cadena de caracteres (dd/mm/YYYY HH:MM:SS).
				El tamaño de esta cadena debe ser de 20 caracteres.

Devuelve:
	- La última posición leida en el archivo.
	- (Ver codigos de error en 'return.h')

*/
// -----------------------------------------------------------------------------
long CPASystem_Config::LoadConfigData (fstream& f)
{
//int i;
//long file_pos = 0;
//int result;

//	if (f == NULL) return ELOG(charES2_FILE,ES2_FILE);
//
//// _____________________________________________________________________________
//// ----- System Config
//
//   // Id. BASE
//   if ((result = ReadField (f, &load_sys_.id_base)) < 0) return ELOG(charES2_FILE,ES2_FILE);
//
//   // Hardware Address BASE
//   if ((result = ReadField (f, &load_sys_.address)) < 0) return ELOG(charES2_FILE,ES2_FILE);
//
//   // Multiplexed System
//   if ((result = ReadField (f, &load_sys_.multiplexed)) < 0) return ELOG(charES2_FILE,ES2_FILE);
//
//// _____________________________________________________________________________
//// ----- Hardware BASE Config
//
//	// WNREP Register
//   if ((result = ReadField (f, &load_reg_->WNREP)) < 0) return ELOG(charES2_FILE,ES2_FILE);
//
//	// WDIRCAT Register
//   if ((result = ReadField (f, &load_reg_->WDIRCAT)) < 0) return ELOG(charES2_FILE,ES2_FILE);
//
//	// DLRPOS Register
//   if ((result = ReadField (f, &load_reg_->DLRPOS)) < 0) return ELOG(charES2_FILE,ES2_FILE);
//
//	// WFPB Register
//   if ((result = ReadField (f, &load_reg_->WFPB)) < 0) return ELOG(charES2_FILE,ES2_FILE);
//
//	// WCPB Register
//   if ((result = ReadField (f, (unsigned short *)&load_reg_->WCPB, BASE_NUM_CPB)) < 0) return ELOG(charES2_FILE,ES2_FILE);
//
//	// WMDIEZ Register
//   if ((result = ReadField (f, &load_reg_->WMDIEZ.REG)) < 0) return ELOG(charES2_FILE,ES2_FILE);
//
//	// WMREDU Register
//   if ((result = ReadField (f, &load_reg_->WMREDU.REG)) < 0) return ELOG(charES2_FILE,ES2_FILE);
//
//	// WMUXEI Register
//   if ((result = ReadField (f, &load_reg_->WMUXEI.REG)) < 0) return ELOG(charES2_FILE,ES2_FILE);
//
//	// RCO Register
//   if ((result = ReadField (f, &load_reg_->RCO.REG)) < 0) return ELOG(charES2_FILE,ES2_FILE);
//
//	// WCKCAT Register
//   if ((result = ReadField (f, &load_reg_->WCKCAT.REG)) < 0) return ELOG(charES2_FILE,ES2_FILE);
//
//	// WTCAT Register
//   if ((result = ReadField (f, &load_reg_->WTCAT)) < 0) return ELOG(charES2_FILE,ES2_FILE);
//
//	// WNLIN Register
//   if ((result = ReadField (f, &load_reg_->WNLIN.REG)) < 0) return ELOG(charES2_FILE,ES2_FILE);
//
//	// Hardware Gates Config
//   for (i=0; i<FP_BASE_NUM_GATES; i++)
//   {
//   	// GATE.WINI Register
//      if ((result = ReadField (f, &load_reg_->GATE[i].WINI)) < 0) return ELOG(charES2_FILE,ES2_FILE);
//
//   	// GATE.WFIN Register
//      if ((result = ReadField (f, &load_reg_->GATE[i].WFIN)) < 0) return ELOG(charES2_FILE,ES2_FILE);
//
//   	// GATE.WUW Register
//      if ((result = ReadField (f, &load_reg_->GATE[i].WUW)) < 0) return ELOG(charES2_FILE,ES2_FILE);
//
//   	// GATE.WCTRL Register
//      if ((result = ReadField (f, &load_reg_->GATE[i].WCTRL.REG)) < 0) return ELOG(charES2_FILE,ES2_FILE);
//   }
	//fgetpos(f,(fpos_t*)&file_pos);
    return 0;// (long)f.tellg();
}
/* ----------------------------------------------------------------------
	ST_LoadConfig
	==============
Descripcion:
	Carga la configuracion desde un archivo. El archivo debe haber sido creado
	previamente con la funcion 'USC_SaveConfig'.

Parametros:
	(*file): Nombre del archivo.
	(*date): Devuelve la fecha y hora en la que la configuracion fue salvada.
				El formato es una cadena de caracteres (dd/mm/YYYY HH:MM:SS).
				El tamaño de esta cadena debe ser de 20 caracteres.

Devuelve:
	- La última posición leida en el archivo.
	- (Ver codigos de error en 'return.h')
	---------------------------------------------------------------------- */
long CPASystem_Config::LoadFileConfig (fstream& f)
{
int result;
unsigned short field_1, field_2, field_3, field_4;
AnsiString version_file;
long file_position = 0;

   sys_update_ = false;
//   ResetConfigData();

// _____________________________________________________________________________
// ----- File Version

   if ((result = ReadField (f, &version_file)) < 0) return ELOG(charES2_FILE,ES2_FILE);
   if ((result = DecoFileVersion (version_file, &field_1, &field_2, &field_3, &field_4)) < 0)
      return RLOG(result);
   if (field_1 == 0 && field_2 == 0 && field_3 == 0 && field_4 == 1)
	{
      if ((file_position = LoadConfigData(f)) < 0) return RLOG(file_position);
	}
	else return ES2_FILE;

   sys_update_ = true;

//   if ((result = SetFileConfig()) < 0) return RLOG(result);
	return file_position;
}
/* ----------------------------------------------------------------------
   USC_SaveConfig
	==============
Descripcion:
	Salva la configuracion actual en una archivo.

Parametros:
	(*file): Nombre del archivo

Devuelve: (Ver codigos de error en 'return.h')
	---------------------------------------------------------------------- */
long CPASystem_Config::SaveFileConfig (fstream& f)
{
long file_position = 0;

//	if (f == NULL) return ELOG(charES2_FILE,ES2_FILE);
//   if (sys_ == NULL || reg_ == NULL) return ELOG(charEBASE_PtrAccess_NULL, EBASE_PtrAccess_NULL);
//
//// _____________________________________________________________________________
//// ----- File Version
//
//   if ((result = WriteField (f, AnsiString(BASECFG_VERSION))) < 0) return ELOG(charES2_FILE,ES2_FILE);
//
//// _____________________________________________________________________________
//// ----- System Config
//
//   // Id. BASE
//   if ((result = WriteField (f, sys_->id_base)) < 0) return ELOG(charES2_FILE,ES2_FILE);
//
//   // Hardware Address BASE
//   if ((result = WriteField (f, sys_->address)) < 0) return ELOG(charES2_FILE,ES2_FILE);
//
//   // Multiplexed System
//   if ((result = WriteField (f, sys_->multiplexed)) < 0) return ELOG(charES2_FILE,ES2_FILE);
//
//// _____________________________________________________________________________
//// ----- Hardware BASE Config
//
////	vector<regGAN> GAN;            		 // dir 61 (Write)
////   vector<T_BASE_FocalLaw> WMPAR;
//
//
//	// WNREP Register
//   if ((result = WriteField (f, reg_->WNREP)) < 0) return ELOG(charES2_FILE,ES2_FILE);
//
//	// WDIRCAT Register
//   if ((result = WriteField (f, reg_->WDIRCAT)) < 0) return ELOG(charES2_FILE,ES2_FILE);
//
//	// DLRPOS Register
//   if ((result = WriteField (f, reg_->DLRPOS)) < 0) return ELOG(charES2_FILE,ES2_FILE);
//
//	// WFPB Register
//   if ((result = WriteField (f, reg_->WFPB)) < 0) return ELOG(charES2_FILE,ES2_FILE);
//
//	// WCPB Register
//   if ((result = WriteField (f, reg_->WCPB, BASE_NUM_CPB)) < 0) return ELOG(charES2_FILE,ES2_FILE);
//
//	// WMDIEZ Register
//   if ((result = WriteField (f, reg_->WMDIEZ.REG)) < 0) return ELOG(charES2_FILE,ES2_FILE);
//
//	// WMREDU Register
//   if ((result = WriteField (f, reg_->WMREDU.REG)) < 0) return ELOG(charES2_FILE,ES2_FILE);
//
//	// WMUXEI Register
//   if ((result = WriteField (f, reg_->WMUXEI.REG)) < 0) return ELOG(charES2_FILE,ES2_FILE);
//
//	// RCO Register
//   if ((result = WriteField (f, reg_->RCO.REG)) < 0) return ELOG(charES2_FILE,ES2_FILE);
//
//	// WCKCAT Register
//   if ((result = WriteField (f, reg_->WCKCAT.REG)) < 0) return ELOG(charES2_FILE,ES2_FILE);
//
//	// WTCAT Register
//   if ((result = WriteField (f, reg_->WTCAT)) < 0) return ELOG(charES2_FILE,ES2_FILE);
//
//	// WNLIN Register
//   if ((result = WriteField (f, reg_->WNLIN.REG)) < 0) return ELOG(charES2_FILE,ES2_FILE);
//
//	// Hardware Gates Config
//   for (i=0; i<BASE_NUM_GATES; i++)
//   {
//   	// GATE.WINI Register
//      if ((result = WriteField (f, reg_->GATE[i].WINI)) < 0) return ELOG(charES2_FILE,ES2_FILE);
//
//   	// GATE.WFIN Register
//      if ((result = WriteField (f, reg_->GATE[i].WFIN)) < 0) return ELOG(charES2_FILE,ES2_FILE);
//
//   	// GATE.WUW Register
//      if ((result = WriteField (f, reg_->GATE[i].WUW)) < 0) return ELOG(charES2_FILE,ES2_FILE);
//
//   	// GATE.WCTRL Register
//      if ((result = WriteField (f, reg_->GATE[i].WCTRL.REG)) < 0) return ELOG(charES2_FILE,ES2_FILE);
//   }
//
//	// Gain-TGC Config
//   for (i=0; i<BASE_NUM_GATES; i++)
//   {
//   	// GATE.WINI Register
//      if ((result = WriteField (f, reg_->GATE[i].WINI)) < 0) return ELOG(charES2_FILE,ES2_FILE);
//
//   	// GATE.WFIN Register
//      if ((result = WriteField (f, reg_->GATE[i].WFIN)) < 0) return ELOG(charES2_FILE,ES2_FILE);
//
//   	// GATE.WUW Register
//      if ((result = WriteField (f, reg_->GATE[i].WUW)) < 0) return ELOG(charES2_FILE,ES2_FILE);
//
//   	// GATE.WCTRL Register
//      if ((result = WriteField (f, reg_->GATE[i].WCTRL.REG)) < 0) return ELOG(charES2_FILE,ES2_FILE);
//   }
//	fgetpos(f,(fpos_t*)&file_position);
	return file_position;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
void CPASystem_Config::PrintFileConfig(FILE *f)
{
   if (f != NULL && sys_ != NULL)
   {
      fprintf (f, "\n--> PA System %d (%s) - %d BASES [%d..%d] - %d MODULES [%d..%d]",
         sys_->id,
         sys_->name.c_str(),
         sys_->n_bases,
         sys_->id_base_first,
         sys_->id_base_last,
         sys_->n_modules,
         sys_->id_mod_first,
         sys_->id_mod_last);
      if (sys_->enabled) fprintf(f, " (ENABLED)");
      else fprintf(f, " (DISABLED)");
           fprintf(f, "\n-----------------------------------------------------------------------------");

      if (sys_->multiplexed)
         fprintf (f, "\n   Multiplexed %d:%d (%d Channels)",
            sys_->n_active_channels,
            sys_->n_multiplexed_channels,
            sys_->n_total_channels);
      else
         fprintf (f, "\n   Parallel %d:%d (%d Channels)",
            sys_->n_active_channels,
            sys_->n_active_channels,
            sys_->n_total_channels);
   }
}
