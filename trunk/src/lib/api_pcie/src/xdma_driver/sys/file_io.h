/*
-------------------------------------------------------------------------------
-- Description :
-- This file is part of the Xilinx DMA IP Core driver for Windows.
--
-------------------------------------------------------------------------------
*/

#pragma once

// ========================= include dependencies =================================================

#include "xdma.h"

// ========================= declarations =========================================================


// RMG: Equivalente al tipo de dev char de Linux que se emplea para operación:

// The type of the device node - this determines behaviour of EvtIoRead and EvtIoWrite functions
typedef enum {
  DEVNODE_TYPE_USER,
  DEVNODE_TYPE_CONTROL,
  DEVNODE_TYPE_EVENTS,
  DEVNODE_TYPE_BYPASS,
  DEVNODE_TYPE_H2C,
  DEVNODE_TYPE_C2H,
  ID_DEVNODE_UNKNOWN = 255,
} DEVNODE_TYPE;


#define DEFAULT_EVENT_FILE_TIMEOUT_MS  (1000)
#define DEFAULT_DMA_FILE_TIMEOUT_MS    (2000)

// File Context Data
typedef struct _FILE_CONTEXT {
  DEVNODE_TYPE devType;
  
  // RMG: Usan una union para definir el tipo de información 
  // que pasarán en cada caso.
  union { 
    void *bar;                  // USER / CONTROL / BYPASS
    XDMA_EVENT *event;          // EVENTS
    FUSE_XDMA_EVENT *fuseEvent; // FUSE_EVENT;
    XDMA_ENGINE *engine;        // H2C / C2H
  } u;
  WDFQUEUE queue;               // Cola para canales DMA  
  PMDL mdl;                     // RMG: memory descriptor list (MDL) (memory mapped file)
  PVOID virtAddress;            // For mapped BAR
  UINT64 timeout_ms;
} FILE_CONTEXT, *PFILE_CONTEXT;

// RMG: bar es el BAR asociado para cada tipo de dev_char

WDF_DECLARE_CONTEXT_TYPE_WITH_NAME(FILE_CONTEXT, GetFileContext)

// Queue Context Data
typedef struct _QUEUE_CONTEXT {
  XDMA_ENGINE* engine;
} QUEUE_CONTEXT, *PQUEUE_CONTEXT;

WDF_DECLARE_CONTEXT_TYPE_WITH_NAME(QUEUE_CONTEXT, GetQueueContext)



EVT_WDF_IO_IN_CALLER_CONTEXT        EvtDeviceIoInCallerContext;
EVT_WDF_DEVICE_FILE_CREATE          EvtDeviceFileCreate;
EVT_WDF_FILE_CLOSE                  EvtFileClose;
EVT_WDF_FILE_CLEANUP                EvtFileCleanup;
EVT_WDF_IO_QUEUE_IO_DEVICE_CONTROL  EvtIoDeviceControl;
EVT_WDF_IO_QUEUE_IO_READ            EvtIoRead;
EVT_WDF_IO_QUEUE_IO_WRITE           EvtIoWrite;

EVT_WDF_IO_QUEUE_IO_READ            EvtIoReadDma;
EVT_WDF_IO_QUEUE_IO_WRITE           EvtIoWriteDma;
EVT_WDF_IO_QUEUE_IO_READ            EvtIoReadEngineRing;

// RMG FUSE:
EVT_WDF_IO_QUEUE_IO_READ            FUSE_EvtIoReadDma;


NTSTATUS EvtReadUserEvent(WDFREQUEST request, size_t length);
VOID HandleUserEvent(ULONG eventId, void* userData);

// RMG FUSE:
NTSTATUS FUSE_EvtReadUserEvent(WDFREQUEST request, size_t length);
VOID FUSE_HandleUserEvent(ULONG eventId, void *userData, fuse_flags_t flags);
