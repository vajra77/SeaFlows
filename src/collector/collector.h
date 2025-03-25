//
// Created by Francesco Ferreri (Namex) on 29/11/24.
//

#ifndef COLLECTOR_H
#define COLLECTOR_H

#include <rrd.h>
#include "matrix/matrix.h"

typedef struct collector_data {
  int port;
  char *address;
  rrd_client_t *rrd_client;
  matrix_t *matrix;
} collector_data_t;

void* collector_thread(void *);

#endif //COLLECTOR_H
