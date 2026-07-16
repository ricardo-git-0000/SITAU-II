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

#ifndef FUSE_OLINK_H
#define FUSE_OLINK_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdlib.h>
#include <basetsd.h>

  // Configuration structure:
typedef struct {
  UINT16 dev_ID;          // Unique ID of device
  UINT32 base_addr;       // Register base address
  UINT32 f_clk;           // Clock frequency
  int    n_lanes;
  float  data_rate_gbps;  // Line rate in Gbps
} fuse_olink_Config;

const fuse_olink_Config *fuse_olink_LookupConfig(UINT16 dev_ID);

// No detail about the underlying HW, only interrupt vectors:
#include "fuse_olink_ints.h"

typedef void (*fuse_olink_handler_t) (void *arg);

// Device driver control structure:

typedef struct {
  fuse_olink_Config config;
  void    *hw_dev;          // device pointer
  UINT32 is_ready;
  UINT32 is_started;
  
  fuse_olink_handler_t handler;
  void *handler_arg;
} fuse_olink;

//-----------------------------------------------

// Setup functions:
int fuse_olink_initialize(fuse_olink *drv, void *hw_mmap, const fuse_olink_Config *cfg);
int fuse_olink_is_ready(fuse_olink *drv);

// Convenience functions:
int fuse_olink_get_version(fuse_olink *drv, UINT32 *version);
int fuse_olink_get_enable(fuse_olink *drv, int *enable);
int fuse_olink_set_enable(fuse_olink *drv, int enable);

int fuse_olink_get_sw_reset(fuse_olink *drv, int *reset);
int fuse_olink_set_sw_reset(fuse_olink *drv, int reset);

int fuse_olink_set_gie(fuse_olink *drv, int enable);
int fuse_olink_get_int_enable(fuse_olink *drv, UINT32 *mask);
int fuse_olink_set_int_enable(fuse_olink *drv, UINT32 mask);
int fuse_olink_get_int_flags(fuse_olink *drv, UINT32 *flags);
int fuse_olink_clr_int_flags(fuse_olink *drv, UINT32 flags);

typedef struct {
  int addr_bytes;
  int addr_bits;
  int page_size;
  int n_pages;
  int size;
} i2c_mem_t;

// Specific functions:

#define I2C_RD_CMD  1
#define I2C_WR_CMD  0
#define SET_I2C_CMD(addr, wr_cmd) ((((addr) & 0x7F) << 1) | (wr_cmd))


#define OLINK_I2C_TX_START  0x01
#define OLINK_I2C_TX_STOP   0x02
#define OLINK_I2C_RX_NACK   0x04

int fuse_olink_i2c_send_byte(fuse_olink *drv, UINT8 byte, int mode);
int fuse_olink_i2c_recv_byte(fuse_olink *drv, UINT8 *byte, int mode);

int fuse_olink_wr_fmc_mem(fuse_olink *drv, UINT8 i2c_addr, const i2c_mem_t *i2c_mem, unsigned int mem_addr, const void *dst, int size);
int fuse_olink_rd_fmc_mem(fuse_olink *drv, UINT8 i2c_addr, const i2c_mem_t *i2c_mem, unsigned int mem_addr, void *dst, int size);

int fuse_olink_get_fmc_i2c_rst(fuse_olink *drv, int *reset);
int fuse_olink_set_fmc_i2c_rst(fuse_olink *drv, int reset);
int fuse_olink_get_clk_gen_rst(fuse_olink *drv, int *reset);
int fuse_olink_set_clk_gen_rst(fuse_olink *drv, int reset);

typedef enum {
  LB_MODE_NORM_OP = 0,
  LB_MODE_NEAR_END_PCS,
  LB_MODE_NEAR_END_PMA,
  LB_MODE_FAR_END_PMA,
  LB_MODE_FAR_END_PCS
} looback_mode_t;

int fuse_olink_get_looback(fuse_olink *drv, looback_mode_t *mode);
int fuse_olink_set_looback(fuse_olink *drv, looback_mode_t mode);

int fuse_olink_get_mgt_reset(fuse_olink *drv, int *rst);
int fuse_olink_set_mgt_reset(fuse_olink *drv, int rst);
int fuse_olink_get_core_reset(fuse_olink *drv, int *rst);
int fuse_olink_set_core_reset(fuse_olink *drv, int rst);

int fuse_olink_get_aurora_status(fuse_olink *drv, UINT32 *x);

int fuse_olink_get_channel_up(fuse_olink *drv, int *x);
int fuse_olink_get_all_lane_up(fuse_olink *drv, int *mask);
int fuse_olink_get_lane_up(fuse_olink *drv, int lane, int *x);
int fuse_olink_get_soft_err(fuse_olink *drv, int *err);
int fuse_olink_get_hard_err(fuse_olink *drv, int *err);
int fuse_olink_get_mgt_pll_lock(fuse_olink *drv, int *lock);

int fuse_olink_clr_err_cnts(fuse_olink *drv);
int fuse_olink_get_hard_cnt(fuse_olink *drv, unsigned int *err_cnt);
int fuse_olink_get_soft_cnt(fuse_olink *drv, unsigned int *err_cnt);
int fuse_olink_get_channel_cnt(fuse_olink *drv, unsigned int *err_cnt);

int fuse_olink_get_brd_i2c_reset(fuse_olink *drv, int *reset);
int fuse_olink_set_brd_i2c_reset(fuse_olink *drv, int reset);

#ifdef __cplusplus
}
#endif

#endif  // FUSE_OLINK_H
