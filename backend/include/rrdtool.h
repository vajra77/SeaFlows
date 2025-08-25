//
// Created by Francesco Ferreri (Namex) on 19/03/25.
//

#ifndef RRDTOOL_H
#define RRDTOOL_H

#include <stdint.h>


int rrdtool_prepare(const char*, const char*);
int rrdtool_store(const char*, const char*, const uint32_t, const uint32_t);
#endif //RRDTOOL_H
