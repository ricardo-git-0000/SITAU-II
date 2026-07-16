#ifndef FUSE_HW_IMG_H
#define FUSE_HW_IMG_H

#ifdef __cplusplus
extern "C" {
#endif

#ifdef _KERNEL_MODE
#include <basetsd.h>
  typedef UINT32 __uint32_t;
#else
#include <stdint.h>
  typedef uint32_t __uint32_t;
#endif

typedef struct {
  __uint32_t n_images;
  __uint32_t frame_len;
  __uint32_t image_len;
  __uint32_t gap;
  __uint32_t mode;
} hw_img_conf_t;

//#define FUSE_EMUL_MODE_CONTINOUS    0x00000001
//#define FUSE_EMUL_MODE_NB           0x00000002

#define FUSE_EMUL_MODE_CONT     0x00000001U
#define FUSE_EMUL_MODE_NBLOCK   0x00000002U



#ifdef __cplusplus
}
#endif

#endif  // FUSE_HW_IMG_H

// fuse_hw_img