#include "fuse_common.h"

NTSTATUS fuse_kernel_delay_us(int delay_us) {
  LARGE_INTEGER timeout;
  NTSTATUS wait_status = 0;
  timeout.QuadPart = -10 * delay_us; // timeout (relative time, in 100-nanosecond units)
  wait_status = KeDelayExecutionThread(
                  KernelMode,   // WaitMode
                  FALSE,        // Alertable
                  &timeout);    // Timeout
  return wait_status;
}
