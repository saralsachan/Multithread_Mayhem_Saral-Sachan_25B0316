#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

int
main(void)
{
    struct thread_info info;

    if(getthreadinfo(&info) < 0){
        printf("getthreadinfo failed\n");
        exit(1);
    }

    printf("Before fork\n");
    printf("pid=%d tgid=%d is_thread=%d\n",
            info.pid,
            info.tgid,
            info.is_thread);

    int pid = fork();

    if(pid == 0){

        getthreadinfo(&info);

        printf("Child\n");

        printf("pid=%d tgid=%d is_thread=%d\n",
                info.pid,
                info.tgid,
                info.is_thread);

        exit(0);
    }

    getthreadinfo(&info);

    printf("Parent\n");

    printf("pid=%d tgid=%d is_thread=%d\n",
            info.pid,
            info.tgid,
            info.is_thread);

    wait(0);

    exit(0);
}