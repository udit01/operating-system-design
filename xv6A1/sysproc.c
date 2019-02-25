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

// struct Message messages[NPROC] = {{.from_pid = -1, .to_pid=-1, .content = "       \0"}};
// struct spinlock memLock;

#define MSG_SP 1024
typedef struct{
  struct spinlock memLock;
  char msgs[MSG_SP][MSGSIZE];
  int from[MSG_SP];
  int dest[MSG_SP];
  int waiting[NPROC];
  char wait_msgs[NPROC];
} locs;

locs buf = {
  .msgs = { "       \0" },
  .from = { 0 },
  .dest = { 0 },
  .waiting = { 0 },
  .wait_msgs = { "       \0" }
};

int wakeup_process(int);
int sleep_process(int);

int
sys_send(int sender_pid, int rec_pid, void* msg)
{
  char *str ;
  argint(0, &sender_pid);
  argint(1, &rec_pid);
  argptr(2, &str, MSGSIZE);
  
  cprintf("(In sys_send system call) Message to be sent is -> %s \n", str);

  int i = 0;

  acquire(&buf.memLock);
  if(buf.waiting[rec_pid] == 1){
    memmove(&buf.wait_msgs[rec_pid], str, MSGSIZE);
    buf.waiting[rec_pid] = 0;
    wakeup((void*) rec_pid);
    release(&buf.memLock);
    return 0;
  }
  else{
    for( i = 0; i < MSG_SP; i++){
      if (buf.dest[i] <= 0) {
        memmove(buf.msgs[i], str, MSGSIZE);
        buf.from[i] = sender_pid;
        buf.dest[i] = rec_pid;
        release(&buf.memLock);
        return 0;
      }
    }
  }
  return -1;
  // acquire(&memLock);
  // for( i = 0; i < NPROC; i++){
  //   if (messages[i].from_pid == -1) {
  //     messages[i].from_pid = sender_pid;
  //     messages[i].to_pid = rec_pid;
  //     memmove(messages[i].content, str, MSGSIZE);
  //     release(&memLock);
      
  //     cprintf("(In sys_send system call) Message written in mailbox\n");
  //     return 0;
  //   }
  // }

  // // release(&memLock);
  // if( i >= NPROC){
  //   cprintf("(In sys_send system call) Mailbox full or other error\n");
  //   return -1; // error as i exceeded bounds and couldn't put mem anywhere
  // }
  // // Shouldn't come here ever
  // return 0;
}

int
sys_recv(void* msg)
{
  char *str;
  argptr(0, &str, MSGSIZE);
  
  int caller_pid = myproc()->pid;
  int i = 0;
  // int flag = 0;
  char*empty = "       \0";
  acquire(&buf.memLock);
    for(i = 0; i < MSG_SP; i++){
    if(buf.dest[i] == caller_pid){
      // cprintf("Enter recv pid : %d : topid %d\n", me, kern.to_pids[i]);
      memmove(str, buf.msgs[i], MSGSIZE);
      buf.dest[i] = -1;
      memmove(&buf.msgs[i][0], empty, MSGSIZE);
      release(&buf.memLock);
      return 0;
    }
  }
  buf.waiting[caller_pid] = 1;
  argptr(0, (char**)&buf.wait_msgs[caller_pid], 4);
  sleep((void*)caller_pid, &buf.memLock);
  release(&buf.memLock);
  // cprintf("(In sys_recv system call) Message received is -> %s \n", str);
  return 0;

  // while to block until receive message
  // while(flag == 0){
  //   for( i = 0; i < NPROC ; i++){
  //     if(messages[i].to_pid == caller_pid){
  //       acquire(&memLock);
  //       memmove(str, messages[i].content, MSGSIZE);
  //       //reset the block, because this message has been delivered
  //       messages[i].from_pid = -1;
  //       release(&memLock);
  //       flag = 1;
  //     }
  //   }
  // }


  // return 0;
}


int proc_sigsend(int, int, char*);

int
sys_send_multi(int sender_pid, int rec_pids[], void* msg, int num_recivers)
{
  char *str ;
  int *rec_p;
  argint(0, &sender_pid);
  argptr(1, (char**)&rec_p, 8*MSGSIZE);
  argptr(2, &str, MSGSIZE);
  argint(3, &num_recivers);
  
  cprintf("(In sys_send_multi system call) Message to be sent is -> %d \n", str);

  int caller_pid = myproc()->pid;

  acquire(&buf.memLock);

  int retval = 0;
  int i = 0;
  for(i = 0 ; i < num_recivers; i++){
      retval = proc_sigsend(caller_pid, rec_p[i], str);
      // cprintf("LOOP i val = %d, RET VAL = %d \n", i, retval);
      if(retval < 0){
        // some erro and abort
        release(&buf.memLock);
        return -1;
      }
  }
  release(&buf.memLock);
  cprintf("END MULTI SEND SYS CALL \n");
  return 0;


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
}

sig_handler proc_sigset(sig_handler);
//set the singal handler of a process
sig_handler sys_sigset( sig_handler fptr) {
  int addr;
  argint(0, &addr);
  // func_ptr = (sig_handler) addr;
  // sig_handler h;
  // //modify this proces's handler here and return the previous signal handler
  // return h;
  return proc_sigset((sig_handler) addr);
}

// send a singal to the given process
int sys_sigsend(int to_pid, int value){
//   argint(0, &to_pid);
//   argint(1, &value);
//   struct proc *currproc = myproc();
    cprintf("Nothing happened in sys_sigsend");
    return 0;
//   return proc_sigsend(currproc->pid, to_pid, value);
}

void proc_sigret(void);
// restore the system registers , to restore the calling context fully
int sys_sigret(void){
  proc_sigret();
  return 0;
}
