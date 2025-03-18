//
// Created by Francesco Ferreri (Namex) on 17/03/25.
//

#include <stdlib.h>
#include "broker.h"
#include "../sflow/sflow.h"
#include "../matrix/matrix.h"
#include "../queue/queue.h"

void* broker_thread(void *arg){

  broker_data_t *broker_data = (broker_data_t*)arg;

  while(1){
    storable_flow_t	*storable_flow = queue_pop(broker_data->queue);
    if(storable_flow != NULL){
      matrix_add_flow(broker_data->matrix, storable_flow);
      free(storable_flow);
    }
  }
}