//
// Created by Francesco Ferreri (Namex) on 20/08/25.
//
#include <pthread.h>
#include <syslog.h>
#include <unistd.h>
#include <stdlib.h>

#include "broker.h"

#include "rrdtool/rrdtool.h"
#include "bucket/bucket.h"


void* broker_thread(void *arg) {

	pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
	pthread_setcanceltype(PTHREAD_CANCEL_DEFERRED, NULL);

	broker_data_t *broker = arg;

	syslog(LOG_INFO, "Starting broker[%d]", broker->id);

	for (;;) {
		sleep(60);

		syslog(LOG_INFO, "Bucket[%d]: size=%d, occupation=%.2f", broker->id,
			broker->bucket->size, bucket_occupation(broker->bucket));

		bucket_dump_t *dump = bucket_flush(broker->bucket);
		for (int k = 0; k < dump->size; k++) {
			bucket_node_t *node = dump->nodes[k];
			rrdtool_store(node->src, node->dst, node->bytes4, node->bytes6);
			free(node);
		}
		free(dump);
		pthread_testcancel();
	}
	return NULL;
}
