//
// Created by Francesco Ferreri (Namex) on 19/03/25.
//

#ifndef RRDTOOL_H
#define RRDTOOL_H
#include <rrd.h>


int create_rrd(char *);
int update_rrd(char *, unsigned int, unsigned int);
#endif //RRDTOOL_H
