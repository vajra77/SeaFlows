//
// Created by Francesco Ferreri (Namex) on 19/03/25.
//

#ifndef RRDTOOL_H
#define RRDTOOL_H

#define RRDCACHE_ADDRESS "127.0.0.1:42217"

#include "sflow/sflow.h"

void cache_store(const char*, const char*, const uint32_t, const uint32_t, const uint32_t);
void cache_prepare(const char*, const char*, const uint32_t);
#endif //RRDTOOL_H
