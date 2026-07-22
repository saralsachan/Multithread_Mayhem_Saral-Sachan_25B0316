#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

int
main(void)
{
    int p1 = fork();

    if(p1 == 0){

        while(1){
            getpid();
        }
    }

    int p2 = fork();

    if(p2 == 0){

        while(1){
            getpid();
        }
    }

    set_priority(p1, 5);
    set_priority(p2, 50);

    printf("Child %d priority = %d\n", p1, get_priority(p1));
    printf("Child %d priority = %d\n", p2, get_priority(p2));

    sleep(100);

    kill(p1);
    kill(p2);

    wait(0);
    wait(0);

    exit(0);
}