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

#include "rrdtool.h"
#include "sflow/sflow.h"


int create_rrd(const char *filename) {

	const char *argv[] = {
		"DS:in:GAUGE:600:U:U",
		"DS:out:GAUGE:600:U:U",
		"RRA:AVERAGE:0.5:1:600",
		"RRA:AVERAGE:0.5:6:700",
		"RRA:AVERAGE:0.5:24:775",
		"RRA:AVERAGE:0.5:288:797",
		"RRA:MAX:0.5:1:600",
		"RRA:MAX:0.5:6:700",
		"RRA:MAX:0.5:24:775",
		"RRA:MAX:0.5:444:797",
	};

	int err = rrdc_connect(RRDCACHE_ADDRESS);
	if (err) {
		syslog(LOG_ERR, "Unable to connect to rrdcached: %s (error=%d)", rrd_get_error(), err);
		rrd_clear_error();
		return -1;
	}

	err = rrdc_create(filename, 300, time(NULL), 1, 10, argv);
	if (err) 	{
		syslog(LOG_WARNING, "Unable to create RRD file: %s (error=%d)", rrd_get_error(), err);
		rrd_clear_error();
	}

	rrdc_disconnect();
	return err;
}

int update_rrd(const char *filename, const uint32_t in, const uint32_t out) {

	char frmtstr[256];

	snprintf(frmtstr, 256, "N:%u:%u", in, out);

	const char *argv[] = {
		frmtstr,
	};

	int err = rrdc_connect(RRDCACHE_ADDRESS);

	if (err) {
		syslog(LOG_ERR, "Unable to connect to rrdcached: %s (error=%d)", rrd_get_error(), err);
		rrd_clear_error();
		return -1;
	}

	err = rrdc_update(filename, 1, argv);

	if (err) {
		syslog(LOG_WARNING, "Unable to update RRD file %s: %s (error=%d)", filename, rrd_get_error(), err);
		rrd_clear_error();
	}

	rrdc_disconnect();
	return err;
}

int update_flow_rrd(const char *src,
                    const char *dst,
                    const uint32_t proto,
                    const uint32_t in,
                    const uint32_t out) {

	char basename[32];
	char pathname[256];
	char filename[256];
	int err = 0;

	/* direct flow file */
	sprintf(basename, "/data/rrd/flows/%s", src);
	sprintf(pathname, "%s/flow_%s_to_%s_v%d.rrd", basename, src, dst, proto);
	sprintf(filename, "flows/%s/flow_%s_to_%s_v%d.rrd", src, src, dst, proto);

	if (access(basename, F_OK) != 0) {
		if (mkdir(basename, 0755)) {
			syslog(LOG_ERR, "Unable to create directory: %s", basename);
			return err;
		}

		err = create_rrd(filename);
		return err;
	}

	if (access(pathname, F_OK) != 0) {
		err = create_rrd(filename);
		return err;
	}

	err = update_rrd(filename, in, out);

	return err;
}

int update_peer_rrd(const char *peer,
					const uint32_t proto,
					const uint32_t in,
					const uint32_t out) {

	char pathname[256];
	char filename[256];
	int err = 0;

	/* peer file */
	sprintf(filename, "peers/peer_%s_v%d.rrd", peer, proto);
	sprintf(pathname, "/data/rrd/peers/peer_%s_v%d.rrd", peer, proto);

	if (access(pathname, F_OK) != 0) {
		err = create_rrd(filename);
		return err;
	}

	err = update_rrd(filename, in, out);

	return err;
}

int cache_flow(const storable_flow_t *flow) {

  	update_flow_rrd(flow->src_mac, flow->dst_mac, flow->proto, flow->computed_size, 0);
    update_flow_rrd(flow->dst_mac, flow->src_mac, flow->proto, 0, flow->computed_size);
  	update_peer_rrd(flow->src_mac, flow->proto, flow->computed_size, 0);
  	update_peer_rrd(flow->dst_mac, flow->proto, 0, flow->computed_size);

  	return 0;
}

// EOF