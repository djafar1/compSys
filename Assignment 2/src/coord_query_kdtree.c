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

struct Point {
  double lon;
  double lat;
};

struct Node {
  struct Point coord;
  struct Node *left, *right;
};

struct kd_tree {
  struct Node root;
  struct record *rs;
  int n;
};


struct kd_tree* mk_kdtree(struct record* rs, int n) {
  
  assert(0);
}

void free_kdtree(struct kd_tree* data) {
  free(data);
}

const struct record* lookup_kdtree(struct naive_data *data, double lon, double lat) {
  assert(0);
}

int main(int argc, char** argv) {
  return coord_query_loop(argc, argv,
                          (mk_index_fn)mk_kdtree,
                          (free_index_fn)free_kdtree,
                          (lookup_fn)lookup_kdtree);
}
