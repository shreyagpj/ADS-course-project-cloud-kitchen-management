#ifndef KDTREE_H
#define KDTREE_H
#include "../common.h"

typedef struct KDNode {
    GeoPoint point; int staff_id; int busy;
    struct KDNode *left, *right;
} KDNode;

KDNode *kd_insert(KDNode *root, GeoPoint p, int sid);
int kd_nearest_free(const KDNode *root, GeoPoint target);
int kd_set_busy(KDNode *root, int sid, int busy);

KDNode *kd_remove(KDNode *root, int sid);
int kd_get(const KDNode *root, int sid, GeoPoint *op, int *ob);
void kd_free(KDNode *root);
#endif
