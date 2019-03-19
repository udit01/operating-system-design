#include "types.h"
#include "x86.h"
#include "defs.h"
#include "date.h"
#include "param.h"
#include "memlayout.h"
#include "mmu.h"
#include "proc.h"
#include "syscall_trace.h"
#include "spinlock.h"

#define num_message_buffers 1000
#define message_size 8
#define max_queue_elements 500

typedef void (* sig_handler) (char* msg);

typedef struct{
  struct spinlock lock;
  char buffers[num_message_buffers][8];
  int from_pids[num_message_buffers];
  int to_pids[num_message_buffers];
  int queue[NPROC][max_queue_elements];
  int head[NPROC];
  int tail[NPROC];
  int wait_queue[NPROC];
} kernel_buffers;

kernel_buffers kern = {
  .buffers = { "        " },
  .from_pids = { 0 },
  .to_pids = { 0 },
  .head = { 0 },
  .tail = { 0 },
  .queue = {{ 0 }},
  .wait_queue = { 0 }
};

void enque(int pid, int index)
{
  kern.queue[pid][kern.tail[pid]] = index;
  kern.tail[pid] = (kern.tail[pid] + 1) % max_queue_elements;
}

int deque(int pid)
{
  if(kern.head[pid] == kern.tail[pid])
    return -1; // Empty queue
  int result = kern.queue[pid][kern.head[pid]];
  kern.head[pid] = (kern.head[pid] + 1) % max_queue_elements;
  return result;
}

typedef struct{
  struct spinlock lock;
  int num_procs;
  int arrived;
  int pids[NPROC];
} kernel_barrier;

kernel_barrier bar = {
  .num_procs = 0,
  .arrived = 0,
  .pids = { 0 }
};

int
sys_fork(void)
{
  return fork();
}

int
sys_exit(void)
{
  exit();
  return 0;  // not reached
}

int
sys_wait(void)
{
  return wait();
}

int
sys_kill(void)
{
  int pid;

  if(argint(0, &pid) < 0)
    return -1;
  return kill(pid);
}

int
sys_getpid(void)
{
  return myproc()->pid;
}

int
sys_sbrk(void)
{
  int addr;
  int n;

  if(argint(0, &n) < 0)
    return -1;
  addr = myproc()->sz;
  if(growproc(n) < 0)
    return -1;
  return addr;
}

int
sys_sleep(void)
{
  int n;
  uint ticks0;

  if(argint(0, &n) < 0)
    return -1;
  acquire(&tickslock);
  ticks0 = ticks;
  while(ticks - ticks0 < n){
    if(myproc()->killed){
      release(&tickslock);
      return -1;
    }
    sleep(&ticks, &tickslock);
  }
  release(&tickslock);
  return 0;
}

// return how many clock tick interrupts have occurred
// since start.
int
sys_uptime(void)
{
  uint xticks;

  acquire(&tickslock);
  xticks = ticks;
  release(&tickslock);
  return xticks;
}

// MOD-1 : Added syscall printing with counts
int
sys_print_count(void)
{
  for(uint i = 0; i < num_sys_calls; i++){
    if(syscallcounts[i] > 0)
      cprintf("%s %d\n", syscallnames[i], syscallcounts[i]);
  }
  return 0;
}

// MOD-1 : Toggle the tracer on or off
int
sys_toggle(void)
{
  if(trace == TRACE_ON)
    trace = TRACE_OFF;
  else
    trace = TRACE_ON;
  // MOD-1 : Reset all counts
  if(trace == TRACE_ON){
    for(uint i = 0; i < num_sys_calls; i++){
      syscallcounts[i] = 0; 
    }
  }
  return 0;
}

// MOD-1 : Syscall to add two numbera
int
sys_add(int a, int b)
{
  argint(0, &a);
  argint(1, &b);
  return a+b;
}

// MOD-1 : System call to show running processes
void process_status(void);
int
sys_ps(void)
{
  process_status();
  return 0;
}

// MOD-1 : System call to show running processes (detailed) Extra
void process_status_detailed(void);
int
sys_dps(void)
{
  process_status_detailed();
  return 0;
}

// MOD-1 : System call to shutdown machine
int
sys_shutdown(void)
{
  outb(0xf4, 0x00);
  return 0;
}

// MOD-1 : System call to count CPU time of a pid
int scheds, pid_to_count, count;
int
sys_start_timer(int pid) 
{
  scheds = 0;
  pid_to_count = pid;
  count = 1;
  return 0;
}

// MOD-1 : System call to count CPU time of a pid
int
sys_end_timer(int pid) 
{
  count = 0;
  return scheds * 10; // Returns number of milliseconds as 1 jiffy = 10ms
}

// MOD-1 : System call to send message of 8 bytes
struct spinlock lock;
int wakeup_process(int);
int sleep_process(int);
int
sys_send(int sender_pid, int rec_pid, void *msg)
{
  char* ch;
  argint(0, &sender_pid);
  argint(1, &rec_pid);
  argptr(2, &ch, message_size);
  acquire(&kern.lock);
  acquire(&lock);
  for(int i = 0; i < num_message_buffers; i++){
    if(kern.to_pids[i] <= 0){        
      memmove(kern.buffers[i], ch, message_size);
      kern.from_pids[i] = sender_pid;
      kern.to_pids[i] = rec_pid;
      enque(rec_pid, i);
      if(kern.wait_queue[rec_pid] == 1){
        // The receiver is waiting already
        // cprintf("Sender came second %d\n", rec_pid);
        // cprintf("Waking up %d\n", rec_pid);
        // Remove from wait queue
        kern.wait_queue[rec_pid] = 0;
        // Wakeup reciever
        wakeup((void*)rec_pid);
      }
      release(&lock);
      release(&kern.lock);
      return 0;
    }
  }
  release(&kern.lock);
  release(&lock);
  return -1;
}

// MOD-1 : System call to receive message
int
sys_recv(void *msg)
{
  char* ch;
  argptr(0, &ch, message_size);
  int me = myproc()->pid;
  int val;

  while(1){
    acquire(&kern.lock);
    acquire(&lock);
    val = deque(me);
    if(val != -1){
      // cprintf("Enter recv pid : %d : topid %d\n", me, kern.to_pids[i]);
      memmove(ch, kern.buffers[val], message_size);
      kern.to_pids[val] = -1;
      kern.buffers[val][0] = ' ';
      release(&kern.lock);
      release(&lock);
      // cprintf("Exit recv : %d, %s\n", i, ch);
      return 0;
    }
    release(&lock);
    // Put myself in wait queue if no message
    kern.wait_queue[me] = 1;
    // Sleep me
    // cprintf("Sleeping %d\n", me);
    sleep((void*)me, &kern.lock);
    release(&kern.lock);
  }
  return 0;
}


// MOD-1 : System call to send message in multi-cast
int sigsend(int dest_pid, char* msg);
int
sys_send_multi(int sender_pid, int rec_pids[], void *msg, int len)
{
  int* pid;
  char* ch;
  int num;
  argint(0, &sender_pid);
  argptr(1, (char**)&pid, 64);
  argptr(2, &ch, message_size);
  argint(3, &num);
  acquire(&kern.lock);
  acquire(&lock);
  int result = 0;
  for(int t = 0; t < num; t++){
    // cprintf("Msg %s sent to pid : %d\n", ch, pid[t]);
    result = sigsend(pid[t], ch);
    if(result < 0){
      release(&kern.lock);
      release(&lock);
      return -1; // Return error
    }
  }
  release(&kern.lock);
  release(&lock);
  return 0;
}

// MOD-1 : System call to set interrupt handler
int sigset(sig_handler new_sighandler);
int
sys_sigset(sig_handler func)
{
  int sighandler;
  argint(0, &sighandler);
  myproc()->sig_handler = (sig_handler) sighandler;
  return 0;
}

// MOD-1 : Syscall for returning from trap
int sigret(void);
int
sys_sigret(void)
{
  sigret();
  return 0;
}


// MOD-2 : Syscall for barrier init
int
sys_barrier_init(int procs)
{
  argint(0, &procs);
  acquire(&bar.lock);
  bar.num_procs = procs;
  bar.arrived = 0;
  release(&bar.lock);
  return 0;
}

// MOD-2 : Syscall for barrier reached
int
sys_barrier(void)
{
  acquire(&bar.lock);
  bar.arrived ++;
  // cprintf("ArrivedProc = %d, ArrivedTotal = %d\n", myproc()->pid, bar.arrived);
  if(bar.arrived < bar.num_procs){ 
    // Add my pid for wakeup later
    bar.pids[bar.arrived - 1] = myproc()->pid;
    // More to arrive so sleep me
    sleep((void*)myproc()->pid, &bar.lock);
  }
  else{
    // All procs done so wakeup all
    bar.arrived = 0;
    for(int i = 0; i < bar.num_procs; i++)
      wakeup((void*)bar.pids[i]);
  }
  release(&bar.lock);
  return 0;
}