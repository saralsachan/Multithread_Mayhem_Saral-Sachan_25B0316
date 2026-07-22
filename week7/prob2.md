Q: For an ordinary process created by plain fork() (no threads involved), what should getthreadinfo() report for tgid relative to pid?

Answer:

For a normal process, is_thread is initialized to 0, indicating it is not a thread. The tgid is initialized to the process's own pid, making it the leader of its own one-member thread group. Therefore, after fork(), both the parent and child report tgid == pid, but the child has a different pid (and therefore a different tgid) from the parent. This confirms that no address-space sharing or thread grouping exists yet.