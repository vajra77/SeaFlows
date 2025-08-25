//
// Created by Francesco Ferreri (Namex) on 19/03/25.
//

#ifndef RRDTOOL_H
#define RRDTOOL_H

#define RRDCACHED_ADDRESS "127.0.0.1:42217"

#include <stdint.h>

extern char datadir[64];
extern char rrdcached_address[64];


int rrdtool_prepare(const char*, const char*);
int rrdtool_store(const char*, const char*, const uint32_t, const uint32_t);
#endif //RRDTOOL_H
