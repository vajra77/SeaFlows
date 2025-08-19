//
// Created by Francesco Ferreri on 19/08/25.
//

#include <string.h>
#include <unistd.h>

#include "memory.h"
#include "bucket.h"


void bucket_init(bucket_t *bucket) {

    bucket->size = 0;
    bucket->last = -1;
    for(int k = 0; k < MAX_BUCKET; k++) {
        bucket->nodes[k] = NULL;
    }
    pthread_mutex_init(&bucket->mutex, NULL);
}

int bucket_add(bucket_t *bucket, const char *src_mac, const char *dst_mac, const uint32_t nbytes) {

    pthread_mutex_lock(&bucket->mutex);

    int found = 0;

    // direct path

    for (int k = 0; k < bucket->size && !found; k++) {
        bucket_node_t *node = bucket->nodes[k];
        if (!strcmp(node->src, src_mac) && !strcmp(node->dst, dst_mac)) {
            node->in += nbytes;
            found = 1;
        }
    }

    if (!found) {
        bucket_node_t *node = MEM_alloc(sizeof(bucket_node_t));
        if (node == NULL) {
            return -1;
        }
        strcpy(node->src, src_mac);
        strcpy(node->dst, dst_mac);
        node->in = nbytes;
        node->out = 0;
        bucket->last = bucket->size;
        bucket->nodes[bucket->last] = node;
        bucket->size++;
    }

    // reverse path
    found = 0;

    for (int k = 0; k < bucket->size && !found; k++) {
        bucket_node_t *node = bucket->nodes[k];
        if (!strcmp(node->dst, src_mac) && !strcmp(node->src, dst_mac)) {
            node->out += nbytes;
            found = 1;
        }
    }

    if (!found) {
        bucket_node_t *node = MEM_alloc(sizeof(bucket_node_t));
        if (node == NULL) {
            return -1;
        }
        strcpy(node->src, dst_mac);
        strcpy(node->dst, src_mac);
        node->out = nbytes;
        node->in = 0;
        bucket->last = bucket->size;
        bucket->nodes[bucket->last] = node;
        bucket->size++;
    }

    pthread_mutex_unlock(&bucket->mutex);
    return 0;
}

bucket_node_t *bucket_remove(bucket_t *bucket) {

    pthread_mutex_lock(&bucket->mutex);
    if (bucket->size == 0) {
        return NULL;
    }
    bucket_node_t *node = bucket->nodes[bucket->last];
    bucket->nodes[bucket->last] = NULL;
    bucket->last--;
    bucket->size--;

    pthread_mutex_unlock(&bucket->mutex);
    return node;
}