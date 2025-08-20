//
// Created by Francesco Ferreri on 19/08/25.
//

#include <string.h>
#include <pthread.h>
#include <stdlib.h>

#include "bucket.h"


void bucket_init(bucket_t *bucket) {

    bucket->size = 0;
    bucket->last = -1;
    for(int k = 0; k < MAX_BUCKET; k++) {
        bucket->nodes[k] = NULL;
    }
    pthread_mutex_init(&bucket->mutex, NULL);
}

bucket_dump_t *bucket_flush(bucket_t *bucket) {

    pthread_mutex_lock(&bucket->mutex);
    bucket_dump_t *dump = malloc(sizeof(bucket_dump_t));
    memset(dump, 0, sizeof(bucket_dump_t));

    for (int k = 0; k <= bucket->last; k++) {
        dump->nodes[k] = bucket->nodes[k];
        bucket->nodes[k] = NULL;
    }

    dump->size = bucket->last + 1;
    bucket->size = 0;
    bucket->last = -1;

    pthread_mutex_unlock(&bucket->mutex);

    return dump;
}

void bucket_add(bucket_t *bucket, const char *src_mac, const char *dst_mac,
                const uint32_t proto, const uint32_t nbytes) {

    pthread_mutex_lock(&bucket->mutex);

    int found = 0;

    for (int k = 0; !found && (k < bucket->size); k++) {
        bucket_node_t *node = bucket->nodes[k];
        if (!strncmp(node->src, src_mac, MAC_ADDRESS_LEN) &&
            !strncmp(node->dst, dst_mac, MAC_ADDRESS_LEN)) {
            found = 1;
            if (proto == 4) {
                node->bytes4 += nbytes;
            }
            else {
                node->bytes6 += nbytes;
            }
        }
    }

    if (!found && (bucket->size < MAX_BUCKET)) {
        bucket_node_t *node = malloc(sizeof(bucket_node_t));
        strncpy(node->src, src_mac, MAC_ADDRESS_LEN);
        strncpy(node->dst, dst_mac, MAC_ADDRESS_LEN);
        if (proto == 4) {
            node->bytes4 = nbytes;
            node->bytes6 = 0;
        }
        else {
            node->bytes4 = 0;
            node->bytes6 = nbytes;
        }
        bucket->last = bucket->size;
        bucket->nodes[bucket->last] = node;
        bucket->size++;
    }

    pthread_mutex_unlock(&bucket->mutex);
}

bucket_node_t *bucket_remove(bucket_t *bucket) {

    pthread_mutex_lock(&bucket->mutex);
    if (bucket->size == 0) {
        pthread_mutex_unlock(&bucket->mutex);
        return NULL;
    }
    bucket_node_t *node = bucket->nodes[bucket->last];
    bucket->nodes[bucket->last] = NULL;
    bucket->last--;
    bucket->size--;

    pthread_mutex_unlock(&bucket->mutex);
    return node;
}