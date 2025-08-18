//
// Created by Francesco Ferreri (Namex) on 18/08/25.
//

#ifndef MEMORY_H
#define MEMORY_H
#ifdef WITH_GC
#include <gc.h>
#define MEM_alloc(x) GC_alloc(x)
#define MEM_free(x) GC_free(x)
#else
#include <stdlib.h>
#define MEM_alloc(x) malloc(x)
#define MEM_free(x) free(x)
#endif
#endif //MEMORY_H
