#ifndef FUSE_DMA_INTS_H
#define FUSE_DMA_INTS_H

//----------------------
//-- INTERRUPT MASKS: --
//----------------------

#define DMA_BUF_RDY_INT_MASK    0x0001    // There are DMA buffers available                (level interrupt)
#define DMA_NEW_BUF_INT_MASK    0x0002    // New DMA buffer received                        (event interrupt)
#define DMA_RX_OVF_INT_MASK     0x0004    // Rx overflow                                    (event interrupt)
#define DMA_IMG_ID_ERR_INT_MASK 0x0008    // Image ID counter missmatch                     (event interrupt)

#define DMA_IMG_ERR_INT_MASK    0x0010    // Image error (wrong format)                     (event interrupt)
#define DMA_DDR_OVF_INT_MASK    0x0020    // DDR FIFO overflow                              (event interrupt)
#define DMA_DDR_AFULL_INT_MASK  0x0040    // DDR FIFO almost FULL                           (level interrupt)
#define DMA_DDR_AEMPTY_INT_MASK 0x0080    // DDR FIFO almost EMPTY                          (level interrupt)

// For development purposes:
#define DMA_BUSY_INT_MASK       0x0100    // channel DMA status register: busy bit          (level interrupt)
#define DMA_COMPLETED_INT_MASK  0x0200    // channel DMA status register: completed bit     (level interrupt)
#define DMA_STOP_INT_MASK       0x0400    // channel DMA status register: stop bit          (level interrupt)
#define DMA_DESC_DONE_INT_MASK  0x0800    // channel DMA status register: donw bit          (level interrupt)

#define DMA_PKT_DONE_INT_MASK   0x1000    // channel DMA status register: packet done bit   (level interrupt)
#define DMA_PENDING_INT_MASK    0x2000    // channel DMA status register: pending bit       (level interrupt)
#define DMA_RUN_INT_MASK        0x4000    // channel DMA status register: run bit           (level interrupt)
#define DMA_TEST_INT_MASK       0x8000    // SW triggered test interrupt                    (level interrupt)

#endif // FUSE_DMA_INTS_H
