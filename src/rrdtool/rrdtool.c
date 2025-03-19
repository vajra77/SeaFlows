//
// Created by Francesco Ferreri (Namex) on 19/03/25.
//

#include <stdio.h>
#include <rrd.h>
#include <time.h>
#include "rrdtool.h"

void create_rrd(char *filename) {
	const size_t argc = 10;
	char *argv[] = {
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
	rrd_create(filename, 300, time(NULL), 0, argc, argv);
}

void update_rrd(char *filename, unsigned int bytes_v4, unsigned int bytes_v6) {

}
