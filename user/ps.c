#include "kernel/types.h"
#include "kernel/uproc.h"
#include "user/user.h"

#define MAXPROCS 64

// Convert process state numbers into names
char *
statename(int state)
{
  switch (state) {
    case 1: return "used";
    case 2: return "sleeping";
    case 3: return "runnable";
    case 4: return "running";
    case 5: return "zombie";
    default: return "unknown";
  }
}

int
main(void)
{
  struct uproc procs[MAXPROCS];

  int count = getprocs(procs, MAXPROCS);

  if (count < 0) {
    fprintf(2, "ps: getprocs failed\n");
    exit(1);
  }

  // Print table header
  printf("PID\tPPID\tSTATE\tSIZE\tNAME\n");

  // Print one row per process
  for (int i = 0; i < count; i++) {
    printf("%d\t%d\t%s\t%lu\t%s\n",
           procs[i].pid,
           procs[i].ppid,
           statename(procs[i].state),
           procs[i].sz,
           procs[i].name);
  }

  exit(0);
}