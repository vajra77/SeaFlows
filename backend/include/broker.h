//
// Created by Francesco Ferreri (Namex) on 20/08/25.
//

#ifndef BROKER_H
#define BROKER_H

#include <bucket/bucket.h>

typedef struct broker_data {
	int id;
	bucket_t *bucket;
} broker_data_t;

void* broker_thread(void *);
#endif //BROKER_H
