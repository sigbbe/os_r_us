
#include "../include/bbuffer.h"
#include "../include/sem.h"
#include "../include/server.h"

#define _REENTRANT
#define _POSIX_PTHREAD_SEMANTICS
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

int *do_some() {
  // The argument sockfd is a socket that has been created with socket(2), bound
  // to a local address with bind(2), and is listening for connections after a
  // listen(2).
  //   int getaddrinfo(const char *node, const char *service,
  //                   const struct addrinfo *hints, struct addrinfo **res);

  char buffer[BUFSIZ];
  char protoname[] = "tcp";
  int i;
  int socket_fd, client_socket_fd;
  socklen_t client_len;
  struct sockaddr_in client_address, server_address;
  unsigned short server_port = 8080u;

  socket_fd = socket(AF_INET, SOCK_STREAM, 0);
  if (socket_fd == -1) {
    perror("socket");
    exit(EXIT_FAILURE);
  }
  server_address.sin_family = AF_INET;
  server_address.sin_addr.s_addr = htonl(INADDR_ANY);
  server_address.sin_port = htons(server_port);
  if (bind(socket_fd, (struct sockaddr *)&client_address,
           sizeof(client_address)) < 0) {
    // print the error message
    perror("bind failed. Error");
    return NULL;
  }
  puts("bind done");
  close(socket_fd);
  //   if (bind(socket_fd, (struct sockaddr *)&server_address,
  //            sizeof(server_address)) == -1) {
  //     perror("bind");
  //     exit(EXIT_FAILURE);
  //   }
  if (listen(socket_fd, 5) == -1) {
    perror("listen");
    exit(EXIT_FAILURE);
  }
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
  close(socket_fd);
  //   FD_CLR(socket_fd, &read_fds);
  printf("%s\n", "do_some");
  return NULL;
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
  printf("%d\n", server_listen(&server));
  return 0;
}

/*
 *
 * Compiling the program:
 *
 * gcc -std=gnu99 -lpthread src/mtwwwd.c -o mtwwwd
 *
 */