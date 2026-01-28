//
// Created by Francesco Ferreri (Namex) on 29/11/24.
//

#include <sys/socket.h>
#include <sys/time.h>
#include <pthread.h>
#include <arpa/inet.h>
#include <string.h>
#include <syslog.h>
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>
#include <stdbool.h>

#include "rrdtool.h"
#include "sflow.h"
#include "listener.h"
#include "bucket.h"

extern volatile bool keep_running;

void* listener_thread(void *arg) {

	listener_data_t *listener = arg;
	const int sock = socket(AF_INET, SOCK_DGRAM, 0);

	if (sock < 0) return NULL;

	struct timeval tv;
	tv.tv_sec = 1;
    tv.tv_usec = 0;
    setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));

	struct sockaddr_in address;
	bzero(&address, sizeof(address));
	address.sin_family = AF_INET;
	inet_pton(AF_INET, listener->address, &address.sin_addr);
	address.sin_port = htons(listener->port);

	syslog(LOG_INFO, "starting listener[%d] on %s:%d", listener->id, listener->address, listener->port );

	if (bind(sock, (struct sockaddr *)&address, sizeof(address)) < 0 ) {
		close(sock);
		return NULL;
	}

	char raw_data[MAX_SFLOW_DATA];

	storable_flow_t flow;
	sflow_datagram_t datagram;

    while (keep_running) {
		memset(raw_data, 0, MAX_SFLOW_DATA);

		const ssize_t raw_data_len = recvfrom(sock, raw_data, MAX_SFLOW_DATA, 0, NULL, NULL);

		if (raw_data_len < 0) {
            if (errno == EAGAIN || errno == EWOULDBLOCK || errno == EINTR) {
                continue;
            }
            break;
        }

		if(!sflow_decode_datagram(raw_data, raw_data_len, &datagram)) {
			for (int s = 0; s < datagram.header.num_samples; s++) {
                flow_sample_t *sample = &datagram.samples[s];
				for (int r = 0; r < sample->header.num_records; r++) {
					sflow_encode_flow_record(&sample->records[r], sample->header.sampling_rate, &flow);
					rrdtool_prepare(flow.src_mac, flow.dst_mac);
					bucket_add(listener->bucket, flow.src_mac, flow.dst_mac, flow.proto, flow.computed_size);
				}
			}
		}
	}

	syslog(LOG_INFO, "listener[%d] closing socket and exiting", listener->id);
    close(sock);
	return NULL;
}
