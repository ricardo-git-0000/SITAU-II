#ifndef FUSE_DMA_HW_H
#define FUSE_DMA_HW_H

//----------------
//-- REGISTERS: --
//----------------

#define DMA_CTRL_REG_INDEX        0
#define DMA_VERSION_REG_INDEX     1
#define DMA_INT_ENABLE_REG_INDEX  2
#define DMA_INT_FLAG_REG_INDEX    3

#define DMA_IMAGE_ID_REG_INDEX    4
#define DMA_USED_BUFS_REG_INDEX   5
#define DMA_BUFFER_THR_REG_INDEX  6
#define DMA_TEST_REG_INDEX        7

#define DMA_TOTAL_TIME_REG_INDEX  8
#define DMA_ACTIVE_TIME_REG_INDEX 9
#define DMA_LATENCY_A_REG_INDEX   10
#define DMA_LATENCY_B_REG_INDEX   11

//-------------------------
//-- GLOBAL CONTROL REG: --
//-------------------------

#define DMA_CTRL_ENABLE         0x00000001
#define DMA_CTRL_INT_ENABLE     0x00000002
#define DMA_CTRL_SW_RESET       0x00000004

#define DMA_CTRL_MEM_RESET      0x00000008
#define DMA_CTRL_RX_ENABLE      0x00000010

#define DMA_STATUS_DNA_OK       0x00100000
#define GET_MEM_SIZE(x) (((x) >> 16) & 0xF)

//-----------------------
//-- USED BUFFERS REG: --
//-----------------------

#define DMA_FLUSH               0x80000000

//---------------
//-- TEST REG: --
//---------------

#define TEST_IRQ                0x80000000

//---------------------------
//-- BUFFER THRESHOLD REG: --
//---------------------------

#define SET_BUFFER_THRS(thr_l, thr_h)   ((((thr_l) & 0xFFFF) << 00) | (((thr_h) & 0xFFFF) << 16))
#define GET_BUFFER_THR_L(thr)           (((thr) >> 00) & 0xFFFF)
#define GET_BUFFER_THR_H(thr)           (((thr) >> 16) & 0xFFFF)

// Los 16 MSb se utilizan para fijar el patrón a detectar en
// los datos y de esta forma detectar errores de comunicación.

//-------------------
//-- LATENCY REGS: --
//-------------------

// Los registros de laetencia corresponden a sendos contadores que se utilizan
// para determininar las prestaciones del sistema. El la latencia A mide el
// tiempo total de una transacción, es decir, el tiempo de desde que se envía
// el primer dato de una trama hasta que se desactiva la señal de interrupción
// empleada para notificar al SW que se ha recibido la trama.
// Por otra parte, la latencia A corresponde a la latencia de respuesta a la 
// interrupción, es decir, el tiempo que transcurre desde que se activa la 
// interrupción hasta que el SW limpia el flag correspondiente.

#endif  // FUSE_DMA_HW_H
