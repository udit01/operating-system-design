#include "types.h"
#include "stat.h"
#include "user.h"

#define NPROCS 2
#define SIZE 1000
short arr[SIZE];
int ppid;
int c_num;


void signal_handler(int* mean){
	// printf(1, "Interrupt msg received : %d\n", (int)mean);
	printf(1, "Mean : %d, %d\n", mean, arr[c_num*SIZE/NPROCS]);
	int varsum = 0;
	for(int i = c_num*SIZE/NPROCS; i < (c_num+1)*SIZE/NPROCS; i++){
		varsum += ((int)mean - arr[i])*((int)mean - arr[i]);
	}
	send(getpid(), ppid, &varsum);
	exit();
}	


int
main(int argc, char *argv[])
{
	if(argc< 2){
		printf(1,"Need type and input filename\n");
		exit();
	}
	char *filename;
	filename=argv[2];
	int type = atoi(argv[1]);
	printf(1,"Type is %d and filename is %s\n",type, filename);

	int tot_sum = 0;	
	float variance = 0.0;

	// int SIZE=1000;
	// short arr[SIZE];
	char c;
	int fd = open(filename, 0);
	for(int i=0; i<SIZE; i++){
		read(fd, &c, 1);
		// printf(1, "%d", c);
		arr[i]=c-'0';
		tot_sum += arr[i];
		read(fd, &c, 1);
	}	
  	close(fd);
  	// this is to supress warning
  	printf(1,"first elem %d\n", arr[0]);
  
  	//----FILL THE CODE HERE for unicast sum and multicast variance
	// float mean = 0.0;
	ppid = getpid();
	int child_pids[NPROCS];

	int pid = -2;
	for(int i = 0; i < NPROCS; i++){
		c_num= i;
		pid = fork();
		if (pid != 0){
			child_pids[i] = pid;
			printf(1, "CHILD PID : %d\n", child_pids[i]);
		}
		else{
			sigset((sig_handler)&signal_handler);
			int partial_sum = 0;
			for(int i = c_num*SIZE/NPROCS; i < (c_num+1)*SIZE/NPROCS; i++){
				partial_sum += arr[i];
			}
			send(getpid(), ppid, &partial_sum); 
			while(1);
			exit();
		}
	}


	int total = 0;
	int temp = 0;
	// Get results
	for(int i = 0; i < NPROCS; i++){
		// while(recv(&temp) == -1){};
		recv(&temp);
		printf(1, "temp = %d", temp);
		total += temp;
		// printf(1, "Got results1 %d from %d so total = %d\n", temp, i, total);
	}
	
	printf(1, "total = %d\n", total);
	// tot_sum = total;
  	//------------------

  	if(type==0){ //unicast sum
		printf(1,"Sum of array for file %s is %d\n", filename,tot_sum);
	}
	else{ //mulicast variance
		printf(1,"Variance of array for file %s is %d\n", filename,(int)variance);
	}
	exit();
}