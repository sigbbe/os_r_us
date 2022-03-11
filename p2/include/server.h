#ifndef SERVER
#define SERVER
/*
 * Encapsulates the properties of the server.
 */
typedef struct server {
    // file descriptor of the socket in passive
    // mode to wait for connections.
    int listen_fd;
} server_t;

/*
 * Creates a socket for the server and makes it passive such that
 * we can wait for connections on it later.
 */
int server_listen(server_t *server);

/*
 * Accepts new connections and then prints `Hello World` to
 * them.
 */
int server_accept(server_t *server);

#endif