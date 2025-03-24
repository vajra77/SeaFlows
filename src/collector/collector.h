//
// Created by Francesco Ferreri (Namex) on 29/11/24.
//

#ifndef COLLECTOR_H
#define COLLECTOR_H

#include "queue/queue.h"
#include "matrix/matrix.h"

typedef struct collector_data {
  int port;
  char *address;
  queue_t *queue;
  matrix_t *matrix;
} collector_data_t;

void* collector_thread(void *);

#endif //COLLECTOR_H
