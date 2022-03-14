/*
 * Ring buffer used for communicating between a
 * single producer and multiple consumer threads
 *
 * https://en.wikipedia.org/wiki/Circular_buffer
 *
 */

#include "../include/bbuffer.h"
#include "../include/sem.h"
#include <stdio.h>
#include <stdlib.h>

/*
 *
 * struct fields:
 *
 * int size 			number of slots in the buffer
 * int in 				index of the next slot to be written
 * int out 				index of the next slot to be read
 * int count 			number of elements in the buffer
 * int *buffer 			array of slots
 * struct SEM *sem_empty 	semaphore for empty slots
 * struct SEM *sem_full 	semaphore for full slots
 *
 */
struct BNDBUF {
    int size;
    int in;
    int out;
    int count;
    int *buffer;
    struct SEM *sem_empty;
    struct SEM *sem_full;
};

/*
 *
 * Creates a new Bounded Buffer.
 *
 * This function creates a new bounded buffer and all the helper data
 * structures required by the buffer, including semaphores for
 * synchronization. If an error occurs during the initialization the
 * implementation shall free all resources already allocated by then.
 *
 * Parameters:
 *
 * size     The number of integers that can be stored in the bounded buffer.
 *
 * Returns:
 *
 * handle for the created bounded buffer, or NULL if an error occured.
 */

struct BNDBUF *bb_init(unsigned int size) {
  struct BNDBUF *bb = malloc(sizeof(struct BNDBUF));
  bb->size = size;
  bb->in = 0;
  bb->out = 0;
  bb->count = 0;
  bb->buffer = malloc(sizeof(int) * size);
  bb->sem_empty = sem_init(0);
  bb->sem_full = sem_init(size);
  return bb;
}

/*
 *
 * Destroys a Bounded Buffer.
 *
 * All resources associated with the bounded buffer are released.
 *
 * Parameters:
 *
 * bb       Handle of the bounded buffer that shall be freed.
 */

void bb_del(struct BNDBUF *bb) {
  free(bb->buffer);
  sem_del(bb->sem_empty);
  sem_del(bb->sem_full);
  free(bb);
}

/*
 *
 * Retrieve an element from the bounded buffer.
 *
 * This function removes an element from the bounded buffer. If the bounded
 * buffer is empty, the function blocks until an element is added to the
 * buffer.
 *
 * Parameters:
 *
 * bb         Handle of the bounded buffer.
 *
 * Returns:
 *
 * the int element
 */

int bb_get(struct BNDBUF *bb) {
  int element;
  P(bb->sem_empty);
  element = bb->buffer[bb->out];
  bb->out = (bb->out + 1) % bb->size;
  bb->count--;
  V(bb->sem_full);
  return element;
}

/*
 *
 * Add an element to the bounded buffer.
 *
 * This function adds an element to the bounded buffer. If the bounded
 * buffer is full, the function blocks until an element is removed from
 * the buffer.
 *
 * Parameters:
 *
 * bb     Handle of the bounded buffer.
 * fd     Value that shall be added to the buffer.
 *
 * Returns:
 *
 * the int element
 */
void bb_add(struct BNDBUF *bb, int fd) {
  P(bb->sem_full);
  bb->buffer[bb->in] = fd;
  bb->in = (bb->in + 1) % bb->size;
  bb->count++;
  V(bb->sem_empty);
}