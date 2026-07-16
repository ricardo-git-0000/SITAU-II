// ========================= include dependencies =================================================

#include "device.h"
#include "interrupt.h"
#include "dma_engine.h"
#include "trace.h"

// #define FUSE_CHECK

#ifdef FUSE_VERBOSE
  #undef FUSE_VERBOSE
#endif

//#define FUSE_VERBOSE


#ifdef FUSE_VERBOSE
  #define dbg_printf(x)   KdPrint(x)
#else
  #define dbg_printf(x)
#endif


#ifdef DBG
// The trace message header (.tmh) file must be included in a source file before any WPP macro
// calls and after defining a WPP_CONTROL_GUIDS macro (defined in trace.h). see trace.h
#include "dma_engine.tmh"
#endif

// ========================= constants ============================================================

#define XDMA_ENG_IRQ_NUM    (1)
#define XDMA_DESC_MAGIC     (0xAD4B0000)
#define XDMA_WB_COUNT_MASK  (0x00ffffffUL)
#define XDMA_WB_ERR_MASK    (BIT_N(31))

// ========================= static function declarations =========================================

static UINT32 EngineStatus(IN XDMA_ENGINE *engine, IN BOOLEAN clear);
static void EngineGetAlignments(IN OUT XDMA_ENGINE *engine);
static NTSTATUS EngineCreateDescriptorBuffer(IN OUT XDMA_ENGINE *engine);
static NTSTATUS EngineCreateRingBuffer(IN XDMA_ENGINE* engine);
static void EngineConfigureInterrupt(IN OUT XDMA_ENGINE *engine, IN UINT index, IN ULONG engineId);
static void EngineProcessTransfer(IN XDMA_ENGINE *engine);
static UINT EngineProcessRing(IN XDMA_ENGINE *engine);
static void EngineRingAdvance(UINT* index);
static NTSTATUS EngineCreatePollWriteBackBuffer(IN OUT XDMA_ENGINE *engine);
static NTSTATUS EngineWaitForCompletion(IN XDMA_ENGINE* engine, IN LARGE_INTEGER timeout);
static NTSTATUS EnginePollTransfer(IN XDMA_ENGINE* engine, IN LARGE_INTEGER timeout);




static NTSTATUS FUSE_EngineCreate(PXDMA_DEVICE xdma,
                                  XDMA_ENGINE *engine,
                                  DirToDev dir,
                                  ULONG channel,
                                  ULONG engineIndex,
                                  ULONG engineId);
                                  
BOOLEAN FUSE_EngineProgramDma(
              IN WDFDMATRANSACTION Transaction,
              IN WDFDEVICE Device,
              IN WDFCONTEXT context,
              IN WDF_DMA_DIRECTION Direction,
              IN PSCATTER_GATHER_LIST SgList);
                                  
                                  
static void FUSE_EngineProcessTransfer(IN XDMA_ENGINE *engine);
static void FUSE_terminateThread(XDMA_ENGINE *engine);
VOID FUSE_CompletionThread(PVOID context);

// Mark these functions as pageable code
#ifdef ALLOC_PRAGMA
#pragma alloc_text (PAGE, ProbeEngines)
#endif

// WDK 10 static code analysis gives a false warning: "Allocating executable memory via specifying
// a MM_PAGE_PRIORITY type without a bitwise OR with MdlMappingNoExecute"
// However this only applies to Windows 8 and above, thus disable this warning.
// see https://msdn.microsoft.com/en-us/library/windows/hardware/ff554629(v=vs.85).aspx
#pragma warning (disable : 30030)

// ======================== common engine functions ===============================================
static BOOLEAN isReqPending(IN XDMA_ENGINE *engine)
{
  BOOLEAN isReqPending;

  WdfSpinLockAcquire(engine->engineLock);
  isReqPending = engine->isReqPending;
  WdfSpinLockRelease(engine->engineLock);

  return isReqPending;
}

// RMG:
static VOID markReqPending(IN XDMA_ENGINE *engine) {
  WdfSpinLockAcquire(engine->engineLock);
  engine->isReqPending = TRUE;
  WdfSpinLockRelease(engine->engineLock);
}

static VOID unmarkReqPending(IN XDMA_ENGINE *engine)
{
  WdfSpinLockAcquire(engine->engineLock);
  engine->isReqPending = FALSE;
  WdfSpinLockRelease(engine->engineLock);
}

static NTSTATUS EngineCreateDescriptorBuffer(IN OUT XDMA_ENGINE *engine) {
  // allocate host-side buffer for descriptors
  // RMG: max_transfer_size = 8MB, page_size = 4K => n_desc = 2K
  SIZE_T bufferSize = (XDMA_MAX_TRANSFER_SIZE / PAGE_SIZE + 2) * sizeof(DMA_DESCRIPTOR);

  NTSTATUS status = WdfCommonBufferCreate(engine->parentDevice->dmaEnabler, bufferSize,
                      WDF_NO_OBJECT_ATTRIBUTES, &engine->descBuffer);
  if (!NT_SUCCESS(status)) {
    TraceError(DBG_INIT, "WdfCommonBufferCreate failed: %!STATUS!", status);
    return status;
  }

  engine->capacity = (UINT32)(bufferSize / sizeof(DMA_DESCRIPTOR));

  PHYSICAL_ADDRESS descBufferLA = WdfCommonBufferGetAlignedLogicalAddress(engine->descBuffer);
  PUCHAR descBufferVA = (PUCHAR)WdfCommonBufferGetAlignedVirtualAddress(engine->descBuffer);
  RtlZeroMemory(descBufferVA, bufferSize);

  // give hw the physical start address of the descriptor buffer
  engine->sgdma->firstDescLo = descBufferLA.LowPart;
  engine->sgdma->firstDescHi = descBufferLA.HighPart;
  engine->sgdma->firstDescAdj = 0; // depends on transfer - set later in ProgramDMA

  TraceVerbose(DBG_INIT, "descriptor buffer at 0x%08x%08x, size=%lld, capacity=%u",
         engine->sgdma->firstDescHi, engine->sgdma->firstDescLo,
         bufferSize, engine->capacity);

  return status;
}

static void EngineConfigureInterrupt(IN OUT XDMA_ENGINE *engine, IN UINT index, IN ULONG engineId) {
  // engine interrupt request bit(s) - interrupt bit depends on number of engines present
  // see Figure 2-4 on page 46 of pcie dma product guide [1]
  engine->irqBitMask = (1 << XDMA_ENG_IRQ_NUM) - 1;
  engine->irqBitMask <<= (engineId * XDMA_ENG_IRQ_NUM);

  // bind msi interrupt context with this engine
  if (engine->parentDevice->channelInterrupts[index] != NULL) {
    PIRQ_CONTEXT irqContext = GetIrqContext(engine->parentDevice->channelInterrupts[index]);
    irqContext->engine = engine;
  }

  // enable interrupts
  UINT32 regVal = XDMA_CTRL_IE_ALL;
  if ((engine->type == EngineType_ST) && (engine->dir == C2H)) {
    regVal |= XDMA_CTRL_IE_IDLE_STOPPED;
  }
  engine->regs->intEnableMaskW1S = regVal;
  engine->regs->controlW1S = regVal;
  TraceVerbose(DBG_INIT, "engineIrqBitMask=0x%08x, intEnableMask=0x%08x",
         engine->irqBitMask, engine->regs->intEnableMask);
}


// RMG: Este es el callback de usuario al que llama DPC (bottom half) de las
// interrupciones.

static void EngineProcessTransfer(IN XDMA_ENGINE *engine) {
// service an SGDMA engine
  NTSTATUS status = STATUS_SUCCESS;
  WDFREQUEST request;
  UINT32 engineStatus;
  BOOLEAN completed;

  if (engine == NULL) {
    TraceError(DBG_DMA, "engine=NULL");
    return;
  }

  TraceInfo(DBG_DMA, "%s_%u processing transfer completion",
        DirectionToString(engine->dir), engine->channel);

  WdfSpinLockAcquire(engine->engineLock);

  if (FALSE == engine->isReqPending) {
    /* Spurious interrupt ?? */
    TraceError(DBG_DMA, "No request submitted for engine.");
    WdfSpinLockRelease(engine->engineLock);
    return;
  }

  request = WdfDmaTransactionGetRequest(engine->dmaTransaction);
  if (!request) {
    TraceInfo(DBG_DMA, "Interrupt but no request pending?");
    WdfSpinLockRelease(engine->engineLock);
    return;
  }

  // read and clear engine status
  engineStatus = EngineStatus(engine, TRUE);

  EngineStop(engine);

  // clear descriptor buffer
  DMA_DESCRIPTOR *descriptorBuffer = (DMA_DESCRIPTOR *)WdfCommonBufferGetAlignedVirtualAddress(engine->descBuffer);
  size_t descBufferLength = WdfCommonBufferGetLength(engine->descBuffer);
  RtlZeroMemory(descriptorBuffer, descBufferLength);

  // clear poll writeback buffer
  if (engine->poll) {
    XDMA_POLL_WB* wbBuffer = (XDMA_POLL_WB*)WdfCommonBufferGetAlignedVirtualAddress(engine->pollWbBuffer);
    size_t wbBufferLength = WdfCommonBufferGetLength(engine->pollWbBuffer);
    RtlZeroMemory(wbBuffer, wbBufferLength);
  }

  // RMG: En XDMA_EngineProgramDma() se comprueba si hay una solicitud
  // pendiente (isReqPending) y en tal caso no se lanza una nueva

  // If any data is pending then call to WdfDmaTransactionDmaCompleted will immediately result in
  // another call to XDMA_EngineProgramDma. To avoid deadlock, mark the engine as free and release the lock.
  engine->isReqPending = FALSE;
  if (!engine->poll) {
    // reenable interrupt for this dma engine
    EngineEnableInterrupt(engine);
  }
  WdfSpinLockRelease(engine->engineLock);

  switch (engineStatus & XDMA_STAT_EXPECTED_ZERO) {
    case XDMA_ENGINE_STOPPED_OK: // engine not busy and no errors?
    {
      completed = WdfDmaTransactionDmaCompleted(engine->dmaTransaction, &status);
      size_t bytesTransferred = WdfDmaTransactionGetBytesTransferred(engine->dmaTransaction);

      TraceInfo(DBG_DMA, "%s_%u transaction%scomplete, bytesTransferred=%llu",
            DirectionToString(engine->dir), engine->channel,
            completed ? " " : " in", bytesTransferred);

      if (completed) {
        if (!engine->poll) {
          KeSetEvent(&engine->completionWaitSignal, IO_NO_INCREMENT, FALSE);
        }

        status = WdfDmaTransactionRelease(engine->dmaTransaction);
        if (!NT_SUCCESS(status)) {
          TraceError(DBG_DMA, "WdfDmaTransactionRelease failed: %!STATUS!", status);
        }

        WdfRequestCompleteWithInformation(request, status, bytesTransferred);
      }
      break;
    }
    case XDMA_BUSY_BIT: // engine is still busy without sign of errors?
      TraceError(DBG_DMA, "Engine Still Busy, Descriptors Completed=%u",
             engine->regs->completedDescCount);
    default: // any sign of errors
      TraceError(DBG_DMA, "Unexpected engine status 0x%08x", engineStatus);

      completed = WdfDmaTransactionDmaCompletedFinal(engine->dmaTransaction, 0, &status);
      if (completed) {
        if (!engine->poll) {
          TraceInfo(DBG_DMA, "EVENT SIGNALLED");
          KeSetEvent(&engine->completionWaitSignal, IO_NO_INCREMENT, FALSE);
        }

        status = WdfDmaTransactionRelease(engine->dmaTransaction);
        if (!NT_SUCCESS(status)) {
          TraceError(DBG_DMA, "WdfDmaTransactionRelease failed: %!STATUS!", status);
        }

        WdfRequestComplete(request, STATUS_INTERNAL_ERROR);
      }
  } // end-switch

}

static void DumpDescriptor(IN const DMA_DESCRIPTOR* const desc) {
#if DBG
  TraceVerbose(DBG_DESC, "descriptor={.control=0x%08X, .numBytes=%u, srcAddr=0x%08X%08X, .dstAddr=0x%08X%08X, nextAddr=0x%08X%08X}",
         desc->control, desc->numBytes,
         desc->srcAddrHi, desc->srcAddrLo,
         desc->dstAddrHi, desc->dstAddrLo,
         desc->nextHi, desc->nextLo);
#else
  UNREFERENCED_PARAMETER(desc);
#endif
}

// RMG: Table 11: Address Alignment

static BOOLEAN DescriptorIsAligned(IN XDMA_ENGINE *engine, IN DMA_DESCRIPTOR *desc)
// For alignment requirements see product guide [1] page 23 table 2-9
{
  if (engine->addressMode == AddressMode_Fixed) {
    const UINT32 dataPathWidth = (1 << (6 + engine->parentDevice->configRegs->pcieWidth)) / 8;
    const UINT32 addrMask = dataPathWidth - 1;

    if ((desc->dstAddrLo & addrMask) != (desc->srcAddrLo & addrMask) != 0) {
      TraceError(DBG_DESC, "misalignment detected! dataWidth=%u, src=0x%08llx, dst=0x%08llx, bytes=%llu",
             dataPathWidth, desc->srcAddrLo, desc->dstAddrLo, desc->numBytes);
      return FALSE;
    }
    // todo length alignment requirement??

  } else { // AddressMode_Contiguous (i.e. incremental mode)
    BOOLEAN dstAddrMisaligned = (desc->dstAddrLo % engine->alignAddr) != 0;
    BOOLEAN lengthMisaligned = (desc->numBytes % engine->alignLength) != 0;
    BOOLEAN srcAddrMisaligned = (desc->srcAddrLo % engine->alignAddr) != 0;

    if (dstAddrMisaligned || lengthMisaligned || srcAddrMisaligned) {
      TraceVerbose(DBG_DESC, "descriptor alignments: src=0x%08llx, dst=0x%08llx, bytes=%llu",
             desc->srcAddrLo, desc->dstAddrLo, desc->numBytes);
      TraceError(DBG_DESC, "misalignment detected! src?=%u dst?=%u length?=%u",
             srcAddrMisaligned, dstAddrMisaligned, lengthMisaligned);
             
      dbg_printf(("descriptor alignments: src=0x%08llx, dst=0x%08llx, bytes=%llu",
             desc->srcAddrLo, desc->dstAddrLo, desc->numBytes));
      dbg_printf(("misalignment detected! src?=%u dst?=%u length?=%u",
             srcAddrMisaligned, dstAddrMisaligned, lengthMisaligned));

      return FALSE;
    }
  }
  return TRUE;
}

static void OptimizeDescriptors(IN XDMA_ENGINE *engine, IN DMA_DESCRIPTOR * const desc,
                IN const ULONG numDesc)
  // Optimize descriptors for PCIe block fetches.
  // Multiple descriptors which reside in host memory can be fetched in a single PCIe transaction
  // by the device. This is achieved as follows:
  //    - For the first fetch, the number of additional (adjacent) descriptors to fetch is
  //    specified by writing to engine->sgdma->firstDescAdj register.
  //    - For subsequent fetches, the last descriptor of the previous fetch specifies the number of
  //    additional (adjacent) descriptors in the control->nextAdj field
  // There are several factors which limit the amount of descriptors which can be fetched together:
  //    1. The PCIe Max Read Request Size
  //    2. The physical address of the descriptors within a block must not cross a 4K address
  //     boundary
  //    3. The number of descriptors remaining in the transfer
{
  const ULONG mrrsBytes = 1 << (engine->parentDevice->configRegs->pcieMRRS + 7); // RMG N = 2^(n+7)
  const ULONG adjMax = mrrsBytes / sizeof(DMA_DESCRIPTOR) - 1;
  const ULONG adjTotal = numDesc - 1;
  const ULONG adjTo4k = (0x1000 - (engine->sgdma->firstDescLo & 0xFFF)) / sizeof(DMA_DESCRIPTOR) - 1;

  // set the number of adjacent descriptors for the first fetch
  // RMG: firstAdj = min(adjTotal, adjMax, adjTo4k)
  ULONG firstAdj = (adjTotal < adjMax) ? adjTotal : adjMax;
  engine->sgdma->firstDescAdj = (adjTo4k < firstAdj) ? adjTo4k : firstAdj;
  //TraceVerbose(DBG_DMA, "first: PA=%04u, this4k=%04u, total=%04u, thisBlock=%04u",
  //       engine->sgdma->firstDescLo & 0xFFF, adjTo4k, adjTotal, engine->sgdma->firstDescAdj);

  // set the number of adjacent descriptors for subsequent fetches
  ULONG nextAdjMax = adjMax - 1;
  for (UINT i = 0; i < numDesc; i++) {
    // if not last desc then get total desc adj to next desc, else last desc has no next desc
    // RMG: Si no es el último descriptor el número de descriptores
    // adicionales es adjTotal-(i+1). En caso contrario 0
    const ULONG nextAdjTotal = (i != adjTotal) ? adjTotal - (i + 1) : 0;
    // RMG: Calculamos la restricción asociada al cruce de una página de 4K
    const ULONG nextAdjTo4k = (0x1000 - (desc[i].nextLo & 0xFFF)) / sizeof(DMA_DESCRIPTOR) - 1;

    // RMG: Nos quedamos con min(nextAdjTotal, nextAdjMax, nextAdjTo4k)
    ULONG nextAdj = (nextAdjTotal < nextAdjMax) ? nextAdjTotal : nextAdjMax;
    if (nextAdj > nextAdjTo4k) {
      nextAdj = nextAdjTo4k;
    }

    desc[i].control |= (nextAdj << 8);
    //TraceVerbose(DBG_DMA, "next: PA=%04u, this4k=%04u, total=%04u, thisBlock=%04u",
    //       desc[i].nextLo & 0xFFF, nextAdjTo4k, nextAdjTotal, nextAdj);

    // update current max adj count for this block
    if (nextAdjMax != 0) {
      nextAdjMax--;
    } else { // wrap-around
      nextAdjMax = adjMax;
    }
  }
}

static BOOLEAN EngineExists(PXDMA_DEVICE xdma, DirToDev dir, ULONG channel) {
  PUCHAR configBarAddr = (PUCHAR)xdma->bar[xdma->configBarIdx];
  const ULONG offset = (dir * BLOCK_OFFSET) + (channel * ENGINE_OFFSET);
  XDMA_ENGINE_REGS* engineRegs = (XDMA_ENGINE_REGS*)(configBarAddr + offset);
  UINT32 engineID = engineRegs->identifier;
  return (engineID & XDMA_ID_MASK) == XDMA_ID;
}

VOID xdmaCompletionThread(PVOID context)
{
  XDMA_ENGINE* engine = (XDMA_ENGINE *)context;
  ULONG processor = engine->channel;

  if (engine->dir == C2H)
    processor += engine->parentDevice->h2cChannelMax;

  processor = (processor % KeQueryActiveProcessorCount(NULL));
  TraceVerbose(DBG_INIT, "Active thread ID : %u", processor);
  KAFFINITY affinity = (KAFFINITY)1 << processor;
  KeSetSystemAffinityThread(affinity);

  while (1) {

    KeWaitForSingleObject(&engine->semaphore,
                Executive,
                KernelMode,
                FALSE,
                NULL);

    if (engine->terminate) {
      TraceInfo(DBG_INIT, "Terminating thread : %u", engine->channel);
      PsTerminateSystemThread(STATUS_SUCCESS);
      return;
    }

    if (TRUE == isReqPending(engine)) {
      LARGE_INTEGER timeout;
      timeout.QuadPart = -10 * 10000000; // 10 seconds timeout

      NTSTATUS status = EngineWaitForCompletion(engine, timeout);
      if (STATUS_SUCCESS != status) {
        WDFREQUEST request = WdfDmaTransactionGetRequest(engine->dmaTransaction);
        if (request) {
          TraceError(DBG_DMA, "EngineWaitForCompletion failed: %!STATUS!", status);
          WdfDmaTransactionRelease(engine->dmaTransaction);
          WdfRequestComplete(request, status);
        }
      }
    }
  }
}

static NTSTATUS initThread(XDMA_ENGINE* engine)
{
  engine->terminate = FALSE;
  KeInitializeSemaphore(&engine->semaphore, // PRKSEMAPHORE Semaphore,
                        0,                  // LONG         Count,
                        MAXLONG);           // LONG         Limit

  TraceInfo(DBG_INIT, "Creating thread : %u", engine->channel);

  NTSTATUS status;
  status = PsCreateSystemThread(&engine->thHandle,    // PHANDLE            ThreadHandle,
                                (ACCESS_MASK)0,       // ULONG              DesiredAccess,
                                NULL,                 // POBJECT_ATTRIBUTES ObjectAttributes,
                                (HANDLE)0,            // HANDLE             ProcessHandle,
                                NULL,                 // PCLIENT_ID         ClientId,
                                xdmaCompletionThread, // PKSTART_ROUTINE    StartRoutine,
                                engine);              // PVOID              StartContext

  if (!NT_SUCCESS(status)) {
    TraceError(DBG_INIT, "Failed to create thread [%u][%u] - %!STATUS!", engine->dir, engine->channel, status);
    return status;
  }

  // RMG: returns the corresponding pointer to the object's body.
  ObReferenceObjectByHandle(engine->thHandle,   // HANDLE                     Handle,
                            THREAD_ALL_ACCESS,  // ACCESS_MASK                DesiredAccess,
                            NULL,               // POBJECT_TYPE               ObjectType,
                            KernelMode,         // KPROCESSOR_MODE            AccessMode,
                            &engine->thObject,  // PVOID                      *Object,
                            NULL);              // POBJECT_HANDLE_INFORMATION HandleInformation

  // RMG: closes an object handle. Closing an open object handle causes that
  // handle to become invalid. A driver must close every handle that it opens
  // as soon as the handle is no longer required.
  ZwClose(engine->thHandle);

  engine->thInitialized = TRUE;
  return status;
}

static void terminateThread(XDMA_ENGINE* engine)
{
  TraceInfo(DBG_INIT, "Terminating thread : %u", engine->channel);

  if (FALSE == engine->thInitialized)
    return;

  if (engine->thObject) {
    engine->terminate = TRUE;
    KeReleaseSemaphore(&engine->semaphore, 0, 1, FALSE);

    KeWaitForSingleObject(engine->thObject,
      Executive,
      KernelMode,
      FALSE,
      NULL);

    ObDereferenceObject(engine->thObject);
  }
}

static NTSTATUS EngineCreate(PXDMA_DEVICE xdma, XDMA_ENGINE* engine, DirToDev dir, ULONG channel,
               ULONG engineIndex, ULONG engineId) {

  NTSTATUS status;

  engine->parentDevice = xdma;
  engine->channel = channel;
  engine->dir = dir;
  const ULONG offset = (dir * BLOCK_OFFSET) + (channel * ENGINE_OFFSET);
  PUCHAR configBarAddr = (PUCHAR)xdma->bar[xdma->configBarIdx];
  engine->regs = (XDMA_ENGINE_REGS*)(configBarAddr + offset);
  engine->sgdma = (XDMA_SGDMA_REGS*)(configBarAddr + offset + SGDMA_BLOCK_OFFSET);

  // AXI-MM or AXI-ST? 0 = MM, 1 = ST
  engine->type = (engine->regs->identifier & XDMA_ID_ST_BIT) != 0;

  // Incremental or Non-Incremental address mode? 0 = inc, 1=non-inc
  engine->addressMode = (engine->regs->control & XDMA_CTRL_NON_INCR_ADDR) != 0;

  // set interrupt sources
  EngineConfigureInterrupt(engine, engineIndex, engineId);

  // create common buffer for poll mode descriptor write back - if used
  status = EngineCreatePollWriteBackBuffer(engine);
  if (!NT_SUCCESS(status)) {
    TraceError(DBG_INIT, "EngineCreatePollWriteBackBuffer() failed: %!STATUS!", status);
    return status;
  }

  // capture alignment requirements
  EngineGetAlignments(engine);

  // create and bind dma desciptor buffer to hw
  status = EngineCreateDescriptorBuffer(engine);
  if (!NT_SUCCESS(status)) {
    TraceError(DBG_INIT, "EngineCreateDescriptorBuffer() failed: %!STATUS!",
           status);
    return status;
  }

  // allocate wdf dma transaction object
  status = WdfDmaTransactionCreate(xdma->dmaEnabler, WDF_NO_OBJECT_ATTRIBUTES, &engine->dmaTransaction);
  if (!NT_SUCCESS(status)) {
    TraceError(DBG_INIT, "WdfDmaTransactionCreate() failed: %!STATUS!", status);
    return status;
  }

  if ((engine->type == EngineType_ST) && (engine->dir == C2H)) {
    engine->work = EngineProcessRing;
    status = EngineCreateRingBuffer(engine);
    if (!NT_SUCCESS(status)) {
      TraceError(DBG_INIT, "EngineCreateStreamBuffers() failed: %!STATUS!", status);
      return status;
    }

    engine->parentDevice->sgdmaRegs->creditModeEnableW1S = BIT_N(engine->channel) << 16;
    TraceInfo(DBG_INIT, "creditModeEnable=0x%x", engine->parentDevice->sgdmaRegs->creditModeEnable);
  } else {
    engine->work = EngineProcessTransfer;
  }

  // Initialize completion wait and engine lock
  engine->isReqPending = FALSE;

  status = WdfSpinLockCreate(WDF_NO_OBJECT_ATTRIBUTES, &engine->engineLock);
  if (!NT_SUCCESS(status)) {
    TraceError(DBG_INIT, "WdfSpinLockCreate failed: %!STATUS!", status);
    return status;
  }

  KeInitializeEvent(&engine->completionWaitSignal, NotificationEvent, FALSE);

  status = initThread(engine);
  if (!NT_SUCCESS(status)) {
    TraceError(DBG_INIT, "initThread failed: %!STATUS!", status);
    return status;
  }

  engine->enabled = TRUE;

  return status;
}

static void EngineGetAlignments(IN OUT XDMA_ENGINE *engine) {

  UINT32 alignments = engine->regs->alignments;
  UINT32 align_bytes = (alignments & 0x00ff0000U) >> 16;
  UINT32 granularity_bytes = (alignments & 0x0000ff00U) >> 8;
  UINT32 address_bits = (alignments & 0x000000ffU);

  if (alignments) {
    engine->alignAddr = align_bytes;
    engine->alignLength = granularity_bytes;
    engine->alignAddrBits = address_bits;
  } else { // Some default values if alignments are unspecified
    engine->alignAddr = 1;
    engine->alignLength = 1;
    engine->alignAddrBits = 64;
  }

  TraceVerbose(DBG_INIT, "engine[%u][%u] alignments: bytes=%u, granularity=%u, addrBits=%u",
    engine->channel, engine->dir,
    engine->alignAddr, engine->alignLength, engine->alignAddrBits);
}

static UINT32 EngineStatus(IN XDMA_ENGINE *engine, IN BOOLEAN clear)
// read and optionally clear the engine status
{
  UINT32 status = 0;

  if (clear) { // request to clear engine status after read?
    TraceVerbose(DBG_DMA, "%s_%u engine status cleared after next read",
           DirectionToString(engine->dir), engine->channel);
    status = engine->regs->statusRC;
  } else {
    status = engine->regs->status;
  }

  TraceInfo(DBG_DMA, "%s_%u status=0x%08x (%s%s%s%s%s%s%s%s%s)",
        DirectionToString(engine->dir), engine->channel, status,
        (status & XDMA_BUSY_BIT) ? "BUSY " : "IDLE ",
        (status & XDMA_DESCRIPTOR_STOPPED_BIT) ? "DESCRIPTOR_STOPPED " : " ",
        (status & XDMA_DESCRIPTOR_COMPLETED_BIT) ? "DESCRIPTOR_COMPLETED " : " ",
        (status & XDMA_ALIGN_MISMATCH_BIT) ? "ALIGN_MISMATCH " : " ",
        (status & XDMA_MAGIC_STOPPED_BIT) ? "MAGIC_STOPPED " : " ",
        (status & XDMA_FETCH_STOPPED_BIT) ? "FETCH_STOPPED" : " ",
        (status & XDMA_STAT_READ_ERROR) ? "READ_ERROR " : " ",
        (status & XDMA_STAT_DESCRIPTOR_ERROR) ? "DESCRIPTOR_ERROR " : " ",
        (status & XDMA_IDLE_STOPPED_BIT) ? "IDLE_STOPPED" : " ");

  return status;
}

// WdfDmaTransactionInitializeUsingRequest

// RMG: Este es el punto de entrada para la preparación de una lectura
// scather-gather. Es el callback que ejecuta automáticamente Windows al
// llamar a WdfDmaTransactionInitializeUsingRequest() pasandoselo como
// argumento

// Su funcionamiento se detalla en:
//  https://learn.microsoft.com/en-us/windows-hardware/drivers/wdf/programming-dma-hardware
//
//    The EvtProgramDma callback function, which is called at IRQL =
//    DISPATCH_LEVEL, programs the device to start a DMA transfer. The input
//    parameters for this callback function supply the transfer's direction
//    (input or output) and a scatter/gather list. If the transfer consists
//    of a single packet, the scatter/gather list contains a single element.


BOOLEAN XDMA_EngineProgramDma(IN WDFDMATRANSACTION Transaction, IN WDFDEVICE Device,
                IN WDFCONTEXT context, IN WDF_DMA_DIRECTION Direction,
                IN PSCATTER_GATHER_LIST SgList)
  // this programs the engine to start a dma transfer
{
  UNREFERENCED_PARAMETER(Device);

  WDFREQUEST request = WdfDmaTransactionGetRequest(Transaction);
  WDF_REQUEST_PARAMETERS params;
  WDF_REQUEST_PARAMETERS_INIT(&params);
  WdfRequestGetParameters(request, &params);
  LONGLONG deviceOffset = (Direction == WdfDmaDirectionWriteToDevice) ?
    (SIZE_T)params.Parameters.Write.DeviceOffset :
    (SIZE_T)params.Parameters.Read.DeviceOffset;
  size_t numBytesTransferred;

  // get virtual and physical pointers to descriptor buffer
  XDMA_ENGINE *engine = (XDMA_ENGINE *)context;
  DMA_DESCRIPTOR *descriptor = (DMA_DESCRIPTOR *)WdfCommonBufferGetAlignedVirtualAddress(engine->descBuffer);
  PHYSICAL_ADDRESS descBufferLA = WdfCommonBufferGetAlignedLogicalAddress(engine->descBuffer);

  WdfSpinLockAcquire(engine->engineLock);
  if (TRUE == engine->isReqPending) {
    WdfSpinLockRelease(engine->engineLock);
    ASSERTMSG("Request is already pending", (1 == 1));  // RMG: Esto nunca hace nada!!!
  }
  else {
    WdfSpinLockRelease(engine->engineLock);
  }

  // RMG: engine->capacity es el número total de descriptores
  //validate num descs
  if (SgList->NumberOfElements > engine->capacity) {
    TraceError(DBG_INIT, "Too many descriptors...");
    goto ErrExit;
  }

  // offset into the transaction (if it is split)
  numBytesTransferred = WdfDmaTransactionGetBytesTransferred(Transaction);
  deviceOffset += numBytesTransferred;

  TraceVerbose(DBG_DMA, "device addr=%lld, num descriptors=%d",
         deviceOffset, SgList->NumberOfElements);

  // RMG: Aquí es donde se lleva a cabo la inicialización de los descriptores
  // a partir de la información suministrada por Windows en la SgList
  for (ULONG i = 0; i < SgList->NumberOfElements; i++) {
    descriptor[i].control = XDMA_DESC_MAGIC;
    descriptor[i].numBytes = SgList->Elements[i].Length;
    ULONG hostAddrLo = SgList->Elements[i].Address.LowPart;
    LONG hostAddrHi = SgList->Elements[i].Address.HighPart;
    if (Direction == WdfDmaDirectionWriteToDevice) {
      // source is host memory
      descriptor[i].srcAddrLo = hostAddrLo;
      descriptor[i].srcAddrHi = hostAddrHi;
      descriptor[i].dstAddrLo = LIMIT_TO_32(deviceOffset);
      descriptor[i].dstAddrHi = LIMIT_TO_32(deviceOffset >> 32);
    } else {
      // destination is host memory
      descriptor[i].srcAddrLo = LIMIT_TO_32(deviceOffset);
      descriptor[i].srcAddrHi = LIMIT_TO_32(deviceOffset >> 32);
      descriptor[i].dstAddrLo = hostAddrLo;
      descriptor[i].dstAddrHi = hostAddrHi;
    }

    // next descriptor bus address
    descBufferLA.QuadPart += sizeof(DMA_DESCRIPTOR);

    // RMG: Si es el último descriptor fijo su campo next_addr a 0, activo sus
    // bits de STOP y COMPLETED para que al concluir su procesameinto se
    // detenga y se genere una interrupción. Si no es el último simplemente
    // fijo su campo next_addr para que apunte al siguiente descriptor.

    // non-last descriptor(s)?
    if ((i + 1) < SgList->NumberOfElements) {
      descriptor[i].nextLo = descBufferLA.LowPart;
      descriptor[i].nextHi = descBufferLA.HighPart;
    } else { // last descriptor
      descriptor[i].nextLo = 0;
      descriptor[i].nextHi = 0;
      // stop engine and request an interrupt from the engine
      descriptor[i].control |= (XDMA_DESC_STOP_BIT | XDMA_DESC_COMPLETED_BIT);
      if (engine->type == EngineType_ST) {
        descriptor[i].control |= XDMA_DESC_EOP_BIT;
        TraceVerbose(DBG_DMA, "descriptor[i].control=0x%08x", descriptor[i].control);
      }
    }
    if (engine->addressMode == AddressMode_Contiguous) { // incremental address mode
      deviceOffset += SgList->Elements[i].Length;
    }

    if (FALSE == DescriptorIsAligned(engine, &(descriptor[i]))) {
      TraceWarning(DBG_DMA, "Error: Dma Transfer is not aligned");
    }
  }

  // RMG: Gestion del campo next_adj para optimizar la prebúsqueda de descriptores
  OptimizeDescriptors(engine, descriptor, SgList->NumberOfElements);

  for (ULONG i = 0; i < SgList->NumberOfElements; i++) {
    DumpDescriptor(&(descriptor[i]));
  }

  if (engine->poll) {
    engine->numDescriptors = SgList->NumberOfElements;
  }

  MemoryBarrier();
  markReqPending(engine); // Activa un flag de la estructura de control
  // start the engine
  EngineStart(engine);    // Arranca el engine
  MemoryBarrier();

  LARGE_INTEGER timeout;
  timeout.QuadPart = -10 * 10000000; // 10 seconds timeout
  NTSTATUS status = STATUS_SUCCESS;

  if (engine->poll) {
    status = EnginePollTransfer(engine, timeout);
    if (STATUS_SUCCESS != status) {
      // EnginePollTransfer does not complete the request on internal/timeout error.
      TraceError(DBG_DMA, "EnginePollTransfer failed: %!STATUS!", status);
      goto ErrExit;
    }
  }
  else { // Interrupt mode
    if (numBytesTransferred == (size_t)0) {
      /* In split DMA, multiple EvtProgramDma callbacks will be called.
       * After completing the first split, EvtProgramDma will be called from DPC.
       * We can not do sleep wait from DPC, hence we do wait only for first split i.e.
       * from the completion thread.
       */
      KeReleaseSemaphore(&engine->semaphore,  // PRKSEMAPHORE Semaphore,
                          0,                  // KPRIORITY    Increment,
                          1,                  // LONG         Adjustment,
                          FALSE);             // BOOLEAN      Wait
    }
  }

  return TRUE;

ErrExit:
  // FIXME: Current framework ignores the FALSE return value.
  // Therefore cleaning the request here itself.
  status = STATUS_UNSUCCESSFUL;
  WdfDmaTransactionRelease(engine->dmaTransaction);
  WdfRequestComplete(request, status);
  return FALSE;
}

void CountChannels(IN PXDMA_DEVICE xdma, OUT ULONG *h2cCount,OUT ULONG *c2hCount)
{
  *h2cCount = *c2hCount = 0;

  // H2C Channels
  for (ULONG ch = 0; ch < XDMA_MAX_NUM_CHANNELS; ch++) {
    if (EngineExists(xdma, H2C, ch)) {
      (*h2cCount)++;
    }
  }

  // C2H Channels
  for (ULONG ch = 0; ch < XDMA_MAX_NUM_CHANNELS; ch++) {
    if (EngineExists(xdma, C2H, ch)) {
      (*c2hCount)++;
    }
  }
}

NTSTATUS ProbeEngines(IN PXDMA_DEVICE xdma) {
  PAGED_CODE();

  // engineIndex es el indice del bit correspondiente a cada engine en los
  // registros 'Channel Interrupt Enable Mask', 'Channel Interrupt Request' y
  // 'Channel Interrupt Pending'.
  // La máscara correspondiente a cada canal (1 << engineIndex) se almacena en
  // el campo de 'irqBitMask'
  ULONG engineIndex = 0;

  // iterate over H2C (FPGA performs PCIe reads towards FPGA),
  // then C2H (FPGA performs PCIe writes from FPGA)
  ULONG h2cChannelMax, c2hChannelMax;
  CountChannels(xdma, &h2cChannelMax, &c2hChannelMax);

  UINT dir = H2C; // dir = 0
  for (ULONG ch = 0; ch < xdma->h2cChannelMax; ch++) {
    if (EngineExists(xdma, dir, ch)) {
      XDMA_ENGINE* engine = &(xdma->engines[ch][dir]);
      NTSTATUS status = EngineCreate(xdma, engine, dir, ch, engineIndex, engineIndex);
      if (!NT_SUCCESS(status)) {
        TraceError(DBG_INIT, "EngineCreate failed! %!STATUS!", status);
        return status;
      }
      engineIndex++;
      TraceInfo(DBG_INIT, "%s_%u engine created (AXI-%s)",
        DirectionToString(dir), ch, engine->type == EngineType_ST ? "ST" : "MM");
    }
    else {   // skip inactive engines
      TraceInfo(DBG_INIT, "Skipping non-existing engine %s_%u",
        DirectionToString(dir), ch);
    }
  }

  dir = C2H;  // dir = 1
  for (ULONG ch = 0; ch < xdma->c2hChannelMax; ch++) {
    if (EngineExists(xdma, dir, ch)) {
      XDMA_ENGINE *engine = &(xdma->engines[ch][dir]);
#ifdef USE_FUSE_DMA
      NTSTATUS status = FUSE_EngineCreate(xdma, engine, dir, ch, engineIndex, h2cChannelMax + ch);
#else
      NTSTATUS status = EngineCreate(xdma, engine, dir, ch, engineIndex, h2cChannelMax + ch);
#endif
      if (!NT_SUCCESS(status)) {
        TraceError(DBG_INIT, "EngineCreate failed! %!STATUS!", status);
        return status;
      }
      engineIndex++;
      TraceInfo(DBG_INIT, "%s_%u engine created (AXI-%s)",
        DirectionToString(dir), ch, engine->type == EngineType_ST ? "ST" : "MM");
    }
    else {   // skip inactive engines
      TraceInfo(DBG_INIT, "Skipping non-existing engine %s_%u", DirectionToString(dir), ch);
    }
  }

  return STATUS_SUCCESS;
}

void closeEngines(IN PXDMA_DEVICE xdma)
{
  UINT dir = H2C;
  for (ULONG ch = 0; ch < xdma->h2cChannelMax; ch++) {
    XDMA_ENGINE* engine = &(xdma->engines[ch][dir]);
    terminateThread(engine);
  }

  dir = C2H;
  for (ULONG ch = 0; ch < xdma->c2hChannelMax; ch++) {
     XDMA_ENGINE* engine = &(xdma->engines[ch][dir]);
#ifdef USE_FUSE_DMA
     FUSE_terminateThread(engine);
#else
     terminateThread(engine);
#endif
  }
}

void EngineStart(IN XDMA_ENGINE *engine) {
  engine->regs->controlW1S = XDMA_CTRL_RUN_BIT;
  TraceInfo(DBG_DMA, "%s_%u engine started (control=0x%08x)",
        DirectionToString(engine->dir), engine->channel, engine->regs->control);
}

void EngineStop(IN XDMA_ENGINE *engine) {
  engine->regs->controlW1C = XDMA_CTRL_RUN_BIT;
  TraceInfo(DBG_DMA, "%s_%u engine stopped (control=0x%08x)",
        DirectionToString(engine->dir), engine->channel, engine->regs->control);
}

void EngineEnableInterrupt(IN XDMA_ENGINE* engine) {
  if (!engine) {
    TraceError(DBG_IRQ, "engine ptr is NULL!");
    return;

  }
  engine->parentDevice->interruptRegs->channelIntEnableW1S = engine->irqBitMask;
  TraceInfo(DBG_IRQ, "%s_%u enabled interrupt", DirectionToString(engine->dir), engine->channel);
}

void EngineDisableInterrupt(IN XDMA_ENGINE* engine) {
  if (!engine) {
    TraceError(DBG_IRQ, "engine ptr is NULL");
    return;
  }
  engine->parentDevice->interruptRegs->channelIntEnableW1C = engine->irqBitMask;
  TraceInfo(DBG_IRQ, "%s_%u disabled interrupt", DirectionToString(engine->dir), engine->channel);
}

char* DirectionToString(const DirToDev dir) {
  return dir == H2C ? "H2C" : "C2H";
}

// ========================= streaming engine ============================================

// RMG: Esta función crea toda la infraestructura necesaria para trabajar con
// canales stream C2H. Se le llama en el proceso de arranque:
//    EvtDevicePrepareHardware()
//      XDMA_DeviceOpen()
//        ProbeEngines()
//          EngineCreate()
//            EngineCreateRingBuffer()



static NTSTATUS EngineCreateRingBuffer(IN XDMA_ENGINE *engine) {

  // create dma result buffer
  // RMG: No se de donde sale ese +2, pero el resto tiene sentido:
  size_t resultBufferSize = (XDMA_MAX_TRANSFER_SIZE / PAGE_SIZE + 2) * sizeof(DMA_RESULT);
  NTSTATUS status;

  // RMG: creo el buffer donde dejar la información de writeback. Se trata de
  // un WdfCommonBuffer (a memory buffer that both the driver and a DMA device
  // can access simultaneously).
  status = WdfCommonBufferCreate(engine->parentDevice->dmaEnabler,  // WDFDMAENABLER          DmaEnabler
                                 resultBufferSize,                  // size_t                 Length
                                 WDF_NO_OBJECT_ATTRIBUTES,          // PWDF_OBJECT_ATTRIBUTES Attributes
                                 &engine->ring.results);            // WDFCOMMONBUFFER        *CommonBuffer
  if (!NT_SUCCESS(status)) {
    TraceError(DBG_INIT, "WdfCommonBufferCreate failed: %!STATUS!", status);
    return status;
  }

  // RMG: get the virtual address for results
  PUCHAR resultBufferVA = (PUCHAR)WdfCommonBufferGetAlignedVirtualAddress(engine->ring.results);
  RtlZeroMemory(resultBufferVA, resultBufferSize);  // memset to 0

  TraceVerbose(DBG_INIT, "engine[%u][%u] dma result buffer @ pa=0x%08llx",
         engine->channel, engine->dir,
         WdfCommonBufferGetAlignedLogicalAddress(engine->ring.results).QuadPart);

  // RMG: Empiezo a configurar el buffer de datos:
  WDF_COMMON_BUFFER_CONFIG commonBufConfig;
  WDF_COMMON_BUFFER_CONFIG_INIT(&commonBufConfig, FILE_64_BYTE_ALIGNMENT);

  // RMG:
  // En el caso del buffer donde almacenar los datos recibidos, necesitamos
  // un buffer que sea accesible tanto para el driver como para el controlador
  // de DMA. Además, preferiblemente dicho buffer no debe aparecer fragmentado
  // en memoria. Para ello, primero tratamos de reservar un buffer sin
  // fragmentación, asignandoselo al campo 'receiveBuffer' del anillo.
  // Si esto falla, tratamos de reservar múltiples buffers de tamaño página
  // (4KB), cada uno de los cuales se asigna al campo 'rcvBuffer' de cada uno
  // de los elementos de la lista de descriptores (XMDL xmdl[]). Esto
  // representa el peor caso, es decir, un buffer totalmente fragmentado. El
  // número de elementos (XDMA_RING_NUM_BLOCKS) del anillo esta limitado por
  // la capacidad del propio controlador de DMA.

  do {
    /* Fist try for contigous common buffer allocation */
    status = WdfCommonBufferCreateWithConfig(engine->parentDevice->dmaEnabler,  // WDFDMAENABLER             DmaEnabler
                         XDMA_RING_NUM_BLOCKS * XDMA_RING_BLOCK_SIZE,           // size_t                    Length
                         &commonBufConfig,                                      // PWDF_COMMON_BUFFER_CONFIG Config
                         WDF_NO_OBJECT_ATTRIBUTES,                              // PWDF_OBJECT_ATTRIBUTES    Attributes
                         &engine->ring.receiveBuffer);                          // WDFCOMMONBUFFER           *CommonBuffer

    if (!NT_SUCCESS(status)) {
      TraceError(DBG_INIT, "WdfCommonBufferCreate failed: %!STATUS!", status);
      break;
    }

    // RMG: obtengo la dirección virtual y física:
    PUCHAR rxBufferVa = (PUCHAR)WdfCommonBufferGetAlignedVirtualAddress(engine->ring.receiveBuffer);
    PHYSICAL_ADDRESS dmaAddr = WdfCommonBufferGetAlignedLogicalAddress(engine->ring.receiveBuffer);

    RtlZeroMemory(rxBufferVa, XDMA_RING_NUM_BLOCKS * XDMA_RING_BLOCK_SIZE);

    // RMG: Inicializo los campos de direcciones y longitud en el anillo de
    // descriptores. El anillo de descriptores (campo ring) no es otra cosa
    // que un array de elementos XMDL, es decir, de descriptores. El acrónimo
    // XMDL viene de xilinx memory descriptor list, aunque realmente no es
    // una lista sino un mero elemento de la lista (o descriptor).
    // Cada descriptor consta básicamente de 3 campos:
    // - virtual address
    // - physical address
    // - length
    for (UINT i = 0; i < XDMA_RING_NUM_BLOCKS; ++i) {
      engine->ring.xmdl[i].virtAddr = rxBufferVa + (i * XDMA_RING_BLOCK_SIZE);
      engine->ring.xmdl[i].dmaAdrr.QuadPart = dmaAddr.QuadPart + (i * XDMA_RING_BLOCK_SIZE);
      engine->ring.xmdl[i].len = XDMA_RING_BLOCK_SIZE;  // RMG <= Esto no lo usan para nada.
    }

  } while (0);

  if (STATUS_INSUFFICIENT_RESOURCES == status) {
    TraceVerbose(DBG_INIT, "Allocating sparse buffers");
    /* Try allocating sparse buffers */
    for (UINT i = 0; i < XDMA_RING_NUM_BLOCKS; ++i) {
      status = WdfCommonBufferCreateWithConfig(engine->parentDevice->dmaEnabler,
                           XDMA_RING_BLOCK_SIZE,
                           &commonBufConfig,
                           WDF_NO_OBJECT_ATTRIBUTES,
                           &engine->ring.xmdl[i].rcvBuffer);
      if (!NT_SUCCESS(status)) {
        TraceError(DBG_INIT, "WdfCommonBufferCreate failed: %!STATUS!", status);
        return status;
      }

      engine->ring.xmdl[i].virtAddr = WdfCommonBufferGetAlignedVirtualAddress(engine->ring.xmdl[i].rcvBuffer);
      engine->ring.xmdl[i].dmaAdrr = WdfCommonBufferGetAlignedLogicalAddress(engine->ring.xmdl[i].rcvBuffer);
      engine->ring.xmdl[i].len = XDMA_RING_BLOCK_SIZE;  // RMG <= Esto no lo usan para nada.

      RtlZeroMemory(engine->ring.xmdl[i].virtAddr, XDMA_RING_BLOCK_SIZE);
    }
  }
  else if (!NT_SUCCESS(status)) {
    return status;
  }

  for (UINT i = 0; i < XDMA_RING_NUM_BLOCKS; ++i) {
    TraceVerbose(DBG_INIT, "sub-xmdl VA=%p, DmaAddr=%llX",
                engine->ring.xmdl[i].virtAddr,
                engine->ring.xmdl[i].dmaAdrr.QuadPart);
  }

  // RMG: inicializo el spin lock del engine
  status = WdfSpinLockCreate(WDF_NO_OBJECT_ATTRIBUTES, &engine->ring.lock);
  if (!NT_SUCCESS(status)) {
    TraceError(DBG_INIT, "WdfSpinLockCreate failed: %!STATUS!", status);
    return status;
  }

  // RMG: inicializo el 'kernel event' del engine que se utiliza para notificar
  // que se ha completado la transferencia
  KeInitializeEvent(&engine->ring.completionSignal, NotificationEvent, FALSE);

  return status;
}

// RMG: DPC (Deferred Procedure Call) para canales stream C2H

static UINT EngineProcessRing(IN XDMA_ENGINE *engine) {
  // Leo el registro de estado del engine limpiandolo tras la lectura
  UINT32 engineStatus = EngineStatus(engine, TRUE);
  if (engineStatus & XDMA_ALIGN_MISMATCH_BIT & XDMA_MAGIC_STOPPED_BIT & XDMA_FETCH_STOPPED_BIT
    & XDMA_STAT_READ_ERROR & XDMA_STAT_DESCRIPTOR_ERROR) {
    TraceError(DBG_DMA, "Engine error during transfer! 0x%08x", engineStatus);
  }
  UINT eopCount = 0;

  // RMG: obtengo el puntero al buffer de resultados (writeback info)
  DMA_RESULT *results = (DMA_RESULT*)WdfCommonBufferGetAlignedVirtualAddress(engine->ring.results);

  // ...obtengo los indices inicial y final del anillo
  WdfSpinLockAcquire(engine->ring.lock);
  UINT tail = engine->ring.tail;
  UINT head = engine->ring.head;
  WdfSpinLockRelease(engine->ring.lock);

  TraceInfo(DBG_DMA, "%s_%u ring head=%u, tail=%u, eop=%u, credits=%u",
        DirectionToString(engine->dir), engine->channel, head, tail, eopCount,
        engine->sgdma->descCredits);

  // Recorro los resultados de forma circular mientras que encuentre
  // Magic Numbers para contabilizar los EOP
  for (; results[tail].status; EngineRingAdvance(&tail)) {

    if (results[tail].status & XDMA_RESULT_EOP_BIT) {
      eopCount++;
    }

    results[tail].status = 0; // mark current dma result as processed
  }

  TraceInfo(DBG_DMA, "%s_%u ring head=%u, tail=%u, eop=%u, credits=%u",
        DirectionToString(engine->dir), engine->channel, head, tail, eopCount,
        engine->sgdma->descCredits);

  // Actualizo la cola del anillo:
  WdfSpinLockAcquire(engine->ring.lock);
  engine->ring.tail = tail;
  WdfSpinLockRelease(engine->ring.lock);

  // RMG: Si se ha completado al menos una trama se genera el evento de
  // completado para liberar el thread de lectura:

  // If any packets are completed, start the Io Read queue
  // also start the queue on an overflow since we need to tell the client that an overflow happened
  if (eopCount > 0) {
    TraceVerbose(DBG_DMA, "starting engine queue");
    KeSetEvent(&engine->ring.completionSignal, IO_NO_INCREMENT, FALSE);
  }


  // clear poll writeback buffer
  if (engine->poll) {
    XDMA_POLL_WB* wbBuffer = (XDMA_POLL_WB*)WdfCommonBufferGetAlignedVirtualAddress(engine->pollWbBuffer);
    size_t wbBufferLength = WdfCommonBufferGetLength(engine->pollWbBuffer);
    RtlZeroMemory(wbBuffer, wbBufferLength);
    engine->numDescriptors = 0;
  }

  return eopCount;
}


// RMG: Inicialización del anillo de descriptores para interface stream C2H.
// A esta función de le llama al abrir el fichero del canal DMA asociado según
// la siguiente jerarquía de llamadas:
//    EvtDeviceFileCreate()
//      EngineRingSetup()
//        EngineRingProgramDma()

static void EngineRingProgramDma(IN XDMA_ENGINE* engine) {

  // get virtual and physical pointers to descriptor buffer
  DMA_DESCRIPTOR *descriptor = (DMA_DESCRIPTOR*)WdfCommonBufferGetAlignedVirtualAddress(engine->descBuffer);
  PHYSICAL_ADDRESS nextDescLA = WdfCommonBufferGetAlignedLogicalAddress(engine->descBuffer);

  // get physical address to dma result buffer
  PHYSICAL_ADDRESS resultBufferLA = WdfCommonBufferGetAlignedLogicalAddress(engine->ring.results);

  // fill descriptors
  for (ULONG i = 0; i < XDMA_RING_NUM_BLOCKS; ++i) {
    descriptor[i].control = (XDMA_DESC_MAGIC | XDMA_DESC_EOP_BIT | XDMA_DESC_COMPLETED_BIT);
    descriptor[i].numBytes = XDMA_RING_BLOCK_SIZE;  // RMG: 4 KB (PAGE_SIZE)

    // source address are unused, will be overwritten by hardware with dma result
    descriptor[i].srcAddrLo = resultBufferLA.LowPart;
    descriptor[i].srcAddrHi = resultBufferLA.HighPart;
    resultBufferLA.QuadPart += sizeof(DMA_RESULT);

    // destination is host memory
    PHYSICAL_ADDRESS dst = engine->ring.xmdl[i].dmaAdrr;
    descriptor[i].dstAddrLo = dst.LowPart;
    descriptor[i].dstAddrHi = dst.HighPart;

    // next descriptor bus address
    nextDescLA.QuadPart += sizeof(DMA_DESCRIPTOR);  // RMG <= El buffer de descriptores NO está fisicamente fragmentado.
    descriptor[i].nextLo = nextDescLA.LowPart;
    descriptor[i].nextHi = nextDescLA.HighPart;

    if (FALSE == DescriptorIsAligned(engine, descriptor)) {
      TraceWarning(DBG_DMA, "Error: Dma Transfer is not aligned");
    }
  }

  // make the discriptor list circular
  nextDescLA = WdfCommonBufferGetAlignedLogicalAddress(engine->descBuffer);
  DMA_DESCRIPTOR* last = &descriptor[XDMA_RING_NUM_BLOCKS - 1];
  last->nextLo = nextDescLA.LowPart;
  last->nextHi = nextDescLA.HighPart;

  // Optimize for PCIe fetches
  OptimizeDescriptors(engine, descriptor, XDMA_RING_NUM_BLOCKS);

  // Print to log
  TraceVerbose(DBG_DMA, "first desc @ 0x%08x%08x",
         engine->sgdma->firstDescHi, engine->sgdma->firstDescLo);
  for (ULONG i = 0; i < XDMA_RING_NUM_BLOCKS; i++) {
    DumpDescriptor(&(descriptor[i]));
  }
  TraceVerbose(DBG_DMA, "last desc points to 0x%08x%08x", last->nextHi, last->nextLo);

  if (engine->poll) {
    engine->numDescriptors = 0;
  }

  // set initial descriptor credits for throtteling
  engine->sgdma->descCredits = XDMA_RING_NUM_BLOCKS - 1;  // RMG: Por qué N-1?
  TraceInfo(DBG_DMA, "%s_%u set %u initial descriptor credits",
        DirectionToString(engine->dir), engine->channel, engine->sgdma->descCredits);

  MemoryBarrier();

  // RMG: Arranca el engine activando el bit correspondiente del registro
  // de control del engine asociado a este canal:
  // start the engine
  EngineStart(engine);

  MemoryBarrier();
}


// RMG: Borra el buffer de resultados poniendo a cero los campos de todos los
// elementos.
// A esta función de le llama al cerrar el fichero del canal DMA asociado
// según la siguiente jerarquía de llamadas:
//    EvtFileCleanup()
//      EngineRingTeardown()
//        EngineClearDmaResults()

static void EngineClearDmaResults(IN XDMA_ENGINE *engine) {
  TraceVerbose(DBG_DMA, "clearing DMA results...");
  DMA_RESULT * results = (DMA_RESULT*)WdfCommonBufferGetAlignedVirtualAddress(engine->ring.results);
  for (UINT i = 0; i < XDMA_RING_NUM_BLOCKS; ++i) {
    results[i].status = 0;
    results[i].length = 0;
  }
}

// RMG: incremento circular para moverme por el anillo o por los resultados.
static void EngineRingAdvance(UINT* index) {
  if (*index == XDMA_RING_NUM_BLOCKS - 1) { // wrap-around
    *index = 0;
  } else { // normal increment
    ++(*index);
  }
}

// RMG: Configuración del anillo de descriptores para interface stream C2H.
// A esta función de le llama al abrir el fichero del canal DMA asociado.
// Básicamente se inicializan los índices (head & tail), se limpia el
// evento o señal de sincronización y se configuran los descriptores que
// conforman el anillo.
// Se emplea la siguiente jerarquía de llamadas:
//    EvtDeviceFileCreate()
//      EngineRingSetup()

void EngineRingSetup(IN XDMA_ENGINE *engine) {
  engine->ring.head = 0;
  engine->ring.tail = 0;
  KeClearEvent(&engine->ring.completionSignal);
  EngineRingProgramDma(engine);
}

// RMG: Es la función complementaria a EngineRingSetup(). Se ejecuta al
// cerrar el fichero asociado al canal de DMA. En primer lugar detiene
// el engine. A continuación limpia el buffer de resultados y finalmente
// pone a cero los índices del anillo (head & tail). Su jerarquía de
// llamadas es:
//    EvtFileCleanup()
//      EngineRingTeardown()

void EngineRingTeardown(IN XDMA_ENGINE *engine) {
  // RMG: Detiene el engine desactivando el bit correspondiente del registro
  // de control del engine asociado a este canal:
  EngineStop(engine);
  EngineClearDmaResults(engine);
  engine->ring.head = 0;
  engine->ring.tail = 0;
}

// RMG: A esta función se la llama desde el callback de lectura del fichero
// asociado al canal DMA.

NTSTATUS EngineRingCopyBytesToMemory(IN XDMA_ENGINE *engine, WDFMEMORY outputMem,
                   size_t length, LARGE_INTEGER timeout, size_t* bytesRead ) {
  NTSTATUS status = 0;
  if (engine->poll) { // poll mode - poll for completion
    status = EnginePollRing(engine, timeout);
    if (!NT_SUCCESS(status)) {
      goto ErrorExit;
    }
  } else { // interrupt mode - wait for completion signal
    // RMG: Esperamos el disparo del evento desde el DPC (bottom half) de
    // interrupción para procesar los descriptores del anillo:
    status = KeWaitForSingleObject(&engine->ring.completionSignal, Executive, KernelMode, FALSE, &timeout);
    if (status == STATUS_TIMEOUT) {
      goto ErrorExit;
    }
  }

  DMA_RESULT *results = (DMA_RESULT*)WdfCommonBufferGetAlignedVirtualAddress(engine->ring.results);
  size_t offset = 0;
  UINT32 numDescProcessed = 0;
  size_t numBytesRemaining = length;

  // RMG: hago una copia local de head & tail protegido por un spin lock
  WdfSpinLockAcquire(engine->ring.lock);
  UINT head = engine->ring.head;
  UINT tail = engine->ring.tail;
  WdfSpinLockRelease(engine->ring.lock);

  TraceVerbose(DBG_DMA, "%s_%u head=%u, tail=%u, credits=%u",
    DirectionToString(engine->dir), engine->channel, head, tail, engine->sgdma->descCredits);

  // RMG: Recorro los resultados (writeback info) mientras que no haya obtenido
  // todos los datos demandados en la lectura. Se considera que un resultado
  // se ha actualizado si su longitud no es nula.

  while ((head != tail) && numBytesRemaining) {

    // get dma ring buffer address and bytes transferred
    PVOID rxBufferVa = engine->ring.xmdl[head].virtAddr;
    size_t numBytesReceived = results[head].length;

    // limit buffer size
    if (numBytesReceived > numBytesRemaining) {
      numBytesReceived = numBytesRemaining;     // <= He recibido más datos de los que me piden
    } else if (numBytesReceived == 0) {
      // RMG: Primer resultado con longitud nula: limpio la señal y salgo.
      KeClearEvent(&engine->ring.completionSignal);
      break;
    }

    // RMG: Copio el contenido del descriptor sobre el buffer de salida
    // a partir del offset correspondiente

    // copy to user                 // dst      dst_offset, src,    n_bytes
    status = WdfMemoryCopyFromBuffer(outputMem, offset, rxBufferVa, numBytesReceived);
    if (!NT_SUCCESS(status)) {
      TraceError(DBG_DMA, "WdfMemoryCopyFromBuffer failed: %!STATUS!", status);
      goto ErrorExit;
    }

    // contabilizo el número de descriptores procesados
    // para luego ajustar el crédito
    numDescProcessed++;
    offset += numBytesReceived;             // ajusto el offset...
    numBytesRemaining -= numBytesReceived;  // y el número de bytes restantes

    results[head].length = 0;   // consumo el resultado...
    EngineRingAdvance(&head);   // e incremento el comienzo circularmente
  }

  // RMG: Si he consumido en su totalidad el contenido del último descriptor
  // procesado (apuntado por head) emito la señal de completado.
  if (results[head].length == 0) {
    KeClearEvent(&engine->ring.completionSignal);
  }

  // Actualizo los campos del engine y el anillo, protegiendome con
  // un spin lock, ya que los puede modificar el productor.
  WdfSpinLockAcquire(engine->ring.lock);
  engine->ring.head = head;
  engine->sgdma->descCredits = numDescProcessed;
  WdfSpinLockRelease(engine->ring.lock);

  *bytesRead = length - numBytesRemaining;

  TraceVerbose(DBG_DMA, "%s_%u read %lluB available,  head=%u, tail=%u, credits=%u",
         DirectionToString(engine->dir), engine->channel, *bytesRead, head, tail,
         engine->sgdma->descCredits);

  // RMG, NOTA:
  // Si el último descriptor procesado dispone de mas bytes de los que se
  // necesitan, es decir, si no se consume en su totalidad, se deja ahí sin
  // consumir (ni se copian ni se contabilizan) ni uno solo de sus bytes.
  // Cuando se reciban más datos, en una lectura posterior se consumirá en
  // su totalidad.
  //    => Los descriptores se han de consumir en su totalidad.

ErrorExit:
  return status;
}

//========================= polling interface =====================================================

// RMG: Estos solo se usa cuando trabajamos con modo polling. En este caso,
// el buffer tiene un tamaño de 8 word32 aunque solo se usa la primera para
// indicar cuántos descriptores se han procesado.


static NTSTATUS EngineCreatePollWriteBackBuffer(IN OUT XDMA_ENGINE *engine) {
  // allocate host-side buffer for poll mode descriptor write back info

  NTSTATUS status = WdfCommonBufferCreate(engine->parentDevice->dmaEnabler, sizeof(XDMA_POLL_WB),
                      WDF_NO_OBJECT_ATTRIBUTES, &engine->pollWbBuffer);
  if (!NT_SUCCESS(status)) {
    TraceError(DBG_INIT, "WdfCommonBufferCreate failed: %!STATUS!", status);
    return status;
  }

  PHYSICAL_ADDRESS wbBufferLA = WdfCommonBufferGetAlignedLogicalAddress(engine->pollWbBuffer);
  PUCHAR wbBufferVA = (PUCHAR)WdfCommonBufferGetAlignedVirtualAddress(engine->pollWbBuffer);
  RtlZeroMemory(wbBufferVA, sizeof(XDMA_POLL_WB));

  engine->regs->pollModeWbLo = wbBufferLA.LowPart;
  engine->regs->pollModeWbHi = wbBufferLA.HighPart;

  TraceInfo(DBG_INIT, "poll wb buffer at 0x%08x%08x, size=%lld",
        engine->regs->pollModeWbHi, engine->regs->pollModeWbLo, sizeof(XDMA_POLL_WB));
  return status;
}

static NTSTATUS EngineWaitForCompletion(IN XDMA_ENGINE* engine, IN LARGE_INTEGER timeout) {

  NTSTATUS status = KeWaitForSingleObject(&engine->completionWaitSignal, Executive, KernelMode, FALSE, &timeout);
  if (status == STATUS_TIMEOUT) {
    TraceError(DBG_DMA, "Request timed out.");
    WdfSpinLockAcquire(engine->engineLock);
    if (FALSE == engine->isReqPending) {
      /* Wait has timed out but request is been cleared by the DPC. So return success. */
      TraceInfo(DBG_DMA, "Request is already processed.");
      WdfSpinLockRelease(engine->engineLock);
      KeClearEvent(&engine->completionWaitSignal);
      return STATUS_SUCCESS;
    }
    else {
      /* Wait has timed out and still request is not processed.
       * Remove pending flag from engine so DPC wont proceed further to complete the request.
       */
      TraceInfo(DBG_DMA, "Request really timed out.");
      engine->isReqPending = FALSE;
      EngineStatus(engine, TRUE);
      EngineStop(engine);
      WdfSpinLockRelease(engine->engineLock);
      return status;
    }
  }

  KeClearEvent(&engine->completionWaitSignal);
  return STATUS_SUCCESS;
}

static NTSTATUS EnginePollTransfer(IN XDMA_ENGINE* engine, IN LARGE_INTEGER timeout) {

  XDMA_POLL_WB* writeback_data = (XDMA_POLL_WB*)WdfCommonBufferGetAlignedVirtualAddress(engine->pollWbBuffer);
  const ULONG expected = engine->numDescriptors;
  volatile ULONG actual = 0;
  LARGE_INTEGER timeNow;

  KeQuerySystemTime(&timeNow);
  LARGE_INTEGER timeEnd = timeNow;

  // Expecting only Relative(-ve) value to current time.
  if (timeout.QuadPart < 0)
    timeEnd.QuadPart += (-timeout.QuadPart);
  else
    timeEnd.QuadPart += (timeout.QuadPart);

  do {
    actual = writeback_data->completedDescCount;

    if (actual & XDMA_WB_ERR_MASK) {
      TraceError(DBG_DMA, "error on writeback %u", actual);
      unmarkReqPending(engine);
      return STATUS_INTERNAL_ERROR;
    }
    actual &= XDMA_WB_COUNT_MASK;

    if (actual > expected) {
      actual = expected | XDMA_WB_ERR_MASK;
    }

    KeQuerySystemTime(&timeNow);
    if (timeNow.QuadPart >= timeEnd.QuadPart) {
      TraceError(DBG_DMA, "Request timed out. Completed desc %u", actual);
      unmarkReqPending(engine);
      return STATUS_TIMEOUT;
    }
  } while (expected != actual);

  TraceVerbose(DBG_DMA, "%u descriptors completed", actual);

  EngineProcessTransfer(engine);

  return STATUS_SUCCESS;
}

NTSTATUS EnginePollRing(IN XDMA_ENGINE* engine, IN LARGE_INTEGER timeout) {
  XDMA_POLL_WB* writeback_data = (XDMA_POLL_WB*)WdfCommonBufferGetAlignedVirtualAddress(engine->pollWbBuffer);
  volatile ULONG completed = 0;
  volatile UINT eopCount = 0;
  LARGE_INTEGER timeNow;
  KeQuerySystemTime(&timeNow);
  LARGE_INTEGER timeEnd = timeNow;
  timeEnd.QuadPart += (-timeout.QuadPart);

  TraceVerbose(DBG_DMA, "POLLING FOR %llu TIME", timeEnd.QuadPart);

  do {
    completed = writeback_data->completedDescCount;

    if (completed & XDMA_WB_ERR_MASK) {
      TraceError(DBG_DMA, "error on writeback %u", completed);
      return STATUS_INTERNAL_ERROR;
    }
    if (completed) {
      eopCount = EngineProcessRing(engine);
      TraceVerbose(DBG_DMA, "complete=%u, eop=%u", completed, eopCount);
    }

    KeQuerySystemTime(&timeNow);
    if (timeNow.QuadPart >= timeEnd.QuadPart) {
      TraceError(DBG_DMA, "Request timed out");
      return STATUS_TIMEOUT;
    }
  } while (!eopCount);

  TraceVerbose(DBG_DMA, "complete=%u, eop=%u", completed, eopCount);

  return STATUS_SUCCESS;
}

//========================= performance counters interface ========================================

void EngineStartPerf(IN XDMA_ENGINE* engine) {
  ASSERTMSG("argument engine is NULL!", engine != NULL);

  // Automatically stops performance counters when a descriptor with the stop bit is completed.
  engine->regs->perfCtrl = XDMA_PERF_CLEAR;
  engine->regs->perfCtrl = XDMA_PERF_AUTO | XDMA_PERF_RUN;
}

void EngineGetPerf(IN XDMA_ENGINE* engine, OUT XDMA_PERF_DATA* perfData) {
  TraceVerbose(DBG_DMA, "cycleCount=0x%08x%08x dataCount=0x%08x%08x pendingCount=0x%08x%08x",
         engine->regs->perfCycHi, engine->regs->perfCycLo,
         engine->regs->perfDatHi, engine->regs->perfDatLo,
         engine->regs->perfPndHi, engine->regs->perfPndLo);

  ASSERTMSG("argument engine is NULL!", engine != NULL);
  ASSERTMSG("argument perfData is NULL!", perfData != NULL);

  perfData->clockCycleCount = ((UINT64)engine->regs->perfCycHi << 32) + engine->regs->perfCycLo;
  perfData->dataCycleCount = ((UINT64)engine->regs->perfDatHi << 32) + engine->regs->perfDatLo;
  perfData->pendingCount = ((UINT64)engine->regs->perfPndHi << 32) + engine->regs->perfPndHi;
}


// RMG:

//---------------------------------------------------------

void XDMA_EngineSetPollMode(XDMA_ENGINE* engine, BOOLEAN pollMode) {

  EXPECT(engine != NULL);

  if (engine->enabled == TRUE) {
    if (pollMode) {
      //EngineDisableInterrupt(engine);
      engine->regs->controlW1S = XDMA_CTRL_POLL_MODE;
      engine->regs->controlW1C = XDMA_CTRL_IE_ALL;
    } else {
      engine->regs->controlW1C = XDMA_CTRL_POLL_MODE;
      engine->regs->controlW1S = XDMA_CTRL_IE_ALL;
      //EngineEnableInterrupt(engine);
    }
    engine->poll = pollMode;
  }
}

//-----------------------------------------------------------------------------

static void FUSE_EngineProcessTransfer(IN XDMA_ENGINE *engine);
VOID FUSE_CompletionThread(PVOID context);



static void FUSE_printDesc(IN const DMA_DESCRIPTOR *desc) {
//  PHYSICAL_ADDRESS src, dst, next;
//  src.LowPart = desc->srcAddrLo; src.HighPart = desc->srcAddrHi; 
//  dst.LowPart = desc->dstAddrLo; dst.HighPart = desc->dstAddrHi; 
//  next.LowPart = desc->nextAddrLo; next.HighPart = desc->nextAddrHi; 
  UINT32 n_pages;
  n_pages = desc->numBytes / PAGE_SIZE;
  KdPrint(("  * descriptor: control = 0x%8.8X, length = %6d (%3d pages), src = 0x%8.8X_%8.8X, src = 0x%8.8X_%8.8X, src = 0x%8.8X_%8.8X\n", 
    desc->control, desc->numBytes, n_pages,
    desc->srcAddrHi, desc->srcAddrLo,
    desc->dstAddrHi, desc->dstAddrLo,
    desc->nextHi, desc->nextLo));
}

static void FUSE_printDescRing(IN const DMA_DESCRIPTOR *desc, IN ULONG n, IN UINT verbose) {
  UINT32 n_pages;
  UINT32 total_length = 0;
  dbg_printf(("RMG: [%s] +\n", __FUNCTION__));
  
  for (ULONG i = 0; i < n; i++) {
    n_pages = desc[i].numBytes / PAGE_SIZE;
    total_length += desc[i].numBytes;
    if(verbose) {
      KdPrint(("   - descriptor %3d: control = 0x%8.8X, length = %8d (%3d pages), src = 0x%8.8X_%8.8X, dst = 0x%8.8X_%8.8X, next = 0x%8.8X_%8.8X\n", 
        i, desc[i].control, desc[i].numBytes, n_pages,
        desc[i].srcAddrHi, desc[i].srcAddrLo,
        desc[i].dstAddrHi, desc[i].dstAddrLo,
        desc[i].nextHi, desc[i].nextLo));
    }
  }  
  KdPrint(("   => total_length = %d\n", total_length)); 
  dbg_printf(("RMG: [%s] -\n", __FUNCTION__));
}

static void FUSE_EngineConfigureInterrupt(IN OUT XDMA_ENGINE *engine, IN UINT index, IN ULONG engineId) {
  dbg_printf(("RMG: [%s] +\n", __FUNCTION__));
  // engine interrupt request bit(s) - interrupt bit depends on number of engines present
  // ver Figure 9: Packing H2C and C2H
  engine->irqBitMask = (1 << XDMA_ENG_IRQ_NUM) - 1;
  engine->irqBitMask <<= (engineId * XDMA_ENG_IRQ_NUM);

  dbg_printf(("index = %d, engineId = %d\n", index, engineId));
  
  // bind msi interrupt context with this engine
  if (engine->parentDevice->channelInterrupts[index] != NULL) {
    PIRQ_CONTEXT irqContext = GetIrqContext(engine->parentDevice->channelInterrupts[index]);
    irqContext->engine = engine;
  }

  // Habilitamos todas las posibles causas de interrupción:
  UINT32 regVal;
  regVal =  XDMA_CTRL_IE_DESC_STOPPED       |
            XDMA_CTRL_IE_DESC_COMPLETED     |
            XDMA_CTRL_IE_ALIGNMENT_MISMATCH |
            XDMA_CTRL_IE_MAGIC_STOPPED      |
            XDMA_CTRL_IE_INVALID_LENGTH     |
//XDMA_CTRL_IE_IDLE_STOPPED       |
            XDMA_CTRL_IE_READ_ERROR         |
            XDMA_CTRL_IE_WRITE_ERROR        |
            XDMA_CTRL_IE_DESCRIPTOR_ERROR;

  engine->regs->intEnableMaskW1S = regVal;
  engine->regs->controlW1S = regVal | XDMA_CTRL_IE_IDLE_STOPPED;
  TraceVerbose(DBG_INIT, "engineIrqBitMask=0x%08x, intEnableMask=0x%08x", engine->irqBitMask, engine->regs->intEnableMask);
  
  dbg_printf(("engineIrqBitMask=0x%08x, intEnableMask=0x%08x\n", engine->irqBitMask, engine->regs->intEnableMask));
  
  dbg_printf(("RMG: [%s] -\n", __FUNCTION__));
}

// RMG: Estos solo se usa cuando trabajamos con modo polling. En este caso,
// el buffer tiene un tamaño de 8 word32 aunque solo se usa la primera para
// indicar cuántos descriptores se han procesado.
static NTSTATUS FUSE_EngineCreatePollWriteBackBuffer(IN OUT XDMA_ENGINE *engine) {
  // allocate host-side buffer for poll mode descriptor write back info

  NTSTATUS status;
  status = WdfCommonBufferCreate( engine->parentDevice->dmaEnabler,
                                  sizeof(XDMA_POLL_WB),
                                  WDF_NO_OBJECT_ATTRIBUTES,
                                  &engine->pollWbBuffer);
  if (!NT_SUCCESS(status)) {
    TraceError(DBG_INIT, "WdfCommonBufferCreate failed: %!STATUS!", status);
    return status;
  }

  PHYSICAL_ADDRESS bufferPhyAddr;
  void *buffer;

  bufferPhyAddr = WdfCommonBufferGetAlignedLogicalAddress(engine->pollWbBuffer);
  buffer = (void *) WdfCommonBufferGetAlignedVirtualAddress(engine->pollWbBuffer);
  RtlZeroMemory(buffer, sizeof(XDMA_POLL_WB));

  engine->regs->pollModeWbLo = bufferPhyAddr.LowPart;
  engine->regs->pollModeWbHi = bufferPhyAddr.HighPart;

  TraceInfo(DBG_INIT, "poll wb buffer at 0x%08x%08x, size=%lld",
    engine->regs->pollModeWbHi, engine->regs->pollModeWbLo, sizeof(XDMA_POLL_WB));

  return status;
}

static NTSTATUS FUSE_EngineCreateWrBackBuffer(IN OUT XDMA_ENGINE *engine, IN UINT32 N_desc) {
  SIZE_T bufferSize;
  bufferSize = N_desc * sizeof(DMA_RESULT);

  NTSTATUS status;
  status = WdfCommonBufferCreate(engine->parentDevice->dmaEnabler,
                                 bufferSize,
                                 WDF_NO_OBJECT_ATTRIBUTES,
                                 &engine->wrBackBuffer);
  if (!NT_SUCCESS(status)) {
    TraceError(DBG_INIT, "WdfCommonBufferCreate failed: %!STATUS!", status);
    return status;
  }

  PHYSICAL_ADDRESS bufferPhyAddr;
  void *buffer;

  bufferPhyAddr = WdfCommonBufferGetAlignedLogicalAddress(engine->wrBackBuffer);
  buffer = (void *) WdfCommonBufferGetAlignedVirtualAddress(engine->wrBackBuffer);
  RtlZeroMemory(buffer, bufferSize);

  TraceVerbose(DBG_INIT, "engine[%u][%u] dma wrback buffer @ pa=0x%08llx",
    engine->channel, engine->dir, bufferPhyAddr.QuadPart);

  return status;
}

static NTSTATUS FUSE_EngineCreateDescriptorBuffer(IN OUT XDMA_ENGINE *engine, IN UINT32 N_desc) {
  SIZE_T bufferSize;
  bufferSize = N_desc * sizeof(DMA_DESCRIPTOR);

  NTSTATUS status;
  status = WdfCommonBufferCreate(engine->parentDevice->dmaEnabler,
                                 bufferSize,
                                 WDF_NO_OBJECT_ATTRIBUTES,
                                 &engine->descBuffer);
  if (!NT_SUCCESS(status)) {
    TraceError(DBG_INIT, "WdfCommonBufferCreate failed: %!STATUS!", status);
    return status;
  }

  engine->capacity = N_desc;

  PHYSICAL_ADDRESS descPhyAddr;
  DMA_DESCRIPTOR *desc;


  descPhyAddr = WdfCommonBufferGetAlignedLogicalAddress(engine->descBuffer);
  desc = (DMA_DESCRIPTOR *) WdfCommonBufferGetAlignedVirtualAddress(engine->descBuffer);
  RtlZeroMemory(desc, bufferSize);

  // give hw the physical start address of the descriptor desc
  engine->sgdma->firstDescLo = descPhyAddr.LowPart;
  engine->sgdma->firstDescHi = descPhyAddr.HighPart;
  engine->sgdma->firstDescAdj = 0; // depends on transfer - set later in ProgramDMA

  // initialize the descriptors source address to point to their writeback desc:
  if ((engine->type == EngineType_ST) && (engine->dir == C2H)) {
    PHYSICAL_ADDRESS wrBackPhyAddr;
    wrBackPhyAddr = WdfCommonBufferGetAlignedLogicalAddress(engine->wrBackBuffer);
    for (ULONG i = 0; i < N_desc; i++) {
      desc[i].srcAddrLo = wrBackPhyAddr.LowPart;
      desc[i].srcAddrHi = wrBackPhyAddr.HighPart;
      // next wrBack physical address
      wrBackPhyAddr.QuadPart += sizeof(DMA_RESULT);
    }
  }

  TraceVerbose(DBG_INIT, "descriptor buffer at 0x%08x%08x, size=%lld, capacity=%u",
      engine->sgdma->firstDescHi, engine->sgdma->firstDescLo,
      bufferSize, engine->capacity);

  return status;
}

static NTSTATUS FUSE_initThread(XDMA_ENGINE* engine) {
  engine->terminate = FALSE;
  KeInitializeSemaphore(&engine->semaphore, // PRKSEMAPHORE Semaphore,
                        0,                  // LONG         Count,
                        MAXLONG);           // LONG         Limit

  TraceInfo(DBG_INIT, "Creating thread : %u", engine->channel);

  NTSTATUS status;
  status = PsCreateSystemThread(&engine->thHandle,      // PHANDLE            ThreadHandle,
                                (ACCESS_MASK)0,         // ULONG              DesiredAccess,
                                NULL,                   // POBJECT_ATTRIBUTES ObjectAttributes,
                                (HANDLE)0,              // HANDLE             ProcessHandle,
                                NULL,                   // PCLIENT_ID         ClientId,
                                FUSE_CompletionThread,  // PKSTART_ROUTINE    StartRoutine,
                                engine);                // PVOID              StartContext

  if (!NT_SUCCESS(status)) {
    TraceError(DBG_INIT, "Failed to create thread [%u][%u] - %!STATUS!", engine->dir, engine->channel, status);
    return status;
  }

  // RMG: returns the corresponding pointer to the object's body.
  ObReferenceObjectByHandle(engine->thHandle,   // HANDLE                     Handle,
                            THREAD_ALL_ACCESS,  // ACCESS_MASK                DesiredAccess,
                            NULL,               // POBJECT_TYPE               ObjectType,
                            KernelMode,         // KPROCESSOR_MODE            AccessMode,
                            &engine->thObject,  // PVOID                      *Object,
                            NULL);              // POBJECT_HANDLE_INFORMATION HandleInformation

  // RMG: closes an object handle. Closing an open object handle causes that
  // handle to become invalid. A driver must close every handle that it opens
  // as soon as the handle is no longer required.
  ZwClose(engine->thHandle);

  engine->thInitialized = TRUE;
  return status;
}

static void FUSE_terminateThread(XDMA_ENGINE *engine) {
  TraceInfo(DBG_INIT, "Terminating thread : %u", engine->channel);

  if (FALSE == engine->thInitialized)
    return;

  if (engine->thObject) {
    engine->terminate = TRUE;
    KeReleaseSemaphore(&engine->semaphore, 0, 1, FALSE);

    KeWaitForSingleObject(engine->thObject,
      Executive,
      KernelMode,
      FALSE,
      NULL);

    ObDereferenceObject(engine->thObject);
  }
}

static NTSTATUS FUSE_EngineWaitForCompletion(IN XDMA_ENGINE *engine, IN LARGE_INTEGER timeout) {
  dbg_printf((" || Thread: [%s] esperando en completionWaitSignal\n", __FUNCTION__));

  NTSTATUS status = KeWaitForSingleObject(&engine->completionWaitSignal, Executive, KernelMode, FALSE, &timeout);
  if (status == STATUS_TIMEOUT) {
    TraceError(DBG_DMA, "Request timed out.");
    dbg_printf((" || Thread: [%s] Request timed out.\n", __FUNCTION__));
    
    WdfSpinLockAcquire(engine->engineLock);
    if (engine->isReqPending == FALSE) {
      // Wait has timed out but request is been cleared by the DPC. So return success.
      TraceInfo(DBG_DMA, "Request is already processed.");
      WdfSpinLockRelease(engine->engineLock);
      dbg_printf((" || Thread: [%s] Request is already processed.\n", __FUNCTION__));
      KeClearEvent(&engine->completionWaitSignal);
      return STATUS_SUCCESS;
    }
    else {
      // Wait has timed out and still request is not processed.
      // Remove pending flag from engine so DPC wont proceed further to complete the request.
      TraceInfo(DBG_DMA, "Request really timed out.");
      engine->isReqPending = FALSE;
      EngineStatus(engine, TRUE);
      EngineStop(engine);
      WdfSpinLockRelease(engine->engineLock);
      dbg_printf((" || Thread: [%s] Request really timed out.\n", __FUNCTION__));
      return status;
    }
  }
  else {
    dbg_printf((" || Thread: [%s] Recibido evento en completionWaitSignal\n", __FUNCTION__));
  }

  KeClearEvent(&engine->completionWaitSignal);
  
  return STATUS_SUCCESS;
}


VOID FUSE_CompletionThread(PVOID context) {
  XDMA_ENGINE *engine = (XDMA_ENGINE *)context;
  
  // processor representa el procesador al que voy a asociar este thread.
  // Primero obtengo el índice del canal. A continuación obtengo el número
  // N de procesadores disponibles (mediante KeQueryActiveProcessorCount()).
  // Con esta información asigno el thread a uno de los N procesadores del
  // sistema según la expresión proc_idx = channel_idx % N_processors. La
  // asignación se hace con KeSetSystemAffinityThread().
  
  ULONG channel_idx;
  ULONG processor;

  channel_idx = engine->channel;
  if (engine->dir == C2H)
    channel_idx += engine->parentDevice->h2cChannelMax;

  processor = (channel_idx % KeQueryActiveProcessorCount(NULL));
  TraceVerbose(DBG_INIT, "Active thread ID : %u", processor);
  KAFFINITY affinity = (KAFFINITY)1 << processor;
  KeSetSystemAffinityThread(affinity);

  while (1) {
    // Espera hasta que concluya la programación del DMA:
    KeWaitForSingleObject(&engine->semaphore,
                          Executive,
                          KernelMode,
                          FALSE,
                          NULL);
  
    dbg_printf((" || Thread: [%s] Liberado semaforo\n", __FUNCTION__));
    if (engine->terminate) {
      TraceInfo(DBG_INIT, "Terminating thread : %u", engine->channel);
      PsTerminateSystemThread(STATUS_SUCCESS);
      return;
    }

    // Si hay una transferencia pendiente fijamos un plazo de 10 segundos para
    // que concluya la transferencia. La espera se lleva a cabo en la función
    // EngineWaitForCompletion(). Lo primero que se hace en esta función es
    // suspenderse hasta que se reciba un evento en completionWaitSignal desde
    // el DPC. Si se supera el plazo de espera, se detiene el engine
    if (isReqPending(engine) == TRUE) {
      LARGE_INTEGER timeout;
      timeout.QuadPart = -TIMEOUT_1_MS * engine->timeout_ms;  // timeout (relative time, in 100-nanosecond units)
      //KdPrint(("[%s] timeout = %I64d\n", __FUNCTION__, timeout.QuadPart));
      //timeout.QuadPart = -10 * 10000000; // 10 seconds timeout

      NTSTATUS status = FUSE_EngineWaitForCompletion(engine, timeout);
      if (STATUS_SUCCESS != status) {
        WDFREQUEST request = WdfDmaTransactionGetRequest(engine->dmaTransaction);
        if (request) {
          TraceError(DBG_DMA, "EngineWaitForCompletion failed: %!STATUS!", status);
          WdfDmaTransactionRelease(engine->dmaTransaction);
          WdfRequestComplete(request, status);
        }
      }
    }
  }
}


static NTSTATUS FUSE_EngineCreate(PXDMA_DEVICE xdma,
                                  XDMA_ENGINE *engine,
                                  DirToDev dir,
                                  ULONG channel,
                                  ULONG engineIndex,
                                  ULONG engineId)
{

  NTSTATUS status;
  dbg_printf(("RMG: [%s] +\n", __FUNCTION__));

  engine->parentDevice = xdma;
  engine->channel = channel;
  engine->dir = dir;
  const ULONG offset = (dir * BLOCK_OFFSET) + (channel * ENGINE_OFFSET);
  PUCHAR configBarAddr = (PUCHAR)xdma->bar[xdma->configBarIdx];
  engine->regs = (XDMA_ENGINE_REGS*)(configBarAddr + offset);
  engine->sgdma = (XDMA_SGDMA_REGS*)(configBarAddr + offset + SGDMA_BLOCK_OFFSET);

  // AXI-MM or AXI-ST? 0 = MM, 1 = ST
  engine->type = (engine->regs->identifier & XDMA_ID_ST_BIT) != 0;

  // Incremental or Non-Incremental address mode? 0 = inc, 1=non-inc
  engine->addressMode = (engine->regs->control & XDMA_CTRL_NON_INCR_ADDR) != 0;

  // set interrupt sources
  FUSE_EngineConfigureInterrupt(engine, engineIndex, engineId);

  // create common buffer for poll mode descriptor write back - if used
  status = FUSE_EngineCreatePollWriteBackBuffer(engine);
  if (!NT_SUCCESS(status)) {
    TraceError(DBG_INIT, "EngineCreatePollWriteBackBuffer() failed: %!STATUS!", status);
    return status;
  }

  // capture alignment requirements
  EngineGetAlignments(engine);

  // RMG:
  // Si el tamaño del buffer es múltiplo del de una página (4K) y está
  // perfectamente alineado sobre el comienzo de una página el número
  // de páginas sería BUF_SIZE/4K. Ahora bien:
  // - Si el tamaño no es múltiplo hay que incrementar BUF_SIZE/4K en 1.
  // - Si no está alineado hay que incrementar BUF_SIZE/4K en 1.
  UINT32 N_desc = (FUSE_DMA_MAX_TRANSFER_SIZE/PAGE_SIZE) + 2;
  dbg_printf(("RMG: [%s] N_desc = %d\n", __FUNCTION__, N_desc));

  // Creacion del buffer de resultados (wrBackBuffer):
  if ((engine->type == EngineType_ST) && (engine->dir == C2H)) {
    status = FUSE_EngineCreateWrBackBuffer(engine, N_desc);
    if (!NT_SUCCESS(status)) {
      TraceError(DBG_INIT, "FUSE_EngineCreateWrBackBuffer() failed: %!STATUS!", status);
      return status;
    }
  }

  // create and bind dma desciptor buffer to hw
  status = FUSE_EngineCreateDescriptorBuffer(engine, N_desc);
  if (!NT_SUCCESS(status)) {
    TraceError(DBG_INIT, "FUSE_EngineCreateDescriptorBuffer() failed: %!STATUS!", status);
    return status;
  }

  // allocate wdf dma transaction object
  status = WdfDmaTransactionCreate(xdma->dmaEnabler, WDF_NO_OBJECT_ATTRIBUTES, &engine->dmaTransaction);
  if (!NT_SUCCESS(status)) {
    TraceError(DBG_INIT, "WdfDmaTransactionCreate() failed: %!STATUS!", status);
    return status;
  }

  engine->work = FUSE_EngineProcessTransfer;

  // Initialize completion wait and engine lock
  engine->isReqPending = FALSE;

  status = WdfSpinLockCreate(WDF_NO_OBJECT_ATTRIBUTES, &engine->engineLock);
  if (!NT_SUCCESS(status)) {
    TraceError(DBG_INIT, "WdfSpinLockCreate failed: %!STATUS!", status);
    return status;
  }

  KeInitializeEvent(&engine->completionWaitSignal, NotificationEvent, FALSE);

  status = FUSE_initThread(engine);
  if (!NT_SUCCESS(status)) {
    TraceError(DBG_INIT, "initThread failed: %!STATUS!", status);
    return status;
  }

  engine->enabled = TRUE;
//  if (1) return STATUS_UNSUCCESSFUL;

  dbg_printf(("RMG: [%s] -\n", __FUNCTION__));
  return status;
}



#if 0

typedef union _LARGE_INTEGER {
    struct {
        ULONG LowPart;
        LONG HighPart;
    } DUMMYSTRUCTNAME;
    struct {
        ULONG LowPart;
        LONG HighPart;
    } u;
    LONGLONG QuadPart;
} LARGE_INTEGER;

typedef LARGE_INTEGER PHYSICAL_ADDRESS, *PPHYSICAL_ADDRESS;

#endif

static int check_sg_list(IN PSCATTER_GATHER_LIST SgList) {
  int adjacent = 0;
  PHYSICAL_ADDRESS prev_addr;
  ULONG prev_len;
  for (ULONG i = 1; i < SgList->NumberOfElements; i++) {
    prev_addr = SgList->Elements[i-1].Address;
    prev_len = SgList->Elements[i-1].Length;
    if ((prev_addr.QuadPart + prev_len) == SgList->Elements[i].Address.QuadPart) {
      adjacent++;
    }
  }

  int multipage = 0;
  for (ULONG i = 0; i < SgList->NumberOfElements; i++) {
    if (SgList->Elements[i].Length > (4*1024)) {
      multipage++;
    }
  }
  dbg_printf(("RMG: [%s] n_sg = %ld, adjacent = %d, multipage = %d\n", __FUNCTION__, SgList->NumberOfElements, adjacent, multipage));
  return adjacent;
}


//  UNREFERENCED_PARAMETER(Transaction);


//WDF_DMA_ENABLER_CONFIG_INIT

BOOLEAN FUSE_EngineProgramDma(
              IN WDFDMATRANSACTION Transaction,
              IN WDFDEVICE Device,
              IN WDFCONTEXT context,
              IN WDF_DMA_DIRECTION Direction,
              IN PSCATTER_GATHER_LIST SgList)
{
  UNREFERENCED_PARAMETER(Device);
//  UNREFERENCED_PARAMETER(SgList);
//  UNREFERENCED_PARAMETER(Direction);
//  UNREFERENCED_PARAMETER(context);

  dbg_printf(("RMG: [%s] +\n", __FUNCTION__));

  XDMA_ENGINE *engine = (XDMA_ENGINE *) context;
  WDFREQUEST request = WdfDmaTransactionGetRequest(Transaction);

  if (Direction == WdfDmaDirectionWriteToDevice) {
    TraceError(DBG_INIT, "FUSE only uses C2H channels");
    goto ErrExit;
  }
  
#ifdef FUSE_CHECK
  check_sg_list(SgList);
#endif

  // El número de elementos de elementos de la SG list lo gestiona Windows
  // automáticamente en función del tamaño máximo de transacción definido
  // mediante la función WDF_DMA_ENABLER_CONFIG_INIT() en XDMA_DeviceOpen.
  // Si una transacción supera el tamaño de una transferencia, Windows
  // dividirá automáticamente la transacción en varias transferencias y
  // llamando a esta función sucesivas veces.
  // El buffer destino lo trocea y en cada una de esas llamadas le pasa
  // una SG actualizada.

  // En cualquier caso, comprobemos que no se supera el límite establecido
  if (SgList->NumberOfElements > engine->capacity) {
    TraceError(DBG_INIT, "Too many descriptors...");
    KdPrint(("RMG: Too many descriptors... capacity = %d\n", engine->capacity));
    goto ErrExit;
  }


  WDF_REQUEST_PARAMETERS params;
  WDF_REQUEST_PARAMETERS_INIT(&params);
  WdfRequestGetParameters(request, &params);
  LONGLONG deviceOffset;
  size_t n_bytes;

  deviceOffset = (SIZE_T)params.Parameters.Read.DeviceOffset;
  n_bytes = WdfDmaTransactionGetBytesTransferred(Transaction);
  deviceOffset += n_bytes;

#ifdef FUSE_CHECK
  if (1) {
    size_t transferLen;
    size_t readLen;
    transferLen = WdfDmaTransactionGetCurrentDmaTransferLength(Transaction);
    readLen = params.Parameters.Read.Length;
    if (!n_bytes) {
      dbg_printf(("   - First transfer:\n"));
      dbg_printf(("      * transferLen = %ld\n", transferLen));
      dbg_printf(("      * readLen = %ld\n", readLen));
    }
  }
#endif

  DMA_DESCRIPTOR *desc = (DMA_DESCRIPTOR *) WdfCommonBufferGetAlignedVirtualAddress(engine->descBuffer);
  PHYSICAL_ADDRESS descPhyAddr = WdfCommonBufferGetAlignedLogicalAddress(engine->descBuffer);

#if 1  
  WdfSpinLockAcquire(engine->engineLock);
  // Comprueba si hay una transacción pendiente!
  if (engine->isReqPending == TRUE) {
    WdfSpinLockRelease(engine->engineLock);
    ASSERTMSG("Request is already pending", (1 == 1));  // RMG: Esto nunca hace nada!!!
  }
  else {
    WdfSpinLockRelease(engine->engineLock);
  }
#endif


////// offset into the transaction (if it is split)
////numBytesTransferred = WdfDmaTransactionGetBytesTransferred(Transaction);
////deviceOffset += numBytesTransferred;


  for (ULONG i = 0; i < SgList->NumberOfElements; i++) {
    desc[i].control = XDMA_DESC_MAGIC;
    desc[i].numBytes = SgList->Elements[i].Length;
    desc[i].dstAddrLo = SgList->Elements[i].Address.LowPart;
    desc[i].dstAddrHi = SgList->Elements[i].Address.HighPart;

//    desc[i].control |= XDMA_DESC_EOP_BIT;
    
    // next descriptor bus address
    descPhyAddr.QuadPart += sizeof(DMA_DESCRIPTOR);

    // RMG: Si es el último descriptor fijo su campo next_addr a 0, activo sus
    // bits de STOP y COMPLETED para que al concluir su procesameinto se
    // detenga y se genere una interrupción. Si no es el último simplemente
    // fijo su campo next_addr para que apunte al siguiente descriptor.

    // RMG: Esto podría ser precalculado!!!!!
    if (i < (SgList->NumberOfElements-1)) {
      desc[i].nextLo = descPhyAddr.LowPart;
      desc[i].nextHi = descPhyAddr.HighPart;
    }
    else { // last descriptor
      desc[i].nextLo = 0;
      desc[i].nextHi = 0;
      // stop engine and request an interrupt from the engine
      desc[i].control |= (XDMA_DESC_STOP_BIT | XDMA_DESC_COMPLETED_BIT);
      //desc[i].control |= XDMA_DESC_EOP_BIT;
    }

    // incremental address mode
    deviceOffset += SgList->Elements[i].Length;

    if (DescriptorIsAligned(engine, &(desc[i])) == FALSE) {
      TraceWarning(DBG_DMA, "Error: Dma Transfer is not aligned");
      dbg_printf(("Error: Dma Transfer is not aligned\n"));
    }
  }

  
  // RMG: Gestion del campo next_adj para optimizar la prebúsqueda de descriptores
  // OJO!!!!!!: Si la ubicación de los descriptores en memoria es fija, esto se
  // debería poder precalcular, al menos en parte!!!!
  
  OptimizeDescriptors(engine, desc, SgList->NumberOfElements);

  dbg_printf(("   - n_used_desc = %d\n", SgList->NumberOfElements));
  
#ifdef FUSE_CHECK
  FUSE_printDescRing(desc, SgList->NumberOfElements, 0); 
#endif
  
  MemoryBarrier();


  markReqPending(engine); // Activa el flag de transferencia pendiente
  EngineStart(engine);    // Arranca el engine  !!!
  MemoryBarrier();

//  LARGE_INTEGER timeout;
//  timeout.QuadPart = -10 * 10000000; // 10 seconds timeout
  NTSTATUS status = STATUS_SUCCESS;

//  if (1) goto ErrExit;
  
  if (!n_bytes) {
    // Si el tamaño de la transacción es tal que requiere ser dividida en
    // múltiples transferencias (split transaction), para la primera de
    // ellas el número de bytes transferidos hasta ese momento es nulo.
    // Si se trata precisamente de esa primera transferencia liberamos
    // el thread que se encuentra en el semaforo.
    KeReleaseSemaphore(&engine->semaphore,  // PRKSEMAPHORE Semaphore,
                        0,                  // KPRIORITY    Increment,
                        1,                  // LONG         Adjustment,
                        FALSE);             // BOOLEAN      Wait
  }
  
  dbg_printf(("RMG: [%s] -\n", __FUNCTION__));
  return TRUE;

ErrExit:
  // FIXME: Current framework ignores the FALSE return value.
  // Therefore cleaning the request here itself.
  status = STATUS_UNSUCCESSFUL;
  WdfDmaTransactionRelease(engine->dmaTransaction);
  WdfRequestComplete(request, status);

  dbg_printf(("RMG: [%s] -\n", __FUNCTION__));
  return FALSE;
}


static void FUSE_EngineProcessTransfer(IN XDMA_ENGINE *engine) {
// service an SGDMA engine
  NTSTATUS status = STATUS_SUCCESS;
  WDFREQUEST request;
  UINT32 engineStatus;
  BOOLEAN completed;

  dbg_printf(("RMG: [%s] +\n", __FUNCTION__));
  
  if (engine == NULL) {
    TraceError(DBG_DMA, "engine=NULL");
    KdPrint(("   - engine = NULL\n"));
    dbg_printf(("RMG: [%s] -\n", __FUNCTION__));
    return;
  }

  TraceInfo(DBG_DMA, "%s_%u processing transfer completion",
        DirectionToString(engine->dir), engine->channel);

  WdfSpinLockAcquire(engine->engineLock);
  if (engine->isReqPending == FALSE) {
    // Spurious interrupt ??
    TraceError(DBG_DMA, "No request submitted for engine.");
//    KdPrint(("   - No request submitted for engine. Status = 0x%8.8X\n", engine->regs->status));
    KdPrint(("   !!!!!!! NO request submitted for engine. Status = 0x%8.8X\n", engine->regs->status));
    WdfSpinLockRelease(engine->engineLock);
    dbg_printf(("RMG: [%s] -\n", __FUNCTION__));
    return;
  }

  request = WdfDmaTransactionGetRequest(engine->dmaTransaction);
  if (!request) {
    TraceInfo(DBG_DMA, "Interrupt but no request pending?");
    WdfSpinLockRelease(engine->engineLock);
    dbg_printf(("   - Interrupt but no request pending?. Status = 0x%8.8X\n", engine->regs->status));
    dbg_printf(("RMG: [%s] -\n", __FUNCTION__));
    return;
  }

  // read and clear engine status
  dbg_printf(("   - Leo y limpio el estado para desactivar la interrupcion\n"));
  engineStatus = EngineStatus(engine, TRUE);
  dbg_printf(("   - engineStatus = 0x%8.8X\n", engineStatus));
  dbg_printf(("   - Tras limpiar, nuevo status = 0x%8.8X\n", engine->regs->status));
  
  dbg_printf(("   - IRQ regs: pending=0x%08X, enable=0x%08X => request=0x%08X \n",
    engine->parentDevice->interruptRegs->channelIntPending,
    engine->parentDevice->interruptRegs->channelIntEnable,
    engine->parentDevice->interruptRegs->channelIntRequest));
  
  dbg_printf(("   - Paro el engine\n"));
  EngineStop(engine);   // !!!
  dbg_printf(("   - Tras parar, new status = 0x%8.8X\n", engine->regs->status));
  dbg_printf(("   - IRQ regs: pending=0x%08X, enable=0x%08X => request=0x%08X \n",
    engine->parentDevice->interruptRegs->channelIntPending,
    engine->parentDevice->interruptRegs->channelIntEnable,
    engine->parentDevice->interruptRegs->channelIntRequest));

//  // clear descriptor buffer
//  DMA_DESCRIPTOR *desc = (DMA_DESCRIPTOR *)WdfCommonBufferGetAlignedVirtualAddress(engine->descBuffer);
//  size_t bufferLen = WdfCommonBufferGetLength(engine->descBuffer);
//  RtlZeroMemory(desc, bufferLen);

//  // clear poll writeback buffer
//  if (engine->poll) {
//    XDMA_POLL_WB* wbBuffer = (XDMA_POLL_WB*)WdfCommonBufferGetAlignedVirtualAddress(engine->pollWbBuffer);
//    size_t wbBufferLength = WdfCommonBufferGetLength(engine->pollWbBuffer);
//    RtlZeroMemory(wbBuffer, wbBufferLength);
//  }

  dbg_printf(("   - Analizando resultados....\n"));
  // RMG: obtengo el puntero al buffer de resultados (writeback info)
  int eop_cnt, n_desc;
  unsigned int n_bytes, packetLen;
  DMA_RESULT *wrBack;
  wrBack = (DMA_RESULT *) WdfCommonBufferGetAlignedVirtualAddress(engine->wrBackBuffer);
  
  n_desc = eop_cnt = 0;
  packetLen = n_bytes = 0;
  for (ULONG i = 0; i < engine->capacity; i++) {
    if (wrBack[i].status) {
      n_desc++;
      n_bytes += wrBack[i].length;
      if (wrBack[i].status & XDMA_RESULT_EOP_BIT) {
        packetLen = n_bytes;
        n_bytes = 0;
        eop_cnt++;
        dbg_printf(("      * packetLen = %d\n", packetLen));
      }
      wrBack[i].status = 0; // Marcamos el descritor como procesado
    }
    else {
      break;
    }
  }
// !!!!!!
//KdPrint(("   - ... fin de analisis: n_desc = %d, eop_cnt = %d, packetLen = %d\n", n_desc, eop_cnt, packetLen));

  //KdPrint(("[%s] Analisis Rx: n_desc = %d, eop_cnt = %d, packetLen = %d\n", __FUNCTION__, n_desc, eop_cnt, packetLen));


  // If any data is pending then call to WdfDmaTransactionDmaCompleted will immediately result in
  // another call to XDMA_EngineProgramDma. To avoid deadlock, mark the engine as free and release the lock.
  engine->isReqPending = FALSE;
  if (!engine->poll) {
    // reenable interrupt for this dma engine
    // RMG: !!!!! 
    // En el DPC desde el que se llama a este callback se habilitan las
    // interrupciones. Eso sí, se hace una vez ejecutados los DPCs de
    // TODOS los canales. Al hacerlo aquí se evita la espera por la 
    // ejecución de los DPC de los canales previos.
    EngineEnableInterrupt(engine);
  }
  WdfSpinLockRelease(engine->engineLock);

//  dbg_printf(("   - engineStatus = 0x%8.8X\n", engineStatus));
//  dbg_printf(("   - new status = 0x%8.8X\n", engine->regs->status));
  
//  if (1) {
//    UINT32 int_en, int_rqt, int_pend;
//
//    int_en = engine->parentDevice->interruptRegs->channelIntEnable;
//    int_rqt = engine->parentDevice->interruptRegs->channelIntRequest;
//    int_pend = engine->parentDevice->interruptRegs->channelIntPending;
//    
//    dbg_printf(("   - channel interrupt regs: enable=0x%08X request=0x%08X pending=0x%08X\n", 
//      int_en, int_rqt, int_pend));
//  }

  //switch (engineStatus & XDMA_STAT_EXPECTED_ZERO) {
  switch (engineStatus & (XDMA_STATUS_ERROR_MASK | XDMA_BUSY_BIT)) {
    case XDMA_ENGINE_STOPPED_OK: // engine not busy and no errors?
      completed = WdfDmaTransactionDmaCompleted(engine->dmaTransaction, &status);
      size_t bytesTransferred = WdfDmaTransactionGetBytesTransferred(engine->dmaTransaction);

      TraceInfo(DBG_DMA, "%s_%u transaction%scomplete, bytesTransferred=%llu",
            DirectionToString(engine->dir), engine->channel,
            completed ? " " : " in", bytesTransferred);
      dbg_printf(("   - Status analysis : transaction completed = %d, n_bytes = %d\n", completed, bytesTransferred));
      
      if (completed) {
        if (!engine->poll) {
          KeSetEvent(&engine->completionWaitSignal, IO_NO_INCREMENT, FALSE);
        }
        status = WdfDmaTransactionRelease(engine->dmaTransaction);
        if (!NT_SUCCESS(status)) {
          TraceError(DBG_DMA, "WdfDmaTransactionRelease failed: %!STATUS!", status);
        }
        
        if (bytesTransferred != packetLen) {
          KdPrint(("   - Packet length mismatch: bytesTransferred = %d, packetLen = %d\n", bytesTransferred, packetLen));
        }

  //KdPrint(("[%s] Packet length info: bytesTransferred = %d, packetLen = %d\n", __FUNCTION__, bytesTransferred, packetLen));
                
#if 0        
        WdfRequestCompleteWithInformation(request, status, bytesTransferred);
#else        
        WdfRequestCompleteWithInformation(request, status, packetLen);
#endif        
      }
      break;
    case XDMA_BUSY_BIT: // engine is still busy without sign of errors?
      TraceError(DBG_DMA, "Engine Still Busy, Descriptors Completed=%u", engine->regs->completedDescCount);
      KdPrint(("   - Engine Still Busy, Descriptors Completed=%u\n", engine->regs->completedDescCount));
      // RMG: break?????

    default: // any sign of errors
      TraceError(DBG_DMA, "Unexpected engine status 0x%08x", engineStatus);
      KdPrint(("   - Unexpected engine status 0x%08x\n", engineStatus));

      completed = WdfDmaTransactionDmaCompletedFinal(engine->dmaTransaction, 0, &status);
      if (completed) {
        if (!engine->poll) {
          TraceInfo(DBG_DMA, "EVENT SIGNALLED");
          KeSetEvent(&engine->completionWaitSignal, IO_NO_INCREMENT, FALSE);
        }

        status = WdfDmaTransactionRelease(engine->dmaTransaction);
        if (!NT_SUCCESS(status)) {
          TraceError(DBG_DMA, "WdfDmaTransactionRelease failed: %!STATUS!", status);
        }

        WdfRequestComplete(request, STATUS_INTERNAL_ERROR);
      }
  } // end-switch

  dbg_printf(("RMG: [%s] -\n", __FUNCTION__));
}

void FUSE_EngineRingSetup(IN XDMA_ENGINE *engine, IN UINT64 timeout_ms) {
#if 0
  UNREFERENCED_PARAMETER(engine);
  UNREFERENCED_PARAMETER(timeout_ms);
#else
  engine->timeout_ms = timeout_ms;
#endif

//  engine->ring.head = 0;
//  engine->ring.tail = 0;
//  KeClearEvent(&engine->ring.completionSignal);
//  EngineRingProgramDma(engine);
}

void FUSE_EngineRingTeardown(IN XDMA_ENGINE *engine) {
  UNREFERENCED_PARAMETER(engine);
//  EngineStop(engine);
//  EngineClearDmaResults(engine);
//  engine->ring.head = 0;
//  engine->ring.tail = 0;
}
