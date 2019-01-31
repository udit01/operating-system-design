#include "types.h"
#include "user.h"
/* 
 * note that the print statements allow ONLY the parent
 * process to be traced. See below comments for the amount
 * of system calls, and the print statements will print
 * out the correct number during run time.
 *
 */
int main()
{
	// 2 calls sbrk and exec on process starting

	// 1 call
	int pid = getpid();
	
	// starting the trace : 1
	trace(1);
	
	// 3 sys calls
	fork();
	fork();
	wait();
	
	// 1 sys call
	if(getpid() == pid){
		// 9 (include trace(0))	
		printf(1, "total syscalls so far: %d\n", trace(0)); 
		// 26 more from print (2 digits for num)
	}

	// 2 
	fork();
	wait();
	
	// 1 
	if(getpid() == pid){
		// 39 (38 + trace(0))
		printf(1, "totalsys calls so far: %d\n", trace(0)); 
	}
	exit();
}
