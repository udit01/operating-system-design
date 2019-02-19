#include "types.h"
#include "x86.h"
#include "defs.h"
#include "date.h"
#include "param.h"
#include "memlayout.h"
#include "mmu.h"
#include "proc.h"

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


struct Message messages[NPROC] = {{.from_pid = -1, .to_pid=-1, .content = "       \0"}};
struct spinlock lock;

int
sys_send(int sender_pid, int rec_pid, void* msg)
{
  char *str ;
  argint(0, &sender_pid);
  argint(1, &rec_pid);
  argptr(2, &str, MSGSIZE);
  
  cprintf("(In sys_send system call) Message to be sent is -> %s \n", str);

  int i = 0;

  acquire(&lock);
  for( i = 0; i < NPROC; i++){
    if (messages[i].from_pid == -1) {
      messages[i].from_pid = sender_pid;
      messages[i].to_pid = rec_pid;
      memmove(messages[i].content, str, MSGSIZE);
      release(&lock);
      
      cprintf("(In sys_send system call) Message written in mailbox\n");
      return 0;
    }
  }

  release(&lock);
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
        acquire(&lock);
        memmove(str, messages[i].content, MSGSIZE);
        release(&lock);
        flag = 1;
      }
    }
  }

  cprintf("(In sys_recv system call) Message received is -> %s \n", str);

  return 0;
}