## Implementation Notes

### Phase 1: Queue API

For this phase, we decided to implement a doubly linked list over the
conventional singly linked list, for the sake of practice. For this project,
the complexity between the two would be the same, the only difference is that
the doubly linked list would require more lines of code in order to manage
the two pointers for each node, and thus, a higher memory footprint. 

For example, everytime a new node is enqueued or dequeued, both the pointers
"previous" and "next" would need to be changed, but the operations are still 
O(1) since the location of the enqueuing and dequeueing is known, via the 
"head" and "tail" pointers.

However, the main difference in using the doubly linked list occurs at the
queue_delete() function. If the singly linked list were to be used, the 
"previous" node would need to be kept tracked of while the list is being
traversed. In the doubly linked list, the "previous" pointer is already built
in. So, it's really just a different way of writing the code. The complexity
here would be the same in both approaches, O(n), since we don't exactly know
where the node is in the list.

#### Testing

The project prompt recommended unit testing, but we tried something different.
We simply created a queue and checked that it was created successfully. Next,
we enqueued items, and checked that the dequeued items match those items
that were enqueued. We wrote an iterate function, which used queue_iterate()
to check that the items in the queue would update accordingly after each
enqueue or dequeue, and that the number of items in the queue match the
expected number. We also tried to destroy the queue, which should only work
if the queue was non-empty or non-null, otherwise it would return a -1.
Basically, we made sure each function works or fails as expected, using
printf(), in order to cover as many test cases as possible.

### Phase 2: uthread API

The thread control block can be represented by a struct. A few globals are
used: one for holding the current running thread, and three different
queues for representing the different states of a thread.

#### uthread_create()

If ran for the first time, this function initializes the uthread library by:
* Creating the three queues: "ready", "blocked", and "done".
* Initializes the main thread which has a TID of 0 and sets its properties.

If ran on subsequent times, this function:
* Creates a new thread.
* Sets the thread's properties.
* Initializes the thread's execution context.
* Enqueues the thread into "ready" queue.
* Return the thread's TID.

Noteworthy properties: If a thread is "thread a", it can use the property
which stores the TID of the thread it is waiting for. If a thread is
"thread b", it can use the property which stores the TID of the thread it
is blocking. The tid_search() function, queue_iterate(), and these proprties
can help search for threads that are either blocked or needs to be unblocked.

#### uthread_yield()

This function, if the queue is non-empty:
* Dequeues the next thread from "ready" queue.
* Enqueues the current thread into "ready" queue.
* Context switches between current thread and next thread.

#### uthread_self()

This function simply returns thread_running->tid.

#### uthread_exit(), phase 2 version

(This commented-out code is included in the uthread_exit() fuction.)

Since phase 2 does not use the concept of blocked threads yet, the main thread
needs a way to retrieve the return value from the most recently exited
thread. 

This can be done by have a separate queue in which only the main thread 
exists at all times. Everytime uthread_exit() is called, main thread is
dequeued from this "separate" queue, retrieves the return value, and is
re-enqueued into the "separate" queue.

Next, uthread_exit() context switches to the next thread at the head of
the "ready" queue.

#### uthread_join(), phase 2 version

(This commented-out code is included in the uthread_join() fuction.)

In an infinite loop, repeat until the last thread in the "ready" queue:
* Main thread is enqueued into "separate" queue.
* Yield to next thread at head of "ready" queue. Once it finishes, it
will pass its return value to main thread in that "separate" queue, and
re-enqueue the main thread into the "ready" queue. Control switches to thread
at head of "ready" queue.
* Eventually, control returns back to main thread at uthread_join(), and
*retval, if not NULL, is assigned, and main thread is dequeued from "separate"
queue.

#### Testing

All tests in uthread_hello.c and uthread_yield.c passed.

### Phase 3: uthread_join()

This phase concerns primarily uthread_exit() and uthread_join(). How this phase
was implemented was by running through what happens on each step in each
of the two scenarios, and then writing it as it goes.

#### Scenario 1: A joins B, but B hasn't terminated yet.

* A calls uthread_join(B). Enter the uthread_join() function.
* Since B was not found in queue_done, then B has not terminated yet. Thus,
B is somewhere in queue_ready.
* Continuously dequeue and enqueue queue_ready until B is dequeued. If this
process repeats > queue_length(queue) times, then B does not exist.
* Store B's TID into A and store A's TID into B.
* Context switch between A into B.
* B terminates by calling uthread_exit(). It will find A inside queue_blocked,
dequeues it, pass A its return value, and re-enqueues A into queue_ready.
Finally, B context switches into next thread at head of "ready" queue.
* Eventually, control returns to A inside uthread_join(), and if *retval is
not null, it gets assigned the return value.
* A exits uthread_join() and resumes its next line.

#### Scenario 2: A joins B, but B has already terminated.

This implies that B has called uthread_exit() before A has called 
uthread_join(B).

* B runs.
* B terminates by calling uthread_exit().
* Inside the function, it will iterate through queue_blocked, but will not
find any such A, since A hasn't uthread_join(B) yet. 
* B stores its return value into its property and gets enqueued into
queue_done.
* B context switches into the next thread at head of queue_ready.
* Sometime in the future, when A calls uthread_join(B), it enters the join
function. A will find, in queue_done, B, so A will collect B right away,
exit join(), and resume its next line.

### Phase 4: preemption

Not implemented.

## External Sources Used To Understand Project 2
* https://www.geeksforgeeks.org/doubly-linked-list/
* https://www.geeksforgeeks.org/queue-linked-list-implementation/
* https://www.codesdope.com/blog/article/making-a-queue-using-linked-list-in-c/
* https://www.cs.bu.edu/teaching/c/queue/linked-list/funcs.html
* https://www.cs.ubc.ca/~tmm/courses/213-12F/slides/213-2b.pdf
* https://pages.mtu.edu/~shene/NSF-3/e-Book/FUNDAMENTALS/thread-management.html
* https://www.cs.cmu.edu/afs/cs/academic/class/15492-f07/www/pthreads.html