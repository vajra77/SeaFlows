//
// Created by Francesco Ferreri (Namex) on 17/03/25.
//

#ifndef BROKER_H
#define BROKER_H

#include "matrix/matrix.h"
#include "queue/queue.h"


typedef struct broker_data {
	queue_t		*queue;
	matrix_t	*matrix;
} broker_data_t;

void* broker_thread(void *);
#endif //BROKER_H
