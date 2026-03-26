#include <server/server.h>
#define NOB_IMPLEMENTATION
#include "nob.h"
queue_t job_queue;
queue_t done_queue;

#define HOST "localhost"

// ===== Queue ops =====
void enqueue(queue_t *q, void *item) {
    pthread_mutex_lock(&q->lock);

    q->items[q->rear++] = item;
    q->rear %= QUEUE_SIZE;

    pthread_cond_signal(&q->cond);
    pthread_mutex_unlock(&q->lock);
}

void* dequeue(queue_t *q) {
    pthread_mutex_lock(&q->lock);

    while (q->front == q->rear)
        pthread_cond_wait(&q->cond, &q->lock);

    void *item = q->items[q->front++];
    q->front %= QUEUE_SIZE;

    pthread_mutex_unlock(&q->lock);
    return item;
}

// ===== Utils =====
int set_nonblocking(int fd) {
    int flags = fcntl(fd, F_GETFL, 0);
    return fcntl(fd, F_SETFL, flags | O_NONBLOCK);
}

// ===== Worker =====
int event_fd;

void* worker(void *arg) {
    while (1) {
        client_t *c = dequeue(&job_queue);
        // ===== PROCESS =====

        HttpMessage* message = (HttpMessage*) malloc(sizeof(HttpMessage));
        int ret = parseMessage(message,c->inbuf);

        if (ret == 0) {
            if (message->type == REQUEST)  {
                nob_log(NOB_INFO, "Recived request: \n");
                printMessage(message);
            } else if (message->type == RESPONSE)
            {
                
            } else {

            }
        } else {
            // build error message for parsing errors
        }


        c->out_len = c->in_len;
        c->out_sent = 0;
        c->in_len = 0;

        c->want_write = 1;
        enqueue(&done_queue, c);

        // Notify epoll thread
        uint64_t one = 1;
        write(event_fd, &one, sizeof(one));
    }
    return NULL;
}

int create_server() {
    int server_fd = socket(AF_INET, SOCK_STREAM, 0);

    int opt = 1;
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    struct sockaddr_in addr = {0};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(PORT);
    addr.sin_addr.s_addr = INADDR_ANY;

    bind(server_fd, (struct sockaddr*)&addr, sizeof(addr));
    listen(server_fd, SOMAXCONN);
    set_nonblocking(server_fd);

    return server_fd;
} 


// ===== Modify epoll events =====
void mod_epoll(int epfd, int fd, uint32_t events, client_t *client) {
    struct epoll_event ev;
    ev.events = events | EPOLLET;
    ev.data.ptr = client;

    epoll_ctl(epfd, EPOLL_CTL_MOD, fd, &ev);
}

// ===== Close client =====
void close_client(int epfd, client_t *client) {
    epoll_ctl(epfd, EPOLL_CTL_DEL, client->fd, NULL);
    if (client->message) {
        free(client->message);
    }
    close(client->fd);
    free(client);
}


int hasAllHeaders(char* buffer, size_t len) {

    for (size_t i = 0; i < len - 3; i++)
    {
        char b1 = buffer[i];
        char b2 = buffer[i + 1];
        char b3 = buffer[i + 2];
        char b4 = buffer[i + 3];
        if (b1 == '\r' && b2 == '\n' && b3  == '\r' && b4 == '\n' )
        {
            nob_log(NOB_INFO, "All Headers recived, ignoring body");
            return 0;
        }
        
    }
    
    return 1;
}

int main() {
    
    int server_fd = create_server();
    int epfd = epoll_create1(0);
    struct epoll_event ev, events[MAX_EVENTS];

    // ===== eventfd =====
    event_fd = eventfd(0, EFD_NONBLOCK);

    // Register server socket
    ev.events = EPOLLIN | EPOLLET;
    ev.data.fd = server_fd;
    epoll_ctl(epfd, EPOLL_CTL_ADD, server_fd, &ev);

    // Register eventfd
    ev.events = EPOLLIN;
    ev.data.fd = event_fd;
    epoll_ctl(epfd, EPOLL_CTL_ADD, event_fd, &ev);

    // Init queues
    pthread_mutex_init(&job_queue.lock, NULL);
    pthread_cond_init(&job_queue.cond, NULL);

    pthread_mutex_init(&done_queue.lock, NULL);
    pthread_cond_init(&done_queue.cond, NULL);

    // Start workers
    pthread_t threads[THREADS];
    for (int i = 0; i < THREADS; i++)
        pthread_create(&threads[i], NULL, worker, NULL);

    printf("Server on port %d\n", PORT);

    while (1) {
        int n = epoll_wait(epfd, events, MAX_EVENTS, -1);

        for (int i = 0; i < n; i++) {

            // ===== ACCEPT =====
            if (events[i].data.fd == server_fd) {
                while (1) {
                    int fd = accept(server_fd, NULL, NULL);
                    if (fd == -1) {
                        if (errno == EAGAIN) break;
                        break;
                    }

                    set_nonblocking(fd);

                    client_t *c = calloc(1, sizeof(client_t));
                    c->fd = fd;
                    c->state = STATE_READING_HEADERS;
                    c->message = NULL;
                    ev.events = EPOLLIN | EPOLLET;
                    ev.data.ptr = c;
                    epoll_ctl(epfd, EPOLL_CTL_ADD, fd, &ev);
                    

                    printf("Client %d connected\n", fd);
                }
            }

            // ===== eventfd (worker completed jobs) =====
            else if (events[i].data.fd == event_fd) {
                uint64_t count;
                read(event_fd, &count, sizeof(count)); // clear counter

                // Process all completed jobs
                while (1) {
                    pthread_mutex_lock(&done_queue.lock);
                    if (done_queue.front == done_queue.rear) {
                        pthread_mutex_unlock(&done_queue.lock);
                        break;
                    }

                    client_t *c = done_queue.items[done_queue.front++];
                    done_queue.front %= QUEUE_SIZE;
                    pthread_mutex_unlock(&done_queue.lock);
                    c->state = STATE_WRITING_RESPONSE;
                    // Enable write event
                    ev.events = EPOLLOUT | EPOLLET;
                    ev.data.ptr = c;
                    epoll_ctl(epfd, EPOLL_CTL_MOD, c->fd, &ev);
                }
            }

            // ===== CLIENT =====
            else {
                client_t *c = events[i].data.ptr;

                // READ
                if (c->state == STATE_READING_HEADERS) {
                    while (1) {
                        ssize_t nread = read(c->fd,
                                             c->inbuf + c->in_len,
                                             BUFFER_SIZE - c->in_len);

                        if (nread <= 0) {
                            if (nread == 0 || errno != EAGAIN) {
                                close_client(epfd, c);
                            }
                            break;
                        }

                        c->in_len += nread;
                    }

                    if (c->in_len > 0 && !hasAllHeaders(c->inbuf, c->in_len)) {
                        c->state = STATE_PROCESSING;
                        enqueue(&job_queue, c);
                    }
                        
                }

                // WRITE
                if (c->state == STATE_WRITING_RESPONSE) {
                    while (c->out_sent < c->out_len) {
                        ssize_t nwrite = write(c->fd,
                                               c->outbuf + c->out_sent,
                                               c->out_len - c->out_sent);

                        if (nwrite == -1) {
                            if (errno == EAGAIN) break;
                            close_client(epfd, c);
                            break;
                        }

                        c->out_sent += nwrite;
                    }

                    if (c->out_sent == c->out_len) {
                        c->out_len = 0;
                        c->out_sent = 0;
                        c->state = STATE_READING_HEADERS;
                        mod_epoll(epfd, c->fd, EPOLLIN, c);
                    }
                }
            }
        }
    }
}