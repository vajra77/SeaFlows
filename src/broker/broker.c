//
// Created by Francesco Ferreri (Namex) on 17/03/25.
//

#include <stdlib.h>
#include <syslog.h>
#include <gc.h>
#include <unistd.h>

#include "broker.h"
#include "../sflow/sflow.h"
#include "../matrix/matrix.h"
#include "../queue/queue.h"


void* broker_thread(void *arg){

  pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
  pthread_setcanceltype(PTHREAD_CANCEL_DEFERRED, NULL);

  const broker_data_t *broker_data = arg;

  syslog(LOG_DEBUG, "Starting broker thread");

  sleep(1);

  for (;;) {
    storable_flow_t	*storable_flow = queue_pop(broker_data->queue);
    if(storable_flow != NULL){
      //matrix_add_flow(broker_data->matrix, storable_flow);
      GC_free(storable_flow);
    }
    pthread_testcancel();
  }
}
