//
// Created by Francesco Ferreri (Namex) on 29/11/24.
//
#include <sys/socket.h>
#include <pthread.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <strings.h>

#include "collector/error.h"
#include "sflow/export.h"
#include "collector/server.h"


void* handle_request(void *arg)
{
  sflow_raw_data_t *raw_data = (sflow_raw_data_t*)arg;
  free(raw_data);
  return NULL;
}

void* server_thread(void *arg)
{
    server_address_t *server_address = (server_address_t *)arg;
    int sock = socket(AF_INET, SOCK_DGRAM, 0);

    if (sock < 0)
		return NULL;

    struct sockaddr_in server;
	bzero(&server, sizeof(server));

    server.sin_family = AF_INET;
    inet_pton(AF_INET, server_address->address, &server.sin_addr);
	server.sin_port = htons(server_address->port);

	if (bind(sock, (struct sockaddr *)&server, sizeof(server)) < 0 )
          return NULL;

	while(1)
    {
        sflow_raw_data_t *raw_data = (sflow_raw_data_t*)malloc(sizeof(sflow_raw_data_t));

        raw_data->size = recvfrom(sock, raw_data->data, MAX_SFLOW_DATA, 0, NULL, NULL) ;

        pthread_t new_thread;
		pthread_create(&new_thread, NULL, handle_request, (void *)raw_data);
	}
}


