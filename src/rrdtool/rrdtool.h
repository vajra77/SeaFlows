//
// Created by Francesco Ferreri (Namex) on 19/03/25.
//

#ifndef RRDTOOL_H
#define RRDTOOL_H

#define RRDCACHE_ADDRESS "127.0.0.1:42217"

#include "sflow/sflow.h"

void cache_store(const storable_flow_t *);
void cache_prepare(const storable_flow_t *);
#endif //RRDTOOL_H
