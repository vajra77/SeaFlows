//
// Created by Francesco Ferreri (Namex) on 19/03/25.
//

#ifndef RRDTOOL_H
#define RRDTOOL_H

#define RRDCACHE_ADDRESS "127.0.0.1:42217"

#include "sflow/sflow.h"

int cache_flow(const storable_flow_t *);
#endif //RRDTOOL_H
