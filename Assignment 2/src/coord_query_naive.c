#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <stdint.h>
#include <errno.h>
#include <assert.h>
#include <math.h>  

#include "record.h"
#include "coord_query.h"

struct naive_data {
  struct record *rs;
  int n;
};

struct naive_data* mk_naive(struct record* rs, int n) {
  struct naive_data* data = malloc(sizeof(struct naive_data));
  data->n = n;
  data->rs = rs;
  return data;
}

void free_naive(struct naive_data* data) {
  free(data);
}

double calculate_distance (double lon, double lat, double lon2, double lat2){
  return sqrt(pow((lon2 - lon), 2) + pow((lat2 - lat), 2));
}

const struct record* lookup_naive(struct naive_data *data, double lon, double lat) {
  double distance = 10000000;
  const struct record* current_closest = NULL;
  for (int i = 0; i < (data->n); i++){
    if (calculate_distance(lon, lat, data->rs[i].lon, data->rs[i].lat) < distance){
      distance = calculate_distance(lon, lat, data->rs[i].lon, data->rs[i].lat);
      current_closest = &(data->rs[i]);
    }
  } 
  return current_closest;
}

int main(int argc, char** argv) {
  return coord_query_loop(argc, argv,
                          (mk_index_fn)mk_naive,
                          (free_index_fn)free_naive,
                          (lookup_fn)lookup_naive);
}
