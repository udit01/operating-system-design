#include "types.h"
#include "stat.h"
#include "user.h"


void usage(char* name){
    printf(0, "Usage : %s <int 1> <int 2>\nls", name);    
}

int main(int argc, char**argv){
    if(argc < 3){
        usage(argv[0]);
        exit();
    }
    int a = atoi(argv[1]);
    int b = atoi(argv[2]);

    int out = add(a, b);
    
    printf(0, "Added %d and %d : Output = %d\n", a, b, out);
    exit();
}