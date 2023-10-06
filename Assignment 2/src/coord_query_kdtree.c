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


void print_kdtree(struct Node* root, int* count) {
  if (root == NULL) {
      return;
  }
  print_kdtree(root->left, count);
  (*count)++;
  printf("Element %d of 20000, Depth %d, Axis %d: lon=%.2f, lat=%.2f\n", *count, root->depth, root->axis, root->coord->lon, root->coord->lat);
  print_kdtree(root->right, count);
}


struct Node* mk_kdtree(struct record* rs, int n) {
  struct Point* points = malloc(n * sizeof(struct Point));
  for (int i  = 0; i < n; i++)  {
    points[i].lon = rs[i].lon;
    points[i].lat = rs[i].lat;
    points[i].rs = &rs[i];
  }
  struct Node* tree = kdtree(points, 0, 0, n);
  if (tree != NULL) {
    int count = 0;
    printf("KD-tree:\n");
    print_kdtree(tree, &count);
  }
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

#include <float.h> // For DBL_MAX constant

// WE HAVE TO DELETE THIS LATER CAUSE THIS IS NOT MINE
const struct record* lookup_kdtree(struct Node *root, double lon, double lat) {
    if (root == NULL) {
        return NULL;
    }

    const struct Point* closest_point = NULL;
    double closest_distance = DBL_MAX; // Initialize to a large value

    // Start with the root node
    struct Node* current_node = root;

    while (current_node != NULL) {
        // Calculate the distance between the query point and the current node's coordinates
        double distance = hypot(current_node->coord->lon - lon, current_node->coord->lat - lat);

        // If this node is closer, update the closest_point and closest_distance
        if (distance < closest_distance) {
            closest_distance = distance;
            closest_point = current_node->coord;
        }

        // Determine which child node to explore next
        int axis = current_node->axis;
        double node_value = (axis == 0) ? current_node->coord->lon : current_node->coord->lat;
        double query_value = (axis == 0) ? lon : lat;

        if (query_value < node_value) {
            current_node = current_node->left;
        } else {
            current_node = current_node->right;
        }
    }

    if (closest_point != NULL) {
        return closest_point->rs; // Return the record associated with the closest point
    } else {
        return NULL; // No closest point found
    }
}


int main(int argc, char** argv) {
  return coord_query_loop(argc, argv,
                          (mk_index_fn)mk_kdtree,
                          (free_index_fn)free_kdtree,
                          (lookup_fn)lookup_kdtree);
}
