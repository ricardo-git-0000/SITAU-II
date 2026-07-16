#ifndef FUSE_EMUL_HW_H
#define FUSE_EMUL_HW_H

//----------------
//-- REGISTERS: --
//----------------

#define EMUL_CTRL_REG_INDEX         0
#define EMUL_VERSION_REG_INDEX      1
#define EMUL_INT_ENABLE_REG_INDEX   2
#define EMUL_INT_FLAG_REG_INDEX     3

#define EMUL_TX_CMD_REG_INDEX       4
#define EMUL_FRAME_LEN_REG_INDEX    5
#define EMUL_IMAGE_LEN_REG_INDEX    6
#define EMUL_GAP_LEN_REG_INDEX      7

#define EMUL_TOTAL_TIME_REG_INDEX   8
#define EMUL_ACTIVE_TIME_REG_INDEX  9

#define EMUL_FAULT_INJECT_REG_INDEX 10
#define EMUL_TGT_FRAME_REG_INDEX    11
#define EMUL_TGT_IMAGE_REG_INDEX    12


#define EMUL_F_CLK  (250.0e6)

//-------------------------
//-- GLOBAL CONTROL REG: --
//-------------------------

#define EMUL_CTRL_ENABLE        0x00000001
#define EMUL_CTRL_INT_ENABLE    0x00000002
#define EMUL_CTRL_SW_RESET      0x00000004
#define EMUL_CTRL_CONT_MODE     0x00000010

//---------------------
//-- TX COMMAND REG: --
//---------------------

#define EMUL_CMD_TX_RQT         0x80000000
#define EMUL_CMD_TX_BUSY        EMUL_CMD_TX_RQT
#define EMUL_N_IMG_BITS         16
#define EMUL_MAX_N_IMG          ((1U << EMUL_N_IMG_BITS)-1)
#define EMUL_SET_TX_CMD(n)      (((n) & EMUL_MAX_N_IMG) | EMUL_CMD_TX_RQT)


#define EMUL_BEAT_CWIDTH    16
#define EMUL_PACKET_CWIDTH  10
#define EMUL_GAP_CWIDTH     24

#define EMUL_MAX_FRAME_LEN  (1U << EMUL_BEAT_CWIDTH)
#define EMUL_MAX_N_PACKETS  (1U << EMUL_PACKET_CWIDTH)
#define EMUL_MAX_GAP_LEN    ((1U << EMUL_GAP_CWIDTH)-1)


//-----------------------
//-- FAULT INJECT REG: --
//-----------------------

#define SET_FAULT_INJECTION(frame_adjust, image_adjust, inject_err) ( \
            (((frame_adjust) & 0xF) << 0) | \
            (((image_adjust) & 0xF) << 4) | \
            (((inject_err) & 0xF) << 8) )
            
#define INJECT_FRAME_ERR    0x01
#define INJECT_IMAGE_ERR    0x02


#endif  // FUSE_EMUL_HW_H
