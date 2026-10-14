#include "types.h"
#include "riscv.h"
#include "defs.h"
#include "param.h"
#include "memlayout.h"
#include "spinlock.h"
#include "proc.h"
#include "vm.h"
#include "sysinfo.h"
uint64
sys_exit(void)
{
  int n;
  argint(0, &n);
  kexit(n);
  return 0; // not reached
}

uint64
sys_getpid(void)
{
  return myproc()->pid;
}

uint64
sys_fork(void)
{
  return kfork();
}

uint64
sys_wait(void)
{
  uint64 p;
  argaddr(0, &p);
  return kwait(p);
}

uint64
sys_sbrk(void)
{
  uint64 addr;
  int t;
  int n;

  argint(0, &n);
  argint(1, &t);
  addr = myproc()->sz;

  if (t == SBRK_EAGER || n < 0) {
    if (growproc(n) < 0) {
      return -1;
    }
  } else {
    // Lazily allocate memory for this process: increase its memory
    // size but don't allocate memory. If the processes uses the
    // memory, vmfault() will allocate it.
    if (addr + n < addr)
      return -1;
    if (addr + n > TRAPFRAME)
      return -1;
    myproc()->sz += n;
  }
  return addr;
}

uint64
sys_pause(void)
{
  int n;
  uint ticks0;

  argint(0, &n);
  if (n < 0)
    n = 0;
  acquire(&tickslock);
  ticks0 = ticks;
  while (ticks - ticks0 < n) {
    if (killed(myproc())) {
      release(&tickslock);
      return -1;
    }
    sleep_prepare(&ticks);
    release(&tickslock);
    sleep();
    acquire(&tickslock);
  }
  release(&tickslock);
  return 0;
}

uint64
sys_kill(void)
{
  int pid;

  argint(0, &pid);
  return kkill(pid);
}

// return how many clock tick interrupts have occurred
// since start.
uint64
sys_uptime(void)
{
  uint xticks;

  acquire(&tickslock);
  xticks = ticks;
  release(&tickslock);
  return xticks;
}

//tracking which system calls a program makes and printing information about them.
uint64
sys_trace(void){ //handles trace() system call
    int mask; //used to select which system calls xv6 should trace

    argint(0, &mask); // Read the first syscall argument passed to trace(), and store it in mask

    myproc()->trace_mask = mask; // Store mask in current process

    return 0; // Indicate success
}



uint64
sys_sysinfo(void)
{
  uint64 addr;
  struct sysinfo info;
  struct proc *p = myproc();

  // Get the user address of the sysinfo structure
  argaddr(0, &addr);

  // Fill the structure with system information
  info.freemem = countfreemem();
  info.nproc = countprocs();
  info.nopenfile = countopenfiles();

  // Copy the information from kernel memory to user memory
  if (copyout(p->pagetable, p->sz, addr, (char *)&info, sizeof(info)) < 0) {
    return -1;
  }

  return 0;
}

uint64
sys_getprocs(void)
{
  uint64 addr;
  int max;

  // Get the user buffer address
  argaddr(0, &addr);

  // Get the maximum number of entries
  argint(1, &max);

  // Collect process information
  return getprocsinfo(addr, max);
}