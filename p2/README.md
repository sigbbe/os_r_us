# Multithreaded webserver

UID PID PPID LWP C NLWP STIME TTY TIME CMD

sigbbe 35444 35430 35444 0 5 07:59 pts/1 00:00:00 ./mtwwwd /home/sigbbe/ 8000 4 6
sigbbe 35444 35430 35445 0 5 07:59 pts/1 00:00:00 ./mtwwwd /home/sigbbe/ 8000 4 6
sigbbe 35444 35430 35446 0 5 07:59 pts/1 00:00:00 ./mtwwwd /home/sigbbe/ 8000 4 6
sigbbe 35444 35430 35447 0 5 07:59 pts/1 00:00:00 ./mtwwwd /home/sigbbe/ 8000 4 6
sigbbe 35444 35430 35448 0 5 07:59 pts/1 00:00:00 ./mtwwwd /home/sigbbe/ 8000 4 6

# Invoking the program

``` ./make ```

## a) 

## b) Counting semaphores

The counting semaphore struct is defined by:

```
struct SEM {
    int count;
    pthread_cond_t cond;
    pthread_mutex_t mutex;
}
```

The Semaphore, SEM, structure has three attribute members.
(1) the `int count` fields to keep track of the state, if the count is less than or equal to 0, the P (wait) method will block until the V (release) method is called. (2) the `pthread_cond_t cond` field is used for calling the `pthread_cond_wait` method. (3) the `pthread_mutex_t mutex` 

## c)



## d)


## e) 
# Directory traversal exploit
As the server is hosted locally it was not allowed to use "../" commands in the url. We had to use telnet to send request to the server to exploit the Directory traversal. By sending requests this way you could climb up the directory tree, allowing arbitrary file access in the server file system.

# Implentations to prevent directory traversal
We implemented validation for the web root server path. Web root server path is required to be the start of the requested url, or else you will not have access.

Another validation we implemented was to check the public read permission on requested files. If this was allowed you will recieve access, or else you are denied. We only check for public read permission were we could implement owner and group permissions as well, this was not necessary in this exercise.
