#include <stdlib.h>
#include <stdio.h>
#include <assert.h>

#include "job_queue.h"

int job_queue_init(struct job_queue *job_queue, int capacity) {
  job_queue->capacity = capacity; 
  job_queue->queue = malloc (capacity * sizeof(void*));
  job_queue->size = 0;
  job_queue->front = 0;
  job_queue->rear = 0;

}

int job_queue_destroy(struct job_queue *job_queue) {
  assert(0);
}

int job_queue_push(struct job_queue *job_queue, void *data) {
  if (job_queue_curSize(job_queue) == job_queue->size)
  {
    // pause input
    assert(1);
  }
  else
  {
    job_queue->front =+ 1;
    // insert data?;
    assert(0);
  }
}

int job_queue_pop(struct job_queue *job_queue, void **data) {
  assert(0);
}
