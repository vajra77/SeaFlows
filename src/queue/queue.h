//
// Created by Francesco Ferreri (Namex) on 13/12/24.
//

#ifndef QUEUE_H
#define QUEUE_H

#include <pthread.h>

typedef struct queue_node {
  struct queue_node *next;
  void *data;
} qnode_t;

typedef struct queue {
  struct queue_node *head;
  struct queue_node *tail;
  int size;
  pthread_mutex_t lock;
} queue_t;

void queue_init(queue_t *queue);
void queue_destroy(queue_t *queue);
void queue_push(queue_t *queue, void *data);
void *queue_pop(queue_t *queue);
#endif //QUEUE_H
