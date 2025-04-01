//
// Created by Francesco Ferreri (Namex) on 17/03/25.
//

#ifndef SEAFLOWS_H
#define SEAFLOWS_H
#define SEAFLOWS_VERSION_MAJOR 1
#define SEAFLOWS_VERSION_MINOR 0
#define SEAFLOWS_VERSION_PATCH 0
#define SEAFLOWS_VERSION_STRING "1.0.0"
#define SEAFLOWS_LISTENER_PORT 6343

#ifdef WITH_GC
#include <gc.h>
#define MEM_alloc(x) GC_alloc(x)
#define MEM_free(x) GC_free(x)
#else
#include <stdlib.h>
#define MEM_alloc(x) malloc(x)
#define MEM_free(x) free(x)
#endif
#endif //SEAFLOWS_H
