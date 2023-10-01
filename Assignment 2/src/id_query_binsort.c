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
  struct index_record *irs;
  int n;
};

struct binsort_data* mk_binsort(struct record* irs, int n) {
  struct binsort_data* data = malloc(sizeof(struct binsort_data));
  data->n = n;
  data->irs = irs;
  return data;
};

void free_binsort(struct binsort_data* data) {
  free(data);
};

const struct record* lookup_binsort(struct binsort_data *data, int64_t needle) {
    assert(0);
};

int main(int argc, char** argv) {
  return id_query_loop(argc, argv,
                    (mk_index_fn)mk_binsort,
                    (free_index_fn)free_binsort,
                    (lookup_fn)lookup_binsort);
};
