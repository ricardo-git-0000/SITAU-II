#ifndef FUSE_COMMON_H
#define FUSE_COMMON_H

#include <basetsd.h>
#include <wdm.h>

NTSTATUS fuse_kernel_delay_us(int delay_us);


#endif  // FUSE_COMMON_H
