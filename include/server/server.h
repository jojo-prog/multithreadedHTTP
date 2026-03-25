#ifndef SERVER_H
#define SERVER_H

#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <pthread.h>
#include <ctype.h>
#include <arpa/inet.h>
#include <sys/epoll.h>
#include <sys/eventfd.h>

#define PORT 8080
#define MAX_EVENTS 64
#define BUFFER_SIZE 4096
#define THREADS 4
#define QUEUE_SIZE 1024

// ===== Client =====
typedef struct client {
    int fd;

    char inbuf[BUFFER_SIZE];
    size_t in_len;

    char outbuf[BUFFER_SIZE];
    size_t out_len;
    size_t out_sent;

    int want_write;
    struct client *next;
} client_t;

// ===== Job =====
typedef struct {
    client_t *client;
} job_t;

// ===== Queue =====
typedef struct {
    void *items[QUEUE_SIZE];
    int front, rear;

    pthread_mutex_t lock;
    pthread_cond_t cond;
} queue_t;


#endif // !SERVER_H