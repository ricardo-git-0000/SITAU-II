#ifndef FUSE_EMUL_H
#define FUSE_EMUL_H

#include <basetsd.h>

// Configuration structure:
typedef struct {
  UINT16 dev_ID;          // Unique ID of device
  UINT32 base_addr;       // Register base address
  UINT32 f_clk;           // Clock frequency
} fuse_emul_Config;

#include "fuse_emul_ints.h"

typedef void (*fuse_emul_handler_t) (void *arg);

// Device driver control structure:

typedef struct {
  fuse_emul_Config config;
  void    *hw_dev;        // device pointer
  UINT32 is_ready;
  UINT32 is_started;
  
  fuse_emul_handler_t handler;
  void *handler_arg;
} fuse_emul;

// Setup functions:
int fuse_emul_initialize(fuse_emul *drv, void *hw_mmap, const fuse_emul_Config *cfg);

// Convenience functions:
int fuse_emul_get_version(fuse_emul *drv, UINT32 *version);
int fuse_emul_get_enable(fuse_emul *drv, int *enable);
int fuse_emul_set_enable(fuse_emul *drv, int enable);

int fuse_emul_get_sw_reset(fuse_emul *drv, int *reset);
int fuse_emul_set_sw_reset(fuse_emul *drv, int reset);

int fuse_emul_set_gie(fuse_emul *drv, int enable);
int fuse_emul_get_int_enable(fuse_emul *drv, UINT32 *mask);
int fuse_emul_set_int_enable(fuse_emul *drv, UINT32 mask);
int fuse_emul_get_int_flags(fuse_emul *drv, UINT32 *flags);
int fuse_emul_clr_int_flags(fuse_emul *drv, UINT32 flags);

// Specific functions:
int fuse_emul_get_mode(fuse_emul *drv, int *mode);
int fuse_emul_set_mode(fuse_emul *drv, int mode);

//typedef struct {
//  int n_images;
//  int frame_length;
//  int image_length;
//  int gap;
//} _image_conf_t;

int fuse_emul_generate(fuse_emul *drv, UINT32 n_imgs, UINT32 frame_len, UINT32 img_len, UINT32 gap, int cont_mode, int nb);
int fuse_emul_stop_cont(fuse_emul *drv, int nb);


//int calc_image_size(const image_conf_t *conf);
//int calc_image_gap(int frame_length, int image_length, uint64_t BW);



#endif  // FUSE_EMUL_H
