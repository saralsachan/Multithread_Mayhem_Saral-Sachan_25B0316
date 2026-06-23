# Week 1 Report — Saral Sachan

## Machine
- OS: Ubuntu (WSL2 on Windows)
- CPU: 16
- gcc: AMD Ryzen 7 7435HS

## Problem 1 — Observations

### hello_thread
Ran the file multiple times. The order of output changed randomly in the runs.
Learning - The OS Schedules the threads in whatever order it likes. The user code cannot predict it.


### race_counter (ITERS=10000000, default, 10 runs)
Expected: 20000000
Actual values: 10808224, 10836572, 10817758 etc...
Ever correct? NO. Didn't get correct a single time !!!!

### race_counter — ITERS=1000
The values were far more correct with less deviation form actual values

### race_counter — ITERS=100000000
Values are too off. 

### parallel_sum (10 runs)
Expected: 10000000
Actual values:
  2732138, 3177313, 3111893, 2828071, 3118597,
  2800322, 2856392, 3108664, 2826111, 2816100
Range: 2.73M – 3.18M
Correct: 0/10

### bank_chaos (10 runs)
Unexpected outputs

## Problem 2 — Make the race worse

### What I changed
I manually split `counter++` into its three logical steps (LOAD into
a local `tmp`, ADD 1 to `tmp`, STORE `tmp` back to `counter`) and
inserted `usleep(1)` between the LOAD and the STORE. I also dropped
ITERS from 10,000,000 to 100,000 because each iteration now takes
~50µs instead of nanoseconds — otherwise the program would take 15
minutes to finish.

### Why this makes the race more visible
The original race exists because `counter++` is really three steps
and the OS can preempt a thread mid-sequence. But by default that
window is only nanoseconds wide, while OS scheduling happens at
~10ms granularity, so the race rarely "wins."

`usleep(1)` does two things that widen the window enormously:
1. It actually sleeps for ~50µs (kernel scheduling granularity).
2. It yields the CPU — the scheduler immediately runs the other thread.

So now, between Thread A's LOAD and Thread A's STORE, Thread B runs
its own LOAD of the same value. Both threads then ADD 1 to the same
value and STORE back the same result. One increment is lost on
*every* iteration, not just occasionally.

### Result
Across 5 runs:
Final counter:  100069
Final counter:  100264
Final counter:  100343
Final counter:  99935
Final counter:  100042
Expected: 200000 (= 2 * ITERS)
Observed: ~100000 (= ITERS) on every run
The threads are now stomping each other almost completely — close to
50% of all increments are lost, instead of the ~10-25% seen in the
unmodified race_counter.

### Insights from Problem 2
Race conditions are *probabilistic*, not deterministic. They need a
specific timing collision between threads, and how often that collision
happens depends entirely on how wide the load-modify-store window is.
By inserting `usleep(1)` between the load and the store, I stretched
that window from nanoseconds to ~50µs and forced the OS scheduler to
run the other thread inside it on nearly every iteration. The race
went from "rarely wins" to "wins almost every time" — without changing
the program's logic at all.

