## Purpose

This project compares types of lock-free Multi-Producer Multi-Consumer (MPMC) queue.

## Queues

###	Bounded Circular Buffer

A fixed-size ring buffer with atomic counters for head/tail and per-slot metadata.
Example: https://www.1024cores.net/home/lock-free-algorithms/queues/bounded-mpmc-queue

### Unbounded Linked List (Michael & Scott)

Classic unbounded queue with linked nodes. Each enqueue/dequeue is a CAS on pointers.
Paper: https://www.cs.rochester.edu/~scott/papers/1996_PODC_queues.pdf
