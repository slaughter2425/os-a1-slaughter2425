#include "kernel/types.h"
#include "user/user.h"

int
main(int argc, char *argv[])
{
  // Check that a mask and command were provided
  if (argc < 3) {
    fprintf(2, "usage: trace mask command [args...]\n");
    exit(1);
  }

  // Enable tracing using the provided mask
  if (trace(atoi(argv[1])) < 0) {
    fprintf(2, "trace failed\n");
    exit(1);
  }

  // Run the requested command
  exec(argv[2], &argv[2]);

  // This runs only if exec fails
  fprintf(2, "trace: exec failed\n");
  exit(1);
}