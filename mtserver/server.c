/**
* server.c, copyright 2001 Steve Gribble
*
* The server is a single-threaded program.  First, it opens
* up a "listening socket" so that clients can connect to
* it.  Then, it enters a tight loop; in each iteration, it
* accepts a new connection from the client, reads a request,
* computes for a while, sends a response, then closes the
* connection.
*/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <ctype.h>
#include <errno.h>
#include <signal.h>
#include "lib/socklib.h"
#include "common.h"
#include "threadpool.c"
#include <sys/time.h>
extern int errno;

int setup_listen(char *socketNumber);
char *read_request(int fd);
char *process_request(char *request, int *response_length);
void send_response(int fd, char *response, int response_length);
void dispatchRunner(void* arg);
int date;
int isClose;
struct timeval start;
struct timeval finish;
/**
* This program should be invoked as "./server <socketnumber>", for
* example, "./server 4342".
*/
void server_close(int signum){
  printf("SERVER IS SHUTTING DOWN!\n");
  isClose = 1;
  exit(0);
}

int main(int argc, char **argv){
    char buf[1000];
    int  socket_listen;
    int  socket_talk;
    threadpool networkpool;
    if(argc != 4){
        fprintf(stderr, "(SERVER): Invoke as  './server socknum thredNumber n'\n");
        fprintf(stderr, "(SERVER): for example, './server 4434 32 100'\n");
        exit(-1);
    }
    long counter = 0;
    long time = 0;
    isClose = 0;
    isClose = atoi(argv[3]);
    int poolsize = atoi(argv[2]);
    socket_listen = setup_listen(argv[1]);
    networkpool = create_threadpool(poolsize);

    /*
    * Set up the 'listening socket'.  This establishes a network
    * IP_address:port_number that other programs can connect with.
    */
    signal(SIGINT, server_close);

    /*
    * Here's the main loop of our program.  Inside the loop, the
    * one thread in the server performs the following steps:
    *
    *  1) Wait on the socket for a new connection to arrive.  This
    *     is done using the "accept" library call.  The return value
    *     of "accept" is a file descriptor for a new data socket associated
    *     with the new connection.  The 'listening socket' still exists,
    *     so more connections can be made to it later.
    *
    *  2) Read a request off of the listening socket.  Requests
    *     are, by definition, REQUEST_SIZE bytes long.
    *
    *  3) Process the request.
    *
    *  4) Write a response back to the client.
    *
    *  5) Close the data socket associated with the connection
    */

    while(1) {
        if(isClose == 1){
            break;
        }
        socket_talk = saccept(socket_listen);  // step 1
        if (socket_talk < 0) {
            fprintf(stderr, "An error occured in the server; a connection\n");
            fprintf(stderr, "failed because of ");
            perror("");
            exit(1);
        }
        dispatch(networkpool, dispatchRunner, (void*) (size_t) socket_talk);
        if(counter == 1) {
            gettimeofday(&start, 0);    
            printf("Start at time 0\n");
        }
        if(counter % 500 == 0 && counter > 0){
            gettimeofday(&finish, 0);  
            time = (finish.tv_usec + finish.tv_sec*1000000 - (start.tv_usec + (start.tv_sec*1000000)))/1000000;
            printf("%ld tasks, at time %ld s.\n", counter, time);
            printf("%ld this is counter\n",counter);
        } 
        counter++;
    }
    long rate = counter / time;
    destroy_threadpool(networkpool);
    close(socket_listen);
    exit(0);
}
void dispatchRunner(void* arg){
    char *request = NULL;
    char *response = NULL;
    int socket_talk = (int) arg;
    request = read_request(socket_talk);
    if(request != NULL) {
        int response_length;
        response = process_request(request, &response_length);
        if(response != NULL) {
            send_response(socket_talk, response, response_length); 
        }
    }
    close(socket_talk);
    if(request != NULL){
        free(request);
    }
    if(response != NULL){
        free(response);
    }
}

/**
* This function accepts a string of the form "5654", and opens up
* a listening socket on the port associated with that string.  In
* case of error, this function simply bonks out.
*/

int setup_listen(char *socketNumber) {
    int socket_listen;

    if ((socket_listen = slisten(socketNumber)) < 0) {
        perror("(SERVER): slisten");
        exit(1);
    }

    return socket_listen;
}

/**
* This function reads a request off of the given socket.
* This function is thread-safe.
*/

char *read_request(int fd) {
    char *request = (char *) malloc(REQUEST_SIZE*sizeof(char));
    int   ret;

    if (request == NULL) {
        fprintf(stderr, "(SERVER): out of memory!\n");
        exit(-1);
    }

    ret = correct_read(fd, request, REQUEST_SIZE);
    if (ret != REQUEST_SIZE) {
        free(request);
        request = NULL;
    }
    return request;
}

/**
* This function crunches on a request, returning a response.
* This is where all of the hard work happens.
* This function is thread-safe.
*/

#define NUM_LOOPS 0

char *process_request(char *request, int *response_length) {
    char *response = (char *) malloc(RESPONSE_SIZE*sizeof(char));
    int   i,j;
    for (i=0; i<RESPONSE_SIZE; i++)
    response[i] = request[i%REQUEST_SIZE];

    for (j=0; j<NUM_LOOPS+isClose; j++) {
        for (i=0; i<RESPONSE_SIZE; i++) {
            char swap;

            swap = response[((i+1)%RESPONSE_SIZE)];
            response[((i+1)%RESPONSE_SIZE)] = response[i];
            response[i] = swap;
        }
    }
    *response_length = RESPONSE_SIZE;
    return response;
}

