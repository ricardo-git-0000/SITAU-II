#ifndef FUSE_OLINK_INTS_H
#define FUSE_OLINK_INTS_H

//----------------------
//-- INTERRUPT MASKS: --
//----------------------

#define CH_ERR_EVT_INT_MASK     0x0001    // Channel up falling edge        (event interrupt)
#define SOFT_ERR_EVT_INT_MASK   0x0002    // Aurora soft error rising edge  (event interrupt)
#define HARD_ERR_EVT_INT_MASK   0x0004    // Aurora hard error rising edge  (event interrupt)
#define CHANNEL_DOWN_INT_MASK   0x0008    // Not channel up                 (level interrupt)
#define SOFT_ERR_INT_MASK       0x0010    // Aurora soft error              (level interrupt)
#define HARD_ERR_INT_MASK       0x0020    // Aurora hard error              (level interrupt)
#define GT_PLL_LOSS_INT_MASK    0x0040    // Aurora PLL loss                (level interrupt)

#endif // FUSE_OLINK_INTS_H

