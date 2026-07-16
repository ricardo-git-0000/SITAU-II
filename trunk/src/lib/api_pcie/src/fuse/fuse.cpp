#define ECHO_DMA_THREAD
//#define ECHO_PROC_THREAD
#define IFIFO_INF_TIMEOUT


//#define DBG_MSG_ON

#ifdef DBG_MSG_ON
#define dbg_printf(format, ...) printf(format, __VA_ARGS__)
#else
#define dbg_printf(format, ...)
#endif

#include "fuse.h"
#include "log.h"
#include "pcie_error_code.h"

#include <string>
#include <vector>
#include <cstdlib>

#include <fuse_pcie_soc_hw.h>


#include <INITGUID.H>
#include "xdma_public.h"

#include <SetupAPI.h>
#pragma comment(lib, "setupapi.lib")

#define FREQ_AXI_CLK  (250.0e6)

#include "fuse_types.h"
#include "log.h"
#include "pcie_error_code.h"

#define DMA_CMD_NOP   0x00
#define DMA_CMD_START 0x01
#define DMA_CMD_EXIT  0xFF


fuse::fuse(int board_index, int verbose, int non_blocking_events) :
  ififo(1024),
  ofifo(1024)
{
  err_file = stderr;
  log_file = stdout;
  m_board_index = board_index;
  m_verbose = verbose;
  m_use_event_thrd = non_blocking_events;

  user_file = INVALID_HANDLE_VALUE;
  user_mmap = nullptr;
  user_size = 0;

  c2h0_file = INVALID_HANDLE_VALUE;
  ctrl_file = INVALID_HANDLE_VALUE;
  dma_event_file = INVALID_HANDLE_VALUE;
  olink_event_file = INVALID_HANDLE_VALUE;
  emul_event_file = INVALID_HANDLE_VALUE;
  
  // Clear error flags for HW device files (dma & hw events):
  m_rx_dma_err = 0;
  m_olink_event_err = m_dma_event_err = m_emul_event_err = 0;
  
  m_emul_module = false;

  m_max_timeout = 0ULL;
  
  //-------------------------  
  

  SYSTEM_INFO sys_info;
  GetSystemInfo(&sys_info);
  alignment = sys_info.dwPageSize;
  
  
  
  

  dma_buffers = nullptr;
  n_dma_buffers = 0;
  dma_buffer_size = 0;


  // DMA acquisition thread:
  m_dma_read_err = m_ififo_rd_err = m_ofifo_wr_err = 0;
  m_dma_timeout = 0;
  //dma_hw_enable(1);
  create_dma_thread();

  // HW event initialization:
  abort_event = nullptr;
  m_olink_event = 0;
  olink_event_hThread = nullptr;
  m_dma_event = 0;
  dma_event_hThread = nullptr;


  // Precalculate input performance factors:
  iperf_beat_size = SYS_FUSE_DMA_RX_AXIS_DWIDTH/8;
  iperf_clk_freq = (float) (SYS_FUSE_DMA_RX_AXIS_FREQ_MHZ * 1e6);
  iperf_clk_period = (float) (1.0/iperf_clk_freq);
  iperf_bw_factor = iperf_beat_size * iperf_clk_freq;

  // Open logging files:
  log_file = fopen("C://projects//deus_i_win_driver//logging.txt", "w");

}

fuse::~fuse() {
//  destroy_threads();
  destroy_dma_thread();
  destroy_dma_buffers();
  close_dev_files();

  if (log_file && (log_file != stderr)) {
    fclose(log_file);
  }
}

//-----

int fuse::open(int board_index) {
  int ret;
  ret = open_dev_files(board_index);
//  if(!ret) {
//    ret = create_threads();
//  }
  return ret;
}

int fuse::close() {
  int ret;
  ret = close_dev_files();
  return ret;
}

int fuse::abort() {
  BOOL ok;
  ok = ::SetEvent(abort_event);
  return 0;
}


int fuse::get_dev_path(std::wstring &dev_path, int index) {
  BOOL ok;
  int ret = -1;
  const GUID *guid = &GUID_DEVINTERFACE_XDMA;

  HDEVINFO dev_info = SetupDiGetClassDevs(guid, nullptr, nullptr, DIGCF_PRESENT | DIGCF_DEVICEINTERFACE);
  if (dev_info == INVALID_HANDLE_VALUE) {
    fprintf(err_file, "GetDevices INVALID_HANDLE_VALUE\n");
    return ret;
  }

  SP_DEVICE_INTERFACE_DATA dev_interface;
  dev_interface.cbSize = sizeof(SP_DEVICE_INTERFACE_DATA);

  // enumerate through devices
  do {
    ok = SetupDiEnumDeviceInterfaces(dev_info, nullptr, guid, index, &dev_interface);
    if (ok) {
      ULONG detail_size = 0;
      // get required buffer size
      ok = SetupDiGetDeviceInterfaceDetail(dev_info, &dev_interface, nullptr, 0, &detail_size, nullptr);
      if (!ok && GetLastError() != ERROR_INSUFFICIENT_BUFFER) {
        fprintf(err_file, "SetupDiGetDeviceInterfaceDetail - get length failed\n");
        break;
      }
      // allocate space for device interface detail
      SP_DEVICE_INTERFACE_DETAIL_DATA *dev_detail;
      dev_detail = (SP_DEVICE_INTERFACE_DETAIL_DATA *) new char[detail_size];
      if (!dev_detail) {
        fprintf(err_file, "HeapAlloc failed\n");
        break;
      }
      dev_detail->cbSize = sizeof(SP_DEVICE_INTERFACE_DETAIL_DATA);
      // get device interface detail
      ok = SetupDiGetDeviceInterfaceDetail(dev_info, &dev_interface, dev_detail, detail_size, nullptr, nullptr);
      if (!ok) {
        fprintf(err_file, "SetupDiGetDeviceInterfaceDetail - get detail failed\n");
      }
      else {
        dev_path = std::wstring(dev_detail->DevicePath);
        ret = 0;
      }
      delete[] dev_detail;
    }
  } while (0);

  SetupDiDestroyDeviceInfoList(dev_info);
  return 0;
}

int fuse::get_dev_path_table(std::vector<std::wstring> &dev_paths) {
  BOOL ret;
  const GUID *guid = &GUID_DEVINTERFACE_XDMA;

  HDEVINFO dev_info = SetupDiGetClassDevs(guid, nullptr, nullptr, DIGCF_PRESENT | DIGCF_DEVICEINTERFACE);
  if (dev_info == INVALID_HANDLE_VALUE) {
    fprintf(err_file, "GetDevices INVALID_HANDLE_VALUE\n");
    return -1;
  }

  SP_DEVICE_INTERFACE_DATA dev_interface;
  dev_interface.cbSize = sizeof(SP_DEVICE_INTERFACE_DATA);

  std::vector<std::wstring> device_paths;

  // enumerate through devices
  BOOL enum_ok;
  DWORD index;

  index = 0;
  enum_ok = SetupDiEnumDeviceInterfaces(dev_info, nullptr, guid, index, &dev_interface);
  while (enum_ok) {
    ULONG detail_size = 0;
    // get required buffer size
    // Primero obtengo el tamaño del buffer de detllamo a 'SetupDiGetDeviceInterfaceDetail'
    // pasandole un buffer nulo para obtener el tamaño del buffer
    ret = SetupDiGetDeviceInterfaceDetail(dev_info, &dev_interface, nullptr, 0, &detail_size, nullptr);
    if (!ret && GetLastError() != ERROR_INSUFFICIENT_BUFFER) {
      fprintf(err_file, "SetupDiGetDeviceInterfaceDetail - get length failed\n");
      break;
    }
    // allocate space for device interface detail
    SP_DEVICE_INTERFACE_DETAIL_DATA *dev_detail;
    dev_detail = (SP_DEVICE_INTERFACE_DETAIL_DATA *) new char[detail_size];
    if (!dev_detail) {
      fprintf(err_file, "HeapAlloc failed\n");
      break;
    }
    dev_detail->cbSize = sizeof(SP_DEVICE_INTERFACE_DETAIL_DATA);
    // get device interface detail
    ret = SetupDiGetDeviceInterfaceDetail(dev_info, &dev_interface, dev_detail, detail_size, nullptr, nullptr);
    if (!ret) {
      delete[] dev_detail;
      fprintf(err_file, "SetupDiGetDeviceInterfaceDetail - get detail failed\n");
      break;
    }

    std::wstring dev_path(dev_detail->DevicePath);
    dev_paths.emplace_back(dev_detail->DevicePath);
    delete[] dev_detail;

    index++;
    enum_ok = SetupDiEnumDeviceInterfaces(dev_info, nullptr, guid, index, &dev_interface);
  }
  SetupDiDestroyDeviceInfoList(dev_info);
  return 0;
}

int fuse::open_dev_files(int board_index) {
  int ret;
  std::wstring file_path;
  std::wstring dev_path;

  ret = get_dev_path(dev_path, board_index);
  if (ret < 0) {
    return -1;
  }

  // User file (used to get access to peripherals mapped on AXI-Lite bus):
  file_path = dev_path + L"\\user"; // XDMA_FILE_USER
  user_file = ::CreateFile(
                  file_path.c_str(),              // file name
                  GENERIC_READ | GENERIC_WRITE,   // access
                  0,                              // sharing
                  nullptr,                        // SECURITY_ATTRIBUTES
                  OPEN_EXISTING,                  // creation disposition
                  FILE_ATTRIBUTE_NORMAL,          // flags and attributes
                  nullptr);                       // template file

  if (user_file != INVALID_HANDLE_VALUE) {
    XDMA_BAR_INFO user_mmap_info;
    DWORD n_bytes;
    BOOL ok;

    ok = DeviceIoControl(
            user_file,              // hDevice
            IOCTL_MAP_BAR,          // I/O control code
            nullptr,                // Input buffer
            0,                      // Input buffer size
            &user_mmap_info,        // Output buffer
            sizeof(XDMA_BAR_INFO),  // Output buffer size
            &n_bytes,               // number of returned bytes
            nullptr);               // Overlapped
    if (ok) {
      user_mmap = user_mmap_info.mappedAddress;
      user_size = user_mmap_info.barLength;
    }

    USHORT SubSystemID;
    ok = DeviceIoControl(
            user_file,                    // hDevice
            IOCTL_FUSE_GET_SUBSYSTEM_ID,  // I/O control code
            nullptr,                      // Input buffer
            0,                            // Input buffer size
            &SubSystemID,                 // Output buffer
            sizeof(USHORT),               // Output buffer size
            &n_bytes,                     // number of returned bytes
            nullptr);                     // Overlapped

    if (ok) {
      m_emul_module = (SubSystemID = FUSE_DEVELOPMENT_SUBSYSTEM_ID) ? true : false;
    }
  }
  else {
    fprintf(err_file, "[%s] Error while opening device user file '%ls'\n", __FUNCTION__, file_path.c_str());
    return -1;
  }

  // DMA file, H2C channel 0:
  file_path = dev_path + L"\\c2h_0"; // XDMA_FILE_C2H_0
  c2h0_file = ::CreateFile(
                  file_path.c_str(),              // file name
                  GENERIC_READ,                   // access
                  0,                              // sharing
                  nullptr,                        // SECURITY_ATTRIBUTES
                  OPEN_EXISTING,                  // creation disposition
                  FILE_ATTRIBUTE_NORMAL,          // flags and attributes
                  nullptr);                       // template file

  if (c2h0_file == INVALID_HANDLE_VALUE) {
    fprintf(err_file, "[%s] Error while opening device Rx DMA file '%ls'\n", __FUNCTION__, file_path.c_str());
    return -1;
  }
  m_rx_dma_err = 0;
  get_file_timeout(c2h0_file, m_dma_timeout);

  // control file:
  file_path = dev_path + L"\\control"; // XDMA_FILE_CONTROL
  ctrl_file = ::CreateFile(
                  file_path.c_str(),              // file name
                  GENERIC_READ | GENERIC_WRITE,   // access
                  0,                              // sharing
                  nullptr,                        // SECURITY_ATTRIBUTES
                  OPEN_EXISTING,                  // creation disposition
                  FILE_ATTRIBUTE_NORMAL,          // flags and attributes
                  nullptr);                       // template file

  if (ctrl_file == INVALID_HANDLE_VALUE) {
    fprintf(err_file, "[%s] Error while opening device control file '%ls'\n", __FUNCTION__, file_path.c_str());
    return -1;
  }

  // Event file for the fuse DMA module:
  file_path = dev_path + L"\\event_" + std::to_wstring(SYS_FUSE_DMA_INT_ID);
  dma_event_file = ::CreateFile(
                        file_path.c_str(),      // file name
                        GENERIC_READ,           // access
                        0,                      // sharing
                        nullptr,                // SECURITY_ATTRIBUTES
                        OPEN_EXISTING,          // creation disposition
                        //FILE_FLAG_OVERLAPPED,
                        FILE_ATTRIBUTE_NORMAL,  // flags and attributes
                        nullptr);               // template file

  if (dma_event_file == INVALID_HANDLE_VALUE) {
    fprintf(err_file, "[%s] Error while opening fuse DMA event file '%ls'\n", __FUNCTION__, file_path.c_str());
    return -1;
  }
  m_dma_event_err = 0;

  // Event file for the optical link module:
  file_path = dev_path + L"\\event_" + std::to_wstring(SYS_OLINK_INT_ID);
  olink_event_file = ::CreateFile(
                        file_path.c_str(),      // file name
                        GENERIC_READ,           // access
                        0,                      // sharing
                        nullptr,                // SECURITY_ATTRIBUTES
                        OPEN_EXISTING,          // creation disposition
                        FILE_ATTRIBUTE_NORMAL,  // flags and attributes
                        nullptr);               // template file

  if (olink_event_file == INVALID_HANDLE_VALUE) {
    fprintf(err_file, "[%s] Error while opening optical link event file '%ls'\n", __FUNCTION__, file_path.c_str());
    return -1;
  }
  m_olink_event_err = 0;

  if (m_emul_module) {
    // Event file for the emulation module:
    file_path = dev_path + L"\\event_" + std::to_wstring(SYS_FUSE_EMUL_INT_ID);
    emul_event_file = ::CreateFile(
                          file_path.c_str(),      // file name
                          GENERIC_READ,           // access
                          0,                      // sharing
                          nullptr,                // SECURITY_ATTRIBUTES
                          OPEN_EXISTING,          // creation disposition
                          FILE_ATTRIBUTE_NORMAL,  // flags and attributes
                          nullptr);               // template file

    if (emul_event_file == INVALID_HANDLE_VALUE) {
      fprintf(err_file, "[%s] Error while opening fuse emulation event file '%ls'\n", __FUNCTION__, file_path.c_str());
      return -1;
    }
    m_emul_event_err = 0;
  }

  return 0;
}

int fuse::close_dev_files() {
  if (user_file != INVALID_HANDLE_VALUE) {
    // Unmap user file
    if (user_mmap != nullptr && user_size > 0) {
      user_mmap = nullptr;
      user_size = 0;
    }
    CloseHandle(user_file);
    user_file = INVALID_HANDLE_VALUE;
  }

  if (c2h0_file != INVALID_HANDLE_VALUE) {
    CloseHandle(c2h0_file);
    c2h0_file = INVALID_HANDLE_VALUE;
  }

  if (ctrl_file != INVALID_HANDLE_VALUE) {
    CloseHandle(ctrl_file);
    ctrl_file = INVALID_HANDLE_VALUE;
  }

  // Device event (interrupt) files:
  if (dma_event_file != INVALID_HANDLE_VALUE) {
    CloseHandle(dma_event_file);
    dma_event_file = INVALID_HANDLE_VALUE;
  }

  if (olink_event_file != INVALID_HANDLE_VALUE) {
    CloseHandle(user_file);
    olink_event_file = INVALID_HANDLE_VALUE;
  }

  if (emul_event_file != INVALID_HANDLE_VALUE) {
    CloseHandle(emul_event_file);
    emul_event_file = INVALID_HANDLE_VALUE;
  }

  return 0;
}

// *****************************************
// *** HW EVENTS (INTERRUPTS) MANAGEMENT ***
// *****************************************

inline int fuse::_get_hw_event(void *hw_event_file, fuse_event_t &local_hw_event, fuse_event_t &e, int clear) {
  int ret = -1;
  if (m_use_event_thrd) {
    if (hw_event_file != INVALID_HANDLE_VALUE) {
      LONG mask = (clear)? 0U : ~0U;
      e = ::InterlockedAnd((LONG *)&local_hw_event, mask);
      ret = 0;
    }
  }
  return ret;
}

inline int fuse::_get_hw_event(void *hw_event_file, int &hw_event_err, fuse_event_t &e) {
  DWORD n_bytes;
  BOOL ok;
  int ret = -1;
  if (!m_use_event_thrd) {
    ok = ReadFile(hw_event_file, &e, sizeof(fuse_event_t), &n_bytes, NULL);
    ret = (ok)? n_bytes : -1;
    hw_event_err |= (ok)? 0 : 1;
  }
  return ret;
}

inline int fuse::_enable_hw_event(DWORD service_id, fuse_event_t mask) {
  BOOL ok;
  ok = DeviceIoControl(
          user_file,              // hDevice
          service_id,             // I/O control code
          &mask,                  // Input buffer
          sizeof(fuse_event_t),   // Input buffer size
          nullptr,                // Output buffer
          0,                      // Output buffer size
          nullptr,                // number of returned bytes
          nullptr);               // Overlapped

  return (ok) ? 0 : -1;
}


// Optical link:

DWORD fuse::olink_event_thread() {
  int ret;
  fuse_event_t event;
  DWORD n_bytes;
  BOOL ok;
  bool abort;

  dbg_printf("[%s] thread +\n", __FUNCTION__);
  while (1) {
    abort = (::WaitForSingleObject(abort_event, 0) == WAIT_OBJECT_0);
    if(abort) {
      dbg_printf("[%s] aborting thread\n", __FUNCTION__);
      ret = 0;
      break;
    }
    ok = ReadFile(olink_event_file, &event, sizeof(fuse_event_t), &n_bytes, NULL);
    if (!ok) {
      dbg_printf("[%s] read error\n", __FUNCTION__);
      m_olink_event_err = 1;
    }
    else {
      if (n_bytes == sizeof(fuse_event_t)) {
        InterlockedOr((volatile LONG *)&m_olink_event, event);
      }
    }
  }

  dbg_printf("[%s] thread -\n", __FUNCTION__);
  return ret;
}

int fuse::get_event_olink(fuse_event_t &e, int clear) {
  int ret = -1;
  if (m_use_event_thrd) {
    if (olink_event_file != INVALID_HANDLE_VALUE) {
      LONG mask = (clear)? 0U : ~0U;
      e = ::InterlockedAnd((LONG *)&m_olink_event, mask);
      ret = 0;
    }
  }
  return ret;
}

int fuse::get_event_olink(fuse_event_t &e) {
  DWORD n_bytes;
  BOOL ok;
  int ret = -1;
  if (!m_use_event_thrd) {
    ok = ReadFile(olink_event_file, &e, sizeof(fuse_event_t), &n_bytes, NULL);
    ret = (ok)? n_bytes : -1;
    m_olink_event_err |= (char) !ok;
  }
  return ret;
}

int fuse::enable_event_olink(fuse_event_t mask) {
  BOOL ok;
  ok = DeviceIoControl(
          user_file,                        // hDevice
          IOCTL_FUSE_OLINK_ENABLE_EVENTS,   // I/O control code
          &mask,                            // Input buffer
          sizeof(fuse_event_t),             // Input buffer size
          nullptr,                          // Output buffer
          0,                                // Output buffer size
          nullptr,                          // number of returned bytes
          nullptr);                         // Overlapped

  return (ok) ? 0 : -1;
}

int fuse::get_event_err_olink(int &err) {
  err = ::InterlockedExchange8(&m_olink_event_err, 0);
  return 0;
}

int fuse::olink_get_aurora_status(uint32_t &status) {
  DWORD n_bytes;
  BOOL ok;

  ok = DeviceIoControl(
    user_file,                        // hDevice
    IOCTL_FUSE_OLINK_GET_CONN_STATUS, // I/O control code
    nullptr,                          // Input buffer
    0,                                // Input buffer size
    &status,                          // Output buffer
    sizeof(uint32_t),                 // Output buffer size
    &n_bytes,                         // number of returned bytes
    nullptr);                         // Overlapped

  return (ok && (n_bytes == sizeof(uint32_t))) ? 0 : -1;
}

#define OLNK_GET_LANE_STATUS(x) ((x) & 0xFF)
#define OLNK_AURORA_CHANNEL_UP  0x00000100
#define OLNK_AURORA_SOFT_ERR    0x00000200
#define OLNK_AURORA_HARD_ERR    0x00000400
#define OLNK_AURORA_GT_PLL_LOCK 0x00000800

int fuse::olink_get_lane_status(uint32_t &lane_status) {
  uint32_t aurora_status;
  int ret;
  ret = olink_get_aurora_status(aurora_status);
  lane_status = OLNK_GET_LANE_STATUS(aurora_status);
  return ret;
}
int fuse::olink_get_channel_err(int &err) {
  uint32_t aurora_status;
  int ret;
  ret = olink_get_aurora_status(aurora_status);
  err = !(aurora_status & OLNK_AURORA_CHANNEL_UP);
  return ret;
}

int fuse::olink_get_soft_err(int &err) {
  uint32_t aurora_status;
  int ret;
  ret = olink_get_aurora_status(aurora_status);
  err = (aurora_status & OLNK_AURORA_SOFT_ERR);
  return ret;
}

int fuse::olink_get_hard_err(int &err) {
  uint32_t aurora_status;
  int ret;
  ret = olink_get_aurora_status(aurora_status);
  err = (aurora_status & OLNK_AURORA_HARD_ERR);
  return ret;
}

int fuse::olink_get_pll_lock(int &lock) {
  uint32_t aurora_status;
  int ret;
  ret = olink_get_aurora_status(aurora_status);
  lock = (aurora_status & OLNK_AURORA_GT_PLL_LOCK);
  return ret;
}




//  aurora_status_reg(7 downto 0) <= lane_status;
//  aurora_status_reg(8)  <= channel_up;
//  aurora_status_reg(9)  <= soft_err;
//  aurora_status_reg(10) <= hard_err;
//  aurora_status_reg(11) <= gt_pll_lock;



// Fuse DMA:

DWORD fuse::dma_event_thread() {
  int ret;
  fuse_event_t event;
  DWORD n_bytes;
  BOOL ok;
  bool abort;

  dbg_printf("[%s] thread +\n", __FUNCTION__);
  while (1) {
    abort = (::WaitForSingleObject(abort_event, 0) == WAIT_OBJECT_0);
    if(abort) {
      dbg_printf("[%s] aborting thread\n", __FUNCTION__);
      ret = 0;
      break;
    }
    event = 0;
    ok = ReadFile(dma_event_file, &event, sizeof(fuse_event_t), &n_bytes, NULL);
    if (!ok) {
      dbg_printf("[%s] read error\n", __FUNCTION__);
      m_dma_event_err = 1;
    }
    else {
      if (n_bytes == sizeof(fuse_event_t)) {
        ::InterlockedOr((LONG *)&m_dma_event, (LONG)event);
      }
    }
  }

  dbg_printf("[%s] thread -\n", __FUNCTION__);
  return ret;
}

int fuse::get_event_dma(fuse_event_t  &e, int clear) {
  int ret = -1;
  if (m_use_event_thrd) {
    if (dma_event_file != INVALID_HANDLE_VALUE) {
      LONG mask = (clear)? 0U : ~0U;
      e = ::InterlockedAnd((LONG *)&m_dma_event, mask);
      ret = 0;
    }
  }
  return ret;
}

int fuse::get_event_dma(fuse_event_t &e) {
  DWORD n_bytes;
  BOOL ok;
  int ret = -1;
  if (!m_use_event_thrd) {
    ok = ReadFile(dma_event_file, &e, sizeof(fuse_event_t), &n_bytes, NULL);
    ret = (ok)? n_bytes : -1;
    m_dma_event_err |= (char) !ok;
  }
  return ret;
}

int fuse::enable_event_dma(fuse_event_t mask) {
  BOOL ok;
  ok = DeviceIoControl(
          user_file,                        // hDevice
          IOCTL_FUSE_DMA_ENABLE_EVENTS,     // I/O control code
          &mask,                            // Input buffer
          sizeof(fuse_event_t),             // Input buffer size
          nullptr,                          // Output buffer
          0,                                // Output buffer size
          nullptr,                          // number of returned bytes
          nullptr);                         // Overlapped

  return (ok) ? 0 : -1;
}

int fuse::get_event_err_dma(int &err) {
  err = ::InterlockedExchange8(&m_dma_event_err, 0);
  return 0;
}

// Fuse emulator:

int fuse::get_event_emul(fuse_event_t &e, int clear)  {
  int ret = -1;
  if (m_use_event_thrd) {
    if (emul_event_file != INVALID_HANDLE_VALUE) {
      LONG mask = (clear)? 0U : ~0U;
      e = ::InterlockedAnd((LONG *)&m_emul_event, mask);
      ret = 0;
    }
  }
  return ret;
}

int fuse::get_event_emul(fuse_event_t &e) {
  DWORD n_bytes;
  BOOL ok;
  int ret = -1;
  if (!m_use_event_thrd) {
    ok = ReadFile(emul_event_file, &e, sizeof(fuse_event_t), &n_bytes, NULL);
    ret = (ok)? n_bytes : -1;
    m_emul_event_err |= (char) !ok;
  }
  return ret;
}


int fuse::enable_event_emul(fuse_event_t mask) {
  BOOL ok;
  ok = DeviceIoControl(
          user_file,                        // hDevice
          IOCTL_FUSE_EMUL_ENABLE_EVENTS,    // I/O control code
          &mask,                            // Input buffer
          sizeof(fuse_event_t),             // Input buffer size
          nullptr,                          // Output buffer
          0,                                // Output buffer size
          nullptr,                          // number of returned bytes
          nullptr);                         // Overlapped

  return (ok) ? 0 : -1;
}

int fuse::get_event_err_emul(int &err) {
  err = ::InterlockedExchange8(&m_emul_event_err, 0);
  return 0;
}


// SERVICE FUNCTIONS: FILE TIMEOUT

int fuse::get_file_timeout(void *file, timeout_t &timeout) const {
  int ret;
  if (file != INVALID_HANDLE_VALUE) {
    DWORD n_bytes;
    BOOL ok;
    ok = DeviceIoControl(
            file,                             // hDevice
            IOCTL_FUSE_GET_FILE_TIMEOUT,      // I/O control code
            nullptr,                          // Input buffer
            0,                                // Input buffer size
            &timeout,                         // Output buffer
            sizeof(timeout_t),                // Output buffer size
            &n_bytes,                         // number of returned bytes
            nullptr);                         // Overlapped
    ret = (ok && (n_bytes == sizeof(timeout_t))) ? 0 : -1;
  }
  else {
    ret = -1;
  }
  return ret;
}

int fuse::set_file_timeout(void *file, timeout_t timeout) {
  int ret;
  if (file != INVALID_HANDLE_VALUE) {
    BOOL ok;
    ok = DeviceIoControl(
            file,                             // hDevice
            IOCTL_FUSE_SET_FILE_TIMEOUT,      // I/O control code
            &timeout,                         // Input buffer
            sizeof(timeout_t),                // Input buffer size
            nullptr,                          // Output buffer
            0,                                // Output buffer size
            nullptr,                          // number of returned bytes
            nullptr);                         // Overlapped
    if(ok) {
      m_max_timeout = (timeout > m_max_timeout)? timeout : m_max_timeout;
    }
    ret = (ok) ? 0 : -1;
  }
  else {
    ret = -1;
  }
  return ret;
}

int fuse::cancel_event_file(void *file, uint32_t mode) {
  int ret;
  //return 0;
  if (file != INVALID_HANDLE_VALUE) {
    BOOL ok;
    //uint32_t mode = 0x1234;
    ok = DeviceIoControl(
            file,                             // hDevice
            IOCTL_FUSE_CANCEL_USR_EVENT,      // I/O control code
            &mode,                            // Input buffer
            sizeof(mode),                     // Input buffer size
            nullptr,                          // Output buffer
            0,                                // Output buffer size
            nullptr,                          // number of returned bytes
            nullptr);                         // Overlapped

    ret = (ok) ? 0 : -1;
  }
  else {
    ret = -1;
  }
  return ret;
}

// SERVICE FUNCTIONS: I2C COMMS

int fuse::i2c_rst_brd_mux(int rst) {
  BOOL ok;

  ok = DeviceIoControl(
          user_file,                        // hDevice
          IOCTL_FUSE_OLINK_I2C_RST_BRD_MUX, // I/O control code
          &rst,                             // Input buffer
          sizeof(rst),                      // Input buffer size
          nullptr,                          // Output buffer
          0,                                // Output buffer size
          nullptr,                          // number of returned bytes
          nullptr);                         // Overlapped

  return (ok) ? 0 : -1;
}

int fuse::i2c_send_byte(uint8_t byte, int trf_attr) {
  BOOL ok;
  DWORD n_bytes = 0;
  _fuse_i2c_cmd i2c_cmd;

  i2c_cmd.trf_attr = 0;
  i2c_cmd.trf_attr |= (trf_attr & I2C_TRF_ATTR_START)? _FUSE_I2C_START : 0;
  i2c_cmd.trf_attr |= (trf_attr & I2C_TRF_ATTR_STOP)? _FUSE_I2C_STOP : 0;
  i2c_cmd.trf_attr |= (trf_attr & I2C_TRF_ATTR_NACK)? _FUSE_I2C_NACK : 0;
  i2c_cmd.byte = byte;

  ok = DeviceIoControl(
          user_file,                        // hDevice
          IOCTL_FUSE_OLINK_I2C_SEND_BYTE,   // I/O control code
          &i2c_cmd,                         // Input buffer
          sizeof(_fuse_i2c_cmd),            // Input buffer size
          nullptr,                          // Output buffer
          0,                                // Output buffer size
          &n_bytes,                         // number of returned bytes
          nullptr);                         // Overlapped

  return (ok) ? n_bytes : -1;
}

int fuse::i2c_recv_byte(uint8_t *byte, int trf_attr) {
  BOOL ok;
  DWORD n_bytes;
  int ret;

  _fuse_i2c_cmd i2c_cmd;
  _fuse_i2c_cmd i2c_resp;

  i2c_cmd.trf_attr = 0;
  i2c_cmd.trf_attr |= (trf_attr & I2C_TRF_ATTR_START)? _FUSE_I2C_START : 0;
  i2c_cmd.trf_attr |= (trf_attr & I2C_TRF_ATTR_STOP)? _FUSE_I2C_STOP : 0;
  i2c_cmd.trf_attr |= (trf_attr & I2C_TRF_ATTR_NACK)? _FUSE_I2C_NACK : 0;
  i2c_cmd.byte = 0;

  ok = DeviceIoControl(
          user_file,                        // hDevice
          IOCTL_FUSE_OLINK_I2C_RECV_BYTE,   // I/O control code
          &i2c_cmd,                         // Input buffer
          sizeof(_fuse_i2c_cmd),            // Input buffer size
          &i2c_resp,                        // Output buffer
          sizeof(_fuse_i2c_cmd),            // Output buffer size
          &n_bytes,                         // number of returned bytes
          nullptr);                         // Overlapped

  *byte = i2c_resp.byte;
  ret = (ok && (n_bytes == sizeof(_fuse_i2c_cmd))) ? 1 : -1;
  return ret;
}

// SERVICE FUNCTIONS: FUSE DMA MODULE CONTROL

#if 0
int fuse::dma_hw_reset(uint32_t mode) {
  BOOL ok;

  uint32_t m = 0;
  m |= (mode & FUSE_HW_RST_DMA) ? _FUSE_DMA_RST_CORE : 0;
  m |= (mode & FUSE_HW_RST_MEM) ? _FUSE_DMA_RST_MEM  : 0;
  
  return do_ip_hw_reset(int service, m);
  

  ok = DeviceIoControl(
          user_file,                        // hDevice
          IOCTL_FUSE_DMA_RESET_HW,          // I/O control code
          &m,                               // Input buffer
          sizeof(uint32_t),                 // Input buffer size
          nullptr,                          // Output buffer
          0,                                // Output buffer size
          nullptr,                          // number of returned bytes
          nullptr);                         // Overlapped

  return (ok) ? 0 : -1;
}

int fuse::dma_hw_enable(int enable) {
  BOOL ok;
  ok = DeviceIoControl(
    user_file,                        // hDevice
    IOCTL_FUSE_DMA_ENABLE,            // I/O control code
    &enable,                          // Input buffer
    sizeof(int32_t),                  // Input buffer size
    nullptr,                          // Output buffer
    0,                                // Output buffer size
    nullptr,                          // number of returned bytes
    nullptr);                         // Overlapped

  return (ok) ? 0 : -1;
}
#endif

int fuse::dma_hw_flush(uint32_t mode) {
  BOOL ok;

  ok = DeviceIoControl(
    user_file,                        // hDevice
    IOCTL_FUSE_DMA_FLUSH,             // I/O control code
    &mode,                            // Input buffer
    sizeof(uint32_t),                 // Input buffer size
    nullptr,                          // Output buffer
    0,                                // Output buffer size
    nullptr,                          // number of returned bytes
    nullptr);                         // Overlapped

  return (ok) ? 0 : -1;
}

//-----
int fuse::dma_hw_get_filling_level(unsigned int &n) {
  unsigned int level;
  DWORD n_bytes;
  BOOL ok;

  ok = DeviceIoControl(
    user_file,                        // hDevice
    IOCTL_FUSE_DMA_GET_FILLING_LEVEL, // I/O control code
    nullptr,                          // Input buffer
    0,                                // Input buffer size
    &level,                           // Output buffer
    sizeof(unsigned int),             // Output buffer size
    &n_bytes,                         // number of returned bytes
    nullptr);                         // Overlapped

  n = level;
  return (ok && (n_bytes == sizeof(unsigned int))) ? 0 : -1;
}


int fuse::dma_hw_get_filling_thrs(unsigned int &thr_l, unsigned int &thr_h) {
  level_thr_t level;
  DWORD n_bytes;
  BOOL ok;

  ok = DeviceIoControl(
    user_file,                        // hDevice
    IOCTL_FUSE_DMA_GET_FILLING_THRS,  // I/O control code
    nullptr,                          // Input buffer
    0,                                // Input buffer size
    &level,                           // Output buffer
    sizeof(level_thr_t),              // Output buffer size
    &n_bytes,                         // number of returned bytes
    nullptr);                         // Overlapped

  thr_l = level.thr_l;
  thr_h = level.thr_h;

  return (ok && (n_bytes == sizeof(level_thr_t))) ? 0 : -1;
}

int fuse::dma_hw_set_filling_thrs(unsigned int thr_l, unsigned int thr_h) {
  level_thr_t level;
  BOOL ok;

  level.thr_l = thr_l;
  level.thr_h = thr_h;

  ok = DeviceIoControl(
    user_file,                        // hDevice
    IOCTL_FUSE_DMA_SET_FILLING_THRS,  // I/O control code
    &level,                           // Input buffer
    sizeof(level_thr_t),              // Input buffer size
    nullptr,                          // Output buffer
    0,                                // Output buffer size
    nullptr,                          // number of returned bytes
    nullptr);                         // Overlapped

  return (ok) ? 0 : -1;
}

//-----

int fuse::dma_hw_get_iperf_cycles(unsigned int &total, unsigned int &active) {
  fuse_perf_data iperf;
  DWORD n_bytes;
  BOOL ok;

  ok = DeviceIoControl(
    user_file,                        // hDevice
    IOCTL_FUSE_DMA_GET_IPERF_DATA,    // I/O control code
    nullptr,                          // Input buffer
    0,                                // Input buffer size
    &iperf,                           // Output buffer
    sizeof(fuse_perf_data),           // Output buffer size
    &n_bytes,                         // number of returned bytes
    nullptr);                         // Overlapped

  total = iperf.total_cycles;
  active = iperf.active_cycles;

  return (ok && (n_bytes == sizeof(fuse_perf_data))) ? 0 : -1;
}

int fuse::dma_hw_get_iperf_time(float &total, float &active) {
  int ret;
  unsigned int n_total;
  unsigned int n_beats;
  ret = dma_hw_get_iperf_cycles(n_total, n_beats);
  total = iperf_clk_period * n_total;
  active = iperf_clk_period * n_beats;
  return 0;
}

int fuse::dma_hw_get_iperf_BW(float &bw, unsigned int n_gap) {
  int ret;
  unsigned int n_total, cycles;
  unsigned int n_beats;

  ret = dma_hw_get_iperf_cycles(n_total, n_beats);
  cycles = n_total+n_gap;
  if (cycles) {
    bw = iperf_bw_factor * ((float)n_beats)/cycles;
    ret = 0;
  }
  else {
    bw = 0;
    ret = -1;
  }
  return ret;
}

int fuse::dma_hw_get_image_id_seed(unsigned int &id) {
  DWORD n_bytes;
  BOOL ok;

  ok = DeviceIoControl(
    user_file,                        // hDevice
    IOCTL_FUSE_DMA_GET_IMAGE_ID_SEED, // I/O control code
    nullptr,                          // Input buffer
    0,                                // Input buffer size
    &id,                              // Output buffer
    sizeof(unsigned int),             // Output buffer size
    &n_bytes,                         // number of returned bytes
    nullptr);                         // Overlapped

  return (ok && (n_bytes == sizeof(unsigned int))) ? 0 : -1;
}

int fuse::dma_hw_set_image_id_seed(unsigned int id) {
  BOOL ok;

  ok = DeviceIoControl(
    user_file,                        // hDevice
    IOCTL_FUSE_DMA_SET_IMAGE_ID_SEED, // I/O control code
    &id,                              // Input buffer
    sizeof(unsigned int),             // Output buffer size
    nullptr,                          // Output buffer
    0,                                // Output buffer size
    nullptr,                          // number of returned bytes
    nullptr);                         // Overlapped

  return (ok) ? 0 : -1;
}

int fuse::dma_hw_get_mem_size(unsigned int &mem_size_mb) {
  DWORD n_bytes;
  BOOL ok;

  ok = DeviceIoControl(
    user_file,                        // hDevice
    IOCTL_FUSE_DMA_GET_MEM_SIZE,      // I/O control code
    nullptr,                          // Input buffer
    0,                                // Input buffer size
    &mem_size_mb,                     // Output buffer
    sizeof(unsigned int),             // Output buffer size
    &n_bytes,                         // number of returned bytes
    nullptr);                         // Overlapped

  return (ok && (n_bytes == sizeof(unsigned int))) ? 0 : -1;
}

int fuse::dma_hw_dna_ok() {
  DWORD n_bytes;
  BOOL ok;
  int dna_ok;

  ok = DeviceIoControl(
    user_file,                        // hDevice
    IOCTL_FUSE_DMA_DNA_OK,            // I/O control code
    nullptr,                          // Input buffer
    0,                                // Input buffer size
    &dna_ok,                          // Output buffer
    sizeof(int),                      // Output buffer size
    &n_bytes,                         // number of returned bytes
    nullptr);                         // Overlapped

  return (ok && (n_bytes == sizeof(unsigned int))) ? dna_ok : -1;
}


//---

int fuse::get_ip_signature(int service, uint32_t &id) {
  DWORD n_bytes;
  BOOL ok;

  ok = DeviceIoControl(
    user_file,            // hDevice
    service,              // I/O control code
    nullptr,              // Input buffer
    0,                    // Input buffer size
    &id,                  // Output buffer
    sizeof(uint32_t),     // Output buffer size
    &n_bytes,             // number of returned bytes
    nullptr);             // Overlapped

  return (ok && (n_bytes == sizeof(uint32_t))) ? 0 : -1;
}

int fuse::olink_get_signature(uint32_t &id) {
  return get_ip_signature(IOCTL_FUSE_OLINK_GET_SIGNATURE, id);
}

int fuse::dma_get_signature(uint32_t &id) {
  return get_ip_signature(IOCTL_FUSE_DMA_GET_SIGNATURE, id);
}

int fuse::emul_get_signature(uint32_t &id) {
  return get_ip_signature(IOCTL_FUSE_EMUL_GET_SIGNATURE, id);
}

//  version_reg( 3 downto  0) <= REVISION;
//  version_reg( 7 downto  4) <= MINOR_VERSION;
//  version_reg(11 downto  8) <= MAJOR_VERSION;
//  version_reg(15 downto 12) <= x"0";
//  version_reg(31 downto 16) <= MAGIC_NUMBER;

#define GET_IP_REVISION(x)      (((x) >>  0) & 0xF)
#define GET_IP_MINOR_VERSION(x) (((x) >>  4) & 0xF)
#define GET_IP_MAJOR_VERSION(x) (((x) >>  8) & 0xF)
#define GET_IP_MAGIC_NUMBER(x)  (((x) >> 16) & 0xFFFF)

int fuse::olink_get_signature(int &version, int &subversion, int &revision, int &magic_number) {
  uint32_t id;
  int ret;
  ret = olink_get_signature(id);
  version = GET_IP_MAJOR_VERSION(id);
  subversion = GET_IP_MINOR_VERSION(id);
  revision = GET_IP_REVISION(id);
  magic_number = GET_IP_MAGIC_NUMBER(id);
  return ret;
}

int fuse::dma_get_signature(int &version, int &subversion, int &revision, int &magic_number) {
  uint32_t id;
  int ret;
  ret = dma_get_signature(id);
  version = GET_IP_MAJOR_VERSION(id);
  subversion = GET_IP_MINOR_VERSION(id);
  revision = GET_IP_REVISION(id);
  magic_number = GET_IP_MAGIC_NUMBER(id);
  return ret;
}

int fuse::emul_get_signature(int &version, int &subversion, int &revision, int &magic_number) {
  uint32_t id;
  int ret;
  ret = emul_get_signature(id);
  if (!ret) {
    version = GET_IP_MAJOR_VERSION(id);
    subversion = GET_IP_MINOR_VERSION(id);
    revision = GET_IP_REVISION(id);
    magic_number = GET_IP_MAGIC_NUMBER(id);
  }
  else {
    version = 0;
    subversion = 0;
    revision = 0;
    magic_number = -1;
  }
  return ret;
}


//---

int fuse::do_ip_hw_reset(int service, uint32_t mode) {
  BOOL ok;
  ok = DeviceIoControl(
          user_file,          // hDevice
          service,            // I/O control code
          &mode,              // Input buffer
          sizeof(uint32_t),   // Input buffer size
          nullptr,            // Output buffer
          0,                  // Output buffer size
          nullptr,            // number of returned bytes
          nullptr);           // Overlapped
  return (ok) ? 0 : -1;
}

int fuse::olink_hw_reset() {
  return do_ip_hw_reset(IOCTL_FUSE_OLINK_RESET_HW, 0);
}

int fuse::dma_hw_init() {
  int ret;
  uint32_t mode;
  // 1. Reseteo el core y la memoria
  mode = _FUSE_DMA_RST_CFG | _FUSE_DMA_RST_OP_SET | (FUSE_HW_RST_DMA | _FUSE_DMA_RST_MEM);
  ret = do_ip_hw_reset(IOCTL_FUSE_DMA_RESET_HW, mode);
  Sleep(1);
  // 2. Saco al controlador de memoria de la condición de reset:
  mode = _FUSE_DMA_RST_CFG | _FUSE_DMA_RST_OP_CLR | _FUSE_DMA_RST_MEM;
  ret = do_ip_hw_reset(IOCTL_FUSE_DMA_RESET_HW, mode);
  // 3. Espero más tiempo para que se estabilice:
  Sleep(50);
  // 4. Saco al controlador de DMA de la condición de reset:
  mode = _FUSE_DMA_RST_CFG | _FUSE_DMA_RST_OP_CLR | _FUSE_DMA_RST_MEM;
  ret = do_ip_hw_reset(IOCTL_FUSE_DMA_RESET_HW, mode);
  Sleep(1);
  return 0;
}

int fuse::dma_hw_reset(uint32_t mode) {
  uint32_t m = 0;
  m |= (mode & FUSE_HW_RST_DMA) ? _FUSE_DMA_RST_CORE : 0;
  m |= (mode & FUSE_HW_RST_MEM) ? _FUSE_DMA_RST_MEM  : 0;
  return do_ip_hw_reset(IOCTL_FUSE_DMA_RESET_HW, m);
}

int fuse::emul_hw_reset() {
  return do_ip_hw_reset(IOCTL_FUSE_EMUL_RESET_HW, 0);
}

//---

int fuse::do_ip_hw_enable(int service, int x) {
  BOOL ok;
  ok = DeviceIoControl(
          user_file,          // hDevice
          service,            // I/O control code
          &x,                 // Input buffer
          sizeof(x),          // Input buffer size
          nullptr,            // Output buffer
          0,                  // Output buffer size
          nullptr,            // number of returned bytes
          nullptr);           // Overlapped
  return (ok) ? 0 : -1;
}

int fuse::olink_hw_enable(int x) {
  return do_ip_hw_reset(IOCTL_FUSE_OLINK_ENABLE, x);
}

int fuse::dma_hw_enable(int x) {
  return do_ip_hw_reset(IOCTL_FUSE_DMA_ENABLE, x);
}

int fuse::emul_hw_enable(int x) {
  return do_ip_hw_reset(IOCTL_FUSE_EMUL_ENABLE, x);
}

//---


int fuse::get_pcie_info(uint16_t &subsys_id, uint8_t &rev_id) {
  pci_info_t pci_info;
  DWORD n_bytes;
  BOOL ok;

  ok = DeviceIoControl(
    user_file,                        // hDevice
    IOCTL_FUSE_GET_PCI_INFO,          // I/O control code
    nullptr,                          // Input buffer
    0,                                // Input buffer size
    &pci_info,                        // Output buffer
    sizeof(pci_info_t),               // Output buffer size
    &n_bytes,                         // number of returned bytes
    nullptr);                         // Overlapped

  subsys_id = pci_info.SubSystemID;
  rev_id = pci_info.RevisionID;
    
  return (ok && (n_bytes == sizeof(pci_info_t))) ? 0 : -1;
}


// **********************************
// *** HW IMAGE SUPPORT FUNCTIONS ***
// **********************************

#define FRAMES_PER_PACKAGE  4
#define HEADER_LEN          1

size_t fuse::calc_hw_image_size(int frame_len, int image_len) const {
  size_t image_size = (image_len * FRAMES_PER_PACKAGE * frame_len * iperf_beat_size) + HEADER_LEN * iperf_beat_size;
  return image_size;
}

int fuse::calc_hw_image_gap(int frame_len, int image_len, uint64_t BW) const {
  size_t image_size;
  int gap_cycles;

  if (BW > iperf_bw_factor) {
    gap_cycles = 0;
  }
  else {
    image_size = (image_len * FRAMES_PER_PACKAGE * frame_len * iperf_beat_size) + HEADER_LEN * iperf_beat_size;
    gap_cycles = (int)(image_size * ((iperf_clk_freq / BW) - (1.0 / iperf_beat_size)));
  }
  return gap_cycles;
}

int fuse::get_hw_image_info(const void *img, int *id, int *frame_len, int *image_len) {
  const fuse_header_t *hdr = (const fuse_header_t *) img;
  int ret;

  if (hdr->magic_num == FUSE_MAGIC_NUM) {
    if (id != nullptr)
      *id = hdr->image_id;
    if (frame_len != nullptr)
      *frame_len = hdr->frame_len + 1;
    if (image_len != nullptr)
      *image_len = hdr->image_len + 1;
    ret = 0;
  }
  else {
    ret = -1;
  }
  return ret;
}

#ifndef WORD32_PER_BEAT
#define WORD32_PER_BEAT (SYS_FUSE_DMA_RX_AXIS_DWIDTH/32)
#endif

int fuse::hw_image_check(const void *img) {
  const uint32_t *data, *p;
  const fuse_header_t *hdr = (const fuse_header_t *)img;
  int pkt, frm, beat, word;
  int i, err;
  union fuse_data pattern;
  int n_beats;      // Number of beats per frame
  int n_packets;    // Number of packets per image

  if (hdr->magic_num != FUSE_MAGIC_NUM) {
    dbg_printf("[%s] Image error: magic number = 0x%8.8X\n", __FUNCTION__, hdr->magic_num);
    return -1;
  }

  n_beats = hdr->frame_len + 1;
  n_packets = hdr->image_len + 1;

  //dbg_printf("[%s] Image ID = %d\n", __FUNCTION__, hdr->image_id);
  //printf("[%s] Image ID = %d\n", __FUNCTION__, hdr->image_id);

  err = 0;
  p = data = (const uint32_t *)(((const uint8_t *)img) + sizeof(fuse_header_t));
  i = 0;
  pattern.word32 = 0;
  for (pkt = 0; pkt < n_packets; pkt++) {
    for (frm = 0; frm < FRAMES_PER_PACKAGE; frm++) {
      for (beat = 0; beat < n_beats; beat++) {
        for (word = 0; word < WORD32_PER_BEAT; word++) {
          pattern.fields.word_idx = word;
          pattern.fields.beat_idx = beat;
          pattern.fields.frame_idx = frm;
          pattern.fields.packet_idx = pkt;
          if (*p != pattern.word32) {
            err = -1;
            return err;
          }
          i++;
          p++;
        }
      }
    }
  }
  return err;
}

// ***************************************
// *** DMA operation control functions ***
// ***************************************

int fuse::create_dma_buffers(int n_buffers, size_t size) {
  dma_buffers = new dma_buffer_t[n_buffers];
  int i;
  n_dma_buffers = n_buffers;
  dma_buffer_size = size;

  //printf("[%s] requesting %d n_buffers of %.2f MB\n", __FUNCTION__, n_buffers, (float)(size*(1.0 / (1U << 20))));

  for (i = 0; i < n_dma_buffers; i++) {
#ifdef DMA_BUF_NEW_OP
    dma_buffers[i] = new (std::align_val_t(alignment)) uint8_t[size];
#else
    dma_buffers[i] = static_cast<dma_buffer_t> (_aligned_malloc(size, alignment));
#endif
  }
  return 0;
}

int fuse::destroy_dma_buffers() {
  int i;
  if (dma_buffers != nullptr) {
    for (i = 0; i < n_dma_buffers; i++) {
      if (dma_buffers[i] != nullptr) {
        _aligned_free(dma_buffers[i]);
        dma_buffers[i] = nullptr;
      }
    }
    delete[] dma_buffers;
    dma_buffers = nullptr;
    n_dma_buffers = 0;
    dma_buffer_size = 0;
  }
  return 0;
}

int fuse::get_dma_buffer(dma_buffer_t *bufs, int n) {
  return ofifo.nb_get(bufs, n);
}

int fuse::free_dma_buffer(dma_buffer_t b) {
  bool ok;
  ok = ififo.nb_put(b);

  if (!ok) {
    int __stop = 1;
  }

  // Deberíamos considerar el flag de usado de la cabecera???
  return (ok)? 0 : -1;
}

int fuse::dma_buffer_rdy() {
  return ofifo.used();
}

int fuse::wait_for_dma_buffer(int n, int timeout_ms) {
  int ret;
  ret = ofifo.wait_for_used(n, timeout_ms);
  return ret;
}

//-----

int fuse::dma_is_running() {
  bool running;
  running = (::WaitForSingleObject(dma_acq_cfg.start_event, 0) == WAIT_OBJECT_0);
  return (running)? 1 : 0;
}

int fuse::dma_start(hw_size_t n_hw_img, int hw_frame_len, int hw_image_len) {
  int running;
  int ret;
  BOOL ok;

  if (c2h0_file == INVALID_HANDLE_VALUE) {
    fprintf(err_file, "[%s] device files are not open\n", __FUNCTION__);
    return -1;
  }

  running = dma_is_running();
  if (!running) {
    ret = 0;
    if (n_hw_img) {
      // Inicializa la fifo de entrada:
      ififo.flush();
      if (ififo.size() != n_dma_buffers) {
        ififo.resize((unsigned int) n_dma_buffers);
      }
      ififo.nb_put(dma_buffers, (unsigned int) n_dma_buffers);

      // Inicializa la fifo de salida:
      ofifo.flush();
      if (ofifo.size() != n_dma_buffers) {
        ofifo.resize((unsigned int)n_dma_buffers);
      }

      dma_acq_cfg.n_hw_img = (n_hw_img < 0) ? -n_hw_img : n_hw_img;
      dma_acq_cfg.cont_mode = (n_hw_img < 0) ? 1 : 0;
      dma_acq_cfg.hw_img_size = calc_hw_image_size(hw_frame_len, hw_image_len);
      dma_acq_cfg.cmd = DMA_CMD_START;
      ok = ::SetEvent(dma_acq_cfg.start_event);
    }
  }
  else {
    fprintf(err_file, "[%s] DMA is still running\n", __FUNCTION__);
	ret = -2;
  }
  return ret;
}

int fuse::dma_abort() {
  //printf("[%s] +\n", __FUNCTION__);
  BOOL ok;
  dma_stop(); // Stop the current acquisition...
  dma_acq_cfg.cmd = DMA_CMD_EXIT;
  ok = ::SetEvent(dma_acq_cfg.start_event);
  while (dma_is_running());
  //printf("[%s] -\n", __FUNCTION__);
  return 0;
}

int fuse::dma_stop() {
  int running;
  BOOL ok;

  ELOG("dma_stop() Start", EPCIE_USER);
  running = dma_is_running();
  if (running) {
	  ELOG("dma_stop() SetEvent", EPCIE_USER);
    ok = ::SetEvent(dma_acq_cfg.stop_event);
	ELOG("dma_stop() Unlock Fifo", EPCIE_USER);
#ifdef IFIFO_INF_TIMEOUT
    ififo.unlock();   // Unlock ififo:
#endif
	ELOG("dma_stop() Waiting for stopping", EPCIE_USER);
    while (dma_is_running());
  }
  return 0;
}


hw_size_t fuse::dma_buffer_recv() const{
  hw_size_t HW_SIZE_ALIGNED n;
  // From https://learn.microsoft.com/en-us/windows/win32/sync/interlocked-variable-access
  //    Simple reads and writes to properly aligned 64-bit variables are
  //    atomic on 64-bit Windows.
  n = (hw_size_t) dma_acq_cfg.total_hw_img;
  return n;
}

int fuse::dma_buffer_leakage() {
  int used, size, free;
  used = ififo.used();
  size = ififo.size();
  free = size - used;
  return free;
}

int fuse::dma_get_status(fuse_status_t &status, int clear) {
  status = 0;
  return 0;
}


//-----


int fuse::emul_run(int n_hw_img, int hw_frame_len, int hw_img_len, int gap, uint32_t mode) {
  hw_img_conf_t  emul_cfg;
  int ret;

  if (m_emul_module) {
    emul_cfg.n_images = n_hw_img;
    emul_cfg.frame_len = hw_frame_len;
    emul_cfg.image_len = hw_img_len;
    emul_cfg.gap = gap;
    emul_cfg.mode = mode;

    BOOL ok;
    ok = DeviceIoControl(
            user_file,                  // hDevice
            IOCTL_FUSE_EMUL_GENERATE,   // I/O control code
            &emul_cfg,                  // Input buffer
            sizeof(hw_img_conf_t),     // Input buffer size
            nullptr,                    // Output buffer
            0,                          // Output buffer size
            nullptr,                    // number of returned bytes
            nullptr);                   // Overlapped

    ret = (ok)? 0 : -1;
  }
  else {
    ret = -1;
  }

  return ret;
}

int fuse::emul_stop_cont(uint32_t mode) {
  int ret;

  if (m_emul_module) {
    BOOL ok;
    ok = DeviceIoControl(
            user_file,                  // hDevice
            IOCTL_FUSE_EMUL_STOP_CONT,  // I/O control code
            &mode,                      // Input buffer
            sizeof(mode),               // Input buffer size
            nullptr,                    // Output buffer
            0,                          // Output buffer size
            nullptr,                    // number of returned bytes
            nullptr);                   // Overlapped

    ret = (ok)? 0 : -1;
  }
  else {
    ret = -1;
  }

  return ret;
}

int fuse::emul_hw_reset(uint32_t mode) {
  int ret = -1;
  BOOL ok;
  if (m_emul_module) {
    ok = DeviceIoControl(
            user_file,                        // hDevice
            IOCTL_FUSE_EMUL_RESET_HW,         // I/O control code
            &mode,                            // Input buffer
            sizeof(uint32_t),                 // Input buffer size
            nullptr,                          // Output buffer
            0,                                // Output buffer size
            nullptr,                          // number of returned bytes
            nullptr);                         // Overlapped
    ret = (ok) ? 0 : -1;
  }
  return ret;
}


//m_hAbortEvent.reset(::CreateEvent(nullptr, TRUE, FALSE, nullptr));
  //wil::unique_handle hThread;


//void fuse::run() {
//}

void fuse::destroy_threads() {
  //BOOL ok;
  HANDLE total_hThread[] = {dma_hThread,
                            olink_event_hThread,
                            dma_event_hThread};

#define TOTAL_N_THREAD (sizeof(total_hThread)/sizeof(HANDLE))

  HANDLE active_hThread[TOTAL_N_THREAD];
  int n_threads = 0;

  // Solicito la detención de los threads:
  ::SetEvent(abort_event);  // <= para los threads de eventos

  if (dma_hThread) {
    dma_abort();
  }
  // ... espero a que se detengan todos los threads
  Sleep((DWORD) m_max_timeout);

  for (int i = 0; i < TOTAL_N_THREAD; i++) {
    if (total_hThread[i] != nullptr) {
      active_hThread[n_threads++] = total_hThread[i];
    }
  }

  ::WaitForMultipleObjects(n_threads, active_hThread, TRUE, INFINITE);

  if (abort_event != nullptr) {
    CloseHandle(abort_event);
  }

  if (dma_acq_cfg.start_event != nullptr) {
    CloseHandle(dma_acq_cfg.start_event);
  }

  if (dma_acq_cfg.stop_event != nullptr) {
    CloseHandle(dma_acq_cfg.stop_event);
  }
}

int fuse::create_dma_thread() {
  dma_acq_cfg.start_event = CreateEvent(nullptr, TRUE, FALSE, nullptr);
  dma_acq_cfg.stop_event  = CreateEvent(nullptr, TRUE, FALSE, nullptr);
  dma_acq_cfg.cmd = 0;
  dma_acq_cfg.n_hw_img = 0;
  dma_acq_cfg.hw_img_size = 0;
  dma_acq_cfg.total_hw_img = 0;
  dma_acq_cfg.cont_mode = 0;

  dma_hThread = ::CreateThread(
                      nullptr,                                  // lpThreadAttributes
                      0,                                        // dwStackSize
                      [](auto param) {                          // lpStartAddress
                        return ((fuse *)param)->dma_thread();
                      },
                      this,                                     // lpParameter
                      0,                                        // dwCreationFlags
                      nullptr);                                 // lpThreadId

  return (dma_hThread == nullptr)? -1 : 0;
}

int fuse::destroy_dma_thread() {
  if (dma_hThread != nullptr) {
    dma_abort();
    ::WaitForSingleObject(dma_hThread, INFINITE);
    if (dma_acq_cfg.start_event != nullptr) {
      CloseHandle(dma_acq_cfg.start_event);
    }
    if (dma_acq_cfg.stop_event != nullptr) {
      CloseHandle(dma_acq_cfg.stop_event);
    }
  }
  dma_hThread = nullptr;
  return 0;  
}


int fuse::create_threads() {
  timeout_t timeout;

  // Evento para solicitar el aborto de todos threads!
  abort_event = CreateEvent(nullptr, TRUE, FALSE, nullptr);

  //-----

  if (m_use_event_thrd) {
    olink_event_hThread = ::CreateThread(
                                nullptr,                                  // lpThreadAttributes
                                0,                                        // dwStackSize
                                [](auto param) {                          // lpStartAddress
                                  return ((fuse *)param)->olink_event_thread();
                                },
                                this,                                     // lpParameter
                                0,                                        // dwCreationFlags
                                nullptr);                                 // lpThreadId

    if (olink_event_hThread == nullptr) {
      return -1;
    }
    get_event_olink_timeout(timeout);
    m_max_timeout = (timeout > m_max_timeout) ? timeout : m_max_timeout;

    dma_event_hThread = ::CreateThread(
                              nullptr,                                  // lpThreadAttributes
                              0,                                        // dwStackSize
                              [](auto param) {                          // lpStartAddress
                                return ((fuse *)param)->dma_event_thread();
                              },
                              this,                                     // lpParameter
                              0,                                        // dwCreationFlags
                              nullptr);                                 // lpThreadId

    if (dma_event_hThread == nullptr) {
      return -1;
    }
    get_event_fuse_dma_timeout(timeout);
    m_max_timeout = (timeout > m_max_timeout) ? timeout : m_max_timeout;
  }

  //-----


  get_dma_timeout(timeout);
  m_max_timeout = (timeout > m_max_timeout) ? timeout : m_max_timeout;

#if 0
#endif

  return 0;
}

int fuse::dma_get_sw_err(int clear) {
  char dma_read_err;   // dma reading error flag
  char ififo_rd_err;   // input fifo reading error flag
  char ofifo_wr_err;   // output fifo writting error flag
  char mask = (clear)? 0U : ~0U;
  dma_read_err = ::InterlockedAnd8(&m_dma_read_err, mask);
  ififo_rd_err = ::InterlockedAnd8(&m_ififo_rd_err, mask);
  ofifo_wr_err = ::InterlockedAnd8(&m_ofifo_wr_err, mask);

  int ret = 0;
  ret |= (dma_read_err)? DMA_READ_SW_ERR : 0;
  ret |= (ififo_rd_err)? DMA_IFIFO_SW_ERR : 0;
  ret |= (ofifo_wr_err)? DMA_OFIFO_SW_ERR : 0;
  return ret;  
}

inline hw_size_t fuse::read_hw_images(hw_size_t n_hw_img, size_t hw_img_size, int cont_mode) {
  int ret;
  BOOL ok;
  DWORD n_bytes;
  hw_size_t hw_img_rem;
  hw_size_t hw_img_cnt;

  InterlockedExchange64((volatile LONG64 *) &dma_acq_cfg.total_hw_img, 0);
  hw_img_cnt = 0;
  hw_img_rem = n_hw_img;
  
  while (hw_img_rem > 0) {
    dma_buffer_t buffer;
#ifdef IFIFO_INF_TIMEOUT
    ok = ififo.get(buffer, INFINITE);
    if (!ok) {
      printf("[%s] ififo GET failure\n", __FUNCTION__);
    }
    // Check the stop event
    if (::WaitForSingleObject(dma_acq_cfg.stop_event, 0) == WAIT_OBJECT_0) {
      //printf("[%s] ififo get: returning because of a STOP event\n", __FUNCTION__);
      ::ResetEvent(dma_acq_cfg.stop_event);
      return hw_img_cnt;
    }
#else
    // 1. Get a buffer from the input fifo. This is a blocking funtion:
    do {
      ok = ififo.get(buffer, 1);
      if (!ok) {
        printf("[%s] ififo GET failure\n", __FUNCTION__);
      }
      // Check the stop event
      if (::WaitForSingleObject(dma_acq_cfg.stop_event, 0) == WAIT_OBJECT_0) {
        ::ResetEvent(dma_acq_cfg.stop_event);
        printf("[%s] ififo get: returning because of a STOP event\n", __FUNCTION__);
        return hw_img_cnt;
      }
    } while (!ok);
#endif

    // 2. Read the dma data
    do {
      // ReadFile returns an ok and zero bytes read when a timeout occurs
      ok = ReadFile(c2h0_file, buffer, (DWORD)hw_img_size, &n_bytes, NULL);
	  //printf("\r\nReadFile() = %d (%d n_bytes)", ok, n_bytes);
      while (ok && !n_bytes) {
        if (::WaitForSingleObject(dma_acq_cfg.stop_event, 0) == WAIT_OBJECT_0) {
          //printf("[%s] ReadFile: returning because of a STOP event\n", __FUNCTION__);
          ::ResetEvent(dma_acq_cfg.stop_event);
          return hw_img_cnt;
        }
        ok = ReadFile(c2h0_file, buffer, (DWORD)hw_img_size, &n_bytes, NULL);
      }
      
      InterlockedOr8(&m_dma_read_err, (char)!ok);
      if (!ok) {
        fprintf(err_file, "[%s] DMA reading error!!!\n", __FUNCTION__);
      }
      
    }
    while(!ok);

    // 3. Analyze the received data
    if (m_verbose) {
      int hw_img_id;
      ret = get_hw_image_info(buffer, &hw_img_id, NULL, NULL);
      if (ret < 0) {
        fuse_header_t *hdr = (fuse_header_t *)buffer;
        printf("[%s] Error reading hw_img %lld: (n_bytes = %d) hw_img_id = %d, magic number = 0x%8.8X\n",
          __FUNCTION__, (int64_t) hw_img_cnt, n_bytes, hdr->image_id, hdr->magic_num);
      }
      else {
        printf("[%s] Read HW image %lld, hw_img_id = %d\n", __FUNCTION__, (int64_t) hw_img_cnt, hw_img_id);
      }
    }
    
    // 4. Put the buffer into the output fifo:
    ok = ofifo.nb_put(buffer);
    InterlockedOr8(&m_ofifo_wr_err, (char)!ok);
    if(!ok) {
      fprintf(err_file, "[%s] Error, output fifo overflow\n", __FUNCTION__);
    }
    
    // 5. Update the number of HW images received
    InterlockedIncrement64((volatile LONG64 *) &dma_acq_cfg.total_hw_img);

    hw_img_cnt++;
    if (!cont_mode) {
      hw_img_rem--;
    }
  }
  return hw_img_cnt;
}

DWORD fuse::dma_thread() {
  hw_size_t hw_img_cnt;
  hw_size_t n_hw_img;
  size_t hw_img_size;
  int cont_mode;
  int dma_cmd;
  int exit = 0;

  //printf("[%s] thread +\n", __FUNCTION__);
  ELOG("dma_thread() START", EPCIE_USER);

  while (!exit) {
    //printf("[%s] waiting for event\n", __FUNCTION__);
    ::WaitForSingleObject(dma_acq_cfg.start_event, INFINITE);

    dma_cmd = dma_acq_cfg.cmd;
    switch(dma_cmd) {
      case DMA_CMD_NOP:
        ::ResetEvent(dma_acq_cfg.start_event);
        break;
      case DMA_CMD_START:
        n_hw_img = dma_acq_cfg.n_hw_img;
        hw_img_size = dma_acq_cfg.hw_img_size;
        cont_mode = dma_acq_cfg.cont_mode;
		ELOG("dma_thread() Read Start", EPCIE_USER);
		hw_img_cnt = read_hw_images(n_hw_img, hw_img_size, cont_mode);
		ELOG("dma_thread() Read End", EPCIE_USER);
        if (hw_img_cnt < 0) {
        }
        ::ResetEvent(dma_acq_cfg.start_event);
        break;
      case DMA_CMD_EXIT:
        exit = 1;
        ::ResetEvent(dma_acq_cfg.start_event);
        break;
      default:
        ;
    }
  }
  ELOG("dma_thread() END", EPCIE_USER);

  //printf("[%s] thread -\n", __FUNCTION__);
  return 0;
}

//if (hw_img_size == INFINITE) {
//}


//-------------------------------------


int fuse::test() {
  fuse_event_t olink_event;
  DWORD n_bytes;
  BOOL ok;

  cancel_event_file(dma_event_file, 0xE0);
  //cancel_event_file(olink_event_file, 0xE1);
  return 0;


  return 0;


  int s = sizeof(DWORD);

  ok = ReadFile(olink_event_file, &olink_event, sizeof(fuse_event_t), &n_bytes, NULL);

  return 0;
}


#if 0
int fuse::acq_start(int n_hw_img, size_t hw_img_size) {
  int ret;

  ret = alloc_dma_buffers(n_hw_img, hw_img_size);

  ififo.flush();
  ififo.resize(n_hw_img);
  ififo.nb_put(dma_buffers, n_hw_img);

  ofifo.flush();
  ofifo.resize(n_hw_img);

  return 0;
}


int fuse::acq_start(int n_hw_img, int frame_length, int image_length) {
  size_t hw_img_size;
  int ret;

  hw_img_size = calc_hw_image_size(frame_length, image_length);

  ret = alloc_dma_buffers(n_hw_img, frame_length, image_length);

  ififo.flush();

  ififo.nb_put(dma_buffers, n_hw_img);

  return 0;
}
#endif
