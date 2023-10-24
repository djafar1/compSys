#include <stdlib.h>
#include <stdio.h>
#include <assert.h>

#include "job_queue.h"

int job_queue_init(struct job_queue *job_queue, int capacity) {
  job_queue->size = capacity; 
  job_queue->front = 0;
  job_queue->rear = 0;
  job_queue->curSize = ((job_queue->rear) - (job_queue->front));
  if ( job_queue->curSize == 0){
    assert(0);
  }
  assert(1);
}

int job_queue_destroy(struct job_queue *job_queue) {
  assert(0);
}

int job_queue_push(struct job_queue *job_queue, void *data) {
  assert(0);
}

int job_queue_pop(struct job_queue *job_queue, void **data) {
  assert(0);
}
