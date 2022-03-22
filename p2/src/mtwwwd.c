
#include "../include/bbuffer.h"
#include "../include/sem.h"
#include "../include/server.h"

#define MAXREQ (4096 * 1024)
#define CRLF "\r\n"
#define READ_BYTES = "rb"

static BNDBUF *bb;
static char *www_path;
static char *FILE_404 = "./404.html";

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

void check_error(int code, char *format_str) {
  if (code < 0) {
    fprintf(stderr, format_str, errno, strerror(errno));
    exit(errno);
  }
}

void parse(const char *line, char *path) {
  /* Find out where everything is */
  char *start_of_path = strchr(line, ' ') + 1;
  char *end_of_path = strchr(start_of_path, '?');
  if (end_of_path == NULL) {
    end_of_path = strrchr(start_of_path, ' ');
  }
  if (start_of_path == NULL || end_of_path == NULL) {
    strncpy(path, FILE_404, 10);
    path[10] = '\0';
    return;
  }
  //   printf("start_of_path=%s, end_of_path=%s\n", start_of_path, end_of_path);
  /* Copy the strings into our memory */
  strncpy(path, start_of_path, end_of_path - start_of_path);
  //   printf("%s\n", "Finished strncpy");

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

void setup_server(const int port, const char *www_path, int *server_sock_fd,
                  struct sockaddr_in server_addr) {
  // TODO: find out why the server recieves a SIGSEGV when handling HTTPS
  // requests

  // TODO: check file premisions before returning content to client

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
  server_addr.sin_port = htons(port);

  // enable reuse of port
  int reuse_port = 1;
  setsockopt(*server_sock_fd, SOCK_STREAM, SO_REUSEPORT, &reuse_port,
             sizeof(reuse_port));

  // bind the socket to the server's address
  check_error(bind(*server_sock_fd, (struct sockaddr *)&server_addr,
                   sizeof(server_addr)),
              "[BIND]\t%d: %s\n");

  // for info
  printf("Serving files under %s on http://0.0.0.0:%d\n", www_path, port);
  // listen for incomming connections
  check_error(listen(*server_sock_fd, 5), "[LISTEN]\t%d: %s\n");
}

void *handle_req(void *);

void *handle_req(void *fd) {
  for (;;) {
    int client_sock_fd = bb_get(bb);
    // for checking write and read success
    int write_bit, read_bit, client_addr_len;

    // allocate space for messages
    char *body = malloc(MAXREQ);
    char *header = malloc(MAXREQ);

    // TODO: get the requested file's file-extension and use that to set
    // content-type
    char *basic_header =
        "HTTP/0.9 200 OK\r\nContent-Type: "
        "text/html\r\nConnection: keep-alive\r\nCache-Control: "
        "max-age=0\r\nAccept-Encoding: gzip, deflate\r\nAccept-Language: "
        "en,en-US;q=0.9,nb;q=0.8,no;q=0.7\r\n\r\n";
    strcpy(header, basic_header);

    char *client_buffer = malloc(MAXREQ);
    char *path = malloc(128);
    char *requested_path = malloc(128);

    // read the request
    read_bit = read(client_sock_fd, client_buffer, MAXREQ);
    check_error(read_bit, "[READ]\t%d: %s\n");

    // parse the requested path from the request
    char *line = NULL;
    line = strtok(client_buffer, CRLF);

    if (line != NULL) {
      parse(line, requested_path);
      requested_path = requested_path + 1;
    } else {
      *requested_path = *FILE_404;
    }

    sprintf(path, "%s/%s", www_path, requested_path);

    char *absolute_path = realpath(path, NULL);

    if (access(absolute_path, F_OK) != -1 && is_dir(absolute_path) == 0) {
      int in_web_root = strncmp(www_path, absolute_path, strlen(www_path));
      int has_read_permision = open(absolute_path, S_IROTH, O_CLOEXEC);
      printf("in_web_root=%d, has_read_permision=%d\n", in_web_root,
             has_read_permision);
      if (in_web_root == 0 && has_read_permision != -1) {
        read_file(absolute_path, body);
      } else {
        read_file(FILE_404, body);
      }
    } else {
      read_file(FILE_404, body);
    }

    // write the response
    write_bit = send(client_sock_fd, header, strlen(header), 0);
    check_error(write_bit, "[SEND HEADER]\t%d: %s\n");

    // close the connection
    write_bit = send(client_sock_fd, body, strlen(body), 0);
    check_error(write_bit, "[SEND BODY]\t%d: %s\n");
    close(client_sock_fd);

    free(client_buffer);
    free(body);
    free(header);
  }
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
  int port = 8000;
  int nthreads = 1;
  int bufferslots = 1;
  www_path = realpath(".", NULL);
  if (argc > 1) {
    www_path = realpath(argv[1], NULL);
  }
  if (argc > 2) {
    port = atoi(argv[2]);
    if (port < 1024) {
      fprintf(stderr, "Port number must be greater than 1024\n");
      exit(1);
    }
  }
  if (argc > 3) {
    nthreads = atoi(argv[3]);
  }
  if (argc > 4) {
    bufferslots = atoi(argv[4]);
  }
  pthread_t threads[nthreads];
  bb = bb_init(bufferslots);

  for (i = 0; i < nthreads; i++) {
    pthread_create(&threads[i], NULL, handle_req, NULL);
  }
  // file descriptors for server and client file descriptors
  int server_sock_fd, client_sock_fd, bind_success;

  // address of the server
  struct sockaddr_in server_addr, client_addr;
  // length of the address
  socklen_t client_addr_len;

  int int_port;

  // setup the server and listen for incomming connections
  setup_server(port, www_path, &server_sock_fd, server_addr);
  client_addr_len = sizeof(client_addr);

  for (;;) {
    // accept a connection
    client_sock_fd = accept(server_sock_fd, (struct sockaddr *)&client_addr,
                            &client_addr_len);
    check_error(client_sock_fd, "[ACCEPT]\t%d: %s\n");

    // add the client socket to the buffer and let some worker thread
    // respond to the request
    bb_add(bb, client_sock_fd);
  }
  // join all threads
  for (i = 0; i < nthreads; i++) {
    pthread_join(threads[i], NULL);
  }
  close(server_sock_fd);
  bb_del(bb);
  return 0;
}

/*
 *
 * Compiling the program:
 *
 * gcc -std=gnu99 -lpthread src/mtwwwd.c -o mtwwwd
 *
 */
