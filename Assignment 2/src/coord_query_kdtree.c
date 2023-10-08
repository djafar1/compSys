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
#define INITIAL_DEPTH 0

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
        }
        return 0;
    } else if (axis == 1) { // Sort by lat
        if (point1->lat < point2->lat) {
            return -1;
        } else if (point1->lat > point2->lat) {
            return 1;
        }
        return 0;
    }
    return 0;
}

struct Node* kdtree(struct Point* points, int n, int depth){
  if (n <= 0) {
    return NULL;
  }
  axis = depth % 2;
  qsort(points, n, sizeof(struct Point), &compare_points);
  int median_index = ceil(n / 2);
  struct Node* node = malloc(sizeof(struct Node));

  node->point = (points + median_index);
  node->depth = depth; // Pass depth information just to print it
  node->axis = axis;
  node->left = kdtree(points, median_index, depth + 1); //start til midt
  node->right = kdtree((points + median_index + 1), n - median_index - 1, depth + 1); //midt til slut
  return node;
}

struct Node* mk_kdtree(struct record* rs, int n) {
  struct Point* points = malloc(n * sizeof(struct Point));
  for (int i  = 0; i < n; i++)  {
    points[i].lon = rs[i].lon;
    points[i].lat = rs[i].lat;
    points[i].rs = &rs[i];
  }
  struct Node* tree = kdtree(points, n, INITIAL_DEPTH);
  return tree;
}

void free_kdtree(struct Node* node) {
  if (node == NULL) {
    return;
  }
  free_kdtree(node->left);   // Recursively free the left subtree
  free_kdtree(node->right);  // Recursively free the right subtree
  free(node->point);         // Free the points array
  free(node);                // Free the current node
}

double calculate_distance(struct Point *p1, struct Point *p2){
  return sqrt((pow((p2->lon - p1->lon), 2))+ (pow((p2->lat - p1->lat), 2)));
}

// Procedure lookup(closest, query, node)
struct Point* lookup(struct Point *closest, struct Point *query, struct Node *node){
  // if node is NULL then
  if (node == NULL){
    return closest;
  }
  
  // else if node.point is closer to query than closest then
  if (calculate_distance(query, node->point) < calculate_distance(query, closest)){
    // replace closest with node.point;
    closest = node->point;
  }

  double x = axis == 0 ? node->point->lon : node->point->lat; // node.point[node.axis]
  double y = axis == 0 ? query->lon : query->lat;             // query [node.axis]

  double diff = x - y; // node.point[node.axis] -  query [node.axis]
  // radius ← the distance between query and closest
  double radius = calculate_distance(query, closest);
 
 
  // if diff ≥ 0 ∨ radius > |diff| then
  if (diff >= 0 || radius > fabs(diff)){
    // lookup (closest, query, node.left)
    closest = lookup(closest, query, node->left);
  }
  // if diff ≤ 0 ∨ radius > |diff| then
  if (diff <= 0 || radius > fabs(diff)){
    closest = lookup(closest, query, node->right);
  }
  return closest;
}

const struct record* lookup_kdtree(struct Node *root, double lon, double lat) {
  struct Point *query = malloc(sizeof(struct Point));
  query->lon = lon;
  query->lat = lat;
  struct record *record = lookup(root->point, query, root)->rs;
  free(query);
  return record;
}

int main(int argc, char** argv) {
  return coord_query_loop(argc, argv,
                          (mk_index_fn)mk_kdtree,
                          (free_index_fn)free_kdtree,
                          (lookup_fn)lookup_kdtree);
}
