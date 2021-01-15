#include <assert.h>
#include <signal.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <stdbool.h>
#include <limits.h>

#include "context.h"
#include "preempt.h"
#include "queue.h"
#include "uthread.h"

typedef struct tcb_obj {
	uthread_ctx_t ctx;
	uthread_t tid;
	void *stack;
	uthread_func_t func;
	void *arg;
	int retval;
	uthread_t tid_thread_a_blocked;
	uthread_t tid_thread_b_waiting_for;
} tcb_obj_t;

bool initialization = false;
tcb_obj_t *thread_running;
queue_t queue;
queue_t queue_blocked;
queue_t queue_done;
uthread_t thread_id = 0;

/*
 * @thread: the current thread in the queue
 * @thread_id: the tid of @thread of interest
 * 
 * This function checks if @threads->tid is equivalent to @thread_id.
 *
 * Return 1 if equivalent, then @data in queue_iterate() will 
 * receive thread_in_queue. Return 0 otherwise.
*/
static int tid_search(void *thread, void *thread_id) {
	tcb_obj_t *thread_in_queue = (tcb_obj_t *)thread;
	uthread_t *tid_to_search_for = (uthread_t *)thread_id;

	if (thread_in_queue->tid == *tid_to_search_for) {
		return 1;
	} else {
		return 0;
	}
}

/*
 * 1. dequeue next thread from queue_ready
 * 2. enqueue current thread to queue_ready
 * 3. switch(current thread, next thread)
*/
void uthread_yield(void)
{
	tcb_obj_t *thread_next;

	// only yield if queue_dequeue() does not return -1
	if (queue_dequeue(queue, (void **)(&thread_next)) == 0) {
		// move current thread to end of queue_ready
		queue_enqueue(queue, (void *)(thread_running));

		// context switch between "current" and "next" threads
		tcb_obj_t *thread_prev = thread_running;
		thread_running = thread_next;
		uthread_ctx_switch(&thread_prev->ctx, &thread_running->ctx);
	}
}

/*
 * returns TID of currently running thread
*/
uthread_t uthread_self(void)
{
	return thread_running->tid;
}

/*
 * creates a new thread and enqueues it into queue_ready.
 * if ran for the first time, initializes the uthread library.
*/
int uthread_create(uthread_func_t func, void *arg)
{
	if (initialization == false) {
		// create the queues
		queue = queue_create();
		queue_blocked = queue_create();
		queue_done = queue_create();

		// return -1 if any queue were created unsuccessfully
		if (queue == NULL || queue_blocked == NULL || queue_done == NULL) {
			return -1;
		}

		// allocate space for main thread
		thread_running = (tcb_obj_t*)malloc(sizeof(tcb_obj_t));

		// return -1 if allocation error
		if (thread_running == NULL) {
			return -1;
		}

		// set the properties of main thread
		thread_running->tid = thread_id++;
		thread_running->tid_thread_a_blocked = -1;
		thread_running->tid_thread_b_waiting_for = -1;

		// dont run initialization on subsequent calls to uthread_create()
		initialization = true;
	}

	// create new thread
	tcb_obj_t *thread_new = (tcb_obj_t*)malloc(sizeof(tcb_obj_t));

	// return -1 if new thread was created unsuccessfully
	if (thread_new == NULL) {
		return -1;
	}

	// set the member fields
	thread_new->tid = thread_id++;
	thread_new->func = func;
	thread_new->arg = arg;
	thread_new->stack = uthread_ctx_alloc_stack();
	thread_new->tid_thread_a_blocked = -1;
	thread_new->tid_thread_b_waiting_for = -1;

	// return -1 if stack was created unsuccessfully
	if (thread_new->stack == NULL) {
		return -1;
	}

	// initialize thread's execution context
	int returnValue_init = 0;
	returnValue_init = uthread_ctx_init(&thread_new->ctx, (void*)(thread_new->stack), thread_new->func, thread_new->arg);

	// return -1 if thread execution context was initialized unsuccessfully
	if (returnValue_init == -1) {
		return -1;
	}

	// enqueue newly created thread into queue_ready
	queue_enqueue(queue, (void*)(thread_new));

	// return TID
	return thread_new->tid;
}

// exiting threads will implicity call uthread_ext()
void uthread_exit(int retval)
{
	/***** begin phase 2 testing code *****
	tcb_obj_t *thread_main;
	if (queue_dequeue(queue_blocked, (void **)(&thread_main)) == 0)
	{
		// have the main() thread collect the exiting thread's return value
		thread_main->retval = retval;

		// re-enqueue the main thread into queue_blocked
		queue_enqueue(queue_blocked, (void*)(thread_main));
	}
	*****end phase 2 testing code*****/

	// thread_b checks for thread_a in queue_blocked
	tcb_obj_t* thread_a = NULL;
	queue_iterate(queue_blocked, tid_search, (void*) (uthread_t*)(&(thread_running->tid_thread_a_blocked)), (void**)&thread_a);
	if (thread_a != NULL) {
		// thread_b's return value is passed to thread_a
		thread_a->retval = retval;

		// delete thread_a from queue_blocked
		queue_delete(queue_blocked, (void*)(thread_a));

		// thread_a is no longer waiting for thread_b
		thread_a->tid_thread_b_waiting_for = -1;

		// re-enqueue thread_a into queue_ready
		queue_enqueue(queue, (void*)(thread_a));
	}

	// thread_b is no longer blocking thread_a
	thread_running->tid_thread_a_blocked = -1;

	// thread_b stores its own return value
	thread_running->retval = retval;

	// enqueue thread_b into queue_done
	queue_enqueue(queue_done, (void*)(thread_running));

	// context switch between "previous" and "next" threads
	tcb_obj_t *thread_next;
	queue_dequeue(queue, (void **)(&thread_next));
	tcb_obj_t *thread_prev = thread_running;
	thread_running = thread_next;
	uthread_ctx_switch(&thread_prev->ctx, &thread_running->ctx);
}

/*
 * blocks calling thread until target thread completes
*/
int uthread_join(uthread_t tid, int *retval)
{
	// return -1 if errors
	if (thread_id == 0 || thread_id == thread_running->tid) {
		return -1;
	}

	// thread_a checks for thread_b in queue_done
	tcb_obj_t* thread_b = NULL;
	queue_iterate(queue_done, tid_search, (void*) (uthread_t*)(&tid), (void**)&thread_b);

	if (thread_b != NULL) {
		// case 1: thread_b has already terminated. thread_a will collect
		// return value, clean up, exit join(), and resume exection.

		tcb_obj_t *thread_b_terminated = (tcb_obj_t *)thread_b;

		if (retval != NULL) {
			*retval = (int)(uthread_t)thread_b_terminated->retval;
		}

		uthread_ctx_destroy_stack(thread_b_terminated->stack);
		free(thread_b_terminated);

		return 0;
	} else {
		/*
		case 2: thread_a wants to join thread_b but thread_b isn't done.
		this implies that thread_b is somewhere in queue_ready
		*/

		tcb_obj_t *thread_at_head_of_list;
		int counter_q_dq = 0;

		// continuously dequeue and re-enqueue until we obtain thread_b
		while(1) {
			if (queue_dequeue(queue, (void **)(&thread_at_head_of_list)) == 0) {
				if (tid == thread_at_head_of_list->tid) {
					// block thread_a. thread_b will unblock it upon exiting.
					queue_enqueue(queue_blocked, (void*)thread_running);

					// store thread_a's tid into thread_b
					thread_at_head_of_list->tid_thread_a_blocked = thread_running->tid;

					// store thread_b's tid into thread_a
					thread_running->tid_thread_b_waiting_for = thread_at_head_of_list->tid;

					// context switch between thread_a and thread_b
					tcb_obj_t *thread_prev = thread_running;
					thread_running = thread_at_head_of_list;
					uthread_ctx_switch(&thread_prev->ctx, &thread_running->ctx);

					// eventually control will return to thread_a.
					// when thread_b exited, it passed its retval to thread_a
					if (retval != NULL) {
						*retval = thread_running->retval;
					}

					// thread_a has completed in joining with thread_b,
					// and thread_a will now exit uthread_join().
					break;
				} else {
					// incorrect thread dequeued.
					// re-enqueue it & repeat process.
					queue_enqueue(queue, (void *)(thread_at_head_of_list));
				}
			}

			// at the end of each queue + dequeue process, increase counter
			counter_q_dq++;

			// thread_b does not exist if we have done the queue + dequeue 
			// process more than the number of items in the queue
			if (counter_q_dq >= queue_length(queue)) {
				return -1;
			}
		}
	}

	// thread_a exits join() and resumes execution
	return 0;

	/***** begin phase 2 testing code *****
	while(1) {
		queue_enqueue(queue_blocked, (void *)(thread_running));

		if (queue_length(queue) >= 1) {
			uthread_yield(); 
		} else {
			break;
		}

		if (retval != NULL)
		{	
			tcb_obj_t *thread_main;
			if (queue_dequeue(queue_blocked, (void **)(&thread_main)) == 0)
			{
				*retval = thread_main->retval;
			}
		}
		else
		{
			tcb_obj_t *thread_main;
			queue_dequeue(queue_blocked, (void **)(&thread_main));
		}
	}
	***** end phase 2 testing code *****/
}