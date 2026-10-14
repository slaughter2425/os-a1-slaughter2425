#ifndef _SYSINFO_H_
#define _SYSINFO_H_

#include "types.h"

struct sysinfo {
  uint64 freemem;    // Bytes of free physical memory
  uint64 nproc;      // Number of active/allocated processes
  uint64 nopenfile;  // Number of open file-table entries
};

#endif