//
// Created by Francesco Ferreri (Namex) on 13/12/24.
//

#include <stdlib.h>
#include <pthread.h>
#include "matrix.h"

unsigned int matrix_hash(const char *address) {
 	return 0;
}

void matrix_init(matrix_t *matrix) {
  	pthread_mutex_init(&(matrix->lock), NULL);
  	matrix->sources = NULL;
  	matrix->size = 0;
}

void matrix_destroy(matrix_t *matrix) {
    while(matrix->sources != NULL) {
      	srcnode_t *src = matrix->sources;
      	while(src->destinations != NULL) {
          	dstnode_t *dst = src->destinations;
          	src->destinations = src->destinations->next;
        	free(dst);
      	}
    	matrix->sources = matrix->sources->next;
        free(src);
    }
}

void matrix_add_flow(matrix_t *matrix, storable_flow_t *flow) {
	pthread_mutex_lock((&(matrix->lock)));

    unsigned int src_key = matrix_hash(flow->src_mac);
    unsigned int dst_key = matrix_hash(flow->dst_mac);

	int src_found = 0;
    srcnode_t	*src_ptr = matrix->sources;

    while(src_ptr != NULL) {
    	if(src_key == src_ptr->key){
			src_found = 1;
    	}
        src_ptr = src_ptr->next;
    }

    if(src_found) {
    	int dst_found = 0;
        dstnode_t *dst_ptr = src_ptr->destinations;

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
