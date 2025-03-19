//
// Created by Francesco Ferreri (Namex) on 29/11/24.
//
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <pthread.h>
#include "seaflows.h"
#include "collector/collector.h"
#include "broker/broker.h"
#include "matrix/matrix.h"


#define MAX_THREADS 24


void usage(){
	printf("Usage:\n");
	printf("\tcollector [options]\n");
	printf("Options:\n");
	printf("\t-h\t\t\tShow this help and exit\n");
	printf("\t-a <ip_address>\t\tListen address\n");
	printf("\t-t <n_threads>\t\tNumber of listener threads\n");
}

void* matrix_dumper_thread(void *arg) {
	matrix_t **flow_matrix = arg;
	sleep(300);
	for(int i = 0; i < MAX_THREADS; i++) {
		if(flow_matrix[i] != NULL) {
			matrix_dump(flow_matrix[i]);
		}
	}
	return NULL;
}

int main(const int argc, char **argv) {

	 char listen_address[1024];
	 int num_threads = 0;
	 int c;

	 if(argc < 2){
		 usage();
		 exit(EXIT_FAILURE);
	 }

	 while((c = getopt(argc, argv, "a:t:h")) != -1) {
		 switch(c) {
			case 'a':
				strcpy(listen_address, optarg);
				break;
		 	case 't':
				num_threads = atoi(optarg);
				break;
			case 'h':
				usage();
				exit(EXIT_SUCCESS);
		 	default:
		 		break;
		 }
	 }

     if(num_threads >= MAX_THREADS){
       printf("Number of threads is too large\n");
       exit(EXIT_FAILURE);
     }

	pid_t pid = fork();

	/* An error occurred */
	if (pid < 0)
		exit(EXIT_FAILURE);

	/* Success: Let the parent terminate */
	if (pid > 0)
		exit(EXIT_SUCCESS);

	/* On success: The child process becomes session leader */
	if (setsid() < 0)
		exit(EXIT_FAILURE);

	/* Catch, ignore and handle signals */
	/*TODO: Implement a working signal handler */
	signal(SIGCHLD, SIG_IGN);
	signal(SIGHUP, SIG_IGN);

	/* Fork off for the second time*/
	pid = fork();

	/* An error occurred */
	if (pid < 0)
		exit(EXIT_FAILURE);

	/* Success: Let the parent terminate */
	if (pid > 0)
		exit(EXIT_SUCCESS);

	/* Set new file permissions */
	umask(0);

	/* Change the working directory to the root directory */
	/* or another appropriated directory */
	chdir("/");

	/* Close all open file descriptors */
	int x;
	for (x = sysconf(_SC_OPEN_MAX); x>=0; x--){
		close (x);
	}


	pthread_t			collector_threads[MAX_THREADS];
    pthread_t			broker_threads[MAX_THREADS];

    queue_t				*message_queues[MAX_THREADS];
	memset(message_queues, 0, sizeof(message_queues));

    matrix_t			*flow_matrix[MAX_THREADS];
	memset(flow_matrix, 0, sizeof(flow_matrix));

	collector_data_t	collector_data[MAX_THREADS];
    broker_data_t		broker_data[MAX_THREADS];

    for(int i = 0; i < num_threads; i++){
    	message_queues[i] = malloc(sizeof(queue_t));
		queue_init(message_queues[i]);
		flow_matrix[i] = malloc(sizeof(matrix_t));
		matrix_init(flow_matrix[i]);
    }

	/* create threads */
	for(int i = 0; i < num_threads; i++) {
		collector_data[i].port = SEAFLOWS_LISTENER_PORT + i;
		collector_data[i].address = listen_address;
        collector_data[i].queue = message_queues[i];

        broker_data[i].queue = message_queues[i];
        broker_data[i].matrix = flow_matrix[i];

		pthread_create(&collector_threads[i], NULL, collector_thread, (void*)&collector_data[i]);
        pthread_create(&broker_threads[i], NULL, broker_thread, (void*)&broker_data[i]);
	}
	pthread_t dumper_thread;
	pthread_create(&dumper_thread, NULL, matrix_dumper_thread, (void*)flow_matrix);

	/* join threads */
	for(int i = 0; i < num_threads; i++) {
		pthread_join(collector_threads[i], NULL);
        pthread_join(broker_threads[i], NULL);
		free(message_queues[i]);
		free(flow_matrix[i]);
	}
	pthread_join(dumper_thread, NULL);

	exit(0);
}
