//
// Created by Francesco Ferreri (Namex) on 29/11/24.
//

#ifndef SERVER_H
#define SERVER_H

typedef struct server_address {
  int port;
  char *address;
} server_address_t;

void* server_thread(void *);

#endif //SERVER_H
