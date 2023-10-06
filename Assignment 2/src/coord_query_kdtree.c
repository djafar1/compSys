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
  struct Point *coord;
  struct Node *left, *right;
  int axis;
  int depth;
};

int current_axis = 0;

int compare_points(const void *a, const void *b) {
    const struct Point *point1 = (const struct Point *)a;
    const struct Point *point2 = (const struct Point *)b;
    if (current_axis == 0) { // Sort by lon
        if (point1->lon < point2->lon) {
            return -1;
        } else if (point1->lon > point2->lon) {
            return 1;
        } else {
            return 0;
        }
    } else if (current_axis == 1) { // Sort by lat
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
  int axis = depth % 2;
  current_axis = axis;
  qsort(&points[start], end - start + 1, sizeof(struct Point), compare_points);
  int median_index = (start + end) / 2;
  struct Node* node = malloc(sizeof(struct Node));
  node->coord = &points[median_index];
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
  printf("Element %d of 20000, Depth %d, Axis %d: lon=%.2f, lat=%.2f\n", *count, root->depth, root->axis, root->coord->lon, root->coord->lat);
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
    free(data->coord);         // Free the points array
    free(data);                // Free the current node
}

double squared_distance(const struct Point *p1, const struct Point *p2) {
    double lon_diff = p1->lon - p2->lon;
    double lat_diff = p1->lat - p2->lat;
    return (lon_diff * lon_diff) + (lat_diff * lat_diff);
}




const struct Point* lookup_kdtree_recursive(struct Node* node, const struct Point* query, const struct Point* closest) {
    if (node == NULL) {
        return closest;
    }

    // Calculate the squared distance from the query point to the current node
    double dist = squared_distance(node->coord, query);

    // Check if closest is NULL or if the current node is closer than the current closest point
    if (closest == NULL || dist < squared_distance(closest, query)) {
        closest = node->coord;
    }

    // Choose the next axis to search based on the current node's axis
    int next_axis = (node->axis + 1) % 2;

    // Recursively search the appropriate subtree
    const struct Point* next_closest = NULL;
    if ((next_axis == 0 && query->lon < node->coord->lon) || (next_axis == 1 && query->lat < node->coord->lat)) {
        closest = lookup_kdtree_recursive(node->left, query, closest);
        if (query->lon - node->coord->lon <= sqrt(dist)) {
            next_closest = lookup_kdtree_recursive(node->right, query, closest);
        }
    } else {
        closest = lookup_kdtree_recursive(node->right, query, closest);
        if (node->coord->lon - query->lon <= sqrt(dist)) {
            next_closest = lookup_kdtree_recursive(node->left, query, closest);
        }
    }

    // Choose the closer point between the current closest and the closest from the other subtree
    double dist_next = squared_distance(next_closest, query);
    if (dist_next < squared_distance(closest, query)) {
        closest = next_closest;
    }

    return closest;
}





const struct record* lookup_kdtree(struct Node *root, double lon, double lat) {
    struct Point query;
    query.lon = lon;
    query.lat = lat;
    const struct Point *closest_point = NULL;
    closest_point = lookup_kdtree_recursive(root, &query, closest_point);
    return closest_point->rs; // Return the record associated with the closest point
}

int main(int argc, char** argv) {
  return coord_query_loop(argc, argv,
                          (mk_index_fn)mk_kdtree,
                          (free_index_fn)free_kdtree,
                          (lookup_fn)lookup_kdtree);
}
