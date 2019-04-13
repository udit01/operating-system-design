#include "types.h"
#include "stat.h"
#include "user.h"

#define fabs(a) (a>=0) ? a : -a

#define N 10
#define EPSILON 0.01
#define procs 2

int parent_pid;
int num;
int parta = 0;
int partb = 0;

int main(int argc, char *argv[])
{
	// double diff;
	// int i,j;
	// double mean;
	// double u[N][N];
	// double w[N][N];
	int child_pids[procs];
    
	int num;
	
	// mean = 0.0;

    barrier_init(procs + 1);

	parent_pid = getpid();
	int child_flag = 1;

	int pid = -2;
	for(int i = 0; i < procs; i++){
		num= i;
		pid = fork();
		if (pid != 0)
			child_pids[i] = pid;
		else
			goto child;
		printf(1, "Init %d\n", pid);
	}

	child_flag = 0;
	dps();

	child:
    if(child_flag == 1){
		// Child process
		printf(1, "Child proc %d, num %d\n", getpid(), num);
		parta = getpid();
		partb = 2;
		printf(1, "Updated parta to %d\n", parta);
        barrier();
		send(getpid(), parent_pid, &parta);
		printf(1, "Exiting child!\n");
		exit();
	}
    else{
        int a = 1;
        barrier();
        printf(1, "a = %d\n", a);
		recv(&a);
        printf(1, "childs %d, %d return = %d, %d\n", child_pids[0], child_pids[1], a, partb);
		wait(); wait();
    }
}
