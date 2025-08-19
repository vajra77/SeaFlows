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
int queue_size(queue_t *);
void queue_enqueue(queue_t *, void *);
void *queue_dequeue(queue_t *);
void *queue_get_head(queue_t *);
void queue_del_head(queue_t *);
#endif //QUEUE_H
