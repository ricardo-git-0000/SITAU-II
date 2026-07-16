#ifndef FUSE_REMOTE_H
#define FUSE_REMOTE_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

#define FUSE_REMOTE_ADDR "192.168.1.120"
//#define FUSE_REMOTE_ADDR "192.168.0.60"
#define FUSE_REMOTE_PORT 1234

#ifdef __GNUC__
#define PACKED __attribute__((packed))
#else
#define PACKED
#endif

#ifdef _WIN32
#pragma pack(1)
#endif

#ifndef FUSE_MAGIC_NUM
#define FUSE_MAGIC_NUM  0x45535546
#endif

typedef struct {
  uint32_t magic_num;
  uint16_t length;
  uint16_t msg_id;
  uint16_t csum;
  uint16_t cmd_reply;
  uint8_t  data[0];
} PACKED fuse_msg_t;

typedef struct {
  uint32_t n_img;
  uint32_t frame_len;
  uint32_t image_len;
  uint32_t gap;
  uint32_t mode;
} PACKED cmd_gen_t;

// Fault injection command
typedef struct {
  int32_t frame_delta;
  uint32_t target_frame;
  int32_t image_delta;
  uint32_t target_image;
  uint32_t fault;
} PACKED cmd_fault_t;

typedef struct {
  uint32_t img_id;
} PACKED cmd_img_id_t;

typedef struct {
  uint32_t pause;
} PACKED cmd_pause_t;

#ifndef FUSE_EMUL_MODE_CONT
#define FUSE_EMUL_MODE_CONT     0x00000001U
#endif

#ifndef FUSE_EMUL_MODE_NBLOCK
#define FUSE_EMUL_MODE_NBLOCK   0x00000002U
#endif

// FUSE emul status masks:
#define FUSE_EMUL_CONT_MODE     0x0001
#define FUSE_EMUL_TX_BUSY       0x0002
#define FUSE_EMUL_RUN_DONE      0x0004
#define FUSE_EMUL_IMG_DONE      0x0008

#define FUSE_AURORA_LOOPBACK_NORMAL     0
#define FUSE_AURORA_LOOPBACK_NORMAL     0




#ifdef _WIN32
#pragma pack()
#endif

#define FUSE_CMD_EMUL_START           0x0003
#define FUSE_CMD_EMUL_STOP            0x0004
#define FUSE_CMD_GET_EMUL_STATUS      0x0005
#define FUSE_CMD_GET_OLINK_STATUS     0x0006
#define FUSE_CMD_SET_OLINK_LOOPBACK   0x0007
#define FUSE_CMD_GET_SYS_STATUS       0x0008
#define FUSE_CMD_FAULT_INJECTION      0x0009
#define FUSE_CMD_SET_IMG_ID_SEED      0x000A
#define FUSE_CMD_EMUL_PAUSE           0x000B
#define FUSE_CMD_EMUL_ABORT           0x000C

#define FUSE_REPLY_ACK                0x5555
#define FUSE_REPLY_NACK               0xAAAA

#define FUSE_REPLY_EMUL_STATUS        (FUSE_CMD_GET_EMUL_STATUS | 0x0100)
#define FUSE_REPLY_OLINK_STATUS       (FUSE_CMD_GET_OLINK_STATUS | 0x0100)
#define FUSE_REPLY_SYS_STATUS         (FUSE_CMD_GET_SYS_STATUS | 0x0100)

#ifdef __cplusplus
}
#endif


#endif  // FUSE_REMOTE_H


