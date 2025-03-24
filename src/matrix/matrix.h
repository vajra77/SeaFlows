//
// Created by Francesco Ferreri (Namex) on 13/12/24.
//

#ifndef MATRIX_H
#define MATRIX_H

#include "sflow/sflow.h"


typedef struct destination_node {
	char mac[MAC_ADDR_SIZE];
	unsigned int key;
	unsigned int bytes_v4;
	unsigned int bytes_v6;
	unsigned int bytes_nk;
	struct destination_node *next;
} dstnode_t;

typedef struct source_node {
	char mac[MAC_ADDR_SIZE];
    unsigned int bytes_v4;
    unsigned int bytes_v6;
	unsigned int bytes_nk;
	unsigned int num_destinations;
    struct destination_node  *destinations;
    struct source_node *next;
} srcnode_t;

typedef struct matrix {
	pthread_mutex_t lock;
	unsigned int dirty;
	unsigned int num_sources;
  	struct source_node *sources;
} matrix_t;

void matrix_init(matrix_t *matrix);
void matrix_destroy(matrix_t *matrix);
void matrix_clear(matrix_t *matrix);
void matrix_dump(matrix_t *matrix);
void matrix_add_flow(matrix_t *matrix, const storable_flow_t *flow);
#endif //MATRIX_H
