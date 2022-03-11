
#include "../include/bbuffer.h"
#include "../include/sem.h"
#include "../include/server.h"

#define PORT 6789
#define MAXREQ (4096 * 1024)

#define _REENTRANT
#define _POSIX_PTHREAD_SEMANTICS
#include <errno.h>
#include <netdb.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

void *thread_main(void *args) {
  //   struct BNDBUF *bb = (struct BNDBUF *)args;
  //   int i;
  //   for (i = 0; i < 10; i++) {
  //     P(bb->sem_empty);
  //     bb->buffer[bb->in] = i;
  //     bb->in = (bb->in + 1) % bb->size;
  //     bb->count++;
  //     V(bb->sem_full);
  //   }
  return NULL;
};

int do_some() {
  // The argument sockfd is a socket that has been created with socket(2), bound
  // to a local address with bind(2), and is listening for connections after a
  // listen(2).
  //   int getaddrinfo(const char *node, const char *service,
  //                   const struct addrinfo *hints, struct addrinfo **res);
  char buffer[64];
  sprintf(buffer, "HÆLLÆ %s", "PERO");
  char body[64];
  char msg[64];
  char protoname[] = "tcp";
  int i, n;
  int socket_fd, client_socket_fd;
  socklen_t client_len;
  struct sockaddr_in client_address, server_address;
  //   struct sockaddr *client_addr;

  socket_fd = socket(AF_INET, SOCK_STREAM, 0);
  if (socket_fd == -1) {
    printf("[SOCKET] %d: %s\n", errno, strerror(errno));
    return 1;
  }
  server_address.sin_family = AF_INET;
  server_address.sin_addr.s_addr = htonl(INADDR_ANY);
  server_address.sin_port = htons(PORT);
  if (bind(socket_fd, (struct sockaddr *)&client_address,
           sizeof(client_address)) < 0) {
    // print the error message
    printf("[BIND] %d: %s\n", errno, strerror(errno));
    return 1;
  }
  if (listen(socket_fd, 5) == -1) {
    printf("[LISTEN] %d: %s\n", errno, strerror(errno));
    return 1;
  }
  printf("listening on port %d\n", PORT);
  client_socket_fd =
      accept(socket_fd, (struct sockaddr *)&client_address, &client_len);
  if (client_socket_fd == -1) {
    printf("[ACCEPT] %d: %s\n", errno, strerror(errno));
    return 1;
  }
  //   n = read(client_socket_fd, buffer, sizeof(buffer) - 1);
  //   if (n < 0) {
  //     printf("[READ] %d: %s\n", errno, strerror(errno));
  //     return 1;
  //   }
  snprintf(body, sizeof(body),
           "<html>\n<body>\n<h1>Hello web browser</h1>\nYour request "
           "was\n<pre>%s</pre>\n</body>\n</html>\n",
           buffer);

  snprintf(msg, sizeof(msg),
           "HTTP/1.0 200 OK\n"
           "Content-Type: text/html\n"
           "Content-Length: %lu\n\n%s",
           strlen(body), body);
  n = send(client_socket_fd, msg, strlen(msg), 0);
  if (n < 0) {
    printf("[HEADER] %d: %s\n", errno, strerror(errno));
    return 1;
  }
  n = send(client_socket_fd, body, strlen(body), 0);
  if (n < 0) {
    printf("[BODY] %d: %s\n", errno, strerror(errno));
    return 1;
  }
  close(client_socket_fd);
  close(socket_fd);
  // fprintf(stderr, "listening on port %d\n", server_port);
  // int BUFFER_SIZE = 100;
  // memset(buffer, 0, BUFFER_SIZE);
  // int n = read(socket_fd, buffer, (BUFFER_SIZE - 1));
  // if (n == 0) {
  //   fprintf(stderr, "Connection to client lost\n\n");
  //   break;
  // } else if (n < 0) {
  //   fprintf(stderr, "Error reading from socket %s\n", strerror(1));
  //   break;
  // }
  // /* Print the message we received */
  // printf("Message received: %s\n", buffer);
  //   FD_CLR(socket_fd, &read_fds);
  return 0;
}

/*
 *
 * You can call the program like this:
 *
 * mtwwwd <www-path> <port> #threads #bufferslots
 *
 * or like this:
 *
 * mtwwwd www-path port
 *
 */
int main(int argc, char **argv, char **envp) {
  int i;
  int nthreads = 1;
  int nbufferslots = 1;
  char *www_path = ".";
  char *port = "8080";
  char *threads = "1";
  char *bufferslots = "1";
  char *tmp;
  struct BNDBUF *bb;
  if (argc > 1) {
    www_path = argv[1];
  }
  if (argc > 2) {
    port = argv[2];
  }
  if (argc > 3) {
    threads = argv[3];
  }
  if (argc > 4) {
    bufferslots = argv[4];
  }
  //   nthreads = atoi(threads);
  //   nbufferslots = atoi(bufferslots);
  //   printf("mtwwwd: starting %d threads, %d bufferslots\n", nthreads,
  //          nbufferslots);
  //   bb = bb_init(nbufferslots);
  //   for (i = 0; i < nthreads; i++) {
  //     pthread_t thread;
  //     pthread_create(&thread, NULL, &thread_main, (void *)www_path);
  //   }
  server_t server;
  //   printf("%d\n", server_listen(&server));
  printf("%d\n", do_some());
  return 0;
}

/*
 *
 * Compiling the program:
 *
 * gcc -std=gnu99 -lpthread src/mtwwwd.c -o mtwwwd
 *
 */