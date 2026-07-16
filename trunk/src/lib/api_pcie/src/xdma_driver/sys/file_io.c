/*
--
-- Vendor         : Xilinx
-- Revision       : $Revision: #18 $
-- Date           : $DateTime: 2019/05/31 03:43:11 $
-- Last Author    : $Author: arayajig $
--
-------------------------------------------------------------------------------
-- Description :
-- This file is part of the Xilinx DMA IP Core driver for Windows.
--
-- IO Request flow diagram:
-- ------------------------
-- User Operation (e.g. ReadFile())
-- |
-- |-> IO Request -> EvtIoRead()--> ReadBarToRequest()               // PCI BAR access
--               |            |---> EvtIoReadDma()                   // normal dma c2h transfer
--               |            |---> EvtIoReadEngineRing()            // for streaming interface
--               |            |---> CopyDescriptorsToRequestMemory() // get dma descriptors to user-space
--               |            |---> ServiceUserEvent()               // wait on user interrupt
--               |
--               |-> EvtIoWrite()-> WriteBarFromRequest()            // PCI BAR access
--                             |--> EvtIoWriteDma()                  // normal DMA H2C transfer
--                             |--> WriteBypassDescriptor()          // write descriptors from userspace to bypass BARs
--
-------------------------------------------------------------------------------
*/

// ========================= include dependencies =================================================

#include <ntddk.h>
#include <initguid.h> // required for GUID definitions
#include <wdmguid.h> // required for WMILIB_CONTEXT

#include "fuse_pcie_soc_hw.h"


//#define FUSE_CANCEL_RQT

#include "driver.h"
#include "dma_engine.h"
#include "xdma_public.h"
#include "file_io.h"

#include "trace.h"
#ifdef DBG
// The trace message header (.tmh) file must be included in a source file before any WPP macro
// calls and after defining a WPP_CONTROL_GUIDS macro (defined in trace.h). see trace.h
#include "file_io.tmh"
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


EVT_WDF_REQUEST_CANCEL      EvtCancelDma;

// RMG:
EVT_WDF_REQUEST_CANCEL      FUSE_EvtCancelReadUserEvent;

// ====================== device file nodes =======================================================

// RMG:
// Los nombres de los ficheros están incluidos en xdma_public.h

const static struct {
    DEVNODE_TYPE devType;
    const wchar_t *wstr;
    ULONG channel;        // <= esto se comporta como un índice del elemento
} FileNameLUT[] = {
    { DEVNODE_TYPE_H2C,         XDMA_FILE_H2C_0,        0 },
    { DEVNODE_TYPE_C2H,         XDMA_FILE_C2H_0,        0 },
    { DEVNODE_TYPE_H2C,         XDMA_FILE_H2C_1,        1 },
    { DEVNODE_TYPE_C2H,         XDMA_FILE_C2H_1,        1 },
    { DEVNODE_TYPE_H2C,         XDMA_FILE_H2C_2,        2 },
    { DEVNODE_TYPE_C2H,         XDMA_FILE_C2H_2,        2 },
    { DEVNODE_TYPE_H2C,         XDMA_FILE_H2C_3,        3 },
    { DEVNODE_TYPE_C2H,         XDMA_FILE_C2H_3,        3 },
    { DEVNODE_TYPE_USER,        XDMA_FILE_USER,         0 },
    { DEVNODE_TYPE_CONTROL,     XDMA_FILE_CONTROL,      0 },
    { DEVNODE_TYPE_BYPASS,      XDMA_FILE_BYPASS,       0 },
    { DEVNODE_TYPE_EVENTS,      XDMA_FILE_EVENT_0,      0 },
    { DEVNODE_TYPE_EVENTS,      XDMA_FILE_EVENT_1,      1 },
    { DEVNODE_TYPE_EVENTS,      XDMA_FILE_EVENT_2,      2 },
    { DEVNODE_TYPE_EVENTS,      XDMA_FILE_EVENT_3,      3 },
    { DEVNODE_TYPE_EVENTS,      XDMA_FILE_EVENT_4,      4 },
    { DEVNODE_TYPE_EVENTS,      XDMA_FILE_EVENT_5,      5 },
    { DEVNODE_TYPE_EVENTS,      XDMA_FILE_EVENT_6,      6 },
    { DEVNODE_TYPE_EVENTS,      XDMA_FILE_EVENT_7,      7 },
    { DEVNODE_TYPE_EVENTS,      XDMA_FILE_EVENT_8,      8 },
    { DEVNODE_TYPE_EVENTS,      XDMA_FILE_EVENT_9,      9 },
    { DEVNODE_TYPE_EVENTS,      XDMA_FILE_EVENT_10,     10 },
    { DEVNODE_TYPE_EVENTS,      XDMA_FILE_EVENT_11,     11 },
    { DEVNODE_TYPE_EVENTS,      XDMA_FILE_EVENT_12,     12 },
    { DEVNODE_TYPE_EVENTS,      XDMA_FILE_EVENT_13,     13 },
    { DEVNODE_TYPE_EVENTS,      XDMA_FILE_EVENT_14,     14 },
    { DEVNODE_TYPE_EVENTS,      XDMA_FILE_EVENT_15,     15 },
};

// RMG: dada como entrada el nombre del fichero devuelve el tipo y el índice
// convert from filename to device node
static VOID GetDevNodeType(PUNICODE_STRING fileName, PFILE_CONTEXT file, ULONG* index) {
  // RMG: recorremos los elementos de la LUT
  for (UINT i = 0; i < sizeof(FileNameLUT) / sizeof(FileNameLUT[0]); ++i) {
    if (!wcscmp(fileName->Buffer, FileNameLUT[i].wstr)) {
      file->devType = FileNameLUT[i].devType;
      *index = FileNameLUT[i].channel;
      return;
    }
  }
  TraceError(DBG_IO, "GetDevNodeID() returns ID_DEVNODE_UNKNOWN");
  file->devType = ID_DEVNODE_UNKNOWN;
}

// RMG: The framework calls a driver's EvtDeviceFileCreate callback function
// when a user application or another driver opens the device to perform an
// I/O operation, such as reading or writing a file.
VOID EvtDeviceFileCreate(IN WDFDEVICE device, IN WDFREQUEST Request, IN WDFFILEOBJECT WdfFile) {
  PUNICODE_STRING fileName = WdfFileObjectGetFileName(WdfFile);
  DeviceContext *ctx = GetDeviceContext(device);
  PXDMA_DEVICE xdma = &(ctx->xdma);
  PFILE_CONTEXT devNode = GetFileContext(WdfFile);
  NTSTATUS status = STATUS_SUCCESS;

  //KdPrint(("RMG: [%s] abriendo fichero\n", __FUNCTION__));

  // no filename given?
  if (fileName == NULL) {
    TraceError(DBG_IO, "Error: no filename given.");
    status = STATUS_INVALID_PARAMETER;
    goto ErrExit;
  }

  // device node has zero length?
  ASSERTMSG("fileName is empty string", fileName->Length != 0);
  ULONG index = 0;

  // RMG: A partir del nombre del fichero determino el tipo y el índice
  GetDevNodeType(fileName, devNode, &index);
  if (devNode->devType == ID_DEVNODE_UNKNOWN) {
    TraceError(DBG_IO, "Error: invalid device node given: %wZ", fileName);
    status = STATUS_INVALID_PARAMETER;
    goto ErrExit;
  }

  //KdPrint(("RMG: [%s] abriendo fichero type = %d\n", __FUNCTION__, devNode->devType));

  // additional checks/setup for based on device type
  switch (devNode->devType) {
    case DEVNODE_TYPE_CONTROL:
      devNode->u.bar = xdma->bar[xdma->configBarIdx];
      break;
    case DEVNODE_TYPE_USER:
      if (xdma->userBarIdx < 0) {
        TraceError(DBG_IO, "Failed to create 'user' device file. User BAR does not exist!");
        status = STATUS_INVALID_PARAMETER;
        goto ErrExit;
      }
      devNode->u.bar = xdma->bar[xdma->userBarIdx];     // <= BAR usado control
      break;
    case DEVNODE_TYPE_BYPASS:
      if (xdma->bypassBarIdx < 0) {
        TraceError(DBG_IO, "Failed to create 'bypass' device file. User BAR does not exist!");
        status = STATUS_INVALID_PARAMETER;
        goto ErrExit;
      }
      devNode->u.bar = xdma->bar[xdma->bypassBarIdx];   // <= BAR usado usario
      break;
    case DEVNODE_TYPE_H2C:
    case DEVNODE_TYPE_C2H:
    {
      DirToDev dir = devNode->devType == DEVNODE_TYPE_H2C ? H2C : C2H;
      XDMA_ENGINE* engine = &(xdma->engines[index][dir]);

      if (engine->enabled == FALSE) {
        TraceError(DBG_IO, "Error: engine %s_%d not enabled in XDMA IP core",
               dir == H2C ? "h2c" : "c2h", index);
        status = STATUS_INVALID_PARAMETER;
        goto ErrExit;
      }

      if ((engine->type == EngineType_ST) && (dir == C2H)) {
#ifdef USE_FUSE_DMA
        FUSE_EngineRingSetup(engine, DEFAULT_DMA_FILE_TIMEOUT_MS);
        devNode->timeout_ms = DEFAULT_DMA_FILE_TIMEOUT_MS;
#else
        EngineRingSetup(engine);    // RMG: Esto es importante, es nuestro caso!!!
#endif
      }

      devNode->u.engine = engine;
      devNode->queue = ctx->engineQueue[dir][index];
      TraceVerbose(DBG_IO, "pollMode=%u", devNode->u.engine->poll);
      if (devNode->u.engine->poll) {
        EngineDisableInterrupt(devNode->u.engine);
      } else {
        EngineEnableInterrupt(devNode->u.engine);
      }
      break;
    }
    case DEVNODE_TYPE_EVENTS:
      // RMG: User interrupt processing
#ifdef USE_FUSE_INTS
      //KdPrint(("RMG: [%s] abriendo fichero de eventos %d\n", __FUNCTION__, index));
      devNode->u.fuseEvent = &(xdma->fuseUserEvents[index]);
      devNode->timeout_ms = DEFAULT_EVENT_FILE_TIMEOUT_MS;
#else
      devNode->u.event = &(xdma->userEvents[index]);
#endif
      break;
#ifdef USE_FUSE_INTS
#else
#endif

    default:
      break;
  }
  TraceInfo(DBG_IO, "Created %wZ device file", fileName);

ErrExit:
  WdfRequestComplete(Request, status);
  TraceVerbose(DBG_IO, "returns %!STATUS!", status);
}

// RMG: The framework calls a driver's EvtFileClose callback function
// when the last handle for a file object has been closed and released,
// and all outstanding I/O requests have been completed or canceled.
VOID EvtFileClose(IN WDFFILEOBJECT FileObject) {
  PUNICODE_STRING fileName = WdfFileObjectGetFileName(FileObject);
  TraceInfo(DBG_IO, "Closing file %wZ", fileName);
}

// RMG: The framework calls a driver's EvtFileCleanup callback function
// when the last handle to the specified file object has been closed.
VOID EvtFileCleanup(IN WDFFILEOBJECT FileObject) {
  PUNICODE_STRING fileName = WdfFileObjectGetFileName(FileObject);
  PFILE_CONTEXT file = GetFileContext(FileObject);

  if (file->devType == DEVNODE_TYPE_C2H) {
    if (file->u.engine->type == EngineType_ST) {
#ifdef USE_FUSE_DMA
      FUSE_EngineRingTeardown(file->u.engine);
#else
      EngineRingTeardown(file->u.engine);   // <= ojo !!! demolición
#endif
    }
  }
  if (file->devType == DEVNODE_TYPE_CONTROL ||
      file->devType == DEVNODE_TYPE_USER    ||
      file->devType == DEVNODE_TYPE_BYPASS)
  {
    // Para deshacer memory descriptor list (MDL) (memory mapped file)
    if (file->mdl != NULL) {
      MmUnmapLockedPages(file->virtAddress, file->mdl);
      IoFreeMdl(file->mdl);
      file->mdl = NULL;
    }
  }

  TraceVerbose(DBG_IO, "Cleanup %wZ", fileName);
}

//-------------------------------------

// RMG: comprobación de que los accesos a un determinado BAR de xdma con
// índice nBar de con un offset y un length determinado se puede realizar
// correctamente
//    => Parece que finalmente no se utiliza

static NTSTATUS ValidateBarParams(IN PXDMA_DEVICE xdma, ULONG nBar, size_t offset, size_t length) {
  if (length == 0) {
    TraceError(DBG_IO, "Error: attempting to read 0 bytes");
    return STATUS_INVALID_DEVICE_REQUEST;
  }

  // invalid BAR index?
  if (nBar >= xdma->numBars) {
    TraceError(DBG_IO, "Error: attempting to read BAR %u but only 2 exist", nBar);
    return STATUS_INVALID_DEVICE_REQUEST;
  }

  // access outside valid BAR address range?
  if (offset + length >= xdma->barLength[nBar]) {
    TraceError(DBG_IO, "Error: attempting to read BAR %u offset=%llu size=%llu",
           nBar, offset, length);
    return STATUS_INVALID_DEVICE_REQUEST;
  }
  return STATUS_SUCCESS;
}

// RMG: Lee datos de PCIe y lo deja en la IO request
// Read from PCIe mmap'ed memory into an IO request
static NTSTATUS ReadBarToRequest(WDFREQUEST request, PVOID bar) {
  WDF_REQUEST_PARAMETERS params;
  WDF_REQUEST_PARAMETERS_INIT(&params);
  WdfRequestGetParameters(request, &params);
  size_t offset = (size_t)params.Parameters.Read.DeviceOffset;
  size_t length = params.Parameters.Read.Length;

  // Static Driver Verifier is not smart enough to see that length is checked in ValidateBarParams
  // Therefore we need to check it here as well
  if (length == 0) {
    TraceError(DBG_IO, "Error: attempting to read 0 bytes");
    return STATUS_INVALID_DEVICE_REQUEST;
  }

  // calculate virtual address of the mmap'd BAR location
  PUCHAR readAddr = (PUCHAR)bar + offset;

  // get handle to the IO request memory which will hold the read data
  WDFMEMORY requestMemory;
  NTSTATUS status = WdfRequestRetrieveOutputMemory(request, &requestMemory);
  if (!NT_SUCCESS(status)) {
    TraceError(DBG_IO, "WdfRequestRetrieveOutputMemory failed: %!STATUS!", status);
    return status;
  }

  // get pointer to buffer
  PVOID reqBuffer = WdfMemoryGetBuffer(requestMemory, NULL);

  // write to BAR
  if (length % sizeof(ULONG) == 0) {
    READ_REGISTER_BUFFER_ULONG((volatile ULONG*)readAddr, (PULONG)reqBuffer, (ULONG)length / sizeof(ULONG));
  } else if (length % sizeof(USHORT) == 0) {
    READ_REGISTER_BUFFER_USHORT((volatile USHORT*)readAddr, (PUSHORT)reqBuffer, (ULONG)length / sizeof(USHORT));
  } else {
    READ_REGISTER_BUFFER_UCHAR((volatile UCHAR*)readAddr, (PUCHAR)reqBuffer, (ULONG)length);
  }

  return status;
}

// RMG NOTE:
// The READ_REGISTER_BUFFER_xxx routine dereferences the supplied pointer,
// inserts a memory barrier, and reads a set of xxx values from the
// specified register address into a buffer.


// RMG: Escribe en PCIe los datos suministrados con 'request'
// Write from an IO request into PCIe mmap'ed memory
static NTSTATUS WriteBarFromRequest(WDFREQUEST request, PVOID bar) {
  WDF_REQUEST_PARAMETERS params;
  WDF_REQUEST_PARAMETERS_INIT(&params);
  WdfRequestGetParameters(request, &params);
  size_t offset = (size_t)params.Parameters.Read.DeviceOffset;
  size_t length = params.Parameters.Read.Length;

  // Static Driver Verifier is not smart enough to see that length is checked in ValidateBarParams
  // Therefore we need to check it here as well
  if (length == 0) {
    TraceError(DBG_IO, "Error: attempting to read 0 bytes");
    return STATUS_INVALID_DEVICE_REQUEST;
  }

  // calculate virtual address of the mmap'd BAR location
  PUCHAR writeAddr = (PUCHAR)bar + offset;

  WDFMEMORY requestMemory;
  // get handle to the IO request memory which holds data to write
  NTSTATUS status = WdfRequestRetrieveInputMemory(request, &requestMemory);
  if (!NT_SUCCESS(status)) {
    TraceError(DBG_IO, "WdfRequestRetrieveInputMemory failed: %!STATUS!", status);
    return status;
  }

  // get pointer to buffer
  PVOID reqBuffer = WdfMemoryGetBuffer(requestMemory, NULL);

  // write to BAR
  if (length % sizeof(ULONG) == 0) {
    WRITE_REGISTER_BUFFER_ULONG((volatile ULONG*)writeAddr, (PULONG)reqBuffer, (ULONG)length / sizeof(ULONG));
  } else if (length % sizeof(USHORT) == 0) {
    WRITE_REGISTER_BUFFER_USHORT((volatile USHORT*)writeAddr, (PUSHORT)reqBuffer, (ULONG)length / sizeof(USHORT));
  } else {
    WRITE_REGISTER_BUFFER_UCHAR((volatile UCHAR*)writeAddr, (PUCHAR)reqBuffer, (ULONG)length);
  }

  TraceVerbose(DBG_IO, "Finished Write Request: Request=%p", request);

  return status;
}

// RMG:
// If a driver has registered an EvtIoRead callback function for a device's
// I/O queue, the callback function receives every read request from the
// queue. For more information, see Request Handlers.

// Callback function on Device node ReadFile
VOID EvtIoRead(IN WDFQUEUE queue, IN WDFREQUEST request, IN size_t length) {
  NTSTATUS status = STATUS_SUCCESS;
  PFILE_CONTEXT file = GetFileContext(WdfRequestGetFileObject(request));

  TraceVerbose(DBG_IO, "(Queue=%p, Request=%p, Length=%llu)", queue, request, length);

  UNREFERENCED_PARAMETER(length); // length can be extracted from request

  TraceVerbose(DBG_IO, "(Queue=%p, Request=%p, Length=%llu)", queue, request, length);
  TraceVerbose(DBG_IO, "devNodeType %d", file->devType);

  switch (file->devType) {
    case DEVNODE_TYPE_USER:
    case DEVNODE_TYPE_CONTROL:
    case DEVNODE_TYPE_BYPASS:
      ASSERTMSG("no BAR ptr attached to file context", file->u.bar != NULL);
      // handle request here without forwarding - read from PCIe BAR into request memory
      status = ReadBarToRequest(request, file->u.bar); // ok
      if (NT_SUCCESS(status)) {
        // complete the request - read bytes are in the requestMemory
        WdfRequestCompleteWithInformation(request, status, length);
      }
      break;
    case DEVNODE_TYPE_EVENTS:
#ifdef USE_FUSE_DMA
      //KdPrint(("RMG: [%s] leyendo fichero de eventos\n", __FUNCTION__));
      ASSERTMSG("no event attached to file context", file->u.fuseEvent != NULL);
      // Pasan la llamada a la función que procesa la lectura de los
      // ficheros de eventos
      status = FUSE_EvtReadUserEvent(request, length); // ok
#else
      ASSERTMSG("no event attached to file context", file->u.event != NULL);
      // Pasan la llamada a la función que procesa la lectura de los
      // ficheros de eventos
      status = EvtReadUserEvent(request, length); // ok
#endif
      break;
    case DEVNODE_TYPE_C2H:  // !!!
    {
      ASSERTMSG("no engine attached to file context", file->u.engine != NULL);

      // RMG: En el caso de las lecturas DMA, no se procesan directamente sino
      // que a través de una cola, de forma que acabará siendo procesada
      // finalmente por EvtIoReadDma()
      //
      // forward request to engine queue - completed by EvtIoReadDma later
      status = WdfRequestForwardToIoQueue(request, file->queue);
      break;
    }
    case DEVNODE_TYPE_H2C:
    default:
      TraceError(DBG_IO, "fails with invalid DevNodeID %d", file->devType);
      status = STATUS_INVALID_DEVICE_REQUEST;
  }

  if (!NT_SUCCESS(status)) {
    TraceError(DBG_IO, "request complete with %!STATUS!", status);
    WdfRequestComplete(request, status);
  }

  return; // request has been either completed directly or forwarded to a queue
}

// RMG:
// If a driver has registered an EvtIoWrite callback function for a device's
// I/O queue, the callback function receives every write request from the
// queue. For more information, see Request Handlers.

// Callback function on Device node write operations
VOID EvtIoWrite(IN WDFQUEUE queue, IN WDFREQUEST request, IN size_t length) {
  NTSTATUS status = STATUS_SUCCESS;
  PFILE_CONTEXT file = GetFileContext(WdfRequestGetFileObject(request));

  UNREFERENCED_PARAMETER(length); // length can be extracted from request

  TraceVerbose(DBG_IO, "(Queue=%p, Request=%p)", queue, request);
  TraceVerbose(DBG_IO, "DevNodeID %d", file->devType);

  switch (file->devType) {
    case DEVNODE_TYPE_USER:
    case DEVNODE_TYPE_CONTROL:
    case DEVNODE_TYPE_BYPASS:
      ASSERTMSG("no BAR ptr attached to file context", file->u.bar != NULL);
      // handle request here without forwarding. write to PCIe BAR from request memory
      status = WriteBarFromRequest(request, file->u.bar);
      if (NT_SUCCESS(status)) {
        WdfRequestCompleteWithInformation(request, status, length);  // complete the request
      }
      break;
    case DEVNODE_TYPE_H2C:
      ASSERTMSG("no engine attached to file context", file->u.engine != NULL);

      // forward request to write engine queue, it ends up in EvtIoWriteDma() later
      status = WdfRequestForwardToIoQueue(request, file->queue);
      break;
    case DEVNODE_TYPE_C2H:
    default:
      status = STATUS_INVALID_DEVICE_REQUEST;
      break;
  }

  if (!NT_SUCCESS(status)) {
    WdfRequestComplete(request, status);
    TraceInfo(DBG_IO, "Error Request 0x%p: %!STATUS!", request, status);
  }

  return; // request has been either completed directly or forwarded to a queue
}

//-------------------------------------

// RMG: Procesamiento de IOCTL_XDMA_PERF_GET
static NTSTATUS IoctlGetPerf(IN WDFREQUEST request, IN XDMA_ENGINE* engine) {
  ASSERT(engine != NULL);
  XDMA_PERF_DATA perfData = { 0 };
  EngineGetPerf(engine, &perfData);

  // get handle to the IO request memory which will hold the read data
  WDFMEMORY requestMemory;
  NTSTATUS status = WdfRequestRetrieveOutputMemory(request, &requestMemory);
  if (!NT_SUCCESS(status)) {
    TraceError(DBG_IO, "WdfRequestRetrieveOutputMemory failed: %!STATUS!", status);
    return status;
  }

  // copy from perfData into request memory
  status = WdfMemoryCopyFromBuffer(requestMemory, 0, &perfData, sizeof(perfData));
  if (!NT_SUCCESS(status)) {
    TraceError(DBG_IO, "WdfMemoryCopyFromBuffer failed: %!STATUS!", status);
    return status;
  }

  return status;
}

// RMG: Procesamiento de IOCTL_XDMA_ADDRMODE_GET
static NTSTATUS IoctlGetAddrMode(IN WDFREQUEST request, IN XDMA_ENGINE* engine) {
  ASSERT(engine != NULL);
  ULONG addrMode = (engine->regs->control & XDMA_CTRL_NON_INCR_ADDR) != 0; // 0 = inc, 1=non-inc
  TraceVerbose(DBG_IO, "addrMode=%u", addrMode);

  // get handle to the IO request memory which will hold the read data
  WDFMEMORY requestMemory;
  NTSTATUS status = WdfRequestRetrieveOutputMemory(request, &requestMemory);
  if (!NT_SUCCESS(status)) {
    TraceError(DBG_IO, "WdfRequestRetrieveOutputMemory failed: %!STATUS!", status);
    return status;
  }

  // copy from perfData into request memory
  status = WdfMemoryCopyFromBuffer(requestMemory, 0, &addrMode, sizeof(addrMode));
  if (!NT_SUCCESS(status)) {
    TraceError(DBG_IO, "WdfMemoryCopyFromBuffer failed: %!STATUS!", status);
    return status;
  }

  return status;
}

// RMG: Procesamiento de IOCTL_XDMA_ADDRMODE_SET
static NTSTATUS IoctlSetAddrMode(IN WDFREQUEST request, IN XDMA_ENGINE* engine) {
  ASSERT(engine != NULL);

  // get handle to the IO request memory which will hold the read data
  WDFMEMORY requestMemory;
  NTSTATUS status = WdfRequestRetrieveInputMemory(request, &requestMemory);
  if (!NT_SUCCESS(status)) {
    TraceError(DBG_IO, "WdfRequestRetrieveOutputMemory failed: %!STATUS!", status);
    return status;
  }
  ULONG addrMode = 0;

  // copy from perfData into request memory
  status = WdfMemoryCopyToBuffer(requestMemory, 0, &addrMode, sizeof(addrMode));
  if (!NT_SUCCESS(status)) {
    TraceError(DBG_IO, "WdfMemoryCopyFromBuffer failed: %!STATUS!", status);
    return status;
  }

  if (addrMode) {
    engine->regs->controlW1S = XDMA_CTRL_NON_INCR_ADDR;
  } else {
    engine->regs->controlW1C = XDMA_CTRL_NON_INCR_ADDR;
  }
  engine->addressMode = addrMode;

  TraceVerbose(DBG_IO, "addrMode=%u", addrMode);

  return status;
}

// RMG:
// Gestion de ficheros mapeados en memoria para acceso a periféricos
// Hay que analizarlo con más detenimiento!!

static NTSTATUS IoctlMapBar(IN PFILE_CONTEXT file, IN WDFREQUEST request, IN ULONG barIndex, IN PXDMA_DEVICE xdma) {
  NTSTATUS status = STATUS_INTERNAL_ERROR;

  size_t totalLength;
  PVOID virtAddr;
  PUCHAR buffer;
  size_t bytesReturned = 0;

  PXDMA_BAR_INFO xbar_info;

  status = WdfRequestRetrieveOutputBuffer(request, sizeof(UCHAR), &buffer, &totalLength);
  if (!NT_SUCCESS(status)) {
    TraceError(DBG_IO, "Retrieve Buffer Failed - Length: %Iu", totalLength);
    return status;
  }

  if (buffer == NULL) {
    TraceError(DBG_IO, "Buffer is NULL");
    status = STATUS_NOT_SUPPORTED;
    return status;
  }

  if (totalLength != sizeof(XDMA_BAR_INFO)) {
    TraceError(DBG_IO, "Input data not equal to XDMA Keyhole Struct");
    status = STATUS_NOT_SUPPORTED;
    return status;
  }

  bytesReturned = totalLength;

  // Cast to user space map bar data struct
  xbar_info = (PXDMA_BAR_INFO)buffer;

  // Create memory descriptor list (MDL) for virtual memory of xdma BAR
  file->mdl = IoAllocateMdl(xdma->bar[barIndex], xdma->barLength[barIndex], FALSE, FALSE, NULL);
  if (!file->mdl) {
    TraceError(DBG_IO, "Bad MDL allocation");
    return STATUS_INSUFFICIENT_RESOURCES;
  }

  //Prepare MDL for non-paged virtual memory for MmMapLockedPagesSpecifyCache function
  MmBuildMdlForNonPagedPool(file->mdl);

  try {
    //Function to allow pointer to virtual memory to be accessed by userspace
    virtAddr = MmMapLockedPagesSpecifyCache(file->mdl, UserMode, MmNonCached, NULL, FALSE, NormalPagePriority | MdlMappingNoExecute);
    if (!virtAddr) {
      TraceError(DBG_IO, "Bad Mapping");
      IoFreeMdl(file->mdl);
      file->mdl = NULL;
      return STATUS_INSUFFICIENT_RESOURCES;
    }
  } except(EXCEPTION_EXECUTE_HANDLER) {
    IoFreeMdl(file->mdl);
    file->mdl = NULL;
    TraceError(DBG_IO, "Bad Mapping");
    return STATUS_INSUFFICIENT_RESOURCES;
  }

  //Pass back address and BAR length
  file->virtAddress = virtAddr;
  xbar_info->mappedAddress = virtAddr;
  xbar_info->barLength = xdma->barLength[barIndex];

  TraceInfo(DBG_IO, "Buffer Passed, IOCTL Map Bar Success. Virtual Address: %p", virtAddr);

  if (NT_SUCCESS(status)) {
    WdfRequestCompleteWithInformation(request, status, bytesReturned);  // complete the request
  }

  return status;
}

// RMG: revisado por encima!
static NTSTATUS IoctlKeyholeWriteRegister(IN PFILE_CONTEXT file, IN WDFREQUEST request, IN ULONG barIndex, IN PXDMA_DEVICE xdma) {
  NTSTATUS status = STATUS_INTERNAL_ERROR;

  PXDMA_KEYHOLE_DATA kholeData;
  PVOID buffer;
  size_t totalLength;

  status = WdfRequestRetrieveInputBuffer(request, sizeof(PXDMA_KEYHOLE_DATA), &buffer, &totalLength);
  if (!NT_SUCCESS(status)) {
    TraceError(DBG_IO, "Retrieve Buffer Failed - Length: %Iu", totalLength);
    return status;
  }

  if (buffer == NULL) {
    TraceError(DBG_IO, "Buffer is NULL");
    status = STATUS_NOT_SUPPORTED;
    return status;
  }

  if (totalLength != sizeof(XDMA_KEYHOLE_DATA)) {
    TraceError(DBG_IO, "Input data not equal to XDMA Keyhole Struct");
    status = STATUS_NOT_SUPPORTED;
    return status;
  }

  // Cast to data struct for user application data retrieval
  kholeData = (PXDMA_KEYHOLE_DATA)buffer;

  if (kholeData->size <= 0 || kholeData->offset >=  xdma->barLength[barIndex]) {
    TraceError(DBG_IO, "Struct data incorrect, size is zero, pointer is null, or offset is greater than bar length");
    status = STATUS_NOT_SUPPORTED;
    return status;
  }

  TraceInfo(DBG_IO, "Retrieve Buffer Success - Length: %Iu", kholeData->size);
  TraceInfo(DBG_IO, "Retrieve Buffer Success - Offset: %Iu", kholeData->offset);
  TraceInfo(DBG_IO, "Retrieve Buffer Success - Buffer Address: %p", kholeData->ptrAddr);

  PUCHAR writeAddr = (PUCHAR)file->u.bar + kholeData->offset;

  //Repeatedly write 32-bit values from arrray to address without increment
  for(size_t totalSize = kholeData->size; totalSize > 0; totalSize--) {
    WRITE_REGISTER_ULONG((volatile ULONG*)writeAddr, *kholeData->ptrAddr);
    kholeData->ptrAddr++;
  }

  if (NT_SUCCESS(status)) {
    WdfRequestComplete(request, status);  // complete the request
  }

  return status;
}

// RMG: Tratamiento de operaciones ioctl asociadas a BARs
static NTSTATUS IoctlBar(
          IN PFILE_CONTEXT file,
          IN WDFREQUEST request,
          IN ULONG IoControlCode,
          IN ULONG barIndex,
          IN PXDMA_DEVICE xdma)
{
  NTSTATUS status = STATUS_INTERNAL_ERROR;

  switch (IoControlCode) {
    //Repeatedly write from an array size amount of times to a given register
    case IOCTL_WRITE_KEYHOLE_REGISTER:
      TraceInfo(DBG_IO, "IOCTL_WRITE_KEYHOLE_REGISTER");
      status = IoctlKeyholeWriteRegister(file, request, barIndex, xdma);
      break;
    default:
      TraceError(DBG_IO, "Unknown IOCTL code for device node!");
      status = STATUS_NOT_SUPPORTED;
      break;
  }

  return status;
}

// RMG: Tratamiento de las diferentes operaciones ioctl para un canal
static NTSTATUS IoctlChannel(IN PFILE_CONTEXT file, IN WDFREQUEST request, IN ULONG IoControlCode) {
  UNREFERENCED_PARAMETER(file);

  NTSTATUS status = STATUS_INTERNAL_ERROR;

  PQUEUE_CONTEXT queue = GetQueueContext(file->queue);

  // ioctl codes defined in xdma_public.h
  switch (IoControlCode) {
    case IOCTL_XDMA_PERF_START:
      TraceInfo(DBG_IO, "%s_%u IOCTL_XDMA_PERF_START",
        queue->engine->dir == H2C ? "H2C" : "C2H", queue->engine->channel);
      EngineStartPerf(queue->engine);
      status = STATUS_SUCCESS;
      WdfRequestComplete(request, status);
      break;
    case IOCTL_XDMA_PERF_GET:
      TraceInfo(DBG_IO, "%s_%u IOCTL_XDMA_PERF_GET",
        queue->engine->dir == H2C ? "H2C" : "C2H", queue->engine->channel);
      status = IoctlGetPerf(request, queue->engine);
      if (NT_SUCCESS(status)) {
        WdfRequestCompleteWithInformation(request, status, sizeof(XDMA_PERF_DATA));
      }
      break;
    case IOCTL_XDMA_ADDRMODE_GET:
      TraceInfo(DBG_IO, "%s_%u IOCTL_XDMA_ADDRMODE_GET",
        queue->engine->dir == H2C ? "H2C" : "C2H", queue->engine->channel);
      status = IoctlGetAddrMode(request, queue->engine);
      if (NT_SUCCESS(status)) {
        WdfRequestCompleteWithInformation(request, status, sizeof(ULONG));
      }
      break;
    case IOCTL_XDMA_ADDRMODE_SET:
      TraceInfo(DBG_IO, "%s_%u IOCTL_XDMA_ADDRMODE_SET",
        queue->engine->dir == H2C ? "H2C" : "C2H", queue->engine->channel);
      status = IoctlSetAddrMode(request, queue->engine);
      if (NT_SUCCESS(status)) {
        WdfRequestComplete(request, STATUS_SUCCESS);
      }
      break;
    default:
      TraceError(DBG_IO, "Unknown IOCTL code!");
      status = STATUS_NOT_SUPPORTED;
      break;
  }

  return status;
}


// RMG: Creo que todo esto es para gestionar el acceso a los periféricos
// mediente ficheros mapeados en memoria

// RMG: A driver's EvtIoInCallerContext event callback function preprocesses
// an I/O request before the framework places it into an I/O queue.

// RMG: The framework calls a driver's EvtIoInCallerContext callback function
// so that the driver can examine each I/O request, and possibly perform
// preliminary processing on the request, before the framework places it in
// an I/O queue.


VOID EvtDeviceIoInCallerContext(IN WDFDEVICE  device, IN WDFREQUEST request) {
  NTSTATUS status = STATUS_SUCCESS;
  WDF_REQUEST_PARAMETERS params;
  PFILE_CONTEXT file = GetFileContext(WdfRequestGetFileObject(request));

  WDF_REQUEST_PARAMETERS_INIT(&params);
  WdfRequestGetParameters(request, &params);

  if (params.Type == WdfRequestTypeDeviceControl && params.Parameters.DeviceIoControl.IoControlCode == IOCTL_MAP_BAR) {
    DeviceContext* ctx = GetDeviceContext(device);
    PXDMA_DEVICE xdma = &(ctx->xdma);
    /* User, Control, Bypass set as either 0, 1, 2 or
     * -1 for any BAR which does not exist excluding Control which is always there (driver error if not)
     */
    int barIndex = xdma->userBarIdx;

    switch (file->devType) {
      case DEVNODE_TYPE_BYPASS:
        barIndex++;
      case DEVNODE_TYPE_CONTROL:
        barIndex++;
      case DEVNODE_TYPE_USER:
        /* Map BAR to userspace */
        status = IoctlMapBar(file, request, barIndex, xdma);
        if (!NT_SUCCESS(status)) {
          WdfRequestComplete(request, status);
        }
        return;
      default:
        WdfRequestComplete(request, STATUS_UNSUCCESSFUL);
    }
  }

//KdPrint(("[%s] Encolando solicitud, request = 0x%p\n", __FUNCTION__, request));

  status = WdfDeviceEnqueueRequest(device, request);
  if (!NT_SUCCESS(status)) {
    TraceError(DBG_IO, "Failed to forward request %!STATUS!", status);
  }
}

// RMG: If a driver has registered an EvtIoDeviceControl callback function
// for a device's I/O queue, the callback function receives every I/O control
// request (IRP_MJ_DEVICE_CONTROL) from the queue. For more information,
// see Request Handlers.

// The IRP_MJ_DEVICE_CONTROL request is sent by the I/O Manager and other
// operating system components, as well as other kernel-mode drivers.
// Normally this IRP is sent on behalf of a user-mode application that
// has called the Microsoft Win32 DeviceIoControl function or on behalf
// of a kernel-mode component that has called ZwDeviceIoControlFile.

static int FUSE_io_dev_control(IN PFILE_CONTEXT file, IN WDFREQUEST rqt,
                               IN XDMA_DEVICE *xdma,  IN ULONG code);

// todo separate ioctl functions for sgdma and other?
VOID EvtIoDeviceControl(
  IN WDFQUEUE Queue,
  IN WDFREQUEST request,
  IN size_t OutputBufferLength,
  IN size_t InputBufferLength,
  IN ULONG IoControlCode)
{
  UNREFERENCED_PARAMETER(OutputBufferLength);
  UNREFERENCED_PARAMETER(InputBufferLength);
  UNREFERENCED_PARAMETER(Queue);

  NTSTATUS status = STATUS_INTERNAL_ERROR;

  PFILE_CONTEXT file = GetFileContext(WdfRequestGetFileObject(request));

//KdPrint(("[%s] Reciendo solicitud (IOCTL), request = 0x%p\n", __FUNCTION__, request));

  WDFDEVICE device = WdfFileObjectGetDevice(WdfRequestGetFileObject(request));
  DeviceContext *ctx = GetDeviceContext(device);
  PXDMA_DEVICE xdma = &(ctx->xdma);

  // RMG: Proceso primero los códigos FUSE y en caso contrario los de Xilinx:
  int served = FUSE_io_dev_control(file, request, xdma, IoControlCode);
  if (served) {
    TraceVerbose(DBG_IO, "USE IOCTL processing");
  }
  else {
    // User, Control, Bypass set as either 0, 1, 2 or -1 for any BAR which does
    // not exist excluding Control which is always there (driver error if not)
    int barIndex = xdma->userBarIdx;

    switch (file->devType) {
      case DEVNODE_TYPE_BYPASS:
        barIndex++;
      case DEVNODE_TYPE_CONTROL:
        barIndex++;
      case DEVNODE_TYPE_USER:
        status = IoctlBar(file, request, IoControlCode, barIndex, xdma);
        break;
      case DEVNODE_TYPE_H2C:
      case DEVNODE_TYPE_C2H:
        status = IoctlChannel(file, request, IoControlCode);
        break;
      default:
        TraceError(DBG_IO, "Unknown IOCTL code for device node!");
        status = STATUS_NOT_SUPPORTED;
        break;
    }
    if (!NT_SUCCESS(status)) {
      WdfRequestComplete(request, status);
    }
    TraceVerbose(DBG_IO, "exit with status: %!STATUS!", status);
  }

}

// RMG:
// callback for when a write I/O request enters the SGDMA write queue
VOID EvtIoWriteDma(IN WDFQUEUE wdfQueue, IN WDFREQUEST Request, IN size_t length) {
  UNREFERENCED_PARAMETER(length);
  NTSTATUS status = STATUS_INTERNAL_ERROR;
  PQUEUE_CONTEXT  queue = GetQueueContext(wdfQueue);

  TraceVerbose(DBG_IO, "%!FUNC!(queue=%p, request=%p, length=%llu)", wdfQueue, Request, length);

  XDMA_ENGINE* engine = queue->engine;
  TraceInfo(DBG_IO, "%s_%u writing %llu bytes to device",
    DirectionToString(engine->dir), engine->channel, length);

  // initialize a DMA transaction from the request
  status = WdfDmaTransactionInitializeUsingRequest(
              queue->engine->dmaTransaction,
              Request,
              XDMA_EngineProgramDma,
              WdfDmaDirectionWriteToDevice);

  if (!NT_SUCCESS(status)) {
    TraceError(DBG_IO, "WdfDmaTransactionInitializeUsingRequest failed: %!STATUS!", status);
    goto ErrExit;
  }

  // supply the Queue as context for EvtProgramDma
  status = WdfDmaTransactionExecute(queue->engine->dmaTransaction, queue->engine);
  if (!NT_SUCCESS(status)) {
    TraceError(DBG_IO, "WdfDmaTransactionExecute failed: %!STATUS!", status);
    goto ErrExit;
  }

  return; // success
ErrExit:
  WdfDmaTransactionRelease(queue->engine->dmaTransaction);
  WdfRequestComplete(Request, status);
  TraceError(DBG_IO, "Error Request 0x%p: %!STATUS!", Request, status);
}

// NOTES about DMA:
// https://learn.microsoft.com/en-us/windows-hardware/drivers/wdf/dma-transactions-and-dma-transfers
// DMA transaction
// A DMA transaction is a complete I/O operation, such as a single read or
// write request from an application.
// DMA transfer
// A DMA transfer is a single hardware operation that transfers data from
// computer memory to a device or from the device to computer memory.
//
// A single DMA transaction always consists of at least one DMA transfer,
// but a transaction can consist of many transfers.


// RMG:
//
VOID EvtIoReadDma(IN WDFQUEUE wdfQueue, IN WDFREQUEST Request, IN size_t length)
{
  UNREFERENCED_PARAMETER(length);
  NTSTATUS status = STATUS_INTERNAL_ERROR;
  PQUEUE_CONTEXT queue = GetQueueContext(wdfQueue);

  TraceVerbose(DBG_IO, "%!FUNC!(queue=%p, request=%p, length=%llu)", wdfQueue, Request, length);

  XDMA_ENGINE* engine = queue->engine;
  TraceInfo(DBG_IO, "%s_%u reading %llu bytes from device",
    DirectionToString(engine->dir), engine->channel, length);

  // initialize a DMA transaction from the request
  status = WdfDmaTransactionInitializeUsingRequest(
              queue->engine->dmaTransaction,    // WDFDMATRANSACTION   DmaTransaction,
              Request,                          // WDFREQUEST          Request,
              XDMA_EngineProgramDma,            // PFN_WDF_PROGRAM_DMA EvtProgramDmaFunction,
              WdfDmaDirectionReadFromDevice);   // WDF_DMA_DIRECTION   DmaDirection

  // XDMA_EngineProgramDma es la función 'event callback function', es decir,
  // la que prepara todo.

  if (!NT_SUCCESS(status)) {
    TraceError(DBG_IO, "WdfDmaTransactionInitializeUsingRequest failed: %!STATUS!", status);
    goto ErrExit;
  }

  // supply the Queue as context for EvtProgramDma
  // RMG:                           transaction                    arg for callback
  status = WdfDmaTransactionExecute(queue->engine->dmaTransaction, queue->engine);
  if (!NT_SUCCESS(status)) {
    TraceError(DBG_IO, "WdfDmaTransactionExecute failed: %!STATUS!", status);
    goto ErrExit;
  }

  return; // success
ErrExit:
  WdfDmaTransactionRelease(queue->engine->dmaTransaction);
  WdfRequestComplete(Request, status);
  TraceError(DBG_IO, "Error Request 0x%p: %!STATUS!", Request, status);
}

// RMG: callback para lecturas con interfaces tipo streams !!!!
VOID EvtIoReadEngineRing(IN WDFQUEUE wdfQueue, IN WDFREQUEST Request, IN size_t length) {
  NTSTATUS status = STATUS_UNSUCCESSFUL;
  PQUEUE_CONTEXT queue = GetQueueContext(wdfQueue);
  XDMA_ENGINE *engine = queue->engine;

  // get output buffer
  WDFMEMORY outputMem;
  status = WdfRequestRetrieveOutputMemory(Request, &outputMem);
  if (!NT_SUCCESS(status)) {
    TraceError(DBG_IO, "WdfRequestRetrieveOutputMemory failed: %!STATUS!", status);
    WdfRequestCompleteWithInformation(Request, status, 0);
    return;
  }

  TraceInfo(DBG_IO, "%s_%u requesting %llu bytes from ring buffer",
        DirectionToString(engine->dir), engine->channel, length);

  LARGE_INTEGER timeout;
  timeout.QuadPart = -10 * 10000000; // 10 seconds timeout
  size_t numBytes = 0;
  status = EngineRingCopyBytesToMemory(engine, outputMem, length, timeout, &numBytes);

  WdfRequestCompleteWithInformation(Request, status, numBytes);
}

// RMG: Finalmente parece que no la usan:
VOID EvtCancelReadUserEvent(IN WDFREQUEST request) {
  PFILE_CONTEXT file = GetFileContext(WdfRequestGetFileObject(request));
  KEVENT* event = (KEVENT*)file->u.event->userData;
  KePulseEvent(event, IO_NO_INCREMENT, FALSE);

  NTSTATUS status = WdfRequestUnmarkCancelable(request);
  if (status != STATUS_CANCELLED) {
    WdfRequestComplete(request, STATUS_CANCELLED);
  }
}

// NOTAS sobre eventos:
// https://learn.microsoft.com/en-us/windows-hardware/drivers/kernel/event-objects

// RMG:
NTSTATUS EvtReadUserEvent(WDFREQUEST request, size_t length) {
  NTSTATUS status = 0;
  BOOLEAN eventValue = TRUE;


  dbg_printf(("RMG: [%s] +\n", __FUNCTION__));

  if (length != sizeof(BOOLEAN)) {
    status = STATUS_INVALID_PARAMETER;
    dbg_printf(("RMG: [%s] Error: %d\n", __FUNCTION__, status));
    TraceError(DBG_IO, "Error: %!STATUS!", status);
    goto Exit;
  }

  //status = WdfRequestMarkCancelableEx(request, EvtCancelReadUserEvent);
  //if (!NT_SUCCESS(status)) {
  //  TraceError(DBG_IO, "WdfRequestMarkCancelableEx failed: %!STATUS!", status);
  //  goto Exit;
  //}

  // wait for event to occur - return error on timeout
  PFILE_CONTEXT file = GetFileContext(WdfRequestGetFileObject(request));
  KEVENT *event = (KEVENT *)file->u.event->userData; // userData es un void *

  KeClearEvent(event);  // RMG: set the event to a not-signaled state.
  LARGE_INTEGER timeout;
  timeout.QuadPart = -3 * 10000000; // 3 second timeout (relative time, in 100-nanosecond units)
  status = KeWaitForSingleObject(
              event,        // PVOID Object
              Executive,    // KWAIT_REASON WaitReason
              KernelMode,   // KPROCESSOR_MODE WaitMod,
              FALSE,        // BOOLEAN Alertable,
              &timeout);    // PLARGE_INTEGER Timeout
  if (status == STATUS_TIMEOUT) {
    // RMG:
    dbg_printf(("RMG: [%s] timeout!!!\n", __FUNCTION__));
    eventValue = FALSE;
  }

  // RMG: Obtengo el handle al 'framework memory object' asociado a la
  // solicitud de I/O, esto es, el output buffer.
  //    To access the buffer, the driver must call WdfMemoryGetBuffer.

  // get output buffer
  WDFMEMORY outputMem;
  status = WdfRequestRetrieveOutputMemory(request, &outputMem);
  if (!NT_SUCCESS(status)) {
    TraceError(DBG_IO, "WdfRequestRetrieveOutputMemory failed: %!STATUS!", status);
    goto Exit;
  }

  // get buffer size
  size_t bufSize = 0;
  WdfMemoryGetBuffer(outputMem, &bufSize);  // obtain the size (in bytes) of a memory object's buffer
  if (bufSize != sizeof(BOOLEAN)) {
    TraceError(DBG_IO, "Error: length is %llu but must be %llu", bufSize, sizeof(UINT32));
    status = STATUS_INVALID_PARAMETER;
    goto Exit;
  }

  // RMG: copio al object buffer el valor de eventValue
  // copy the user irq events from our internal buffer to the WdfMemory
  status = WdfMemoryCopyFromBuffer(outputMem, 0, &eventValue, bufSize);
  if (!NT_SUCCESS(status)) {
    TraceError(DBG_IO, "WdfMemoryCopyFromBuffer failed: %!STATUS!", status);
    goto Exit;
  }

  // RMG: completes a specified I/O request, supplies a completion status
  // and stores completion information (bufSize)
  WdfRequestCompleteWithInformation(request, status, bufSize);
  TraceInfo(DBG_IO, "user events returned is 0x%08X", eventValue);

Exit:
  TraceVerbose(DBG_IO, "user EP=0x%08X", eventValue);
  dbg_printf(("RMG: [%s] -\n", __FUNCTION__));
  return status;
}


// RMG: En EvtDevicePrepareHardware (driver.c) se hace fija esta función
// como handler de usuario de los eventos. A esta función se la llama
// desde algún punto de interrupt.c

// Creo que la rutina ISR acaba llamando a esta función que básicamente lo
// que hace es despertar en handler de lectura de eventos EvtReadUserEvent(),
// el cual, lo primero que hace es quedarse dormido a la espera del evento
// que lo despierte (KeWaitForSingleObject())


VOID HandleUserEvent(ULONG eventId, void* userData) {
  dbg_printf(("RMG: [%s] +\n", __FUNCTION__));

  ASSERTMSG("userData=NULL!", userData != NULL);
  KEVENT* event = (KEVENT*)userData;

  dbg_printf(("RMG: [%s] event_%u signaling completion\n", __FUNCTION__, eventId));
  TraceInfo(DBG_IO, "event_%u signaling completion", eventId);

  // RMG: atomically sets an event object to a signaled state, attempts
  // to satisfy as many waits as possible, and then resets the event
  // object to a not-signaled state.
  KePulseEvent(event, IO_NO_INCREMENT, FALSE);
  dbg_printf(("RMG: [%s] -\n", __FUNCTION__));
}

//-----------------------------------------------

// RMG FUSE:

VOID FUSE_EvtIoReadDma(IN WDFQUEUE wdfQueue, IN WDFREQUEST Request, IN size_t length) {
  UNREFERENCED_PARAMETER(length);

  NTSTATUS status = STATUS_INTERNAL_ERROR;
  PQUEUE_CONTEXT queue = GetQueueContext(wdfQueue);
  XDMA_ENGINE *engine = queue->engine;

  // Esto viene de EvtIoReadDma()
  TraceVerbose(DBG_IO, "%!FUNC!(queue=%p, request=%p, length=%llu)", wdfQueue, Request, length);

  // Esto viene de EvtIoReadEngineRing()
  TraceInfo(DBG_IO, "%s_%u reading %llu bytes from device",
    DirectionToString(engine->dir), engine->channel, length);

  // initialize a DMA transaction from the request
  status = WdfDmaTransactionInitializeUsingRequest(
              queue->engine->dmaTransaction,    // WDFDMATRANSACTION   DmaTransaction,
              Request,                          // WDFREQUEST          Request,
              FUSE_EngineProgramDma,            // PFN_WDF_PROGRAM_DMA EvtProgramDmaFunction,
              WdfDmaDirectionReadFromDevice);   // WDF_DMA_DIRECTION   DmaDirection

  if (0) {
    ULONG n_map_regs = 0;
    ULONG n_sg = 0;
    // NOTA:
    // The driver might call WdfDmaTransactionGetTransferInfo BEFORE calling
    // WdfDmaTransactionAllocateResources or WdfDmaTransactionExecute.
    WdfDmaTransactionGetTransferInfo(queue->engine->dmaTransaction, &n_map_regs, &n_sg);
    dbg_printf(("  - n_map_regs = %ld\n", n_map_regs));
    dbg_printf(("  - n_sg = %ld\n", n_sg));
  }

  // FUSE_EngineProgramDma() es la función 'event callback function', es decir,
  // la que prepara todo.

  if (!NT_SUCCESS(status)) {
    TraceError(DBG_IO, "WdfDmaTransactionInitializeUsingRequest failed: %!STATUS!", status);
    goto ErrExit;
  }

  // supply the Queue as context for EvtProgramDma
  status = WdfDmaTransactionExecute(queue->engine->dmaTransaction, queue->engine);
  if (!NT_SUCCESS(status)) {
    TraceError(DBG_IO, "WdfDmaTransactionExecute failed: %!STATUS!", status);
    goto ErrExit;
  }

  return; // success
ErrExit:
  WdfDmaTransactionRelease(queue->engine->dmaTransaction);
  WdfRequestComplete(Request, status);
  TraceError(DBG_IO, "Error Request 0x%p: %!STATUS!", Request, status);
}

NTSTATUS FUSE_EvtReadUserEvent(WDFREQUEST request, size_t length) {
  NTSTATUS wait_status = 0;
  NTSTATUS status = 0;
  fuse_flags_t flags = 0;
  LONG cancel;

  _dbg_note("[%s] + \n", __FUNCTION__);

  if (length != sizeof(fuse_flags_t)) {
    status = STATUS_INVALID_PARAMETER;
    KdPrint(("RMG: [%s] Error: %d\n", __FUNCTION__, status));
    TraceError(DBG_IO, "Error: %!STATUS!", status);
    goto Exit;
  }

#ifdef FUSE_CANCEL_RQT
  // RMG: WdfRequestMarkCancelableEx es la versión moderna de WdfRequestMarkCancelable que
  // evita algunos problemas de deadlock.
  status = WdfRequestMarkCancelableEx(request, FUSE_EvtCancelReadUserEvent);
  if (!NT_SUCCESS(status)) {
    TraceError(DBG_IO, "WdfRequestMarkCancelableEx failed: %!STATUS!", status);
    goto Exit;
  }
#endif

  // RMG:
  fuse_flags_t *usrFlags;
  size_t totalLength;

  status = WdfRequestRetrieveOutputBuffer(
              request,              // WDFREQUEST Request,
              sizeof(fuse_flags_t), // size_t     MinimumRequiredSize,
              &usrFlags,            // PVOID      *Buffer,
              &totalLength);        // size_t     *Length
  if (!NT_SUCCESS(status)) {
    TraceError(DBG_IO, "Retrieve Buffer Failed - Length: %Iu", totalLength);
    _dbg_error("Retrieve Buffer Failed - Length: %Iu\n", totalLength);
    goto Exit;
  }


  //status = WdfRequestMarkCancelableEx(request, EvtCancelReadUserEvent);
  //if (!NT_SUCCESS(status)) {
  //  TraceError(DBG_IO, "WdfRequestMarkCancelableEx failed: %!STATUS!", status);
  //  goto Exit;
  //}

  // En el contexto del fichero tenemos un puntero a una estructura
  // FUSE_XDMA_EVENT, la cual incluye tanto el valor de los flags como el
  // objeto de sincronización (KEVENT)

  // wait for event to occur - return error on timeout
  PFILE_CONTEXT file = GetFileContext(WdfRequestGetFileObject(request));
  FUSE_XDMA_EVENT *fuseEvent = file->u.fuseEvent;
  KEVENT *event;

  if (1) {
    int event_ID = fuseEvent->event_ID;
    switch(event_ID) {
      case SYS_FUSE_DMA_INT_ID:
        _dbg_warning("[%s] Processing FUSE DMA events (interrupts)\n", __FUNCTION__);
        break;
      case SYS_OLINK_INT_ID:
        _dbg_warning("[%s] Processing OPTICAL LINK events (interrupts)\n", __FUNCTION__);
        break;
      case SYS_FUSE_EMUL_INT_ID:
        _dbg_warning("[%s] Processing EMULATOR events (interrupts)\n", __FUNCTION__);
        break;
      default:
        _dbg_warning("[%s] Processing UNKNOWN MODULE events (interrupts)\n", __FUNCTION__);
    }
  }

  //KdPrint(("[%s] (evento %d) Esperando...\n", __FUNCTION__, fuseEvent->event_ID));
  InterlockedExchange(&fuseEvent->cancel, 0);
  event = (KEVENT *)fuseEvent->syncObj;
  KeClearEvent(event);  // RMG: set the event to a not-signaled state.

  LARGE_INTEGER timeout;
  timeout.QuadPart = -TIMEOUT_1_MS * file->timeout_ms;     // timeout (relative time, in 100-nanosecond units)

  wait_status = KeWaitForSingleObject(
                  event,        // PVOID Object
                  Executive,    // KWAIT_REASON WaitReason
                  KernelMode,   // KPROCESSOR_MODE WaitMod,
                  FALSE,        // BOOLEAN Alertable,
                  &timeout);    // PLARGE_INTEGER Timeout

  cancel = InterlockedExchange(&fuseEvent->cancel, 0);
  // RMG: STATUS_TIMEOUT se considera un estado exitoso, es decir, NT_SUCCESS(STATUS_TIMEOUT) != 0
  // https://learn.microsoft.com/en-us/windows-hardware/drivers/kernel/using-ntstatus-values
  if (wait_status == STATUS_TIMEOUT) {
    flags = 0;
    totalLength = 0;
    //KdPrint(("[%s] (evento %d) ...timeout!!!\n", __FUNCTION__, fuseEvent->event_ID));
  }
  else {
    flags = fuseEvent->flags;
  }

  //KdPrint(("[%s] (evento %d) cancel = %d\n", __FUNCTION__, fuseEvent->event_ID, cancel));

  *usrFlags = flags;
  WdfRequestCompleteWithInformation(request, wait_status, totalLength);
  TraceInfo(DBG_IO, "user events returned is 0x%08X", flags);

Exit:
  TraceVerbose(DBG_IO, "user EP=0x%08X", flags);
  _dbg_note(("[%s] - \n", __FUNCTION__));
  return status;
}

VOID FUSE_HandleUserEvent(ULONG eventId, void *syncObj, fuse_flags_t flags) {
  UNREFERENCED_PARAMETER(flags);

  dbg_printf(("RMG: [%s] +\n", __FUNCTION__));

  ASSERTMSG("syncObj=NULL!", syncObj != NULL);
  KEVENT *event = (KEVENT *) syncObj;

  //DbgPrint("RMG: [%s] event_%u signaling completion. flags = 0x%8.8X\n", __FUNCTION__, eventId, flags);
  TraceInfo(DBG_IO, "event_%u signaling completion", eventId);

  // RMG: atomically sets an event object to a signaled state, attempts
  // to satisfy as many waits as possible, and then resets the event
  // object to a not-signaled state.
  KePulseEvent(event, IO_NO_INCREMENT, FALSE);
  dbg_printf(("RMG: [%s] -\n", __FUNCTION__));
}

VOID FUSE_EvtCancelReadUserEvent(IN WDFREQUEST request) {
  KdPrint(("[%s] +\n", __FUNCTION__));
#if 0
  KdPrint(("[%s] Ejecutando callback de cancelación!!!!!\n", __FUNCTION__));
#else
  PFILE_CONTEXT file = GetFileContext(WdfRequestGetFileObject(request));
  FUSE_XDMA_EVENT *fuseEvent = file->u.fuseEvent;
  KEVENT *event = (KEVENT *)fuseEvent->syncObj;
  //fuse_flags_t flags = 0;


  if (KeReadStateEvent(event)) {
    fuseEvent->flags = 0;
    KePulseEvent(event, IO_NO_INCREMENT, FALSE);
  }

  // A driver can call WdfRequestUnmarkCancelable to disable cancellation of
  // an I/O request, if the driver previously called WdfRequestMarkCancelable
  // or WdfRequestMarkCancelableEx to enable cancellation of the request.

  // RMG: Trato de deshacer la cancelación de la solicitud. Si la solicitud ya
  // estaba cancelada devolverá STATUS_CANCELLED y en tal caso no completo la
  // solicitud con STATUS_CANCELLED:
  if (!WdfRequestIsCanceled(request)) {
    KdPrint(("[%s] completando la cancelación....\n", __FUNCTION__));
    WdfRequestComplete(request, STATUS_CANCELLED);
  }


//  NTSTATUS status = WdfRequestUnmarkCancelable(request);
//  if (status != STATUS_CANCELLED) {
//    WdfRequestComplete(request, STATUS_CANCELLED);
//  }
#endif

  KdPrint(("[%s] -\n", __FUNCTION__));
}


//--- IOCTLs:

static NTSTATUS FUSE_IoctlGetFileTimeout(IN WDFREQUEST request, IN FILE_CONTEXT *file, OUT size_t *resp_size) {
  KdPrint(("[%s] +\n", __FUNCTION__));
  // get handle to the IO request memory which will hold the read data
  WDFMEMORY requestMemory;
  NTSTATUS status = WdfRequestRetrieveOutputMemory(request, &requestMemory);
  if (!NT_SUCCESS(status)) {
    TraceError(DBG_IO, "WdfRequestRetrieveOutputMemory failed: %!STATUS!", status);
    return status;
  }
  // copy from source into request memory
  const size_t size = sizeof(UINT64);
  status = WdfMemoryCopyFromBuffer(requestMemory, 0, &file->timeout_ms, size);
  if (!NT_SUCCESS(status)) {
    TraceError(DBG_IO, "WdfMemoryCopyFromBuffer failed: %!STATUS!", status);
    return status;
  }
  *resp_size = size;
  //KdPrint(("[%s] timeout = 0x%I64X\n", __FUNCTION__, file->timeout_ms));
  KdPrint(("[%s] -\n", __FUNCTION__));
  return status;
}

static NTSTATUS FUSE_IoctlSetFileTimeout(IN WDFREQUEST request, IN FILE_CONTEXT *file) {
  KdPrint(("[%s] +\n", __FUNCTION__));

  // get handle to the IO request memory which will hold the write data
  WDFMEMORY requestMemory;
  NTSTATUS status = WdfRequestRetrieveInputMemory(request, &requestMemory);
  if (!NT_SUCCESS(status)) {
    TraceError(DBG_IO, "WdfRequestRetrieveInputMemory failed: %!STATUS!", status);
    //KdPrint(("WdfRequestRetrieveInputMemory failed: = %d\n", status));
    return status;
  }
  // copy from request memory into destination
  status = WdfMemoryCopyToBuffer(requestMemory, 0, &file->timeout_ms, sizeof(file->timeout_ms));
  if (!NT_SUCCESS(status)) {
    TraceError(DBG_IO, "WdfMemoryCopyToBuffer failed: %!STATUS!", status);
    //KdPrint(("WdfMemoryCopyToBuffer failed: = %d\n", status));
    return status;
  }

  //if (file->timeout_ms == FUSE_INFINITE_TIMEOUT) {
  //  KdPrint(("[%s] timeout infinito!!! => 0x%I64X\n", __FUNCTION__, file->timeout_ms));
  //}

  //KdPrint(("[%s] timeout = 0x%I64X\n", __FUNCTION__, file->timeout_ms));

  KdPrint(("[%s] -\n", __FUNCTION__));
  return status;
}

//UCHAR   RevisionID;
//USHORT  SubVendorID;
//USHORT  SubSystemID;
#define PCI_HEADER_SIZE  0x40

static NTSTATUS FUSE_IoctlGetPciInfo(IN WDFREQUEST request, IN XDMA_DEVICE *xdma, OUT size_t *resp_size) {
  UNREFERENCED_PARAMETER(xdma);
  //ASSERT(xdma != NULL);
  // get handle to the IO request memory which will hold the read data
  WDFMEMORY requestMemory;
  NTSTATUS status = WdfRequestRetrieveOutputMemory(request, &requestMemory);
  if (!NT_SUCCESS(status)) {
    TraceError(DBG_IO, "WdfRequestRetrieveOutputMemory failed: %!STATUS!", status);
    return status;
  }
  
  pci_info_t pci_info;
  const size_t size = sizeof(pci_info_t);

  WDFDEVICE device = WdfFileObjectGetDevice(WdfRequestGetFileObject(request));
  BUS_INTERFACE_STANDARD  BusInterface;
  // Get the BUS_INTERFACE_STANDARD for our device so that we can read & write to PCI config space.
  status = WdfFdoQueryForInterface(
              device,
              &GUID_BUS_INTERFACE_STANDARD,
              (PINTERFACE)&BusInterface,
              sizeof(BUS_INTERFACE_STANDARD),
              1,                                // Version
              NULL);                            // InterfaceSpecificData
    
  if (NT_SUCCESS(status)) {
    DECLSPEC_ALIGN(MEMORY_ALLOCATION_ALIGNMENT) UCHAR buffer[PCI_HEADER_SIZE];
    PPCI_COMMON_CONFIG pPciConfig = (PPCI_COMMON_CONFIG)buffer;
    
    ULONG bytesRead = 0;

    RtlZeroMemory(buffer, sizeof(buffer));
    bytesRead = BusInterface.GetBusData(
                  BusInterface.Context,                       // Context
                  PCI_WHICHSPACE_CONFIG,                      // PCI configuration space
                  buffer,                                     // target buffer
                  FIELD_OFFSET(PCI_COMMON_CONFIG, VendorID),  // Offset
                  PCI_HEADER_SIZE);                           // Length

    if (bytesRead == PCI_HEADER_SIZE) {
      pci_info.VendorID = pPciConfig->VendorID;
      pci_info.DeviceID = pPciConfig->DeviceID;
      pci_info.RevisionID = pPciConfig->RevisionID;
      pci_info.SubVendorID = pPciConfig->u.type0.SubVendorID;
      pci_info.SubSystemID = pPciConfig->u.type0.SubSystemID;

      // copy from pci_info into request memory
      status = WdfMemoryCopyFromBuffer(requestMemory, 0, &pci_info, size);
      if (!NT_SUCCESS(status)) {
        TraceError(DBG_IO, "WdfMemoryCopyFromBuffer failed: %!STATUS!", status);
      }
    }
  }
  
  *resp_size = size;
  return status;
}


static NTSTATUS FUSE_IoctlGetRevisionID(IN WDFREQUEST request, IN XDMA_DEVICE *xdma, OUT size_t *resp_size) {
  ASSERT(xdma != NULL);
  // get handle to the IO request memory which will hold the read data
  WDFMEMORY requestMemory;
  NTSTATUS status = WdfRequestRetrieveOutputMemory(request, &requestMemory);
  if (!NT_SUCCESS(status)) {
    TraceError(DBG_IO, "WdfRequestRetrieveOutputMemory failed: %!STATUS!", status);
    return status;
  }
  // copy from perfData into request memory
  const size_t size = sizeof(xdma->RevisionID);
  status = WdfMemoryCopyFromBuffer(requestMemory, 0, &xdma->RevisionID, size);
  if (!NT_SUCCESS(status)) {
    TraceError(DBG_IO, "WdfMemoryCopyFromBuffer failed: %!STATUS!", status);
    return status;
  }
  *resp_size = size;
  return status;
}

static NTSTATUS FUSE_IoctlGetSubSystemID(IN WDFREQUEST request, IN XDMA_DEVICE *xdma, OUT size_t *resp_size) {
  ASSERT(xdma != NULL);
  // get handle to the IO request memory which will hold the read data
  WDFMEMORY requestMemory;
  NTSTATUS status = WdfRequestRetrieveOutputMemory(request, &requestMemory);
  if (!NT_SUCCESS(status)) {
    TraceError(DBG_IO, "WdfRequestRetrieveOutputMemory failed: %!STATUS!", status);
    return status;
  }
  // copy from perfData into request memory
  //KdPrint(("[%s] SubSystemID = 0x%4.4X, sizeof(SubSystemID) = %d\n", __FUNCTION__, xdma->SubSystemID, sizeof(xdma->SubSystemID)));
  KdPrint(("[%s] SubSystemID = 0x%4.4X\n", __FUNCTION__, xdma->SubSystemID));

  const size_t size = sizeof(xdma->SubSystemID);
  status = WdfMemoryCopyFromBuffer(requestMemory, 0, &xdma->SubSystemID, size);
  if (!NT_SUCCESS(status)) {
    TraceError(DBG_IO, "WdfMemoryCopyFromBuffer failed: %!STATUS!", status);
    return status;
  }
  *resp_size = size;
  return status;
}

// ------------------------
// IP CONVENIENCE SERVICES:
// ------------------------

static NTSTATUS FUSE_IoctlIpGetSignature(IN WDFREQUEST request, IN XDMA_DEVICE *xdma, IN int dev_ID, OUT size_t *resp_size) {
  _dbg_note("[%s] + \n", __FUNCTION__);
  ASSERT(xdma != NULL);
  // get handle to the IO request memory which will hold the read data
  WDFMEMORY requestMemory;
  NTSTATUS status = WdfRequestRetrieveOutputMemory(request, &requestMemory);
  if (!NT_SUCCESS(status)) {
    TraceError(DBG_IO, "WdfRequestRetrieveOutputMemory failed: %!STATUS!", status);
    return status;
  }

  UINT32 signature;
  int ok = 1;

  switch(dev_ID) {
    case OLINK_DEV_ID:
      fuse_olink_get_version(&(xdma->olink_drv), &signature);
      break;
    case FUSE_DMA_DEV_ID:
      fuse_dma_get_version(&(xdma->fuse_dma_drv), &signature);
      break;
    case EMUL_DEV_ID:
      fuse_emul_get_version(&(xdma->emul_drv), &signature);
      break;
    default:
      ok = 0;
      break;
  }

  if(ok) {
    _dbg_note("[%s] dev_id = 0x%2.2X, signature = 0x%8.8X\n", __FUNCTION__, dev_ID, signature);
    *resp_size = sizeof(UINT32);
    status = WdfMemoryCopyFromBuffer(requestMemory, 0, &signature, sizeof(UINT32));
    if (!NT_SUCCESS(status)) {
      TraceError(DBG_IO, "WdfMemoryCopyFromBuffer failed: %!STATUS!", status);
    }
  }
  else {
    *resp_size = 0;
    status = STATUS_NOT_SUPPORTED; //STATUS_NO_SUCH_DEVICE;
  }
  _dbg_note("[%s] - \n", __FUNCTION__);
  return status;
}

//---

static NTSTATUS FUSE_IoctlFuseDMAResetHw(IN WDFREQUEST request, IN XDMA_DEVICE *xdma) {
	_dbg_note("[%s] + \n", __FUNCTION__);
	// get handle to the IO request memory which will hold the read data
	WDFMEMORY requestMemory;
	NTSTATUS status = WdfRequestRetrieveInputMemory(request, &requestMemory);
	if (!NT_SUCCESS(status)) {
		TraceError(DBG_IO, "WdfRequestRetrieveInputMemory failed: %!STATUS!", status);
		_dbg_error("WdfRequestRetrieveInputMemory failed: = %d\n", status);
		return status;
	}

	UINT32 mode;
	// copy from request memory into mask;
	status = WdfMemoryCopyToBuffer(requestMemory, 0, &mode, sizeof(UINT32));
	if (!NT_SUCCESS(status)) {
		TraceError(DBG_IO, "WdfMemoryCopyToBuffer failed: %!STATUS!", status);
		_dbg_error("[%s] WdfMemoryCopyToBuffer failed: = %d\n", __FUNCTION__, status);
		return status;
	}

	_dbg_note("[%s] mode = 0x%8.8X\n", __FUNCTION__, mode);
	DbgPrint("[%s] mode = 0x%8.8X\n", __FUNCTION__, mode);

	if (mode & _FUSE_DMA_RST_CFG) {
    int set = mode & _FUSE_DMA_RST_OP_SET;
    if (mode & _FUSE_DMA_RST_MEM) {
      fuse_dma_set_mem_reset(&(xdma->fuse_dma_drv), set);
    }
    if (mode & _FUSE_DMA_RST_CORE) {
      fuse_dma_set_sw_reset(&(xdma->fuse_dma_drv), set);
    }
  }
	else {
    if (mode & _FUSE_DMA_RST_MEM) {
      fuse_dma_set_mem_reset(&(xdma->fuse_dma_drv), 1);
      fuse_kernel_delay_us(1);
      fuse_dma_set_mem_reset(&(xdma->fuse_dma_drv), 0);
      fuse_kernel_delay_us(1);
    }
    if (mode & _FUSE_DMA_RST_CORE) {
      fuse_dma_set_sw_reset(&(xdma->fuse_dma_drv), 1);
      fuse_kernel_delay_us(1);
      fuse_dma_set_sw_reset(&(xdma->fuse_dma_drv), 0);
      fuse_kernel_delay_us(1);
    }
  }

	_dbg_note(("[%s] - \n", __FUNCTION__));
	return status;
}


static NTSTATUS FUSE_IoctlIpResetHw(IN WDFREQUEST request, IN XDMA_DEVICE *xdma, IN int dev_ID) {
  _dbg_note("[%s] + \n", __FUNCTION__);
  // get handle to the IO request memory which will hold the read data
  WDFMEMORY requestMemory;
  NTSTATUS status = WdfRequestRetrieveInputMemory(request, &requestMemory);
  if (!NT_SUCCESS(status)) {
    TraceError(DBG_IO, "WdfRequestRetrieveInputMemory failed: %!STATUS!", status);
    _dbg_error("WdfRequestRetrieveInputMemory failed: = %d\n", status);
    return status;
  }

  UINT32 mode;
  // copy from request memory into mask;
  status = WdfMemoryCopyToBuffer(requestMemory, 0, &mode, sizeof(UINT32));
  if (!NT_SUCCESS(status)) {
    TraceError(DBG_IO, "WdfMemoryCopyToBuffer failed: %!STATUS!", status);
    _dbg_error("[%s] WdfMemoryCopyToBuffer failed: = %d\n", __FUNCTION__, status);
    return status;
  }

  _dbg_note("[%s] dev_id = 0x%2.2X, mode = 0x%8.8X\n", __FUNCTION__, dev_ID, mode);

  int ok = 1;
  switch(dev_ID) {
    case OLINK_DEV_ID:
      fuse_olink_set_sw_reset(&(xdma->olink_drv), 1);
      fuse_kernel_delay_us(1);
      fuse_olink_set_sw_reset(&(xdma->olink_drv), 0);
      fuse_kernel_delay_us(1);
      break;
    case FUSE_DMA_DEV_ID:
      if (mode & _FUSE_DMA_RST_MEM) {
        fuse_dma_set_mem_reset(&(xdma->fuse_dma_drv), 1);
        fuse_kernel_delay_us(1);
        fuse_dma_set_mem_reset(&(xdma->fuse_dma_drv), 0);
        fuse_kernel_delay_us(1);
      }
      if (mode & _FUSE_DMA_RST_CORE) {
        fuse_dma_set_sw_reset(&(xdma->fuse_dma_drv), 1);
        fuse_kernel_delay_us(1);
        fuse_dma_set_sw_reset(&(xdma->fuse_dma_drv), 0);
        fuse_kernel_delay_us(1);
      }
      break;
    case EMUL_DEV_ID:
      fuse_emul_set_sw_reset(&(xdma->emul_drv), 1);
      fuse_kernel_delay_us(1);
      fuse_emul_set_sw_reset(&(xdma->emul_drv), 0);
      fuse_kernel_delay_us(1);
      break;
    default:
      ok = 0;
      break;
  }
  _dbg_note(("[%s] - \n", __FUNCTION__));
  return status;
}

//---

static NTSTATUS FUSE_IoctlIpEnable(IN WDFREQUEST request, IN XDMA_DEVICE *xdma, IN int dev_ID) {
  _dbg_note("[%s] + \n", __FUNCTION__);
  // get handle to the IO request memory which will hold the read data
  WDFMEMORY requestMemory;
  NTSTATUS status = WdfRequestRetrieveInputMemory(request, &requestMemory);
  if (!NT_SUCCESS(status)) {
    TraceError(DBG_IO, "WdfRequestRetrieveInputMemory failed: %!STATUS!", status);
    _dbg_error("WdfRequestRetrieveInputMemory failed: = %d\n", status);
    return status;
  }

  UINT32 value;
  // copy from request memory into mask;
  status = WdfMemoryCopyToBuffer(requestMemory, 0, &value, sizeof(UINT32));
  if (!NT_SUCCESS(status)) {
    TraceError(DBG_IO, "WdfMemoryCopyToBuffer failed: %!STATUS!", status);
    _dbg_error("[%s] WdfMemoryCopyToBuffer failed: = %d\n", __FUNCTION__, status);
    return status;
  }

  _dbg_note("[%s] dev_id = 0x%2.2X, value = 0x%8.8X\n", __FUNCTION__, dev_ID, value);
  DbgPrint("[%s] dev_id = 0x%2.2X, value = 0x%8.8X\n", __FUNCTION__, dev_ID, value);

  int ok = 1;
  switch(dev_ID) {
    case OLINK_DEV_ID:
      fuse_olink_set_enable(&(xdma->olink_drv), value);
      break;
    case FUSE_DMA_DEV_ID:
      fuse_dma_set_rx_enable(&(xdma->fuse_dma_drv), value);
      // fuse_dma_set_enable(&(xdma->fuse_dma_drv), value);		!!!!!!
      break;
    case EMUL_DEV_ID:
      fuse_emul_set_enable(&(xdma->emul_drv), value);
      break;
    default:
      ok = 0;
      break;
  }
  _dbg_note(("[%s] - \n", __FUNCTION__));
  return status;
}

//---

static NTSTATUS FUSE_IoctlIpEnableEvents(IN WDFREQUEST request, IN XDMA_DEVICE *xdma, IN int dev_ID) {
  _dbg_note("[%s] + \n", __FUNCTION__);
  // get handle to the IO request memory which will hold the read data
  WDFMEMORY requestMemory;
  NTSTATUS status = WdfRequestRetrieveInputMemory(request, &requestMemory);
  if (!NT_SUCCESS(status)) {
    TraceError(DBG_IO, "WdfRequestRetrieveInputMemory failed: %!STATUS!", status);
    _dbg_error("WdfRequestRetrieveInputMemory failed: = %d\n", status);
    return status;
  }

  UINT32 mask;
  // copy from request memory into mask;
  status = WdfMemoryCopyToBuffer(requestMemory, 0, &mask, sizeof(UINT32));
  if (!NT_SUCCESS(status)) {
    TraceError(DBG_IO, "WdfMemoryCopyToBuffer failed: %!STATUS!", status);
    _dbg_error("[%s] WdfMemoryCopyToBuffer failed: = %d\n", __FUNCTION__, status);
    return status;
  }

  _dbg_note("[%s] dev_id = 0x%2.2X, mask = 0x%8.8X\n", __FUNCTION__, dev_ID, mask);
  
  int ok = 1;
  switch(dev_ID) {
    case OLINK_DEV_ID:
      fuse_olink_set_int_enable(&(xdma->olink_drv), mask);
      break;
    case FUSE_DMA_DEV_ID:
      fuse_dma_set_int_enable(&(xdma->fuse_dma_drv), mask);
      break;
    case EMUL_DEV_ID:
      fuse_emul_set_int_enable(&(xdma->emul_drv), mask);
      break;
    default:
      ok = 0;
      break;
  }
  _dbg_note(("[%s] - \n", __FUNCTION__));
  return status;
}

// ----------------------
// OPTICAL LINK SERVICES:
// ----------------------

static NTSTATUS FUSE_IoctlOlinkGetConnStatus(IN WDFREQUEST request, IN XDMA_DEVICE *xdma, OUT size_t *resp_size) {
  _dbg_note("[%s] + \n", __FUNCTION__);
  ASSERT(xdma != NULL);
  // get handle to the IO request memory which will hold the read data
  WDFMEMORY requestMemory;
  NTSTATUS status = WdfRequestRetrieveOutputMemory(request, &requestMemory);
  if (!NT_SUCCESS(status)) {
    TraceError(DBG_IO, "WdfRequestRetrieveOutputMemory failed: %!STATUS!", status);
    return status;
  }

  UINT32 aurora_status = 0;
  fuse_olink_get_aurora_status(&(xdma->olink_drv), &aurora_status);
  _dbg_note("[%s] aurora_status = %d\n", __FUNCTION__, aurora_status);

  status = WdfMemoryCopyFromBuffer(requestMemory, 0, &aurora_status, sizeof(UINT32));
  if (!NT_SUCCESS(status)) {
    TraceError(DBG_IO, "WdfMemoryCopyFromBuffer failed: %!STATUS!", status);
    return status;
  }
  *resp_size = sizeof(UINT32);
  _dbg_note("[%s] - \n", __FUNCTION__);
  return status;
}

static NTSTATUS FUSE_IoctlI2cSendByte(IN WDFREQUEST request, IN XDMA_DEVICE *xdma, OUT size_t *resp_size) {
  _dbg_note("[%s] + \n", __FUNCTION__);
  ASSERT(xdma != NULL);
  // get handle to the IO request memory which will hold the read data
  WDFMEMORY requestMemory;
  NTSTATUS status = WdfRequestRetrieveInputMemory(request, &requestMemory);
  if (!NT_SUCCESS(status)) {
    TraceError(DBG_IO, "WdfRequestRetrieveInputMemory failed: %!STATUS!", status);
    _dbg_error("[%s] WdfRequestRetrieveInputMemory failed: = %d\n", __FUNCTION__, status);
    return status;
  }
  _fuse_i2c_cmd i2c_cmd;

  // copy from request memory into conf_emul;
  status = WdfMemoryCopyToBuffer(requestMemory, 0, &i2c_cmd, sizeof(_fuse_i2c_cmd));
  if (!NT_SUCCESS(status)) {
    TraceError(DBG_IO, "WdfMemoryCopyToBuffer failed: %!STATUS!", status);
    _dbg_error("[%s] WdfMemoryCopyToBuffer failed: = %d\n", __FUNCTION__, status);
    return status;
  }

  int ret;
  ret = fuse_olink_i2c_send_byte(&(xdma->olink_drv), i2c_cmd.byte, i2c_cmd.trf_attr);
  _dbg_note("[%s] I2C cmd: ret = %d, trf_attr = 0x%2.2X, byte = 0x%2.2X\n", __FUNCTION__, ret, i2c_cmd.trf_attr, i2c_cmd.byte);

  if (ret >= 0) {
    *resp_size = ret;
    status = STATUS_SUCCESS;
  }
  else {
    *resp_size = 0;
    status = STATUS_IO_TIMEOUT;
  }

  _dbg_note("[%s] - \n", __FUNCTION__);
  return status;
}

static NTSTATUS FUSE_IoctlI2cRecvByte(IN WDFREQUEST request, IN XDMA_DEVICE *xdma, OUT size_t *resp_size) {
  _dbg_note("[%s] + \n", __FUNCTION__);
  ASSERT(xdma != NULL);

  WDFMEMORY requestMemory;
  NTSTATUS status = WdfRequestRetrieveOutputMemory(request, &requestMemory);
  if (!NT_SUCCESS(status)) {
    TraceError(DBG_IO, "WdfRequestRetrieveOutputMemory failed: %!STATUS!", status);
    return status;
  }

  _fuse_i2c_cmd i2c_cmd;
  status = WdfMemoryCopyToBuffer(requestMemory, 0, &i2c_cmd, sizeof(_fuse_i2c_cmd));
  if (!NT_SUCCESS(status)) {
    TraceError(DBG_IO, "WdfMemoryCopyToBuffer failed: %!STATUS!", status);
    return status;
  }

  int ret;
  UINT8 byte;
  UINT32 trf_attr = i2c_cmd.trf_attr;

  ret = fuse_olink_i2c_recv_byte(&(xdma->olink_drv), &byte, trf_attr);
  _dbg_note("[%s] I2C cmd: ret = %d, trf_attr = 0x%2.2X, byte = 0x%2.2X\n", __FUNCTION__, ret, trf_attr, byte);

  i2c_cmd.byte = byte;
  status = WdfMemoryCopyFromBuffer(requestMemory, 0, &i2c_cmd, sizeof(_fuse_i2c_cmd));
  if (!NT_SUCCESS(status)) {
    TraceError(DBG_IO, "WdfMemoryCopyFromBuffer failed: %!STATUS!", status);
    return status;
  }
  *resp_size = sizeof(_fuse_i2c_cmd);
  status = (ret == 1) ? STATUS_SUCCESS : STATUS_IO_TIMEOUT;

  _dbg_note("[%s] - \n", __FUNCTION__);
  return status;
}

static NTSTATUS FUSE_IoctlI2cRstBrdMux(IN WDFREQUEST request, IN XDMA_DEVICE *xdma) {
  _dbg_note("[%s] + \n", __FUNCTION__);
  // get handle to the IO request memory which will hold the read data
  WDFMEMORY requestMemory;
  NTSTATUS status = WdfRequestRetrieveInputMemory(request, &requestMemory);
  if (!NT_SUCCESS(status)) {
    TraceError(DBG_IO, "WdfRequestRetrieveInputMemory failed: %!STATUS!", status);
    _dbg_error("[%s] WdfRequestRetrieveInputMemory failed: = %d\n", __FUNCTION__, status);
    return status;
  }
  UINT32 rst;

  // copy from request memory into rst;
  status = WdfMemoryCopyToBuffer(requestMemory, 0, &rst, sizeof(UINT32));
  if (!NT_SUCCESS(status)) {
    TraceError(DBG_IO, "WdfMemoryCopyToBuffer failed: %!STATUS!", status);
    _dbg_error("[%s] WdfMemoryCopyToBuffer failed: = %d\n", __FUNCTION__, status);
    return status;
  }

  _dbg_note("[%s] rst = 0x%8.8X\n", __FUNCTION__, rst);
  fuse_olink_set_brd_i2c_reset(&(xdma->olink_drv), rst);

  _dbg_note("[%s] - \n", __FUNCTION__);
  return status;
}

// ------------------
// FUSE DMA SERVICES:
// ------------------
#if 0
static NTSTATUS FUSE_IoctlFuseDmaRxEnable(IN WDFREQUEST request, IN XDMA_DEVICE *xdma) {
  _dbg_note("[%s] + \n", __FUNCTION__);
  // get handle to the IO request memory which will hold the read data
  WDFMEMORY requestMemory;
  NTSTATUS status = WdfRequestRetrieveInputMemory(request, &requestMemory);
  if (!NT_SUCCESS(status)) {
    TraceError(DBG_IO, "WdfRequestRetrieveInputMemory failed: %!STATUS!", status);
    _dbg_error("WdfRequestRetrieveInputMemory failed: = %d\n", status);
    return status;
  }
  UINT32 enable;

  // copy from request memory into mask;
  status = WdfMemoryCopyToBuffer(requestMemory, 0, &enable, sizeof(UINT32));
  if (!NT_SUCCESS(status)) {
    TraceError(DBG_IO, "WdfMemoryCopyToBuffer failed: %!STATUS!", status);
    _dbg_error("[%s] WdfMemoryCopyToBuffer failed: = %d\n", __FUNCTION__, status);
    return status;
  }

  _dbg_note("[%s] enable = 0x%8.8X\n", __FUNCTION__, mask);
  fuse_dma_set_rx_enable(&(xdma->fuse_dma_drv), enable);

  _dbg_note(("[%s] - \n", __FUNCTION__));
  return status;
}
#endif

static NTSTATUS FUSE_IoctlFuseDmaFlush(IN WDFREQUEST request, IN XDMA_DEVICE *xdma) {
  _dbg_note("[%s] + \n", __FUNCTION__);
  // get handle to the IO request memory which will hold the read data
  WDFMEMORY requestMemory;
  NTSTATUS status = WdfRequestRetrieveInputMemory(request, &requestMemory);
  if (!NT_SUCCESS(status)) {
    TraceError(DBG_IO, "WdfRequestRetrieveInputMemory failed: %!STATUS!", status);
    _dbg_error("WdfRequestRetrieveInputMemory failed: = %d\n", status);
    return status;
  }
  UINT32 flush;

  // copy from request memory into mask;
  status = WdfMemoryCopyToBuffer(requestMemory, 0, &flush, sizeof(UINT32));
  if (!NT_SUCCESS(status)) {
    TraceError(DBG_IO, "WdfMemoryCopyToBuffer failed: %!STATUS!", status);
    _dbg_error("[%s] WdfMemoryCopyToBuffer failed: = %d\n", __FUNCTION__, status);
    return status;
  }

  _dbg_note("[%s] flush = 0x%8.8X\n", __FUNCTION__, flush);
  fuse_dma_flush(&(xdma->fuse_dma_drv));

  _dbg_note("[%s] - \n", __FUNCTION__);
  return status;
}

static NTSTATUS FUSE_IoctlFuseDmaGetFillingLevel(IN WDFREQUEST request, IN XDMA_DEVICE *xdma, OUT size_t *resp_size) {
  _dbg_note("[%s] + \n", __FUNCTION__);
  ASSERT(xdma != NULL);
  // get handle to the IO request memory which will hold the read data
  WDFMEMORY requestMemory;
  NTSTATUS status = WdfRequestRetrieveOutputMemory(request, &requestMemory);
  if (!NT_SUCCESS(status)) {
    TraceError(DBG_IO, "WdfRequestRetrieveOutputMemory failed: %!STATUS!", status);
    return status;
  }

  int level = 0;
  fuse_dma_get_used_buffers(&(xdma->fuse_dma_drv), &level);
  _dbg_note("[%s] level = %d\n", __FUNCTION__, level);

  status = WdfMemoryCopyFromBuffer(requestMemory, 0, &level, sizeof(int));
  if (!NT_SUCCESS(status)) {
    TraceError(DBG_IO, "WdfMemoryCopyFromBuffer failed: %!STATUS!", status);
    return status;
  }
  *resp_size = sizeof(int);
  _dbg_note("[%s] - \n", __FUNCTION__);
  return status;
}

static NTSTATUS FUSE_IoctlFuseDmaGetThrs(IN WDFREQUEST request, IN XDMA_DEVICE *xdma, OUT size_t *resp_size) {
  _dbg_note("[%s] + \n", __FUNCTION__);
  ASSERT(xdma != NULL);
  // get handle to the IO request memory which will hold the read data
  WDFMEMORY requestMemory;
  NTSTATUS status = WdfRequestRetrieveOutputMemory(request, &requestMemory);
  if (!NT_SUCCESS(status)) {
    TraceError(DBG_IO, "WdfRequestRetrieveOutputMemory failed: %!STATUS!", status);
    return status;
  }

  level_thr_t level;
  fuse_dma_get_thresholds(&(xdma->fuse_dma_drv), &level.thr_l, &level.thr_h);
  _dbg_note("[%s] thr_l = %d, thr_h = %d\n", __FUNCTION__, level.thr_l, level.thr_h);

  status = WdfMemoryCopyFromBuffer(requestMemory, 0, &level, sizeof(level_thr_t));
  if (!NT_SUCCESS(status)) {
    TraceError(DBG_IO, "WdfMemoryCopyFromBuffer failed: %!STATUS!", status);
    return status;
  }
  *resp_size = sizeof(level_thr_t);
  _dbg_note("[%s] - \n", __FUNCTION__);
  return status;
}

static NTSTATUS FUSE_IoctlFuseDmaSetThrs(IN WDFREQUEST request, IN XDMA_DEVICE *xdma) {
  _dbg_note("[%s] + \n", __FUNCTION__);
  // get handle to the IO request memory which will hold the read data
  WDFMEMORY requestMemory;
  NTSTATUS status = WdfRequestRetrieveInputMemory(request, &requestMemory);
  if (!NT_SUCCESS(status)) {
    TraceError(DBG_IO, "WdfRequestRetrieveInputMemory failed: %!STATUS!", status);
    _dbg_error("WdfRequestRetrieveInputMemory failed: = %d\n", status);
    return status;
  }
  level_thr_t level;
  int ret;

  // copy from request memory into mask;
  status = WdfMemoryCopyToBuffer(requestMemory, 0, &level, sizeof(level_thr_t));
  if (!NT_SUCCESS(status)) {
    TraceError(DBG_IO, "WdfMemoryCopyToBuffer failed: %!STATUS!", status);
    _dbg_error("[%s] WdfMemoryCopyToBuffer failed: = %d\n", __FUNCTION__, status);
    return status;
  }

  _dbg_note("[%s] thr_l = %d, thr_h = %d\n", __FUNCTION__, level.thr_l, level.thr_h);
  ret = fuse_dma_set_thresholds(&(xdma->fuse_dma_drv), level.thr_l, level.thr_h);
  status = (!ret)? STATUS_SUCCESS : STATUS_INVALID_PARAMETER;

  _dbg_note("[%s] - \n", __FUNCTION__);
  return status;
}

//----

static NTSTATUS FUSE_IoctlFuseDmaGetIperfData(IN WDFREQUEST request, IN XDMA_DEVICE *xdma, OUT size_t *resp_size) {
  _dbg_note("[%s] + \n", __FUNCTION__);
  ASSERT(xdma != NULL);
  // get handle to the IO request memory which will hold the read data
  WDFMEMORY requestMemory;
  NTSTATUS status = WdfRequestRetrieveOutputMemory(request, &requestMemory);
  if (!NT_SUCCESS(status)) {
    TraceError(DBG_IO, "WdfRequestRetrieveOutputMemory failed: %!STATUS!", status);
    return status;
  }

  fuse_perf_data iperf;
  fuse_dma_get_perf_cycles(&(xdma->fuse_dma_drv), &iperf.total_cycles, &iperf.active_cycles);
  _dbg_note("[%s] total_cycles = %d, active_cycles = %d\n", __FUNCTION__, iperf.total_cycles, iperf.active_cycles);

  status = WdfMemoryCopyFromBuffer(requestMemory, 0, &iperf, sizeof(fuse_perf_data));
  if (!NT_SUCCESS(status)) {
    TraceError(DBG_IO, "WdfMemoryCopyFromBuffer failed: %!STATUS!", status);
    return status;
  }
  *resp_size = sizeof(fuse_perf_data);
  _dbg_note("[%s] - \n", __FUNCTION__);
  return status;
}

static NTSTATUS FUSE_IoctlFuseDmaGetImgId(IN WDFREQUEST request, IN XDMA_DEVICE *xdma, OUT size_t *resp_size) {
  _dbg_note("[%s] + \n", __FUNCTION__);
  ASSERT(xdma != NULL);
  // get handle to the IO request memory which will hold the read data
  WDFMEMORY requestMemory;
  NTSTATUS status = WdfRequestRetrieveOutputMemory(request, &requestMemory);
  if (!NT_SUCCESS(status)) {
    TraceError(DBG_IO, "WdfRequestRetrieveOutputMemory failed: %!STATUS!", status);
    return status;
  }

  UINT32 img_id;
  fuse_dma_get_image_id_seed(&(xdma->fuse_dma_drv), &img_id);
  _dbg_note("[%s] image id = %d\n", __FUNCTION__, img_id);

  status = WdfMemoryCopyFromBuffer(requestMemory, 0, &img_id, sizeof(UINT32));
  if (!NT_SUCCESS(status)) {
    TraceError(DBG_IO, "WdfMemoryCopyFromBuffer failed: %!STATUS!", status);
    return status;
  }
  *resp_size = sizeof(UINT32);
  _dbg_note("[%s] - \n", __FUNCTION__);
  return status;
}

static NTSTATUS FUSE_IoctlFuseDmaSetImgId(IN WDFREQUEST request, IN XDMA_DEVICE *xdma) {
  _dbg_note("[%s] + \n", __FUNCTION__);
  // get handle to the IO request memory which will hold the read data
  WDFMEMORY requestMemory;
  NTSTATUS status = WdfRequestRetrieveInputMemory(request, &requestMemory);
  if (!NT_SUCCESS(status)) {
    TraceError(DBG_IO, "WdfRequestRetrieveInputMemory failed: %!STATUS!", status);
    _dbg_error("WdfRequestRetrieveInputMemory failed: = %d\n", status);
    return status;
  }
  UINT32 img_id;
  int ret;

  // copy from request memory into mask;
  status = WdfMemoryCopyToBuffer(requestMemory, 0, &img_id, sizeof(UINT32));
  if (!NT_SUCCESS(status)) {
    TraceError(DBG_IO, "WdfMemoryCopyToBuffer failed: %!STATUS!", status);
    _dbg_error("[%s] WdfMemoryCopyToBuffer failed: = %d\n", __FUNCTION__, status);
    return status;
  }

  _dbg_note("[%s] image id = %d\n", __FUNCTION__, img_id);
  ret = fuse_dma_set_image_id_seed(&(xdma->fuse_dma_drv), img_id);
  status = (!ret)? STATUS_SUCCESS : STATUS_INVALID_PARAMETER;

  _dbg_note("[%s] - \n", __FUNCTION__);
  return status;
}

static NTSTATUS FUSE_IoctlFuseDmaGetMemSize(IN WDFREQUEST request, IN XDMA_DEVICE *xdma, OUT size_t *resp_size) {
  _dbg_note("[%s] + \n", __FUNCTION__);
  ASSERT(xdma != NULL);
  // get handle to the IO request memory which will hold the read data
  WDFMEMORY requestMemory;
  NTSTATUS status = WdfRequestRetrieveOutputMemory(request, &requestMemory);
  if (!NT_SUCCESS(status)) {
    TraceError(DBG_IO, "WdfRequestRetrieveOutputMemory failed: %!STATUS!", status);
    return status;
  }

  UINT32 msize_code, mem_size_mb;
  fuse_dma_get_mem_size(&(xdma->fuse_dma_drv), &msize_code);
  _dbg_note("[%s] msize_code = %d\n", __FUNCTION__, msize_code);
  mem_size_mb = 1U << msize_code;

  status = WdfMemoryCopyFromBuffer(requestMemory, 0, &mem_size_mb, sizeof(UINT32));
  if (!NT_SUCCESS(status)) {
    TraceError(DBG_IO, "WdfMemoryCopyFromBuffer failed: %!STATUS!", status);
    return status;
  }
  *resp_size = sizeof(UINT32);
  _dbg_note("[%s] - \n", __FUNCTION__);
  return status;
}

static NTSTATUS FUSE_IoctlFuseDmaDnaOK(IN WDFREQUEST request, IN XDMA_DEVICE *xdma, OUT size_t *resp_size) {
  _dbg_note("[%s] + \n", __FUNCTION__);
  ASSERT(xdma != NULL);
  // get handle to the IO request memory which will hold the read data
  WDFMEMORY requestMemory;
  NTSTATUS status = WdfRequestRetrieveOutputMemory(request, &requestMemory);
  if (!NT_SUCCESS(status)) {
    TraceError(DBG_IO, "WdfRequestRetrieveOutputMemory failed: %!STATUS!", status);
    return status;
  }

  UINT32 dna_ok;
  dna_ok = fuse_dma_dna_ok(&(xdma->fuse_dma_drv));
  _dbg_note("[%s] dna_ok = %d\n", __FUNCTION__, dna_ok);

  status = WdfMemoryCopyFromBuffer(requestMemory, 0, &dna_ok, sizeof(UINT32));
  if (!NT_SUCCESS(status)) {
    TraceError(DBG_IO, "WdfMemoryCopyFromBuffer failed: %!STATUS!", status);
    return status;
  }
  *resp_size = sizeof(UINT32);
  _dbg_note("[%s] - \n", __FUNCTION__);
  return status;
}


// -----------------------
// FUSE EMULATOR SERVICES:
// -----------------------

static NTSTATUS FUSE_IoctlEmulStart(IN WDFREQUEST request, IN XDMA_DEVICE *xdma) {
  _dbg_note("[%s] + \n", __FUNCTION__);
  ASSERT(xdma != NULL);
  // get handle to the IO request memory which will hold the read data
  WDFMEMORY requestMemory;
  NTSTATUS status = WdfRequestRetrieveInputMemory(request, &requestMemory);
  if (!NT_SUCCESS(status)) {
    TraceError(DBG_IO, "WdfRequestRetrieveInputMemory failed: %!STATUS!", status);
    _dbg_error("[%s] WdfRequestRetrieveInputMemory failed: = %d\n", __FUNCTION__, status);
    return status;
  }
  hw_img_conf_t emul_conf;

  // copy from request memory into conf_emul;
  status = WdfMemoryCopyToBuffer(requestMemory, 0, &emul_conf, sizeof(hw_img_conf_t));
  if (!NT_SUCCESS(status)) {
    TraceError(DBG_IO, "WdfMemoryCopyToBuffer failed: %!STATUS!", status);
    _dbg_error("[%s] WdfMemoryCopyToBuffer failed: = %d\n", __FUNCTION__, status);
    return status;
  }

#if 0
  KdPrint(("[%s] Emulation start request\n", __FUNCTION__));
  KdPrint(("    - n_images     = %d\n", emul_conf.n_images));
  KdPrint(("    - frame_length = %d\n", emul_conf.frame_len));
  KdPrint(("    - image_length = %d\n", emul_conf.image_len));
  KdPrint(("    - gap          = %d\n", emul_conf.gap));
  KdPrint(("    - mode         = %d\n", emul_conf.mode));
#endif

  int mode_cont = (emul_conf.mode & FUSE_EMUL_MODE_CONT) ? 1 : 0;
  int mode_nb = (emul_conf.mode & FUSE_EMUL_MODE_NBLOCK) ? 1 : 0;
  int ret;
  ret = fuse_emul_generate(
            &(xdma->emul_drv),
            emul_conf.n_images,
            emul_conf.frame_len,
            emul_conf.image_len,
            emul_conf.gap,
            mode_cont,
            mode_nb);

  status = (ret < 0)? STATUS_IO_TIMEOUT : STATUS_SUCCESS;

  _dbg_note("[%s] - \n", __FUNCTION__);
  return status;
}

static NTSTATUS FUSE_IoctlEmulStopCont(IN WDFREQUEST request, IN XDMA_DEVICE *xdma) {
  _dbg_note("[%s] + \n", __FUNCTION__);
  ASSERT(xdma != NULL);
  // get handle to the IO request memory which will hold the read data
  WDFMEMORY requestMemory;
  NTSTATUS status = WdfRequestRetrieveInputMemory(request, &requestMemory);
  if (!NT_SUCCESS(status)) {
    TraceError(DBG_IO, "WdfRequestRetrieveInputMemory failed: %!STATUS!", status);
    _dbg_error("[%s] WdfRequestRetrieveInputMemory failed: = %d\n", __FUNCTION__, status);
    return status;
  }
  UINT32 mode;
  int ret;

  // copy from request memory into conf_emul;
  status = WdfMemoryCopyToBuffer(requestMemory, 0, &mode, sizeof(UINT32));
  if (!NT_SUCCESS(status)) {
    TraceError(DBG_IO, "WdfMemoryCopyToBuffer failed: %!STATUS!", status);
    _dbg_error("[%s] WdfMemoryCopyToBuffer failed: = %d\n", __FUNCTION__, status);
    return status;
  }

  int mode_nb = (mode & FUSE_EMUL_MODE_NBLOCK) ? 1 : 0;
  ret = fuse_emul_stop_cont(&(xdma->emul_drv), mode_nb);

  status = (ret < 0)? STATUS_IO_TIMEOUT : STATUS_SUCCESS;

  _dbg_note("[%s] - \n", __FUNCTION__);
  return status;
}

//-------------------------

static NTSTATUS FUSE_IoctlCancelReadUserEvent(IN WDFREQUEST request, IN FILE_CONTEXT *file);


// ------------------
// PCIe DMA SERVICES:
// ------------------

static NTSTATUS FUSE_IoctlGetDmaTimeout(IN WDFREQUEST request, IN FILE_CONTEXT *file, OUT size_t *resp_size) {
  _dbg_note("[%s] + \n", __FUNCTION__);

  QUEUE_CONTEXT *queue = GetQueueContext(file->queue);
  XDMA_ENGINE *engine = queue->engine;
  // get handle to the IO request memory which will hold the read data
  WDFMEMORY requestMemory;
  NTSTATUS status = WdfRequestRetrieveOutputMemory(request, &requestMemory);
  if (!NT_SUCCESS(status)) {
    TraceError(DBG_IO, "WdfRequestRetrieveOutputMemory failed: %!STATUS!", status);
    return status;
  }
  // copy from source into request memory
  const size_t size = sizeof(UINT64);
  status = WdfMemoryCopyFromBuffer(requestMemory, 0, &engine->timeout_ms, size);
  if (!NT_SUCCESS(status)) {
    TraceError(DBG_IO, "WdfMemoryCopyFromBuffer failed: %!STATUS!", status);
    return status;
  }
  *resp_size = size;
  _dbg_note("[%s] timeout = 0x%I64X\n", __FUNCTION__, engine->timeout_ms);
  
  _dbg_note("[%s] - \n", __FUNCTION__);
  return status;
}

static NTSTATUS FUSE_IoctlSetDmaTimeout(IN WDFREQUEST request, IN FILE_CONTEXT *file) {
  _dbg_note("[%s] + \n", __FUNCTION__);

  QUEUE_CONTEXT *queue = GetQueueContext(file->queue);
  XDMA_ENGINE *engine = queue->engine;
  // get handle to the IO request memory which will hold the write data
  WDFMEMORY requestMemory;
  NTSTATUS status = WdfRequestRetrieveInputMemory(request, &requestMemory);
  if (!NT_SUCCESS(status)) {
    TraceError(DBG_IO, "WdfRequestRetrieveInputMemory failed: %!STATUS!", status);
    //KdPrint(("WdfRequestRetrieveInputMemory failed: = %d\n", status));
    return status;
  }
  // copy from request memory into destination
  status = WdfMemoryCopyToBuffer(requestMemory, 0, &engine->timeout_ms, sizeof(engine->timeout_ms));
  if (!NT_SUCCESS(status)) {
    TraceError(DBG_IO, "WdfMemoryCopyToBuffer failed: %!STATUS!", status);
    //KdPrint(("WdfMemoryCopyToBuffer failed: = %d\n", status));
    return status;
  }
  _dbg_note("[%s] timeout = 0x%I64X\n", __FUNCTION__, engine->timeout_ms);

  _dbg_note("[%s] - \n", __FUNCTION__);
  return status;
}


//-------------------------------------

static int FUSE_io_dev_control(
  IN PFILE_CONTEXT  file,
  IN WDFREQUEST     request,
  IN XDMA_DEVICE    *xdma,
  IN ULONG          IoControlCode)
{
  size_t resp_size;
  int served = 0;
  NTSTATUS status = STATUS_INTERNAL_ERROR;

  if (xdma->FUSE_valid_design) {
    //KdPrint(("[%s] Procesando mis IOCTLs, devType = %d, IoControlCode= 0x%X\n", __FUNCTION__, (int) file->devType, IoControlCode));
    switch (file->devType) {
      case DEVNODE_TYPE_BYPASS:
        break;
      case DEVNODE_TYPE_CONTROL:
        break;
      case DEVNODE_TYPE_EVENTS:
        switch (IoControlCode) {
          case IOCTL_FUSE_GET_FILE_TIMEOUT:
            served = 1;
            status = FUSE_IoctlGetFileTimeout(request, file, &resp_size);
            WdfRequestCompleteWithInformation(request, status, resp_size);
            break;
          case IOCTL_FUSE_SET_FILE_TIMEOUT:
            served = 1;
            status = FUSE_IoctlSetFileTimeout(request, file);
            WdfRequestComplete(request, status);
            break;
          case IOCTL_FUSE_CANCEL_USR_EVENT:
            served = 1;
            status = FUSE_IoctlCancelReadUserEvent(request, file);
            WdfRequestComplete(request, status);
            break;

          default:
            served = 0;
            break;
        }
        break;
      case DEVNODE_TYPE_USER:
        switch (IoControlCode) {
          case IOCTL_FUSE_GET_PCI_INFO:
            served = 1;
            status = FUSE_IoctlGetPciInfo(request, xdma, &resp_size);
            WdfRequestCompleteWithInformation(request, status, resp_size);
            break;
          
          case IOCTL_FUSE_GET_REVISION_ID:
            served = 1;
            status = FUSE_IoctlGetRevisionID(request, xdma, &resp_size);
            WdfRequestCompleteWithInformation(request, status, resp_size);
            break;
          case IOCTL_FUSE_GET_SUBSYSTEM_ID:
            served = 1;
            status = FUSE_IoctlGetSubSystemID(request, xdma, &resp_size);
            WdfRequestCompleteWithInformation(request, status, resp_size);
            break;

          // Optical link services:
          case IOCTL_FUSE_OLINK_GET_SIGNATURE:
            served = 1;
            status = FUSE_IoctlIpGetSignature(request, xdma, OLINK_DEV_ID, &resp_size);
            WdfRequestCompleteWithInformation(request, status, resp_size);
            break;
          case IOCTL_FUSE_OLINK_RESET_HW:
            served = 1;
            status = FUSE_IoctlIpResetHw(request, xdma, OLINK_DEV_ID);
            WdfRequestComplete(request, status);
            break;
          case IOCTL_FUSE_OLINK_ENABLE:
            served = 1;
            status = FUSE_IoctlIpEnable(request, xdma, OLINK_DEV_ID);
            WdfRequestComplete(request, status);
            break;
          case IOCTL_FUSE_OLINK_ENABLE_EVENTS:
            served = 1;
            status = FUSE_IoctlIpEnableEvents(request, xdma, OLINK_DEV_ID);
            WdfRequestComplete(request, status);
            break;
          case IOCTL_FUSE_OLINK_GET_CONN_STATUS:
            served = 1;
            status = FUSE_IoctlOlinkGetConnStatus(request, xdma, &resp_size);
            WdfRequestCompleteWithInformation(request, status, resp_size);
            break;
          case IOCTL_FUSE_OLINK_I2C_SEND_BYTE:
            served = 1;
            status = FUSE_IoctlI2cSendByte(request, xdma, &resp_size);
            WdfRequestCompleteWithInformation(request, status, resp_size);
            break;
          case IOCTL_FUSE_OLINK_I2C_RECV_BYTE:
            served = 1;
            status = FUSE_IoctlI2cRecvByte(request, xdma, &resp_size);
            WdfRequestCompleteWithInformation(request, status, resp_size);
            break;
          case IOCTL_FUSE_OLINK_I2C_RST_BRD_MUX:
            served = 1;
            status = FUSE_IoctlI2cRstBrdMux(request, xdma);
            WdfRequestComplete(request, STATUS_SUCCESS);
            break;

          // FUSE DMA services:
          case IOCTL_FUSE_DMA_GET_SIGNATURE:
            served = 1;
            status = FUSE_IoctlIpGetSignature(request, xdma, FUSE_DMA_DEV_ID, &resp_size);
            WdfRequestCompleteWithInformation(request, status, resp_size);
            break;
          case IOCTL_FUSE_DMA_RESET_HW:
            served = 1;
            //status = FUSE_IoctlFuseDMAResetHw(request, xdma);
            status = FUSE_IoctlIpResetHw(request, xdma, FUSE_DMA_DEV_ID);
            WdfRequestComplete(request, status);
            break;
          case IOCTL_FUSE_DMA_ENABLE:
            served = 1;
#if 0
            status = FUSE_IoctlFuseDmaRxEnable(request, xdma);
#else
            status = FUSE_IoctlIpEnable(request, xdma, FUSE_DMA_DEV_ID);
#endif
            WdfRequestComplete(request, status);
            break;
          case IOCTL_FUSE_DMA_ENABLE_EVENTS:
            served = 1;
            //status = FUSE_IoctlDmaEnableEvents(request, xdma);
            status = FUSE_IoctlIpEnableEvents(request, xdma, FUSE_DMA_DEV_ID);
            WdfRequestComplete(request, STATUS_SUCCESS);
            break;
          case IOCTL_FUSE_DMA_FLUSH:
            served = 1;
            status = FUSE_IoctlFuseDmaFlush(request, xdma);
            WdfRequestComplete(request, status);
            break;
          case IOCTL_FUSE_DMA_GET_FILLING_LEVEL:
            served = 1;
            status = FUSE_IoctlFuseDmaGetFillingLevel(request, xdma, &resp_size);
            WdfRequestCompleteWithInformation(request, status, resp_size);
            break;
          case IOCTL_FUSE_DMA_GET_FILLING_THRS:
            served = 1;
            status = FUSE_IoctlFuseDmaGetThrs(request, xdma, &resp_size);
            WdfRequestCompleteWithInformation(request, status, resp_size);
            break;
          case IOCTL_FUSE_DMA_SET_FILLING_THRS:
            served = 1;
            status = FUSE_IoctlFuseDmaSetThrs(request, xdma);
            WdfRequestComplete(request, status);
            break;
          case IOCTL_FUSE_DMA_GET_IPERF_DATA:
            served = 1;
            status = FUSE_IoctlFuseDmaGetIperfData(request, xdma, &resp_size);
            WdfRequestCompleteWithInformation(request, status, resp_size);
            break;
          case IOCTL_FUSE_DMA_GET_IMAGE_ID_SEED:
            served = 1;
            status = FUSE_IoctlFuseDmaGetImgId(request, xdma, &resp_size);
            WdfRequestCompleteWithInformation(request, status, resp_size);
            break;
          case IOCTL_FUSE_DMA_SET_IMAGE_ID_SEED:
            served = 1;
            status = FUSE_IoctlFuseDmaSetImgId(request, xdma);
            WdfRequestComplete(request, status);
            break;
          case IOCTL_FUSE_DMA_GET_MEM_SIZE:
            served = 1;
            status = FUSE_IoctlFuseDmaGetMemSize(request, xdma, &resp_size);
            WdfRequestCompleteWithInformation(request, status, resp_size);
            break;
          case IOCTL_FUSE_DMA_DNA_OK:
            served = 1;
            status = FUSE_IoctlFuseDmaDnaOK(request, xdma, &resp_size);
            WdfRequestCompleteWithInformation(request, status, resp_size);
            break;
            
          case IOCTL_FUSE_EMUL_GET_SIGNATURE:
            served = 1;
            if (xdma->FUSE_has_emul_module) {
              status = FUSE_IoctlIpGetSignature(request, xdma, EMUL_DEV_ID, &resp_size);
              WdfRequestCompleteWithInformation(request, status, resp_size);
            }
            else {
              WdfRequestComplete(request, STATUS_NOT_SUPPORTED);
            }
            break;
          case IOCTL_FUSE_EMUL_RESET_HW:
            served = 1;
            if (xdma->FUSE_has_emul_module) {
              status = FUSE_IoctlIpResetHw(request, xdma, EMUL_DEV_ID);
              WdfRequestComplete(request, status);
            }
            else {
              WdfRequestComplete(request, STATUS_NOT_SUPPORTED);
            }
            break;
          case IOCTL_FUSE_EMUL_ENABLE:
            served = 1;
            if (xdma->FUSE_has_emul_module) {
              status = FUSE_IoctlIpEnable(request, xdma, EMUL_DEV_ID);
              WdfRequestComplete(request, status);
            }
            else {
              WdfRequestComplete(request, STATUS_NOT_SUPPORTED);
            }
            break;
          case IOCTL_FUSE_EMUL_ENABLE_EVENTS:
            served = 1;
            if (xdma->FUSE_has_emul_module) {
              status = FUSE_IoctlIpEnableEvents(request, xdma, EMUL_DEV_ID);
              WdfRequestComplete(request, status);
            }
            else {
              WdfRequestComplete(request, STATUS_NOT_SUPPORTED);
            }
            break;
          case IOCTL_FUSE_EMUL_GENERATE:
            served = 1;
            if (xdma->FUSE_has_emul_module) {
              status = FUSE_IoctlEmulStart(request, xdma);
              WdfRequestComplete(request, status);
            }
            else {
              WdfRequestComplete(request, STATUS_NOT_SUPPORTED);
            }
            break;
          case IOCTL_FUSE_EMUL_STOP_CONT:
            served = 1;
            if (xdma->FUSE_has_emul_module) {
              status = FUSE_IoctlEmulStopCont(request, xdma);
              WdfRequestComplete(request, status);
            }
            else {
              WdfRequestComplete(request, STATUS_NOT_SUPPORTED);
            }
            break;
          default:
            served = 0;
            break;
        }
        break;
      case DEVNODE_TYPE_H2C:
        break;
      case DEVNODE_TYPE_C2H:
        switch (IoControlCode) {
          case IOCTL_FUSE_GET_FILE_TIMEOUT:
            served = 1;
            status = FUSE_IoctlGetDmaTimeout(request, file, &resp_size);
            WdfRequestCompleteWithInformation(request, status, resp_size);
            break;
          case IOCTL_FUSE_SET_FILE_TIMEOUT:
            served = 1;
            status = FUSE_IoctlSetDmaTimeout(request, file);
            WdfRequestComplete(request, status);
            break;
          default:
            served = 0;
            break;
        }
        break;
      default:
        break;
    }
  }

  return served;
}


#if 0
static NTSTATUS FUSE_IoctlFuseDmaGetSignature(IN WDFREQUEST request, IN XDMA_DEVICE *xdma, OUT size_t *resp_size) {
  ASSERT(xdma != NULL);
  // get handle to the IO request memory which will hold the read data
  WDFMEMORY requestMemory;
  NTSTATUS status = WdfRequestRetrieveOutputMemory(request, &requestMemory);
  if (!NT_SUCCESS(status)) {
    TraceError(DBG_IO, "WdfRequestRetrieveOutputMemory failed: %!STATUS!", status);
    return status;
  }

  UINT32 signature;
  fuse_dma_get_version(&(xdma->fuse_dma_drv), &signature);
  KdPrint(("[%s] signature = 0x%8.8X\n", __FUNCTION__, signature));

  status = WdfMemoryCopyFromBuffer(requestMemory, 0, &signature, sizeof(UINT32));
  if (!NT_SUCCESS(status)) {
    TraceError(DBG_IO, "WdfMemoryCopyFromBuffer failed: %!STATUS!", status);
    return status;
  }
  *resp_size = sizeof(UINT32);
  dbg_printf(("[%s] - \n", __FUNCTION__));
  return status;
}

static NTSTATUS FUSE_IoctlResetDmaHw(IN WDFREQUEST request, IN XDMA_DEVICE *xdma) {
  KdPrint(("[%s] + \n", __FUNCTION__));
  // get handle to the IO request memory which will hold the read data
  WDFMEMORY requestMemory;
  NTSTATUS status = WdfRequestRetrieveInputMemory(request, &requestMemory);
  if (!NT_SUCCESS(status)) {
    TraceError(DBG_IO, "WdfRequestRetrieveInputMemory failed: %!STATUS!", status);
    KdPrint(("WdfRequestRetrieveInputMemory failed: = %d\n", status));
    return status;
  }
  UINT32 mode;

  // copy from request memory into mask;
  status = WdfMemoryCopyToBuffer(requestMemory, 0, &mode, sizeof(UINT32));
  if (!NT_SUCCESS(status)) {
    TraceError(DBG_IO, "WdfMemoryCopyToBuffer failed: %!STATUS!", status);
    KdPrint(("WdfMemoryCopyToBuffer failed: = %d\n", status));
    return status;
  }

  KdPrint(("[%s] mode = 0x%8.8X\n", __FUNCTION__, mode));

  if (mode & _FUSE_DMA_RST_MEM) {
    fuse_dma_set_mem_reset(&(xdma->fuse_dma_drv), 1);
    fuse_kernel_delay_us(1);
    fuse_dma_set_mem_reset(&(xdma->fuse_dma_drv), 0);
    fuse_kernel_delay_us(1);
  }

  if (mode & _FUSE_DMA_RST_CORE) {
    fuse_dma_set_sw_reset(&(xdma->fuse_dma_drv), 1);
    fuse_kernel_delay_us(1);
    fuse_dma_set_sw_reset(&(xdma->fuse_dma_drv), 0);
    fuse_kernel_delay_us(1);
  }

//  fuse_olink_set_int_enable(&(xdma->olink_drv), mode);

  KdPrint(("[%s] - \n", __FUNCTION__));
  return status;
}

static NTSTATUS FUSE_IoctlEmulResetHw(IN WDFREQUEST request, IN XDMA_DEVICE *xdma) {
  KdPrint(("[%s] + \n", __FUNCTION__));
  // get handle to the IO request memory which will hold the read data
  WDFMEMORY requestMemory;
  NTSTATUS status = WdfRequestRetrieveInputMemory(request, &requestMemory);
  if (!NT_SUCCESS(status)) {
    TraceError(DBG_IO, "WdfRequestRetrieveInputMemory failed: %!STATUS!", status);
    KdPrint(("WdfRequestRetrieveInputMemory failed: = %d\n", status));
    return status;
  }
  UINT32 mode;

  // copy from request memory into mask;
  status = WdfMemoryCopyToBuffer(requestMemory, 0, &mode, sizeof(UINT32));
  if (!NT_SUCCESS(status)) {
    TraceError(DBG_IO, "WdfMemoryCopyToBuffer failed: %!STATUS!", status);
    KdPrint(("WdfMemoryCopyToBuffer failed: = %d\n", status));
    return status;
  }

  KdPrint(("[%s] mode = 0x%8.8X\n", __FUNCTION__, mode));
  fuse_emul_set_sw_reset(&(xdma->emul_drv), 1);
  fuse_kernel_delay_us(1);
  fuse_emul_set_sw_reset(&(xdma->emul_drv), 0);
  fuse_kernel_delay_us(1);
  KdPrint(("[%s] - \n", __FUNCTION__));
  return status;
}

#endif


#if 0
static NTSTATUS FUSE_IoctlOlinkEnableEvents(IN WDFREQUEST request, IN XDMA_DEVICE *xdma) {
  KdPrint(("[%s] + \n", __FUNCTION__));
  // get handle to the IO request memory which will hold the read data
  WDFMEMORY requestMemory;
  NTSTATUS status = WdfRequestRetrieveInputMemory(request, &requestMemory);
  if (!NT_SUCCESS(status)) {
    TraceError(DBG_IO, "WdfRequestRetrieveInputMemory failed: %!STATUS!", status);
    KdPrint(("WdfRequestRetrieveInputMemory failed: = %d\n", status));
    return status;
  }
  UINT32 mask;

  // copy from request memory into mask;
  status = WdfMemoryCopyToBuffer(requestMemory, 0, &mask, sizeof(UINT32));
  if (!NT_SUCCESS(status)) {
    TraceError(DBG_IO, "WdfMemoryCopyToBuffer failed: %!STATUS!", status);
    KdPrint(("WdfMemoryCopyToBuffer failed: = %d\n", status));
    return status;
  }

  KdPrint(("[%s] mask = 0x%8.8X\n", __FUNCTION__, mask));
  fuse_olink_set_int_enable(&(xdma->olink_drv), mask);

  KdPrint(("[%s] - \n", __FUNCTION__));
  return status;
}

static NTSTATUS FUSE_IoctlDmaEnableEvents(IN WDFREQUEST request, IN XDMA_DEVICE *xdma) {
  KdPrint(("[%s] + \n", __FUNCTION__));
  // get handle to the IO request memory which will hold the read data
  WDFMEMORY requestMemory;
  NTSTATUS status = WdfRequestRetrieveInputMemory(request, &requestMemory);
  if (!NT_SUCCESS(status)) {
    TraceError(DBG_IO, "WdfRequestRetrieveInputMemory failed: %!STATUS!", status);
    KdPrint(("WdfRequestRetrieveInputMemory failed: = %d\n", status));
    return status;
  }
  UINT32 mask;

  // copy from request memory into mask;
  status = WdfMemoryCopyToBuffer(requestMemory, 0, &mask, sizeof(UINT32));
  if (!NT_SUCCESS(status)) {
    TraceError(DBG_IO, "WdfMemoryCopyToBuffer failed: %!STATUS!", status);
    KdPrint(("WdfMemoryCopyToBuffer failed: = %d\n", status));
    return status;
  }

  KdPrint(("[%s] mask = 0x%8.8X\n", __FUNCTION__, mask));
  fuse_dma_set_int_enable(&(xdma->fuse_dma_drv), mask);

  KdPrint(("[%s] - \n", __FUNCTION__));
  return status;
}

static NTSTATUS FUSE_IoctlEmulEnableEvents(IN WDFREQUEST request, IN XDMA_DEVICE *xdma) {
  KdPrint(("[%s] + \n", __FUNCTION__));
  // get handle to the IO request memory which will hold the read data
  WDFMEMORY requestMemory;
  NTSTATUS status = WdfRequestRetrieveInputMemory(request, &requestMemory);
  if (!NT_SUCCESS(status)) {
    TraceError(DBG_IO, "WdfRequestRetrieveInputMemory failed: %!STATUS!", status);
    KdPrint(("WdfRequestRetrieveInputMemory failed: = %d\n", status));
    return status;
  }
  UINT32 mask;

  // copy from request memory into mask;
  status = WdfMemoryCopyToBuffer(requestMemory, 0, &mask, sizeof(UINT32));
  if (!NT_SUCCESS(status)) {
    TraceError(DBG_IO, "WdfMemoryCopyToBuffer failed: %!STATUS!", status);
    KdPrint(("WdfMemoryCopyToBuffer failed: = %d\n", status));
    return status;
  }

  KdPrint(("[%s] mask = 0x%8.8X\n", __FUNCTION__, mask));
  //UNREFERENCED_PARAMETER(xdma);
  fuse_emul_set_int_enable(&(xdma->emul_drv), mask);

  KdPrint(("[%s] - \n", __FUNCTION__));
  return status;
}
#endif

static NTSTATUS FUSE_IoctlCancelReadUserEvent(IN WDFREQUEST request, IN FILE_CONTEXT *file) {
  _dbg_note("[%s] + \n", __FUNCTION__);
  //UNREFERENCED_PARAMETER(request);
  // get handle to the IO request memory which will hold the read data
  WDFMEMORY requestMemory;
  NTSTATUS status = WdfRequestRetrieveInputMemory(request, &requestMemory);
  if (!NT_SUCCESS(status)) {
    TraceError(DBG_IO, "WdfRequestRetrieveInputMemory failed: %!STATUS!", status);
    _dbg_error("WdfRequestRetrieveInputMemory failed: = %d\n", status);
    return status;
  }
  UINT32 mode;

  // copy from request memory into mask;
  status = WdfMemoryCopyToBuffer(requestMemory, 0, &mode, sizeof(UINT32));
  if (!NT_SUCCESS(status)) {
    TraceError(DBG_IO, "WdfMemoryCopyToBuffer failed: %!STATUS!", status);
    _dbg_error("[%s] WdfMemoryCopyToBuffer failed: = %d\n", __FUNCTION__, status);
    return status;
  }

//  KdPrint(("[%s] mode = 0x%8.8X\n", __FUNCTION__, mode));

#if 1
  FUSE_XDMA_EVENT *fuseEvent = file->u.fuseEvent;
  KEVENT *event = (KEVENT *)fuseEvent->syncObj;

#if DBG
  LONG state = KeReadStateEvent(event);
  KdPrint(("[%s] (evento %d) Cancelando, event state = %d\n", __FUNCTION__, fuseEvent->event_ID, state));
#endif

//  if (state)
  {
    InterlockedExchange(&fuseEvent->cancel, 1);
    fuseEvent->flags = 0;
    KePulseEvent(event, IO_NO_INCREMENT, FALSE);
  }
#else
  UNREFERENCED_PARAMETER(file);
#endif

  _dbg_note(("[%s] - \n", __FUNCTION__));
  return status;
}
