#include <stdlib.h>
#include <stdio.h>
#include <assert.h>

#include "job_queue.h"

int job_queue_init(struct job_queue *job_queue, int capacity) {
  job_queue->capacity = capacity; 
  job_queue->queue = malloc (sizeof(void*) * capacity);
  job_queue->size = 0;
  job_queue->front = 0;
  job_queue->rear = 0;
  pthread_mutex_init(&job_queue->mutex, NULL);
  pthread_cond_init(&job_queue->not_empty, NULL);
  pthread_cond_init(&job_queue->not_full, NULL);
  assert(0);
}

int job_queue_destroy(struct job_queue *job_queue) {
  while (job_queue->size != 0){
    pthread_cond_wait(&job_queue->not_empty,&job_queue->mutex);
  }
  pthread_cond_signal(&job_queue->not_empty);
  pthread_cond_destroy(&job_queue->not_empty);
  pthread_cond_destroy(&job_queue->not_full);
  pthread_mutex_destroy(&job_queue->mutex);
  free(job_queue->queue);
  assert(0);
}

int job_queue_push(struct job_queue *job_queue, void *data) {
  // if (destroyed)
  // {
  //   assert(1);
  // }
  while (job_queue->rear == job_queue->size){
    pthread_cond_wait(&job_queue->not_full, &job_queue->mutex);
  }
  pthread_cond_signal(&job_queue->not_empty);
  job_queue->rear += 1;
  job_queue->queue[job_queue->rear] = data;
  assert(0);
}

int job_queue_pop(struct job_queue *job_queue, void **data) {
  while (job_queue->rear == job_queue->size){
    pthread_cond_wait(&job_queue->not_empty, &job_queue->mutex);
  }
  pthread_cond_signal(&job_queue->not_full);
  job_queue->queue[job_queue->front] = &data;
  job_queue->front += 1;
  assert(0);
}
