// -----------------------------------------------------------------------------
/**
 @file shared_mem.c

 @brief Este archivo contiene la implementaciÃ³n de las funciones de configuraciÃ³n
 y acceso de los encoders
 
 @author (rg) Ricardo GonzÃ¡lez

 <pre>
 MODIFICATION HISTORY:

 Ver   Who  Date     Changes
 ----- ---- -------- -----------------------------------------------------------
 1.00a (rg) 13/03/2024 First release
 </pre>
*/
// -----------------------------------------------------------------------------
// _____________________________________________________________________________
// ------ L I B R E R I A S   P R O P I A S   D E L   C O M P I L A D O R
// =============================================================================

// _____________________________________________________________________________
// ------ L I B R E R I A S   P R O P I A S   D E L   P R O Y E C T O
// =============================================================================

#include "shared_mem.h"
#include "shared_mem_type.h"

TSHRD_0x00000002 gb_shared;
u8 gb_shared_mem[MEM_FRAME_SIZE];
char* gb_file_log_sitau2 = "LOG_SITAU2.TXT";

// ----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
void SHRD_Update(u8 *frame_shared_mem)
{
    TSHRD_0x00000002 check;
    u32 version;
	u8* ptr = frame_shared_mem + 16;

    version = *((u32*)ptr);
    ptr += 4;
    check = *(TSHRD_0x00000002*)(ptr);
    if (check.check_word == 0x1234 && check.check_byte == 0x5A)
        gb_shared = *(TSHRD_0x00000002*)(ptr);
    else
        int kk = 1;
    gb_shared.sw_version_cpu0.date[SHRD_VERSION_DATE_SIZE-1] = 0;
    gb_shared.sw_version_cpu0.description[SHRD_VERSION_DESCRIPTION_SIZE-1] = 0;
    gb_shared.sw_version_cpu1.date[SHRD_VERSION_DATE_SIZE - 1] = 0;
    gb_shared.sw_version_cpu1.description[SHRD_VERSION_DESCRIPTION_SIZE - 1] = 0;
    for (int i = 0; i < gb_shared.log.e->count && i < SHRD_LOG_ERROR_SIZE; i++)
    {
        gb_shared.log.e[i].msg[SHRD_LOG_ERROR_MSG_SIZE-1] = 0;
        gb_shared.log.e[i].file[SHRD_LOG_ERROR_FILE_SIZE-1] = 0;
    }
    SHRD_LogError();
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
void SHRD_LogError(void)
{
    int i, j;
    //char* file;
    FILE* f = NULL;

    if (f != nullptr || (f = fopen(gb_file_log_sitau2, "at")) != nullptr)
    {
        for (i = 0; i < gb_shared.log.count && i < SHRD_LOG_ERROR_SIZE; i++)
        {
            fprintf(f, "\n\n---> [%d]: %s", i, gb_shared.log.e[i].msg);
            fprintf(f, "\n     Error code: %d", gb_shared.log.e[i].code);
            fprintf(f, "\n     File:       %s ", gb_shared.log.e[i].file);
            fprintf(f, "\n     Line:       %d ", gb_shared.log.e[i].line);
            for (j = 0; j < gb_shared.log.e[i].count && j < SHRD_LOG_ERROR_STACK_SIZE; j++)
            {
                fprintf(f, "\n     Error code: %d", gb_shared.log.e[i].s[j].code);
                fprintf(f, "\n     File:       %s ", gb_shared.log.e[i].s[j].file);
                fprintf(f, "\n     Line:       %d ", gb_shared.log.e[i].s[j].line);
            }
        }
        fflush(f);
        fclose(f);
        f = nullptr;
    }
}
