//
// Created by Francesco Ferreri (Namex) on 29/11/24.
//

#include <sys/socket.h>
#include <pthread.h>
#include <arpa/inet.h>
#include <strings.h>
#include <syslog.h>
#include <unistd.h>

#include "memory.h"
#include "rrdtool/rrdtool.h"
#include "sflow/sflow.h"
#include "collector/collector.h"
#include "bucket/bucket.h"


void* collector_thread(void *arg) {

	pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
	pthread_setcanceltype(PTHREAD_CANCEL_DEFERRED, NULL);

	collector_data_t *collector = arg;
	const int sock = socket(AF_INET, SOCK_DGRAM, 0);

	if (sock < 0)
		return NULL;

	struct sockaddr_in address;
	bzero(&address, sizeof(address));

	address.sin_family = AF_INET;
	inet_pton(AF_INET, collector->address, &address.sin_addr);
	address.sin_port = htons(collector->port);

	syslog(LOG_INFO, "Starting collector[%d] on %s:%d", collector->id, collector->address, collector->port );
	if (bind(sock, (struct sockaddr *)&address, sizeof(address)) < 0 )
		  return NULL;

	for (;;) {
		char raw_data[MAX_SFLOW_DATA];
		bzero(raw_data, MAX_SFLOW_DATA);

		const ssize_t raw_data_len = recvfrom(sock, raw_data, MAX_SFLOW_DATA, 0, NULL, NULL);

		sflow_datagram_t *datagram = sflow_decode_datagram(raw_data, raw_data_len);

		if (datagram) {
			for (const flow_sample_t* sample = datagram->samples; sample != NULL; sample = sample->next) {
				for (const flow_record_t* record = sample->records; record != NULL; record = record->next) {
					storable_flow_t	*flow = sflow_encode_flow_record(record, sample->header.sampling_rate);
					if (flow != NULL) {
						rrdtool_prepare(flow->src_mac, flow->dst_mac);
						bucket_add(collector->bucket, flow->src_mac, flow->dst_mac, flow->proto, flow->computed_size);
						MEM_free(flow);
					}
				}
			}
			sflow_free_datagram(datagram);
		}
		pthread_testcancel();
	}

	return NULL;
}
