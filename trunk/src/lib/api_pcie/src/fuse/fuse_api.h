#ifndef FUSE_USER_API_H
#define FUSE_USER_API_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

// Declaración de tipos opacos para los principales elementos del API.

typedef void * fuse_handle_t;
typedef void * dma_buffer_t;
typedef uint32_t fuse_event_t;
typedef uint64_t fuse_status_t;

// fuse_create: Abre la comunicación con el device driver de la tarjeta PCIe 
// cuyo indice indica el argumento 'board_index'. Este indice está comprendido
// entre 0 y N-1, siendo N el número total de tarjetas montadas en el PC. Si,
// como en la mayoría de los casos, sólo hay una tarjeta en el sistema dicho
// índice será 0.
// La función devuelve un handle que será empleado en las restantes funciones 
// de este API. Esta es la primera función del API a ejecutar.

// Valor devuelto: handle de la tarjeta o NULL en caso de error.

fuse_handle_t fuse_create(int board_index);

// fuse_close: Cierra la comunicación con el dispositivo. Recibe como único 
// argumento el handler obtenido mediante fuse_create(). 
// Valor devuelto: 0 operación correcta, < 0 error de operación
int fuse_close(fuse_handle_t fuse);


int fuse_set_hw_conf(fuse_handle_t fuse);




// fuse_get_event_olink: Proporciona la máscara de eventos correspondientes el
// módulo optical link (olink). Cada uno de los bits de esta máscara 
// corresponde a uno de los eventos que puede probocar la generación de 
// interrupciones por parte de dicho módulo. Los eventos se devuelven a través
// del argumento 'mask' pasado por referencia. Por último, el argmento clear,
// indica si se han de borrar los flags activados para futuras lecturas: 0 no
// los borra, != 0 los borra.
// Valor devuelto: 0 operación correcta, < 0 error de operación

int fuse_get_event_olink(fuse_handle_t fuse, fuse_event_t *mask, int clear);

// fuse_enable_event_olink: Habilita los eventos correspondientes al módulo 
// optical link (olink). Cada uno de los bits de esta máscara corresponde 
// a uno de los eventos que puede probocar la generación de interrupciones 
// por parte de dicho módulo. Solo los eventos habilitados serán notificados
// en la función fuse_get_event_olink(). Es decir, un evento no habilitado,
// aunque se produzca, no será notificado al usuario.
// Valor devuelto: 0 operación correcta, < 0 error de operación

int fuse_enable_event_olink(fuse_handle_t fuse, fuse_event_t mask);

// Funciones de gestión de eventos para el módulo fuse DMA:
int fuse_get_event_dma(fuse_handle_t fuse, fuse_event_t *mask, int clear);
int fuse_enable_event_dma(fuse_handle_t fuse, fuse_event_t mask);

// Funciones de gestión de eventos para el módulo fuse PCI:
int fuse_get_event_pci(fuse_handle_t fuse, fuse_event_t *mask, int clear);
int fuse_enable_event_pci(fuse_handle_t fuse, fuse_event_t mask);

// Funciones de gestión de eventos para el módulo fuse emulation. Este módulo
// se utiliza únicamente en labores de depuración, no estando incluido en 
// todas las versiones del diseño final.

//int fuse_get_event_pci(fuse_handle_t fuse, fuse_event_t *mask, int clear);
//int fuse_enable_event_pci(fuse_handle_t fuse, fuse_event_t mask);

// ---------------------------------------------
// -- Funciones para lectura de datos vía DMA --
// ---------------------------------------------

// fuse_calc_image_size: Calcula el tamaño de una imagen HW (hw_img) en bytes
// en función de los parámetros o dimensiones internas que la caracterizan: 
// longitud de tramas (frame_len) y número de secuencias de tramas (image_len).
// En el estado actual del diseño este tamaño no puede exceder los 8 MB.
// Valor devuelto: > 0 operación correcta, 0 error de operación

size_t fuse_calc_image_size(fuse_handle_t fuse, int frame_len, int image_len);

// fuse_create_dma_buffers: Crea los buffers internos sobre los que se 
// almacenarán los datos recibidos mediante DMA. Estos buffers se crean 
// internamente con un determinado alineamiento para no penalizar las 
// prestaciones del sistema.
// Los buffers se deben crear antes de empezar a recibir datos.
// El argumento 'n_hw_img' indica el número total de buffers a reservar, 
// mientras que el argumento 'hw_img_size' indica el tamaño de cada buffer. 
// Este tamaño no puede exceder los 8 MB.
// Valor devuelto: 0 operación correcta, < 0 error de operación

int fuse_create_dma_buffers(fuse_handle_t fuse, int n_hw_img, size_t hw_img_size);

// fuse_destroy_dma_buffers: Destruye los buffers previamente creados mediante
// la función 'fuse_create_dma_buffers()', liberando la memoria reservada a 
// tal efecto.
// Antes de destruir los buffers se debe asegurar que no están siendo usados 
// por el usuario, es decir, que todos ellos han sido devueltos mediante la 
// función 'fuse_free_dma_buffer()'. A este respecto, puede resultar útil el
// uso de la función fuse_dma_buffer_leakage(), que indica el número de 
// buffers que aún no han sido devueltos por el usuario.
// Valor devuelto: 0 operación correcta, < 0 error de operación

int fuse_destroy_dma_buffers(fuse_handle_t fuse);



// TBD:
// Se trataría poder reutilizar los buffers previamente creados, por ejemplo,
// cuando tenemos ya reservados un determinado número de buffers y queremos
// modificar dicho número (aumentar o disminuir). Tiene su interes cuando el
// tamaño total de memoria reservada es grande (> 4GB).
// Es una alternativa a la mera destrucción de los buffers originales y la
// creación de los nuevos con las nuevas características.

// int fuse_resize_dma_buffers(fuse_handle_t fuse, int n_hw_img, size_t hw_img_size);



// fuse_get_dma_buffer: Solicita la obtención de imágenes HW (hw_img) 
// recibidas vía DMA. El argumento 'n' indica el número de hw_img solicitadas.
// Este número puede ser inferior al disponible en el momento de realizar
// la llamada. La función devuelve el número total de hw_img obtenidas, 
// almacenando en el argumento 'bufs' los punteros a los buffers donde se 
// almacenan dichas hw_img.
// Es responsabilidad del usuario devolver los bufferes una vez que han sido
// utilizados.
// Valor devuelto: >= 0 número de hw_img obtenidas, < 0 error de operación

int fuse_get_dma_buffer(fuse_handle_t fuse, dma_buffer_t *bufs, int n);
                            
// fuse_free_dma_buffer: Libera un buffer previamente obtenido mediante la 
// función 'fuse_get_dma_buffer'. Esto permite la reutilización de dicho 
// buffer para almacenar nuevos datos en transferencias DMA posteriores.
// Es responsabilidad del usuario devolver los buffers una vez ha utilizado
// los buffers que contienen.
// Valor devuelto: 0 operación correcta, < 0 error de operación

int fuse_free_dma_buffer(fuse_handle_t fuse, dma_buffer_t b);

// fuse_dma_buffer_rdy: Esta función indica el número de buffers disponibles
// en ese momento. El contenido de estos buffers se pueden obtener mediante la 
// función 'fuse_get_dma_buffer'.
// Valor devuelto: 0 operación correcta, < 0 error de operación

int fuse_dma_buffer_rdy(fuse_handle_t fuse);


// fuse_dma_start: Comienza la adquisición de hw_img vía DMA. Antes de comenzar
// la adqusición, mediante la función 'fuse_create_dma_buffers' se deben reservar 
// previamente los buffers donde se almacenarán los datos recibidos.
//   
// El argumento 'n_hw_img' indica el número total de hw_img a recibir. Una vez 
// alcanzado dicho número se detendrá la recepción de datos. Esto implica que no
// se le suministrarán al HW más buffers donde almacenar los datos recibidos.
// Si este número es negativo, se entenderá que la adquisición se realiza en 
// modo continuo, en cuyo caso deberá ser detenida mediante la función 
// 'fuse_dma_stop()'.
//
// Los siguientes argumentos definen las dimensiones internas de cada hw_img:
// 'frame_len' es la longitud de las tramas en beats (32 bytes), mientras 
// que 'image_len' es el número de secuencias de tramas (cada secuencia consta 
// de 4 tramas).
// Valor devuelto: 0 operación correcta, < 0 error de operación

int fuse_dma_start(fuse_handle_t fuse, int n_hw_img, int frame_len, int image_len);

// fuse_dma_stop: Detiene una adquisición previamente arrancada mediante la 
// función 'fuse_dma_start()'. 

int fuse_dma_stop(fuse_handle_t fuse);

// TBD: De momento innecesaria
// int fuse_dma_pause(fuse_handle_t fuse);


// fuse_dma_is_running: Es función devuelve un valor que indica el estado 
// actual de la adquisición: 0 detenida, > 0 en ejecución. En caso de error
// devuelve un valor negativo.
int fuse_dma_is_running(fuse_handle_t fuse);

// fuse_dma_buffer_recv: Devuelve el número total de hw_img recibidas desde la
// última orden de arranque dada mediante la funcion 'fuse_dma_start()'.
// Este número no es el número de imágenes obtenidas por el usuario mediante 
// llamadas a la función 'fuse_get_dma_buffer()'.
// En un escenario típico de acquisición no continua, en el que se desean 
// obtener N hw_imgs, el usuario arrancaría la adquisición mediante la función
// 'fuse_dma_start()'. A continuación, esperaría a que se completase la 
// recepción de esas N hw_img, llamando repetidas veces a la función 
// 'fuse_dma_buffer_recv()'. Una vez completada la recepción podría obtener
// todas las imágenes de una sola vez mediante una sola llamada a la función
// 'fuse_get_dma_buffer()'.
// Valor devuelto: 0 operación correcta, < 0 error de operación

long long fuse_dma_buffer_recv(fuse_handle_t fuse);

// fuse_dma_buffer_leakage: Devuelve el número total de buffers en uso por 
// parte del usuario. Se considera que un buffer está en uso desde que ha 
// sido obtenido mediante 'fuse_get_dma_buffer()' hasta que ha sido devuelto
// mediante 'fuse_free_dma_buffer()'.
// Valor devuelto: 0 operación correcta, < 0 error de operación

int fuse_dma_buffer_leakage(fuse_handle_t fuse);


// fuse_get_event_olink: Proporciona la palabra de estado del gestor DMA.
// Cada uno de los bits de esta palabra indican situaciones excepcionales
// que afectan al funcionamiento del equipo. Esta información de estado
// puede estar relacionada tanto con aspectos HW (p.e. caída del enlace
// óptico que conecta el sistema de adquisición con el PC) como aspectos
// SW (p.e. agotamiento de los recursos internos del controlar, al no 
// devolverse los buffers dma con la suficiente rapidez, etc.)
// Los eventos se devuelven a través del argumento 'status' pasado por 
// referencia. Por último, el argmento clear indica si se han de borrar 
// los flags activados para futuras lecturas: 0 no los borra, != 0 los borra.
// Valor devuelto: 0 operación correcta, < 0 error de operación.

int fuse_dma_get_status(fuse_handle_t fuse, fuse_status_t *status, int clear);

// A continuación se definen las máscaras correspondientes a distintas
// situaciones de error:

#define FUSE_STATUS_HW_ERR  0x00000001ULL
#define FUSE_STATUS_SW_ERR  0x00000002ULL


#ifdef __cplusplus
}
#endif

#endif  // FUSE_USER_API_H


