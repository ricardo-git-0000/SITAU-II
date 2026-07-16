
// ====================== include dependancies ========================================================

#include <ntddk.h>
#include <initguid.h> // required for GUID definitions
#include <wdmguid.h> // required for WMILIB_CONTEXT

#include "device.h"
#include "interrupt.h"
#include "dma_engine.h"
#include "xdma_public.h"

#ifdef FUSE_VERBOSE
  #define dbg_printf(x)   KdPrint(x)
#else
  #define dbg_printf(x)
#endif

#include "trace.h"
#ifdef DBG
// The trace message header (.tmh) file must be included in a source file before any WPP macro 
// calls and after defining a WPP_CONTROL_GUIDS macro (defined in trace.h). see trace.h
#include "device.tmh"
#endif

// ====================== Function declarations ========================================================

// declare following functions as pageable code
#ifdef ALLOC_PRAGMA

#endif

// WDK 10 static code analysis feature expects to target Windows 10 and thus recommends not to
// use MmMapIoSpace and use MmMapIoSpaceEx instead. However this function is not available pre 
// Win 10. Thus disable this warning. 
// see https://social.msdn.microsoft.com/Forums/en-US/f8a3fb63-10de-481c-b629-8b5f3d254c5e/unexpected-code-analysis-behavior?forum=wdk
#pragma warning (disable : 30029) 

// ====================== constants ========================================================

// Version constants for the XMDA IP core
typedef enum XDMA_IP_VERSION_T {
  v2015_4 = 1,
  v2016_1 = 2,
  v2016_2 = 3,
  v2016_3 = 4,
  v2016_4 = 5,
  v2017_1 = 6,
  v2017_2 = 7,
  v2017_3 = 8
} XDMA_IP_VERSION;

// ====================== static functions ========================================================

// Get the XDMA IP core version
static XDMA_IP_VERSION GetVersion(IN OUT PXDMA_DEVICE xdma) {
  XDMA_IP_VERSION version = xdma->configRegs->identifier & 0x000000ffUL;
  TraceVerbose(DBG_INIT, "version is 0x%x", version);
  return version;
}

// Initialize the XDMA_DEVICE structure with default values
static void DeviceDefaultInitialize(IN PXDMA_DEVICE xdma) {
  ASSERT(xdma != NULL);

  // bars
  xdma->numBars = 0;
  for (UINT32 i = 0; i < XDMA_MAX_NUM_BARS; i++) {
    xdma->bar[i] = NULL;
    xdma->barLength[i] = 0;
  }
  xdma->configBarIdx = 0;
  xdma->userBarIdx = -1;
  xdma->bypassBarIdx = -1;

  // registers 
  xdma->configRegs = NULL;
  xdma->interruptRegs = NULL;
  xdma->sgdmaRegs = NULL;

  // engines
  for (UINT dir = H2C; dir < 2; dir++) { // 0=H2C, 1=C2H
    for (ULONG ch = 0; ch < XDMA_MAX_NUM_CHANNELS; ch++) {
      xdma->engines[ch][dir].enabled = FALSE;
      xdma->engines[ch][dir].poll = FALSE;
    }
  }

  // interrupts - nothing to do

  // user events
  for (int i = 0; i < XDMA_MAX_USER_IRQ; i++) {
    xdma->userEvents[i].work = NULL;
    xdma->userEvents[i].userData = NULL;
#ifdef USE_FUSE_INTS
    xdma->fuseUserEvents[i].event_ID = i;
    xdma->fuseUserEvents[i].userCB = NULL;
    xdma->fuseUserEvents[i].syncObj = NULL;
#endif
  }
}

// Iterate through PCIe resources and map BARS into host memory
static NTSTATUS MapBARs(IN PXDMA_DEVICE xdma, IN WDFCMRESLIST ResourcesTranslated) {

  const ULONG numResources = WdfCmResourceListGetCount(ResourcesTranslated);
  TraceVerbose(DBG_INIT, "# PCIe resources = %d", numResources);

  for (UINT i = 0; i < numResources; i++) {
    PCM_PARTIAL_RESOURCE_DESCRIPTOR resource = WdfCmResourceListGetDescriptor(ResourcesTranslated, i);
    if (!resource) {
      TraceError(DBG_INIT, "WdfResourceCmGetDescriptor() fails");
      return STATUS_DEVICE_CONFIGURATION_ERROR;
    }

    if (resource->Type == CmResourceTypeMemory) {
      xdma->barLength[xdma->numBars] = resource->u.Memory.Length;
      xdma->bar[xdma->numBars] = MmMapIoSpace(resource->u.Memory.Start,
                          resource->u.Memory.Length, MmNonCached);
      if (xdma->bar[xdma->numBars] == NULL) {
        TraceError(DBG_INIT, "MmMapIoSpace returned NULL! for BAR%u", xdma->numBars);
        return STATUS_DEVICE_CONFIGURATION_ERROR;
      }
      TraceInfo(DBG_INIT, "MM BAR %d (addr:0x%lld, length:%u) mapped at 0x%08p",
            xdma->numBars, resource->u.Memory.Start.QuadPart,
            resource->u.Memory.Length, xdma->bar[xdma->numBars]);
      xdma->numBars++;
    }
  }
  return STATUS_SUCCESS;
}

// Is the BAR at index 'idx' the config BAR?
static BOOLEAN IsConfigBAR(IN PXDMA_DEVICE xdma, IN UINT idx) {

  XDMA_IRQ_REGS* irqRegs = (XDMA_IRQ_REGS*)((PUCHAR)xdma->bar[idx] + IRQ_BLOCK_OFFSET);
  XDMA_CONFIG_REGS* cfgRegs = (XDMA_CONFIG_REGS*)((PUCHAR)xdma->bar[idx] + CONFIG_BLOCK_OFFSET);

  UINT32 interruptID = irqRegs->identifier & XDMA_ID_MASK;
  UINT32 configID = cfgRegs->identifier & XDMA_ID_MASK;

  return ((interruptID == XDMA_ID) && (configID == XDMA_ID)) ? TRUE : FALSE;
}

// Identify which BAR is the config BAR
static UINT FindConfigBAR(IN PXDMA_DEVICE xdma) {
  for (UINT i = 0; i < xdma->numBars; ++i) {
    if (IsConfigBAR(xdma, i)) {
      TraceInfo(DBG_INIT, "config BAR is %u", i);
      return i;
    }
  }
  return xdma->numBars; //not found - return past-the-end index
}

// Identify all BARs
static NTSTATUS IdentifyBars(IN PXDMA_DEVICE xdma) {

  // find DMA config BAR (usually BAR1, see section 'Target Bridge' in [1]) 
  xdma->configBarIdx = FindConfigBAR(xdma);
  if (xdma->configBarIdx == xdma->numBars) {
    TraceError(DBG_INIT, "findConfigBar() failed: bar is %d", xdma->configBarIdx);
    return STATUS_DRIVER_INTERNAL_ERROR;
  }
  // if config bar is bar0 then user bar doesnt exit
  xdma->userBarIdx = xdma->configBarIdx == 1 ? 0 : -1;

  // if config bar is not the last bar then bypass bar exists
  xdma->bypassBarIdx = xdma->numBars - xdma->configBarIdx == 2 ? xdma->numBars - 1 : -1;

  TraceInfo(DBG_INIT, "%!FUNC!, BAR index: user=%d, control=%d, bypass=%d",
        xdma->userBarIdx, xdma->configBarIdx, xdma->bypassBarIdx);
  return STATUS_SUCCESS;
}

// Get the config, interrupt and sgdma module register offsets
static void GetRegisterModules(IN PXDMA_DEVICE xdma) {
  PUCHAR configBarAddr = (PUCHAR)xdma->bar[xdma->configBarIdx];
  xdma->configRegs = (XDMA_CONFIG_REGS*)(configBarAddr + CONFIG_BLOCK_OFFSET);
  xdma->interruptRegs = (XDMA_IRQ_REGS*)(configBarAddr + IRQ_BLOCK_OFFSET);
  xdma->sgdmaRegs = (XDMA_SGDMA_COMMON_REGS*)(configBarAddr + SGDMA_COMMON_BLOCK_OFFSET);
}

// ====================== API functions ========================================

static NTSTATUS FUSE_get_PCI_conf_info(IN WDFDEVICE device, IN XDMA_DEVICE *xdma);
static NTSTATUS FUSE_set_version(IN XDMA_DEVICE *xdma);
static NTSTATUS FUSE_initialize_IPs(IN WDFDEVICE device, IN XDMA_DEVICE *xdma);


// Equivalente al xdma_device_open() de libxdma.c de la versión linux:
//  void *xdma_device_open(const char *mname, struct pci_dev *pdev, int *user_max,
//                         int *h2c_channel_max, int *c2h_channel_max)

NTSTATUS XDMA_DeviceOpen(WDFDEVICE wdfDevice,
             PXDMA_DEVICE xdma,
             ULONG *userMax,
             ULONG *h2cChannelMax,
             ULONG *c2hChannelMax,
             WDFCMRESLIST ResourcesRaw,
             WDFCMRESLIST ResourcesTranslated) 
{
  NTSTATUS status = STATUS_INTERNAL_ERROR;

  if (NULL == xdma) {
    KdPrint(("RMG: [%s] xdma = NULL\n", __FUNCTION__));
    return STATUS_INVALID_PARAMETER;
  }

  DeviceDefaultInitialize(xdma);

  xdma->wdfDevice = wdfDevice;

  // map PCIe BARs to host memory
  status = MapBARs(xdma, ResourcesTranslated);
  if (!NT_SUCCESS(status)) {
    KdPrint(("RMG: [%s] MapBARs() failed! status = %d\n", __FUNCTION__, status));
    TraceError(DBG_INIT, "MapBARs() failed! %!STATUS!", status);
    return status;
  }

  // identify BAR configuration - user(optional), config, bypass(optional)
  status = IdentifyBars(xdma);
  if (!NT_SUCCESS(status)) {
    KdPrint(("RMG: [%s] IdentifyBars() failed! status = %d\n", __FUNCTION__, status));
    TraceError(DBG_INIT, "IdentifyBars() failed! %!STATUS!", status);
    return status;
  }

  // get the module offsets in config BAR
  GetRegisterModules(xdma);

  // Confirm XDMA IP core version matches this driver
  UINT version = GetVersion(xdma);
  if (version != v2017_1) {
    TraceWarning(DBG_INIT, "Version mismatch! Expected 2017.1 (0x%x) but got (0x%x)",
           v2017_1, version);
  }

  status = SetupInterrupts(xdma, userMax, h2cChannelMax, c2hChannelMax, ResourcesRaw, ResourcesTranslated);
  if (!NT_SUCCESS(status)) {
    KdPrint(("RMG: [%s] SetupInterrupts failed: status = %d\n", __FUNCTION__, status));
    TraceError(DBG_INIT, "SetupInterrupts failed: %!STATUS!", status);
    return status;
  }

  // WDF DMA Enabler - at least 8 bytes alignment
  WdfDeviceSetAlignmentRequirement(xdma->wdfDevice, 8 - 1); // TODO - choose correct value
  
  // Los pasos están definidos en 'Enabling DMA Transactions':
  // https://learn.microsoft.com/en-us/windows-hardware/drivers/wdf/enabling-dma-transactions     
  
  WDF_DMA_ENABLER_CONFIG dmaConfig;
  
  
  size_t maxXferLen;
#ifdef USE_FUSE_DMA
  maxXferLen = FUSE_DMA_MAX_TRANSFER_SIZE;
#else
  maxXferLen = XDMA_MAX_TRANSFER_SIZE;
#endif    
  WDF_DMA_ENABLER_CONFIG_INIT(&dmaConfig, WdfDmaProfileScatterGather64Duplex, maxXferLen);
  
  //       DMA_ENABLER_CONFIG Config,     WDF_DMA_PROFILE Profile,            size_t MaximumLength
  // Profile = A WDF_DMA_PROFILE-typed value, which identifies the type of 
  // bus-master DMA operation that will be associated with the DMA enabler 
  // object.
  //    WdfDmaProfileScatterGather64Duplex: The device supports packet-based, 
  //    scatter/gather DMA operations, using 64-bit addressing. The device 
  //    also supports duplex operation.
  // MaximumLength = The default maximum size, in bytes, that the device can 
  // handle in a single DMA transfer. (Drivers can override this default value
  // for individual DMA transactions by calling WdfDmaTransactionSetMaximumLength.)

  status = WdfDmaEnablerCreate(xdma->wdfDevice,           // WDFDEVICE 
                               &dmaConfig,                // PWDF_DMA_ENABLER_CONFIG
                               WDF_NO_OBJECT_ATTRIBUTES,  // PWDF_OBJECT_ATTRIBUTES
                               &xdma->dmaEnabler);        // WDFDMAENABLER *  
  if (!NT_SUCCESS(status)) {
    KdPrint(("RMG: [%s] WdfDmaEnablerCreate() failed: status = %d\n", __FUNCTION__, status));
    TraceError(DBG_INIT, " WdfDmaEnablerCreate() failed: %!STATUS!", status);
    return status;
  }

  // Detect and initialize engines configured in HW IP 
  status = ProbeEngines(xdma);
  if (!NT_SUCCESS(status)) {
    KdPrint(("RMG: [%s] ProbeEngines failed: status = %d\n", __FUNCTION__, status));
    TraceError(DBG_INIT, "ProbeEngines failed: %!STATUS!", status);
    return status;
  }

  xdma->max_n_buffers = 0;
  
  
  //---
  xdma->FUSE_valid_design = FALSE;
  xdma->FUSE_version = 0;
  xdma->FUSE_has_emul_module = FALSE;

  status = FUSE_get_PCI_conf_info(wdfDevice, xdma);
  if (!NT_SUCCESS(status)) {
    dbg_printf(("RMG: [%s] FUSE_get_PCI_conf_info failed = %d\n", __FUNCTION__, status));
    TraceError(DBG_INIT, "FUSE_get_PCI_conf_info failed: %!STATUS!", status);
    return status;
  }

  status = FUSE_set_version(xdma);
  if (!NT_SUCCESS(status)) {
    dbg_printf(("RMG: [%s] FUSE_set_version failed = %d\n", __FUNCTION__, status));
    TraceError(DBG_INIT, "FUSE_set_version failed: %!STATUS!", status);
    return status;
  }

  if (xdma->FUSE_valid_design) {
    status = FUSE_initialize_IPs(wdfDevice, xdma);
    if (!NT_SUCCESS(status)) {
      dbg_printf(("RMG: [%s] FUSE_initialize_IPs failed = %d\n", __FUNCTION__, status));
      TraceError(DBG_INIT, "FUSE_initialize_IPs failed: %!STATUS!", status);
      return status;
    }
  }

//  if (1) return STATUS_UNSUCCESSFUL;

  return status;
}

void XDMA_DeviceClose(PXDMA_DEVICE xdma) {

  KdPrint(("\n\n\n"));
  KdPrint(("RMG: [%s] max_n_buffer = %d\n", __FUNCTION__, xdma->max_n_buffers));
  KdPrint(("\n\n\n"));

  // todo - stop every engine?

  // reset irq vectors?
  if (xdma && xdma->interruptRegs) {
    xdma->interruptRegs->userVector[0] = 0;
    xdma->interruptRegs->userVector[1] = 0;
    xdma->interruptRegs->userVector[2] = 0;
    xdma->interruptRegs->userVector[3] = 0;
    xdma->interruptRegs->channelVector[0] = 0;
    xdma->interruptRegs->channelVector[1] = 0;
  }

  closeEngines(xdma);

  // Unmap any I/O ports. Disconnecting from the interrupt will be done automatically by the framework.
  for (UINT i = 0; i < xdma->numBars; i++) {
    if (xdma->bar[i] != NULL) {
      TraceInfo(DBG_INIT, "Unmapping BAR%d, VA:(%p) Length %ul",
            i, xdma->bar[i], xdma->barLength[i]);
      MmUnmapIoSpace(xdma->bar[i], xdma->barLength[i]);
      xdma->bar[i] = NULL;
    }
  }
}

//----------------------------------------

static NTSTATUS FUSE_get_PCI_conf_info(IN WDFDEVICE device, IN XDMA_DEVICE *xdma) {
  
  NTSTATUS status;
  BUS_INTERFACE_STANDARD  BusInterface;

  KdPrint(("[%s] +\n", __FUNCTION__));

#define PCI_HEADER_SIZE  0x40

  // Get the BUS_INTERFACE_STANDARD for our device so that we can
  // read & write to PCI config space.
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
      xdma->RevisionID  = pPciConfig->RevisionID;
      xdma->SubVendorID = pPciConfig->u.type0.SubVendorID;
      xdma->SubSystemID = pPciConfig->u.type0.SubSystemID;
      KdPrint(("[%s] RevisionID = 0x%2.2X\n", __FUNCTION__, xdma->RevisionID));
      KdPrint(("[%s] SubVendorID = 0x%4.4X\n", __FUNCTION__, xdma->SubVendorID));
      KdPrint(("[%s] SubSystemID = 0x%4.4X\n", __FUNCTION__, xdma->SubSystemID));
    }
  }

  KdPrint(("[%s] -\n", __FUNCTION__));
  return status;  
}

static NTSTATUS FUSE_set_version(IN XDMA_DEVICE *xdma) {
  NTSTATUS status = STATUS_SUCCESS;

  xdma->FUSE_valid_design = FALSE;
  xdma->FUSE_version = 0;
  xdma->FUSE_has_emul_module = FALSE;

  if (xdma->SubSystemID) {
    switch (xdma->SubSystemID) {
      case FUSE_DEVELOPMENT_SUBSYSTEM_ID:
        xdma->FUSE_valid_design = TRUE;
        xdma->FUSE_version = (UINT32) xdma->RevisionID;
        xdma->FUSE_has_emul_module = TRUE;
        break;
      case FUSE_PRODUCTION_SUBSYSTEM_ID:
        xdma->FUSE_valid_design = TRUE;
        xdma->FUSE_version = (UINT32)xdma->RevisionID;
        xdma->FUSE_has_emul_module = FALSE;
        break;

      case FUSE_AXKU041_SUBSYSTEM_ID: // Chinese board
        xdma->FUSE_valid_design = TRUE;
        xdma->FUSE_version = (UINT32)xdma->RevisionID;
        xdma->FUSE_has_emul_module = FALSE;
        break;
        
      case FUSE_HTG_K800_SUBSYSTEM_ID:
        xdma->FUSE_valid_design = TRUE;
        xdma->FUSE_version = (UINT32)xdma->RevisionID;
        xdma->FUSE_has_emul_module = FALSE;
        break;
      default:
        break;
    }
  }

  return status;
}


//-----------------------------------------------

#include "fuse_pcie_soc_hw.h"
#include "fuse_common.h"
#include "fuse_platform.h"

static const fuse_emul_Config emul_cfg = {
  .dev_ID = 0,                      // Unique ID of device
  .base_addr = SYS_FUSE_EMUL_BASE_ADDR, // Register base address
  .f_clk = PCIE_AXI_CLK_FREQ,       // Clock frequency
};

static const fuse_olink_Config olink_cfg = {
  .dev_ID = 0,                                  // Unique ID of device
  .base_addr = SYS_OLINK_BASE_ADDR,             // Register bank base address
  .f_clk = PCIE_AXI_CLK_FREQ,                   // Clock frequency
  .n_lanes = SYS_OLINK_N_LANES,                 // Number of lanes
  .data_rate_gbps = SYS_OLINK_LANE_RATE_GBPS,   // Line rate in Gbps
};

static const fuse_dma_Config dma_cfg = {
  .dev_ID = 0,                                          // Unique ID of device
  .base_addr = SYS_FUSE_DMA_BASE_ADDR,                  // Register bank base address
  .mem_size_MB = SYS_FUSE_DMA_MEM_SIZE_MB,              // DDR memory size (in MBytes)
  .axi_freq_MHz = SYS_FUSE_DMA_AXI_FREQ_MHZ,            // AXI clock frequency
  .rx_axis_beat_size = (SYS_FUSE_DMA_RX_AXIS_DWIDTH/8), // Rx AXI stream beat size (in bytes)
  .rx_axis_freq_MHz = SYS_FUSE_DMA_RX_AXIS_FREQ_MHZ,    // Rx AXI stream clock frequency
  .tx_axis_beat_size = (SYS_FUSE_DMA_TX_AXIS_DWIDTH/8), // Tx AXI stream beat size (in bytes)
  .tx_axis_freq_MHz = SYS_FUSE_DMA_TX_AXIS_FREQ_MHZ,    // Tx AXI stream clock frequency
  .dev_dna = SYS_FUSE_DMA_DEV_DNA,                      // Device DNA (hex stream)
};

static NTSTATUS FUSE_initialize_IPs(IN WDFDEVICE device, IN XDMA_DEVICE *xdma) {
  NTSTATUS status = STATUS_INTERNAL_ERROR;
  UNREFERENCED_PARAMETER(device);
  void *hw_mmap = xdma->bar[xdma->userBarIdx];
  int ret;

  KdPrint(("[%s] +\n", __FUNCTION__));

  if (xdma->FUSE_has_emul_module) {
    memset(&(xdma->emul_drv), 0, sizeof(fuse_emul));
    ret = fuse_emul_initialize(&(xdma->emul_drv), hw_mmap, &emul_cfg);
    if (ret < 0) {
      dbg_printf(("RMG: [%s] fuse_emul_initialize failed = %d\n", __FUNCTION__, ret));
      TraceError(DBG_INIT, "fuse_emul_initialize() failed: %!STATUS!", ret);
      return STATUS_INTERNAL_ERROR;
    }

    fuse_emul_set_sw_reset(&(xdma->emul_drv), 1);
    fuse_kernel_delay_us(1);
    fuse_emul_set_sw_reset(&(xdma->emul_drv), 0);
    fuse_kernel_delay_us(1);

    fuse_emul_clr_int_flags(&(xdma->emul_drv), ~0U);
    fuse_emul_set_gie(&(xdma->emul_drv), 1);
  }

  memset(&(xdma->fuse_dma_drv), 0, sizeof(fuse_dma));
  ret = fuse_dma_initialize(&(xdma->fuse_dma_drv), hw_mmap, &dma_cfg);
  if (ret < 0) {
    dbg_printf(("RMG: [%s] fuse_dma_initialize failed = %d\n", __FUNCTION__, ret));
    TraceError(DBG_INIT, "fuse_dma_initialize() failed: %!STATUS!", ret);
    return STATUS_INTERNAL_ERROR;
  }
  fuse_dma_set_gie(&(xdma->fuse_dma_drv), 1);   // <= ACTIVO GLOBALMENTE las interrupciones !!!

  // Cuando recargo el driver reseteo el controlador de memoria...
  fuse_dma_set_mem_reset(&(xdma->fuse_dma_drv), 1);
  fuse_kernel_delay_us(1);
  fuse_dma_set_mem_reset(&(xdma->fuse_dma_drv), 0);
  fuse_kernel_delay_us(1);

  // ...y el controlador fuse DMA
  fuse_dma_set_sw_reset(&(xdma->fuse_dma_drv), 1);
  fuse_kernel_delay_us(1);
  fuse_dma_set_sw_reset(&(xdma->fuse_dma_drv), 0);
  fuse_kernel_delay_us(1);

  // fuse_dma_flush(&(xdma->fuse_dma_drv));  // Flush de la DDR fifo

  DbgPrint("RMG: [%s] rx enable inicial\n", __FUNCTION__);
  fuse_dma_set_rx_enable(&(xdma->fuse_dma_drv), 1);     // <= ACTIVO la llegada de datos!!!

  memset(&(xdma->olink_drv), 0, sizeof(fuse_olink));
  ret = fuse_olink_initialize(&(xdma->olink_drv), hw_mmap, &olink_cfg);
  if (ret < 0) {
    dbg_printf(("RMG: [%s] fuse_olink_initialize failed = %d\n", __FUNCTION__, ret));
    TraceError(DBG_INIT, "fuse_olink_initialize() failed: %!STATUS!", ret);
    return STATUS_INTERNAL_ERROR;
  }

#if 0
  if (xdma->SubSystemID == FUSE_HTG_K800_SUBSYSTEM_ID) {
    //fuse_olink_set_brd_i2c_reset(&(xdma->olink_drv), 1);
    //fuse_kernel_delay_us(1);
    //fuse_olink_set_brd_i2c_reset(&(xdma->olink_drv), 0);
    //fuse_kernel_delay_us(1);
    ret = HTG_K800_init_gt_clk_gen(&(xdma->olink_drv));
  }
#endif

  fuse_olink_set_mgt_reset(&(xdma->olink_drv), 1);
  fuse_kernel_delay_us(100 * 1000);
  fuse_olink_set_mgt_reset(&(xdma->olink_drv), 0);
  fuse_kernel_delay_us(100 * 1000);

  int pll_locked;
  fuse_olink_get_mgt_pll_lock(&(xdma->olink_drv), &pll_locked);
  status = (pll_locked)? STATUS_SUCCESS : STATUS_INTERNAL_ERROR;
  
  DbgPrint("[%s] pll_locked = %d\n", __FUNCTION__, pll_locked ? 1 : 0);
  fuse_olink_set_gie(&(xdma->olink_drv), 1);  // <= ACTIVO GLOBALMENTE las interrupciones !!!

  KdPrint(("[%s] -\n", __FUNCTION__));
  status = STATUS_SUCCESS;
  return status;
}


//FUSE_initialize_IPs
//    return STATUS_INVALID_PARAMETER;

//if (xdma) {
//  KdPrint(("[%s] ---\n", __FUNCTION__));
//  return STATUS_SUCCESS;
//}
