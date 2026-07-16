/*
* XDMA Driver public API
* ===============================
*
* Copyright 2018 Xilinx Inc.
* Copyright 2010-2012 Sidebranch
* Copyright 2010-2012 Leon Woestenberg <leon@sidebranch.com>
*
* Maintainer:
* -----------
* Alexander Hornburg <alexander.hornburg@xilinx.com>
*
* Description:
* ------------
* This is a sample driver for the Xilinx Inc. 'DMA/Bridge Subsystem for PCIe v3.0' (XDMA) IP.
*
*/

#ifndef __XDMA_WINDOWS_H__
#define __XDMA_WINDOWS_H__

// 74c7e4a9-6d5d-4a70-bc0d-20691dff9e9d
DEFINE_GUID(GUID_DEVINTERFACE_XDMA,
            0x74c7e4a9, 0x6d5d, 0x4a70, 0xbc, 0x0d, 0x20, 0x69, 0x1d, 0xff, 0x9e, 0x9d);

#define XDMA_FILE_USER      L"\\user"
#define XDMA_FILE_CONTROL   L"\\control"
#define XDMA_FILE_BYPASS    L"\\bypass"

#define XDMA_FILE_EVENT_0   L"\\event_0"
#define XDMA_FILE_EVENT_1   L"\\event_1"
#define XDMA_FILE_EVENT_2   L"\\event_2"
#define XDMA_FILE_EVENT_3   L"\\event_3"
#define XDMA_FILE_EVENT_4   L"\\event_4"
#define XDMA_FILE_EVENT_5   L"\\event_5"
#define XDMA_FILE_EVENT_6   L"\\event_6"
#define XDMA_FILE_EVENT_7   L"\\event_7"
#define XDMA_FILE_EVENT_8   L"\\event_8"
#define XDMA_FILE_EVENT_9   L"\\event_9"
#define XDMA_FILE_EVENT_10  L"\\event_10"
#define XDMA_FILE_EVENT_11  L"\\event_11"
#define XDMA_FILE_EVENT_12  L"\\event_12"
#define XDMA_FILE_EVENT_13  L"\\event_13"
#define XDMA_FILE_EVENT_14  L"\\event_14"
#define XDMA_FILE_EVENT_15  L"\\event_15"

#define XDMA_FILE_H2C_0     L"\\h2c_0"
#define XDMA_FILE_H2C_1     L"\\h2c_1"
#define XDMA_FILE_H2C_2     L"\\h2c_2"
#define XDMA_FILE_H2C_3     L"\\h2c_3"

#define XDMA_FILE_C2H_0     L"\\c2h_0"
#define XDMA_FILE_C2H_1     L"\\c2h_1"
#define XDMA_FILE_C2H_2     L"\\c2h_2"
#define XDMA_FILE_C2H_3     L"\\c2h_3"

#define XDMA_IOCTL(index) CTL_CODE(FILE_DEVICE_UNKNOWN, index, METHOD_BUFFERED, FILE_ANY_ACCESS)

// Original Xilinx services:
#define IOCTL_XDMA_GET_VERSION            XDMA_IOCTL(0x00)
#define IOCTL_XDMA_PERF_START             XDMA_IOCTL(0x01)
#define IOCTL_XDMA_PERF_STOP              XDMA_IOCTL(0x02)
#define IOCTL_XDMA_PERF_GET               XDMA_IOCTL(0x03)
#define IOCTL_XDMA_ADDRMODE_GET           XDMA_IOCTL(0x04)
#define IOCTL_XDMA_ADDRMODE_SET           XDMA_IOCTL(0x05)
#define IOCTL_WRITE_KEYHOLE_REGISTER      XDMA_IOCTL(0x06)
#define IOCTL_MAP_BAR                     XDMA_IOCTL(0x07)

// Fuse common services:
#define IOCTL_FUSE_GET_PCI_INFO           XDMA_IOCTL(0x10)
#define IOCTL_FUSE_GET_REVISION_ID        XDMA_IOCTL(0x11)
#define IOCTL_FUSE_GET_SUBSYSTEM_ID       XDMA_IOCTL(0x12)
#define IOCTL_FUSE_GET_FILE_TIMEOUT       XDMA_IOCTL(0x13)
#define IOCTL_FUSE_SET_FILE_TIMEOUT       XDMA_IOCTL(0x14)

#define IOCTL_FUSE_CANCEL_USR_EVENT       XDMA_IOCTL(0x15)
#define IOCTL_FUSE_CANCEL_DMA_EVENT       XDMA_IOCTL(0x16)

// Optical link services:
#define IOCTL_FUSE_OLINK_GET_SIGNATURE    XDMA_IOCTL(0x20)
#define IOCTL_FUSE_OLINK_RESET_HW         XDMA_IOCTL(0x21)
#define IOCTL_FUSE_OLINK_ENABLE           XDMA_IOCTL(0x22)
#define IOCTL_FUSE_OLINK_ENABLE_EVENTS    XDMA_IOCTL(0x23)
#define IOCTL_FUSE_OLINK_GET_CONN_STATUS  XDMA_IOCTL(0x24)
#define IOCTL_FUSE_OLINK_I2C_SEND_BYTE    XDMA_IOCTL(0x25)
#define IOCTL_FUSE_OLINK_I2C_RECV_BYTE    XDMA_IOCTL(0x26)
#define IOCTL_FUSE_OLINK_I2C_RST_BRD_MUX  XDMA_IOCTL(0x27)

// Fuse DMA services:
#define IOCTL_FUSE_DMA_GET_SIGNATURE      XDMA_IOCTL(0x40)
#define IOCTL_FUSE_DMA_RESET_HW           XDMA_IOCTL(0x41)
#define IOCTL_FUSE_DMA_ENABLE             XDMA_IOCTL(0x42)
#define IOCTL_FUSE_DMA_ENABLE_EVENTS      XDMA_IOCTL(0x43)
#define IOCTL_FUSE_DMA_FLUSH              XDMA_IOCTL(0x44)
#define IOCTL_FUSE_DMA_GET_FILLING_LEVEL  XDMA_IOCTL(0x45)
#define IOCTL_FUSE_DMA_GET_FILLING_THRS   XDMA_IOCTL(0x46)
#define IOCTL_FUSE_DMA_SET_FILLING_THRS   XDMA_IOCTL(0x47)
#define IOCTL_FUSE_DMA_GET_IPERF_DATA     XDMA_IOCTL(0x48)
#define IOCTL_FUSE_DMA_GET_IMAGE_ID_SEED  XDMA_IOCTL(0x49)
#define IOCTL_FUSE_DMA_SET_IMAGE_ID_SEED  XDMA_IOCTL(0x4A)
#define IOCTL_FUSE_DMA_GET_MEM_SIZE       XDMA_IOCTL(0x4B)
#define IOCTL_FUSE_DMA_DNA_OK             XDMA_IOCTL(0x4C)

// Fuse emulator services:
#define IOCTL_FUSE_EMUL_GET_SIGNATURE     XDMA_IOCTL(0x60)
#define IOCTL_FUSE_EMUL_RESET_HW          XDMA_IOCTL(0x61)
#define IOCTL_FUSE_EMUL_ENABLE            XDMA_IOCTL(0x62)
#define IOCTL_FUSE_EMUL_ENABLE_EVENTS     XDMA_IOCTL(0x63)
#define IOCTL_FUSE_EMUL_GENERATE          XDMA_IOCTL(0x64)
#define IOCTL_FUSE_EMUL_STOP_CONT         XDMA_IOCTL(0x65)



//#define IOCTL_FUSE_GET_DMA_TIMEOUT        XDMA_IOCTL(0xB)
//#define IOCTL_FUSE_SET_DMA_TIMEOUT        XDMA_IOCTL(0xC)
//#define IOCTL_FUSE_GET_EVENT_TIMEOUT      XDMA_IOCTL(0xD)
//#define IOCTL_FUSE_SET_EVENT_TIMEOUT      XDMA_IOCTL(0xE)



// structure for IOCTL_XDMA_PERF_GET
typedef struct {
  UINT64 clockCycleCount;
  UINT64 dataCycleCount;
  UINT64 pendingCount;
} XDMA_PERF_DATA;

// structure for IOCTL_FUSE_EMUL_GENERATE
typedef struct {
  UINT32 n_images;
  UINT32 frame_length;
  UINT32 image_length;
  UINT32 gap;
  UINT32 mode;
} _fuse_emul_conf;

// structure for IOCTL_FUSE_DMA_GET_IPERF_DATA
typedef struct {
  UINT32 total_cycles;
  UINT32 active_cycles;
} fuse_perf_data;



#define FUSE_DEVELOPMENT_SUBSYSTEM_ID   0xCAFE
#define FUSE_PRODUCTION_SUBSYSTEM_ID    0xBEEF
#define FUSE_HTG_K800_SUBSYSTEM_ID      0xF45E
#define FUSE_AXKU041_SUBSYSTEM_ID       0xA1FA

#define FUSE_INFINITE_TIMEOUT (~0ULL)


#include "fuse_hw_img.h"

// Beta, Discontinued, Hidden, Pre - Production, Production, Removed, Superseded

#define _FUSE_I2C_START       0x0002
#define _FUSE_I2C_STOP        0x0004
#define _FUSE_I2C_NACK        0x0008

#define _FUSE_DMA_RST_CORE		  0x0001
#define _FUSE_DMA_RST_MEM		    0x0002

#define _FUSE_DMA_RST_CFG       0x0100
#define _FUSE_DMA_RST_OP_SET    0x0200
#define _FUSE_DMA_RST_OP_CLR    0x0000

//#define _FUSE_DMA_RST_OP(x)     ((x) & 0x01) << 9)

//#define _FUSE_DMA_RST_CLR		    0x0400


typedef struct {
  UINT32 trf_attr;
  UINT8  byte;
} _fuse_i2c_cmd;

typedef struct {
  UINT32 thr_l;
  UINT32 thr_h;
} level_thr_t;

typedef struct {
  UINT16 VendorID;
  UINT16 DeviceID;
  UINT16 SubVendorID;
  UINT16 SubSystemID;
  UINT8 RevisionID;
} pci_info_t;

/* For use on user bar repeatedly writing to a given register one 32 bit value at a time
// ptrAddr - the pointer to the data you want to write e.g. an array
// size - the number of 32 bit words you would like to write to the register
// offset the address of the register

example use
-----------
const ULONG size = 16;
ULONG AXI_ADDRESS_EXAMPLE = 0x88;
alignas(32) ULONG wr_buffer[size];

DWORD nb;
XDMA_KEYHOLE_DATA kholeData;
kholeData.offset = AXI_ADDRESS_EXAMPLE;
kholeData.ptrAddr = wr_buffer;
kholeData.size = size;
BOOL success = DeviceIoControl(user, IOCTL_WRITE_KEYHOLE_REGISTER, &kholeData, sizeof(XDMA_KEYHOLE_DATA), nullptr, 0, &nb, NULL);
if (!success) {
  throw std::runtime_error("ioctl failed!" + GetLastError());
}

*/
typedef struct {
  PULONG ptrAddr;
  ULONG size;
  ULONG offset;
} XDMA_KEYHOLE_DATA, *PXDMA_KEYHOLE_DATA;


typedef struct {
  PVOID mappedAddress;
  ULONG barLength;
} XDMA_BAR_INFO, *PXDMA_BAR_INFO;

#endif/*__XDMA_WINDOWS_H__*/

