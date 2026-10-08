#include "kdtree.h"
KDNode *kd_insert(KDNode *r, GeoPoint p, int id)     { (void)p;(void)id; TODO(); return r; }
int     kd_nearest_free(const KDNode *r, GeoPoint t) { (void)r;(void)t; TODO(); return -1; }
int     kd_set_busy(KDNode *r, int id, int b)        { (void)r;(void)id;(void)b; TODO(); return -1; }
