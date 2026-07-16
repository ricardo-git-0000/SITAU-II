#ifndef shared_memH
#define shared_memH
// -----------------------------------------------------------------------------
/**
 @file shared_mem.h

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

#include "tad.h"
#include "shared_mem_type.h"


// _____________________________________________________________________________
// ------ D E F I N I C I ó N   D E   C O N S T A N T E S
// =============================================================================

//#define SHRD_INFO			0x00000001
#define SHRD_SIZE_ENCODERS	4

// CPU0	256MB
#define CPU0_C_START		0x00100000
#define CPU0_C_SIZE			0x0FF00000
// MEMORIA COMPARTIDA PARA BUFFERS	256MB
#define SHARED_MEM_START	0x10000000
#define SHARED_MEM_SIZE		0x08000000
// CPU1	128 MB
#define CPU1_C_START		0x18000000
#define CPU1_C_SIZE			0x08000000
// BUFFER DE IMAGENES	512MB
#define IMAGE_BUF_START		0x20000000
#define IMAGE_BUF_SIZE		0x20000000

#define HOST2UCI_BUF_START	(SHARED_MEM_START)
#define	HOST2UCI_BUF_SIZE	0x02000000

#define UCI2HOST_BUF_START	(HOST2UCI_BUF_START+HOST2UCI_BUF_SIZE)
#define	UCI2HOST_BUF_SIZE	0x02000000

#define LAST_SHRD_MB_SIZE	(1024*1024)
#define LAST_SHRD_MB_START	(SHARED_MEM_START+SHARED_MEM_SIZE-LAST_SHRD_MB_SIZE)

#define LAST_SHRD_KB_SIZE	(1024)
#define LAST_SHRD_KB_START	(SHARED_MEM_START+SHARED_MEM_SIZE-LAST_SHRD_KB_SIZE)

#define SHRD_MEM_INFO_BYTES		(4)
#define SHRD_MEM_HEADER			LAST_SHRD_KB_START

#define SHRD_MEM_VERSION_SIZE	(sizeof(struct version_control_s))
#define SHRD_MEM_VERSION_START	(SHRD_MEM_HEADER + SHRD_MEM_INFO_BYTES)
#define SHRD_MEM_VERSION_INFO	(SHRD_MEM_VERSION_START + 1)
#define SHRD_MEM_VERSION		(SHRD_MEM_VERSION_INFO + SHRD_MEM_INFO_BYTES)
#define SHRD_MEM_VERSION_END	(SHRD_MEM_VERSION + SHRD_MEM_VERSION_SIZE + 1)

#define SHRD_MEM_UCI_SIZE		(sizeof(TSHRD))
#define SHRD_MEM_UCI_START		(SHRD_MEM_VERSION_END + 1)
#define SHRD_MEM_UCI_INFO		(SHRD_MEM_UCI_START + 1)
#define SHRD_MEM_UCI			(SHRD_MEM_UCI_INFO + SHRD_MEM_INFO_BYTES)
#define SHRD_MEM_UCI_END		(SHRD_MEM_UCI + SHRD_MEM_UCI_SIZE + 1)

#define MEM_FRAME_SIZE			LAST_SHRD_KB_SIZE + 16

// _____________________________________________________________________________
// ------ T I P O S   D E   D A T O S
// =============================================================================

// _____________________________________________________________________________
// ------ T I P O S   D E   D A T O S
// =============================================================================

// _____________________________________________________________________________
// ------ D E F I N I C I Ó N   D E   F U N C I O N E S
// =============================================================================

void SHRD_Update(u8 *frame_shred_mem);
void SHRD_LogError(void);

// _____________________________________________________________________________
// ------ V A R I A B L E S   G L O B A L E S
// =============================================================================

extern TSHRD_0x00000002 gb_shared;
//extern T_Version gb_version_cpu0;
//extern T_Version gb_version_cpu1;
extern u8 gb_shared_mem[MEM_FRAME_SIZE];

#endif
