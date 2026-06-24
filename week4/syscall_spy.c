#include "kernel/types.h"
#include "user/user.h"

// Keep this manual declaration if your user.h is still missing it
// int sleep(int); 

int main(void) {
    int initial_pid = getpid();
    printf("Syscall Spy Report for PID %d:\n", initial_pid);

    // 1. getpid()
    printf("getpid() -> %d\n", getpid());

    // 2. uptime()
    printf("uptime() -> %d\n", uptime());

    // 3. fork()
    int child_pid = fork();

    if (child_pid < 0) {
        printf("fork() failed\n");
        exit(1);
    }

    if (child_pid == 0) {
        // --- CHILD BRANCH ---

        int my_pid = getpid();
        printf("--- in child (PID %d) ---\n", my_pid);
        printf("getpid() -> %d\n", my_pid);
        
        exit(0);
    } else {
        // --- PARENT BRANCH ---
        printf("fork() -> %d (child PID)\n", child_pid);

        // Wait for the child process to finish and exit before continuing
        int reaped_pid = wait(0);

        printf("--- back in parent (PID %d) ---\n", initial_pid);
        printf("wait() -> %d (reaped child PID %d)\n", reaped_pid, child_pid);

        int sleep_ret = sleep(10);
        printf("sleep(10) -> %d\n", sleep_ret);

        // 5. write()
        int write_ret = write(1, "Hello, world!\n", 14);
        printf("write(1,...) -> %d\n", write_ret);

        // 6. dup()
        int dup_ret = dup(0);
        printf("dup(0) -> %d\n", dup_ret);

        exit(0);
    }
}