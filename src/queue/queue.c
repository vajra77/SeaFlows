//
// Created by Francesco Ferreri (Namex) on 18/08/25.
//

#include "queue.h"
#include "memory.h"

void queue_init(struct queue *queue) {
	queue->head = NULL;
	queue->tail = NULL;
	queue->size = 0;
	pthread_mutex_init(&(queue->mutex), NULL);
}

void queue_push(queue_t *queue, void *data) {
	pthread_mutex_lock(&(queue->mutex));
	node_t *new_node = MEM_alloc(sizeof(node_t));

	new_node->data = data;
	new_node->next = NULL;

	if (queue->head == NULL) {
		queue->head = new_node;
		queue->tail = new_node;
		queue->size = 1;
	}
	else {
		queue->tail->next = new_node;
		queue->tail = new_node;
		queue->tail->next = NULL;
		queue->size++;
	}
	pthread_mutex_unlock(&(queue->mutex));
}

void *queue_pop(queue_t *queue) {
	pthread_mutex_lock(&(queue->mutex));

	if (queue->head == NULL) {
		pthread_mutex_unlock(&(queue->mutex));
		return NULL;
	}

	void *data = queue->head->data;
	node_t *temp = queue->head;
	queue->head = queue->head->next;
	queue->size--;
	MEM_free(temp);
	pthread_mutex_unlock(&(queue->mutex));
	return data;
}