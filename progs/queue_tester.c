#include <stdio.h>
#include <stdlib.h>

#include <uthread.h>
#include <queue.h>

int iterate_handler(void *data, void *arg)
{
	puts((char *)data);
	return 0;
}
    
int main(void)
{
    printf("* Begin Testing *\n\n");

	char *item1 = "item1";
	char *item2 = "item2";
	char *item3 = "item3";

	queue_t q = queue_create();

	if (q == NULL) {
		puts("queue_create(): failed\n");
		return -1;
	} else {
		puts("queue_create(): success\n");
	}

	int returnCode = 0;
	int count = 0;

	if (queue_length(q) != count) {
		puts("\nqueue_length(): failed");
		return -1;
	} else {
		printf("\nqueue_length(q) is: %d\n", queue_length(q));
	}
	
	returnCode = queue_enqueue(q, item1);
	if (returnCode == -1) {
		puts("\nqueue_enqueue(): failed");
	} else {
		puts("\nqueue_enqueue(): success.");
		count++;
		printf("count = %d\n", count);
	}

	if (queue_length(q) != count) {
		puts("\nqueue_length(): failed");
		return -1;
	} else {
		printf("\nqueue_length(q) is: %d\n", queue_length(q));
	}

	returnCode = queue_enqueue(q, item2);
	if (returnCode == -1) {
		puts("\nqueue_enqueue(): failed");
	} else {
		puts("\nqueue_enqueue(): success.");
		count++;
		printf("count = %d\n", count);
	}

	if (queue_length(q) != count) {
		puts("\nqueue_length(): failed");
		return -1;
	} else {
		printf("\nqueue_length(q) is: %d\n", queue_length(q));
	}

	returnCode = queue_enqueue(q, item3);
	if (returnCode == -1) {
		puts("\nqueue_enqueue(): failed");
	} else {
		puts("\nqueue_enqueue(): success.");
		count++;
		printf("count = %d\n", count);
	}

	if (queue_length(q) != count) {
		puts("\nqueue_length(): failed");
		return -1;
	} else {
		printf("\nqueue_length(q) is: %d\n", queue_length(q));
	}

	printf("\n\nqueue_iterate(): there are %d items in the queue", count);
	puts("\n* begin iterate: *");
	queue_iterate(q, iterate_handler, NULL, NULL);
	puts("* end iterate * ");

	void *data;

	count--;
	if (queue_dequeue(q, &data) != 0 || data != item1 || queue_length(q) != count ) {
		puts("\n\nqueue_dequeue(): failed");
		return -1;
	}
	else
	{
		puts("\n\nqueue_dequeue(): success");
		printf("count = %d\n", count);
	}

	if (queue_length(q) != count) {
		puts("\nqueue_length(): failed");
		return -1;
	} else {
		printf("\nqueue_length(q) is: %d\n", queue_length(q));
	}

	printf("\n\nqueue_iterate(): there are %d items in the queue", count);
	puts("\n* begin iterate: *");
	queue_iterate(q, iterate_handler, NULL, NULL);
	puts("* end iterate * ");

	if (queue_destroy(q) == -1) {
		printf("\nqueue_destroy failed: queue is either unempty or NULL");
	} else
	{
		printf("\nqueue_destroy success: queue was either empty");
	}	

	count--;
	if (queue_delete(q, item2) != 0 || queue_length(q) != count) {
		puts("\nqueue_delete failed");
		return -1;
	} else {
		puts("\n\nqueue_delete(): success");
	}

	if (queue_length(q) != count) {
		puts("\nqueue_length(): failed");
		return -1;
	} else {
		printf("\nqueue_length(q) is: %d\n", queue_length(q));
	}

	printf("\n\nqueue_iterate(): there are %d items in the queue", count);
	puts("\n* begin iterate: *");
	queue_iterate(q, iterate_handler, NULL, NULL);
	puts("* end iterate * ");

	count--;
	if (queue_dequeue(q, &data) != 0 || data != item3 || queue_length(q) != count) {
		puts("queue_dequeue(): failed");
		return -1;
	} 
	else
	{
		puts("\n\nqueue_dequeue(): success");
		printf("count = %d\n", count);
	}

	if (queue_length(q) != count) {
		puts("\nqueue_length(): failed");
		return -1;
	} else {
		printf("\nqueue_length(q) is: %d\n", queue_length(q));
	}

	if (queue_destroy(q) == -1) {
		printf("\nqueue_destroy() failed: queue is either unempty or NULL");
	} else
	{
		printf("\nqueue_destroy() success: queue was empty and thus deletable");
	}	

	printf("\n\nqueue_iterate(): there are %d items in the queue", count);
	puts("\n* begin iterate: *");
	if (queue_length(q) != 0)
	{
		queue_iterate(q, iterate_handler, NULL, NULL);
	}
	puts("* end iterate * ");

	if (queue_length(q) != count) {
		puts("\nqueue_length(): failed");
		return -1;
	} else {
		printf("\nqueue_length(q) is: %d\n", queue_length(q));
	}
	
	puts("\n\n** all tests passed **");
	
	return 0;
}

/*		sample output:
* Begin Testing *

queue_create(): success


queue_length(q) is: 0

queue_enqueue(): success.
count = 1

queue_length(q) is: 1

queue_enqueue(): success.
count = 2

queue_length(q) is: 2

queue_enqueue(): success.
count = 3

queue_length(q) is: 3


queue_iterate(): there are 3 items in the queue
* begin iterate: *
item1
item2
item3
* end iterate * 


queue_dequeue(): success
count = 2

queue_length(q) is: 2


queue_iterate(): there are 2 items in the queue
* begin iterate: *
item2
item3
* end iterate * 

queue_destroy failed: queue is either unempty or NULL

queue_delete(): success

queue_length(q) is: 1


queue_iterate(): there are 1 items in the queue
* begin iterate: *
item3
* end iterate * 


queue_dequeue(): success
count = 0

queue_length(q) is: 0

queue_destroy() success: queue was empty and thus deletable

queue_iterate(): there are 0 items in the queue
* begin iterate: *
* end iterate * 

queue_length(q) is: 0


** all tests passed **

*/