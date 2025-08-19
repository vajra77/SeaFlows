//
// Created by Francesco Ferreri on 19/08/25.
//

#ifndef BUCKET_H
#define BUCKET_H
#define MAX_BUCKET 2048
#define MAC_ADDRESS_LEN 13

#include <stdint.h>
#include <pthread.h>


typedef struct bucket_node {
    char src[MAC_ADDRESS_LEN];
    char dst[MAC_ADDRESS_LEN];
    uint32_t proto;
    uint32_t in;
    uint32_t out;
} bucket_node_t;

typedef struct bucket {
    pthread_mutex_t mutex;
    int last;
    int size;
    void* nodes[MAX_BUCKET];
} bucket_t;

void bucket_init(bucket_t *);
void bucket_add(bucket_t *, const char *, const char *, const uint32_t, const uint32_t);
bucket_node_t *bucket_remove(bucket_t *);
void bucket_dump(bucket_t *);
#endif //BUCKET_H
