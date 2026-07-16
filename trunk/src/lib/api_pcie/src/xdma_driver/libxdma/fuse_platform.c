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

#include "fuse_platform.h"
#include "fuse_olink.h"
#include <fuse_olink_hw.h>




static int TCA9548_get_port_mask(fuse_olink *olink_drv, UINT8 i2c_addr, UINT8 *port_mask) {
  UINT8 byte;
  int ret;
  KdPrint(("[%s] +\n", __FUNCTION__));
  byte = SET_I2C_CMD(i2c_addr, I2C_RD_CMD);
  ret = fuse_olink_i2c_send_byte(olink_drv, byte, OLINK_I2C_TX_START);
  if (ret != 1) {
    return -1;
  }
  ret = fuse_olink_i2c_recv_byte(olink_drv, &byte, OLINK_I2C_TX_STOP | OLINK_I2C_RX_NACK);
  *port_mask = byte;
  KdPrint(("[%s] -\n", __FUNCTION__));
  return ((ret != 1) ? -1 : 0);
}

static int TCA9548_set_port(fuse_olink *olink_drv, UINT8 i2c_addr, UINT8 port_mask) {
  UINT8 byte;
  int ret;
  KdPrint(("[%s] +\n", __FUNCTION__));
  byte = SET_I2C_CMD(i2c_addr, I2C_WR_CMD);
  ret = fuse_olink_i2c_send_byte(olink_drv, byte, OLINK_I2C_TX_START);
  if (ret != 1) {
    return -1;
  }
  byte = port_mask;
  ret = fuse_olink_i2c_send_byte(olink_drv, byte, OLINK_I2C_TX_STOP);
  KdPrint(("[%s] -\n", __FUNCTION__));
  return ((ret != 1) ? -1 : 0);
}

#define CLK_GEN_I2C_ADDR_0  (0xD0 >> 1)
#define CLK_GEN_I2C_ADDR_1  (0xD4 >> 1)

static int clk_gen_rd_block(fuse_olink *olink_drv, int offset, void *dst, int size) {
  int i;
  UINT8 byte;
  int ret;
  UINT8 i2c_addr;
  UINT8 *p = (UINT8 *)dst;

  i2c_addr = CLK_GEN_I2C_ADDR_1;

  // Envio el comando I2C
  byte = SET_I2C_CMD(i2c_addr, I2C_WR_CMD);
  ret = fuse_olink_i2c_send_byte(olink_drv, byte, OLINK_I2C_TX_START);
  if (ret == 0) {
    // Si falla pruebo con la otra i2c_addr:
    i2c_addr = CLK_GEN_I2C_ADDR_0;
    byte = SET_I2C_CMD(i2c_addr, I2C_WR_CMD);
    ret = fuse_olink_i2c_send_byte(olink_drv, byte, OLINK_I2C_TX_START);
    if (ret == 0) {
      KdPrint(("[%s], i2c_addr error\n", __FUNCTION__));
      return -1;
    }
  }

  // Envio la dirección base (offset):
  byte = (UINT8)(offset & 0xFF);
  ret = fuse_olink_i2c_send_byte(olink_drv, byte, 0);
  if (ret < 0) {
    KdPrint(("[%s], i2c offset error\n", __FUNCTION__));
    return -1;
  }

  // Paso a modo lectura:
  byte = SET_I2C_CMD(i2c_addr, I2C_RD_CMD);
  ret = fuse_olink_i2c_send_byte(olink_drv, byte, OLINK_I2C_TX_START);
  if (ret < 0) {
    KdPrint(("[%s], I2C_RD_CMD error\n", __FUNCTION__));
    return -1;
  }


  for (i = 0; i < size - 1; i++) {
    ret = fuse_olink_i2c_recv_byte(olink_drv, p++, 0);
    if (ret < 0) {
      KdPrint(("[%s], i2c recv data error\n", __FUNCTION__));
      return -1;
    }
  }

  ret = fuse_olink_i2c_recv_byte(olink_drv, p++, OLINK_I2C_TX_STOP | OLINK_I2C_RX_NACK);
  if (ret < 0) {
    KdPrint(("[%s], i2c recv data error\n", __FUNCTION__));
    return -1;
  }

  return size;
}

static int clk_gen_wr_block(fuse_olink *olink_drv, int offset, const void *dst, int size) {
  int i;
  UINT8 byte;
  int ret;
  UINT8 i2c_addr;
  const UINT8 *p = (const UINT8 *)dst;

  i2c_addr = CLK_GEN_I2C_ADDR_1;

  // Envio el comando I2C
  byte = SET_I2C_CMD(i2c_addr, I2C_WR_CMD);
  ret = fuse_olink_i2c_send_byte(olink_drv, byte, OLINK_I2C_TX_START);
  if (ret == 0) {
    // Si falla pruebo con la otra i2c_addr:
    i2c_addr = CLK_GEN_I2C_ADDR_0;
    byte = SET_I2C_CMD(i2c_addr, I2C_WR_CMD);
    ret = fuse_olink_i2c_send_byte(olink_drv, byte, OLINK_I2C_TX_START);
    if (ret == 0) {
      KdPrint(("[%s], i2c_addr error\n", __FUNCTION__));
      return -1;
    }
  }

  // Envio la dirección base (offset):
  byte = (UINT8)(offset & 0xFF);
  ret = fuse_olink_i2c_send_byte(olink_drv, byte, 0);
  if (ret < 0) {
    KdPrint(("[%s], i2c offset error\n", __FUNCTION__));
    return -1;
  }

  // Envío los datos:
  for (i = 0; i < size - 1; i++) {
    ret = fuse_olink_i2c_send_byte(olink_drv, *p++, 0);
    if (ret < 0) {
      KdPrint(("[%s], i2c send data error\n", __FUNCTION__));
      return -1;
    }

  }

  ret = fuse_olink_i2c_send_byte(olink_drv, *p++, OLINK_I2C_TX_STOP);
  if (ret < 0) {
    KdPrint(("[%s], i2c recv data error\n", __FUNCTION__));
    return -1;
  }

  return size;
}

static int clk_gen_wr_reg(fuse_olink *olink_drv, int offset, UINT8 value) {
  UINT8 byte;
  int ret;
  UINT8 i2c_addr;

  i2c_addr = CLK_GEN_I2C_ADDR_1;

  // Envio el comando I2C
  byte = SET_I2C_CMD(i2c_addr, I2C_WR_CMD);
  ret = fuse_olink_i2c_send_byte(olink_drv, byte, OLINK_I2C_TX_START);
  if (ret == 0) {
    // Si falla pruebo con la otra i2c_addr:
    i2c_addr = CLK_GEN_I2C_ADDR_0;
    byte = SET_I2C_CMD(i2c_addr, I2C_WR_CMD);
    ret = fuse_olink_i2c_send_byte(olink_drv, byte, OLINK_I2C_TX_START);
    if (ret == 0) {
      KdPrint(("[%s], i2c_addr error\n", __FUNCTION__));
      return -1;
    }
  }
  // Envio la dirección base (offset):
  byte = (UINT8)(offset & 0xFF);
  ret = fuse_olink_i2c_send_byte(olink_drv, byte, 0);
  if (ret < 0) {
    KdPrint(("[%s], i2c offset error\n", __FUNCTION__));
    return -1;
  }
  // Envio el dato:
  ret = fuse_olink_i2c_send_byte(olink_drv, value, OLINK_I2C_TX_STOP);
  if (ret < 0) {
    KdPrint(("[%s], i2c recv data error\n", __FUNCTION__));
    return -1;
  }

  return 1;
}


static int clk_gen_rd_reg(fuse_olink *olink_drv, int offset, UINT8 *reg) {
  UINT8 byte;
  int ret;
  UINT8 i2c_addr;

  i2c_addr = CLK_GEN_I2C_ADDR_1;

  // Envio el comando I2C
  byte = SET_I2C_CMD(i2c_addr, I2C_WR_CMD);
  ret = fuse_olink_i2c_send_byte(olink_drv, byte, OLINK_I2C_TX_START);
  if (ret == 0) {
    // Si falla pruebo con la otra i2c_addr:
    i2c_addr = CLK_GEN_I2C_ADDR_0;
    byte = SET_I2C_CMD(i2c_addr, I2C_WR_CMD);
    ret = fuse_olink_i2c_send_byte(olink_drv, byte, OLINK_I2C_TX_START);
    if (ret == 0) {
      KdPrint(("[%s], i2c_addr error\n", __FUNCTION__));
      return -1;
    }
  }

  // Envio la dirección base (offset):
  byte = (UINT8)(offset & 0xFF);
  ret = fuse_olink_i2c_send_byte(olink_drv, byte, 0);
  if (ret < 0) {
    KdPrint(("[%s], i2c offset error\n", __FUNCTION__));
    return -1;
  }

  // Paso a modo lectura:
  byte = SET_I2C_CMD(i2c_addr, I2C_RD_CMD);
  ret = fuse_olink_i2c_send_byte(olink_drv, byte, OLINK_I2C_TX_START);
  if (ret < 0) {
    KdPrint(("[%s], I2C_RD_CMD error\n", __FUNCTION__));
    return -1;
  }

  ret = fuse_olink_i2c_recv_byte(olink_drv, reg, OLINK_I2C_TX_STOP | OLINK_I2C_RX_NACK);
  if (ret < 0) {
    KdPrint(("[%s], i2c recv data error\n", __FUNCTION__));
    return -1;
  }

  return 1;
}


#include <idt5901_hw.h>
#include <math.h>

static int idt5901_enable(fuse_olink *olink_drv, int en_mask) {
  UINT8 reg_value;
  UINT8 mask, value;
  int ret;

  // CLK_OS/Shutdown Function (0x69):
  ret = clk_gen_rd_reg(olink_drv, VC5_CLK_OS_SHDN, &reg_value);
  mask = VC5_CLK_OS_SHDN_OS_MASK;
  value = (UINT8) en_mask;
  reg_value &= ~mask;
  reg_value |= value;
  ret = clk_gen_wr_reg(olink_drv, VC5_CLK_OS_SHDN, reg_value);
  return 0;
}

// Nota:
// CLKSEL Polarity can be changed by I2C programming as shown in the table below
//static int idt5901_setup(fuse_olink *olink_drv, double fb_div, double clk1_div) {

static int _idt5901_setup(fuse_olink *olink_drv, int clk_idx, 
                          UINT32 fb_div_int, UINT32 fb_div_fract, 
                          UINT32 clk1_div_int, UINT32 clk1_div_fract) 
{

  UINT32 reg_value;
  UINT32 mask, value;
  UINT8 idt_regs[256];
//  double fract_part, int_part;
  int ret;
  int i;

  KdPrint(("[%s] +\n", __FUNCTION__));
  // Antes de nada desactivo todas las salidas:
  ret = idt5901_enable(olink_drv, 0x00);

  // Leo todos los registros del generador de reloj:
  ret = clk_gen_rd_block(olink_drv, 0, idt_regs, 0x76);

  // Primary Source and Shutdown Register (0x10):
  reg_value = idt_regs[VC5_PRIM_SRC_SHDN];
  mask =  VC5_PRIM_SRC_SHDN_EN_XTAL     |
          VC5_PRIM_SRC_SHDN_EN_CLKIN    | 
          VC5_PRIM_SRC_SHDN_SP          | 
          VC5_PRIM_SRC_SHDN_EN_GBL_SHDN;
  value = VC5_PRIM_SRC_SHDN_EN_XTAL;
          
  reg_value &= ~mask;
  reg_value |= value;
  ret = clk_gen_wr_reg(olink_drv, VC5_PRIM_SRC_SHDN, (UINT8) reg_value);

  // CLK_OE/Shutdown Function (0x68):
  reg_value = idt_regs[VC5_CLK_OE_SHDN];
  mask =  VC5_CLK_OE_SHDN_CLK0 |
          VC5_CLK_OE_SHDN_CLK1 |
          VC5_CLK_OE_SHDN_CLK2 |
          VC5_CLK_OE_SHDN_CLK3 |
          VC5_CLK_OE_SHDN_CLK4;
  value = VC5_CLK_OE_SHDN_CLK(clk_idx+1);
  
  reg_value |= mask;    // Esto va al reves!
  reg_value &= ~value;
  ret = clk_gen_wr_reg(olink_drv, VC5_CLK_OE_SHDN, (UINT8) reg_value);

  // CLK_OS/Shutdown Function (0x69):
#if 0
  reg_value = idt_regs[VC5_CLK_OS_SHDN];
  mask =  VC5_CLK_OS_SHDN_CLK0 |
          VC5_CLK_OS_SHDN_CLK1 |
          VC5_CLK_OS_SHDN_CLK2 |
          VC5_CLK_OS_SHDN_CLK3 |
          VC5_CLK_OS_SHDN_CLK4;
  value = VC5_CLK_OS_SHDN_CLK1;
  
  reg_value &= ~mask;
  reg_value |= value;
  ret = clk_gen_wr_reg(olink_drv, VC5_CLK_OS_SHDN, reg_value);
#endif

  // Crystal X1 Load Capacitor Register (0x12):
  reg_value = idt_regs[VC5_XTAL_X1_LOAD_CAP];

  // Factory Reserved Bits (0x13):
  reg_value = idt_regs[VC5_XTAL_X2_LOAD_CAP];

  // Reference Divider Register (0x15):
  reg_value = idt_regs[VC5_REF_DIVIDER];

  // VCO Control Register and Pre-Divider (0x16):
  reg_value = idt_regs[VC5_VCO_CTRL_AND_PREDIV];

  // VCO Band and Factory Reserved Bits (0x11):
  reg_value = idt_regs[VC5_VCO_BAND];

  //--------------------------------
  
//  UINT32 fb_div_int;
//  UINT32 fb_div_fract;
//  double fb_div_org;

//  fract_part = modf(fb_div, &int_part);
//  fb_div_int = (UINT32)int_part;
//  fb_div_fract = (UINT32)((1U << 24) * fract_part);

  // Feedback Integer Divider Register (0x17):
  reg_value = 0;
  for (i = 0; i < 2; i++) {
    reg_value <<= 8;
    reg_value |= idt_regs[VC5_FEEDBACK_INT_DIV+i];
  }

//  fb_div_org = (reg_value >> 4);

  reg_value &= 0x000F;
  reg_value |= fb_div_int << 4;

  for (i = 1; i >= 0; i--) {
    value = reg_value & 0xFF;
    ret = clk_gen_wr_reg(olink_drv, VC5_FEEDBACK_INT_DIV + i, (UINT8) value);
    reg_value >>= 8;
  }

  // Feedback Fractional Divider Registers (0x19):
  reg_value = 0;
  for (i = 0; i < 3; i++) {
    reg_value <<= 8;
    reg_value |= idt_regs[VC5_FEEDBACK_FRAC_DIV(i)];
  }
//  fb_div_org += (((double)reg_value) / (1U << 24));

  reg_value = fb_div_fract;
  for (i = 2; i >= 0; i--) {
    value = reg_value & 0xFF;
    ret = clk_gen_wr_reg(olink_drv, VC5_FEEDBACK_FRAC_DIV(i), (UINT8) value);
    reg_value >>= 8;
  }
  
  //--------------------------------

//  UINT32 clk1_div_int;
//  UINT32 clk1_div_fract;
//  double clk1_div_org;

//  fract_part = modf(clk1_div, &int_part);
//  clk1_div_int = (UINT32)int_part;
//  clk1_div_fract = (UINT32)((1U << 24) * fract_part);

  // Output Divider 1 Control Register Settings (0x21):
  reg_value = idt_regs[VC5_OUT_DIV_CONTROL(clk_idx)];
  mask =  VC5_OUT_DIV_CONTROL_RESET     |
          VC5_OUT_DIV_CONTROL_SELB_NORM |
          VC5_OUT_DIV_CONTROL_SEL_EXT   |
          VC5_OUT_DIV_CONTROL_INT_MODE  |
          VC5_OUT_DIV_CONTROL_EN_FOD;

  value = VC5_OUT_DIV_CONTROL_RESET | VC5_OUT_DIV_CONTROL_EN_FOD;
  
  reg_value &= ~mask;
  reg_value |= value;
  ret = clk_gen_wr_reg(olink_drv, VC5_OUT_DIV_CONTROL(clk_idx), (UINT8) reg_value);

  // Output Divider 1 Integer Part (0x2D, 0x2E):
  reg_value = 0;
  for (i = 0; i < 2; i++) {
    reg_value <<= 8;
    reg_value |= idt_regs[VC5_OUT_DIV_INT(clk_idx, i)];
  }
//  clk1_div_org = (reg_value >> 4);

  reg_value &= 0x000F;
  reg_value |= clk1_div_int << 4;

  for (i = 1; i >= 0; i--) {
    value = reg_value & 0xFF;
    ret = clk_gen_wr_reg(olink_drv, VC5_OUT_DIV_INT(clk_idx, i), (UINT8) value);
    reg_value >>= 8;
  }

  // Output Divider 1 Fractional Settings (0x22):
  reg_value = 0;
  for (i = 0; i < 4; i++) {
    reg_value <<= 8;
    reg_value |= idt_regs[VC5_OUT_DIV_FRAC(clk_idx, i)];
  }
//  clk1_div_org += (((double)(reg_value >> 2)) / (1U << 24));

  reg_value &= 0x00000001;
  reg_value |= clk1_div_fract << 2;
  //org_value |= VC5_OUT_DIV_FRAC4_OD_SSCE;

  for (i = 3; i >= 0; i--) {
    value = reg_value & 0xFF;
    ret = clk_gen_wr_reg(olink_drv, VC5_OUT_DIV_FRAC(clk_idx, i), (UINT8) value);
    reg_value >>= 8;
  }

  //--------------------------------

  // Clock1 Output Configuration (0x60):
  reg_value = idt_regs[VC5_CLK_OUTPUT_CFG(clk_idx, 0)];
  mask =  VC5_CLK_OUTPUT_CFG0_CFG_MASK  |
          VC5_CLK_OUTPUT_CFG0_PWR_MASK  |
          VC5_CLK_OUTPUT_CFG0_SLEW_MASK;

  value = VC5_CLK_OUTPUT_CFG0_CFG_LVDS  |
          VC5_CLK_OUTPUT_CFG0_PWR_25    |
          VC5_CLK_OUTPUT_CFG0_SLEW_100;

  reg_value &= ~mask;
  reg_value |= value;
  ret = clk_gen_wr_reg(olink_drv, VC5_CLK_OUTPUT_CFG(clk_idx, 0), (UINT8) reg_value);

  // Clock1 Output Configuration (0x61):
  reg_value = idt_regs[VC5_CLK_OUTPUT_CFG(clk_idx, 1)];
  mask = VC5_CLK_OUTPUT_CFG1_EN_CLKBUF;
  value = VC5_CLK_OUTPUT_CFG1_EN_CLKBUF;
  reg_value &= ~mask;
  reg_value |= value;
  ret = clk_gen_wr_reg(olink_drv, VC5_CLK_OUTPUT_CFG(clk_idx, 1), (UINT8) reg_value);

  // Habilito las salidas:
  idt5901_enable(olink_drv, VC5_CLK_OS_SHDN_CLK(clk_idx+1));

  KdPrint(("[%s] -\n", __FUNCTION__));
  return 0;
}

#define VC5_VCO_CALIB   0x1D

static int idt5901_calibrate(fuse_olink *olink_drv) {
  UINT8 reg_value;
  int ret;
  
  KdPrint(("[%s] +\n", __FUNCTION__));
  // CLK_OS/Shutdown Function (0x69):
  ret = clk_gen_rd_reg(olink_drv, VC5_VCO_CALIB, &reg_value);
  reg_value &= ~(1 << 1);
  ret = clk_gen_wr_reg(olink_drv, VC5_VCO_CALIB, reg_value);
  
  ret = clk_gen_rd_reg(olink_drv, VC5_VCO_BAND, &reg_value);
  reg_value &= ~(1 << 5);
  ret = clk_gen_wr_reg(olink_drv, VC5_VCO_BAND, reg_value);

  ret = clk_gen_rd_reg(olink_drv, 0x1C, &reg_value);
  
  reg_value &= ~(1 << 7);
  ret = clk_gen_wr_reg(olink_drv, 0x1C, reg_value);  // writing the bit to 0...
  reg_value |=  (1 << 7);
  ret = clk_gen_wr_reg(olink_drv, 0x1C, reg_value);  // ... then 1 ...
  reg_value &= ~(1 << 7);
  ret = clk_gen_wr_reg(olink_drv, 0x1C, reg_value);  // ... and then back to 0

  fuse_kernel_delay_us(200000);
  KdPrint(("[%s] -\n", __FUNCTION__));
  return 0;
}


#define HTG_K800_BRD_MUX_I2C_ADDR       0x70

#define HTG_K800_CLK_GEN_I2C_MUX_PORT  (7)
#define HTG_K800_CLK_GEN_I2C_MUX_MASK  (1U << HTG_K800_CLK_GEN_I2C_MUX_PORT)

int HTG_K800_init_gt_clk_gen(fuse_olink *olink_drv) {
  int ret;
  UINT8 byte;
  UINT8 port_mask;
  
  KdPrint(("[%s] +\n", __FUNCTION__));

  // Verify arguments
  ret = fuse_olink_is_ready(olink_drv);
  if (ret) {
    // Reset the board I2C mux:
    fuse_olink_set_brd_i2c_reset(olink_drv, 1);
    fuse_kernel_delay_us(1);
    fuse_olink_set_brd_i2c_reset(olink_drv, 0);
    fuse_kernel_delay_us(1);


    port_mask = HTG_K800_CLK_GEN_I2C_MUX_MASK;
    ret = TCA9548_set_port(olink_drv, HTG_K800_BRD_MUX_I2C_ADDR, port_mask);
    ret = TCA9548_get_port_mask(olink_drv, HTG_K800_BRD_MUX_I2C_ADDR, &byte);

    // En el kernel no se puede/debe utilizar coma flotante:
    // https://stackoverflow.com/questions/1583196/building-visual-c-app-that-doesnt-use-crt-functions-still-references-some
    // https://stackoverflow.com/questions/12517429/error-lnk2001-unresolved-external-symbol-fltused-in-wdk
    
    UINT32 fb_div_int, fb_div_fract; 
    UINT32 clk1_div_int, clk1_div_fract;
    
    fb_div_int = 112;
    fb_div_fract = 0;
    clk1_div_int = 7;
    clk1_div_fract = 0;
    
    int clk_idx = 1;
        
    ret = _idt5901_setup( olink_drv, clk_idx,
                          fb_div_int, fb_div_fract, 
                          clk1_div_int, clk1_div_fract);
    
    
    //ret = idt5901_calibrate(olink_drv);
    ret = 0;
  }
  else {
    ret = -1;
  }
  KdPrint(("[%s] -\n", __FUNCTION__));
  return 0;
}
