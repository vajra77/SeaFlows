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
#include <syslog.h>
#include <pthread.h>
#include "collector.h"

#define MAX_THREADS 24


void usage(){
	printf("Usage:\n");
	printf("\tcollector [options]\n");
	printf("Options:\n");
	printf("\t-h, --help\t\tShow this help and exit\n");
	printf("\t-a, --address <ip_address>\t\tListen address\n");
	printf("\t-t, --threads <n_threads>\t\tNumber of listener threads\n");
}

void launch_collector(char *listen_address, int num_threads){

	pthread_t    listener_threads[MAX_THREADS];

  	/* create threads */
  	for(int i = 0; i < num_threads; i++) {
		collector_address_t collector_addr;
		collector_addr.port = 9090 + i;
		collector_addr.address = listen_address;
		pthread_create(&listener_threads[i], NULL, collector_thread, (void*)&collector_addr);
  	}

  	/* join threads */
  	for(int i = 0; i < num_threads; i++) {
		pthread_join(listener_threads[i], NULL);
  	}
}

int main(int argc, char **argv) {

	 char listen_address[1024];
	 int num_threads;
	 int c;

	 if(argc < 2){
		 usage();
		 exit(1);
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
				 exit(0);
			 default:
				 usage();
				 exit(1);
		 }
	 }

     if(num_threads >= MAX_THREADS){
       printf("Number of threads is too large\n");
       exit(1);
     }

	pid_t pid;

	/* Fork off the parent process */
	pid = fork();

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

	launch_collector(listen_address, num_threads);

	exit(0);
}