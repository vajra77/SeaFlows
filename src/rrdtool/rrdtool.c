//
// Created by Francesco Ferreri (Namex) on 19/03/25.
//

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/syslog.h>
#include <rrd.h>
#include <rrd_client.h>

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

	int err = rrdc_connect("127.0.0.1:42217");
	if (err) {
		syslog(LOG_ERR, "Unable to connect to rrdcached: %s (error=%d)", rrd_get_error(), err);
		rrd_clear_error();
		return -1;
	}

	err = rrdc_create(filename, 300, time(NULL), 1, 10, argv);
	if (err) 	{
		syslog(LOG_ERR, "Unable to create RRD file: %s (error=%d)", rrd_get_error(), err);
		rrd_clear_error();
	}
	rrdc_disconnect();
	return err;
}

int update_flow_rrd(char *filename, const dstnode_t *dst) {

	char str_bytes_v4[256];
	char str_bytes_v6[256];

	snprintf(str_bytes_v4, 256, "%u", dst->bytes_v4);
	snprintf(str_bytes_v6, 256, "%u", dst->bytes_v6);

	const char *argv[] = {
		"N",
		str_bytes_v4,
		str_bytes_v6,
	};

	int err = rrdc_connect("127.0.0.1:42217");

	if (err) {
		syslog(LOG_ERR, "Unable to connect to rrdcached: %s (error=%d)", rrd_get_error(), err);
		rrd_clear_error();
		return -1;
	}

	err = rrdc_update(filename, 3, argv);

	syslog(LOG_ERR, "Updated %s with values: %u, %u", filename, dst->bytes_v4, dst->bytes_v6);

	if (err) {
		syslog(LOG_ERR, "Unable to update RRD file: %s (error=%d)", rrd_get_error(), err);
		rrd_clear_error();
	}

	rrdc_disconnect();
	return err;
}

int update_peer_rrd(char *filename, const srcnode_t *src) {

	char str_bytes_v4[256];
	char str_bytes_v6[256];

	snprintf(str_bytes_v4, 256, "%u", src->bytes_v4);
	snprintf(str_bytes_v6, 256, "%u", src->bytes_v6);

	const char *argv[] = {
		"N",
		str_bytes_v4,
		str_bytes_v6,
	};

	int err = rrdc_connect("127.0.0.1:42217");
	if (err) {
		syslog(LOG_ERR, "Unable to connect to rrdcached: %s (error=%d)", rrd_get_error(), err);
		rrd_clear_error();
		return -1;
	}

	err = rrdc_update(filename, 3, argv);
	syslog(LOG_ERR, "Updated %s with values: %u, %u", filename, src->bytes_v4, src->bytes_v6);

	if (err) {
		syslog(LOG_ERR, "Unable to update RRD file: %s (error=%d)", rrd_get_error(), err);
		rrd_clear_error();
	}

	rrdc_disconnect();
	return err;
}

int rrd_store_flow(const srcnode_t *src, const dstnode_t *dst) {

	char basename[32];
	char pathname[256];
	char filename[256];
	int err = 0;

	/* flow file */
	sprintf(basename, "/data/rrd/flows/%s", src->mac);
	sprintf(pathname, "%s/flow_%s_to_%s.rrd", basename, src->mac, dst->mac);
	sprintf(filename, "flows/%s/flow_%s_to_%s.rrd", src->mac, src->mac, dst->mac);

	if (access(basename, F_OK) != 0) {
		if (mkdir(basename, 0755)) {
			syslog(LOG_ERR, "Unable to create directory: %s", basename);
			return err;
		}
		err = create_rrd(filename);
	}
	else {
		if (access(pathname, F_OK) != 0)
			err = create_rrd(filename);
	}

	if (err == 0)
		err = update_flow_rrd(filename, dst);

	return err;
}

int rrd_store_peer(const srcnode_t *src) {

	char basename[32];
	char pathname[256];
	char filename[256];
	int err = 0;

	/* flow file */
	sprintf(basename, "/data/rrd/peers/%s", src->mac);
	sprintf(pathname, "%s/peer_%s.rrd", basename, src->mac);
	sprintf(filename, "peers/%s/peer_%s.rrd", src->mac, src->mac);

	if (access(basename, F_OK) != 0) {
		if (mkdir(basename, 0755)) {
			syslog(LOG_ERR, "Unable to create directory: %s", basename);
			return err;
		}
		err = create_rrd(filename);
	}
	else {
		if (access(pathname, F_OK) != 0)
			err = create_rrd(filename);
	}

	if (err == 0)
		err = update_peer_rrd(filename, src);

	return err;
}

