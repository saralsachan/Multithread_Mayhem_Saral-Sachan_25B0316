#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

int
main(void)
{
    printf("Testing getsyscount()\n");

    getpid();
    uptime();

    int count = getsyscount();

    printf("System calls made = %d\n", count);

    exit(0);
}