//
// Created by Francesco Ferreri (Namex) on 20/08/25.
//

#ifndef GC_H
#define GC_H

#define MAX_GC_DATA 2048

#include <stdint.h>

typedef struct gc {
	int size;
  	void *data[MAX_GC_DATA];
} gc_t;

void gc_init(gc_t *gc);
void *gc_alloc(gc_t *, size_t);
void gc_free(gc_t *, void *);
void gc_cleanup(gc_t *);
#endif //GC_H
