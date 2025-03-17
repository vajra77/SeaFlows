//
// Created by Francesco Ferreri (Namex) on 13/12/24.
//

#include "matrix.h"

typedef struct matrix {
	pthread_mutex_t *lock;
	struct source_node *sources;
	int size;
} matrix_t;

void matrix_init(matrix_t *matrix) {
  	pthread_mutex_init((&(matrix->lock), NULL);
  	matrix->sources = NULL;
  	matrix->size = 0;
}

void matrix_destroy(matrix_t *matrix) {
    while(matrix->sources != NULL) {
      	src = matrix->sources;
      	while(src->destinations != NULL) {
          	dst = src->destinations;
          	src->destinations = src->destinations->next;
          	free(dst->data);
        	free(dst);
      	}
    	matrix->sources = matrix->sources->next;
        free(src);
    }
}

void matrix_add_flow(matrix_t *matrix, storable_flow_t *flow) {
	pthread_mutex_lock((&(matrix->lock)));

    src_key = matrix_hash(flow->src_mac);
    dst_key = matrix_hash(flow->dst_mac);

	src_found = 0;
    src_ptr = matrix->sources;

    while(src_ptr != NULL) {
    	if(src_key == ptr->key){
			src_found = 1;
    	}
        src_ptr = src_ptr->next;
    }

    if(src_found) {
    	dst_found = 0;
        dst_ptr = src_ptr->destinations;

        while(dst_ptr != NULL) {
        	if(dst_key == dst_ptr->key){

        	}
            dst_ptr = dst_ptr->next;
        }

        if(dst_found) {

        }
        else {

        }
    }
    else {

    }

    pthread_mutex_unlock((&(matrix->lock)));
}
