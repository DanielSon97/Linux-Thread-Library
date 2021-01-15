/*
 * Simple hello world test
 *
 * Tests the creation of a single thread and its successful return.
 */

#include <stdio.h>
#include <stdlib.h>

#include <uthread.h>

int hello(void* arg)
{
	printf("Entering thread Hello(): Hello world! I will be returning '100' back to main!\n");
	printf("Exiting thread Hello()...\n");

	return 100;
}

int main(void)
{
	uthread_t tid;
	int returnValue;

	tid = uthread_create(hello, NULL);

	printf("Exiting Main()...\n");
	uthread_join(tid, &returnValue);
	printf("Entering Main()...\n");

	printf("Main(): The return value received from thread Hello() is: %d\n", returnValue);

	return 0;
}
