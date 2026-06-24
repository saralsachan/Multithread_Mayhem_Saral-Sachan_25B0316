#include "kernel/types.h"
#include "user/user.h"

int main(int argc, char *argv[]){
    int n = 5;

    if(argc>1){
        n = atoi(argv[1]);
    }
    int parent_pid = 2;

    for(int currDepth = 0; currDepth<n; currDepth++){
        printf("Depth: %d, PID: %d, Parent PID: %d\n", currDepth, getpid(), parent_pid);

        if(currDepth == n-1){
            printf("I am the leaf node, I have no more children.\n");
            break;
        }
        parent_pid = getpid();
        int pid = fork();
        if(pid<0){ //OS ran out of memory
            printf("Fork invalid\n");
            exit(1);
        }
        else if(pid>0){ 
            wait(0);
            exit(0);
        }
        //child branch pid == 0
    }
    printf("Total process chain: %d\n", n);
    exit(0);
}