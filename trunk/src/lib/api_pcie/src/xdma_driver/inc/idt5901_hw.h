#ifndef IDT5901_HW_H
#define IDT5901_HW_H

#define BIT(n)    (1UL << (n))

//#define GENMASK(h, L) (((~0UL) - (1UL << (L)) + 1) & (~0UL >> (32 - 1 - (h))))
#define GENMASK(L, R) (((~0UL) - (1UL << (R)) + 1) & (~0UL >> (32 - 1 - (L))))


#define VC5_LVPECL    0
#define VC5_CMOS      1
#define VC5_HCSL33    2
#define VC5_LVDS      3
#define VC5_CMOS2     4
#define VC5_CMOSD     5
#define VC5_HCSL25    6

// VersaClock5 registers
#define VC5_OTP_CONTROL               0x00
                                      
// Factory-reserved register block    
#define VC5_RSVD_DEVICE_ID            0x01
#define VC5_RSVD_ADC_GAIN_7_0         0x02
#define VC5_RSVD_ADC_GAIN_15_8        0x03
#define VC5_RSVD_ADC_OFFSET_7_0       0x04
#define VC5_RSVD_ADC_OFFSET_15_8      0x05
#define VC5_RSVD_TEMPY                0x06
#define VC5_RSVD_OFFSET_TBIN          0x07
#define VC5_RSVD_GAIN                 0x08
#define VC5_RSVD_TEST_NP              0x09
#define VC5_RSVD_UNUSED               0x0a
#define VC5_RSVD_BANDGAP_TRIM_UP      0x0b
#define VC5_RSVD_BANDGAP_TRIM_DN      0x0c
#define VC5_RSVD_CLK_R_12_CLK_AMP_4   0x0d
#define VC5_RSVD_CLK_R_34_CLK_AMP_4   0x0e
#define VC5_RSVD_CLK_AMP_123          0x0f
                                      
// Configuration register block
#define VC5_PRIM_SRC_SHDN                       0x10
#define VC5_PRIM_SRC_SHDN_EN_XTAL               BIT(7)
#define VC5_PRIM_SRC_SHDN_EN_CLKIN              BIT(6)
#define VC5_PRIM_SRC_SHDN_EN_DOUBLE_XTAL_FREQ   BIT(3)
#define VC5_PRIM_SRC_SHDN_SP                    BIT(1)
#define VC5_PRIM_SRC_SHDN_EN_GBL_SHDN           BIT(0)

#define VC5_VCO_BAND                    0x11
#define VC5_XTAL_X1_LOAD_CAP            0x12
#define VC5_XTAL_X2_LOAD_CAP            0x13
#define VC5_REF_DIVIDER                 0x15
#define VC5_REF_DIVIDER_SEL_PREDIV2     BIT(7)
#define VC5_REF_DIVIDER_REF_DIV(n)      ((n) & 0x3f)

#define VC5_VCO_CTRL_AND_PREDIV                 0x16
#define VC5_VCO_CTRL_AND_PREDIV_BYPASS_PREDIV   BIT(7)

#define VC5_FEEDBACK_INT_DIV          0x17
#define VC5_FEEDBACK_INT_DIV_BITS     0x18
#define VC5_FEEDBACK_FRAC_DIV(n)      (0x19 + (n))
#define VC5_RC_CONTROL0               0x1e
#define VC5_RC_CONTROL1               0x1f

// CLOCK CONFIGURATIONS (CLK1-CLK4):
// ---------------------------------

// These registers are named 'Unused Factory Reserved Registers'
#define VC5_RESERVED_X0(idx)            (0x20 + ((idx) * 0x10))
#define VC5_RESERVED_X0_BYPASS_SYNC     BIT(7)  // bypass_sync<idx> bit

// Output divider control for divider 1,2,3,4
#define VC5_OUT_DIV_CONTROL(idx)        (0x21 + ((idx) * 0x10))
#define VC5_OUT_DIV_CONTROL_RESET       BIT(7)
#define VC5_OUT_DIV_CONTROL_SELB_NORM   BIT(3)
#define VC5_OUT_DIV_CONTROL_SEL_EXT     BIT(2)
#define VC5_OUT_DIV_CONTROL_INT_MODE    BIT(1)
#define VC5_OUT_DIV_CONTROL_EN_FOD      BIT(0)

#define VC5_OUT_DIV_FRAC(idx, n)        (0x22 + ((idx) * 0x10) + (n))
#define VC5_OUT_DIV_FRAC4_OD_SSCE       BIT(1)

#define VC5_OUT_DIV_STEP_SPREAD(idx, n) (0x26 + ((idx) * 0x10) + (n))
#define VC5_OUT_DIV_SPREAD_MOD(idx, n)  (0x29 + ((idx) * 0x10) + (n))
#define VC5_OUT_DIV_SKEW_INT(idx, n)    (0x2b + ((idx) * 0x10) + (n))
#define VC5_OUT_DIV_INT(idx, n)         (0x2d + ((idx) * 0x10) + (n))
#define VC5_OUT_DIV_SKEW_FRAC(idx)      (0x2f + ((idx) * 0x10))

// OUTPUT CONFIGURATIONS:
// ----------------------

// Clock control register for clock 1,2
#define VC5_CLK_OUTPUT_CFG(idx, n)      (0x60 + ((idx) * 0x2) + (n))
#define VC5_CLK_OUTPUT_CFG0_CFG_SHIFT   5
#define VC5_CLK_OUTPUT_CFG0_CFG_MASK    GENMASK(7, VC5_CLK_OUTPUT_CFG0_CFG_SHIFT)

#define VC5_CLK_OUTPUT_CFG0_CFG_LVPECL  (VC5_LVPECL << VC5_CLK_OUTPUT_CFG0_CFG_SHIFT)
#define VC5_CLK_OUTPUT_CFG0_CFG_CMOS    (VC5_CMOS   << VC5_CLK_OUTPUT_CFG0_CFG_SHIFT)
#define VC5_CLK_OUTPUT_CFG0_CFG_HCSL33  (VC5_HCSL33 << VC5_CLK_OUTPUT_CFG0_CFG_SHIFT)
#define VC5_CLK_OUTPUT_CFG0_CFG_LVDS    (VC5_LVDS   << VC5_CLK_OUTPUT_CFG0_CFG_SHIFT)
#define VC5_CLK_OUTPUT_CFG0_CFG_CMOS2   (VC5_CMOS2  << VC5_CLK_OUTPUT_CFG0_CFG_SHIFT)
#define VC5_CLK_OUTPUT_CFG0_CFG_CMOSD   (VC5_CMOSD  << VC5_CLK_OUTPUT_CFG0_CFG_SHIFT)
#define VC5_CLK_OUTPUT_CFG0_CFG_HCSL25  (VC5_HCSL25 << VC5_CLK_OUTPUT_CFG0_CFG_SHIFT)

#define VC5_CLK_OUTPUT_CFG0_PWR_SHIFT   3
#define VC5_CLK_OUTPUT_CFG0_PWR_MASK    GENMASK(4, VC5_CLK_OUTPUT_CFG0_PWR_SHIFT)
#define VC5_CLK_OUTPUT_CFG0_PWR_18      (0<<VC5_CLK_OUTPUT_CFG0_PWR_SHIFT)
#define VC5_CLK_OUTPUT_CFG0_PWR_25      (2<<VC5_CLK_OUTPUT_CFG0_PWR_SHIFT)
#define VC5_CLK_OUTPUT_CFG0_PWR_33      (3<<VC5_CLK_OUTPUT_CFG0_PWR_SHIFT)
#define VC5_CLK_OUTPUT_CFG0_SLEW_SHIFT  0
#define VC5_CLK_OUTPUT_CFG0_SLEW_MASK   GENMASK(1, VC5_CLK_OUTPUT_CFG0_SLEW_SHIFT)
#define VC5_CLK_OUTPUT_CFG0_SLEW_80     (0<<VC5_CLK_OUTPUT_CFG0_SLEW_SHIFT)
#define VC5_CLK_OUTPUT_CFG0_SLEW_85     (1<<VC5_CLK_OUTPUT_CFG0_SLEW_SHIFT)
#define VC5_CLK_OUTPUT_CFG0_SLEW_90     (2<<VC5_CLK_OUTPUT_CFG0_SLEW_SHIFT)
#define VC5_CLK_OUTPUT_CFG0_SLEW_100    (3<<VC5_CLK_OUTPUT_CFG0_SLEW_SHIFT)
#define VC5_CLK_OUTPUT_CFG1_EN_CLKBUF   BIT(0)

#define VC5_CLK_OE_SHDN        0x68
#define VC5_CLK_OS_SHDN        0x69

#define VC5_GLOBAL_REGISTER               0x76
#define VC5_GLOBAL_REGISTER_GLOBAL_RESET  BIT(5)

// The minimum VCO frequency is 2.5 GHz. The maximum is variant specific.
#define VC5_PLL_VCO_MIN        2500000000UL

// VC5 Input mux settings
#define VC5_MUX_IN_XIN    BIT(0)
#define VC5_MUX_IN_CLKIN  BIT(1)

// Maximum number of clk_out supported by this driver
#define VC5_MAX_CLK_OUT_NUM  5

// Maximum number of FODs supported by this driver
#define VC5_MAX_FOD_NUM  4

// flags to describe chip features

#define VC5_HAS_INTERNAL_XTAL     BIT(0)    // chip has built-in oscilator
#define VC5_HAS_PFD_FREQ_DBL      BIT(1)    // chip has PFD requency doubler
#define VC5_HAS_BYPASS_SYNC_BIT   BIT(2)    // chip has bits to disable FOD sync


#define VC5_CLK_OS_SHDN_OS_SHIFT   3
#define VC5_CLK_OS_SHDN_OS_MASK    GENMASK(7, VC5_CLK_OS_SHDN_OS_SHIFT)

#define VC5_CLK_OS_SHDN_CLK0    0x80U
#define VC5_CLK_OS_SHDN_CLK1    0x40U
#define VC5_CLK_OS_SHDN_CLK2    0x20U
#define VC5_CLK_OS_SHDN_CLK3    0x10U
#define VC5_CLK_OS_SHDN_CLK4    0x08U

#define VC5_CLK_OS_SHDN_CLK(n)  (((n) < VC5_MAX_CLK_OUT_NUM)? (1U << (7-(n))) : 0)

#define VC5_CLK_OE_SHDN_OS_SHIFT   3
#define VC5_CLK_OE_SHDN_OS_MASK    GENMASK(7, VC5_CLK_OE_SHDN_OS_SHIFT)

#define VC5_CLK_OE_SHDN_CLK0    0x80U
#define VC5_CLK_OE_SHDN_CLK1    0x40U
#define VC5_CLK_OE_SHDN_CLK2    0x20U
#define VC5_CLK_OE_SHDN_CLK3    0x10U
#define VC5_CLK_OE_SHDN_CLK4    0x08U

#define VC5_CLK_OE_SHDN_CLK(n)  (((n) < VC5_MAX_CLK_OUT_NUM)? (1U << (7-(n))) : 0)

#endif  // IDT5901_HW_H
