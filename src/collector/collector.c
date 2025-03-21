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
    const int sock = socket(AF_INET, SOCK_DGRAM, 0);

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
		char raw_data[MAX_SFLOW_DATA];
        bzero(raw_data, MAX_SFLOW_DATA);

		const ssize_t len = recvfrom(sock, raw_data, MAX_SFLOW_DATA, 0, NULL, NULL);

		syslog(LOG_DEBUG, "Received UDP datagram");
		const sflow_datagram_t *datagram = sflow_decode_datagram(raw_data, len);

		if (datagram) {
		// for (const flow_sample_t* sample = datagram->samples; sample != NULL; sample = sample->next) {
		// 	for (const flow_record_t* record = sample->records; record != NULL; record = record->next) {
		// 		storable_flow_t	*flow = sflow_encode_flow_record(record, sample->header.sampling_rate);
		// 		queue_push(collector_data->queue, flow);
		// 	}
		// }
		// sflow_free_datagram(datagram);
			syslog(LOG_DEBUG, "Datagram decoded");
		}

		// pthread_testcancel();
	}
}
