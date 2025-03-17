//
// Created by Francesco Ferreri (Namex) on 13/12/24.
//

#ifndef MATRIX_H
#define MATRIX_H

#include <pthread.h>
#include "sflow/sflow.h"

typedef struct destination_node {
  unsigned int key;
  unsigned int bytes_v4;
  unsigned int bytes_v6;
  struct destination_node *next;
} dstnode_t;

typedef struct source_node {
  unsigned int key;
  unsigned int bytes_v4;
  unsigned int bytes_v6;
  struct destination_node  *destinations;
  struct source_node *next;
} srcnode_t;

typedef struct matrix {
	pthread_mutex_t lock;
  	struct source_node *sources;
	int size;
} matrix_t;

void matrix_init(matrix_t *matrix);
void matrix_destroy(matrix_t *matrix);
void matrix_clear(matrix_t *matrix);
void matrix_add_flow(matrix_t *matrix, storable_flow_t *flow);
#endif //MATRIX_H
