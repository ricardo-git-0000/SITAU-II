#pragma once

#include "xdma.h"


// NOTAS:
// The object context area is a driver-defined storage area for data that 
// the driver uses with a particular object. 
// A KMDF object can have one or more object context areas. To declare 
// the context area type, use the WDF_DECLARE_CONTEXT_TYPE_WITH_NAME macro 
// in a header file. WDF_DECLARE_CONTEXT_TYPE_WITH_NAME assigns a 
// driver-specified name to the accessor method.
//  => definimos el nombre del método de acceso


typedef struct DeviceContext_t {
  XDMA_DEVICE xdma;
  WDFQUEUE    engineQueue[2][XDMA_MAX_NUM_CHANNELS];
  KEVENT      eventSignals[XDMA_MAX_USER_IRQ];
} DeviceContext;

// GetDeviceContext es una función mía

WDF_DECLARE_CONTEXT_TYPE_WITH_NAME(DeviceContext, GetDeviceContext)