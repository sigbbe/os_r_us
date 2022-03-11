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

/**
 * Creates a socket for the server and makes it passive such that
 * we can wait for connections on it later.
 */
int server_listen(server_t *server) {
  // The `socket(2)` syscall creates an endpoint for communication
  // and returns a file descriptor that refers to that endpoint.
  //
  // It takes three arguments (the last being just to provide greater
  // specificity):
  // -    domain (communication domain)
  //      AF_INET              IPv4 Internet protocols
  //
  // -    type (communication semantics)
  //      SOCK_STREAM          Provides sequenced, reliable,
  //                           two-way, connection-based byte
  //                           streams.
  int err;
  err = (server->listen_fd = socket(AF_INET, SOCK_STREAM, 0));
  if (err == -1) {
    perror("socket");
    printf("Failed to create socket endpoint\n");
    return err;
  }
  return 0;
}

/**
 * Accepts new connections and then prints `Hello World` to
 * them.
 */
int server_accept(server_t *server) {
  //
  return 0;
}