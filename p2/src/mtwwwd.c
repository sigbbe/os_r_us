
#include "../include/bbuffer.h"
#include "../include/sem.h"
#include "../include/server.h"

#define PORT 8000
#define MAXREQ (4096 * 1024)
#define CRLF "\r\n"

#define _REENTRANT
#define _POSIX_PTHREAD_SEMANTICS
#include <assert.h>
#include <errno.h>
#include <malloc.h>
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

void check_error(int code, char *format_str, int *should_run) {
  if (code < 0) {
    *should_run = 0;
    fprintf(stderr, format_str, errno, strerror(errno));
  }
}

int parse(const char *line, char r_path[], char r_query[]) {
  /* Find out where everything is */
  const char *start_of_path = strchr(line, ' ') + 1;
  const char *start_of_query = strchr(start_of_path, '?');
  const char *end_of_query = strchr(start_of_query, ' ');

  /* Get the right amount of memory */
  char path[start_of_query - start_of_path];
  char query[end_of_query - start_of_query];

  /* Copy the strings into our memory */
  strncpy(path, start_of_path, start_of_query - start_of_path);
  strncpy(query, start_of_query, end_of_query - start_of_query);

  /* Null terminators (because strncpy does not provide them) */
  path[sizeof(path)] = 0;
  query[sizeof(query)] = 0;

  //   printf("BAISELURE\n");
  strncpy(r_path, path, sizeof(path));
  strncpy(r_query, query, sizeof(query));
}

void read_file(char *path, char buf[], int size) {
  FILE *fp = fopen(path, "rb");
  fseek(fp, 0, SEEK_END);
  long fsize = ftell(fp);
  fseek(fp, 0, SEEK_SET); /* same as rewind(f); */

  fread(buf, fsize, 1, fp);
  fclose(fp);
  buf[fsize] = 0;
}

int do_some() {
  // The argument sockfd is a socket that has been created with socket(2), bound
  // to a local address with bind(2), and is listening for connections after a
  // listen(2).

  // for checking write success
  int should_run, write_bit, read_bit, request_counter = 0;
  should_run = 1;

  // file descriptors for server and client file descriptors
  int server_sock_fd, client_sock_fd, bind_success;
  // address of the server
  struct sockaddr_in server_addr, client_addr;
  // length of the address
  socklen_t client_addr_len;

  // create a socket
  server_sock_fd = socket(AF_INET, SOCK_STREAM, 0);
  check_error(server_sock_fd, "[SOCKET]\t%d: %s\n", &should_run);

  // why do we need to set server address to 0?
  bzero((char *)&server_addr, sizeof(server_addr));
  // IP protocol family
  server_addr.sin_family = AF_INET;
  // allow any address to connect
  server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
  // set port number
  server_addr.sin_port = htons(PORT);

  // enable reuse of port
  int optval = 1;
  setsockopt(server_sock_fd, SOCK_STREAM, SO_REUSEPORT, &optval,
             sizeof(optval));

  // bind the socket to the server's address
  bind_success = bind(server_sock_fd, (struct sockaddr *)&server_addr,
                      sizeof(server_addr));
  check_error(bind_success, "[BIND]\t%d: %s\n", &should_run);

  // allocate space for messages
  char *buffer = malloc(MAXREQ);
  char *body = malloc(MAXREQ);
  char *header = malloc(MAXREQ);

  char *basic_header =
      "HTTP/1.1 200 OK\r\nContent-Type: "
      "text/html\r\nConnection: keep-alive\r\nCache-Control: "
      "max-age=0\r\nAccept-Encoding: gzip, deflate\r\nAccept-Language: "
      "en,en-US;q=0.9,nb;q=0.8,no;q=0.7\r\n\r\n";
  strcpy(header, basic_header);
  read_file("/home/sigbbe/hackerman/os_r_us/p2/418.html", body, MAXREQ);
  strcat(buffer, header);
  strcat(buffer, body);

  if (should_run) {
    // for info
    printf("listening on http://0.0.0.0:%d\n", PORT);
    // listen for incomming connections
    check_error(listen(server_sock_fd, 5), "[LISTEN]\t%d: %s\n", &should_run);
  }

  char *client_buffer = malloc(MAXREQ);
  char *path = malloc(32);
  char *query = malloc(32);

  while (should_run) {
    // request_counter += 1;
    // printf("Number of requests: %d\n", request_counter);

    client_addr_len = sizeof(client_addr);
    // accept a connection
    client_sock_fd = accept(server_sock_fd, (struct sockaddr *)&client_addr,
                            &client_addr_len);
    check_error(client_sock_fd, "[ACCEPT]\t%d: %s\n", &should_run);

    // read the request
    read_bit = read(client_sock_fd, client_buffer, MAXREQ);
    check_error(read_bit, "[READ]\t%d: %s\n", &should_run);
    should_run = 0;
    char *line = NULL;
    line = strtok(client_buffer, CRLF);
    parse(line, path, query);
    printf("PATH: %s\n", line);

    write_bit = write(client_sock_fd, buffer, strlen(buffer));
    check_error(write_bit, "[RESPONSE]\t%d: %s\n", &should_run);
    close(client_sock_fd);
  }

  close(server_sock_fd);

  free(client_buffer);
  free(buffer);
  free(body);
  free(header);
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
  return do_some();
}

/*
 *
 * Compiling the program:
 *
 * gcc -std=gnu99 -lpthread src/mtwwwd.c -o mtwwwd
 *
 */