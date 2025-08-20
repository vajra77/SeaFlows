//
// Created by Francesco Ferreri (Namex) on 19/03/25.
//

#ifndef RRDTOOL_H
#define RRDTOOL_H

#define RRDCACHED_ADDRESS "127.0.0.1:42217"

#include <stdint.h>


void rrdtool_store(const char*, const char*, const uint32_t, const uint32_t);
void rrdtool_prepare(const char*, const char*);
#endif //RRDTOOL_H
