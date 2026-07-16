#ifndef FUSE_OLINK_HW_H
#define FUSE_OLINK_HW_H

#define OLNK_REGS_OFFSET           (4096*0)  // 4KB

#define OLNK_LANE_0_OFFSET         (4096*0)  // 4KB
#define OLNK_LANE_1_OFFSET         (4096*0)  // 4KB
#define OLNK_LANE_2_OFFSET         (4096*0)  // 4KB
#define OLNK_LANE_3_OFFSET         (4096*0)  // 4KB

#define OPT_LINK_PAGE_SIZE    4096
#define DRP_LANE_0_OFFSET (OPT_LINK_PAGE_SIZE*(1+0))
#define DRP_LANE_1_OFFSET (OPT_LINK_PAGE_SIZE*(1+1))
#define DRP_LANE_2_OFFSET (OPT_LINK_PAGE_SIZE*(1+2))
#define DRP_LANE_3_OFFSET (OPT_LINK_PAGE_SIZE*(1+3))

#define DRP_CH_0_RXCDR_CFG0_IDX 0x000E

//----------------
//-- REGISTERS: --
//----------------

#define OLNK_CTRL_REG_INDEX           0
#define OLNK_VERSION_REG_INDEX        1
#define OLNK_INT_ENABLE_REG_INDEX     2
#define OLNK_INT_FLAG_REG_INDEX       3

#define OLNK_I2C_CTRL_REG_INDEX       4     // Not used!
#define OLNK_I2C_CMD_REG_INDEX        5
#define OLNK_I2C_DATA_REG_INDEX       6

#define OLNK_AURORA_CTRL_REG_INDEX    8
#define OLNK_AURORA_STAT_REG_INDEX    9
#define OLNK_FMC_CTRL_REG_INDEX       10

#define OLNK_HARD_ERR_REG_INDEX       11
#define OLNK_SOFT_ERR_REG_INDEX       12
#define OLNK_CHANNEL_ERR_REG_INDEX    13

//-------------------------
//-- GLOBAL CONTROL REG: --
//-------------------------

#define OLNK_CTRL_ENABLE         0x00000001
#define OLNK_CTRL_INT_ENABLE     0x00000002
#define OLNK_CTRL_SW_RESET       0x00000004
#define OLNK_CTRL_I2C_PORT_RST   0x00000008
#define OLNK_CTRL_BRD_I2C_RST    0x00000010

//----------------------
//-- I2C COMMAND REG: --
//----------------------

#define OLNK_I2C_CMD_TX         0x00000001
#define OLNK_I2C_CMD_START      0x00000002
#define OLNK_I2C_CMD_STOP       0x00000004
#define OLNK_I2C_CMD_RX_NACK    0x00000008
#define OLNK_I2C_CMD_RQT        0x00000010
#define OLNK_I2C_STS_TX_NACK    0x00000020
#define OLNK_I2C_STS_BUS_BUSY   0x00000040
#define OLNK_I2C_STS_ARB_LOST   0x00000080

#define OLNK_I2C_STS_MASK       0x000000E0
#define OLNK_I2C_GET_STATUS(x)  (((x) >> 5) & 0x07)


#define OLNK_I2C_CMD_BUSY       OLNK_I2C_CMD_RQT

//---------------------------
//--- AURORA CONTROL REG: ---
//---------------------------

#define OLNK_AURORA_MGT_RST     0x00000001
#define OLNK_AURORA_CORE_RST    0x00000002
#define OLNK_AURORA_PWR_DWN     0x00000004

#define OLNK_GET_LOOPBACK(x)    (((x) >> 4) & 0x7)
#define OLNK_SET_LOOPBACK(x)    (((x) & 0x7) << 4)
#define OLNK_LOOPBACK_MASK      (0x7 << 4)

#define OLNK_LB_NORM_OP         0x00
#define OLNK_LB_NEAR_END_PCS    0x01
#define OLNK_LB_NEAR_END_PMA    0x02
#define OLNK_LB_FAR_END_PMA     0x04
#define OLNK_LB_FAR_END_PCS     0x06

//--------------------------
//--- AURORA STATUS REG: ---
//--------------------------

#define OLNK_GET_LANE_STATUS(x) ((x) & 0xFF)
#define OLNK_AURORA_CHANNEL_UP  0x00000100
#define OLNK_AURORA_SOFT_ERR    0x00000200
#define OLNK_AURORA_HARD_ERR    0x00000400
#define OLNK_AURORA_GT_PLL_LOCK 0x00000800

//------------------------
//--- FMC CONTROL REG: ---
//------------------------

#define OLNK_FMC_I2C_RST        0x00000001
#define OLNK_FMC_CLK_GEN_RST    0x00000002
#define OLNK_FMC_CLK_GEN_LOS    0x00000004
#define OLNK_FMC_CLK_GEN_LOL    0x00000008
#define OLNK_FMC_CLK_GEN_INT    0x00000010

#define OLNK_FMC_QSFP_1_LPMode  0x00000100
#define OLNK_FMC_QSFP_1_Modsel  0x00000200
#define OLNK_FMC_QSFP_1_Reset   0x00000400
#define OLNK_FMC_QSFP_1_Int     0x00000800
#define OLNK_FMC_QSFP_1_ModPrs  0x00001000

#endif  // FUSE_OLINK_HW_H
