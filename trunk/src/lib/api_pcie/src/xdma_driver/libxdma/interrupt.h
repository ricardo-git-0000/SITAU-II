#pragma once

// ========================= include dependencies =================================================
#include <ntddk.h>
#include "dma_engine.h"

// ========================= declarations =================================================

#define XDMA_MAX_USER_IRQ (16)
#define XDMA_MAX_NUM_IRQ (XDMA_MAX_USER_IRQ + XDMA_MAX_CHAN_IRQ)

//typedef void(*fuseISR_t)(XDMA_DEVICE *xdma, void *syncObj, fuse_flags_t flags);


typedef struct _IRQ_CONTEXT {
  ULONG eventId;
  UINT32 channelIrqPending; // channel irq that have fired
  UINT32 userIrqPending;    // user event irq that have fired
  
  UINT32 fuseUserFlags[XDMA_MAX_USER_IRQ];  // RMG: Para pasar al DPC los eventos fuse
  //fuseISR_t *fuseUserISR;
  
  XDMA_ENGINE *engine;            // RMG: Esto solo se usa para MSIX & MSI
  volatile XDMA_IRQ_REGS* regs;
  PXDMA_DEVICE xdma;
} IRQ_CONTEXT, *PIRQ_CONTEXT;
WDF_DECLARE_CONTEXT_TYPE_WITH_NAME(IRQ_CONTEXT, GetIrqContext)


/// Initialize the interrupt resources given by the OS for use by DMA engines and user events 
NTSTATUS SetupInterrupts(IN PXDMA_DEVICE xdma,
                         IN OUT ULONG *userMax,
                         IN OUT ULONG *h2cChannelMax,
                         IN OUT ULONG *c2hChannelMax,
                         IN WDFCMRESLIST ResourcesRaw,
                         IN WDFCMRESLIST ResourcesTranslated);

