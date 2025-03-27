//
// Created by Francesco Ferreri (Namex) on 13/12/24.
//

#include <stdlib.h>
#include <pthread.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/syslog.h>
#include <gc.h>
#include <rrd.h>

#include "matrix.h"
#include "rrdtool/rrdtool.h"
#include "sflow/net.h"


srcnode_t *get_src_node(const matrix_t *matrix, const char *mac) {
	srcnode_t *node = matrix->sources;

	while (node) {
		if (!strcmp(mac, node->mac)) {
			return node;
		}
		node = node->next;
	}

	return NULL;
}

void add_src_node(matrix_t *matrix, srcnode_t *node) {

	if (!matrix->sources) {
		matrix->sources = node;
		matrix->num_sources = 1;
	}
	else {
		srcnode_t *ptr = matrix->sources;
		while (ptr->next) {
			ptr = ptr->next;
		}
		ptr->next = node;
		matrix->num_sources++;
	}
}

dstnode_t *get_dst_node(const srcnode_t *src, const char *mac) {

	dstnode_t *node = src->destinations;

	while (node) {
		if (!strcmp(mac, node->mac)) {
			return node;
		}
		node = node->next;
	}

	return NULL;
}

void add_dst_node(srcnode_t *src, dstnode_t *node) {

	if (!src->destinations) {
		src->destinations = node;
		src->num_destinations = 1;
	}
	else {
		dstnode_t *ptr = src->destinations;
		while (ptr->next) {
			ptr = ptr->next;
		}
		ptr->next = node;
		src->num_destinations++;
	}
}


void matrix_init(matrix_t *matrix) {
	pthread_mutex_init(&(matrix->lock), NULL);
	matrix->sources = NULL;
	matrix->num_sources = 0;
	matrix->dirty = 0;
}

void matrix_destroy(matrix_t *matrix) {
	while(matrix->sources != NULL) {
		srcnode_t *src = matrix->sources;
		while(src->destinations != NULL) {
			dstnode_t *dst = src->destinations;
			src->destinations = src->destinations->next;
			GC_free(dst);
		}
		matrix->sources = matrix->sources->next;
		GC_free(src);
	}
}

void matrix_add_flow(matrix_t *matrix, const storable_flow_t *flow) {

	pthread_mutex_lock(&matrix->lock);
	srcnode_t *src_node = get_src_node(matrix, flow->src_mac);

	if (!src_node) {
		src_node = GC_malloc(sizeof(srcnode_t));
		bzero(src_node, sizeof(srcnode_t));
		strcpy(src_node->mac, flow->src_mac);
		src_node->next = NULL;
		add_src_node(matrix, src_node);
	}

	switch (flow->proto) {
		case ETHERTYPE_IPV4:
			src_node->bytes_v4 += flow->computed_size;
			break;

		case ETHERTYPE_IPV6:
			src_node->bytes_v6 += flow->computed_size;
			break;

		default:
			src_node->bytes_nk += flow->computed_size;
			break;
	}

	dstnode_t *dst_node = get_dst_node(src_node, flow->dst_mac);

	if (!dst_node) {
		dst_node = GC_malloc(sizeof(dstnode_t));
		bzero(dst_node, sizeof(dstnode_t));
		strcpy(dst_node->mac, flow->dst_mac);
		dst_node->next = NULL;
		add_dst_node(src_node, dst_node);
	}

	switch (flow->proto) {
		case ETHERTYPE_IPV4:
			dst_node->bytes_v4 += flow->computed_size;
			break;
		case ETHERTYPE_IPV6:
			dst_node->bytes_v6 += flow->computed_size;
			break;
		default:
			dst_node->bytes_nk += flow->computed_size;
			break;
	}

	matrix->dirty = 1;
	pthread_mutex_unlock(&matrix->lock);
}

void matrix_dump(matrix_t *matrix) {
	pthread_mutex_lock(&matrix->lock);
	for(srcnode_t *src_ptr = matrix->sources; src_ptr != NULL; src_ptr = src_ptr->next) {
		for (dstnode_t *dst_ptr = src_ptr->destinations; dst_ptr != NULL; dst_ptr = dst_ptr->next) {

			rrd_store_flow(src_ptr, dst_ptr);

			/* clear dst data */
			dst_ptr->bytes_v4 = 0;
			dst_ptr->bytes_v6 = 0;
			dst_ptr->bytes_nk = 0;
		}

		rrd_store_peer(src_ptr);
		/* clear src data */
		src_ptr->bytes_v4 = 0;
		src_ptr->bytes_v6 = 0;
		src_ptr->bytes_nk = 0;
	}
	matrix->dirty = 0;
	pthread_mutex_unlock(&(matrix->lock));
}
