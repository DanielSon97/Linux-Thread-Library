#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "queue.h"

struct queue_item_t {
	struct queue_item_t *next;
	struct queue_item_t *prev;
	void *data;
};

struct queue {
	struct queue_item_t *head;
	struct queue_item_t *tail;
	int count;
};

queue_t queue_create(void)
{
	struct queue *result = malloc(sizeof (struct queue));

	if (result != NULL) {
		result->head = NULL;
		result->tail = NULL;
		result->count = 0;
	}

	return result;
}

/*
 * queue_destroy - Deallocate a queue
 * @queue: Queue to deallocate
 *
 * Deallocate the memory associated to the queue object pointed by @queue.
 *
 * Return: -1 if @queue is NULL or if @queue is not empty. 0 if @queue was
 * successfully destroyed.
 */
int queue_destroy(queue_t queue)
{
	if (queue == NULL || queue->count > 0) {
		return -1;
	}

	free(queue);

	return 0;
}

int queue_enqueue(queue_t queue, void *data)
{
	if (queue == NULL || data == NULL) {
		return -1;
	}

	struct queue_item_t *item = malloc(sizeof (struct queue_item_t));

	item->data = data;
	item->next = NULL;

	if (NULL == item) {
		return -1;
	}

	if (NULL == queue->head) {
		item->prev = NULL;
		queue->head = item;
		queue->tail = item;
	}
	else {
		queue->tail->next = item;
		item->prev = queue->tail;
		queue->tail = item;
	}

	queue->count++;

	return 0;
}

int queue_dequeue(queue_t queue, void **data)
{
	if (NULL == queue || NULL == data || 0 == queue->count) {
		return -1;
	}

	struct queue_item_t *item = queue->head;

	*data = item->data;

	if (1 == queue->count) {
		queue->head = NULL;
		queue->tail = NULL;
	}
	else {
		queue->head = queue->head->next;
		queue->head->prev = NULL;
	}

	free(item);
	queue->count--;

	return 0;
}

int queue_delete(queue_t queue, void *data)
{
	if (queue == NULL || data == NULL) {
		return -1;
	}

	struct queue_item_t *item = queue->head;

	while (item != NULL) {
		if (item->data == data) {
			break;
		}

		item = item->next;
	}

	if (item != NULL) {
		if (item->prev != NULL) {
			item->prev->next = item->next;
		}
		else {
			queue->head = item->next;
		}

		if (item->next != NULL) {
			item->next->prev = item->prev;
		}
		else {
			queue->tail = item->prev;
		}

		free(item);
		queue->count--;
	}

	return (NULL == item ? -1 : 0);
}

int queue_iterate(queue_t queue, queue_func_t func, void *arg, void **data)
{
	if (queue == NULL || func == NULL) {
		return -1;
	}

	struct queue_item_t *item = queue->head;

	while (item != NULL) {
		if (func(item->data, arg) != 0) {
			if (data != NULL) {
				*data = item->data;
			}

			break;
		}

		item = item->next;
	}

	return 0;
}

int queue_length(queue_t queue)
{
	if (NULL == queue) {
		return -1;
	}

	return queue->count;
}