//
// Created by Francesco Ferreri (Namex) on 13/12/24.
//

#ifndef MATRIX_H
#define MATRIX_H

#include <pthread.h>
#include "sflow/sflow.h"

typedef struct destination_node {
  unsigned int hash;
  void *data;
  struct destination_node *next;
} dstnode_t;

typedef struct source_node {
  unsigned int hash;
  struct destination_node  *destinations;
  struct source_node *next;
} srcnode_t;

typedef struct matrix {
  pthread_mutex_t *lock;
  struct source_node *sources;
  int size;
} matrix_t;

matrix_t *matrix_create(int size);
void matrix_destroy(matrix_t *matrix);
void matrix_clear(matrix_t *matrix);
int matrix_insert(matrix_t *matrix, storable_flow_t *flow);
#endif //MATRIX_H
