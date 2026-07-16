#pragma once

// ========================= include dependencies =================================================

#include <ntddk.h>
#include "reg.h"
#include "dma_engine.h"
#include "interrupt.h"


// FUSE devices:
#include "fuse_common.h"
#include "fuse_emul.h"
#include "fuse_dma.h"
#include "fuse_olink.h"

#define TIMEOUT_1_US  (10)                  // en unidades de 100 ns
#define TIMEOUT_1_MS  (1000*TIMEOUT_1_US)   // en unidades de 100 ns

// ========================= constants ============================================================

#define XDMA_MAX_NUM_BARS (3)

// ========================= type declarations ====================================================

//typedef struct {
//  unsigned int flags;
//  void *syncObj;
//} fuseEvent;

typedef void (*fuseEventCB_t)(ULONG eventId, void *syncObj, fuse_flags_t flags);

typedef struct FUSE_XDMA_EVENT_T {
  int           event_ID;
  fuseEventCB_t userCB;   // user callback 
  void          *syncObj; // Syncronization object (KEVENT)
  fuse_flags_t  flags;    // interrupt flags (payload for the event)
  LONG          cancel;
  WDFINTERRUPT  _irq;     // wdf interrupt handle (no se usa!!!)
} FUSE_XDMA_EVENT;



/// Callback function type for user-defined work to be executed on receiving a user event.
typedef void(*PFN_XDMA_USER_WORK)(ULONG eventId, void* userData);

/// user event interrupt context
typedef struct XDMA_EVENT_T {
  PFN_XDMA_USER_WORK work;  // user callback 
  void *userData;           // custom user data. will be passed into work callback function
  WDFINTERRUPT irq;         // wdf interrupt handle
} XDMA_EVENT;

/// The XDMA device context
typedef struct XDMA_DEVICE_T {

  // WDF 
  WDFDEVICE wdfDevice;

  // PCIe BAR access
  UINT numBars;
  PVOID bar[XDMA_MAX_NUM_BARS]; // kernel virtual address of BAR
  ULONG barLength[XDMA_MAX_NUM_BARS];
  ULONG configBarIdx;
  LONG userBarIdx;
  LONG bypassBarIdx;
  volatile XDMA_CONFIG_REGS *configRegs;
  volatile XDMA_IRQ_REGS *interruptRegs;
  volatile XDMA_SGDMA_COMMON_REGS * sgdmaRegs;

  // DMA Engine management
  XDMA_ENGINE engines[XDMA_MAX_NUM_CHANNELS][XDMA_NUM_DIRECTIONS];
  WDFDMAENABLER dmaEnabler;   // WDF DMA Enabler for the engine queues

  // Interrupt Resources
  WDFINTERRUPT lineInterrupt;
  WDFINTERRUPT channelInterrupts[XDMA_MAX_CHAN_IRQ];

  // user events
  XDMA_EVENT userEvents[XDMA_MAX_USER_IRQ];
  // FUSE user events
  FUSE_XDMA_EVENT fuseUserEvents[XDMA_MAX_USER_IRQ];
  int max_n_buffers;

  // RMG:
  UCHAR   RevisionID;
  USHORT  SubVendorID;
  USHORT  SubSystemID;

  // FUSE devices:
  BOOLEAN FUSE_valid_design;
  UINT32  FUSE_version;
  BOOLEAN FUSE_has_emul_module;

  fuse_olink  olink_drv;
  fuse_dma    fuse_dma_drv;
  fuse_emul   emul_drv;

  // available user and channel interrupts
  ULONG userMax;
  ULONG h2cChannelMax;
  ULONG c2hChannelMax;

} XDMA_DEVICE, *PXDMA_DEVICE;

#define OLINK_DEV_ID      0x01
#define FUSE_DMA_DEV_ID   0x02
#define EMUL_DEV_ID       0x03

// ========================= function declarations ================================================


