//
// Created by Francesco Ferreri (Namex) on 29/11/24.
//
#include <pthread.h>
#include "server.h"
#include "collector.h"


int main(int argc, char **argv) {

    pthread_t    listener_threads[MAX_THREADS];

    /* create threads */
    for(int i = 0; i < MAX_THREADS; i++) {

      pthread_create(&listener_threads[i], NULL, server_thread, NULL);
    }

    /* join threads */
    for(int i = 0; i < MAX_THREADS; i++) {
      pthread_join(listener_threads[i], NULL);
    }
}