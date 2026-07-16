#ifndef FUSE_DEBUG_H
#define FUSE_DEBUG_H

// FUSE SEVERITY LEVES (same as VHDL):
#define FUSE_NOTE      1
#define FUSE_WARNING   2
#define FUSE_ERROR     3
#define FUSE_FAILURE   4

#ifndef FUSE_ERR_LEVEL
#define FUSE_ERR_LEVEL FUSE_WARNING
#endif

#include <wdm.h>

#define _dbg_printf(level, ...)  DbgPrint(__VA_ARGS__)

#if (FUSE_NOTE >= FUSE_ERR_LEVEL) 
#define _dbg_note(...) DbgPrint(__VA_ARGS__)
#else
#define _dbg_note(...)
#endif

#if (FUSE_WARNING >= FUSE_ERR_LEVEL) 
#define _dbg_warning(...) DbgPrint(__VA_ARGS__)
#else
#define _dbg_warning(...)
#endif

#if (FUSE_ERROR >= FUSE_ERR_LEVEL) 
#define _dbg_error(...) DbgPrint(__VA_ARGS__)
#else
#define _dbg_error(...)
#endif

#if (FUSE_FAILURE >= FUSE_ERR_LEVEL) 
#define _dbg_failure(...) DbgPrint(__VA_ARGS__)
#else
#define _dbg_failure(...)
#endif


#endif  // FUSE_DEBUG_H
