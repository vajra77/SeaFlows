//
// Created by Francesco Ferreri (Namex) on 29/11/24.
//

#ifndef COLLECTOR_H
#define COLLECTOR_H


typedef struct collector_data {
  int port;
  char *address;
} collector_data_t;

void* collector_thread(void *);

#endif //COLLECTOR_H
