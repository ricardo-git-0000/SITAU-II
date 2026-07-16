//---------------------------------------------------------------------------

#ifndef stfp_infoH
#define stfp_infoH                                                   
//---------------------------------------------------------------------------
#if defined(_MSC_VER)
   #include <vector>
#else
   #include <vector.h>
   #include <system.hpp>
#endif

using namespace std;

#include "c_log_file.h"

//---------------------------------------------------------------------------

#define _VERSION_DLL_MAJOR 	    "1"
#define _VERSION_DLL_MINOR 	    "0"
#define _VERSION_DLL_RELEASE    "23"
#define _VERSION_DLL_BUILD 	    "0"
#define _VERSION_DLL_DATE 		"28/04/2026"

#define NUM_ENCODER 4

#define i_USB_1_1             0
#define i_USB_2_0             1

#define i_OS_WIN32            1

#define i_BW_AIR              1

#define SIZE_VERSION_DLL        50
#define SIZE_SPEED_USB_DLL      50
#define SIZE_NAME_BIT           50
#define SIZE_FPGA_BIT           50
#define SIZE_DATE_BIT           50
#define SIZE_TIME_BIT           50

#define SYSTEM_TYPE_PA	0
#define SYSTEM_TYPE_MCH 1
#define SYSTEM_TYPE_MIX 2

typedef struct TInfoChannel {
    int ch;
    int base;
    int mod;
    int ch_mod;
} TInfoChannel;

typedef struct TInfoFPGA {
    char NameBIT[SIZE_NAME_BIT];
    char FPGABIT[SIZE_FPGA_BIT];
    char DateBIT[SIZE_DATE_BIT];
    char TimeBIT[SIZE_TIME_BIT];
} TInfoFPGA;

typedef struct TInfo {
    char VersionDLL[SIZE_VERSION_DLL];
    char SpeedUSBDLL[SIZE_SPEED_USB_DLL];
    TInfoFPGA UCI;
    TInfoFPGA MOD;
    TInfoFPGA BASE;
} TInfo;

typedef union T_HWVersion {
   unsigned long REG;
   struct {
      unsigned short RELEASE : 8; // Bit[0:7]
      unsigned short MINOR   : 8; // Bit[8:15]
      unsigned short MAJOR   : 8; // Bit[16:23]
      unsigned short BRANCH  : 8; // Bit[24:31]
   } BIT;
} T_HWVersion;

extern vector<TInfoChannel> gb_element_to_channel;
extern vector<unsigned short> gb_pin_to_channel;

#endif
