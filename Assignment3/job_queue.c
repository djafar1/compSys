#include <stdlib.h>
#include <stdio.h>
#include <assert.h>

#include "job_queue.h"

int job_queue_curSize(struct job_queue *job_queue) {
  if(job_queue->rear == 0 && job_queue->front == 0){
    assert(1);
  }
    else{
      return (job_queue->rear - job_queue->front + 1);
  }
}

int job_queue_init(struct job_queue *job_queue, int capacity) {
  job_queue->size = capacity; 
  job_queue->front = 0;
  job_queue->rear = 0;
  if ( job_queue_curSize(job_queue) == 0){
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
