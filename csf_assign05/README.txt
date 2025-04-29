Sample README.txt

Eventually your report about how you implemented thread synchronization
in the server should go here

In our server, the one piece of truly shared state is the map of active chat rooms (m_rooms), so we guard it with a POSIX mutex (pthread_mutex_t m_lock) and wrap every lookup or 
insertion in a small‐scope RAII “Guard” that locks on construction and unlocks on destruction. Each Room object also carries its own mutex so that its members set can be safely 
modified by multiple threads in add_member, remove_member, and broadcast_message. Finally, our per-user MessageQueue pairs a mutex (to protect its internal deque) with a counting 
semaphore (initialized to zero) so that enqueue() simply pushes and sem_post()s, while dequeue() does a timed sem_timedwait() then pops under lock—this lets receivers block 
efficiently waiting for new messages without spinning.