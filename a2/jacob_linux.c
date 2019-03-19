#include <stdio.h> 
#include <sys/ipc.h> 
#include <sys/msg.h> 
#include <math.h>
#include <stdlib.h>
#include <time.h> 

int N, T, P, L;
float EPSILON;

int msgids[8] = {0};
int num;

// structure for message queue 
struct mesg_buffer { 
    long mesg_type; 
    char mesg_text[100]; 
} message; 

void send(int sender_pid, int receiver_index, float * value)
{
    message.mesg_type = 1; 
    // printf("Sending %f to %d from %d\n", *value, receiver_index, sender_pid);
    sprintf(message.mesg_text, "%f", *value);
    int msgid = msgids[receiver_index];
    msgsnd(msgid, &message, sizeof(message), 0);
}

void recv(float *value)
{
    int msgid = msgids[num];
    // printf("Got value %f by %d\n", atof(message.mesg_text), getpid());
    msgrcv(msgid, &message, sizeof(message), 1, 0); 
    *value = atof(message.mesg_text);
}

void parseInput(){
    FILE * fp;
    char * line = NULL;
    size_t len = 0;
    ssize_t read;

    fp = fopen("assig2a.inp", "r");

    getline(&line, &len, fp);
    N = atoi(line);
    getline(&line, &len, fp);
    EPSILON = atof(line);
    getline(&line, &len, fp);
    T = atof(line);
    getline(&line, &len, fp);
    P = atoi(line);
    getline(&line, &len, fp);
    L = atoi(line);

    printf("Inp : %d, %f, %d, %d, %d\n", N, EPSILON, T, P, L);

    if (line)
        free(line);
}

int main(int argc, char *argv[])
{
	parseInput();
	float diff;
	int i,j;
	float mean = 0.0;
	float u[20][20];
	float w[20][20];
	int count;
	int procs = P;
	int proc_pids[procs];
	proc_pids[0] = getpid(); // Master proc
	int child_flag = 1;
	int pid = -1;
	int prev = 0, next = 0;
	float diff_temp = 0;
	int first = 0, last = 0;
    char fname[2] = "p0";
    // printf("Start\n");
    for(int i = 0; i < procs; i++){
        fname[1] = i + '0';
        msgids[i] = msgget(ftok(fname, 65), 0666 | IPC_CREAT); // Creates msg queue and returns identifier for each proc
    }

    // printf("Time\n");

	clock_t start_t, end_t, total_t;
    start_t = clock();

	// Initialise u matrix
	for (i = 0; i < N; i++){
		u[i][0] = u[i][N-1] = u[0][i] = T;
		u[N-1][i] = 0.0;
		mean += u[i][0] + u[i][N-1] + u[0][i] + u[N-1][i];
	}

	// Set mean
	mean /= (4.0 * N);

	// Set u matrix
	for (i = 1; i < N-1; i++ )
		for ( j= 1; j < N-1; j++) u[i][j] = mean;

	// Initialise procs - master has num = 0
	for(i = 1; i < procs; i++){
		num = i;
		pid = fork();
		if (pid != 0)
			proc_pids[i] = pid;
		else
			goto child;
		// printf("Init proc %d with pid %d\n", i, proc_pids[i]);
	}

	child_flag = 0;
	num = 0;

	child:
    prev = num - 1;
    next = num + 1;
    proc_pids[num] = getpid();

	first = num*(N-2)/procs + 1; last = (num+1)*(N-2)/procs;
	// printf("Proc %d pid %d first %d last %d prev %d next %d\n", num, proc_pids[num], first, last, prev, next);

	// Parallelised jacobi method
	for(;;){
		diff = 0.0;
		// Share border results with next and prev
		if(num != procs-1)
			for(j = 1; j < N-1; j++)
				send(proc_pids[num], next, &u[last][j]);
		
		if(num != 0)
			for(j = 1; j < N-1; j++)
				recv(&u[first - 1][j]);
	
		if(num != 0)
			for(j = 1; j < N-1; j++)
				send(proc_pids[num], prev, &u[first][j]);
		
		if(num != procs-1)
			for(j = 1; j < N-1; j++)
				recv(&u[last + 1][j]);

		// Compute jacobi for submatrix
		for(i = first; i <= last; i++){
			for(j =1 ; j < N-1; j++){
				w[i][j] = ( u[i-1][j] + u[i+1][j] + u[i][j-1] + u[i][j+1])/4.0;
				if( fabs(w[i][j] - u[i][j]) > diff )
					diff = fabs(w[i][j] - u[i][j]);
				count++;	
			}
		}

		// Send difference
		if(num != 0){
			send(proc_pids[num], 0, &diff);
			recv(&diff);
		}
		else{
			for(j = 1; j < procs; j++){
				recv(&diff_temp);
				if(diff_temp > diff)
					diff = diff_temp;
			}
			diff = (count >= L) ? 0 : diff; // if hard count limit reached then diff = 0
			for(j = 1; j < procs; j++)
				send(proc_pids[0], j, &diff);
		}

		if(diff <= EPSILON) break;
		for (i =1; i< N-1; i++)	
			for (j =1; j< N-1; j++) u[i][j] = w[i][j];
	}

	// printf("Work done by %d \n", num, proc_pids[num]);
	
	// Printing matrix
	if(num == 0){
		for(i = 0; i <= last; i++){
			for(j = 0; j<N; j++){
				printf("%d ", (int)u[i][j]);
			}
			printf("\n");
		}
		int temp = 0;
		for(i = 1; i < procs; i++){
			send(proc_pids[0], i, &proc_pids[i+1]);
			recv(&temp);
			count += temp;
		}
		for(j = 0; j<N; j++){
			printf("%d ", (int)u[N-1][j]);
		}
		printf("\nNumber of iteration: %d\n",count);
        end_t = clock();
        double totaltime = (double)(end_t - start_t) * 1000 / CLOCKS_PER_SEC;
		printf("Time = %f ms\n", totaltime); 

		for(i = 0; i < procs - 1; i++)
			wait();
	}
	else{
		recv(&next);
		for(i = first; i <= last; i++){
			for(j = 0; j<N; j++){
				printf("%d ", (int)u[i][j]);
			}
			printf("\n");
		}
		send(proc_pids[num], 0, &count);
	}
	
	exit(0);
}
