#include <stdlib.h>
#include <stdio.h>
#include <assert.h>

#include "job_queue.h"

int job_queue_init(struct job_queue *job_queue, int capacity) {
  job_queue->destroy = false;
  job_queue->capacity = capacity; 
  job_queue->queue = malloc (sizeof(void*) * capacity);
  // Malloc failure handle
  if (job_queue->queue == NULL) {
    fprintf(stderr, "Malloc failed for queue\n");
    exit(EXIT_FAILURE);
  }
  
  job_queue->size = 0;
  job_queue->front = 0;
  job_queue->rear = 0;
  pthread_mutex_init(&job_queue->mutex, NULL);
  pthread_cond_init(&job_queue->not_empty, NULL);
  pthread_cond_init(&job_queue->not_full, NULL);
  return 0;
}

int job_queue_destroy(struct job_queue *job_queue) {
  pthread_mutex_lock(&job_queue->mutex);
  job_queue->destroy = true;
  pthread_cond_broadcast(&job_queue->not_empty);
  while (job_queue->size > 0) {
    pthread_cond_wait(&job_queue->not_empty, &job_queue->mutex);
  }
  pthread_mutex_unlock(&job_queue->mutex);
  pthread_mutex_destroy(&job_queue->mutex);
  pthread_cond_destroy(&job_queue->not_empty);
  pthread_cond_destroy(&job_queue->not_full);
  free(job_queue->queue);
  return 0;
}

int job_queue_push(struct job_queue *job_queue, void *data) {
  pthread_mutex_lock(&job_queue->mutex);
  while ((job_queue->rear + 1) % job_queue->capacity == job_queue->front) {
    pthread_cond_wait(&job_queue->not_full, &job_queue->mutex);
  }
  job_queue->queue[job_queue->rear] = data;
  job_queue->size += 1;
  job_queue->rear = (job_queue->rear + 1) % job_queue->capacity;
  pthread_cond_signal(&job_queue->not_empty);
  pthread_mutex_unlock(&job_queue->mutex);
  return 0;
}

int job_queue_pop(struct job_queue *job_queue, void **data) {
  pthread_mutex_lock(&job_queue->mutex);
  while (job_queue->front == job_queue->rear && !job_queue->destroy) {
    pthread_cond_wait(&job_queue->not_empty, &job_queue->mutex);
  }
  if (job_queue->destroy && job_queue->size == 0) {
    pthread_cond_signal(&job_queue->not_empty);
    pthread_mutex_unlock(&job_queue->mutex);
    return -1;
  }
  *data = job_queue->queue[job_queue->front];
  job_queue->size -= 1;
  job_queue->front = (job_queue->front + 1) % job_queue->capacity;
  pthread_cond_signal(&job_queue->not_full);
  pthread_mutex_unlock(&job_queue->mutex);
  return 0;
}
