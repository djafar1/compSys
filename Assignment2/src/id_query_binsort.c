#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <stdint.h>
#include <errno.h>
#include <assert.h>

#include "record.h"
#include "id_query.h"

struct binsort_record{
    int64_t osm_id;
    const struct record *record;    
};


struct binsort_data {
  struct binsort_record *irs;
  int n;
};

int sort_by_osm_id (const void *a, const void *b) {
  const struct binsort_record* record1 = (const struct binsort_record*)a;
  const struct binsort_record* record2 = (const struct binsort_record*)b;
  if (record1->osm_id < record2->osm_id) {
      return -1;
   } else if (record1->osm_id > record2->osm_id) {
       return 1;
   } else {
      return 0;
  }
}

struct binsort_data* mk_binsort(struct record* irs, int n) {
  struct binsort_data* data = malloc(sizeof(struct binsort_data));
  if (data == NULL) {
    fprintf(stderr, "Malloc failed for data\n");
    exit(EXIT_FAILURE);
  }
  data->irs = malloc(sizeof(struct binsort_record)*n);
  if (data->irs == NULL) {
    fprintf(stderr, "Malloc failed for irs\n");
    free(data); // freeing data since this didn't fail being malloced.
    exit(EXIT_FAILURE);
  }
  for (int i = 0; i < n; i++){
    data->irs[i].osm_id = irs[i].osm_id;
    data->irs[i].record = &irs[i];
  }
  data->n = n;
  qsort(data->irs, n, sizeof(struct binsort_record), sort_by_osm_id);
  return data;
}

void free_binsort(struct binsort_data* data) {
  free(data->irs);
  free(data);
}


const struct record* lookup_binsort(struct binsort_data *data, int64_t needle) {
  int64_t left = 0;
  int64_t right = data->n -1;
  int64_t middle = 0;
  while (left <= right){
    middle = left + (right - left) / 2;
    if (data->irs[middle].osm_id == needle){
      return data->irs[middle].record;
    }
    if (data->irs[middle].osm_id < needle){
      left = middle + 1;
    }
    else{
      right = middle - 1;
    }
  }
  return NULL;
}

int main(int argc, char** argv) {
  return id_query_loop(argc, argv,
                    (mk_index_fn)mk_binsort,
                    (free_index_fn)free_binsort,
                    (lookup_fn)lookup_binsort);
}
