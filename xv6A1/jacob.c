// MOD-2 : Parallel implementation of jacobi
#include "types.h"
#include "stat.h"
#include "user.h"
#include "fcntl.h"

int N = 10, P = 2, L = 100000;
float T = 100.0, EPSILON = 0.01;

double fabs(double a){
	if(a > 0)
		return a;
	return -a;
}

void parseInput(){
    int fp;
    char line[10] = "          ";
	int len;

    fp = open("assig2a.inp", O_CREATE | O_RDWR);

    getline(line, &len, fp);
    N = atoi(line);
    getline(line, &len, fp);
    EPSILON = atof(line);
    getline(line, &len, fp);
    T = atof(line);
    getline(line, &len, fp);
    P = atoi(line);
    getline(line, &len, fp);
    L = atoi(line);

    // printf(1, "Inp : %d, %d, %d, %d, %d\n", N, (int)(1000*EPSILON), (int)T, P, L);
	// printfloat(1, T);
	// printfloat(1, EPSILON);
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
	int num;
	int prev = 0, next = 0;
	float diff_temp = 0;
	int first = 0, last = 0;

	int start = uptime();

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
		// printf(1, "Init proc %d with pid %d\n", i, proc_pids[i]);
	}

	child_flag = 0;
	num = 0;
	// dps();

	child:
	if(child_flag == 1){
		// Child process
		proc_pids[num] = getpid();
		prev = proc_pids[num - 1];
		if(num != procs-1)
			recv(&next);
	}
	else{
		// Master sends the next proc pid
		for(i = 1; i < procs-1; i++)
			send(proc_pids[0], proc_pids[i], &proc_pids[i+1]);
		
		// For master next = proc_pids[1]
		next = proc_pids[1];
	}

	first = num*(N-2)/procs + 1; last = (num+1)*(N-2)/procs;
	// printf(1, "Proc %d first %d last %d\n", proc_pids[num], first, last);

	// Parallelised jacobi method
	for(;;){
		diff = 0.0;
		// Share border results with next and prev
		if(num != procs-1)
			for(j = 1; j < N; j++)
				send(proc_pids[num], next, &u[last][j]);
		
		if(num != 0)
			for(j = 1; j < N; j++)
				recv(&u[first - 1][j]);
	
		if(num != 0)
			for(j = 1; j < N; j++)
				send(proc_pids[num], prev, &u[first][j]);
		
		if(num != procs-1)
			for(j = 1; j < N; j++)
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
			send(proc_pids[num], proc_pids[0], &diff);
			recv(&diff);
		}
		else{
			for(j = 1; j < procs; j++){
				recv(&diff_temp);
				if(diff_temp > diff)
					diff = diff_temp;
			}
			diff = (count >= L) ? 0 : diff;
			for(j = 1; j < procs; j++)
				send(proc_pids[0], proc_pids[j], &diff);
		}

		if(diff <= EPSILON) break;
		for (i =1; i< N-1; i++)	
			for (j =1; j< N-1; j++) u[i][j] = w[i][j];
	}

	// printf(1, "Work done by %d\n", proc_pids[num]);
	
	// Printing matrix
	if(num == 0){
		for(i = 0; i <= last; i++){
			for(j = 0; j<N; j++){
				printf(1, "%d ", (int)u[i][j]);
			}
			printf(1,"\n");
		}
		int temp = 0;
		for(i = 1; i < procs; i++){
			send(proc_pids[0], proc_pids[i], &proc_pids[i+1]);
			recv(&temp);
			count += temp;
		}
		for(j = 0; j<N; j++){
			printf(1, "%d ", (int)u[N-1][j]);
		}
		printf(1, "\nNumber of iteration: %d\n",count);
		printf(1, "Time = %d Ticks\n", uptime() - start); 

		for(i = 0; i < procs - 1; i++)
			wait();
	}
	else{
		recv(&next);
		for(i = first; i <= last; i++){
			for(j = 0; j<N; j++){
				printf(1, "%d ", (int)u[i][j]);
			}
			printf(1,"\n");
		}
		send(proc_pids[num], proc_pids[0], &count);
	}
	
	exit();
}
