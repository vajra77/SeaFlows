//
// Created by Francesco Ferreri (Namex) on 19/03/25.
//

#ifndef RRDTOOL_H
#define RRDTOOL_H

#include <rrd.h>

#include "matrix/matrix.h"

int rrd_store_flow(const srcnode_t *, const dstnode_t  *);
int rrd_store_peer(const srcnode_t *);
#endif //RRDTOOL_H
