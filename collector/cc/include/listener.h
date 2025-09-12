//
// Created by Francesco Ferreri (Namex) on 29/11/24.
//

#ifndef LISTENER_H
#define LISTENER_H

#include "bucket.h"


typedef struct listener_data {
  int id;
  int port;
  char *address;
  bucket_t *bucket;
} listener_data_t;

void* listener_thread(void *);

#endif //LISTENER_H