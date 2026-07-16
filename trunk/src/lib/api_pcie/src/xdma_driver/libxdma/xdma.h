#pragma once

#include <ntddk.h>
#include <wdf.h>

#include "device.h"

#define XDMA_MAKE_VERSION(major, minor, patch) (((major) << 24) | ((minor) << 26) | (patch))
#define XDMA_VERSION_MAJOR(version) (((uint32_t)(version) >> 24) & 0xff)
#define XDMA_VERSION_MINOR(version) (((uint32_t)(version) >> 16) & 0xff)
#define XDMA_VERSION_PATCH(version) ((uint32_t)(version) & 0xffff)

// TODO Bump this number when making a new release
#define XDMA_LIB_VERSION XDMA_MAKE_VERSION(2017, 4, 1)

/**
 * \brief Open and initialize an XDMA device given by the WDFDEVICE handle.
 * \param wdfDevice     [IN]        The OS device handle
 * \param xdma          [IN]        The XDMA device context
 * \param userMax       [IN OUT]    Max # of user/event (interrupts) to be configured
 * \param h2cChannelMax [IN OUT]    Max # of h2c channels to be enabled
 * \param c2hChannelMax [IN OUT]    Max # of c2h channels to be enabled
 * \param ResourcesRaw  [IN]        List of PCIe resources assigned to this device
 * \param ResourcesTranslated [IN]  List of PCIe resources assigned to this device
 * \return STATUS_SUCCESS on successful completion. All other return values indicate error conditions. 
 * \NOTE: If the user provisioned is more than the max specified or zero,
 * \libxdma will update the userMax, h2cChannelMax, c2hChannelMax.
 */
NTSTATUS XDMA_DeviceOpen(WDFDEVICE wdfDevice,
                         PXDMA_DEVICE xdma,
                         ULONG *userMax,
                         ULONG *h2cChannelMax,
                         ULONG *c2hChannelMax,
                         WDFCMRESLIST ResourcesRaw,
                         WDFCMRESLIST ResourcesTranslated);

/**
 * \brief Close and cleanup the XDMA device.
 * \param xdma          [IN]        The XDMA device context
 */
void XDMA_DeviceClose(PXDMA_DEVICE xdma);

/**
 * \brief Register a callback function to execute when user events occur. 
 * \param xdma          [IN]        The XDMA device context
 * \param index         [IN]        The Event ID of the user event (0-15)
 * \param handler       [IN]        The callback function to execute on event detection
 * \param userData      [IN]        Custom user data/handle which will be passed to the callback 
 *                                  function. 
 * \return STATUS_SUCCESS on successful completion. All other return values indicate error conditions. 
 */
NTSTATUS XDMA_UserIsrRegister(PXDMA_DEVICE xdma,
                              ULONG index,
                              PFN_XDMA_USER_WORK handler,
                              void* userData);

/**
 * \brief Enable interrupt handling for a specific user event. 
 * \param xdma          [IN]        The XDMA device context
 * \param eventId       [IN]        The Event ID of the user event (0-15)
 * \return STATUS_SUCCESS on successful completion. All other return values indicate error conditions. 
 */
NTSTATUS XDMA_UserIsrEnable(PXDMA_DEVICE xdma, ULONG eventId);

/**
 * \brief Disable interrupt handling for a specific user event. 
 * \param xdma          [IN]        The XDMA device context
 * \param eventId       [IN]        The Event ID of the user event (0-15)
 * \return STATUS_SUCCESS on successful completion. All other return values indicate error conditions. 
 */
NTSTATUS XDMA_UserIsrDisable(PXDMA_DEVICE xdma, ULONG eventId);

/**
 * \brief OS callback function for programming the XDMA engine
 * \param Transaction    [IN]        The WDFDMATRANSACTION handle
 * \param Device         [IN]        The WDFDEVICE handle
 * \param Context        [IN]        The XDMA engine context
 * \param Direction      [IN]        Data transaction direction. H2C=WdfDmaDirectionToDevice. C2H=WdfDmaDirectionFromDevice
 * \param SgList         [IN]        The Scatter-Gather list describing the Host-side memory.
 * \return TRUE on success, else FALSE
 */
EVT_WDF_PROGRAM_DMA XDMA_EngineProgramDma;

/**
 * \brief Select between polling and interrupts as a mechanism for determining dma transfer 
 *        completion on a per DMA engine basis.
 * \param engine        [IN]        The DMA engine context
 * \param pollMode      [IN]        true = use polling, false = use interrupts
 */
void XDMA_EngineSetPollMode(XDMA_ENGINE* engine, BOOLEAN pollMode);

//----------------------------------------

// FUSE RMG:
 
/**
 * \brief OS callback function for programming the XDMA engine fr FUSE
 * \param Transaction    [IN]        The WDFDMATRANSACTION handle
 * \param Device         [IN]        The WDFDEVICE handle
 * \param Context        [IN]        The XDMA engine context
 * \param Direction      [IN]        Data transaction direction. H2C=WdfDmaDirectionToDevice. C2H=WdfDmaDirectionFromDevice
 * \param SgList         [IN]        The Scatter-Gather list describing the Host-side memory.
 * \return TRUE on success, else FALSE
 */
EVT_WDF_PROGRAM_DMA FUSE_EngineProgramDma;

NTSTATUS FUSE_UserIsrRegister(PXDMA_DEVICE xdma,
  ULONG index,
  fuseEventCB_t handler,
  void *syncObj);
