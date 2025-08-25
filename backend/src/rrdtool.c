//
// Created by Francesco Ferreri (Namex) on 18/08/25.
//

#include <rrd.h>
#include <rrd_client.h>
#include <stdio.h>
#include <stdlib.h>
#include <syslog.h>
#include <unistd.h>
#include <time.h>
#include <sys/stat.h>

#include "config.h"
#include "rrdtool.h"


int create_rrd(const char *filename) {

	const char *argv[] = {
		"DS:bytes4:GAUGE:600:U:U",
		"DS:bytes6:GAUGE:600:U:U",
		"RRA:AVERAGE:0.5:1:600",
		"RRA:AVERAGE:0.5:6:700",
		"RRA:AVERAGE:0.5:24:775",
		"RRA:AVERAGE:0.5:288:797",
		"RRA:MAX:0.5:1:600",
		"RRA:MAX:0.5:6:700",
		"RRA:MAX:0.5:24:775",
		"RRA:MAX:0.5:444:797",
	};

	int err = rrdc_connect(RRDCACHED_ADDRESS);
	if (err) {
		syslog(LOG_ERR, "unable to connect to rrdcached: %s (error=%d)", rrd_get_error(), err);
		rrd_clear_error();
		return -1;
	}

	err = rrdc_create(filename, 300, time(NULL), 1, 10, argv);
	if (err) 	{
#ifdef DEBUG
		syslog(LOG_DEBUG, "unable to create RRD file: %s (error=%d)", filename, err);
#endif
		rrd_clear_error();
	}

	rrdc_disconnect();
	return err;
}

int update_rrd(const char *filename, const uint32_t bytes4, const uint32_t bytes6) {

	char frmtstr[256];

	snprintf(frmtstr, 256, "N:%u:%u", bytes4, bytes6);

	const char *argv[] = {
		frmtstr,
	};

	int err = rrdc_connect(RRDCACHED_ADDRESS);

	if (err) {
#ifdef DEBUG
		syslog(LOG_DEBUG, "unable to connect to rrdcached: %s (error=%d)", rrd_get_error(), err);
#endif
		rrd_clear_error();
		return -1;
	}

	err = rrdc_update(filename, 1, argv);

	if (err) {
#ifdef DEBUG
		syslog(LOG_DEBUG, "unable to update RRD file %s: %s (error=%d)", filename, rrd_get_error(), err);
#endif
		rrd_clear_error();
	}

	rrdc_disconnect();
	return err;
}

int rrdtool_prepare(const char *src, const char *dst) {

	char basename[32];
	char pathname[256];
	char filename[256];

	int err = 0;

	/* direct flow file */
	sprintf(basename, "%s/flows/%s", RRD_DIR, src);
	sprintf(pathname, "%s/flow_%s_to_%s.rrd", basename, src, dst);
	sprintf(filename, "flows/%s/flow_%s_to_%s.rrd", src, src, dst);

	if (access(basename, F_OK) != 0) {
		if (mkdir(basename, 0755)) {
			syslog(LOG_ERR, "unable to create directory: %s", basename);
			return err;
		}
		err = create_rrd(filename);
		if (err) return err;
	}

	if (access(pathname, F_OK) != 0) {
		err = create_rrd(filename);
		if (err) return err;
	}

	return err;
}

int rrdtool_store(const char *src, const char *dst, const uint32_t bytes4, const uint32_t bytes6) {

	char basename[32];
	char pathname[256];
	char filename[256];

	int err = 0;

	/* direct flow file */
	sprintf(basename, "%s/flows/%s", RRD_DIR, src);
	sprintf(pathname, "%s/flow_%s_to_%s.rrd", basename, src, dst);
	sprintf(filename, "flows/%s/flow_%s_to_%s.rrd", src, src, dst);

	if (access(basename, F_OK) != 0) {
		if (mkdir(basename, 0755)) {
			syslog(LOG_ERR, "unable to create directory: %s", basename);
			return err;
		}
		err = create_rrd(filename);
		if (err) return err;
	}

	if (access(pathname, F_OK) != 0) {
		err = create_rrd(filename);
		if (err) return err;
	}

	err = update_rrd(filename, bytes4, bytes6);
	return err;
}
// EOF