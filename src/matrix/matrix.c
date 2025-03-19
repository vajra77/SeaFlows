//
// Created by Francesco Ferreri (Namex) on 13/12/24.
//

#include <stdlib.h>
#include <pthread.h>
#include "matrix.h"
#include "sflow/net.h"


unsigned int mac_hash(const char mac[6]) {
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

void matrix_add_flow(matrix_t *matrix, const storable_flow_t *flow) {
	pthread_mutex_lock((&(matrix->lock)));

    const unsigned int src_key = mac_hash(flow->src_mac);
    const unsigned int dst_key = mac_hash(flow->dst_mac);

    srcnode_t	*src_ptr = matrix->sources;
	srcnode_t	*lst_src = matrix->sources;

    while(src_ptr != NULL) {
    	if(src_key == src_ptr->key){
    		switch (flow->proto) {
    			case ETHERTYPE_IPV4:
    				src_ptr->bytes_v4 += flow->computed_size;
    				break;
    			case ETHERTYPE_IPV6:
    				src_ptr->bytes_v6 += flow->computed_size;
    				break;
    			default:
    				break;
    		}
    		break; /* exit loop */
    	}
    	lst_src = src_ptr;
        src_ptr = src_ptr->next;
    }

	if (src_ptr == NULL) {
		src_ptr = malloc(sizeof(srcnode_t));
		src_ptr->key = mac_hash(flow->src_mac);
		switch (flow->proto) {
			case ETHERTYPE_IPV4:
				src_ptr->bytes_v4 += flow->computed_size;
				break;
			case ETHERTYPE_IPV6:
				src_ptr->bytes_v6 += flow->computed_size;
				break;
			default:
				break;
		}
		lst_src->next = src_ptr;
		matrix->size += 1;
		src_ptr->destinations = NULL;
		src_ptr->next = NULL;
	}

    dstnode_t *dst_ptr = src_ptr->destinations;
	dstnode_t *lst_dst = src_ptr->destinations;

    while(dst_ptr != NULL) {
    	if(dst_key == dst_ptr->key){
			switch (flow->proto) {
				case ETHERTYPE_IPV4:
					dst_ptr->bytes_v4 += flow->computed_size;
					break;
				case ETHERTYPE_IPV6:
					dst_ptr->bytes_v6 += flow->computed_size;
					break;
				default:
					break;
			}
    	}
    	lst_dst = dst_ptr;
	    dst_ptr = dst_ptr->next;
    }

	if(dst_ptr == NULL) {
		dst_ptr = malloc(sizeof(dstnode_t));
		dst_ptr->key = mac_hash(flow->dst_mac);
		switch (flow->proto) {
			case ETHERTYPE_IPV4:
				dst_ptr->bytes_v4 += flow->computed_size;
			break;
			case ETHERTYPE_IPV6:
				dst_ptr->bytes_v6 += flow->computed_size;
			break;
			default:
				break;
		}
		lst_dst->next = dst_ptr;
		src_ptr->next = NULL;
    }

    pthread_mutex_unlock((&(matrix->lock)));
}

void matrix_dump(matrix_t *matrix) {
	pthread_mutex_lock(&(matrix->lock));
	srcnode_t *src_ptr;
	for(src_ptr = matrix->sources; src_ptr != NULL; src_ptr = src_ptr->next) {

	}
	pthread_mutex_unlock(&(matrix->lock));
}
