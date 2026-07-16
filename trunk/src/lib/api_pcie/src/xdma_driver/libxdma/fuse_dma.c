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
//  -- Design Name: 
//  -- Module Name: (systemc module)
//  -- Project Name: uGrid on chip.
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

#include <stdio.h>

#include <wdm.h>
#ifdef FUSE_VERBOSE
  #define dbg_printf(...)   KdPrint((__VA_ARGS__))
#else
  #define dbg_printf(x)
#endif

#include <stdlib.h>
#include <string.h>
#include <limits.h>

#include "fuse_dma.h"
#include <fuse_dma_hw.h>

// RMG:
#include <fuse_pcie_soc_hw.h>

#define DEV_IS_READY    0x11111111U
#define DEV_IS_STARTED  0x22222222U

//------------------------------------------------------------------

#define assert_void(x)      ASSERT(x)
#define assert_non_void(x)  ASSERT(x)


static inline void _set_reg(fuse_dma *drv, int reg_index, UINT32 value) {
  volatile UINT32 * regs = (volatile UINT32 *)drv->hw_dev;
  regs[reg_index] = value;
}

static inline UINT32 _get_reg(fuse_dma *drv, int reg_index) {
  volatile UINT32 * regs = (volatile UINT32 *)drv->hw_dev;
  return regs[reg_index];
}

static inline void _set_reg_bits(fuse_dma *drv, int reg_index, UINT32 mask) {
  volatile UINT32 * regs = (volatile UINT32 *)drv->hw_dev;
  UINT32 value = regs[reg_index];
  value |= mask;
  regs[reg_index] = value;
}

static inline void _clr_reg_bits(fuse_dma *drv, int reg_index, UINT32 mask) {
  volatile UINT32 * regs = (volatile UINT32 *)drv->hw_dev;
  UINT32 value = regs[reg_index];
  value &= ~mask;
  regs[reg_index] = value;
}

//----

// Convenience functions:

int fuse_dma_get_version(fuse_dma *drv, UINT32 *version) {
  // Verify arguments
  assert_non_void(drv != NULL);
  assert_non_void(drv->is_ready == DEV_IS_READY);
  
  *version = _get_reg(drv, DMA_VERSION_REG_INDEX);
  return 0;  
}

int fuse_dma_get_enable(fuse_dma *drv, int *enable) {
  // Verify arguments
  assert_non_void(drv != NULL);
  assert_non_void(drv->is_ready == DEV_IS_READY);
  
  *enable = (_get_reg(drv, DMA_CTRL_REG_INDEX) & DMA_CTRL_ENABLE);
  return 0;  
}

int fuse_dma_set_enable(fuse_dma *drv, int enable) {
  // Verify arguments
  assert_non_void(drv != NULL);
  assert_non_void(drv->is_ready == DEV_IS_READY);
  DbgPrint("RMG: [%s] enable = %d\n", __FUNCTION__, enable);
  
  if(enable) {
    _set_reg_bits(drv, DMA_CTRL_REG_INDEX, DMA_CTRL_ENABLE);
  }
  else {
    _clr_reg_bits(drv, DMA_CTRL_REG_INDEX, DMA_CTRL_ENABLE);
  }
  return 0;  
}


int fuse_dma_get_sw_reset(fuse_dma *drv, int *reset) {
  // Verify arguments
  assert_non_void(drv != NULL);
  assert_non_void(drv->is_ready == DEV_IS_READY);
  
  *reset = (_get_reg(drv, DMA_CTRL_REG_INDEX) & DMA_CTRL_SW_RESET);
  return 0;  
}

int fuse_dma_set_sw_reset(fuse_dma *drv, int reset) {
  // Verify arguments
  assert_non_void(drv != NULL);
  assert_non_void(drv->is_ready == DEV_IS_READY);
  DbgPrint("RMG: [%s] rst = %d\n", __FUNCTION__, reset);
  if(reset) {
    _set_reg_bits(drv, DMA_CTRL_REG_INDEX, DMA_CTRL_SW_RESET);
  }
  else {
    _clr_reg_bits(drv, DMA_CTRL_REG_INDEX, DMA_CTRL_SW_RESET);
  }
  return 0;
}

int fuse_dma_set_gie(fuse_dma *drv, int enable) {
  // Verify arguments
  assert_non_void(drv != NULL);
  assert_non_void(drv->is_ready == DEV_IS_READY);
  
  if(enable) {
    _set_reg_bits(drv, DMA_CTRL_REG_INDEX, DMA_CTRL_INT_ENABLE);
  }
  else {
    _clr_reg_bits(drv, DMA_CTRL_REG_INDEX, DMA_CTRL_INT_ENABLE);
  }
  return 0;  
}

int fuse_dma_get_int_enable(fuse_dma *drv, UINT32 *mask) {
  // Verify arguments
  assert_non_void(drv != NULL);
  assert_non_void(drv->is_ready == DEV_IS_READY);
  
  *mask = _get_reg(drv, DMA_INT_ENABLE_REG_INDEX);
  return 0;  
}

int fuse_dma_set_int_enable(fuse_dma *drv, UINT32 mask) {
  // Verify arguments
  assert_non_void(drv != NULL);
  assert_non_void(drv->is_ready == DEV_IS_READY);
  
  _set_reg(drv, DMA_INT_ENABLE_REG_INDEX, mask);
  return 0;  
}

int fuse_dma_get_int_flags(fuse_dma *drv, UINT32 *flags) {
  // Verify arguments
  assert_non_void(drv != NULL);
  assert_non_void(drv->is_ready == DEV_IS_READY);
  
  *flags = _get_reg(drv, DMA_INT_FLAG_REG_INDEX);
  return 0;  
}

int fuse_dma_clr_int_flags(fuse_dma *drv, UINT32 flags) {
  // Verify arguments
  assert_non_void(drv != NULL);
  assert_non_void(drv->is_ready == DEV_IS_READY);
  
  _set_reg(drv, DMA_INT_FLAG_REG_INDEX, flags);
  return 0;  
}

// Inicialization functions:

static void default_handler(void *arg) {
#if DBG
  assert_void(arg != NULL);
#else
  UNREFERENCED_PARAMETER(arg);
#endif
}

static void fuse_dma_init_cfg(fuse_dma *drv, void *hw_mmap, const fuse_dma_Config *cfg) {
  // Verify arguments
  assert_void(drv != NULL);
  assert_void(cfg != NULL);

  drv->is_ready = 0;
  memset(drv, 0, sizeof(fuse_dma));

  // Save the HW configuration:
  memcpy(&drv->config, cfg, sizeof(fuse_dma_Config));
  drv->hw_dev = ((UINT8 *) hw_mmap) + drv->config.base_addr;

        // drv->T_clk = (float)(1.0 / cfg->f_clk);

  // Initialize the default interrupt handler:
  drv->handler = default_handler;
  drv->handler_arg = drv;

  // Mark the device as ready once the configuration info has been initialized
  drv->is_ready = DEV_IS_READY;
}


static int fuse_dma_init_hw(fuse_dma *drv) {
  // Verify arguments
  assert_non_void(drv != NULL);
  assert_non_void(drv->is_ready == DEV_IS_READY);

  if (drv->is_started != DEV_IS_STARTED) {
    fuse_dma_set_sw_reset(drv, 1);  // Reset the core
    fuse_dma_set_gie(drv, 0);       // Enable globally the interrupts
    
    _set_reg(drv, DMA_INT_ENABLE_REG_INDEX, 0);   // Disable all interrupt sources
    _set_reg(drv, DMA_INT_FLAG_REG_INDEX, ~0U);   // clean interrupt flags
        
    fuse_dma_set_sw_reset(drv, 0); // Kick the module out from reset condition
    drv->is_started = DEV_IS_STARTED;
  }
  
  return 0;
}

int fuse_dma_initialize(fuse_dma *drv, void *hw_mmap, const fuse_dma_Config *cfg) {
  int ret;

  KdPrint(("[%s] +\n", __FUNCTION__));
  assert_non_void(drv != NULL);

  if (drv->is_started == DEV_IS_STARTED) {
    return 0;
  }

  if (!cfg) {
    return -1;
  }

  fuse_dma_init_cfg(drv, hw_mmap, cfg);
  ret = fuse_dma_init_hw(drv);
  KdPrint(("[%s] -\n", __FUNCTION__));
  return ret;
}

// Specific functions:

int fuse_dma_get_rx_enable(fuse_dma *drv, int *enable) {
  // Verify arguments
  assert_non_void(drv != NULL);
  assert_non_void(drv->is_ready == DEV_IS_READY);

  *enable = (_get_reg(drv, DMA_CTRL_REG_INDEX) & DMA_CTRL_RX_ENABLE);
  return 0;
}

int fuse_dma_set_rx_enable(fuse_dma *drv, int enable) {
  // Verify arguments
  assert_non_void(drv != NULL);
  assert_non_void(drv->is_ready == DEV_IS_READY);
  DbgPrint("RMG: [%s] enable = %d\n", __FUNCTION__, enable);

  if(enable) {
    _set_reg_bits(drv, DMA_CTRL_REG_INDEX, DMA_CTRL_RX_ENABLE);
  }
  else {
    _clr_reg_bits(drv, DMA_CTRL_REG_INDEX, DMA_CTRL_RX_ENABLE);
  }
  return 0;
}

int fuse_dma_flush(fuse_dma *drv) {
  // Verify arguments
  assert_non_void(drv != NULL);
  assert_non_void(drv->is_ready == DEV_IS_READY);

  _set_reg(drv, DMA_USED_BUFS_REG_INDEX, DMA_FLUSH);
  return 0;
}

int fuse_dma_get_used_buffers(fuse_dma *drv, int *n_buf) {
  // Verify arguments
  assert_non_void(drv != NULL);
  assert_non_void(drv->is_ready == DEV_IS_READY);

  *n_buf = _get_reg(drv, DMA_USED_BUFS_REG_INDEX);
  return 0;
}

int _fuse_dma_get_used_buffers(fuse_dma *drv) {
  // Verify arguments
  assert_non_void(drv != NULL);
  assert_non_void(drv->is_ready == DEV_IS_READY);
  return _get_reg(drv, DMA_USED_BUFS_REG_INDEX);
}


int fuse_dma_get_mem_reset(fuse_dma *drv, int *reset) {
  // Verify arguments
  assert_non_void(drv != NULL);
  assert_non_void(drv->is_ready == DEV_IS_READY);

  *reset = (_get_reg(drv, DMA_CTRL_REG_INDEX) & DMA_CTRL_MEM_RESET);
  return 0;
}

int fuse_dma_set_mem_reset(fuse_dma *drv, int reset) {
  // Verify arguments
  assert_non_void(drv != NULL);
  assert_non_void(drv->is_ready == DEV_IS_READY);
  DbgPrint("[%s] reset = %d\n", __FUNCTION__, reset);
  if (reset) {
    _set_reg_bits(drv, DMA_CTRL_REG_INDEX, DMA_CTRL_MEM_RESET);
  }
  else {
    _clr_reg_bits(drv, DMA_CTRL_REG_INDEX, DMA_CTRL_MEM_RESET);
  }
  return 0;
}

int fuse_dma_trig_test_int(fuse_dma *drv) {
  // Verify arguments
  assert_non_void(drv != NULL);
  assert_non_void(drv->is_ready == DEV_IS_READY);

  _set_reg(drv, DMA_TEST_REG_INDEX, 1);
  return 0;
}

//  int fuse_dma_get_clock_freq(fuse_dma *drv, float *freq) {
//    // Verify arguments
//    assert_non_void(drv != NULL);
//    assert_non_void(drv->is_ready == DEV_IS_READY);
//    *freq = (float) drv->config.f_clk;
//    return 0;
//  }


int fuse_dma_latency_full(fuse_dma *drv, unsigned int *n_cycles) {
  // Verify arguments
  assert_non_void(drv != NULL);
  assert_non_void(drv->is_ready == DEV_IS_READY);

  *n_cycles = _get_reg(drv, DMA_LATENCY_A_REG_INDEX);
  return 0;
}

int fuse_dma_latency_irq(fuse_dma *drv, unsigned int *n_cycles) {
  // Verify arguments
  assert_non_void(drv != NULL);
  assert_non_void(drv->is_ready == DEV_IS_READY);

  *n_cycles = _get_reg(drv, DMA_LATENCY_B_REG_INDEX);
  return 0;
}

//  int fuse_dma_lat_time_full(fuse_dma *drv, float *t) {
//    // Verify arguments
//    assert_non_void(drv != NULL);
//    assert_non_void(drv->is_ready == DEV_IS_READY);
//    UINT32 n_cycles;
//    n_cycles = _get_reg(drv, DMA_LATENCY_A_REG_INDEX);
//    *t = (float)(n_cycles*drv->T_clk);
//  
//    return 0;
//  }
//  
//  int fuse_dma_lat_time_irq(fuse_dma *drv, float *t) {
//    // Verify arguments
//    assert_non_void(drv != NULL);
//    assert_non_void(drv->is_ready == DEV_IS_READY);
//    UINT32 n_cycles;
//    n_cycles = _get_reg(drv, DMA_LATENCY_B_REG_INDEX);
//    *t = (float)(n_cycles*drv->T_clk);
//    return 0;
//  }


#if 0
fuse_dma_get_latency_full(dma_drv, &latency_full);
fuse_dma_get_latency_irq(dma_drv, &latency_irq);

printf("latency_full = %ud\n", latency_full);
printf("latency_irq = %ud\n", latency_irq);

printf("latency_full [ms] = %f\n", (float)(latency_full*(1 / SYS_AXI_CLK_FREQ)*1.0e3));
printf("latency_irq [ms] = %f\n", (float)(latency_irq*(1 / SYS_AXI_CLK_FREQ)*1.0e3));
#endif

int fuse_dma_get_thresholds(fuse_dma *drv, unsigned int *thr_l, unsigned int *thr_h) {
  // Verify arguments
  assert_non_void(drv != NULL);
  assert_non_void(drv->is_ready == DEV_IS_READY);

  UINT32 value;
  value = _get_reg(drv, DMA_BUFFER_THR_REG_INDEX);
  *thr_l = GET_BUFFER_THR_L(value);
  *thr_h = GET_BUFFER_THR_H(value);

  return 0;
}

#define DDR_FIFO_DEPTH  (256)

int fuse_dma_set_thresholds(fuse_dma *drv, unsigned int thr_l, unsigned int thr_h) {
  // Verify arguments
  assert_non_void(drv != NULL);
  assert_non_void(drv->is_ready == DEV_IS_READY);

  UINT32 value;
  int ret;
  if ((thr_l < DDR_FIFO_DEPTH) && (thr_l < DDR_FIFO_DEPTH)) {
    value = SET_BUFFER_THRS(thr_l, thr_h);
    _set_reg(drv, DMA_BUFFER_THR_REG_INDEX, value);
    ret = 0;
  }
  else {
    ret = -1;
  }
  return ret;
}

//-----

int fuse_dma_get_perf_cycles(fuse_dma *drv, unsigned int *total, unsigned int *active) {
  // Verify arguments
  assert_non_void(drv != NULL);
  assert_non_void(drv->is_ready == DEV_IS_READY);

  *active = 1 + _get_reg(drv, DMA_ACTIVE_TIME_REG_INDEX);
  *total = 1 + _get_reg(drv, DMA_TOTAL_TIME_REG_INDEX);
  return 0;
}



#ifdef APP

int fuse_dma_get_perf_clk_freq(fuse_dma *drv, float *freq) {
  // Verify arguments
  assert_non_void(drv != NULL);
  assert_non_void(drv->is_ready == DEV_IS_READY);

  *freq = drv->config.rx_axis_freq_MHz;
  return 0;
}

int fuse_dma_get_perf_time(fuse_dma *drv, float *total, float *active) {
  // Verify arguments
  assert_non_void(drv != NULL);
  assert_non_void(drv->is_ready == DEV_IS_READY);

  *total = drv->rx_T_clk * (1 + _get_reg(drv, DMA_TOTAL_TIME_REG_INDEX));
  *active = drv->rx_T_clk * (1 + _get_reg(drv, DMA_ACTIVE_TIME_REG_INDEX));
  return 0;
}

int fuse_dma_get_perf_BW(fuse_dma *drv, float *BW, unsigned int n_gap) {
  // Verify arguments
  assert_non_void(drv != NULL);
  assert_non_void(drv->is_ready == DEV_IS_READY);

  UINT32 n_total, n_beats;
  n_beats = 1 + _get_reg(drv, DMA_ACTIVE_TIME_REG_INDEX);
  n_total = 1 + _get_reg(drv, DMA_TOTAL_TIME_REG_INDEX);

  *BW = drv->rx_bw_factor * ((float)n_beats)/(n_total+n_gap);
  return 0;
}

#endif  


int fuse_dma_get_image_id_seed(fuse_dma *drv, UINT32 *id) {
  // Verify arguments
  assert_non_void(drv != NULL);
  assert_non_void(drv->is_ready == DEV_IS_READY);
  
  *id = _get_reg(drv, DMA_IMAGE_ID_REG_INDEX);
  return 0;  
}

int fuse_dma_set_image_id_seed(fuse_dma *drv, UINT32 id) {
  // Verify arguments
  assert_non_void(drv != NULL);
  assert_non_void(drv->is_ready == DEV_IS_READY);
  
  _set_reg(drv, DMA_IMAGE_ID_REG_INDEX, id);
  return 0;  
}

//---

int fuse_dma_get_mem_size(fuse_dma *drv, UINT32 *msize) {
  // Verify arguments
  assert_non_void(drv != NULL);
  assert_non_void(drv->is_ready == DEV_IS_READY);
  UINT32 reg;
  reg = _get_reg(drv, DMA_CTRL_REG_INDEX);
  *msize = GET_MEM_SIZE(reg);
  return 0;  
}

int fuse_dma_dna_ok(fuse_dma *drv) {
  // Verify arguments
  assert_non_void(drv != NULL);
  assert_non_void(drv->is_ready == DEV_IS_READY);
  int ret;
  ret = (_get_reg(drv, DMA_CTRL_REG_INDEX) & DMA_STATUS_DNA_OK)? 1 : 0;
  return ret;
}
