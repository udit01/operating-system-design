#include "types.h"
#include "defs.h"
#include "param.h"
#include "memlayout.h"
#include "mmu.h"
#include "proc.h"
#include "x86.h"
#include "syscall.h"

// User code makes a system call with INT T_SYSCALL.
// System call number in %eax.
// Arguments on the stack, from the user call to the C
// library system call function. The saved user %esp points
// to a saved program counter, and then the first argument.

// Fetch the int at addr from the current process.
int
fetchint(uint addr, int *ip)
{
  struct proc *curproc = myproc();

  if(addr >= curproc->sz || addr+4 > curproc->sz)
    return -1;
  *ip = *(int*)(addr);
  return 0;
}

// Fetch the nul-terminated string at addr from the current process.
// Doesn't actually copy the string - just sets *pp to point at it.
// Returns length of string, not including nul.
int
fetchstr(uint addr, char **pp)
{
  char *s, *ep;
  struct proc *curproc = myproc();

  if(addr >= curproc->sz)
    return -1;
  *pp = (char*)addr;
  ep = (char*)curproc->sz;
  for(s = *pp; s < ep; s++){
    if(*s == 0)
      return s - *pp;
  }
  return -1;
}

// Fetch the nth 32-bit system call argument.
int
argint(int n, int *ip)
{
  return fetchint((myproc()->tf->esp) + 4 + 4*n, ip);
}

// Fetch the nth word-sized system call argument as a pointer
// to a block of memory of size bytes.  Check that the pointer
// lies within the process address space.
int
argptr(int n, char **pp, int size)
{
  int i;
  struct proc *curproc = myproc();
 
  if(argint(n, &i) < 0)
    return -1;
  if(size < 0 || (uint)i >= curproc->sz || (uint)i+size > curproc->sz)
    return -1;
  *pp = (char*)i;
  return 0;
}

// Fetch the nth word-sized system call argument as a string pointer.
// Check that the pointer is valid and the string is nul-terminated.
// (There is no shared writable memory, so the string can't change
// between this check and being used by the kernel.)
int
argstr(int n, char **pp)
{
  int addr;
  if(argint(n, &addr) < 0)
    return -1;
  return fetchstr(addr, pp);
}

extern int sys_chdir(void);
extern int sys_close(void);
extern int sys_dup(void);
extern int sys_exec(void);
extern int sys_exit(void);
extern int sys_fork(void);
extern int sys_fstat(void);
extern int sys_getpid(void);
extern int sys_kill(void);
extern int sys_link(void);
extern int sys_mkdir(void);
extern int sys_mknod(void);
extern int sys_open(void);
extern int sys_pipe(void);
extern int sys_read(void);
extern int sys_sbrk(void);
extern int sys_sleep(void);
extern int sys_unlink(void);
extern int sys_wait(void);
extern int sys_write(void);
extern int sys_uptime(void);
extern int sys_print_count(void);
extern int sys_toggle(void);
extern int sys_add(void);
extern int sys_ps(void);
extern int sys_send(void);
extern int sys_recv(void);
extern int sys_sigset(void);
extern int sys_sigsend(void);
extern int sys_sigret(void);

static int (*syscalls[])(void) = {
[SYS_fork]    sys_fork,
[SYS_exit]    sys_exit,
[SYS_wait]    sys_wait,
[SYS_pipe]    sys_pipe,
[SYS_read]    sys_read,
[SYS_kill]    sys_kill,
[SYS_exec]    sys_exec,
[SYS_fstat]   sys_fstat,
[SYS_chdir]   sys_chdir,
[SYS_dup]     sys_dup,
[SYS_getpid]  sys_getpid,
[SYS_sbrk]    sys_sbrk,
[SYS_sleep]   sys_sleep,
[SYS_uptime]  sys_uptime,
[SYS_open]    sys_open,
[SYS_write]   sys_write,
[SYS_mknod]   sys_mknod,
[SYS_unlink]  sys_unlink,
[SYS_link]    sys_link,
[SYS_mkdir]   sys_mkdir,
[SYS_close]   sys_close,
[SYS_print_count] sys_print_count,
[SYS_toggle]  sys_toggle,
[SYS_add]     sys_add,
[SYS_ps]      sys_ps,
[SYS_send]      sys_send,
[SYS_recv]      sys_recv,
[SYS_sigset]      sys_sigset,
[SYS_sigsend]      sys_sigsend,
[SYS_sigret]      sys_sigret,
};

static char* syscallnames[] = {
[SYS_fork]    "sys_fork",
[SYS_exit]    "sys_exit",
[SYS_wait]    "sys_wait",
[SYS_pipe]    "sys_pipe",
[SYS_read]    "sys_read",
[SYS_kill]    "sys_kill",
[SYS_exec]    "sys_exec",
[SYS_fstat]   "sys_fstat",
[SYS_chdir]   "sys_chdir",
[SYS_dup]     "sys_dup",
[SYS_getpid]  "sys_getpid",
[SYS_sbrk]    "sys_sbrk",
[SYS_sleep]   "sys_sleep",
[SYS_uptime]  "sys_uptime",
[SYS_open]    "sys_open",
[SYS_write]   "sys_write",
[SYS_mknod]   "sys_mknod",
[SYS_unlink]  "sys_unlink",
[SYS_link]    "sys_link",
[SYS_mkdir]   "sys_mkdir",
[SYS_close]   "sys_close",  
[SYS_print_count] "sys_print_count",
[SYS_toggle]  "sys_toggle",
[SYS_add]     "sys_add",
[SYS_ps]      "sys_ps",
[SYS_send]      "sys_send",
[SYS_recv]      "sys_recv",
[SYS_sigset]      "sys_sigset",
[SYS_sigsend]      "sys_sigsend",
[SYS_sigret]      "sys_sigret",
};

int trace=0; // default is trace off
int countCalls[NELEM(syscalls)] = {0}; // NUM CALLS +1 init to 0

void
syscall(void)
{
  int num;
  struct proc *curproc = myproc();

  num = curproc->tf->eax;
  if(num > 0 && num < NELEM(syscalls) && syscalls[num]) {
    // if (printCall == 1) {
    //   cprintf("%s %d\n", syscallnames[num], countCall);
    // }
    if(trace == 1){ // if trace is on
      countCalls[num]++;
    }
    // countCall++;
    // PROBLEM at call 23 because prototype doesn't match..?
    curproc->tf->eax = syscalls[num]();
    // print the system call name and count number of times ? Print all at the end of the process does process call exit at the end?
  } else {
    cprintf("%d %s: unknown sys call %d\n",
            curproc->pid, curproc->name, num);
    curproc->tf->eax = -1;
  }
}

int 
sys_toggle(void)
{
  trace = 1 - trace;
  if (trace == 1) {
    // If transition to trace ON, then re_initilize array
    int i = 0;
    for(i = 0 ; i < NELEM(syscalls) ; i++){
      countCalls[i] = 0;
    }
  }
  return 0;
}
int sys_print_count(void){

  int i = 0;
  for (i = 1; i < NELEM(syscalls) ; i++){
    cprintf("%s %d\n", syscallnames[i], countCalls[i]);
  }
  return 0;
}