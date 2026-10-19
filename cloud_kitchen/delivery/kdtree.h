/* kdtree.h - 2-D KD-TREE of delivery partners, keyed by (lat, lon).
 * Even depth splits on lat, odd depth splits on lon.
 * delivery.c keeps one root pointer and uses it like this:
 *
 *   root = kd_insert(root, location, staff_id);      // new partner
 *   int sid = kd_nearest_free(root, target);         // nearest NOT-busy partner, -1 if none
 *   kd_set_busy(root, sid, 1);                       // on a delivery (0 = free again)
 *
 * Nearest search hint: go to the near side first; visit the far side only if the
 * distance to the splitting line is smaller than the best distance found so far.
 * Skip busy nodes when choosing "best", but still search below them.            */
#ifndef KDTREE_H
#define KDTREE_H
#include "../common.h"

typedef struct KDNode {
    GeoPoint point; int staff_id; int busy;
    struct KDNode *left, *right;
} KDNode;

KDNode *kd_insert(KDNode *root, GeoPoint p, int staff_id);
int     kd_nearest_free(const KDNode *root, GeoPoint target);
int     kd_set_busy(KDNode *root, int staff_id, int busy);
#endif
