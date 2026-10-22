#include <stdlib.h>
#include "kdtree.h"

// Return lat or long depending on axis
static double coord(GeoPoint p, int axis) {
     return axis == 0 ? p.lat : p.lon; 
}

// Calculate the distance between two points
static double dist(GeoPoint a, GeoPoint b) {
    double dx = a.lat - b.lat, dy = a.lon - b.lon;
    return dx * dx + dy * dy;
}

// Allocate a new node for the kd tree
static KDNode *new_node(GeoPoint p, int id) {
    KDNode *n = (KDNode *)malloc(sizeof *n);
    if (!n) 
        return NULL;
    n->point = p; 
    n->staff_id = id;
    n->busy = 0;
    n->left = n->right = NULL;
    return n;
}

static KDNode *insert_rec(KDNode *r, GeoPoint p, int id, int depth) {
    if (!r) 
        return new_node(p, id);

    // Even depth implies comparison based on latitude
    int axis = depth % 2;

    // Choose either left of right subtree for insertion
    KDNode **side = (coord(p, axis) < coord(r->point, axis)) ? &r->left : &r->right;
    KDNode *child = insert_rec(*side, p, id, depth + 1);
    if (child) 
        *side = child;
    return r;
}

// Call insertion function starting with depth 0
KDNode *kd_insert(KDNode *root, GeoPoint p, int staff_id) {
    return insert_rec(root, p, staff_id, 0);
}

// Find nearest available delivery driver
static void nearest_rec(const KDNode *r, GeoPoint t, int depth, const KDNode **best, double *best_d) {
    if (!r) 
        return;

    if (!r->busy) {
        double d = dist(r->point, t);
        if (!*best || d < *best_d || (d == *best_d && r->staff_id < (*best)->staff_id)) {
            *best = r;
            *best_d = d;
        }
    }

    int axis = depth % 2;
    double diff = coord(t, axis) - coord(r->point, axis);
    const KDNode *near_side = diff < 0 ? r->left  : r->right;
    const KDNode *far_side  = diff < 0 ? r->right : r->left;

    nearest_rec(near_side, t, depth + 1, best, best_d);
    if (!*best || diff * diff <= *best_d)
        nearest_rec(far_side, t, depth + 1, best, best_d);
}

// Returns -1 if no free delivery driver was found
int kd_nearest_free(const KDNode *root, GeoPoint target) {
    const KDNode *best = NULL;
    double best_d = 0;
    nearest_rec(root, target, 0, &best, &best_d);
    return best ? best->staff_id : -1;
}

static KDNode *find_rec(KDNode *r, int id) {
    if (!r) 
        return NULL;
    if (r->staff_id == id) 
        return r;
    return find_rec(r->left, id);
}

// Set busy flag of driver once a delivery is assigned
int kd_set_busy(KDNode *root, int staff_id, int busy) {
    KDNode *n = find_rec(root, staff_id);
    if (!n) 
        return -1;
    n->busy = busy ? 1 : 0;
    return 0;
}

// Return the values stored in node if found
int kd_get(const KDNode *root, int staff_id, GeoPoint *out_point, int *out_busy) {
    KDNode *n = find_rec((KDNode *)root, staff_id);
    if (!n) 
        return -1;
    if (out_point) 
        *out_point = n->point;
    if (out_busy)  
        *out_busy  = n->busy;
    return 0;
}

// Find minimum to replace ndoe after deletion
static KDNode *find_min(KDNode *r, int axis, int depth) {
    if (!r) 
        return NULL;

    int cur = depth % 2;
    if (cur == axis)
        return r->left ? find_min(r->left, axis, depth + 1) : r;

    KDNode *best = r;
    KDNode *l = find_min(r->left,  axis, depth + 1);
    KDNode *rt = find_min(r->right, axis, depth + 1);
    if (l  && coord(l->point,  axis) < coord(best->point, axis)) 
        best = l;
    if (rt && coord(rt->point, axis) < coord(best->point, axis)) 
        best = rt;
    return best;
}

// Delete required geo point from the kd tree
static KDNode *remove_rec(KDNode *r, GeoPoint p, int id, int depth) {
    if (!r) 
        return NULL;
    int axis = depth % 2;

    if (r->staff_id == id) {
        if (r->right) {
            KDNode *m = find_min(r->right, axis, depth + 1);
            r->point = m->point; 
            r->staff_id = m->staff_id;
            r->busy = m->busy;
            r->right = remove_rec(r->right, m->point, m->staff_id, depth + 1);
        } else if (r->left) {
            KDNode *m = find_min(r->left, axis, depth + 1);
            r->point = m->point; 
            r->staff_id = m->staff_id;
            r->busy = m->busy;
            r->right = remove_rec(r->left, m->point, m->staff_id, depth + 1);
            r->left = NULL;
        } else {
            free(r);
            return NULL;
        }
        return r;
    }

    if (coord(p, axis) < coord(r->point, axis)) 
        r->left  = remove_rec(r->left,  p, id, depth + 1);
    else
        r->right = remove_rec(r->right, p, id, depth + 1);
    return r;
}

// Interface for deleting a node from the kd tree
KDNode *kd_remove(KDNode *root, int staff_id) {
    KDNode *n = find_rec(root, staff_id);
    if (!n) 
        return root;
    return remove_rec(root, n->point, staff_id, 0);
}

void kd_free(KDNode *root) {
    if (!root) 
        return;
    kd_free(root->left);
    kd_free(root->right);
    free(root);
}
