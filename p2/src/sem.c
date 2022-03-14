#include <stdio.h>
#define SEM_H
#include "../include/sem.h"
#include <malloc.h>
#include <pthread.h>
#include <stdlib.h>

/*
 * Opaque type of a semaphore.
 */
struct SEM {
    int count;
    pthread_cond_t cond;
    pthread_mutex_t mutex;
};

/*
 *
 * Creates a new semaphore.
 *
 * This function creates a new semaphore. If an error occurs during the
 * initialization, the implementation shall free all resources already
 * allocated so far.
 *
 * Parameters:
 *
 * initVal      the initial value of the semaphore
 *
 * Returns:
 *
 * handle for the created semaphore, or NULL if an error occured.
 */
struct SEM *sem_init(int initVal) {
  // allocate memory for the semaphore
  struct SEM *sem = malloc(sizeof(struct SEM));

  // define required members of the semaphore
  pthread_cond_t cond;
  pthread_condattr_t attr;
  pthread_mutex_t mutex;

  // initialize the lock, condition variable and the count
  pthread_mutex_init(&mutex, NULL);
  sem->count = initVal;
  pthread_cond_init(&cond, &attr);
  sem->cond = cond;

  return sem;
}

/*
 *
 * Destroys a semaphore and frees all associated resources.
 *
 * Parameters:
 *
 * sem           handle of the semaphore to destroy
 *
 * Returns:
 *
 * 0 on success, negative value on error.
 *
 * In case of an error, not all resources may have been freed, but
 * nevertheless the semaphore handle must not be used any more.
 */
int sem_del(struct SEM *sem) {
  int i, j;
  i = pthread_mutex_destroy(&sem->mutex);
  j = pthread_cond_destroy(&sem->cond);
  free(sem);
  return (i || j) * -1;
}

/*
 *
 * P (wait) operation.
 *
 * Attempts to decrement the semaphore value by 1.
 * If the semaphore value is 0, the operation blocks
 * until a V operation increments the value and the P
 * operation succeeds.
 *
 * Parameters:
 *
 * sem           handle of the semaphore to decrement
 */
void P(struct SEM *sem) {
  pthread_mutex_lock(&sem->mutex);
  while (sem->count == 0) {
    pthread_cond_wait(&sem->cond, &sem->mutex);
  }
  sem->count--;
  pthread_mutex_unlock(&sem->mutex);
}

/*
 *
 * V (signal) operation.
 *
 * Increments the semaphore value by 1 and notifies P operations that are
 * blocked on the semaphore of the change.
 *
 * Parameters:
 *
 * sem           handle of the semaphore to increment
 */
void V(struct SEM *sem) {
  pthread_mutex_lock(&sem->mutex);
  sem->count++;
  pthread_mutex_unlock(&sem->mutex);
  pthread_cond_signal(&sem->cond);
}