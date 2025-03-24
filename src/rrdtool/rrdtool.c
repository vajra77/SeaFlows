//
// Created by Francesco Ferreri (Namex) on 19/03/25.
//

#include <stdio.h>
#include <stdlib.h>
#include <rrd.h>
#include "rrdtool.h"

int create_rrd(char *filename) {
	const size_t argc = 15;
	char *argv[] = {
		filename,
		"--step", "300",
		"--start", "now",
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
	return rrd_create(argc, argv);
}

int update_rrd(char *filename, const unsigned int bytes_v4, const unsigned int bytes_v6) {

	char str_bytes_v4[256];
	char str_bytes_v6[256];

	snprintf(str_bytes_v4, 256, "%lu", bytes_v4);
	snprintf(str_bytes_v6, 256, "%lu", bytes_v6);

	char *argv[] = {
		filename,
		"N:%s:%s",
		str_bytes_v4,
		str_bytes_v6,
	};
	return rrd_update(4, argv);
}
