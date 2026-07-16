
// ========================= include dependencies =================================================

#include "driver.h"
#include "file_io.h"
#include "trace.h"

// RMG for accessing to PCI conf area:
#include <wdmguid.h> // required for WMILIB_CONTEXT


#ifdef DBG
// The trace message header (.tmh) file must be included in a source file before any WPP macro 
// calls and after defining a WPP_CONTROL_GUIDS macro (defined in trace.h). see trace.h
#include "driver.tmh"
#endif

#ifdef FUSE_VERBOSE
  #define dbg_printf(x)   KdPrint(x)
#else
  #define dbg_printf(x)
#endif

// ========================= declarations ================================================= 

// RMG: Anotaciones para SDV (Static Driver Verifier)

DRIVER_INITIALIZE DriverEntry;
DRIVER_UNLOAD DriverUnload;

EVT_WDF_DRIVER_DEVICE_ADD           EvtDeviceAdd;
EVT_WDF_DEVICE_CONTEXT_CLEANUP      EvtDeviceCleanup;
EVT_WDF_DEVICE_PREPARE_HARDWARE     EvtDevicePrepareHardware;
EVT_WDF_DEVICE_RELEASE_HARDWARE     EvtDeviceReleaseHardware;

static NTSTATUS EngineCreateQueue(WDFDEVICE device, XDMA_ENGINE* engine, WDFQUEUE* queue);

// Mark these functions as pageable code
#ifdef ALLOC_PRAGMA
#pragma alloc_text (INIT, DriverEntry)
#pragma alloc_text (PAGE, DriverUnload)
#pragma alloc_text (PAGE, EvtDeviceAdd)
#pragma alloc_text (PAGE, EvtDevicePrepareHardware)
#pragma alloc_text (PAGE, EvtDeviceReleaseHardware)
#pragma alloc_text (PAGE, EngineCreateQueue)
#endif

// ========================= definitions =================================================

const char * const dateTimeStr = "Built " __DATE__ ", " __TIME__ ".";

// RMG: Esta es una función de soporte para obtener el valor del parámetro
// POLL_MODE desde el registro de Windows.

// NOTES:
// Drivers typically use a set of system-defined registry keys to store or 
// access driver-specific or device-specific information. Your driver might 
// access the following registry keys:
// * Parameters key
// The driver's Parameters key can contain configuration information for 
// your driver. For Kernel-Mode Driver Framework (KMDF) drivers, this key 
// is located in the HKLM\SYSTEM\CurrentControlSet\Services tree, under the 
// driver's service name.
//  HKEY_LOCAL_MACHINE\SYSTEM\CurrentControlSet\Services\XDMA\Parameters
// => https://learn.microsoft.com/en-us/windows-hardware/drivers/install/hklm-system-currentcontrolset-services-registry-tree

// Hay una entrada en el fichero .inx para definir la clave del registro:
//    [XDMA_Inst.NT.Services.AddReg]
//    HKR,Parameters,"POLL_MODE",0x00010001,0


// Get the driver parameter for POLL_MODE from the Windows registry 
static NTSTATUS GetPollModeParameter(IN PULONG pollMode) {
  WDFDRIVER driver = WdfGetDriver();
  WDFKEY key;
  NTSTATUS status;
  
  status = WdfDriverOpenParametersRegistryKey(
              driver, 
              STANDARD_RIGHTS_ALL,
              WDF_NO_OBJECT_ATTRIBUTES, 
              &key);
  
  if (!NT_SUCCESS(status)) {
    TraceError(DBG_INIT, "WdfDriverOpenParametersRegistryKey failed: %!STATUS!", status);
    WdfRegistryClose(key);
    return status;
  }
  
  // La clave del registro que buscamos es: POLL_MODE
  DECLARE_CONST_UNICODE_STRING(valueName, L"POLL_MODE");
  
  status = WdfRegistryQueryULong(key, &valueName, pollMode);
  if (!NT_SUCCESS(status)) {
    TraceError(DBG_INIT, "WdfRegistryQueryULong failed: %!STATUS!", status);
    WdfRegistryClose(key);
    return status;
  }
  
  ULONG tracepollmode;
  tracepollmode = *pollMode;
  //dbg_printf(("RMG: [%s] pollMode = %u\n", __FUNCTION__, tracepollmode));
  TraceVerbose(DBG_INIT, "pollMode=%u", tracepollmode);
  
  WdfRegistryClose(key);
  return status;
}

// RMG: punto de entrada del driver
// main entry point - Called when driver is installed
NTSTATUS DriverEntry(IN PDRIVER_OBJECT driverObject, IN PUNICODE_STRING registryPath) {
  NTSTATUS status = STATUS_SUCCESS;
  WDF_DRIVER_CONFIG DriverConfig;
  WDFDRIVER Driver;


    
  // Initialize WPP Tracing
  WPP_INIT_TRACING(driverObject, registryPath);
  TraceInfo(DBG_INIT, "XDMA Driver - %s", dateTimeStr);

  DbgPrint("RMG: instalando driver !!!!!!\n");

  KdPrint(("RMG: instalando driver\n"));
  TraceVerbose(DBG_INIT, "RMG: [%s] instalando driver!!!", __FUNCTION__);
  
  // Initialize the Driver Config; register the device add event callback
  // EvtDeviceAdd() will be called when a device is found
  WDF_DRIVER_CONFIG_INIT(&DriverConfig, EvtDeviceAdd);

  /*
  isWheaEnabled = PshedIsSystemWheaEnabled();
  if (isWheaEnabled == FALSE)
  {
    TraceInfo(DBG_INIT, "PSHED Enabled: %!STATUS!", status);
    status = STATUS_SUCCESS;
  }
  else {
    TraceInfo(DBG_INIT, "PSHED failed: %!STATUS!", status);
  }
  */


  // Creates a WDFDRIVER object, the top of our device's tree of objects
  status = WdfDriverCreate(driverObject, 
                           registryPath, 
                           WDF_NO_OBJECT_ATTRIBUTES, 
                           &DriverConfig,
                           &Driver);

  if (!NT_SUCCESS(status)) {
    TraceError(DBG_INIT, "WdfDriverCreate failed: %!STATUS!", status);
    WPP_CLEANUP(driverObject);
    return status;
  }

  driverObject->DriverUnload = DriverUnload;
  return status;
}

// RMG: PAGED_CODE() ensures that the calling thread is running at an IRQL 
// that is low enough to permit paging

// RMG: punto de salida del driver
// Called before the driver is removed
VOID DriverUnload(IN PDRIVER_OBJECT driverObject) {
  PAGED_CODE();
  UNREFERENCED_PARAMETER(driverObject);

  KdPrint(("RMG: desinstalando driver!!!\n"));
  TraceVerbose(DBG_INIT, "RMG: [%s] desinstalando driver!!!", __FUNCTION__);

  TraceVerbose(DBG_INIT, "%!FUNC!");

  WPP_CLEANUP(driverObject); // cleanup tracing

  return;
}

NTSTATUS EvtDeviceAdd(IN WDFDRIVER Driver, IN PWDFDEVICE_INIT DeviceInit) {
  NTSTATUS status = STATUS_SUCCESS;
  
  PAGED_CODE();
  
  TraceVerbose(DBG_INIT, "(Driver=0x%p)", Driver);

  //  We prefer Direct I/O
  //  Direct I/O only works with deferred buffer retrieval No guarantee that Direct I/O is
  //  actually used Direct I/O is only used for buffers that are full pages Buffered I/O is used
  //  for other parts of the transfer  
  WdfDeviceInitSetIoType(DeviceInit, WdfDeviceIoDirect);
  
  // Set call-backs for any of the functions we are interested in. If no call-back is set, the 
  // framework will take the default action by itself.
  WDF_PNPPOWER_EVENT_CALLBACKS PnpPowerCallbacks;
  WDF_PNPPOWER_EVENT_CALLBACKS_INIT(&PnpPowerCallbacks);
  PnpPowerCallbacks.EvtDevicePrepareHardware = EvtDevicePrepareHardware;  // ok
  PnpPowerCallbacks.EvtDeviceReleaseHardware = EvtDeviceReleaseHardware;  // ok
  WdfDeviceInitSetPnpPowerEventCallbacks(DeviceInit, &PnpPowerCallbacks);
  
  WDF_POWER_POLICY_EVENT_CALLBACKS powerPolicyCallbacks;
  WDF_POWER_POLICY_EVENT_CALLBACKS_INIT(&powerPolicyCallbacks);
  WdfDeviceInitSetPowerPolicyEventCallbacks(DeviceInit, &powerPolicyCallbacks);
  
  // Register file object call-backs
  WDF_OBJECT_ATTRIBUTES fileAttributes;
  WDF_FILEOBJECT_CONFIG fileConfig;
  WDF_FILEOBJECT_CONFIG_INIT(&fileConfig, EvtDeviceFileCreate, EvtFileClose, EvtFileCleanup);
  WDF_OBJECT_ATTRIBUTES_INIT(&fileAttributes);
  
  fileAttributes.SynchronizationScope = WdfSynchronizationScopeNone;
  WDF_OBJECT_ATTRIBUTES_SET_CONTEXT_TYPE(&fileAttributes, FILE_CONTEXT);
  WdfDeviceInitSetFileObjectConfig(DeviceInit, &fileConfig, &fileAttributes);

  // NOTAS:
  // It is possible for a driver to intercept an I/O request before the 
  // framework places the request in an I/O queue. To intercept I/O requests,
  // the driver must call WdfDeviceInitSetIoInCallerContextCallback to 
  // register an EvtIoInCallerContext callback function.
  WdfDeviceInitSetIoInCallerContextCallback(DeviceInit, EvtDeviceIoInCallerContext);
  
  // Specify the context type and size for the device we are about to create.
  WDF_OBJECT_ATTRIBUTES deviceAttributes;
  WDF_OBJECT_ATTRIBUTES_INIT_CONTEXT_TYPE(&deviceAttributes, DeviceContext);
  
  // ContextCleanup will be called by the framework when it deletes the device. So you can defer
  // freeing any resources allocated to Cleanup callback in the event EvtDeviceAdd returns any 
  // error after the device is created.
  deviceAttributes.EvtCleanupCallback = EvtDeviceCleanup;
  WDFDEVICE device;
  
  status = WdfDeviceCreate(&DeviceInit, &deviceAttributes, &device);
  if (!NT_SUCCESS(status)) {
    TraceError(DBG_INIT, "WdfDeviceCreate failed: %!STATUS!", status);
    return status;
  }
  
  // Create a user-space device interface
  status = WdfDeviceCreateDeviceInterface(device, (LPGUID)&GUID_DEVINTERFACE_XDMA, NULL);
  if (!NT_SUCCESS(status)) {
    TraceError(DBG_INIT, "WdfDeviceCreateDeviceInterface failed %!STATUS!", status);
    return status;
  }
  
  //----
  
  // Creación de la cola de I/O para este dispositivo con:
  //  - atributos: sin ningún tipo de sincronización.
  //  - configuración: Despacho paralelo
  
  WDF_OBJECT_ATTRIBUTES attribs;
  WDF_OBJECT_ATTRIBUTES_INIT(&attribs);
  attribs.SynchronizationScope = WdfSynchronizationScopeNone;
  WDF_OBJECT_ATTRIBUTES_SET_CONTEXT_TYPE(&attribs, QUEUE_CONTEXT);
  
  // create the default queue upon all I/O requests arrive
  // accept multiple I/O request to run in parallel, they are sequentialized later
  WDF_IO_QUEUE_CONFIG queueConfig;
  WDF_IO_QUEUE_CONFIG_INIT_DEFAULT_QUEUE(&queueConfig, WdfIoQueueDispatchParallel);
  queueConfig.EvtIoDeviceControl = EvtIoDeviceControl; // callback handler for control requests
  queueConfig.EvtIoRead = EvtIoRead; // callback handler for read requests
  queueConfig.EvtIoWrite = EvtIoWrite; // callback handler for write requests
  WDFQUEUE entryQueue;
  status = WdfIoQueueCreate(device, &queueConfig, &attribs, &entryQueue);
  if (!NT_SUCCESS(status)) {
    TraceError(DBG_INIT, "WdfIoQueueCreate failed: %!STATUS!", status);
    return status;
  }
  
  TraceVerbose(DBG_INIT, "returns %!STATUS!", status);
  return status;
}

// Any device specific cleanup  - TODO device reset?
VOID EvtDeviceCleanup(IN WDFOBJECT device) {
  UNREFERENCED_PARAMETER(device);
  TraceInfo(DBG_INIT, "%!FUNC!");
}


// RMG: callback de EvtDevicePrepareHardware()
// Initialize device hardware and host buffers: Called by plug and play manager
NTSTATUS EvtDevicePrepareHardware(
  IN WDFDEVICE device, 
  IN WDFCMRESLIST Resources,
  IN WDFCMRESLIST ResourcesTranslated) 
{
  PAGED_CODE();
  UNREFERENCED_PARAMETER(Resources);
  
  dbg_printf(("RMG: [%s]+\n", __FUNCTION__));
  TraceVerbose(DBG_INIT, "-->Entry");
  DeviceContext *ctx = GetDeviceContext(device);  // <= en driver.h
  PXDMA_DEVICE xdma = &(ctx->xdma);
//  ULONG userMax = XDMA_MAX_USER_IRQ; // 0;
//  ULONG h2cChannelMax = XDMA_MAX_NUM_CHANNELS; // 0;
//  ULONG c2hChannelMax = XDMA_MAX_NUM_CHANNELS; // 0;
  ULONG userMax = 3;
  ULONG h2cChannelMax = 1;
  ULONG c2hChannelMax = 1;
  
  NTSTATUS status;
  status = XDMA_DeviceOpen( device, 
                            xdma, 
                            &userMax, 
                            &h2cChannelMax, 
                            &c2hChannelMax,
                            Resources, 
                            ResourcesTranslated);
  if (!NT_SUCCESS(status)) {
    dbg_printf(("RMG: [%s] XDMA_DeviceOpen failed = %d\n", __FUNCTION__, status));
    TraceError(DBG_INIT, "XDMA_DeviceOpen failed: %!STATUS!", status);
    return status;
  }
  
  // get poll mode parameter and configure engines as poll mode if needed
  ULONG pollMode = 0;
  status = GetPollModeParameter(&pollMode);
  if (!NT_SUCCESS(status)) {
    TraceError(DBG_INIT, "GetPollModeParameter failed: %!STATUS!", status);
    return status;
  }
  
  for (UINT dir = H2C; dir < 2; dir++) { // 0=H2C, 1=C2H
    for (ULONG ch = 0; ch < XDMA_MAX_NUM_CHANNELS; ch++) {
      XDMA_ENGINE* engine = &(xdma->engines[ch][dir]);
      XDMA_EngineSetPollMode(engine, (BOOLEAN)pollMode);
    }
  }
  
  // create a queue for each engine
  for (UINT dir = H2C; dir < 2; dir++) { // 0=H2C, 1=C2H
    for (ULONG ch = 0; ch < XDMA_MAX_NUM_CHANNELS; ch++) {
      XDMA_ENGINE* engine = &(xdma->engines[ch][dir]);
      if (engine->enabled == TRUE) {
        status = EngineCreateQueue(device, engine, &(ctx->engineQueue[dir][ch]));
        if (!NT_SUCCESS(status)) {
          TraceError(DBG_INIT, "EngineCreateQueue() failed: %!STATUS!", status);
          return status;
        }
      }
    }
  }
  
  // RMG: Para los KEVENT que tenemos en 'DeviceContext' los inicializamos y
  // registramos su handler
  for (UINT i = 0; i < XDMA_MAX_USER_IRQ; ++i) {
    // Inicializamos el objeto evento como tipo 'notification' y lo fijamos a
    // not-signaled state
#ifdef USE_FUSE_INTS
    KeInitializeEvent(&ctx->eventSignals[i], NotificationEvent, FALSE);
    // Fijamos el handler de usuario para ese evento (interrupt.c)
    FUSE_UserIsrRegister(xdma, i, FUSE_HandleUserEvent, &ctx->eventSignals[i]);
#else 
    KeInitializeEvent(&ctx->eventSignals[i], NotificationEvent, FALSE);
    // Fijamos el handler de usuario para ese evento (interrupt.c)
    XDMA_UserIsrRegister(xdma, i, HandleUserEvent, &ctx->eventSignals[i]);
#endif
  }

 // wdmguid.h

  // RMG:
  if(0)
  {
    NTSTATUS ret;
    BUS_INTERFACE_STANDARD  BusInterface;
    //UCHAR   RevisionID;
    //USHORT  SubVendorID;
    //USHORT  SubSystemID;

    KdPrint(("[%s] LELLENDO CONFIGURACIÓN PCI\n", __FUNCTION__));


#define PCI_HDR_SIZE  0x40

    // Get the BUS_INTERFACE_STANDARD for our device so that we can
    // read & write to PCI config space.
    ret = WdfFdoQueryForInterface(
                device,
                &GUID_BUS_INTERFACE_STANDARD,
                (PINTERFACE)&BusInterface,
                sizeof(BUS_INTERFACE_STANDARD),
                1,                                // Version
                NULL);                            // InterfaceSpecificData
    if (NT_SUCCESS(ret)) {
      DECLSPEC_ALIGN(MEMORY_ALLOCATION_ALIGNMENT) UCHAR buffer[PCI_HDR_SIZE];
      PPCI_COMMON_CONFIG pPciConfig = (PPCI_COMMON_CONFIG)buffer;

      ULONG bytesRead = 0;


      RtlZeroMemory(buffer, sizeof(buffer));
      bytesRead = BusInterface.GetBusData(
                    BusInterface.Context,                       // Context
                    PCI_WHICHSPACE_CONFIG,                      // PCI configuration space
                    buffer,                                     // target buffer
                    FIELD_OFFSET(PCI_COMMON_CONFIG, VendorID),  // Offset
                    PCI_HDR_SIZE);                              // Length

      if (bytesRead == PCI_HDR_SIZE) {
        xdma->RevisionID  = pPciConfig->RevisionID;
        xdma->SubVendorID = pPciConfig->u.type0.SubVendorID;
        xdma->SubSystemID = pPciConfig->u.type0.SubSystemID;

        KdPrint(("[%s] RevisionID = 0x%2.2X\n", __FUNCTION__, xdma->RevisionID));
        KdPrint(("[%s] SubVendorID = 0x%4.4X\n", __FUNCTION__, xdma->SubVendorID));
        KdPrint(("[%s] SubSystemID = 0x%4.4X\n", __FUNCTION__, xdma->SubSystemID));
      }

    }
  }
  
  TraceVerbose(DBG_INIT, "<--Exit returning %!STATUS!", status);
  dbg_printf(("RMG: [%s]-\n", __FUNCTION__));
  return status;
}



// Unmap PCIe resources
NTSTATUS EvtDeviceReleaseHardware(IN WDFDEVICE Device, IN WDFCMRESLIST ResourcesTranslated) {
  PAGED_CODE();
  UNREFERENCED_PARAMETER(ResourcesTranslated);
  TraceVerbose(DBG_INIT, "DeviceReleaseHw");
  
  DeviceContext* ctx = GetDeviceContext(Device);
  if (ctx != NULL) {
    XDMA_DeviceClose(&ctx->xdma);
  }
  
  TraceVerbose(DBG_INIT, "exit");
  return STATUS_SUCCESS;
}

// Create a WDF IO queue for a DMA engine

NTSTATUS EngineCreateQueue(WDFDEVICE device, XDMA_ENGINE* engine, WDFQUEUE* queue) {
  NTSTATUS status = STATUS_SUCCESS;
  WDF_IO_QUEUE_CONFIG config;
  WDF_OBJECT_ATTRIBUTES attribs;
  PQUEUE_CONTEXT context;
  
  PAGED_CODE();
  
  // engine queue is sequential
  WDF_IO_QUEUE_CONFIG_INIT(&config, WdfIoQueueDispatchSequential);
  
  ASSERTMSG("direction is neither H2C nor C2H!", (engine->dir == C2H) || (engine->dir == H2C));
  if (engine->dir == H2C) { // callback handler for write requests
    config.EvtIoWrite = EvtIoWriteDma;
    TraceInfo(DBG_INIT, "EvtIoWrite=EvtIoWriteDma");
  } 
  else if (engine->dir == C2H) { // callback handler for read requests
    if (engine->type == EngineType_ST) {
#ifdef USE_FUSE_DMA
      // RMG FUSE:
      config.EvtIoRead = FUSE_EvtIoReadDma; 
      dbg_printf(("RMG: [%s] Using FUSE_EvtIoReadDma\n", __FUNCTION__));
      TraceInfo(DBG_INIT, "EvtIoRead=FUSE_EvtIoReadDma");
#else
      config.EvtIoRead = EvtIoReadEngineRing;
      TraceInfo(DBG_INIT, "EvtIoRead=EvtIoReadEngineRing");
#endif      
    } 
    else {
      config.EvtIoRead = EvtIoReadDma;
      TraceInfo(DBG_INIT, "EvtIoRead=EvtIoReadDma");
    }
  }
  
  // serialize all callbacks related to this queue. see ref [2]
  WDF_OBJECT_ATTRIBUTES_INIT(&attribs);
  attribs.SynchronizationScope = WdfSynchronizationScopeQueue;
  WDF_OBJECT_ATTRIBUTES_SET_CONTEXT_TYPE(&attribs, QUEUE_CONTEXT);
  status = WdfIoQueueCreate(device, &config, &attribs, queue);
  if (!NT_SUCCESS(status)) {
    TraceError(DBG_INIT, "WdfIoQueueCreate failed %d", status);
    return status;
  }
  
  // store arguments into queue context
  context = GetQueueContext(*queue);
  context->engine = engine;
  return status;
}


