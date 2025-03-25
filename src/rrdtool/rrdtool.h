//
// Created by Francesco Ferreri (Namex) on 19/03/25.
//

#ifndef RRDTOOL_H
#define RRDTOOL_H
#include <rrd.h>

#include "matrix/matrix.h"

int rrd_store_flow(const rrd_client_t *, const srcnode_t *, const dstnode_t  *);
#endif //RRDTOOL_H
