// MOD-2 : Implementation of Maekawa distributed mutal exclusion algorithm
#include "types.h"
#include "stat.h"
#include "user.h"
#include "fcntl.h"

#define max_queue_elements 100

int P = 25, P1 = 5, P2 = 15, P3 = 5;

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

void parseInput(){
    int fp;
    char line[10] = "          ";
	int len;

    fp = open("assig2b.inp", O_CREATE | O_RDWR);

    getline(line, &len, fp);
    P = atoi(line);
    getline(line, &len, fp);
    P1 = atoi(line);
    getline(line, &len, fp);
    P2 = atoi(line);
    getline(line, &len, fp);
    P3 = atoi(line);

    printf(1, "Inp : %d, %d, %d, %d\n", P, P1, P2, P3);
}

int main(int argc, char *argv[])
{
    parseInput();
	int i = 0,j = 0;
    int proc_pids[5][5];
    int size = 0;
    int pid;
    int parent_pid = getpid();
    int wait_time = 0;
    int num_procs_done = 0;
    // int set[9];

    barrier_init(P);

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
    // printf(1, "Found (%d, %d) as my pid = %d\n", i, j, getpid());

    // Set my wait_time
    if(size*i + j < P1 + P2)
        wait_time = 200;

    // Send request to quorum - msg = my pid
    for(int p = 0; p < size; p++){
        for(int q = 0; q < size; q++){
            if(p == i && q == j){
                for(int x = 0; x < size; x++){
                    for(int y = 0; y < size; y++){
                        if(x == i || y == j){
                            send(getpid(), proc_pids[x][y], &proc_pids[i][j]);
                            // printf(1, "Request(%d) to %d\n", getpid(), proc_pids[x][y]);
                        }
                    }
                }
            }
            barrier();

        }
    }


    barrier();

    // Check all requests
    int temp;
    recv(&temp);
    send(getpid(), temp, &temp);
    // printf(1, "Reply to(%d) by %d\n", temp, getpid());
    for(int p = 0; p < 2*size - 2; p++){
        recv(&temp);
        if(temp == -1)
            break;
        enque(temp);
        // printf(1, "Enq %d by %d\n", temp, getpid());
    }

    barrier();

    // Wait for replies from all procs in quorum - msg = my pid
    int num = 0;
    while(1){
        recv(&temp);
        if(temp == -1)
            continue;
        else if(temp == getpid())
            num++;
        else if(temp == 100){
            temp = deque();
            send(getpid(), temp, &temp);
            num_procs_done++;
            // printf(1, "Reply(%d) by %d\n", temp, getpid());
        }
        else{
            enque(temp);
        }

        // printf(1, "Pid %d num = %d\n", getpid(), num);
        
        if(num == (2*size - 1))
            break;
    }

    if(size*i + j < P1)
        goto nosend;

    // Execute critical section
    printf(1, "%d acquired the lock at time %d\n", getpid(), uptime());
    sleep(wait_time);
    printf(1, "%d released the lock at time %d\n", getpid(), uptime());

    nosend:

    temp = 100; // 100 signifies release
    proc_pids[0][0] = parent_pid;

    // Send release to quorum
    for(int x = 0; x < size; x++){
        for(int y = 0; y < size; y++){
            if(x == i || y == j){
                send(getpid(), proc_pids[x][y], &temp);
                // printf(1, "Release to %d\n", proc_pids[x][y]);
            }
        }
    }

    // Send replies to others
    while(1){
        recv(&temp);
        if(temp == -1)
            continue;
        else if(temp == 100){
            temp = deque();
            send(getpid(), temp, &temp);
            num_procs_done++;
        }
        // printf(1, "Numprocs done at %d is %d\n", proc_pids[i][j], num_procs_done);

        if(num_procs_done >= 2*size - 1)
            break;
    }


    if(getpid() == parent_pid){
        for(int i = 0; i < P-1; i++)
            wait(); // wait for all child procs to exit
    }
	
	exit();
}
