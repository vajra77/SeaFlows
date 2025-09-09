//
// Created by Francesco Ferreri (Namex) on 20/08/25.
//

#include <stdlib.h>
#include <string.h>
#include "gc.h"


void gc_init(gc_t *gc) {

    gc->size = 0;
    bzero(gc->data, MAX_GC_DATA);
}

void *gc_alloc(gc_t *gc, size_t size) {

    if(gc->size >= MAX_GC_DATA) {
    	return NULL;
    }
    void *ptr = malloc(size);
    gc->data[gc->size] = ptr;
    gc->size++;
    return ptr;
}

void gc_cleanup(gc_t *gc) {

	for (int k = 0; k < gc->size; k++) {
    	free(gc->data[k]);
  	}
}