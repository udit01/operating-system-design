#include "types.h"
#include "stat.h"
#include "user.h"

#define MSGSIZE 8

char* message ;

void interrupt_handler(int from_pid, int value){
    printf(1, "Inside process = %d, Interrupted by = %d, with value = %d. \n", getpid(), from_pid, value);
    
    //copy message from where?
    // Pull out message from stack or something ?
    // memmove(message, , MSGSIZE);
    
}

int main(void)
{
	printf(1,"IPC Test case for multiple send & receive by the interrupt handling model.\n");
	
    sigset( interrupt_handler );
    message = (char*)malloc(MSGSIZE);
    message = "\0";    
    int recvs[2];
    // int cid1, cid2 ;
    if( (recvs[0] = fork()) == 0 ){
        //child process 1
        while(strcmp(message,"\0") == 0);
        printf(1, "In pid : %d, global message value : %s, msg (virt) address %d.\n", getpid(), message, message );
        exit();
        // After interrupt should come here?
        
    }
    else{
        if( (recvs[1] = fork()) == 0 ){
            //child process 2
            while(strcmp(message,"\0") == 0);
            printf(1, "In pid : %d, global message value : %s, msg (virt) address %d.\n", getpid(), message, message );
            exit();
        }
        else{
            //send messages here
            char *msg_to_send = (char *)malloc(MSGSIZE);
		    msg_to_send = "MultMsg\0";
            send_multi(getpid(), recvs, msg_to_send, 2);
            wait(); // wait for all children to exit?or 1 child to exit ?

        }
    }

	
	exit();
}