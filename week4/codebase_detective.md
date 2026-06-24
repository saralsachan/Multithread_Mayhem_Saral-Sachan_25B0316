### (A) Process States
- The five active operational states are - SLEEPING, RUNNABLE, RUNNING, ZOMBIE, UNUSED
- The process in right after fork() returns a child is RUNNABLE.The child is fully set up and placed in the queue, waiting for the scheduler to allocate CPU time to it.
- The state of process between exit() ans wait() is ZOMBIE. A zombie process is a terminated process that remains in the system's process table because its parent process hasn't read its exit status.The process has halted execution and freed most of its memory, but its entry in the process table cannot be recycled until the parent reaps it via wait().

### (B) The boot process
- List of initialization steps executed in main() in order- consoleinit() -> printfinit() -> kinit() -> kvminit() -> kvminithart() -> procinit() -> trapinit() -> trapinithart() -> plicinit() -> plicinithart() -> binit() -> inint() -> fileinit() -> virtio_disk_init() -> userinit()
- console(consoleinit()) subsystem is initialised first
- the first user process(userinit()) is initialised last
- scheduler() - It runs an infinite loop that scans the process table looking for a process in the RUNNABLE state. When it finds one, it changes the state to RUNNING, performs a context switch to load the process's registers/page table, and hands the CPU over to execute it.

### (C) The system call table
- 21 standard system calls
- The array maps the system call numbers directly to their corresponding function pointers. The macros defined in syscall.h (e.g., #define SYS_fork 1) are used as the exact array indices (e.g., [SYS_fork] sys_fork), allowing for fast O(1) lookups when a trap occurs.
- No specific fallback function is called. The syscall() handler catches the out-of-bounds number, prints an error message to the console ("unknown sys call %d\n"), and returns -1 to the user program by setting the a0 register.

### (D) The shell
- The shell uses fork() to spawn a child process. Inside the child branch, it calls runcmd(), which parses the command structure and uses exec() to replace the child's memory with the new executable. Meanwhile, the parent shell process uses wait() to block until the child completes.
- Conceptually, the shell links the output of the left command to the input of the right command
