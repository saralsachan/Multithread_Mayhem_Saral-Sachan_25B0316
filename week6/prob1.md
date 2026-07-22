# Problem 1 – Scheduler Trace

## Checkpoint 1 – Process Creation

**File:** `kernel/proc.c`  
**Function:** `allocproc()`  


A new process is created when `fork()` calls `allocproc()`. The function scans the global `proc[]` table until it finds a process whose state is `UNUSED`. It acquires the process lock, allocates a PID, trapframe, page table, and kernel stack, initializes the process context, and prepares the process for execution. After `allocproc()` returns, the process has been allocated successfully but is not yet runnable.

---

## Checkpoint 2 – Becoming RUNNABLE

**File:** `kernel/proc.c`  
**Function:** `fork()`  


After the parent process has been copied successfully, `fork()` changes the child's state to `RUNNABLE`. This transition is performed while holding `p->lock` to prevent races with the scheduler or another CPU. Holding the lock guarantees that the scheduler cannot observe a partially initialized process.

---

## Checkpoint 3 – Scheduler Picks the Process

**File:** `kernel/proc.c`  
**Function:** `scheduler()`  


The scheduler executes in an infinite loop on every CPU. At the beginning of each iteration, interrupts are enabled so that timer interrupts can occur. The scheduler scans every process in the process table, acquires each process's lock individually, and checks whether its state is `RUNNABLE`. If so, it changes the state to `RUNNING`, records the process in the CPU structure, and performs a context switch using `swtch()`. After the process later yields the CPU, execution resumes immediately after the `swtch()` call, the scheduler clears `c->proc`, releases the process lock, and continues scanning the process table.

---

## Checkpoint 4 – Context Switch

**File:** `kernel/swtch.S`  
**Function:** `swtch()`  


The `swtch()` routine performs the actual context switch by saving the currently executing context and restoring another. It saves the following registers into the old context:

- `ra`
- `sp`
- `s0`–`s11`

It then restores the same registers from the new context. Since the return address (`ra`) is restored from the new context, the final `ret` instruction jumps directly to the instruction where that process last stopped executing. This allows execution to continue exactly where it previously yielded.

---

## Checkpoint 5 – Process Running

**File:** `kernel/proc.c`  
**Function:** `scheduler()` / CPU Context  


After the scheduler switches to a process, the process executes normally in kernel mode and eventually returns to user mode. During this time, the scheduler itself is not running. Its register state has been saved inside the CPU's `context` (`c->context`). The scheduler remains suspended until the running process voluntarily yields or is interrupted by the timer.

---

## Checkpoint 6 – Yielding the CPU

**Files:**
- `kernel/trap.c`
- `kernel/proc.c`

**Functions:** `usertrap()`, `yield()`, `sched()`  


When a timer interrupt occurs while a process is running, control enters `usertrap()`. Recognizing the timer interrupt, the kernel calls `yield()`. The process acquires its own lock, changes its state from `RUNNING` back to `RUNNABLE`, and invokes `sched()`. The `sched()` function performs another context switch by calling `swtch()`, saving the process's current register state and restoring the scheduler's saved context. As a result, execution resumes immediately after the previous `swtch()` inside the scheduler.

---

## Checkpoint 7 – Scheduler Continues

**File:** `kernel/proc.c`  
**Function:** `scheduler()`  


Once the scheduler regains control, it resumes immediately after the `swtch()` call. It clears the CPU's current process pointer (`c->proc`), releases the lock of the process that just yielded, and continues scanning the process table. The next process whose state is `RUNNABLE` is selected, marked as `RUNNING`, and another context switch occurs. This repeated cycle forms xv6's round-robin scheduling policy.

---

# Overall Scheduler Lifecycle

The complete scheduling path followed by a process is:
fork()
↓
allocproc()
↓
Process initialized
↓
fork() sets state = RUNNABLE
↓
scheduler() scans proc[]
↓
Scheduler finds RUNNABLE process
↓
State changes to RUNNING
↓
swtch(&c->context, &p->context)
↓
Process executes
↓
Timer interrupt
↓
usertrap()
↓
yield()
↓
sched()
↓
swtch(&p->context, &c->context)
↓
Scheduler resumes
↓
Scheduler selects next RUNNABLE process

## Observations

1. xv6 uses a simple round-robin scheduler that scans the process table from beginning to end.
2. Every access to a process's scheduling state is protected by `p->lock`, preventing race conditions on multicore systems.
3. Context switching does not copy the entire process; only the CPU register context is saved and restored.
4. The scheduler itself never disappears—it simply sleeps inside its saved CPU context until another process yields the processor.
5. Timer interrupts provide preemptive multitasking by forcing running processes to periodically return control to the scheduler.