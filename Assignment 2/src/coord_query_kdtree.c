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
  struct record *rs;
};

struct Node {
  struct Point *point;
  struct Node *left, *right;
  int axis;
  int depth;
};

int axis = 0;

int compare_points(const void *a, const void *b) {
    const struct Point *point1 = (const struct Point *)a;
    const struct Point *point2 = (const struct Point *)b;
    if (axis == 0) { // Sort by lon
        if (point1->lon < point2->lon) {
            return -1;
        } else if (point1->lon > point2->lon) {
            return 1;
        } else {
            return 0;
        }
    } else if (axis == 1) { // Sort by lat
        if (point1->lat < point2->lat) {
            return -1;
        } else if (point1->lat > point2->lat) {
            return 1;
        } else {
            return 0;
        }
    } else {
        return 0;
    }
}

struct Node* kdtree(struct Point* points, int depth, int start, int end){
  if (start > end) {
    return NULL;
  }
  axis = depth % 2;
  qsort(&points[start], end - start + 1, sizeof(struct Point), compare_points);
  int median_index = (start + end) / 2;
  struct Node* node = malloc(sizeof(struct Node));
  node->point = &points[median_index];
  node->depth = depth; // Pass depth information just to print it
  node->axis = axis;
  node->left = kdtree(points, depth + 1, start, median_index - 1);
  node->right = kdtree(points, depth + 1, median_index + 1, end);
  return node;
}

/*
void print_kdtree(struct Node* root, int* count) {
  if (root == NULL) {
      return;
  }
  print_kdtree(root->left, count);
  (*count)++;
  printf("Element %d of 20000, Depth: %d, Axis %d: lon=%.2f, lat=%.2f\n", *count, root->depth, root->axis, root->point->lon, root->point->lat);
  print_kdtree(root->right, count);
}*/


struct Node* mk_kdtree(struct record* rs, int n) {
  struct Point* points = malloc(n * sizeof(struct Point));
  for (int i  = 0; i < n; i++)  {
    points[i].lon = rs[i].lon;
    points[i].lat = rs[i].lat;
    points[i].rs = &rs[i];
  }
  struct Node* tree = kdtree(points, 0, 0, n);
  /*if (tree != NULL) {
    int count = 0;
    printf("KD-tree:\n");
    print_kdtree(tree, &count);
  }*/
  return tree;
}

void free_kdtree(struct Node* data) {
  if (data == NULL) {
    return;
  }
  free_kdtree(data->left);   // Recursively free the left subtree
  free_kdtree(data->right);  // Recursively free the right subtree
  free(data->point);         // Free the points array
  free(data);                // Free the current node
}

const struct record* lookup_recursive(struct Point *query, struct Node *node){
  
}

const struct record* lookup_kdtree(struct Node *root, double lon, double lat) {
  struct Point* query = malloc(sizeof(struct Point));
  query->lon = lon;
  query->lat = lat;
  const struct record* record = lookup_recursive(query, root);
  free(query);
  return record;
}


int main(int argc, char** argv) {
  return coord_query_loop(argc, argv,
                          (mk_index_fn)mk_kdtree,
                          (free_index_fn)free_kdtree,
                          (lookup_fn)lookup_kdtree);
}
