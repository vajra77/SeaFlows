//
// Created by Francesco Ferreri (Namex) on 19/03/25.
//

#ifndef RRDTOOL_H
#define RRDTOOL_H
#include <rrd.h>

void create_rrd(char *filename);
void update_rrd(char *filename);
#endif //RRDTOOL_H
