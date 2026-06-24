#include "kernel/types.h"
#include "user/user.h"

#define SYSCALL_COUNT 23

int main(int argc, char *argv[]) {
    // 1. Print the greeting
    printf("Hello, xv6! This is Saral.\n");

    // 2. Print the number of arguments passed to it.
    // We subtract 1 from argc because the command name itself (argv[0]) doesn't count towards the user-provided arguments in the expected output
    printf("Number of arguments: %d\n", argc - 1);

    // 3. Print each argument on its own line.
    for(int i = 1; i < argc; i++) {
        printf("arg %d: %s\n", i, argv[i]);
    }

    // Print the system call count.
    printf("xv6 knows %d system calls.\n", SYSCALL_COUNT);

    // Every program must explicitly exit.
    exit(0);
}