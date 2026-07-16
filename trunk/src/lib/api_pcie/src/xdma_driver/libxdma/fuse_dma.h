//  ------------------------------------------------------------
//  -- Copyright: 2017 Raul Mateos-Gil. University of Alcala
//  --            https://soc-repo.depeca.uah.es/gitlab
//  ------------------------------------------------------------
//  
//  ----------------------------------------------------------------------------------
//  -- Company: University of Alcala
//  -- Authors: Raul Mateos
//  -- 
//  -- Create Date: 06/11/2017 10:23:13
//  -- Design Name: xxx
//  -- Module Name: xxx (systemc module)
//  -- Project Name: xxx SoC.
//  -- Target Devices: Xilinx 7 series (Zynq)
//  -- Tool Versions: Questa Sim 10.4c / Vivado 2015.4
//  -- Description: 
//  -- 
//  -- Dependencies: RMG support package
//  -- 
//  -- Revision:
//  -- Revision 0.01 - File Created
//  -- Additional Comments:
//  -- 
//  ----------------------------------------------------------------------------------
//  -- Modification history:
//  --  06/11/2017: source code established.
//  ----------------------------------------------------------------------------------

#ifndef FUSE_DMA_H
#define FUSE_DMA_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdlib.h>
#include <basetsd.h>

// Configuration structure:
typedef struct {
  UINT16 dev_ID;            // Unique ID of device
  UINT32 base_addr;         // Register base address
  UINT32 mem_size_MB;       // DDR memory size (in MBytes)
  float  axi_freq_MHz;      // AXI clock frequency
  UINT32 rx_axis_beat_size; // Rx AXI stream beat size (in bytes)
  float  rx_axis_freq_MHz;  // Rx AXI stream clock frequency
  UINT32 tx_axis_beat_size; // Tx AXI stream beat size (in bytes)
  float  tx_axis_freq_MHz;  // Tx AXI stream clock frequency
  const char *dev_dna;      // Device DNA (hex stream)
} fuse_dma_Config;

const fuse_dma_Config *fuse_dma_LookupConfig(UINT16 dev_ID);

// No detail about the underlying HW, only interrupt vectors:
#include "fuse_dma_ints.h"

typedef void (*fuse_dma_handler_t) (void *arg);

// Device driver control structure:

typedef struct {
  fuse_dma_Config config;
  void    *hw_dev;          // device pointer
  UINT32  is_ready;
  UINT32  is_started;
  UINT32  rx_beat_size;
  float   rx_T_clk;
  float   rx_bw_factor;
  
  fuse_dma_handler_t handler;
  void *handler_arg;
} fuse_dma;

// Setup functions:
int fuse_dma_initialize(fuse_dma *drv, void *hw_mmap, const fuse_dma_Config *cfg);

// Convenience functions:
int fuse_dma_get_version(fuse_dma *drv, UINT32 *version);
int fuse_dma_get_enable(fuse_dma *drv, int *enable);
int fuse_dma_set_enable(fuse_dma *drv, int enable);

int fuse_dma_get_sw_reset(fuse_dma *drv, int *reset);
int fuse_dma_set_sw_reset(fuse_dma *drv, int reset);

int fuse_dma_set_gie(fuse_dma *drv, int enable);
int fuse_dma_get_int_enable(fuse_dma *drv, UINT32 *mask);
int fuse_dma_set_int_enable(fuse_dma *drv, UINT32 mask);
int fuse_dma_get_int_flags(fuse_dma *drv, UINT32 *flags);
int fuse_dma_clr_int_flags(fuse_dma *drv, UINT32 flags);

// Specific functions:
int fuse_dma_get_rx_enable(fuse_dma *drv, int *mode);
int fuse_dma_set_rx_enable(fuse_dma *drv, int mode);

int fuse_dma_flush(fuse_dma *drv);
int fuse_dma_get_used_buffers(fuse_dma *drv, int *n_buf); 
int _fuse_dma_get_used_buffers(fuse_dma *drv);

int fuse_dma_get_mem_reset(fuse_dma *drv, int *reset);
int fuse_dma_set_mem_reset(fuse_dma *drv, int reset);

int fuse_dma_get_thresholds(fuse_dma *drv, unsigned int *thr_l, unsigned int *thr_h);
int fuse_dma_set_thresholds(fuse_dma *drv, unsigned int thr_l, unsigned int thr_h);

//int fuse_dma_generate(fuse_dma *drv, int n_images, int frame_len, int image_len, int gap);

int fuse_dma_trig_test_int(fuse_dma *drv);

//  int fuse_dma_get_clock_freq(fuse_dma *drv, float *freq);

int fuse_dma_latency_full(fuse_dma *drv, unsigned int *n_cycles);
int fuse_dma_latency_irq(fuse_dma *drv, unsigned int *n_cycles);

int fuse_dma_get_perf_cycles(fuse_dma *drv, unsigned int *total, unsigned int *active);

int fuse_dma_get_image_id_seed(fuse_dma *drv, UINT32 *id);
int fuse_dma_set_image_id_seed(fuse_dma *drv, UINT32 id);

int fuse_dma_get_mem_size(fuse_dma *drv, UINT32 *msize);
int fuse_dma_dna_ok(fuse_dma *drv);


#ifdef APP

int fuse_dma_get_perf_clk_freq(fuse_dma *drv, float *freq);
int fuse_dma_get_perf_time(fuse_dma *drv, float *total, float *active);
int fuse_dma_get_perf_BW(fuse_dma *drv, float *BW, unsigned int gap);

#endif

//  int fuse_dma_lat_time_full(fuse_dma *drv, float *t);
//  int fuse_dma_lat_time_irq(fuse_dma *drv, float *t);

#ifdef __cplusplus
}
#endif

#endif  // FUSE_DMA_H
