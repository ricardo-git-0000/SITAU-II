#pragma once

#define DEVS_ON_DRIVER

#include <windows.h>
#include <string>
#include <vector>

#include "sw_fifo.h"

//#include "dma_buffer.h"

// Interrupt defs:
#include <fuse_olink_ints.h>
#include <fuse_dma_ints.h>
#include <fuse_emul_ints.h>

#include <fuse_hw_img.h>

typedef void * dma_buffer_t;
typedef uint32_t fuse_event_t;
typedef uint64_t fuse_status_t;
typedef uint64_t timeout_t;
typedef int64_t  hw_size_t;

#define HW_EVENT_ALIGNED  __declspec(align(8))
#define HW_SIZE_ALIGNED   __declspec(align(8))


#define FUSE_STATUS_HW_ERR  0x00000001ULL
#define FUSE_STATUS_SW_ERR  0x00000002ULL

#define MAX_HW_IMAGE_SIZE   ((size_t)8388608UL)


// I2C Transfer attributes:
#define I2C_TRF_ATTR_START    0x0002
#define I2C_TRF_ATTR_STOP     0x0004
#define I2C_TRF_ATTR_NACK     0x0008

// FUSE DMA
#define FUSE_HW_RST_DMA       0x01
#define FUSE_HW_RST_MEM       0x02

#define DMA_READ_SW_ERR       0x01
#define DMA_IFIFO_SW_ERR      0x02
#define DMA_OFIFO_SW_ERR      0x04


class fuse {
public:
  int test();
public:
  // The DMA thread is created in the constructor
  fuse(int board_index = 0, int verbose = 0, int non_blocking_events = 1);
  ~fuse();
  
  int open(int board_index = 0);  // open = open_dev_files + create_threads
  int close();
  int abort();

  // HW EVENT MANAGEMENT:
  // Enable interrupt mask
  int enable_event_olink(fuse_event_t mask);
  int enable_event_dma(fuse_event_t mask);
  int enable_event_emul(fuse_event_t mask);
  
  // Non-blocking version (with thread support)
  int get_event_olink(fuse_event_t &e, int clear);
  int get_event_dma(fuse_event_t &e, int clear);
  int get_event_emul(fuse_event_t &e, int clear);

  // Blocking version (w/o thread support)
  int get_event_olink(fuse_event_t &e);
  int get_event_dma(fuse_event_t &e);
  int get_event_emul(fuse_event_t &e);
  
  // Get reading file error flag:  
  int get_event_err_olink(int &err);
  int get_event_err_dma(int &err);
  int get_event_err_emul(int &err);

  // File (DMA and event) access timeout management:
  int get_dma_timeout(timeout_t &timeout_ms) const {
    return get_file_timeout(c2h0_file, timeout_ms);
  }
  int set_dma_timeout(timeout_t timeout_ms) {
    m_dma_timeout = timeout_ms;
    return set_file_timeout(c2h0_file, timeout_ms);
  }

  int get_event_emul_timeout(timeout_t &timeout_ms) const {
    return get_file_timeout(emul_event_file, timeout_ms);
  }
  int set_event_emul_timeout(timeout_t timeout_ms) {
    return set_file_timeout(emul_event_file, timeout_ms);
  }

  int get_event_olink_timeout(timeout_t &timeout_ms) const {
    return get_file_timeout(olink_event_file, timeout_ms);
  }
  int set_event_olink_timeout(timeout_t timeout_ms) {
    return set_file_timeout(olink_event_file, timeout_ms);
  }

  int get_event_fuse_dma_timeout(timeout_t &timeout_ms) const {
    return get_file_timeout(dma_event_file, timeout_ms);
  }
  int set_event_fuse_dma_timeout(timeout_t timeout_ms) {
    return set_file_timeout(dma_event_file, timeout_ms);
  }

  // Optical link status info:
  int olink_get_lane_status(uint32_t &lane_status);
  int olink_get_channel_err(int &err);
  int olink_get_soft_err(int &err);
  int olink_get_hard_err(int &err);
  int olink_get_pll_lock(int &lock);
  
//  aurora_status_reg(7 downto 0) <= lane_status;
//  aurora_status_reg(8)  <= channel_up;
//  aurora_status_reg(9)  <= soft_err;
//  aurora_status_reg(10) <= hard_err;
//  aurora_status_reg(11) <= gt_pll_lock;
//  aurora_status_reg(31 downto 12) <= (others => '0');
  
  
  //-- I2C comms:
  int i2c_rst_brd_mux(int rst);
  int i2c_send_byte(uint8_t byte, int trf_attr = 0);
  int i2c_recv_byte(uint8_t *byte, int trf_attr = 0);
  
  //-- Fuse DMA management:
  int dma_hw_flush(uint32_t mode = 0);
//  int dma_hw_enable(int enable = 1);

  // Fuse DMA, DDR FIFO filling management:
  int dma_hw_get_filling_level(unsigned int &n); // Devuelve el numero de imagenes almacenandas en la DDR
  int dma_hw_get_filling_thrs(unsigned int &thr_l, unsigned int &thr_h);
  int dma_hw_set_filling_thrs(unsigned int thr_l, unsigned int thr_h);

  // Fuse DMA, input performance measurement:
  int dma_hw_get_iperf_cycles(unsigned int &total, unsigned int &active);
  int dma_hw_get_iperf_time(float &total, float &active);
  int dma_hw_get_iperf_BW(float &bw, unsigned int n_gap = 0);
  
  // Fuse DMA, input ID checking:
  int dma_hw_get_image_id_seed(unsigned int &id);
  int dma_hw_set_image_id_seed(unsigned int id);
  
  int dma_hw_get_mem_size(unsigned int &mem_size_mb);
  int dma_hw_dna_ok();
  
  int get_pcie_info(uint16_t &subsys_id, uint8_t &rev_id);
  // IP signature info:
  int olink_get_signature(uint32_t &signature);
  int dma_get_signature(uint32_t &signature);
  int emul_get_signature(uint32_t &signature);

  int olink_get_signature(int &version, int &subversion, int &revision, int &magic_number);
  int dma_get_signature(int &version, int &subversion, int &revision, int &magic_number);
  int emul_get_signature(int &version, int &subversion, int &revision, int &magic_number);
  
  // IP HW reset:
  int dma_hw_init();
  int dma_hw_reset(uint32_t mode = 0x3);
  int olink_hw_reset();
  int emul_hw_reset();

  // IP HW enable:
  int dma_hw_enable(int x);
  int olink_hw_enable(int x);
  int emul_hw_enable(int x);
      
  //---------------

  // HW image support functions:
  size_t calc_hw_image_size(int frame_len, int image_len) const;
  int calc_hw_image_gap(int frame_len, int image_len, uint64_t BW) const;
  // Synthetic image (generated by the emulator) checking:
  int get_hw_image_info(const void *img, int *id, int *frame_len, int *image_len);
  int hw_image_check(const void *img);

  // DMA operation control functions:
  int create_dma_buffers(int n_hw_img, size_t hw_img_size);
  int destroy_dma_buffers();
  int resize_dma_buffers(int n_hw_img, size_t hw_img_size);
  
  int get_dma_buffer(dma_buffer_t *bufs, int n);
  int free_dma_buffer(dma_buffer_t b);
  int dma_buffer_rdy();
  int wait_for_dma_buffer(int n, int timeout_ms);

  int dma_start(hw_size_t n_hw_img, int hw_frame_len, int hw_image_len);
  int dma_stop();
  int dma_is_running();
  hw_size_t dma_buffer_recv() const;  // Return the number of hw_images received since the last dma_start
  int dma_buffer_leakage();
  int dma_get_status(fuse_status_t &status, int clear);
  int dma_get_sw_err(int clear);    // Devuelve flags de error SW de lectura DMA.



  // Emulation management:
  int emul_run(int n_img, int frame_len, int image_len, int gap, uint32_t mode = 0);
  int emul_stop_cont(uint32_t mode = 0);
  //int emul_run(const img_cfg_t &hw_img_cfg, int cont_mode = 0);
  int emul_terminate();
  int emul_hw_reset(uint32_t mode);

  // Service functions to access to the FPGA HW:
//  int get_ddr_hw_images(int &n); // Devuelve el numero de imagenes almacenandas en la DDR


  //----

  int destroy_dma_thread();
  int close_dev_files();
  int create_dma_thread();
  
private:
  FILE *err_file;
  FILE *log_file;
  int m_board_index;
  int m_verbose;
  int m_use_event_thrd;   // Use an independent thread HW events (interrupts)

  // Access to HW device files (dma & hw events)
  int get_dev_path(std::wstring &dev_paths, int index = 0);
  int get_dev_path_table(std::vector<std::wstring> &dev_paths);
  int open_dev_files(int board_index);
  //int close_dev_files();
  
  void *user_file;  // Dev file for accessing peripherals mapped on the AXI-Lite bus
  void *user_mmap;  // AXI lite peripherals: memory map pointer
  size_t user_size; // AXI lite peripherals: memory map size

  // Device file handlers:
  void *c2h0_file;        // PCIe DMA rd channel (card to host)
  void *ctrl_file;        // Control file
  void *dma_event_file;   // Fuse DMA hw event (interrupt) file
  void *olink_event_file; // Optical link hw event (interrupt) file
  void *emul_event_file;  // Fuse emulator hw event (interrupt) file
  
  bool m_emul_module;     // The design includes a emulation module

  char m_rx_dma_err;      // C2H DMA file reading error
  char m_olink_event_err; // optical link event file reading error
  char m_dma_event_err;   // fuse dma event file reading error
  char m_emul_event_err;  // fuse emulator event file reading error

  // HW event (interrupt) management:  
  inline int _get_hw_event(void *hw_event_file, fuse_event_t &local_hw_event, fuse_event_t &e, int clear);
  inline int _get_hw_event(void *hw_event_file, int &hw_event_err, fuse_event_t &e);
  inline int _enable_hw_event(DWORD service_id, fuse_event_t mask);
  
  fuse_event_t HW_EVENT_ALIGNED m_olink_event;
  fuse_event_t HW_EVENT_ALIGNED m_dma_event;
  fuse_event_t HW_EVENT_ALIGNED m_emul_event;
  
  timeout_t m_max_timeout;  // max file timeout. Used during thread destruction
  int set_file_timeout(void *file, timeout_t to);
  int get_file_timeout(void *file, timeout_t &to) const;
  int cancel_event_file(void *file, uint32_t mode = 0);
  
  // Convenience services:
  int get_ip_signature(int service, uint32_t &id);
  int do_ip_hw_reset(int service, uint32_t mode = 0);
  int do_ip_hw_enable(int service, int x);

  // Optical link support functions:
  int olink_get_aurora_status(uint32_t &status);
  
  
  // THREAD MANAGEMENT:
  /*int create_dma_thread();*/
  //int destroy_dma_thread();
  timeout_t m_dma_timeout;
  
  int create_event_threads();
  int create_threads();
  void destroy_threads();

  
  // HW events (interrupts):
  DWORD olink_event_thread();
  HANDLE olink_event_hThread;

  DWORD dma_event_thread();
  HANDLE dma_event_hThread;
  


  //int alloc_dma_buffers(size_t n_buffers, size_t size);
  //int alloc_dma_buffers(int n_images, int frame_len, int image_len);
  //int free_dma_buffers();
  
  dma_buffer_t *dma_buffers;
  size_t n_dma_buffers;
  size_t dma_buffer_size;
  size_t alignment;


  sw_fifo ififo;
  sw_fifo ofifo;




  // DMA thread management:
  DWORD dma_thread();
  HANDLE dma_hThread;
  
  HANDLE abort_event;

  inline int read_hw_images_old(hw_size_t n_hw_img, size_t hw_img_size);
  inline hw_size_t read_hw_images(hw_size_t n_hw_img, size_t hw_img_size, int cont_mode);
  char m_dma_read_err;  // dma reading error flag
  char m_ififo_rd_err;  // input fifo reading error flag
  char m_ofifo_wr_err;  // output fifo writting error flag

  typedef struct {
    HANDLE start_event;
    HANDLE stop_event;
    int cmd;
    hw_size_t n_hw_img;
    size_t hw_img_size;
    uint64_t HW_SIZE_ALIGNED total_hw_img;
    int cont_mode;
  } dma_acq_conf_t;

  volatile dma_acq_conf_t dma_acq_cfg;
  int dma_abort();

  //-----

  //DWORD proc_thread();
  //DWORD emul_thread();
  //HANDLE proc_hThread;
  //HANDLE emul_hThread;


  //HANDLE emul_start_event;
  //volatile hw_img_conf_t emul_img_cfg;


  //HANDLE acq_start_event;

  // HW devices:
  // fuse_emul emul;

  //-------
  
  // Input performance factors:
  int      iperf_beat_size;
  float    iperf_clk_freq;
  float    iperf_clk_period;
  float    iperf_bw_factor;

};


// Window Implementation Libraries (WIL) 


// wil::unique_handle is a HANDLE wrapper that calls CloseHandle upon 
// destruction. It’s modeled mostly after the C++ std::unique_ptr<> 
// type.
// Notice that getting the internal HANDLE is done by calling get().
// To replace the value inside a unique_handle(and close the old one) 
// use the reset function; calling reset with no arguments just closes 
// the underlying handle, making the wrapper object an empty shell.