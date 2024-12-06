//
// Created by Francesco Ferreri (Namex) on 29/11/24.
//
#include <pthread.h>
#include "collector.h"

#define MAX_THREADS 16


int main(int argc, char **argv) {

    pthread_t    listener_threads[MAX_THREADS];

    /* create threads */
    for(int i = 0; i < 1; i++) {
        collector_address_t collector_addr;
        collector_addr.port = 9090;
        collector_addr.address = "127.0.0.1";
        pthread_create(&listener_threads[i], NULL, collector_thread, (void*)&collector_addr);
    }

    /* join threads */
    for(int i = 0; i < MAX_THREADS; i++) {
        pthread_join(listener_threads[i], NULL);
    }
}