#ifndef pcieH
#define pcieH

// ______________________________________________________________________
// ------ L I B R E R I A S   P R O P I A S   D E L   C O M P I L A D O R
// ======================================================================

#include <vector>

using namespace std;

// ______________________________________________________________________
// ------ L I B R E R I A S   P R O P I A S   D E L   P R O Y E C T O
// ======================================================================

#include <fuse.h>
#include "pcie_geometry.h" // Comparte la geometria sin alterar la interfaz publica ST2.

// _____________________________________________________________________________
// ------ C O N S T A N T S
// =============================================================================

#define _VERSION_DLL_MAJOR    "1"
#define _VERSION_DLL_MINOR    "1"
#define _VERSION_DLL_RELEASE  "0"
#define _VERSION_DLL_BUILD    "0"
#define _VERSION_DLL_DATE     "16/07/2026"

#define N_CHANNELS_MODULE     32
#define N_BYTES_SAMPLES       2
#define N_BYTES_BEAT          32
#define SIZE_HEADER_BYTES     32

#define TIMEOUT_DMA_BLOCKS_MS	2

// ______________________________________________________________________
// ------ T I P O S   D E   D A T O S
// ======================================================================

typedef struct T_PtrImage
{
  vector<dma_buffer_t> ptr_block;
} T_PtrImage;


int PCIe_SetGeometryBuffers(const T_PCIeGeometry &geometry, int buffer_blocks, int total_images); // Usa una unica geometria por arranque.
int PCIe_InitGeometryManager(int fifo_pointers, const T_PCIeGeometry &geometry); // Comparte el formato exacto de DMA con el gestor.
int PCIe_ResolveGeometry(T_PCIeGeometry &geometry); // Resuelve y valida la configuracion del canal virtual cero antes de abrir DMA.
int PCIe_Open(void);
int PCIe_Close(void);
int PCIe_GetStatus(unsigned long long *status_mask);

int PCIe_GetFreeBlocksNumber(void);
int PCIe_InitBoard(void);
int PCIe_GetPtrBlocks(int *n_read_blocks, void **ptr_blocks, int n_max_blocks);
int PCIe_FreePtrBlocks(int n_blocks);
int PCIe_EnableRx(int enable);
void PCIe_CheckPCIeBlockSaw(void *ptr_block, unsigned long long *channel_errors);
int PCIe_CheckPCIeHeader_fast(void *ptr_block);
int PCIe_image_manager_init(int fifo_pointers, int focal_laws_per_image, int focal_laws_per_block, int frame_length);
long long PCIe_GetAcquiredBlocksNumber(void);
int PCIe_GetUnreadBlocksNumber(void);
int PCIe_SaveImageData(char *file, short int *data, unsigned long n_data);
int PCIe_CreateFileData(char *file);
int PCIe_AddFileImageData(char *file, short int *data, unsigned long n_data);
int PCIE_MovingAverage_KernelSize(int data);
//int PCIe_GetImageData(short int *data, unsigned long size_data, unsigned long *current_image_index, int *last_image);
//int PCIe_GetImageData_Sorted(short int *data, unsigned long size_data, unsigned long *current_image_index, int *last_image);
//int PCIe_GetOneImage_Pointers(short int **focal_laws_pointers, unsigned long *current_image_index, int *last_image, unsigned long *timestamp);
int PCIe_FreeOneImage_Pointers(unsigned long *image_index_fred);
int PCIe_FreeImageData(void);

//int PCIe_SetBuffers(int n_samples, int n_focal_laws_image, int n_images, int n_focal_laws_block);
int PCIe_SetBuffers_mod(int n_samples, int n_focal_laws_image, int n_images_buffer, int n_images_total, int n_focal_laws_block);
int PCIe_Start(int thread_acq_enabled, int time_sleep, int priority, int sorted, int check_header, unsigned long n_images_block, unsigned long n_blocks, int thread_dma_enabled, int thread_olink_enabled);
int PCIe_Stop(void);
int PCIe_ThreadIsRunning(void);
int PCIe_ThreadIsStopped(void);
int PCIe_DMAThreadIsRunning(void);

int ST2_PrintPASystemConfig(void);

extern fuse *gb_drv_handle;
extern int gb_pcie_n_samples;
extern int gb_pcie_n_images_buffer;
//extern size_t gb_size_block;
extern int gb_pcie_n_blocks_acq_total;
extern int gb_pcie_n_beats_frames;
extern int gb_pcie_n_focal_laws_block;
extern int gb_check_pcie_header;
extern size_t gb_pcie_buffer_size_block;
extern bool gb_driver_enabled;
extern int gb_pcie_buffer_n_blocks_buf;
extern int gb_moving_average_kernel_size;
extern bool gb_pcie_enabled;
extern int gb_pcie_closed;

#endif
