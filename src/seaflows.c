//
// Created by Francesco Ferreri (Namex) on 29/11/24.
//
#include <ctype.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <pthread.h>
#include <syslog.h>
#include <gc.h>

#include "seaflows.h"
#include "memory.h"
#include "collector/collector.h"
#include "rrdtool/rrdtool.h"
#include "bucket/bucket.h"


#define MAX_THREADS 24
#define MAX_FLOWS 1024

/* thread share/control variables */
static pthread_t			threads[MAX_THREADS];
static collector_data_t		collector[MAX_THREADS];

static bucket_t bucket_v4;
static bucket_t bucket_v6;


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
		pthread_cancel(threads[i]);
	}

	for(int i = 0; i < MAX_THREADS; i++) {
		pthread_join(threads[i], NULL);
	}

	closelog();
	exit(EXIT_SUCCESS);
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

	chdir("/data/rrd");

	openlog("seaflows", LOG_PID, LOG_DAEMON);

	bucket_init(&bucket_v4);
	bucket_init(&bucket_v6);

	/* create threads */
	for(int i = 0; i < num_threads; i++) {

		collector[i].port = SEAFLOWS_LISTENER_PORT + i;
		collector[i].address = listen_address;
		collector[i].bucket_v4 = bucket_v4;
		collector[i].bucket_v6 = bucket_v6;

		pthread_create(&threads[i], NULL, collector_thread, &collector[i]);
	}

	for (;;) {
		/* round-robin */
		for (int i = 0; i < num_threads; i++) {
			bucket_node_t *node4 = bucket_remove(&collector[i].bucket_v4);
			if (node4 != NULL) {
				rrdtool_store(node4->src, node4->dst, 4, node4->in, node4->out);
				MEM_free(node4);
			}

			bucket_node_t *node6 = bucket_remove(&collector[i].bucket_v6);
			if (node6 != NULL) {
				rrdtool_store(node6->src, node6->dst, 6, node6->in, node6->out);
				MEM_free(node6);
			}
		}
	}

	exit(EXIT_SUCCESS);
}
