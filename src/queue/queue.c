//
// Created by Francesco Ferreri (Namex) on 13/12/24.
//

#include <stdlib.h>
#include <syslog.h>
#include <gc.h>

#include "queue.h"


void queue_init(queue_t *queue) {
  queue->head = NULL;
  queue->tail = NULL;
  queue->size = 0;
  pthread_mutex_init(&(queue->lock), NULL);
}

void queue_destroy(queue_t *queue) {
  for (; queue->head != NULL; queue->head = queue->head->next) {
    GC_free(queue->head);
  }
  pthread_mutex_destroy(&(queue->lock));
  GC_free(queue);
}

void queue_push(queue_t *queue, void *data) {

  qnode_t *new_node = GC_malloc(sizeof(qnode_t));
  new_node->data = data;
  new_node->next = NULL;

  pthread_mutex_lock(&queue->lock);
  if (queue->size == 0) {
    queue->head = new_node;
    queue->tail = new_node;
    queue->size = 1;
  } else {
    queue->tail->next = new_node;
    queue->tail = new_node;
    queue->size++;
  }
  pthread_mutex_unlock(&queue->lock);
  syslog(LOG_DEBUG, "Added data to queue");
}

void *queue_pop(queue_t *queue) {

  void    *data = NULL;
  qnode_t *temp = NULL;

  syslog(LOG_DEBUG, "Removing data from queue");
  pthread_mutex_lock(&queue->lock);
  switch (queue->size) {

    case 0:
      break;

    case 1:
      temp = queue->head;
      data = queue->head->data;
      queue->head = NULL;
      queue->tail = NULL;
      queue->size = 0;
      break;

    default:
      temp = queue->head;
      data = queue->head->data;
      queue->head = queue->head->next;
      queue->size--;
      break;
  }
  pthread_mutex_unlock(&queue->lock);

  if (temp != NULL) {
    GC_free(temp);
  }
  return data;
}