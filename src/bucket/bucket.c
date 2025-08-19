//
// Created by Francesco Ferreri on 19/08/25.
//

#include <string.h>
#include <pthread.h>
#include <syslog.h>

#include "memory.h"
#include "bucket.h"
#include "rrdtool/rrdtool.h"


void bucket_init(bucket_t *bucket) {

    bucket->size = 0;
    bucket->last = -1;
    for(int k = 0; k < MAX_BUCKET; k++) {
        bucket->nodes[k] = NULL;
    }
    pthread_mutex_init(&bucket->mutex, NULL);
}

void bucket_dump(bucket_t *bucket) {

    pthread_mutex_lock(&bucket->mutex);
    while(bucket->size > 0) {
        bucket_node_t *node = bucket_remove(bucket);
        //rrdtool_store(node->src, node->dst, node->proto, node->in, node->out);
#ifdef DEBUG
        syslog(LOG_DEBUG, "Dump: %s -> %s = (%u, %u) [IPv%d]",
               node->src, node->dst, node->in, node->out, node->proto);
#endif
        MEM_free(node);
    }
    pthread_mutex_unlock(&bucket->mutex);
}

void bucket_add(bucket_t *bucket, const char *src_mac, const char *dst_mac,
                const uint32_t proto, const uint32_t nbytes) {

    pthread_mutex_lock(&bucket->mutex);

    // direct path
    int found = 0;

    for (int k = 0; !found && (k < bucket->size); k++) {
        bucket_node_t *node = bucket->nodes[k];
        if (!strncmp(node->src, src_mac, MAC_ADDRESS_LEN) &&
            !strncmp(node->dst, dst_mac, MAC_ADDRESS_LEN) &&
            (node->proto == proto)) {
            found = 1;
            node->in += nbytes;
        }
    }

    if (!found && (bucket->size < MAX_BUCKET)) {
        bucket_node_t *node = MEM_alloc(sizeof(bucket_node_t));
        strncpy(node->src, src_mac, MAC_ADDRESS_LEN);
        strncpy(node->dst, dst_mac, MAC_ADDRESS_LEN);
        node->proto = proto;
        node->in = nbytes;
        node->out = 0;
        bucket->last = bucket->size;
        bucket->nodes[bucket->last] = node;
        bucket->size++;
#ifdef DEBUG
        syslog(LOG_DEBUG, "Bucket: added %s => %s (%u, %u) [IPv%d] (size=%d)",
               node->dst, node->src, node->in, node->out, node->proto, bucket->size);
#endif
    }

    // reverse path
    found = 0;

    for (int k = 0; !found && (k < bucket->size); k++) {
        bucket_node_t *node = bucket->nodes[k];
        if (!strncmp(node->dst, src_mac, MAC_ADDRESS_LEN) &&
            !strncmp(node->src, dst_mac, MAC_ADDRESS_LEN) &&
            (node->proto == proto)) {
            found = 1;
            node->out += nbytes;
        }
    }

    if (!found && (bucket->size < MAX_BUCKET)) {
        bucket_node_t *node = MEM_alloc(sizeof(bucket_node_t));
        strncpy(node->src, dst_mac, MAC_ADDRESS_LEN);
        strncpy(node->dst, src_mac, MAC_ADDRESS_LEN);
        node->proto = proto;
        node->in = 0;
        node->out = nbytes;
        bucket->last = bucket->size;
        bucket->nodes[bucket->last] = node;
        bucket->size++;
#ifdef DEBUG
        syslog(LOG_DEBUG, "Bucket: added %s => %s (%u, %u) [IPv%d] (size=%d)",
               node->dst, node->src, node->in, node->out, node->proto, bucket->size);
#endif
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