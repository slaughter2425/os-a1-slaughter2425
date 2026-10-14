#ifndef _UPROC_H_
#define _UPROC_H_

#include "types.h"

struct uproc {
  int pid;       // Process ID
  int ppid;      // Parent's process ID, or 0
  int state;     // Process state
  uint64 sz;     // Size of user memory in bytes
  char name[16]; // Process name
};

#endif