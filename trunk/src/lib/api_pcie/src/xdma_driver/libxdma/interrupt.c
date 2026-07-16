// ========================= include dependencies =================================================
#include "device.h"
#include "dma_engine.h"
#include "interrupt.h"
#include "pcie_common.h"

#include "trace.h"
#ifdef DBG
// The trace message header (.tmh) file must be included in a source file before any WPP macro 
// calls and after defining a WPP_CONTROL_GUIDS macro (defined in trace.h). see trace.h
#include "interrupt.tmh"
#endif

#define FUSE_ERR_LEVEL 2
#include "fuse_debug.h"

#ifdef FUSE_VERBOSE
  #undef FUSE_VERBOSE
#endif

#ifdef FUSE_VERBOSE
  #define dbg_printf(x)   KdPrint(x)
#else
  #define dbg_printf(x)
#endif


// FUSE wrappers:
static int fuse_isr(PIRQ_CONTEXT irq, UINT32 irq_mask);


// ====================== declarations ============================================================

// RMG: gestión de legacy interrupts (both user & channels):
EVT_WDF_INTERRUPT_ISR        EvtInterruptIsr;
EVT_WDF_INTERRUPT_DPC        EvtInterruptDpc;
EVT_WDF_INTERRUPT_ENABLE     EvtInterruptEnable;
EVT_WDF_INTERRUPT_DISABLE    EvtInterruptDisable;

// RMG: gestión de MSIX & MSI interrupts: channels
EVT_WDF_INTERRUPT_ISR        EvtChannelInterruptIsr;
EVT_WDF_INTERRUPT_DPC        EvtChannelInterruptDpc;
EVT_WDF_INTERRUPT_ENABLE     EvtChannelInterruptEnable;
EVT_WDF_INTERRUPT_DISABLE    EvtChannelInterruptDisable;

// RMG: gestión de MSIX & MSI interrupts: user
EVT_WDF_INTERRUPT_ISR        EvtUserInterruptIsr;
EVT_WDF_INTERRUPT_DPC        EvtUserInterruptDpc;
EVT_WDF_INTERRUPT_ENABLE     EvtUserInterruptEnable;
EVT_WDF_INTERRUPT_DISABLE    EvtUserInterruptDisable;

// ====================== static setup functions =================================================

static UINT32 BuildVectorReg(UINT32 a, UINT32 b, UINT32 c, UINT32 d) {
  UINT32 reg_val = 0;
  reg_val |= (a & 0x1f) << 0;
  reg_val |= (b & 0x1f) << 8;
  reg_val |= (c & 0x1f) << 16;
  reg_val |= (d & 0x1f) << 24;
  return reg_val;
}

// RMG: Configuración de las interrupciones de usuario para MSIX & MSI
static NTSTATUS SetupUserInterrupt(IN PXDMA_DEVICE xdma, IN ULONG index,
                                   IN PCM_PARTIAL_RESOURCE_DESCRIPTOR resource,
                                   IN PCM_PARTIAL_RESOURCE_DESCRIPTOR translatedResource) {
  WDF_INTERRUPT_CONFIG config;
  WDF_INTERRUPT_CONFIG_INIT(&config, EvtUserInterruptIsr, EvtUserInterruptDpc);
  config.InterruptRaw = resource;
  config.InterruptTranslated = translatedResource;
  config.EvtInterruptEnable = EvtUserInterruptEnable;
  config.EvtInterruptDisable = EvtUserInterruptDisable;

  WDF_OBJECT_ATTRIBUTES attribs;
  WDF_OBJECT_ATTRIBUTES_INIT(&attribs);
  WDF_OBJECT_ATTRIBUTES_INIT_CONTEXT_TYPE(&attribs, IRQ_CONTEXT);

  NTSTATUS status = WdfInterruptCreate(xdma->wdfDevice, 
                                       &config, 
                                       &attribs, 
                                       &(xdma->userEvents[index].irq));

  if (!NT_SUCCESS(status)) {
    TraceError(DBG_INIT, "WdfInterruptCreate failed: %!STATUS!", status);
  }

  PIRQ_CONTEXT irqContext = GetIrqContext(xdma->userEvents[index].irq);
  irqContext->eventId = index; // msg Id = irq index = event id
  irqContext->regs = xdma->interruptRegs;
  irqContext->xdma = xdma;
  return status;
}

// RMG: Configuración de las interrupciones de DMA (channels) para MSIX & MSI
static NTSTATUS SetupChannelInterrupt(IN PXDMA_DEVICE xdma, IN ULONG index,
                                      IN PCM_PARTIAL_RESOURCE_DESCRIPTOR resource,
                                      IN PCM_PARTIAL_RESOURCE_DESCRIPTOR translatedResource) {
  WDF_INTERRUPT_CONFIG config;
  WDF_INTERRUPT_CONFIG_INIT(&config, EvtChannelInterruptIsr, EvtChannelInterruptDpc);
  config.InterruptRaw = resource;
  config.InterruptTranslated = translatedResource;
  config.EvtInterruptEnable = EvtChannelInterruptEnable;
  config.EvtInterruptDisable = EvtChannelInterruptDisable;

  WDF_OBJECT_ATTRIBUTES attribs;
  WDF_OBJECT_ATTRIBUTES_INIT(&attribs);
  WDF_OBJECT_ATTRIBUTES_INIT_CONTEXT_TYPE(&attribs, IRQ_CONTEXT);

  NTSTATUS status = WdfInterruptCreate(xdma->wdfDevice, &config, &attribs,
                     &(xdma->channelInterrupts[index]));
  if (!NT_SUCCESS(status)) {
    TraceError(DBG_INIT, "WdfInterruptCreate failed: %!STATUS!", status);
  }
  PIRQ_CONTEXT irqContext = GetIrqContext(xdma->channelInterrupts[index]);
  irqContext->regs = xdma->interruptRegs;
  irqContext->xdma = xdma;
  return status;
}

// Estas dos funciones se utilizan para gestionar legacy interrupts (ni MSIX ni MSI):
static NTSTATUS SetupDeviceInterrupt(IN PXDMA_DEVICE xdma, IN PCM_PARTIAL_RESOURCE_DESCRIPTOR resource,
                                     IN PCM_PARTIAL_RESOURCE_DESCRIPTOR translatedResource) {
  WDF_INTERRUPT_CONFIG config;
  WDF_INTERRUPT_CONFIG_INIT(&config, EvtInterruptIsr, EvtInterruptDpc);
  config.InterruptRaw = resource;
  config.InterruptTranslated = translatedResource;
  config.EvtInterruptEnable = EvtInterruptEnable;
  config.EvtInterruptDisable = EvtInterruptDisable;

  WDF_OBJECT_ATTRIBUTES attribs;
  WDF_OBJECT_ATTRIBUTES_INIT(&attribs);
  WDF_OBJECT_ATTRIBUTES_INIT_CONTEXT_TYPE(&attribs, IRQ_CONTEXT);

  NTSTATUS status = WdfInterruptCreate(xdma->wdfDevice, &config, &attribs, &xdma->lineInterrupt);
  if (!NT_SUCCESS(status)) {
    TraceError(DBG_INIT, "WdfInterruptCreate failed: %!STATUS!", status);
  }
  PIRQ_CONTEXT irqContext = GetIrqContext(xdma->lineInterrupt);
  irqContext->xdma = xdma;
  irqContext->regs = xdma->interruptRegs;
  return status;
}

static NTSTATUS SetupSingleInterrupt(IN PXDMA_DEVICE xdma, IN WDFCMRESLIST ResourcesRaw,
                                     IN WDFCMRESLIST ResourcesTranslated) {
  PCM_PARTIAL_RESOURCE_DESCRIPTOR resource;
  PCM_PARTIAL_RESOURCE_DESCRIPTOR resourceRaw;
  NTSTATUS status = STATUS_INTERNAL_ERROR;
  ULONG numResources = WdfCmResourceListGetCount(ResourcesTranslated);
  UINT32 vectorValue = 0;

dbg_printf(("RMG: [%s] +\n", __FUNCTION__));
  ASSERT(xdma->interruptRegs != NULL);

  for (UINT i = 0; i < numResources; i++) {
    resource = WdfCmResourceListGetDescriptor(ResourcesTranslated, i);
    resourceRaw = WdfCmResourceListGetDescriptor(ResourcesRaw, i);

    if (resource->Type != CmResourceTypeInterrupt) {
      continue;
    }

    status = SetupDeviceInterrupt(xdma, resourceRaw, resource);
    if (!NT_SUCCESS(status)) {
      TraceError(DBG_INIT, "Error in setup device interrupt: %!STATUS!", status);
      return status;
    }

    if (!(resource->Flags & CM_RESOURCE_INTERRUPT_MESSAGE)) { // LINE interrupt

      status = GetLineInterruptPin(xdma->wdfDevice, &vectorValue); // todo is this required?
      if (!NT_SUCCESS(status)) {
        TraceError(DBG_INIT, "GetLineInterruptPin failed! %!STATUS!", status);
        return status;
      }

      // Windows: A=1, B=2, C=3, D=4
      // XDMA:  A=0, B=1, C=2, D=3
      vectorValue--;
    }

    status = STATUS_SUCCESS;
    break;
  }

  TraceVerbose(DBG_INIT, "User/Channel interrupt vector value = %u", vectorValue);

  xdma->interruptRegs->userVector[0] = BuildVectorReg(vectorValue, vectorValue, vectorValue, vectorValue);
  xdma->interruptRegs->userVector[1] = BuildVectorReg(vectorValue, vectorValue, vectorValue, vectorValue);
  xdma->interruptRegs->userVector[2] = BuildVectorReg(vectorValue, vectorValue, vectorValue, vectorValue);
  xdma->interruptRegs->userVector[3] = BuildVectorReg(vectorValue, vectorValue, vectorValue, vectorValue);

  xdma->interruptRegs->channelVector[0] = BuildVectorReg(vectorValue, vectorValue, vectorValue, vectorValue);
  xdma->interruptRegs->channelVector[1] = BuildVectorReg(vectorValue, vectorValue, vectorValue, vectorValue);

dbg_printf(("RMG: [%s] -\n", __FUNCTION__));
  return status;
}

static void programInterrupts(IN PXDMA_DEVICE xdma, IN ULONG h2cChannels, IN ULONG c2hChannels) {

    UINT j;
    ULONG i = 0;

    for (j = 0; i < xdma->userMax; ++j) {
        UINT32 val = 0;
        UINT k;
        UINT shift = 0;

        for (k = 0; k < 4 && i < xdma->userMax; ++i, ++k, shift += 8) {
            val |= (i & 0x1F) << shift;
        }

        xdma->interruptRegs->userVector[j] = val;
    }

    UINT vec = xdma->userMax;
    UINT channelMax = h2cChannels + c2hChannels;

    for (j = 0, i = 0; i < channelMax; ++j) {
        UINT32 val = 0;
        UINT k;
        UINT shift = 0;

        for (k = 0; k < 4 && i < channelMax; ++i, ++k, shift += 8) {
            if (i < xdma->h2cChannelMax) {
                val |= (vec & 0x1F) << shift;
                ++vec;
            }
            if (i >= h2cChannels && i < (h2cChannels + xdma->c2hChannelMax)) {
                val |= (vec & 0x1F) << shift;
                ++vec;
            }
        }

        xdma->interruptRegs->channelVector[j] = val;
    }
}

static NTSTATUS SetupMsixInterrupts(IN PXDMA_DEVICE xdma, IN WDFCMRESLIST ResourcesRaw,
                                    IN WDFCMRESLIST ResourcesTranslated) {
  PCM_PARTIAL_RESOURCE_DESCRIPTOR resource;
  PCM_PARTIAL_RESOURCE_DESCRIPTOR resourceRaw;
  NTSTATUS status = STATUS_SUCCESS;
  ULONG numResources = WdfCmResourceListGetCount(ResourcesTranslated);
  ULONG interruptCount = 0;
  ULONG totRequiredIrqs = xdma->userMax + xdma->h2cChannelMax + xdma->c2hChannelMax;

dbg_printf(("RMG: [%s] +\n", __FUNCTION__));
  ASSERT(xdma->interruptRegs != NULL);

  for (UINT i = 0; (i < numResources) && (interruptCount < totRequiredIrqs); i++) {
    resource = WdfCmResourceListGetDescriptor(ResourcesTranslated, i);
    resourceRaw = WdfCmResourceListGetDescriptor(ResourcesRaw, i);

    if (resource->Type != CmResourceTypeInterrupt) {
      continue;
    }

    // assign first available interrupt resources to user events
    if (interruptCount < xdma->userMax) {
      status = SetupUserInterrupt(xdma, interruptCount, resourceRaw, resource);
    }
    else { // assign next interrupt resources to dma engines
      ULONG index = interruptCount - xdma->userMax;
      status = SetupChannelInterrupt(xdma, index, resourceRaw, resource);
    }
    if (!NT_SUCCESS(status)) {
      TraceError(DBG_INIT, "Error in setup device interrupt: %!STATUS!", status);
      return status;
    }

    ++interruptCount;
  }

dbg_printf(("RMG: [%s] -\n", __FUNCTION__));
  return status;
}

static NTSTATUS SetupMultiMsiInterrupts(IN PXDMA_DEVICE xdma, IN WDFCMRESLIST ResourcesRaw,
                                        IN WDFCMRESLIST ResourcesTranslated, IN USHORT numVectors) {
  PCM_PARTIAL_RESOURCE_DESCRIPTOR resource;
  PCM_PARTIAL_RESOURCE_DESCRIPTOR resourceRaw;
  NTSTATUS status = STATUS_INTERNAL_ERROR;
  ULONG numResources = WdfCmResourceListGetCount(ResourcesTranslated);

dbg_printf(("RMG: [%s] +\n", __FUNCTION__));
  ASSERT(xdma->interruptRegs != NULL);

  for (UINT i = 0; i < numResources; i++) {
    resource = WdfCmResourceListGetDescriptor(ResourcesTranslated, i);
    resourceRaw = WdfCmResourceListGetDescriptor(ResourcesRaw, i);

    if (resource->Type != CmResourceTypeInterrupt) {
      continue;
    }
    resource->u.MessageInterrupt.Raw.MessageCount = numVectors;
    resourceRaw->u.MessageInterrupt.Raw.MessageCount = numVectors;

    // individual resource/msgId for each user interrupt (0-userMax)
    for (ULONG n = 0; n < xdma->userMax; n++) {
      status = SetupUserInterrupt(xdma, n, resourceRaw, resource);
      if (!NT_SUCCESS(status)) {
        TraceError(DBG_INIT, "Error in setup user interrupt: %!STATUS!", status);
        return status;
      }
    }

    // individual resource/msgId for each channel interrupt (H2C 0-N and C2H 0-N)
    for (ULONG n = 0; n < (xdma->h2cChannelMax + xdma->c2hChannelMax); n++) {
      status = SetupChannelInterrupt(xdma, n, resourceRaw, resource);
      if (!NT_SUCCESS(status)) {
        TraceError(DBG_INIT, "Error in setup channel interrupt: %!STATUS!", status);
        return status;
      }
    }

    status = STATUS_SUCCESS;
    break;
  }

dbg_printf(("RMG: [%s] -\n", __FUNCTION__));
  return status;
}

static NTSTATUS CountInterruptResources(IN WDFCMRESLIST ResourcesTranslated,
                                        OUT PULONG numInterruptResources) {
    const ULONG numResources = WdfCmResourceListGetCount(ResourcesTranslated);
    TraceVerbose(DBG_INIT, "# PCIe resources = %d", numResources);

    for (UINT i = 0; i < numResources; i++) {
        PCM_PARTIAL_RESOURCE_DESCRIPTOR resource = WdfCmResourceListGetDescriptor(ResourcesTranslated, i);
        if (!resource) {
            TraceError(DBG_INIT, "WdfResourceCmGetDescriptor() fails");
            return STATUS_DEVICE_CONFIGURATION_ERROR;
        }

        if (resource->Type == CmResourceTypeInterrupt) {
            (*numInterruptResources)++;
        }
    }
    return STATUS_SUCCESS;
}

// ====================== line/msi interrupt callback functions ===================================

NTSTATUS EvtInterruptEnable(IN WDFINTERRUPT Interrupt, IN WDFDEVICE device) {
    UNREFERENCED_PARAMETER(device);
    IRQ_CONTEXT* irq = GetIrqContext(Interrupt);
    EXPECT(irq != NULL);
    EXPECT(irq->regs != NULL);
    irq->regs->channelIntEnableW1S = 0xFFFFFFFFUL;
    irq->regs->userIntEnableW1S = 0xFFFFFFFFUL;
    TraceVerbose(DBG_IRQ, "enabled ALL interrupts");
    return STATUS_SUCCESS;
}

NTSTATUS EvtInterruptDisable(IN WDFINTERRUPT Interrupt, IN WDFDEVICE device) {
    UNREFERENCED_PARAMETER(device);
    IRQ_CONTEXT* irq = GetIrqContext(Interrupt);
    EXPECT(irq != NULL);
    EXPECT(irq->regs != NULL);
    irq->regs->channelIntEnableW1C = 0xFFFFFFFFUL;
    irq->regs->userIntEnableW1C = 0xFFFFFFFFUL;
    TraceVerbose(DBG_IRQ, "disabled ALL interrupts");
    return STATUS_SUCCESS;
}

// RMG: (legacy interrupts) callback function to service the interrupt at DIRQL
// interrupt service routine - handle line interrupts
BOOLEAN EvtInterruptIsr(IN WDFINTERRUPT Interrupt, IN ULONG MessageID) {
  PIRQ_CONTEXT irq = GetIrqContext(Interrupt);
  UINT32 chIrq = 0;
  UINT32 userIrq = 0;
  
  dbg_printf(("RMG: [%s] +\n", __FUNCTION__));
  TraceInfo(DBG_IRQ, "irq messageId = %u", MessageID);

  EXPECT(irq != NULL);
  EXPECT(irq->regs != NULL);
  
  // RMG:
  // 1. Determino los elementos solicitantes de interrupción mediante la 
  //    lectura del registro del controlador PCIe
  // 2. Los anoto en el campo de interrupciones pendientes (channelIrqPending/
  //    userIrqPending) de IRQ_CONTEXT. Posteriormente los procesaré en el
  //    bottom half o Deferred Procedure Call DPC (función EvtInterruptDpc)
  // 3. Una vez anotados borro los flags en el controlador PCIe

  // read channel interrupt request registers
  // channel interrupt(s) requested?
  chIrq = irq->regs->channelIntRequest;   // RMG: IRQ Block Channel Interrupt Request
  TraceVerbose(DBG_IRQ, "chan EN=0x%08X RQ=0x%08X PE=0x%08X",
    irq->regs->channelIntEnable, irq->regs->channelIntRequest, irq->regs->channelIntPending);

dbg_printf(("   - irq channel regs: pending=0x%08X, enable=0x%08X => request=0x%08X\n", 
  irq->regs->channelIntPending, irq->regs->channelIntEnable, irq->regs->channelIntRequest));      
    
  // RMG: Aquí evaluamos qué canales DMA que han pedido interrupción:
  if (chIrq) {
    irq->channelIrqPending |= chIrq;        // remember fired channel interrupts...
    irq->regs->channelIntEnableW1C = chIrq; // ...disable fired channel interrupts
    
    //int channel = 0;
    //int dir = C2H;
    //XDMA_ENGINE *engine = &irq->xdma->engines[channel][dir];
    dbg_printf(("   - C2H0 status = 0x%8.8X\n", irq->xdma->engines[0][C2H].regs->status));
  }

  // read user interrupts that are pending in the controller - flushes previous write
  // user interrupt(s) requested?
  userIrq = irq->regs->userIntRequest;    // RMG: IRQ Block User Interrupt Request
  TraceVerbose(DBG_IRQ, "user EN=0x%08X RQ=0x%08X PE=0x%08X",
    irq->regs->userIntEnable, irq->regs->userIntRequest, irq->regs->userIntPending);
  
  if (userIrq) {
    //KdPrint(("RMG: [%s] llamando a fuse_isr, userIrq = 0x%8.8X\n", __FUNCTION__, userIrq));
    fuse_isr(irq, userIrq);                 // procesamos las interrupciones de usuario
    irq->userIrqPending |= userIrq;         // remember fired user interrupts
    irq->regs->userIntEnableW1C = userIrq;  // disable fired user interrupts
  }

  // was the interrupt handled correctly?
  if (!chIrq && !userIrq) {
    TraceWarning(DBG_IRQ, "Spurious interrupt");
    return FALSE;
  }

  // schedule deferred work
  WdfInterruptQueueDpcForIsr(Interrupt);
  dbg_printf(("RMG: [%s] -\n", __FUNCTION__));
  return TRUE;
}

// RMG: (legacy interrupts) callback function if the driver requires additional 
// interrupt-servicing tasks at DISPATCH_LEVEL

// Deferred interrupt service handler
VOID EvtInterruptDpc(IN WDFINTERRUPT interrupt, IN WDFOBJECT device) {
  UNREFERENCED_PARAMETER(device);
  PIRQ_CONTEXT irq = GetIrqContext(interrupt);

  dbg_printf(("RMG: [%s] +\n", __FUNCTION__));
  
  // dma engine interrupt pending?
  TraceVerbose(DBG_IRQ, "channelIrqPending=0x%08X", irq->channelIrqPending);
  for (UINT dir = H2C; dir < 2; dir++) { // 0=H2C, 1=C2H
    for (UINT channel = 0; channel < XDMA_MAX_NUM_CHANNELS; channel++) {
      XDMA_ENGINE *engine = &irq->xdma->engines[channel][dir];
      if (engine->enabled && (irq->channelIrqPending & engine->irqBitMask)) {
        TraceInfo(DBG_IRQ, "%s_%u servicing interrupt", DirectionToString(dir), channel);
dbg_printf(("   - %s_%u interrupt DPC: status = 0x%8.8X\n", DirectionToString(dir), channel, engine->regs->status));      
        ASSERT(engine->work != NULL);
        // RMG: Se llama al user handler (work). Este handler se ha registrado
        // previamente desde la función EngineCreate() (dma_engine.c).
        // Se pueden utilizar dos posibles handlers:
        // - EngineProcessRing() para canales stream C2H
        // - EngineProcessTransfer() para los restantes casos.
        engine->work(engine);
      }
    }
  }

  // user event interrupt pending?
  TraceVerbose(DBG_IRQ, "userIrqPending=0x%08X", irq->userIrqPending);
  for (UINT i = 0; i < XDMA_MAX_USER_IRQ; ++i) {
    // RMG: Si tenemos un user handler (work) lo llamamos.
    // Esto lo han registrado con XDMA_UserIsrRegister() (interrupt.c)
    // que se llama desde EvtDevicePrepareHardware() (driver.c)
#ifdef USE_FUSE_INTS  // DPC!!!
    if (irq->userIrqPending & BIT_N(i)) {
      FUSE_XDMA_EVENT *fuseUserEvent = &irq->xdma->fuseUserEvents[i];
      if (fuseUserEvent->userCB != NULL) {
        //KdPrint(("RMG: [%s] llamando al fuseUserCB de la usr_irq %d con flags = 0x%8.8X\n", __FUNCTION__, i, irq->fuseUserFlags[i]));
        // Actualizamos el campo de fuseUserEvent que contiene los flags...
        fuseUserEvent->flags = irq->fuseUserFlags[i];
        // Llamamos al callback de usuario: FUSE_HandleUserEvent()
        fuseUserEvent->userCB(i, fuseUserEvent->syncObj, irq->fuseUserFlags[i]);
//        WdfInterruptAcquireLock(interrupt);
        irq->fuseUserFlags[i] = 0;
//        WdfInterruptReleaseLock(interrupt);
      }
    }
#else
    XDMA_EVENT *userEvent = &irq->xdma->userEvents[i];
    if (irq->userIrqPending & BIT_N(i)) {
      if (userEvent->work != NULL) {
        userEvent->work(i, userEvent->userData);
      }
    }
#endif
  }

  // re-enable interrupts
  WdfInterruptAcquireLock(interrupt);
  irq->regs->channelIntEnableW1S = irq->channelIrqPending;  // !!!!!!
  irq->channelIrqPending = 0x0;

  // FIXME - Remove the user interrupt source condition before reenabling the user interrupt
  // This depends on user logic and how the interrupt has been triggered!
  // This reference driver puts the responsibility on the user-space application to remove the 
  // user event interrupt source condition.

  // RMG: Esto es para interrupciones de usuario:
  // Vuelvo a habilitar las interrupciones que previamente he desactiado
  // y borro los flags de interrupciones pendientes!!!
  irq->regs->userIntEnableW1S = irq->userIrqPending;
  irq->userIrqPending = 0x0;
  WdfInterruptReleaseLock(interrupt);
  
  
  TraceVerbose(DBG_IRQ, "channel EN=0x%08X RQ=0x%08X PE=0x%08X",
         irq->regs->channelIntEnable, irq->regs->channelIntRequest, irq->regs->channelIntPending);
  TraceVerbose(DBG_IRQ, "user EN=0x%08X RQ=0x%08X PE=0x%08X",
         irq->regs->userIntEnable, irq->regs->userIntRequest, irq->regs->userIntPending);
         
  dbg_printf(("RMG: [%s] -\n", __FUNCTION__));
  return;
}

// ====================== MSI-X interrupt callback functions ======================================

// RMG: Funciones para gestionar las interrupciones DMA (channel) (MSIX & MSI):
// ----------------------------------------------------------------------------

// RMG: (MSIX & MSI) Habilitación de interrupciones DMA (channel)
NTSTATUS EvtChannelInterruptEnable(IN WDFINTERRUPT Interrupt, IN WDFDEVICE device) {
  UNREFERENCED_PARAMETER(device);
  PIRQ_CONTEXT irq = GetIrqContext(Interrupt);
  EXPECT(irq != NULL);
  EXPECT(irq->regs != NULL);
  EngineEnableInterrupt(irq->engine);
  return STATUS_SUCCESS;
}

// RMG: (MSIX & MSI) Inhabilitación de interrupciones DMA (channel)
NTSTATUS EvtChannelInterruptDisable(IN WDFINTERRUPT Interrupt, IN WDFDEVICE device) {
  UNREFERENCED_PARAMETER(device);
  PIRQ_CONTEXT irq = GetIrqContext(Interrupt);
  EXPECT(irq != NULL);
  EXPECT(irq->regs != NULL);
  EngineDisableInterrupt(irq->engine);
  return STATUS_SUCCESS;
}

// RMG: (MSIX & MSI) callback function to service the interrupt at DIRQL

// interrupt service routine - handles msi/msi-x interrupts only
BOOLEAN EvtChannelInterruptIsr(IN WDFINTERRUPT Interrupt, IN ULONG MessageID) {
  PIRQ_CONTEXT irq = GetIrqContext(Interrupt);

  EXPECT(irq != NULL);

  TraceVerbose(DBG_IRQ, "%s_%u interrupt occurred! messageId=%u",
    DirectionToString(irq->engine->dir), irq->engine->channel, MessageID);

  EngineDisableInterrupt(irq->engine);

  return WdfInterruptQueueDpcForIsr(Interrupt);   // schedule deferred work
}

// RMG: (MSIX & MSI) callback function if the driver requires additional 
// interrupt-servicing tasks at DISPATCH_LEVEL

// Deferred interrupt service handler
VOID EvtChannelInterruptDpc(IN WDFINTERRUPT interrupt, IN WDFOBJECT device) {
  UNREFERENCED_PARAMETER(device);
  PIRQ_CONTEXT irq = GetIrqContext(interrupt);

  // do engine specific work (either EngineProcessTransfer (MM) or EngineProcessRing (ST))
  irq->engine->work(irq->engine);
}

// RMG: Funciones para gestionar las interrupciones de usuario (MSIX & MSI):
// -------------------------------------------------------------------------

// RMG: (MSIX & MSI) Habilitación de interrupciones de usuario
NTSTATUS EvtUserInterruptEnable(IN WDFINTERRUPT Interrupt, IN WDFDEVICE device) {
  UNREFERENCED_PARAMETER(device);
  PIRQ_CONTEXT irq = GetIrqContext(Interrupt);
  EXPECT(irq != NULL);
  EXPECT(irq->regs != NULL);
  irq->regs->userIntEnableW1S = BIT_N(irq->eventId); // message id and event id are same
  TraceInfo(DBG_IRQ, "event_%u enabled interrupt", irq->eventId);
  return STATUS_SUCCESS;
}

// RMG: (MSIX & MSI) Inhabilitación de interrupciones de usuario
NTSTATUS EvtUserInterruptDisable(IN WDFINTERRUPT Interrupt, IN WDFDEVICE device) {
  UNREFERENCED_PARAMETER(device);
  PIRQ_CONTEXT irq = GetIrqContext(Interrupt);
  EXPECT(irq != NULL);
  EXPECT(irq->regs != NULL);
  irq->regs->userIntEnableW1C = BIT_N(irq->eventId); // message id and event id are same
  TraceInfo(DBG_IRQ, "event_%u disabled interrupt", irq->eventId);
  return STATUS_SUCCESS;
}

// RMG: (MSIX & MSI) callback function to service the interrupt at DIRQL
BOOLEAN EvtUserInterruptIsr(IN WDFINTERRUPT Interrupt, IN ULONG MessageID) {
  TraceInfo(DBG_IRQ, "event_%u occurred!", MessageID);
  IRQ_CONTEXT* irq = GetIrqContext(Interrupt);
  EXPECT(irq != NULL);
  EXPECT(irq->regs != NULL);
  // disable user event interrupt
  irq->regs->userIntEnableW1C = BIT_N(MessageID); // message id and event id are same
  return WdfInterruptQueueDpcForIsr(Interrupt);   // schedule deferred work;
}

// RMG: (MSIX & MSI) callback function if the driver requires additional 
// interrupt-servicing tasks at DISPATCH_LEVEL

// Deferred interrupt service handler
VOID EvtUserInterruptDpc(IN WDFINTERRUPT interrupt, IN WDFOBJECT device) {
  UNREFERENCED_PARAMETER(device);
  IRQ_CONTEXT* irq = GetIrqContext(interrupt);
  EXPECT(irq != NULL);
  XDMA_EVENT* userEvent = &irq->xdma->userEvents[irq->eventId];
  EXPECT(userEvent != NULL);

  dbg_printf(("RMG: [%s] +\n", __FUNCTION__));

  // message id and event id are same
  if (userEvent->work != NULL) {
    TraceInfo(DBG_IRQ, "event_%d executing work handler", irq->eventId);
    userEvent->work(irq->eventId, userEvent->userData);
  }
  dbg_printf(("RMG: [%s] -\n", __FUNCTION__));
  
  // reenable interrupt
  WdfInterruptAcquireLock(interrupt);
  irq->regs->userIntEnableW1S = BIT_N(irq->eventId);
  WdfInterruptReleaseLock(interrupt);
}

// ====================== internal api implementation ==============================================

NTSTATUS SetupInterrupts(IN PXDMA_DEVICE xdma,
                         IN OUT ULONG *userMax,
                         IN OUT ULONG *h2cChannelMax,
                         IN OUT ULONG *c2hChannelMax,
                         IN WDFCMRESLIST ResourcesRaw,
                         IN WDFCMRESLIST ResourcesTranslated) 
{
  NTSTATUS status = STATUS_SUCCESS;
  ULONG totRequiredIrqs = 0;
  ULONG numIrqResources = 0;
  USHORT numMsiVectors = 0;
  USHORT numMsixVectors = 0;

  if (NULL == userMax || NULL == h2cChannelMax || NULL == c2hChannelMax) {
    KdPrint(("RMG: [%s] NULL pointers!!!\n", __FUNCTION__));
    return STATUS_INVALID_PARAMETER;
  }

  //  dbg_printf(("RMG: [%s] h2cChannelMax = %d, c2hChannelMax = %d, userMax = %d\n", 
  //            __FUNCTION__, *h2cChannelMax, *c2hChannelMax, *userMax));
  
  xdma->userMax = *userMax;
  xdma->h2cChannelMax = *h2cChannelMax;
  xdma->c2hChannelMax = *c2hChannelMax;

  // Aquí es donde determino el número real de canales:
  CountChannels(xdma, h2cChannelMax, c2hChannelMax);
  
  TraceVerbose(DBG_INIT, "Found h2c channels=%u, c2h channels=%u", *h2cChannelMax, *c2hChannelMax);
  dbg_printf(("RMG: [%s] Found h2c channels=%u, c2h channels=%u\n",__FUNCTION__, *h2cChannelMax, *c2hChannelMax));

  if (xdma->userMax == 0 || xdma->userMax > XDMA_MAX_USER_IRQ)
    xdma->userMax = XDMA_MAX_USER_IRQ;
  if (xdma->h2cChannelMax == 0 || xdma->h2cChannelMax > XDMA_MAX_NUM_CHANNELS)
    xdma->h2cChannelMax = XDMA_MAX_NUM_CHANNELS;
  if (xdma->c2hChannelMax == 0 || xdma->c2hChannelMax > XDMA_MAX_NUM_CHANNELS)
    xdma->c2hChannelMax = XDMA_MAX_NUM_CHANNELS;

  if (xdma->h2cChannelMax > *h2cChannelMax) {
    dbg_printf(("RMG: [%s] h2cChannelMax overrange\n", __FUNCTION__));
    dbg_printf(("   - xdma->h2cChannelMax = %d, *h2cChannelMax = %d\n", xdma->h2cChannelMax, *h2cChannelMax));
    dbg_printf(("   - xdma->c2hChannelMax = %d, *c2hChannelMax = %d\n", xdma->c2hChannelMax, *c2hChannelMax));
    return STATUS_INVALID_PARAMETER;
  }
  if (xdma->c2hChannelMax > *c2hChannelMax) {
    dbg_printf(("RMG: [%s] c2hChannelMax overrange\n", __FUNCTION__));
    return STATUS_INVALID_PARAMETER;
  }

  status = GetNumMsixVectors(xdma->wdfDevice, &numMsixVectors);
  if (!NT_SUCCESS(status)) {
    //dbg_printf(("RMG: [%s] GetNumMsixVectors failed: %d\n", __FUNCTION__, status));
    TraceError(DBG_INIT, "GetNumMsixVectors failed: %!STATUS!", status);
  }
  status = GetNumMsiVectors(xdma->wdfDevice, &numMsiVectors);
  if (!NT_SUCCESS(status)) {
    //dbg_printf(("RMG: [%s] GetNumMsiVectors failed: %d\n", __FUNCTION__, status));
    TraceError(DBG_INIT, "GetNumMsiVectors failed: %!STATUS!", status);
  }
  status = CountInterruptResources(ResourcesTranslated, &numIrqResources);
  if (!NT_SUCCESS(status)) {
    //dbg_printf(("RMG: [%s] CountInterruptResources failed: %d\n", __FUNCTION__, status));
    TraceError(DBG_INIT, "CountInterruptResources failed: %!STATUS!", status);
  }

  //dbg_printf(("RMG: [%s] numIrqResources=%u, MSIX=%u, MSI=%u\n", __FUNCTION__, numIrqResources, numMsixVectors, numMsiVectors));
  TraceVerbose(DBG_INIT, "numIrqResources=%u, MSIX=%u, MSI=%u", numIrqResources, numMsixVectors, numMsiVectors);

  totRequiredIrqs = xdma->userMax + xdma->h2cChannelMax + xdma->c2hChannelMax;

  if (numMsixVectors >= totRequiredIrqs) { /* MSIX */
    //dbg_printf(("RMG: [%s] using MSIX!!!\n", __FUNCTION__));
    status = SetupMsixInterrupts(xdma, ResourcesRaw, ResourcesTranslated);
    programInterrupts(xdma, *h2cChannelMax, *c2hChannelMax);
  } else if (numMsiVectors >= totRequiredIrqs) { /* Multi MSI */
    //dbg_printf(("RMG: [%s] using Multi MSI!!!\n", __FUNCTION__));
    status = SetupMultiMsiInterrupts(xdma, ResourcesRaw, ResourcesTranslated, numMsiVectors);
    programInterrupts(xdma, *h2cChannelMax, *c2hChannelMax);
  } else if (numIrqResources != 0) {
    //dbg_printf(("RMG: [%s] using single interrupt!!!\n", __FUNCTION__));
    status = SetupSingleInterrupt(xdma, ResourcesRaw, ResourcesTranslated);
  } else {
    status = STATUS_UNSUCCESSFUL;
  }

  if (!NT_SUCCESS(status)) {
    TraceError(DBG_INIT, "Setting up interrupts failed: %!STATUS!", status);
  }

  *userMax = xdma->userMax;
  *h2cChannelMax = xdma->h2cChannelMax;
  *c2hChannelMax = xdma->c2hChannelMax;
  
  //  dbg_printf(("RMG: [%s] h2cChannelMax = %d, c2hChannelMax = %d, userMax = %d\n", 
  //            __FUNCTION__, *h2cChannelMax, *c2hChannelMax, *userMax));
  

  return status;
}

// ============================== public api implementation =======================================

// RMG: Estas funciones se utilizan para fijar los callbacks de usuario
NTSTATUS XDMA_UserIsrRegister(PXDMA_DEVICE xdma, ULONG eventId, PFN_XDMA_USER_WORK handler, void *userData) {
  if (eventId >= XDMA_MAX_USER_IRQ) {
    TraceError(DBG_INIT, "Invalid index! %u", eventId);
    return STATUS_INVALID_PARAMETER;
  }
  xdma->userEvents[eventId].work = handler;
  xdma->userEvents[eventId].userData = userData;
  return STATUS_SUCCESS;
}

NTSTATUS XDMA_UserIsrEnable(PXDMA_DEVICE xdma, ULONG eventId) {
  EXPECT(xdma != NULL);
  if (eventId >= XDMA_MAX_USER_IRQ) {
    TraceError(DBG_INIT, "Invalid index! %u", eventId);
    return STATUS_INVALID_PARAMETER;
  }
  xdma->interruptRegs->userIntEnableW1S = BIT_N(eventId);
  return STATUS_SUCCESS;
}

NTSTATUS XDMA_UserIsrDisable(PXDMA_DEVICE xdma, ULONG eventId) {
  EXPECT(xdma != NULL);
  if (eventId >= XDMA_MAX_USER_IRQ) {
    TraceError(DBG_INIT, "Invalid index! %u", eventId);
    return STATUS_INVALID_PARAMETER;
  }
  xdma->interruptRegs->userIntEnableW1C = BIT_N(eventId);
  return STATUS_SUCCESS;
}

//-------------------------------------

// RMG:

NTSTATUS FUSE_UserIsrRegister(PXDMA_DEVICE xdma, ULONG eventId, fuseEventCB_t handler, void *syncObj) {
  dbg_printf(("RMG: [%s]+\n", __FUNCTION__));
  if (eventId >= XDMA_MAX_USER_IRQ) {
    TraceError(DBG_INIT, "Invalid index! %u", eventId);
    return STATUS_INVALID_PARAMETER;
  }
  xdma->fuseUserEvents[eventId].event_ID = eventId;
  xdma->fuseUserEvents[eventId].userCB = handler;
  xdma->fuseUserEvents[eventId].syncObj = syncObj;
  dbg_printf(("RMG: [%s]-\n", __FUNCTION__));
  return STATUS_SUCCESS;
}

// RMG:

#include <fuse_pcie_soc_hw.h>

#include <fuse_dma_hw.h>
#include <fuse_dma_ints.h>
#include "fuse_dma.h"

#include <fuse_olink_hw.h>
#include <fuse_olink_ints.h>
#include "fuse_olink.h"

#include <fuse_emul_hw.h>
#include <fuse_emul_ints.h>
#include "fuse_emul.h"

//  #define CHECK3(...) { printf(__VA_ARGS__); }
//  #if DBG



//#define _dbg_printf(level, ...) { \
//                                  #if ((level) >= FUSE_ERR_LEVEL) \
//                                    DbgPrint(__VA_ARGS__) \
//                                  #endif \
//                                }
                                  
//#define _dbg_printf(level, ...) #if ((level) >= FUSE_ERR_LEVEL) DbgPrint(__VA_ARGS__) #endif



// Procesamiento de las interrupciones del módulo fuse_dma
static int fuse_dma_isr(struct XDMA_DEVICE_T *xdma, fuse_flags_t *flags) {
  UINT32 int_flags, int_enable, int_rqt;
  int n_buffers;

  _dbg_note("RMG: [%s] +\n", __FUNCTION__);

  fuse_dma_get_int_flags(&xdma->fuse_dma_drv, &int_flags);
  fuse_dma_get_int_enable(&xdma->fuse_dma_drv, &int_enable);
  int_rqt = int_flags & int_enable;

      
  // 0. DMA buffer ready
  if (int_rqt & DMA_BUF_RDY_INT_MASK) {

  }

  // 1. New buffer received
  if (int_rqt & DMA_NEW_BUF_INT_MASK) {
    fuse_dma_get_used_buffers(&xdma->fuse_dma_drv, &n_buffers);
    _dbg_note("[%s] RMG: New hw_image received, number of used buffers = %d\n", __FUNCTION__, n_buffers);
    if (n_buffers > xdma->max_n_buffers) {
      xdma->max_n_buffers = n_buffers;
    }
  }
  
  // 2. DMA Rx overflow: the input FIFO has overflowed and there is data loss. 
  // This is an unrecoverable error.
  if (int_rqt & DMA_RX_OVF_INT_MASK) {
    _dbg_error("[%s] Rx module overflow, unrecoverable data loss!!!\n", __FUNCTION__);
  }

  // 3. Image ID counter missmatch.
  if (int_rqt & DMA_IMG_ID_ERR_INT_MASK) {
    _dbg_warning("[%s] Image ID counter missmatch\n", __FUNCTION__);
  }
  
  // 4. Image error (wrong format) 
  if (int_rqt & DMA_IMG_ERR_INT_MASK) {
    _dbg_error("[%s] Image error (wrong format), hw image discarded!!!\n", __FUNCTION__);
  }

  // 5. DDR FIFO overflow 
  if (int_rqt & DMA_DDR_OVF_INT_MASK) {
    _dbg_warning("[%s] FIFO DDR overflow, hw_images loss!!!\n", __FUNCTION__);
  }

  // 6. DDR FIFO almost FULL 
  if (int_rqt & DMA_DDR_AFULL_INT_MASK) {
    _dbg_warning("[%s] n_buffers = %3d FIFO DDR is almost FULL, risk of hw_images loss\n",
        __FUNCTION__,
        _fuse_dma_get_used_buffers(&xdma->fuse_dma_drv));
  }

  // 7. DDR FIFO almost EMPTY
  if (int_rqt & DMA_DDR_AEMPTY_INT_MASK) {
    _dbg_warning("[%s] n_buffers = %3d FIFO DDR is almost EMPTY, transmission can be restarted\n",
        __FUNCTION__,
        _fuse_dma_get_used_buffers(&xdma->fuse_dma_drv));
  }
  
  // For development purposes:

  // 15. Test interrupt.
  if (int_rqt & DMA_TEST_INT_MASK) {
    _dbg_note("[%s] SW triggered test interrupt\n", __FUNCTION__);
  }

  // Una vez procesados limpiamos los flags y devolvemos el valor
  fuse_dma_clr_int_flags(&xdma->fuse_dma_drv, int_rqt);
  *flags = int_rqt;

  _dbg_note("RMG: [%s] -\n", __FUNCTION__);
  return 0;
}

//  UNREFERENCED_PARAMETER(xdma);
//  UNREFERENCED_PARAMETER(flags);

static int fuse_olink_isr(struct XDMA_DEVICE_T *xdma, fuse_flags_t *flags) {
  UINT32 int_flags, int_enable, int_rqt;
  
  _dbg_note("RMG: [%s] +\n", __FUNCTION__);

  fuse_olink_get_int_flags(&xdma->olink_drv, &int_flags);
  fuse_olink_get_int_enable(&xdma->olink_drv, &int_enable);
  int_rqt = int_flags & int_enable;
  
  // 0. Channel up falling edge (event interrupt)
  if (int_rqt & CH_ERR_EVT_INT_MASK) {
    _dbg_error("[%s] Channel up falling edge!!!\n", __FUNCTION__);
  }
  
  // 1. Aurora soft error rising edge (event interrupt)
  if (int_rqt & SOFT_ERR_EVT_INT_MASK) {
    _dbg_error("[%s] Aurora soft error activation!!!\n", __FUNCTION__);
  }
  
  // 2. Aurora hard error rising edge (event interrupt)
  if (int_rqt & HARD_ERR_EVT_INT_MASK) {
    _dbg_error("[%s] Aurora hard error activation!!!\n", __FUNCTION__);
  }
  
  // 3. Not channel up (level interrupt)
  if (int_rqt & CHANNEL_DOWN_INT_MASK) {
    _dbg_error("[%s] Channel down !!!\n", __FUNCTION__);
  }
  
  // 4. Aurora soft error (level interrupt)
  if (int_rqt & SOFT_ERR_INT_MASK) {
    _dbg_error("[%s] Aurora soft error!!!\n", __FUNCTION__);
  }
  
  // 5. Aurora hard error (level interrupt)
  if (int_rqt & HARD_ERR_INT_MASK) {
    _dbg_error("[%s] Aurora hard error!!!\n", __FUNCTION__);
  }
  
  // 6. Aurora PLL loss (level interrupt)
  if (int_rqt & GT_PLL_LOSS_INT_MASK) {
    _dbg_error("[%s] Aurora PLL loss!!!\n", __FUNCTION__);
  }
  
  // Una vez procesados limpiamos los flags y devolvemos el valor
  fuse_olink_clr_int_flags(&xdma->olink_drv, int_rqt);
  *flags = int_rqt;
    
  _dbg_note("RMG: [%s] -\n", __FUNCTION__);
  return 0;
}

static int fuse_emul_isr(struct XDMA_DEVICE_T *xdma, fuse_flags_t *flags) {
  UINT32 int_flags, int_enable, int_rqt;
  
  dbg_printf(("RMG: [%s] +\n", __FUNCTION__));

  fuse_emul_get_int_flags(&xdma->emul_drv, &int_flags);
  fuse_emul_get_int_enable(&xdma->emul_drv, &int_enable);
  int_rqt = int_flags & int_enable;

  if (int_rqt & IMG_DONE_INT_MASK) {
    KdPrint(("[%s] Processing IMG_DONE interrupt\n", __FUNCTION__));
  }

  if (int_rqt & RUN_DONE_INT_MASK) {
    KdPrint(("[%s] Processing RUN_DONE interrupt\n", __FUNCTION__));
  }
  
  // Una vez procesados limpiamos los flags y devolvemos el valor
  fuse_emul_clr_int_flags(&xdma->emul_drv, int_rqt);
  *flags = int_rqt;
    
  dbg_printf(("RMG: [%s] -\n", __FUNCTION__));
  return 0;
}


typedef int(*fuseISR_t)(XDMA_DEVICE *xdma, fuse_flags_t *flags);

static const fuseISR_t fuseISRtable[] = {
  fuse_dma_isr,     // 0
  fuse_olink_isr,   // 1
  fuse_emul_isr,    // 2
  NULL
};

#define N_FUSE_ISR (sizeof(fuseISRtable)/sizeof(fuseISR_t))


//UNREFERENCED_PARAMETER(irq);
static int fuse_isr(PIRQ_CONTEXT irq, UINT32 irq_mask) {
  struct XDMA_DEVICE_T *xdma = irq->xdma;
  UINT32 mask;
  fuse_flags_t flags;
  int i;

  dbg_printf(("RMG: [%s] +\n", __FUNCTION__));
  for (i = 0, mask = 1; i < N_FUSE_ISR; i++) {
    if (irq_mask & mask) {
      fuseISR_t isr = fuseISRtable[i];
      if (isr != NULL) {
        isr(xdma, &flags);
        irq->fuseUserFlags[i] |= flags;
      }
    }
    mask <<= 1;
  }
  dbg_printf(("RMG: [%s] -\n", __FUNCTION__));
  return 0;  
}

// Con las interrupciones de los canales lo que hacen en la ISR es:
//    irq->channelIrqPending |= chIrq;        // remember fired channel interrupts...
//    irq->regs->channelIntEnableW1C = chIrq; // ...disable fired channel interrupts
// Luego en el DPC lo primero que hacen es llamar al callback de usuario:
//    engine->work(engine);
// donde se limpian los flags de estado. 
// Finalmente antes de salir del DPC:
//    WdfInterruptAcquireLock(interrupt);
//    irq->regs->channelIntEnableW1S = irq->channelIrqPending;
//    irq->channelIrqPending = 0x0;
//    WdfInterruptReleaseLock(interrupt);


//---------------------------------------------------
