#include "user.h"
#include "types.h"
#include "stat.h"

#define MSGSIZE 8

// char* message ;
// int sys_send_multi(int, int*, void*, int);

void interrupt_handler(char *m_){
    // printf(1, "Inside process = %d, Interrupted by = %d, with value = %d. \n", getpid(), from_pid, value);
    printf(1, "Message(in interrupt) = %d \n", (int)m_);
    
    //copy message from where?
    // Pull out message from stack or something ?
    // memmove(message, , MSGSIZE);
    exit();
}

int main(void)
{
    // char * message;
	printf(1,"IPC Test case for multiple send & receive by the interrupt handling model.\n");
	
    sigset( &interrupt_handler );
    // message = (char*)malloc(MSGSIZE);
    // message = "\0";    
    int recvs[2];
    // int cid1, cid2 ;
    if( (recvs[0] = fork()) == 0 ){
        //child process 1
        // while(strcmp(message,"\0") == 0);
        // printf(1, "In pid : %d, global message value : %s, msg (virt) address %d.\n", getpid(), message, message );
        sigset( &interrupt_handler );    
        while(1);
        printf(1, "EXITING CHILD 1");
        exit();
        // After interrupt should come here?
        
    }
    else{
        if( (recvs[1] = fork()) == 0 ){
            //child process 2
            // while(strcmp(message,"\0") == 0);
            // printf(1, "In pid : %d, global message value : %s, msg (virt) address %d.\n", getpid(), message, message );
            sigset( &interrupt_handler );            
            while(1);
            printf(1, "EXITING CHILD 2");
            exit();
        }
        else{
            //send messages here
            char *msg_to_send = (char *)malloc(MSGSIZE);
		    msg_to_send = "MultMsg\0";
            printf(1,"In parent, msg to send : %d", (int)msg_to_send);
            send_multi(getpid(), (int*)recvs, (void*)msg_to_send, 2);
            
            wait();
            wait();
            printf(1, "EXITING PARENT");
        }
    }

	
	exit();
}