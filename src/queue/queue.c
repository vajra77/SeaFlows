//
// Created by Francesco Ferreri (Namex) on 13/12/24.
//

#include <stdlib.h>
#include "queue.h"

void queue_init(queue_t *queue) {
  queue->head = NULL;
  queue->tail = NULL;
  queue->size = 0;
  pthread_mutex_init(&(queue->lock), NULL);
}

void queue_destroy(queue_t *queue) {
  for (; queue->head != NULL; queue->head = queue->head->next) {
    free(queue->head);
  }
  pthread_mutex_destroy(&(queue->lock));
  free(queue);
}

void queue_push(queue_t *queue, void *data) {
  pthread_mutex_lock(&(queue->lock));

  qnode_t *new_node = malloc(sizeof(qnode_t));
  new_node->data = data;
  new_node->next = NULL;

  queue->tail->next = new_node;
  queue->tail = new_node;
  queue->size++;

  pthread_mutex_unlock(&(queue->lock));
}

void *queue_pop(queue_t *queue) {
  pthread_mutex_lock(&(queue->lock));

  void *data = queue->head->data;
  qnode_t *temp = queue->head;

  if (queue->head == queue->tail) {
    queue->head = NULL;
    queue->tail = NULL;
    queue->size = 0;
  }
  else {
    queue->head = queue->head->next;
    queue->size--;
  }

  free(temp);

  pthread_mutex_unlock(&(queue->lock));
  return data;
}