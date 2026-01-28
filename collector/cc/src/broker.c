//
// Created by Francesco Ferreri (Namex) on 20/08/25.
//
#include <pthread.h>
#include <syslog.h>
#include <unistd.h>
#include <stdlib.h>

#include "config.h"
#include "broker.h"
#include "rrdtool.h"
#include "bucket.h"


void* broker_thread(void *arg) {

	broker_data_t *broker = arg;

	syslog(LOG_INFO, "starting broker[%d], saving RRD files to %s", broker->id, RRD_DIR);

	bucket_dump_t dump;
	int seconds_counter = 0;
	int log_counter = 0;

	while (keep_running) {
		sleep(1);
		seconds_counter++;

		if (seconds_counter >= 60) {
			if (++log_counter >= 120) {
				syslog(LOG_INFO, "bucket[%d]: occupation=%.2f%%",
					   broker->bucket->id,
					   100 * bucket_occupation(broker->bucket));
				log_counter = 0;
			}

			bucket_flush(broker->bucket, &dump);

			for (int k = 0; k < dump.size; k++) {
				bucket_node_t node = dump.nodes[k];
				rrdtool_store(node.src, node.dst, node.bytes4, node.bytes6);
			}

			seconds_counter = 0; // Reset contatore
		}
	}

	syslog(LOG_INFO, "broker[%d] exiting safely", broker->id);
	return NULL;
}
