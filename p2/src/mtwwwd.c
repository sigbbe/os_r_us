
#include "../include/bbuffer.h"
#include "../include/sem.h"
#include "../include/server.h"

#define MAXREQ (4096 * 1024)
#define CRLF "\r\n"

#define _REENTRANT
#define _POSIX_PTHREAD_SEMANTICS
#include <assert.h>
#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <limits.h>
#include <malloc.h>
#include <netdb.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/stat.h>
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

void check_error(int code, char *format_str) {
  if (code < 0) {
    fprintf(stderr, format_str, errno, strerror(errno));
    exit(errno);
  }
}

void parse(const char *line, char *path) {
  /* Find out where everything is */
  char *start_of_path = strchr(line, ' ') + 1;
  char *end_of_path;
  if (strchr(start_of_path, '?') == NULL) {
    end_of_path = strrchr(start_of_path, ' ');
  } else {
    end_of_path = strchr(start_of_path, '?');
  }
  /* Copy the strings into our memory */
  strncpy(path, start_of_path, end_of_path - start_of_path);

  /* Null terminators (because strncpy does not provide them) */
  path[end_of_path - start_of_path] = 0;
}

void ip_int_to_char(int ip, char *ip_char) {
  sprintf(ip_char, "%d.%d.%d.%d", (ip >> 24) & 0xFF, (ip >> 16) & 0xFF,
          (ip >> 8) & 0xFF, ip & 0xFF);
}

int is_dir(const char *path) {
  struct stat statbuf;
  if (stat(path, &statbuf) != 0)
    return 0;
  return S_ISDIR(statbuf.st_mode);
}

void read_file(char *path, char buf[]) {
  FILE *fp = fopen(path, "rb");
  fseek(fp, 0, SEEK_END);
  long fsize = ftell(fp);
  fseek(fp, 0, SEEK_SET); /* same as rewind(f); */

  fread(buf, fsize, 1, fp);
  fclose(fp);
  buf[fsize] = 0;
}

void setup_server(const char *port, int int_port, const char *www_path,
                  int *server_sock_fd, struct sockaddr_in server_addr) {
  int_port = atoi(port);

  // create a socket
  *server_sock_fd = socket(AF_INET, SOCK_STREAM, 0);
  check_error(*server_sock_fd, "[SOCKET]\t%d: %s\n");

  // why do we need to set server address to 0?
  bzero((char *)&server_addr, sizeof(server_addr));
  // IP protocol family
  server_addr.sin_family = AF_INET;
  // allow any address to connect
  server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
  // set port number
  server_addr.sin_port = htons(int_port);

  // enable reuse of port
  int reuse_port = 1;
  setsockopt(*server_sock_fd, SOCK_STREAM, SO_REUSEPORT, &reuse_port,
             sizeof(reuse_port));

  // bind the socket to the server's address
  check_error(bind(*server_sock_fd, (struct sockaddr *)&server_addr,
                   sizeof(server_addr)),
              "[BIND]\t%d: %s\n");

  // for info
  printf("Serving files under %s on http://0.0.0.0:%d\n", www_path, int_port);
  // listen for incomming connections
  check_error(listen(*server_sock_fd, 5), "[LISTEN]\t%d: %s\n");
}

int process_request(const char *www_path, const char *port) {
  // The argument sockfd is a socket that has been created with socket(2), bound
  // to a local address with bind(2), and is listening for connections after a
  // listen(2).

  // file descriptors for server and client file descriptors
  int server_sock_fd, client_sock_fd, bind_success;
  // address of the server
  struct sockaddr_in server_addr, client_addr;
  // length of the address
  socklen_t client_addr_len;

  int int_port;

  setup_server(port, int_port, www_path, &server_sock_fd, server_addr);

  // for checking write and read success
  int write_bit, read_bit;

  // allocate space for messages
  char *body = malloc(MAXREQ);
  char *header = malloc(MAXREQ);

  char *basic_header =
      "HTTP/1.1 200 OK\r\nContent-Type: "
      "text/html\r\nConnection: keep-alive\r\nCache-Control: "
      "max-age=0\r\nAccept-Encoding: gzip, deflate\r\nAccept-Language: "
      "en,en-US;q=0.9,nb;q=0.8,no;q=0.7\r\n\r\n";
  strcpy(header, basic_header);

  char *client_buffer = malloc(MAXREQ);
  char *path = malloc(128);
  char *requested_path = malloc(128);

  for (;;) {
    client_addr_len = sizeof(client_addr);
    // accept a connection
    client_sock_fd = accept(server_sock_fd, (struct sockaddr *)&client_addr,
                            &client_addr_len);
    check_error(client_sock_fd, "[ACCEPT]\t%d: %s\n");

    // read the request
    read_bit = read(client_sock_fd, client_buffer, MAXREQ);
    check_error(read_bit, "[READ]\t%d: %s\n");

    // parse the requested path from the request
    char *line = NULL;
    line = strtok(client_buffer, CRLF);
    parse(line, requested_path);
    requested_path = requested_path + 1;
    sprintf(path, "%s/%s", www_path, requested_path);

    printf("%s\n", path);
    //   if the requested path is a valid file read it
    if (access(path, F_OK) != -1 && is_dir(path) == 0) {
      read_file(path, body);
    } else {
      read_file("./404.html", body);
    }

    // write the response
    write_bit = send(client_sock_fd, header, strlen(header), 0);
    check_error(write_bit, "[SEND HEADER]\t%d: %s\n");

    // close the connection
    write_bit = send(client_sock_fd, body, strlen(body), 0);
    check_error(write_bit, "[SEND BODY]\t%d: %s\n");
    close(client_sock_fd);
  }

  close(server_sock_fd);

  free(client_buffer);
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
  char *www_path = realpath(".", NULL);
  char *port = "8000";
  char *threads = "1";
  char *bufferslots = "1";
  char *tmp;
  struct BNDBUF *bb;
  if (argc > 1) {
    www_path = realpath(argv[1], NULL);
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
  return process_request(www_path, port);
}

/*
 *
 * Compiling the program:
 *
 * gcc -std=gnu99 -lpthread src/mtwwwd.c -o mtwwwd
 *
 */