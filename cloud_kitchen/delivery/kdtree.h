#ifndef KDTREE_H
#define KDTREE_H
#include "../common.h"

typedef struct KDNode {
    GeoPoint point; int staff_id; int busy;
    struct KDNode *left, *right;
} KDNode;

KDNode *kd_insert(KDNode *root, GeoPoint p, int staff_id);
int kd_nearest_free(const KDNode *root, GeoPoint target);
int kd_set_busy(KDNode *root, int staff_id, int busy);

KDNode *kd_remove(KDNode *root, int staff_id);
int kd_get(const KDNode *root, int staff_id, GeoPoint *out_point, int *out_busy);
void kd_free(KDNode *root);
#endif
