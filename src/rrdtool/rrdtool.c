//
// Created by Francesco Ferreri (Namex) on 19/03/25.
//

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <sys/syslog.h>
#include <rrd.h>
#include <rrd_client.h>

#include "sflow/sflow.h"
#include "matrix/matrix.h"
#include "rrdtool.h"


int create_rrd(rrd_client_t *client, char *filename) {

	const char *argv[] = {
		"DS:ipv4_bytes:ABSOLUTE:600:U:U",
		"DS:ipv6_bytes:ABSOLUTE:600:U:U",
		"RRA:AVERAGE:0.5:1:600",
		"RRA:AVERAGE:0.5:6:700",
		"RRA:AVERAGE:0.5:24:775",
		"RRA:AVERAGE:0.5:288:797",
		"RRA:MAX:0.5:1:600",
		"RRA:MAX:0.5:6:700",
		"RRA:MAX:0.5:24:775",
		"RRA:MAX:0.5:444:797",
	};

	return rrd_client_create(client, filename, 300, time(NULL), 1, 10, argv);
}

int update_rrd(rrd_client_t *client, char *filename, const dstnode_t *dst) {

	char str_bytes_v4[256];
	char str_bytes_v6[256];

	snprintf(str_bytes_v4, 256, "%u", dst->bytes_v4);
	snprintf(str_bytes_v6, 256, "%u", dst->bytes_v6);

	const char *argv[] = {
		str_bytes_v4,
		str_bytes_v6,
	};
	return rrd_client_update(client, filename, 2, argv);
}

int rrd_store_flow(rrd_client_t *client, const srcnode_t *src, const dstnode_t *dst) {

	char filename[256];

	/* flow file */
	sprintf(filename, "/data/rrd/flows/flow_%s_to_%s.rrd", src->mac, dst->mac);
	if (!access(filename, F_OK)) {
		const int result = create_rrd(client, filename);
		syslog(LOG_DEBUG, "Created new RRD file %s, got: %d", filename, result);
	}
	const int result = update_rrd(client, filename, dst);
	syslog(LOG_DEBUG, "Done updating flow dump to %s, got: %d", filename, result);
	return result;
}
