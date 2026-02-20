//
// Created by Francesco Ferreri on 19/08/25.
//

#include <string.h>
#include <pthread.h>
#include <stdlib.h>
#include <syslog.h>

#include "bucket.h"


void bucket_init(bucket_t *bucket, const int id) {

 	memset(bucket, 0, sizeof(bucket_t));
    bucket->size = 0;
    bucket->id = id;
    pthread_mutex_init(&bucket->mutex, NULL);
}

void bucket_flush(bucket_t *bucket, bucket_dump_t *dump) {

    pthread_mutex_lock(&bucket->mutex);
    memset(dump, 0, sizeof(bucket_dump_t));

    for (int k = 0; k < bucket->size; k++) {
      memcpy(&dump->nodes[k], &bucket->nodes[k], sizeof(bucket_node_t));
      memset(&bucket->nodes[k], 0, sizeof(bucket_node_t));
    }

    dump->size = bucket->size;
    bucket->size = 0;

    pthread_mutex_unlock(&bucket->mutex);
}

void bucket_add(bucket_t *bucket, const char *src_mac, const char *dst_mac,
                const uint32_t proto, const uint32_t nbytes) {

    pthread_mutex_lock(&bucket->mutex);

    int found = 0;

    for (int k = 0; !found && (k < bucket->size); k++) {
        if (!strncmp(bucket->nodes[k].src, src_mac, MAC_ADDRESS_LEN) &&
            !strncmp(bucket->nodes[k].dst, dst_mac, MAC_ADDRESS_LEN)) {
            found = 1;
            if (proto == 4) {
                bucket->nodes[k].bytes4 += nbytes;
            }
            else {
                bucket->nodes[k].bytes6 += nbytes;
            }
        }
    }

    if (!found) {
        if (bucket->size < MAX_BUCKET) {
            strncpy(bucket->nodes[bucket->size].src, src_mac, MAC_ADDRESS_LEN);
            strncpy(bucket->nodes[bucket->size].dst, dst_mac, MAC_ADDRESS_LEN);
            if (proto == 4) {
                bucket->nodes[bucket->size].bytes4 = nbytes;
                bucket->nodes[bucket->size].bytes6 = 0;
            }
            else {
                bucket->nodes[bucket->size].bytes4 = 0;
                bucket->nodes[bucket->size].bytes6 = nbytes;
            }
            bucket->size++;
        }
        else {
            syslog(LOG_WARNING, "bucket[%d]: full, discarding flow", bucket->id);
        }
    }
    pthread_mutex_unlock(&bucket->mutex);
}

float bucket_occupation(bucket_t *bucket) {

    pthread_mutex_lock(&bucket->mutex);
    const float result = (float)bucket->size / MAX_BUCKET;
    pthread_mutex_unlock(&bucket->mutex);
    return result;
}