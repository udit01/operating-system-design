#include <stdio.h> 
#include <sys/ipc.h> 
#include <sys/msg.h> 
#include <math.h>
#include <stdlib.h>
#include <time.h> 
#include <errno.h>
#include <linux/unistd.h>       /* for _syscallX macros/related stuff */
#include <linux/kernel.h>       /* for struct sysinfo */
#include <sys/sysinfo.h>

#define max_queue_elements 100

int P, P1, P2, P3;

int parent_pid;
int msgids[5][5] = {0};
int pbar, pbar2;
int i,j;
int size = 0;
int dat;

typedef struct{
  int queue[max_queue_elements];
  int head;
  int tail;
} simple_queue;

simple_queue q = {
  .head = 0,
  .tail = 0,
  .queue = { 0 },
};

void enque(int val)
{
  q.queue[q.tail] = val;
  q.tail = (q.tail + 1) % max_queue_elements;
}

int deque()
{
  if(q.head == q.tail)
    return -1; // Empty queue
  int result = q.queue[q.head];
  q.head = (q.head + 1) % max_queue_elements;
  return result;
}

long get_uptime()
{
    struct sysinfo info;
    sysinfo(&info);
    return info.uptime;
}

void parseInput(){
    FILE * fp;
    char * line = NULL;
    size_t len = 0;
    ssize_t read;

    fp = fopen("assig2b.inp", "r");

    getline(&line, &len, fp);
    P = atoi(line);
    getline(&line, &len, fp);
    P1 = atoi(line);
    getline(&line, &len, fp);
    P2 = atoi(line);
    getline(&line, &len, fp);
    P3 = atoi(line);

    printf("Ps : %d, %d, %d, %d\n", P, P1, P2, P3);

    if (line)
        free(line);
}

// structure for message queue 
struct mesg_buffer { 
    long mesg_type; 
    int mesg_int; 
} message; 

void send(int sender_pid, int receiver_i, int receiver_j, int value)
{
    message.mesg_type = 1; 
    // printf("Sending %d to %d,%d from %d,%d\n", value, receiver_i, receiver_j, i, j);
    message.mesg_int = value;
    int msgid = msgids[receiver_i][receiver_j];
    msgsnd(msgid, &message, sizeof(message), 0);
}

int recv()
{
    int msgid = msgids[i][j];
    msgrcv(msgid, &message, sizeof(message), 1, 0); //printf(message.mesg_text);
    // printf("Got value %d by %d,%d\n", message.mesg_int, i,j);
    return message.mesg_int;
}

void barrier(){
    if(i == 0 && j == 0){
        // printf("first barrier %d %d\n", i, j);
        for(int i = 1; i < P; i++){
            msgrcv(pbar, &message, sizeof(message), 100, 0);
            // printf("Got %d\n", message.mesg_type);
        }
        for(int i = 1; i < P; i++){
            message.mesg_type = 200; //printf("Resume %d\n", message.mesg_type);
            msgsnd(pbar2, &message, sizeof(message), 0);
        }
    }
    else{
        message.mesg_type = 100;
        // printf("bar enter %d,%d\n", i, j);
        msgsnd(pbar, &message, sizeof(message), 0);
        msgrcv(pbar2, &message, sizeof(message), 200, 0); 
        // printf("bar exit %d,%d\n", i, j);
    }
}

int main(int argc, char *argv[])
{
    parseInput();
    int proc_pids[5][5];
    int pid;
    parent_pid = getpid();
    int wait_time = 0;
    int num_procs_done = 0;
    // int set[9];
    char fname[10] = "./qs/p00";
    // printf("Start\n");
    pbar =  msgget(ftok("./qs/pbar", 65), 0666 | IPC_CREAT);
    pbar2 =  msgget(ftok("./qs/pbar2", 65), 0666 | IPC_CREAT);
    for(int i = 0; i < 5; i++){
        for(int j = 0; j < 5; j++){
            fname[6] = i + '0'; fname[7] = j + '0';
            msgids[i][j] = msgget(ftok(fname, 65), 0666 | IPC_CREAT); // Creates msg queue and returns identifier for each proc
        }
    }

    switch (P){
        case 4: size = 2; break;
        case 9: size = 3; break;
        case 16: size = 4; break;
        case 25: size = 5;
    }

	// Initialise procs
    pid = getpid();
	for(i = 0; i < size; i++){
        for(j = 0; j < size; j++){
            proc_pids[i][j] = pid;
            pid++;
        }
    }
    
    for(i = 1; i < P; i++){
		pid = fork();
		if (pid == 0)
			goto child;
	}

	// dps();

	child:

    proc_pids[0][0] = parent_pid;

    // Find my i,j
	for(i = 0; i < size; i++)
        for(j = 0; j < size; j++)
            if(proc_pids[i][j] == getpid())
                goto next;
    
    next:
    // printf("Found (%d, %d) as my pid = %d\n", i, j, getpid());

    // Set my wait_time
    if(size*i + j < P1 + P2)
        wait_time = 2;

    // Send request to quorum - msg = my pid
    int dat = size*i + j;
    int first = 1;
    for(int p = 0; p < size; p++){
        for(int q = 0; q < size; q++){
            if(p == i && q == j){
                for(int x = 0; x < size; x++){
                    for(int y = 0; y < size; y++){
                        if(x == i || y == j){
                            if(first == 1){
                                send(getpid(), x, y, size*x+y);
                                // printf("Reply to (%d) by %d,%d\n", size*x+y, i, j);
                                first = 0;
                            }
                            else{
                                enque(size*x+y);
                                // printf("Enq temp = %d by %d,%d\n", size*x+y, i,j);
                            }
                        }
                    }
                }
            }
            barrier();

        }
    }

    barrier();

    int temp;

    // Wait for replies from all procs in quorum - msg = my pid
    int num = 0;
    while(1){
        temp = recv();
        if(temp >= 500){
            int temp1 = deque();
            if(temp1 != -1)
                send(getpid(), temp1/size, temp1%size, temp1);
            num_procs_done++;
            // printf("Reply to (%d) by %d,%d\n", temp1, i,j);
            // printf("Numprocs done at %d is %d\n", proc_pids[i][j], num_procs_done);
        }
        else{
            num++;
        }

        // printf("Pid %d,%d num = %d, got %d\n", i,j, num, temp);
        
        if(num >= (2*size - 1))
            break;
    }

    if(size*i + j < P1)
        goto nosend;

    // Execute critical section
    printf("%d acquired the lock at time %d\n", getpid(), get_uptime());
    sleep(wait_time);
    printf("%d released the lock at time %d\n", getpid(), get_uptime());

    nosend:

    temp = deque();
    if(temp != -1)
        send(getpid(), temp/size, temp%size, temp);
    // printf("Reply to (%d) by %d,%d\n", temp, i,j);
    num_procs_done++;
    // printf("Numprocs done at %d is %d\n", proc_pids[i][j], num_procs_done);
    proc_pids[0][0] = parent_pid;

    // Send release to quorum
    for(int x = 0; x < size; x++){
        for(int y = 0; y < size; y++){
            if((x == i || y == j) && !(x == i && y == j)){
                send(getpid(), x, y, 500);// 500 signifies release
                // printf("Release to %d,%d\n", x, y);
            }
        }
    }

    // Send replies to others
    while(num_procs_done < 2*size - 1){
        temp = recv();
        if(temp == 500){
            temp = deque();
            if(temp != -1)
                send(getpid(), temp/size, temp%size, 100);
            // printf("Reply to (%d) by %d,%d\n", temp, i,j);
            num_procs_done++;
        }
        // printf("Numprocs done at %d is %d\n", proc_pids[i][j], num_procs_done);

        if(num_procs_done >= 2*size - 1)
            break;
    }


    if(getpid() == parent_pid){
        for(int i = 0; i < P-1; i++)
            wait(); // wait for all child procs to exit
    }

    printf("Proc %d,%d exiting\n", i, j);
	
	exit(0);
}
