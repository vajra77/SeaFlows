//
// Created by Francesco Ferreri (Namex) on 29/11/24.
//

#include <sys/socket.h>
#include <pthread.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <strings.h>
#include <syslog.h>

#include "sflow/sflow.h"
#include "collector/collector.h"

#include <unistd.h>


void* collector_thread(void *arg) {

	pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
	pthread_setcanceltype(PTHREAD_CANCEL_DEFERRED, NULL);

    collector_data_t *collector_data = arg;
    int sock = socket(AF_INET, SOCK_DGRAM, 0);

    if (sock < 0)
		return NULL;

    struct sockaddr_in address;
	bzero(&address, sizeof(address));

    address.sin_family = AF_INET;
    inet_pton(AF_INET, collector_data->address, &address.sin_addr);
	address.sin_port = htons(collector_data->port);

	syslog(LOG_INFO, "Starting collector on %s:%d", collector_data->address, collector_data->port );
	if (bind(sock, (struct sockaddr *)&address, sizeof(address)) < 0 )
          return NULL;

	for (;;) {
        sflow_raw_data_t *raw_data = malloc(sizeof(sflow_raw_data_t));

        raw_data->size = recvfrom(sock, raw_data->data, MAX_SFLOW_DATA, 0, NULL, NULL);

		sflow_datagram_t *sflow_datagram = sflow_decode_datagram(raw_data);

		for (flow_sample_t* sample = sflow_datagram->samples; sample != NULL; sample = sample->next) {
			for (flow_record_t* record = sample->records; record != NULL; record = record->next) {
				storable_flow_t	*storable_flow = sflow_encode_flow_record(record, sample->header.sampling_rate);
				queue_push(collector_data->queue, storable_flow);
			}
		}

        sflow_free_datagram(sflow_datagram);
		free(raw_data);
		pthread_testcancel();
	}
}
