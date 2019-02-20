#include "types.h"
#include "x86.h"
#include "defs.h"
#include "date.h"
#include "param.h"
#include "memlayout.h"
#include "mmu.h"
#include "proc.h"
#include "spinlock.h"

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

int 
sys_add(int x, int y)
{
  argint(0,&x);
  argint(1,&y);
  //  return ;
  return x+y;
}

void list_running(void);
int 
sys_ps(void)
{
  list_running();
  return 0;
}

struct Message{
  int from_pid;
  int to_pid;
  char content[MSGSIZE];
};


// struct {
//   struct spinlock memLock;
//   // struct Message proc[NPROC];
//   struct Message messages[NPROC] = {{.from_pid = -1, .to_pid=-1, .content = "       \0"}};
// } msgq;

struct Message messages[NPROC] = {{.from_pid = -1, .to_pid=-1, .content = "       \0"}};
struct spinlock memLock;

int
sys_send(int sender_pid, int rec_pid, void* msg)
{
  char *str ;
  argint(0, &sender_pid);
  argint(1, &rec_pid);
  argptr(2, &str, MSGSIZE);
  
  cprintf("(In sys_send system call) Message to be sent is -> %s \n", str);

  int i = 0;

  acquire(&memLock);
  for( i = 0; i < NPROC; i++){
    if (messages[i].from_pid == -1) {
      messages[i].from_pid = sender_pid;
      messages[i].to_pid = rec_pid;
      memmove(messages[i].content, str, MSGSIZE);
      release(&memLock);
      
      cprintf("(In sys_send system call) Message written in mailbox\n");
      return 0;
    }
  }

  release(&memLock);
  if( i >= NPROC){
    cprintf("(In sys_send system call) Mailbox full or other error\n");
    return -1; // error as i exceeded bounds and couldn't put mem anywhere
  }
  // Shouldn't come here ever
  return 0;
}

int
sys_recv(void* msg)
{
  char *str;
  argptr(0, &str, MSGSIZE);
  
  int caller_pid = myproc()->pid;
  int i = 0;
  int flag = 0;

  // while to block until receive message
  while(flag == 0){
    for( i = 0; i < NPROC ; i++){
      if(messages[i].to_pid == caller_pid){
        acquire(&memLock);
        memmove(str, messages[i].content, MSGSIZE);
        //reset the block, because this message has been delivered
        messages[i].from_pid = -1;
        release(&memLock);
        flag = 1;
      }
    }
  }

  cprintf("(In sys_recv system call) Message received is -> %s \n", str);

  return 0;
}

// int
// sys_send_multi(int sender_pid, int rec_pids[], void* msg, int num_recivers)
// {
//   char *str ;
//   argint(0, &sender_pid);
//   argptr(1, &rec_pids, sizeof(int));
//   argptr(2, &str, MSGSIZE);
//   argint(3, &num_recivers);
  
//   cprintf("(In sys_send_multi system call) Message to be sent is -> %s \n", str);

//   int i = 0;
//   int k = 0;

//   acquire(&memLock);
//   for( i = 0; i < NPROC; i++){
//     if (messages[i].from_pid == -1) {
//       messages[i].from_pid = sender_pid;
//       messages[i].to_pid = rec_pids[k];
//       memmove(messages[i].content, str, MSGSIZE);
//       k++;
//       release(&memLock);
//       if(k >= num_recivers){
//         return 0;
//       }
//       // cprintf("(In sys_send system call) Message written in mailbox\n");
//     }
//   }

//   release(&memLock);
//   if( i >= NPROC){
//     cprintf("(In sys_send system call) Mailbox full or other error\n");
//     return -1; // error as i exceeded bounds and couldn't put mem anywhere
//   }
//   // Shouldn't come here ever
//   return 0;
// }

sig_handler proc_sigset(sig_handler);
//set the singal handler of a process
sig_handler sys_sigset( sig_handler func_ptr) {
  char *ch;
  argptr(0, &ch, sizeof(sig_handler));
  func_ptr = (sig_handler) ch;
  // sig_handler h;
  // //modify this proces's handler here and return the previous signal handler
  // return h;
  return proc_sigset(func_ptr);
}

// send a singal to the given process
int sys_sigsend(int to_pid, int value){
  return 0;
}

// restore the system registers , to restore the calling context fully
void sys_sigret(void){
  
}

// int sys_pause(void){
//   // How else pausing for interrupt ? 
//   return 0;
// }
