#ifndef FUSE_TYPES_H
#define FUSE_TYPES_H

#ifdef __cplusplus
extern "C" {
#endif


#define FUSE_MAGIC_NUM  0x45535546

#pragma pack(1)

// Visual Studio will fit consecutive bitfields into an underlying type, 
// if it can, and waste the leftover space.

typedef struct {
  uint32_t  magic_num;
  uint32_t  image_id;
  uint32_t  frame_len;
  uint32_t  image_len : 24;
  uint32_t  flags : 8;
  //uint8_t   flags;
  uint32_t  spare[4];
} fuse_header_t;

//  constant C_FRAME_BITS : integer := 16;
//  constant C_IMAGE_BITS : integer := 10;

#define FRAMES_PER_PACKAGE  4
#define WORD32_PER_BEAT     (256/32)

typedef struct {
  uint32_t word_idx   : 3;
  uint32_t beat_idx   : 16;
  uint32_t frame_idx  : 2;
  uint32_t packet_idx : 10;
  uint32_t            : 1;
} test_pattern_t;

#pragma pack()

union fuse_data {
  uint32_t  word32;
  test_pattern_t fields;
};


#ifdef __cplusplus
}
#endif

#endif  // FUSE_TYPES_H

