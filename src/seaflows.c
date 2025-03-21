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
#include <syslog.h>
#include "seaflows.h"
#include "collector/collector.h"
#include "broker/broker.h"
#include "matrix/matrix.h"


#define MAX_THREADS 24

/* thread share/control variables */
pthread_t			collector_threads[MAX_THREADS];
pthread_t			broker_threads[MAX_THREADS];
pthread_t			dumper_thread;

queue_t				*message_queues[MAX_THREADS];
matrix_t			*flow_matrix[MAX_THREADS];
collector_data_t	collector_data[MAX_THREADS];
broker_data_t		broker_data[MAX_THREADS];


void usage(){
	printf("Usage:\n");
	printf("\tseaflows [options]\n");
	printf("Options:\n");
	printf("\t-h\t\t\tShow this help and exit\n");
	printf("\t-l <ip_address>\t\tListen address\n");
	printf("\t-t <n_threads>\t\tNumber of listener threads\n");
}

void signal_handler(int sig) {
	syslog(LOG_INFO, "Received signal %d", sig);
	for(int i = 0; i < MAX_THREADS; i++) {
		pthread_cancel(collector_threads[i]);
		// pthread_cancel(broker_threads[i]);
		// pthread_cancel(dumper_thread);
	}

	for(int i = 0; i < MAX_THREADS; i++) {
		pthread_join(collector_threads[i], NULL);
		// pthread_join(broker_threads[i], NULL);
		// pthread_join(dumper_thread, NULL);
	}

	// for(int i = 0; i < MAX_THREADS; i++) {
	// 	queue_destroy(message_queues[i]);
	// 	matrix_destroy(flow_matrix[i]);
	// }
	closelog();
	exit(EXIT_SUCCESS);
}

void* matrix_dumper_thread(void *arg) {

	pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
	pthread_setcanceltype(PTHREAD_CANCEL_DEFERRED, NULL);

	matrix_t **flow_matrix = arg;

	for (;;) {
		sleep(300);
		for(int i = 0; i < MAX_THREADS; i++) {
			if(flow_matrix[i] != NULL) {
				matrix_dump(flow_matrix[i]);
			}
		}
		pthread_testcancel();
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

	 while((c = getopt(argc, argv, "l:t:h")) != -1) {
		 switch(c) {
			case 'l':
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

	/* first fork */
	pid_t pid = fork();

	if (pid < 0)
		exit(EXIT_FAILURE);

	if (pid > 0)
		exit(EXIT_SUCCESS);

	if (setsid() < 0)
		exit(EXIT_FAILURE);

	signal(SIGINT, signal_handler);
	signal(SIGHUP, signal_handler);
	signal(SIGQUIT, signal_handler);

	/* second fork */
	pid = fork();

	if (pid < 0)
		exit(EXIT_FAILURE);

	if (pid > 0)
		exit(EXIT_SUCCESS);

	umask(0);

	chdir("/data/rrd/");

	openlog("seaflows", LOG_PID, LOG_DAEMON);

	memset(message_queues, 0, sizeof(message_queues));
	memset(flow_matrix, 0, sizeof(flow_matrix));

	/* create threads */
	for(int i = 0; i < num_threads; i++) {
		message_queues[i] = malloc(sizeof(queue_t));
		queue_init(message_queues[i]);
		flow_matrix[i] = malloc(sizeof(matrix_t));
		matrix_init(flow_matrix[i]);

		collector_data[i].port = SEAFLOWS_LISTENER_PORT + i;
		collector_data[i].address = listen_address;
        collector_data[i].queue = message_queues[i];

        broker_data[i].queue = message_queues[i];
        broker_data[i].matrix = flow_matrix[i];

		pthread_create(&collector_threads[i], NULL, collector_thread, &collector_data[i]);
        // pthread_create(&broker_threads[i], NULL, broker_thread, &broker_data[i]);
	}
	// pthread_create(&dumper_thread, NULL, matrix_dumper_thread, (void*)flow_matrix);

	/* sleep and wait for signals */
	for (;;) {
		sleep(300);
	}

	exit(EXIT_SUCCESS);
}
