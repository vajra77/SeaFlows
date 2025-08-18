//
// Created by Francesco Ferreri (Namex) on 18/08/25.
//

#ifndef QUEUE_H
#define QUEUE_H

#include <pthread.h>

typedef struct node {
	void *data;
    struct node *next;
} node_t;

typedef struct queue {
	int size;
	node_t *head;
    node_t *tail;
    pthread_mutex_t mutex;
} queue_t;

void queue_init(queue_t *);
void queue_push(queue_t *, void *);
void *queue_pop(queue_t *);
#endif //QUEUE_H
