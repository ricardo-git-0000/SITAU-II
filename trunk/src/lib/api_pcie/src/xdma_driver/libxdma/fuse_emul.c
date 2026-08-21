#include "fuse_emul.h"
#include <fuse_emul_hw.h>

#include <wdm.h>
#ifdef FUSE_VERBOSE
  #define dbg_printf(...)   KdPrint((__VA_ARGS__))
#else
  #define dbg_printf(x)
#endif

// RMG:
#include <fuse_pcie_soc_hw.h>
#include "fuse_common.h"

#define DEV_IS_READY    0x11111111U
#define DEV_IS_STARTED  0x22222222U

//#define assert_void(x)
//#define assert_non_void(x)

#define assert_void(x)      ASSERT(x)
#define assert_non_void(x)  ASSERT(x)

static inline void _set_reg(fuse_emul *drv, int reg_index, UINT32 value) {
  volatile UINT32 * regs = (volatile UINT32 *)drv->hw_dev;
  regs[reg_index] = value;
}

static inline UINT32 _get_reg(fuse_emul *drv, int reg_index) {
  volatile UINT32 * regs = (volatile UINT32 *)drv->hw_dev;
  return regs[reg_index];
}

static inline void _set_reg_bits(fuse_emul *drv, int reg_index, UINT32 mask) {
  volatile UINT32 * regs = (volatile UINT32 *)drv->hw_dev;
  UINT32 value = regs[reg_index];
  value |= mask;
  regs[reg_index] = value;
}

static inline void _clr_reg_bits(fuse_emul *drv, int reg_index, UINT32 mask) {
  volatile UINT32 * regs = (volatile UINT32 *)drv->hw_dev;
  UINT32 value = regs[reg_index];
  value &= ~mask;
  regs[reg_index] = value;
}

//----

// Convenience functions:

int fuse_emul_get_version(fuse_emul *drv, UINT32 *version) {
  // Verify arguments
  assert_non_void(drv != NULL);
  assert_non_void(drv->is_ready == DEV_IS_READY);

  *version = _get_reg(drv, EMUL_VERSION_REG_INDEX);
  return 0;
}

int fuse_emul_get_enable(fuse_emul *drv, int *enable) {
  // Verify arguments
  assert_non_void(drv != NULL);
  assert_non_void(drv->is_ready == DEV_IS_READY);

  *enable = (_get_reg(drv, EMUL_CTRL_REG_INDEX) & EMUL_CTRL_ENABLE);
  return 0;
}

int fuse_emul_set_enable(fuse_emul *drv, int enable) {
  // Verify arguments
  assert_non_void(drv != NULL);
  assert_non_void(drv->is_ready == DEV_IS_READY);

  if (enable) {
    _set_reg_bits(drv, EMUL_CTRL_REG_INDEX, EMUL_CTRL_ENABLE);
  }
  else {
    _clr_reg_bits(drv, EMUL_CTRL_REG_INDEX, EMUL_CTRL_ENABLE);
  }
  return 0;
}


int fuse_emul_get_sw_reset(fuse_emul *drv, int *reset) {
  // Verify arguments
  assert_non_void(drv != NULL);
  assert_non_void(drv->is_ready == DEV_IS_READY);

  *reset = (_get_reg(drv, EMUL_CTRL_REG_INDEX) & EMUL_CTRL_SW_RESET);
  return 0;
}

int fuse_emul_set_sw_reset(fuse_emul *drv, int reset) {
  // Verify arguments
  assert_non_void(drv != NULL);
  assert_non_void(drv->is_ready == DEV_IS_READY);

  if (reset) {
    _set_reg_bits(drv, EMUL_CTRL_REG_INDEX, EMUL_CTRL_SW_RESET);
  }
  else {
    _clr_reg_bits(drv, EMUL_CTRL_REG_INDEX, EMUL_CTRL_SW_RESET);
  }
  return 0;
}

int fuse_emul_set_gie(fuse_emul *drv, int enable) {
  // Verify arguments
  assert_non_void(drv != NULL);
  assert_non_void(drv->is_ready == DEV_IS_READY);

  if (enable) {
    _set_reg_bits(drv, EMUL_CTRL_REG_INDEX, EMUL_CTRL_INT_ENABLE);
  }
  else {
    _clr_reg_bits(drv, EMUL_CTRL_REG_INDEX, EMUL_CTRL_INT_ENABLE);
  }
  return 0;
}

int fuse_emul_get_int_enable(fuse_emul *drv, UINT32 *mask) {
  // Verify arguments
  assert_non_void(drv != NULL);
  assert_non_void(drv->is_ready == DEV_IS_READY);

  *mask = _get_reg(drv, EMUL_INT_ENABLE_REG_INDEX);
  return 0;
}

int fuse_emul_set_int_enable(fuse_emul *drv, UINT32 mask) {
  // Verify arguments
  assert_non_void(drv != NULL);
  assert_non_void(drv->is_ready == DEV_IS_READY);

  _set_reg(drv, EMUL_INT_ENABLE_REG_INDEX, mask);
  return 0;
}

int fuse_emul_get_int_flags(fuse_emul *drv, UINT32 *flags) {
  // Verify arguments
  assert_non_void(drv != NULL);
  assert_non_void(drv->is_ready == DEV_IS_READY);

  *flags = _get_reg(drv, EMUL_INT_FLAG_REG_INDEX);
  return 0;
}

int fuse_emul_clr_int_flags(fuse_emul *drv, UINT32 flags) {
  // Verify arguments
  assert_non_void(drv != NULL);
  assert_non_void(drv->is_ready == DEV_IS_READY);

  _set_reg(drv, EMUL_INT_FLAG_REG_INDEX, flags);
  return 0;
}

// Inicialization functions:

static void default_handler(void *arg) {
  UNREFERENCED_PARAMETER(arg);
  assert_void(arg != NULL);
}

static void fuse_emul_init_cfg(fuse_emul *drv, void *hw_mmap, const fuse_emul_Config *cfg) {
  // Verify arguments
  assert_void(drv != NULL);
  assert_void(cfg != NULL);

  drv->is_ready = 0;
  memset(drv, 0, sizeof(fuse_emul));

  // Save the HW configuration:
  memcpy(&drv->config, cfg, sizeof(fuse_emul_Config));
  drv->hw_dev = ((UINT8 *)hw_mmap) + drv->config.base_addr;

  // Initialize the default interrupt handler:
  drv->handler = default_handler;
  drv->handler_arg = drv;

  // Mark the device as ready once the configuration info has been initialized
  drv->is_ready = DEV_IS_READY;
}


static int fuse_emul_init_hw(fuse_emul *drv) {
  // Verify arguments
  assert_non_void(drv != NULL);
  assert_non_void(drv->is_ready == DEV_IS_READY);

  if (drv->is_started != DEV_IS_STARTED) {
    fuse_emul_set_sw_reset(drv, 1);  // Reset the core
    fuse_emul_set_gie(drv, 0);       // Enable globally the interrupts

    _set_reg(drv, EMUL_INT_ENABLE_REG_INDEX, 0);   // Disable all interrupt sources
    _set_reg(drv, EMUL_INT_FLAG_REG_INDEX, ~0U);    // clean interrupt flags

    fuse_emul_set_mode(drv, 0); // Para que no empiece en modo continuo !!!

    fuse_emul_set_sw_reset(drv, 0); // Kick the module out from reset condition
    drv->is_started = DEV_IS_STARTED;
  }

  return 0;
}

int fuse_emul_initialize(fuse_emul *drv, void *hw_mmap, const fuse_emul_Config *cfg) {
  int ret;
  KdPrint(("[%s] +\n", __FUNCTION__));

  assert_non_void(drv != NULL);

  if (drv->is_started == DEV_IS_STARTED) {
    return 0;
  }

  if (!cfg) {
    return -1;
  }

  fuse_emul_init_cfg(drv, hw_mmap, cfg);
  ret = fuse_emul_init_hw(drv);
  KdPrint(("[%s] -\n", __FUNCTION__));
  return ret;
}

#define FRAMES_PER_PACKAGE  8
#define WORD32_PER_BEAT     (256/32)
#define BEAT_SIZE           (256/8)
#define HEADER_SIZE         (1*BEAT_SIZE)

int fuse_emul_generate(fuse_emul *drv, UINT32 n_imgs, UINT32 frame_len, UINT32 img_len, UINT32 gap, int cont_mode, int nb) {
  dbg_printf("[%s] +\n", __FUNCTION__);
  //KdPrint(("[%s] + \n", __FUNCTION__));

  // Verify arguments
  assert_non_void(drv != NULL);
  assert_non_void(drv->is_ready == DEV_IS_READY);

  UINT32 tx_cmd;
  int image_size;
  int ret = 0;

  tx_cmd = _get_reg(drv, EMUL_TX_CMD_REG_INDEX);
  if (tx_cmd & EMUL_CMD_TX_BUSY) {
    return -1;
  }

  if ( (frame_len > EMUL_MAX_FRAME_LEN) || (img_len > EMUL_MAX_N_PACKETS) || (gap > EMUL_MAX_GAP_LEN)) {
    dbg_printf("Bad configuration\n");
    return -1;
  }

  image_size = (img_len * FRAMES_PER_PACKAGE * frame_len * BEAT_SIZE) + HEADER_SIZE;

  ret = fuse_emul_set_mode(drv, cont_mode);
  _set_reg(drv, EMUL_FRAME_LEN_REG_INDEX, frame_len - 1);
  _set_reg(drv, EMUL_IMAGE_LEN_REG_INDEX, img_len - 1);
  _set_reg(drv, EMUL_GAP_LEN_REG_INDEX, gap);

  tx_cmd = EMUL_SET_TX_CMD(n_imgs - 1);
  _set_reg(drv, EMUL_TX_CMD_REG_INDEX, tx_cmd);

  
  if (!cont_mode && !nb) {
    //int retry_cnt = 0;
    do {
      fuse_kernel_delay_us(100);
      tx_cmd = _get_reg(drv, EMUL_TX_CMD_REG_INDEX);
      //KdPrint(("[%s] waiting for tx finish, retries = %d!!!\n", __FUNCTION__, retry_cnt++));
    }
    while(tx_cmd & EMUL_CMD_TX_BUSY);
    KdPrint(("[%s] Done!!!, busy = %d\n", __FUNCTION__, (tx_cmd & EMUL_CMD_TX_BUSY)?1:0));
  }


  dbg_printf("[%s] -\n", __FUNCTION__);
  //KdPrint(("[%s] - \n", __FUNCTION__));
  return image_size;
}

int fuse_emul_stop_cont(fuse_emul *drv, int nb) {
  UINT32 tx_cmd;
  int ret = 0;
  dbg_printf("[%s] +\n", __FUNCTION__);
  //KdPrint(("[%s] + \n", __FUNCTION__));

  // Verify arguments
  assert_non_void(drv != NULL);
  assert_non_void(drv->is_ready == DEV_IS_READY);
  
  int cont_mode;
  
  fuse_emul_get_mode(drv, &cont_mode);
  if (cont_mode) {
    // Salimos del modo continuo:
    ret = fuse_emul_set_mode(drv, 0);
    // Si es 'blocking mode' esperamos a que termine la generación:
    if (!nb) {
      do {
        fuse_kernel_delay_us(100);
        tx_cmd = _get_reg(drv, EMUL_TX_CMD_REG_INDEX);
        //KdPrint(("[%s] waiting for tx finish, retries = %d!!!\n", __FUNCTION__, retry_cnt++));
      }
      while(tx_cmd & EMUL_CMD_TX_BUSY);
      KdPrint(("[%s] Done!!!, busy = %d\n", __FUNCTION__, (tx_cmd & EMUL_CMD_TX_BUSY)?1:0));
    }
  }
  dbg_printf("[%s] -\n", __FUNCTION__);
  //KdPrint(("[%s] - \n", __FUNCTION__));
  return 0;
}

int fuse_emul_get_mode(fuse_emul *drv, int *mode) {
  // Verify arguments
  assert_non_void(drv != NULL);
  assert_non_void(drv->is_ready == DEV_IS_READY);

  *mode = (_get_reg(drv, EMUL_CTRL_REG_INDEX) & EMUL_CTRL_CONT_MODE);
  return 0;
}

int fuse_emul_set_mode(fuse_emul *drv, int mode) {
  // Verify arguments
  assert_non_void(drv != NULL);
  assert_non_void(drv->is_ready == DEV_IS_READY);

  if (mode) {
    _set_reg_bits(drv, EMUL_CTRL_REG_INDEX, EMUL_CTRL_CONT_MODE);
  }
  else {
    _clr_reg_bits(drv, EMUL_CTRL_REG_INDEX, EMUL_CTRL_CONT_MODE);
  }
  return 0;
}

int fuse_emul_fault_injection(fuse_emul *drv, int frame_delta, int target_frame, int image_delta, int target_image) {
  // Verify arguments
  assert_non_void(drv != NULL);
  assert_non_void(drv->is_ready == DEV_IS_READY);

  UINT32 fault_cmd, fault_mask = 0;

  fault_mask |= (frame_delta) ? INJECT_FRAME_ERR : 0;
  fault_mask |= (image_delta) ? INJECT_IMAGE_ERR : 0;
  fault_cmd = SET_FAULT_INJECTION(frame_delta, image_delta, fault_mask);

  _set_reg(drv, EMUL_TGT_FRAME_REG_INDEX, target_frame);
  _set_reg(drv, EMUL_TGT_IMAGE_REG_INDEX, target_image);
  _set_reg(drv, EMUL_FAULT_INJECT_REG_INDEX, fault_cmd);

  return 0;
}
