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


int create_rrd(char *filename) {

	const char *argv[] = {
		"DS:bytes_v4:ABSOLUTE:600:U:U",
		"DS:bytes_v6:ABSOLUTE:600:U:U",
		"RRA:AVERAGE:0.5:1:600",
		"RRA:AVERAGE:0.5:6:700",
		"RRA:AVERAGE:0.5:24:775",
		"RRA:AVERAGE:0.5:288:797",
		"RRA:MAX:0.5:1:600",
		"RRA:MAX:0.5:6:700",
		"RRA:MAX:0.5:24:775",
		"RRA:MAX:0.5:444:797",
	};

	int ret = rrdc_connect("unix:/var/run/rrdcached.sock");
	if (!ret) {
		syslog(LOG_ERR, "Unable to connect to rrdcached");
		return ret;
	}

	ret = rrdc_create(filename, 300, time(NULL), 1, 10, argv);
	if (!ret )
	{
		syslog(LOG_ERR, "Unable to create RRD file");
	}
	rrdc_disconnect();
	return ret;
}

int update_rrd(char *filename, const dstnode_t *dst) {

	char str_bytes_v4[256];
	char str_bytes_v6[256];

	snprintf(str_bytes_v4, 256, "%u", dst->bytes_v4);
	snprintf(str_bytes_v6, 256, "%u", dst->bytes_v6);

	const char *argv[] = {
		str_bytes_v4,
		str_bytes_v6,
	};
	rrdc_connect(NULL);
	const int result = rrdc_update(filename, 2, argv);
	rrdc_disconnect();
	return result;
}

int rrd_store_flow(const srcnode_t *src, const dstnode_t *dst) {

	char filename[256];

	/* flow file */
	sprintf(filename, "/data/rrd/flows/flow_%s_to_%s.rrd", src->mac, dst->mac);
	if (!access(filename, F_OK)) {
		const int result = create_rrd(filename);
		syslog(LOG_DEBUG, "Created new RRD file %s, got: %d", filename, result);
	}
	const int result = update_rrd(filename, dst);
	syslog(LOG_DEBUG, "Done updating flow dump to %s, got: %d", filename, result);
	return result;
}
