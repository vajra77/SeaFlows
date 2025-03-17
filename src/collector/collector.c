//
// Created by Francesco Ferreri (Namex) on 29/11/24.
//

#include <sys/socket.h>
#include <pthread.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <strings.h>

#include "sflow/sflow.h"
#include "collector/error.h"
#include "collector/collector.h"


void* handle_request(void *arg) {
  sflow_raw_data_t *raw_data = (sflow_raw_data_t*)arg;
  sflow_datagram_t *sflow_datagram = sflow_decode_datagram(raw_data);

  for (flow_sample_t* sample = sflow_datagram->samples; sample != NULL; sample = sample->next) {
    for (flow_record_t* record = sample->records; record != NULL; record = record->next) {
  		storable_flow_t	*storable_flow = sflow_encode_flow_record(record);

    }
  }
  /* do something with data:

	1 - create serializable structure
	2 - add structure to queue
	3 - free resources

  */

  sflow_free_datagram(sflow_datagram);
  free(raw_data);
  return NULL;
}

void* collector_thread(void *arg)
{
    collector_address_t *collector_address = (collector_address_t *)arg;
    int sock = socket(AF_INET, SOCK_DGRAM, 0);

    if (sock < 0)
		return NULL;

    struct sockaddr_in address;
	bzero(&address, sizeof(address));

    address.sin_family = AF_INET;
    inet_pton(AF_INET, collector_address->address, &address.sin_addr);
	address.sin_port = htons(collector_address->port);

	if (bind(sock, (struct sockaddr *)&address, sizeof(address)) < 0 )
          return NULL;

	while(1) {
        sflow_raw_data_t *raw_data = (sflow_raw_data_t*)malloc(sizeof(sflow_raw_data_t));

        raw_data->size = recvfrom(sock, raw_data->data, MAX_SFLOW_DATA, 0, NULL, NULL) ;

        pthread_t new_thread;
		pthread_create(&new_thread, NULL, handle_request, (void *)raw_data);
	}
}
