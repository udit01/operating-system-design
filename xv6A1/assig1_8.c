#include "types.h"
#include "stat.h"
#include "user.h"
#define ARR_SIZE 1000
#define NPROCS 4

short arr[ARR_SIZE];
int parent_pid;
int num;

void signal_handler(int* mean){
	printf(1, "Interrupt msg received : %d\n", (int)mean);
	printf(1, "Mean : %d, %d\n", mean, arr[num*ARR_SIZE/NPROCS]);
	int varsum = 0;
	for(int i = num*ARR_SIZE/NPROCS; i < (num+1)*ARR_SIZE/NPROCS; i++){
		varsum += ((int)mean - arr[i])*((int)mean - arr[i]);
	}
	send(getpid(), parent_pid, &varsum);
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

	char c;
	int fd = open(filename, 0);
	printf(1, "filename %s", filename);
	for(int i=0; i<ARR_SIZE; i++){
		read(fd, &c, 1);
		arr[i]= c - '0';
		read(fd, &c, 1);
	}	
  	close(fd);
  	// this is to supress warning
  	printf(1,"first elem %d\n", arr[0]);

	// for(int i = 0; i < ARR_SIZE; i++){
	// 	printf(1,"%d\n", arr[i]);
	// }

	float mean;
  
  	//----FILL THE CODE HERE for unicast sum and multicast variance

	int child_pids[NPROCS];

	parent_pid = getpid();
	int child_flag = 1;

	int pid = -2;
	for(int i = 0; i < NPROCS; i++){
		num= i;
		pid = fork();
		if (pid != 0)
			child_pids[i] = pid;
		else
			goto child;
		printf(1, "Init %d\n", pid);
	}

	child_flag = 0;
	// dps();

	child:
	if(child_flag == 1){
		// Child process
		sigset((sig_handler)&signal_handler);
		printf(1, "Child proc %d\n", getpid());
		int partial_sum = 0;
		for(int i = num*ARR_SIZE/NPROCS; i < (num+1)*ARR_SIZE/NPROCS; i++){
			partial_sum += arr[i];
		}
		printf(1, "Partial sum from proc %d is %d\n", num, partial_sum);
		send(getpid(), parent_pid, &partial_sum); 
		while(1){;}
		printf(1, "Exiting child!\n");
		exit();
	}
	else{
		printf(1, "Parent proc %d", getpid());
		// int to;

		int total = 0;
		int temp = 0;
		// Get results
		for(int i = 0; i < NPROCS; i++){
			while(recv(&temp) == -1){};
			total += temp;
			printf(1, "Got results1 %d from %d so total = %d\n", temp, i, total);
		}
		mean = total / ARR_SIZE;
		printf(1, "Mean = %d\n", (int)mean);
		tot_sum = total;

		int a = mean;

		for(int i = 0; i < NPROCS; i++){
			send(parent_pid, child_pids[i], &a);
		}
	// ?	int p = send_multi(parent_pid, child_pids, &a, NPROCS);
		// printf(1, "Result of send multi = %d\n", p);

		float total2 = 0;
		int temp2 = 0;
		// Get results
		for(int i = 0; i < NPROCS; i++){
			while(recv(&temp2) == -1){};
			total2 += temp2;
			printf(1, "Got results2 %d from %d so total = %d\n", temp2, i, total2);
		}

		variance = total2 / ARR_SIZE;

		for(int i = 0; i < NPROCS; i++){
			wait();
		}
	}


	
  	//------------------

  	if(type==0){ //unicast sum
		printf(1,"Sum of array for file %s is %d\n", filename,tot_sum);
	}
	else{ //mulicast variance
		printf(1,"Variance of array for file %s is %d\n", filename,(int)variance);
	}
	exit();
}
