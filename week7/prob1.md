# Thread Design Report (`thread_design.md`)

# Problem 1 – Address-Space Design Trace

## 1. Understanding `uvmcopy()`

**File:** `kernel/vm.c`
**Function:** `uvmcopy()`
The `uvmcopy()` function is responsible for creating a new address space for a child process during `fork()`. It iterates through every mapped virtual page in the parent's address space, from address `0` up to `sz`.

For each mapped page, it performs three important operations:

### (a) Allocate a new physical page

A new physical memory page is allocated using `kalloc()`. This ensures that the child receives its own independent physical memory rather than sharing the parent's memory.



---

### (b) Copy the page contents

After allocating the page, the kernel copies the entire contents of the parent's physical page into the newly allocated page using `memmove()`. As a result, the child initially sees exactly the same memory contents as the parent.


---

### (c) Install a new page-table entry

Finally, the new physical page is mapped into the child's page table using `mappages()`. The same permission bits (read, write, execute, user permissions) are preserved from the parent's page table.

---

These three steps ensure that every user page in the child has an independent physical copy. After `fork()`, both processes contain identical memory contents, but modifying memory in one process does not affect the other because they no longer share physical pages. This behavior is the correct design for independent processes. The theory also emphasizes that `fork()` creates a private copy of every mapped page rather than sharing memory.

---

# 2. What happens if `uvmcopy()` fails?

**File:** `kernel/proc.c`
**Function:** `fork()`


Inside `fork()`, after allocating a new process, the kernel attempts to duplicate the parent's address space by calling `uvmcopy()`. Since this operation involves allocating many pages, it may fail if memory cannot be allocated.

If `uvmcopy()` returns a negative value, `fork()` immediately cleans up every resource that has already been allocated for the child process. The partially created process is freed using the normal cleanup path before returning an error to the caller.

This demonstrates an important kernel design principle:

**The function that performs a multi-step operation is also responsible for cleaning up partially completed work if any step fails.**

Without this cleanup, the kernel would leak memory pages, page tables, trapframes, or process table entries. Every allocation performed before the failure must therefore be undone before returning.

This pattern appears throughout operating systems because many kernel operations cannot be completed atomically.

---

# 3. Design: Sharing an Address Space Instead of Copying

This question asks how a thread implementation would differ from the current `fork()` implementation.

## (a) Replacing `uvmcopy()`

Instead of creating a completely new address space using `uvmcopy()`, the new thread should simply reuse the parent's page table.

Rather than copying every page, the child process would receive the exact same `pagetable` pointer as its parent. This means both processes reference the same virtual memory mappings and therefore see identical physical memory.

No page allocation or page copying would occur during thread creation.

---

## (b) Freeing the page table safely

If multiple threads share one page table, freeing it when any one thread exits would immediately invalidate the address space for every remaining thread.

To avoid this problem, the shared page table should maintain a reference count.

The reference count is increased whenever another thread begins sharing the address space.

It is decreased whenever a thread exits.

Only when the reference count becomes zero should the kernel actually call `proc_freepagetable()` and release the memory.

A suitable implementation would store this reference count in a shared kernel object that every thread referencing the page table can access. The reference count must be updated while holding an appropriate lock so that two threads exiting simultaneously cannot accidentally free the page table twice.

---

## (c) Values of `p->sz` and `p->pagetable`

Since every thread shares one address space, every thread should describe the same amount of user memory.

Therefore:

* `p->sz` should represent the same address-space size for every thread.
* `p->pagetable` should be exactly the same pointer value as the parent's page table.

Creating a separate page-table pointer that refers to copied mappings would defeat the purpose of threads because memory updates would no longer be shared.

The theory explains that a thread differs from a process mainly because it shares its address space while keeping its own registers, stack pointer, and scheduler state.

---

# 4. Why every thread cannot have its own `p->sz`

If two threads share one page table but maintain different values of `p->sz`, several problems can occur.

First, system calls such as `copyin()` and `copyout()` validate user addresses using `p->sz`. One thread might believe an address is valid while another believes the same address is outside the process.

Second, one thread could increase the shared address space using `sbrk()`, but another thread whose `p->sz` was not updated would still believe the old address-space limit is correct. Valid memory could therefore be rejected by argument validation.

The opposite situation is even more dangerous. If one thread has a larger `p->sz` than actually exists, it may pass validation checks and attempt to access unmapped memory, leading to faults or undefined behavior.

Because all threads operate inside the same shared address space, the address-space size must also be shared. Every thread should observe exactly the same memory boundaries, ensuring that memory allocation, argument validation, and page-table management remain consistent.

---

# Overall Understanding

The current xv6 implementation creates completely independent processes by copying every page of memory during `fork()`. This approach is ideal for processes because changes made by one process never affect another.

Threads require different behavior. Every thread should have its own execution state—including registers, stack, and scheduling information—but should share the same address space, heap, global variables, and open files with other threads in the same program.

Supporting this design requires replacing page-table copying with page-table sharing, maintaining a shared reference count for safe deallocation, and ensuring that all threads observe the same address-space size. The scheduler itself does not require any modification because it already schedules individual `struct proc` entries independently, regardless of whether they represent processes or threads.
