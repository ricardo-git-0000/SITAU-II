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

// RMG:
#include <fuse_pcie_soc_hw.h>
#include "fuse_common.h"

//#include <stdlib.h>
//#include <stdint.h>
//#include <string.h>
//#include <limits.h>

#include "fuse_olink.h"
#include <fuse_olink_hw.h>


#define DEV_IS_READY    0x11111111U
#define DEV_IS_STARTED  0x22222222U

//------------------------------------------------------------------

#define assert_void(x)      ASSERT(x)
#define assert_non_void(x)  ASSERT(x)


static inline void _set_reg(fuse_olink *drv, int reg_index, UINT32 value) {
  volatile UINT32 * regs = (volatile UINT32 *)drv->hw_dev;
  regs[reg_index] = value;
}

static inline UINT32 _get_reg(fuse_olink *drv, int reg_index) {
  volatile UINT32 * regs = (volatile UINT32 *)drv->hw_dev;
  return regs[reg_index];
}

static inline void _set_reg_bits(fuse_olink *drv, int reg_index, UINT32 mask) {
  volatile UINT32 * regs = (volatile UINT32 *)drv->hw_dev;
  UINT32 value = regs[reg_index];
  value |= mask;
  regs[reg_index] = value;
}

static inline void _clr_reg_bits(fuse_olink *drv, int reg_index, UINT32 mask) {
  volatile UINT32 * regs = (volatile UINT32 *)drv->hw_dev;
  UINT32 value = regs[reg_index];
  value &= ~mask;
  regs[reg_index] = value;
}

//----

// Convenience functions:

int fuse_olink_get_version(fuse_olink *drv, UINT32 *version) {
  // Verify arguments
  assert_non_void(drv != NULL);
  assert_non_void(drv->is_ready == DEV_IS_READY);
  
  *version = _get_reg(drv, OLNK_VERSION_REG_INDEX);
  return 0;  
}

int fuse_olink_get_enable(fuse_olink *drv, int *enable) {
  // Verify arguments
  assert_non_void(drv != NULL);
  assert_non_void(drv->is_ready == DEV_IS_READY);
  
  *enable = (_get_reg(drv, OLNK_CTRL_REG_INDEX) & OLNK_CTRL_ENABLE);
  return 0;  
}

int fuse_olink_set_enable(fuse_olink *drv, int enable) {
  // Verify arguments
  assert_non_void(drv != NULL);
  assert_non_void(drv->is_ready == DEV_IS_READY);
  
  if(enable) {
    _set_reg_bits(drv, OLNK_CTRL_REG_INDEX, OLNK_CTRL_ENABLE);
  }
  else {
    _clr_reg_bits(drv, OLNK_CTRL_REG_INDEX, OLNK_CTRL_ENABLE);
  }
  return 0;  
}


int fuse_olink_get_sw_reset(fuse_olink *drv, int *reset) {
  // Verify arguments
  assert_non_void(drv != NULL);
  assert_non_void(drv->is_ready == DEV_IS_READY);
  
  *reset = (_get_reg(drv, OLNK_CTRL_REG_INDEX) & OLNK_CTRL_SW_RESET);
  return 0;  
}

int fuse_olink_set_sw_reset(fuse_olink *drv, int reset) {
  // Verify arguments
  assert_non_void(drv != NULL);
  assert_non_void(drv->is_ready == DEV_IS_READY);
  
  if(reset) {
    _set_reg_bits(drv, OLNK_CTRL_REG_INDEX, OLNK_CTRL_SW_RESET);
  }
  else {
    _clr_reg_bits(drv, OLNK_CTRL_REG_INDEX, OLNK_CTRL_SW_RESET);
  }
  return 0;  
}

int fuse_olink_set_gie(fuse_olink *drv, int enable) {
  // Verify arguments
  assert_non_void(drv != NULL);
  assert_non_void(drv->is_ready == DEV_IS_READY);
  
  if(enable) {
    _set_reg_bits(drv, OLNK_CTRL_REG_INDEX, OLNK_CTRL_INT_ENABLE);
  }
  else {
    _clr_reg_bits(drv, OLNK_CTRL_REG_INDEX, OLNK_CTRL_INT_ENABLE);
  }
  return 0;  
}

int fuse_olink_get_int_enable(fuse_olink *drv, UINT32 *mask) {
  // Verify arguments
  assert_non_void(drv != NULL);
  assert_non_void(drv->is_ready == DEV_IS_READY);
  
  *mask = _get_reg(drv, OLNK_INT_ENABLE_REG_INDEX);
  return 0;  
}

int fuse_olink_set_int_enable(fuse_olink *drv, UINT32 mask) {
  // Verify arguments
  assert_non_void(drv != NULL);
  assert_non_void(drv->is_ready == DEV_IS_READY);
  _set_reg(drv, OLNK_INT_ENABLE_REG_INDEX, mask);
  return 0;  
}

int fuse_olink_get_int_flags(fuse_olink *drv, UINT32 *flags) {
  // Verify arguments
  assert_non_void(drv != NULL);
  assert_non_void(drv->is_ready == DEV_IS_READY);
  
  *flags = _get_reg(drv, OLNK_INT_FLAG_REG_INDEX);
  return 0;  
}

int fuse_olink_clr_int_flags(fuse_olink *drv, UINT32 flags) {
  // Verify arguments
  assert_non_void(drv != NULL);
  assert_non_void(drv->is_ready == DEV_IS_READY);
  
  _set_reg(drv, OLNK_INT_FLAG_REG_INDEX, flags);
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

static void fuse_olink_init_cfg(fuse_olink *drv, void *hw_mmap, const fuse_olink_Config *cfg) {
  // Verify arguments
  assert_void(drv != NULL);
  assert_void(cfg != NULL);

  drv->is_ready = 0;
  memset(drv, 0, sizeof(fuse_olink));

  // Save the HW configuration:
  memcpy(&drv->config, cfg, sizeof(fuse_olink_Config));
  drv->hw_dev = ((UINT8 *) hw_mmap) + drv->config.base_addr;

  // Initialize the default interrupt handler:
  drv->handler = default_handler;
  drv->handler_arg = drv;

  // Mark the device as ready once the configuration info has been initialized
  drv->is_ready = DEV_IS_READY;
}


static int fuse_olink_init_hw(fuse_olink *drv) {
  // Verify arguments
  assert_non_void(drv != NULL);
  assert_non_void(drv->is_ready == DEV_IS_READY);

  if (drv->is_started != DEV_IS_STARTED) {
    fuse_olink_set_sw_reset(drv, 1);  // Reset the core
    fuse_olink_set_gie(drv, 0);       // Enable globally the interrupts
    
    _set_reg(drv, OLNK_INT_ENABLE_REG_INDEX, 0);   // Disable all interrupt sources
    _set_reg(drv, OLNK_INT_FLAG_REG_INDEX, ~0U);   // clean interrupt flags
        
    fuse_olink_set_sw_reset(drv, 0); // Kick the module out from reset condition
    drv->is_started = DEV_IS_STARTED;
  }
  return 0;
}

int fuse_olink_initialize(fuse_olink *drv, void *hw_mmap, const fuse_olink_Config *cfg) {
  int ret;

  KdPrint(("[%s] +\n", __FUNCTION__));
  assert_non_void(drv != NULL);

  if (drv->is_started == DEV_IS_STARTED) {
    return 0;
  }

  if (!cfg) {
    return -1;
  }

  fuse_olink_init_cfg(drv, hw_mmap, cfg);
  ret = fuse_olink_init_hw(drv);
  KdPrint(("[%s] -\n", __FUNCTION__));
  return ret;
}

int fuse_olink_is_ready(fuse_olink *drv) {
  if (drv != NULL) {
    return (drv->is_ready == DEV_IS_READY);
  }
  return 0;
}


// Specific functions:

int fuse_olink_i2c_send_byte(fuse_olink *drv, UINT8 byte, int mode) {
  //dbg_printf("[%s] +\n", __FUNCTION__);

  // Verify arguments
  assert_non_void(drv != NULL);
  assert_non_void(drv->is_ready == DEV_IS_READY);
  
  UINT32 i2c_cmd;
  int ret;
  
  i2c_cmd = _get_reg(drv, OLNK_I2C_CMD_REG_INDEX);
  if (i2c_cmd & OLNK_I2C_CMD_BUSY) {
    return -1;  
  }
  
  //set_dbg_mark(1);
  _set_reg(drv, OLNK_I2C_DATA_REG_INDEX, byte);

  i2c_cmd = OLNK_I2C_CMD_RQT | OLNK_I2C_CMD_TX;
  i2c_cmd |= (mode & OLINK_I2C_TX_START)? OLNK_I2C_CMD_START : 0;
  i2c_cmd |= (mode & OLINK_I2C_TX_STOP)? OLNK_I2C_CMD_STOP : 0;
  _set_reg(drv, OLNK_I2C_CMD_REG_INDEX, i2c_cmd);

  int retry_cnt = 0;
  int n_retries = 10;
  
  do {
    fuse_kernel_delay_us(100);
    i2c_cmd = _get_reg(drv, OLNK_I2C_CMD_REG_INDEX);
  }
  while((i2c_cmd & OLNK_I2C_CMD_BUSY) && (retry_cnt++ < n_retries));
  
  if (!(i2c_cmd & OLNK_I2C_CMD_BUSY)) {
    ret = (i2c_cmd & OLNK_I2C_STS_TX_NACK)? 0 : 1;
  }
  else {
    ret = 0; // -1; !!!!
  }

  //dbg_printf("[%s] -\n", __FUNCTION__);
  return ret;
}

int fuse_olink_i2c_recv_byte(fuse_olink *drv, UINT8 *byte, int mode) {
  // Verify arguments
  assert_non_void(drv != NULL);
  assert_non_void(drv->is_ready == DEV_IS_READY);
  
  UINT32 i2c_cmd;
  int ret;
  
  i2c_cmd = _get_reg(drv, OLNK_I2C_CMD_REG_INDEX);
  if (i2c_cmd & OLNK_I2C_CMD_BUSY) {
    return -1;  
  }
  
  i2c_cmd = OLNK_I2C_CMD_RQT;
  i2c_cmd |= (mode & OLINK_I2C_TX_START)? OLNK_I2C_CMD_START : 0;
  i2c_cmd |= (mode & OLINK_I2C_TX_STOP)? OLNK_I2C_CMD_STOP : 0;
  i2c_cmd |= (mode & OLINK_I2C_RX_NACK)? OLNK_I2C_CMD_RX_NACK : 0;  
  _set_reg(drv, OLNK_I2C_CMD_REG_INDEX, i2c_cmd);

  int retry_cnt = 0;
  int n_retries = 10;
  
  do {
    fuse_kernel_delay_us(100);
    i2c_cmd = _get_reg(drv, OLNK_I2C_CMD_REG_INDEX);
  }
  while((i2c_cmd & OLNK_I2C_CMD_BUSY) && (retry_cnt++ < n_retries));
  
  *byte = (UINT8) _get_reg(drv, OLNK_I2C_DATA_REG_INDEX);
  if (!(i2c_cmd & OLNK_I2C_CMD_BUSY)) {
    ret = (i2c_cmd & OLNK_I2C_STS_ARB_LOST)? -1 : 1;
  }
  else {
    ret = -1;
  }

  return ret;  
}

int fuse_olink_i2c_get_status(fuse_olink *drv, int *status) {
  // Verify arguments
  assert_non_void(drv != NULL);
  assert_non_void(drv->is_ready == DEV_IS_READY);
  
  UINT32 i2c_cmd;
  i2c_cmd = _get_reg(drv, OLNK_I2C_CMD_REG_INDEX);
  *status = OLNK_I2C_GET_STATUS(i2c_cmd);
  return 0;
}

//-----------------

//#define I2C_RD_CMD  1
//#define I2C_WR_CMD  0

UINT8 get_i2c_base_cmd(UINT8 i2c_addr, const i2c_mem_t *i2c_mem, unsigned int mem_addr) {
  UINT8 base_cmd;

  if (i2c_mem->addr_bits > 8 * i2c_mem->addr_bytes) {
    UINT8 cmd_addr_bits, cmd_addr_mask;
    int n_bits = i2c_mem->addr_bits - 8 * i2c_mem->addr_bytes;
    cmd_addr_mask = (1U << n_bits) - 1;
    cmd_addr_bits = ((UINT8)(mem_addr >> (8 * i2c_mem->addr_bytes))) & cmd_addr_mask;
    base_cmd = (i2c_addr << n_bits) | cmd_addr_bits;
    base_cmd <<= 1;
  }
  else {
    base_cmd = (i2c_addr << 1);
  }
  return base_cmd;
}

int fuse_olink_wr_fmc_mem(fuse_olink *drv, UINT8 i2c_addr, const i2c_mem_t *i2c_mem, unsigned int mem_addr, const void *dst, int size) {
  int i2c_page_size = i2c_mem->page_size;
  int i2c_page_mask = i2c_mem->page_size - 1;
  UINT8 *p = (UINT8 *)dst;
  UINT8 base_cmd, byte;
  int ret, i;
  int addr;
  int rem, page_rem, n_bytes;


  dbg_printf("[%s] +\n", __FUNCTION__);

  addr = mem_addr;
  rem = size;
  while (rem > 0) {
    dbg_printf(" + Writing on page %d\n", addr >> 6);
    base_cmd = get_i2c_base_cmd(i2c_addr, i2c_mem, addr);

    // send I2C command, checking that the memory is not busy:
    byte = base_cmd | I2C_WR_CMD;
    do {
      ret = fuse_olink_i2c_send_byte(drv, byte, OLINK_I2C_TX_START);
    } 
    while (!ret);

    // send addr bytes:
    for (i = i2c_mem->addr_bytes - 1; i >= 0; i--) {
      byte = (UINT8)((addr >> (8 * i)) & 0xFF);
      ret = fuse_olink_i2c_send_byte(drv, byte, 0);
    }

    page_rem = i2c_page_size - (addr & i2c_page_mask);
    n_bytes = (rem > page_rem) ? page_rem : rem;
    for (i = 0; i < n_bytes - 1; i++) {
      dbg_printf(" + Sending byte %d\n", i);
      ret = fuse_olink_i2c_send_byte(drv, *p++, 0);
      rem--; addr++;
    }
    dbg_printf(" + Sending byte %d\n", i);
    ret = fuse_olink_i2c_send_byte(drv, *p++, OLINK_I2C_TX_STOP);
    rem--; addr++;
  }

  // Perform a read access to poll the memory ready condition:
  // send I2C command:
  base_cmd = get_i2c_base_cmd(i2c_addr, i2c_mem, mem_addr);
  byte = base_cmd | I2C_RD_CMD;
  do {
    ret = fuse_olink_i2c_send_byte(drv, byte, OLINK_I2C_TX_START);
  } 
  while (!ret);
  ret = fuse_olink_i2c_recv_byte(drv, &byte, OLINK_I2C_TX_STOP | OLINK_I2C_RX_NACK);

  dbg_printf("[%s] -\n", __FUNCTION__);

  return 0;
}

int fuse_olink_rd_fmc_mem(fuse_olink *drv, UINT8 i2c_addr, const i2c_mem_t *i2c_mem, unsigned int mem_addr, void *dst, int size) {
  UINT8 *p = (UINT8 *)dst;
  UINT8 base_cmd, byte;
  int i, ret;

  dbg_printf("[%s] +\n", __FUNCTION__);

  base_cmd = get_i2c_base_cmd(i2c_addr, i2c_mem, mem_addr);

  // send I2C command:
  byte = base_cmd | I2C_WR_CMD;
  ret = fuse_olink_i2c_send_byte(drv, byte, OLINK_I2C_TX_START);

  // send addr bytes:
  for (i = i2c_mem->addr_bytes - 1; i >= 0; i--) {
    byte = (UINT8)((mem_addr >> (8 * i)) & 0xFF);
    ret = fuse_olink_i2c_send_byte(drv, byte, 0);
  }

  // send I2C command:
  byte = base_cmd | I2C_RD_CMD;
  ret = fuse_olink_i2c_send_byte(drv, byte, OLINK_I2C_TX_START);

  for (i = 0; i < size - 1; i++) {
    ret = fuse_olink_i2c_recv_byte(drv, p, 0);
    dbg_printf("addr = 0x%8.8X = 0x%2.2X, ret = 0x%8.8X\n", mem_addr + i, *p, ret);
    p++;
  }
  ret = fuse_olink_i2c_recv_byte(drv, p, OLINK_I2C_TX_STOP | OLINK_I2C_RX_NACK);
  dbg_printf("addr = 0x%8.8X = 0x%2.2X, ret = 0x%8.8X\n", mem_addr + i, *p, ret);
  p++;

  dbg_printf("[%s] +\n", __FUNCTION__);

  return 0;
}

//---------

int fuse_olink_get_fmc_i2c_rst(fuse_olink *drv, int *reset) {
  // Verify arguments
  assert_non_void(drv != NULL);
  assert_non_void(drv->is_ready == DEV_IS_READY);

  *reset = (_get_reg(drv, OLNK_FMC_CTRL_REG_INDEX) & OLNK_FMC_I2C_RST);
  return 0;
}

int fuse_olink_set_fmc_i2c_rst(fuse_olink *drv, int reset) {
  // Verify arguments
  assert_non_void(drv != NULL);
  assert_non_void(drv->is_ready == DEV_IS_READY);

  if(reset) {
    _set_reg_bits(drv, OLNK_FMC_CTRL_REG_INDEX, OLNK_FMC_I2C_RST);
  }
  else {
    _clr_reg_bits(drv, OLNK_FMC_CTRL_REG_INDEX, OLNK_FMC_I2C_RST);
  }
  return 0;
}

int fuse_olink_get_clk_gen_rst(fuse_olink *drv, int *reset) {
  // Verify arguments
  assert_non_void(drv != NULL);
  assert_non_void(drv->is_ready == DEV_IS_READY);

  *reset = (_get_reg(drv, OLNK_FMC_CTRL_REG_INDEX) & OLNK_FMC_CLK_GEN_RST);
  return 0;
}

int fuse_olink_set_clk_gen_rst(fuse_olink *drv, int reset) {
  // Verify arguments
  assert_non_void(drv != NULL);
  assert_non_void(drv->is_ready == DEV_IS_READY);

  if(reset) {
    _set_reg_bits(drv, OLNK_FMC_CTRL_REG_INDEX, OLNK_FMC_CLK_GEN_RST);
  }
  else {
    _clr_reg_bits(drv, OLNK_FMC_CTRL_REG_INDEX, OLNK_FMC_CLK_GEN_RST);
  }
  return 0;
}

//---------------

int fuse_olink_get_looback(fuse_olink *drv, looback_mode_t *mode) {
  // Verify arguments
  assert_non_void(drv != NULL);
  assert_non_void(drv->is_ready == DEV_IS_READY);
  int ret = 0;
  UINT32 reg, lb;

  reg = _get_reg(drv, OLNK_AURORA_CTRL_REG_INDEX);
  lb = OLNK_GET_LOOPBACK(reg);

  switch (lb) {
    case OLNK_LB_NORM_OP:
      *mode = LB_MODE_NORM_OP;
      break;
    case OLNK_LB_NEAR_END_PCS:
      *mode = LB_MODE_NEAR_END_PCS;
      break;
    case OLNK_LB_NEAR_END_PMA:
      *mode = LB_MODE_NEAR_END_PMA;
      break;
    case OLNK_LB_FAR_END_PMA:
      *mode = LB_MODE_FAR_END_PMA;
      break;
    case OLNK_LB_FAR_END_PCS:
      *mode = LB_MODE_FAR_END_PCS;
      break;
    default:
      ret = -1;
  }
  return ret;
}

int fuse_olink_set_looback(fuse_olink *drv, looback_mode_t mode) {

  // Verify arguments
  assert_non_void(drv != NULL);
  assert_non_void(drv->is_ready == DEV_IS_READY);

  dbg_printf("[%s] +\n", __FUNCTION__);
  int ret = 0;
  UINT32 reg, lb;

  reg = _get_reg(drv, OLNK_AURORA_CTRL_REG_INDEX);
  lb = OLNK_GET_LOOPBACK(reg);

  switch (mode) {
    case LB_MODE_NORM_OP:
      lb = OLNK_LB_NORM_OP;
      break;
    case LB_MODE_NEAR_END_PCS:
      lb = OLNK_LB_NEAR_END_PCS;
      break;
    case LB_MODE_NEAR_END_PMA:
      lb = OLNK_LB_NEAR_END_PMA;
      break;
    case LB_MODE_FAR_END_PMA:
      lb = OLNK_LB_FAR_END_PMA;
      break;
    case LB_MODE_FAR_END_PCS:
      lb = OLNK_LB_FAR_END_PCS;
      break;
    default:
      lb = 0;
      ret = -1;
  }

  if (!ret) {
    reg &= ~OLNK_LOOPBACK_MASK;
    reg |= OLNK_SET_LOOPBACK(lb);
    _set_reg(drv, OLNK_AURORA_CTRL_REG_INDEX, reg);
  }

  dbg_printf("[%s] -\n", __FUNCTION__);
  return ret;
}

int fuse_olink_get_mgt_reset(fuse_olink *drv, int *rst) {
  // Verify arguments
  assert_non_void(drv != NULL);
  assert_non_void(drv->is_ready == DEV_IS_READY);

  *rst = (_get_reg(drv, OLNK_AURORA_CTRL_REG_INDEX) & OLNK_AURORA_MGT_RST);
  return 0;
}

int fuse_olink_set_mgt_reset(fuse_olink *drv, int rst) {
  // Verify arguments
  assert_non_void(drv != NULL);
  assert_non_void(drv->is_ready == DEV_IS_READY);

  if (rst) {
    _set_reg_bits(drv, OLNK_AURORA_CTRL_REG_INDEX, OLNK_AURORA_MGT_RST);
  }
  else {
    _clr_reg_bits(drv, OLNK_AURORA_CTRL_REG_INDEX, OLNK_AURORA_MGT_RST);
  }
  return 0;
}

int fuse_olink_get_core_reset(fuse_olink *drv, int *rst) {
  // Verify arguments
  assert_non_void(drv != NULL);
  assert_non_void(drv->is_ready == DEV_IS_READY);

  *rst = (_get_reg(drv, OLNK_AURORA_CTRL_REG_INDEX) & OLNK_AURORA_CORE_RST);
  return 0;
}

int fuse_olink_set_core_reset(fuse_olink *drv, int rst) {
  // Verify arguments
  assert_non_void(drv != NULL);
  assert_non_void(drv->is_ready == DEV_IS_READY);

  if (rst) {
    _set_reg_bits(drv, OLNK_AURORA_CTRL_REG_INDEX, OLNK_AURORA_CORE_RST);
  }
  else {
    _clr_reg_bits(drv, OLNK_AURORA_CTRL_REG_INDEX, OLNK_AURORA_CORE_RST);
  }
  return 0;
}


int fuse_olink_get_aurora_status(fuse_olink *drv, UINT32 *x) {
  // Verify arguments
  assert_non_void(drv != NULL);
  assert_non_void(drv->is_ready == DEV_IS_READY);

  *x = _get_reg(drv, OLNK_AURORA_STAT_REG_INDEX);
  DbgPrint("[%s] aurora_status = 0x%8.8X\n", __FUNCTION__, *x);
  
  return 0;
}


int fuse_olink_get_channel_up(fuse_olink *drv, int *x) {
  // Verify arguments
  assert_non_void(drv != NULL);
  assert_non_void(drv->is_ready == DEV_IS_READY);

  *x = (_get_reg(drv, OLNK_AURORA_STAT_REG_INDEX) & OLNK_AURORA_CHANNEL_UP);
  return 0;
}

int fuse_olink_get_all_lane_up(fuse_olink *drv, int *mask) {
  // Verify arguments
  assert_non_void(drv != NULL);
  assert_non_void(drv->is_ready == DEV_IS_READY);
  UINT32 reg;

  reg = _get_reg(drv, OLNK_AURORA_STAT_REG_INDEX);
  *mask = OLNK_GET_LANE_STATUS(reg);
  return 0;
}

int fuse_olink_get_lane_up(fuse_olink *drv, int lane, int *x) {
  // Verify arguments
  assert_non_void(drv != NULL);
  assert_non_void(drv->is_ready == DEV_IS_READY);
  UINT32 reg, lane_up;

  reg = _get_reg(drv, OLNK_AURORA_STAT_REG_INDEX);
  lane_up = OLNK_GET_LANE_STATUS(reg);
  *x = lane_up & (1U << lane);
  return 0;
}

int fuse_olink_get_soft_err(fuse_olink *drv, int *err) {
  // Verify arguments
  assert_non_void(drv != NULL);
  assert_non_void(drv->is_ready == DEV_IS_READY);

  *err = (_get_reg(drv, OLNK_AURORA_STAT_REG_INDEX) & OLNK_AURORA_SOFT_ERR);
  return 0;
}

int fuse_olink_get_hard_err(fuse_olink *drv, int *err) {
  // Verify arguments
  assert_non_void(drv != NULL);
  assert_non_void(drv->is_ready == DEV_IS_READY);

  *err = (_get_reg(drv, OLNK_AURORA_STAT_REG_INDEX) & OLNK_AURORA_HARD_ERR);
  return 0;
}

int fuse_olink_get_mgt_pll_lock(fuse_olink *drv, int *lock) {
  // Verify arguments
  assert_non_void(drv != NULL);
  assert_non_void(drv->is_ready == DEV_IS_READY);

  *lock = (_get_reg(drv, OLNK_AURORA_STAT_REG_INDEX) & OLNK_AURORA_GT_PLL_LOCK);
  return 0;
}

//---

int fuse_olink_clr_err_cnts(fuse_olink *drv) {
  // Verify arguments
  assert_non_void(drv != NULL);
  assert_non_void(drv->is_ready == DEV_IS_READY);

  _set_reg(drv, OLNK_AURORA_STAT_REG_INDEX, 0);
  return 0;
}

int fuse_olink_get_hard_cnt(fuse_olink *drv, unsigned int *err_cnt) {
  // Verify arguments
  assert_non_void(drv != NULL);
  assert_non_void(drv->is_ready == DEV_IS_READY);

  *err_cnt = _get_reg(drv, OLNK_HARD_ERR_REG_INDEX);
  return 0;
}

int fuse_olink_get_soft_cnt(fuse_olink *drv, unsigned int *err_cnt) {
  // Verify arguments
  assert_non_void(drv != NULL);
  assert_non_void(drv->is_ready == DEV_IS_READY);

  *err_cnt = _get_reg(drv, OLNK_SOFT_ERR_REG_INDEX);
  return 0;
}

int fuse_olink_get_channel_cnt(fuse_olink *drv, unsigned int *err_cnt) {
  // Verify arguments
  assert_non_void(drv != NULL);
  assert_non_void(drv->is_ready == DEV_IS_READY);

  *err_cnt = _get_reg(drv, OLNK_CHANNEL_ERR_REG_INDEX);
  return 0;
}

int fuse_olink_get_brd_i2c_reset(fuse_olink *drv, int *reset) {
  // Verify arguments
  assert_non_void(drv != NULL);
  assert_non_void(drv->is_ready == DEV_IS_READY);

  *reset = (_get_reg(drv, OLNK_CTRL_REG_INDEX) & OLNK_CTRL_BRD_I2C_RST);
  return 0;
}

int fuse_olink_set_brd_i2c_reset(fuse_olink *drv, int reset) {
  // Verify arguments
  assert_non_void(drv != NULL);
  assert_non_void(drv->is_ready == DEV_IS_READY);

  if (reset) {
    _set_reg_bits(drv, OLNK_CTRL_REG_INDEX, OLNK_CTRL_BRD_I2C_RST);
  }
  else {
    _clr_reg_bits(drv, OLNK_CTRL_REG_INDEX, OLNK_CTRL_BRD_I2C_RST);
  }
  return 0;
}



#if 0


#endif

