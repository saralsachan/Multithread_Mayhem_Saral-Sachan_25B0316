# Problem 01 – Trace a System Call (`getpid()`)

## 1. User-space Stub

The user-space stub for `getpid()` is located in `user/usys.S`, which is automatically generated from `user/usys.pl`. Before entering the kernel, it loads the system call number (`SYS_getpid`) into register `a7`, while the function arguments are already placed in registers `a0`–`a5` following the RISC-V calling convention. It then executes the `ecall` instruction to request a kernel service.

## 2. The `ecall` Instruction

When the `ecall` instruction executes, the RISC-V hardware automatically switches the processor from User Mode (U-mode) to Supervisor Mode (S-mode). It saves the current program counter in the `sepc` register, records the trap reason (`Environment Call from U-mode`) in `scause`, and jumps to the address stored in the `stvec` register, which points to the kernel's trap entry code.

## 3. First xv6 Code: `uservec`

The first xv6 code executed after the trap is the `uservec` routine in `kernel/trampoline.S`. This routine saves all the CPU registers into the current process's trapframe so that the process state can be restored later. It then switches from the user page table to the kernel page table and transfers control to the `usertrap()` function.

## 4. Trap Handling

The `usertrap()` function in `kernel/trap.c` determines why the trap occurred. It checks whether `scause` is equal to `8`, which indicates a system call from user mode. If this condition is true, it advances the saved program counter so the `ecall` instruction is not executed again, enables interrupts, and calls the `syscall()` function.

## 5. Syscall Dispatch

The `syscall()` function in `kernel/syscall.c` reads the system call number from the `a7` field of the current process's trapframe. It uses this number to index the `syscalls[]` function-pointer table and invokes the corresponding kernel function. The return value of the system call is then stored in the `a0` field of the trapframe.

## 6. Actual Implementation

The implementation of `sys_getpid()` is located in `kernel/sysproc.c`. It is a very small function that simply returns the process ID of the currently running process using `myproc()->pid`. After returning, the value is stored in `trapframe->a0` by `syscall()`, and when execution returns to user mode, it appears in register `a0` as the return value of `getpid()`.

## 7. Returning to User Mode

After the system call completes, `usertrapret()` in `kernel/trap.c` prepares the process to return to user mode by restoring the trapframe and switching back to the user page table. It then jumps to `userret` in `kernel/trampoline.S`, which restores the remaining registers and executes the `sret` instruction. The `sret` instruction switches the processor back to User Mode, and the program resumes execution immediately after the original `ecall` instruction with the return value available in register `a0`.
