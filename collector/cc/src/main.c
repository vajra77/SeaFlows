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
#include <gc.h>

#include "config.h"
#include "seaflows.h"
#include "listener.h"
#include "broker.h"
#include "bucket.h"
#include "rrdtool.h"


#define MAX_THREADS 24
#define MAX_FLOWS 1024


/* thread share/control variables */
static pthread_t			listener[MAX_THREADS];
static pthread_t			broker[MAX_THREADS];
static int num_threads = 0;

listener_data_t	listener_data[MAX_THREADS];
broker_data_t		broker_data[MAX_THREADS];
bucket_t*			bucket[MAX_THREADS];


void usage(){
	printf("Usage:\n");
	printf("\tseaflows [options]\n");
	printf("Options:\n");
	printf("\t-h\t\t\tShow this help and exit\n");
	printf("\t-l <ip_address>\t\tListen address\n");
	printf("\t-t <n_threads>\t\tNumber of listener threads\n");
}

void signal_handler(const int sig) {
	syslog(LOG_INFO, "received signal %d", sig);
	for(int i = 0; i < num_threads; i++) {
		pthread_cancel(listener[i]);
		pthread_cancel(broker[i]);
	}

	for(int i = 0; i < num_threads; i++) {
		pthread_join(listener[i], NULL);
		pthread_join(broker[i], NULL);
	}

	for(int i = 0; i < num_threads; i++) {
		free(bucket[i]);
	}

	closelog();
	exit(EXIT_SUCCESS);
}

int main(const int argc, char **argv) {

	 char listen_address[30];
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

	int dir = chdir(RRD_DIR);

    if(dir < 0) {
          syslog(LOG_ERR, "chdir failed");
          exit(EXIT_FAILURE);
    }

	openlog("seaflows", LOG_PID, LOG_DAEMON);


	/* create threads */
	for(int i = 0; i < num_threads; i++) {

		bucket[i] = malloc(sizeof(bucket_t));
		bucket_init(bucket[i], i);

		listener_data[i].id = i;
		listener_data[i].port = SEAFLOWS_LISTENER_PORT + i;
		listener_data[i].address = listen_address;
		listener_data[i].bucket = bucket[i];

		broker_data[i].id = i;
		broker_data[i].bucket = bucket[i];

		pthread_create(&listener[i], NULL, listener_thread, &listener_data[i]);
		pthread_create(&broker[i], NULL, broker_thread, &broker_data[i]);
	}

	sleep(10);

	for(int i = 0; i < num_threads; i++) {
		pthread_join(listener[i], NULL);
		pthread_join(broker[i], NULL);
	}

	exit(EXIT_SUCCESS);
}
