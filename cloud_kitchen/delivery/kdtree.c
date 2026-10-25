#include <stdlib.h>
#include "kdtree.h"

//Return lat or long depending on axis
static double coord(GeoPoint p, int axis) {
     return axis == 0 ? p.lat : p.lon; 
}

//Calculate the distance between two points
static double dist(GeoPoint a, GeoPoint b) {
    double dx = a.lat - b.lat, dy = a.lon - b.lon;
    return dx * dx + dy * dy;
}

//Allocate a new node for the kd tree
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

    //Even depth implies comparison based on latitude
    int axis = depth % 2;

    //Choose either left of right subtree for insertion
    KDNode **side = (coord(p, axis) < coord(r->point, axis)) ? &r->left : &r->right;
    KDNode *child = insert_rec(*side, p, id, depth + 1);
    if (child) 
        *side = child;
    return r;
}

//Call insertion function starting with depth 0
KDNode *kd_insert(KDNode *root, GeoPoint p, int sid) {
    return insert_rec(root, p, sid, 0);
}

//Find nearest available delivery driver
static void nearest_rec(const KDNode *r, GeoPoint t, int depth, const KDNode **bt, double *bd) {
    if (!r) 
        return;

    if (!r->busy) {
        double d = dist(r->point, t);
        if (!*bt || d < *bd || (d == *bd && r->staff_id < (*bt)->staff_id)) {
            *bt = r;
            *bd = d;
        }
    }

    int axis = depth % 2;
    double df = coord(t, axis) - coord(r->point, axis);
    const KDNode *ns = df < 0 ? r->left  : r->right;
    const KDNode *fs  = df < 0 ? r->right : r->left;

    nearest_rec(ns, t, depth + 1, bt, bd);
    if (!*bt || df * df <= *bd)
        nearest_rec(fs, t, depth + 1, bt, bd);
}

//Returns -1 if no free delivery driver was found
int kd_nearest_free(const KDNode *root, GeoPoint target) {
    const KDNode *bt = NULL;
    double bd = 0;
    nearest_rec(root, target, 0, &bt, &bd);
    return bt ? bt->staff_id : -1;
}

static KDNode *find_rec(KDNode *r, int id) {
    if (!r) 
        return NULL;
    if (r->staff_id == id) 
        return r;
    KDNode *x = find_rec(r->left, id);
    return x ? x : find_rec(r->right, id);
}

//Set busy flag of driver once a delivery is assigned
int kd_set_busy(KDNode *root, int sid, int busy) {
    KDNode *n = find_rec(root, sid);
    if (!n) 
        return -1;
    n->busy = busy ? 1 : 0;
    return 0;
}

//Return the values stored in node if found
int kd_get(const KDNode *root, int sid, GeoPoint *op, int *ob) {
    KDNode *n = find_rec((KDNode *)root, sid);
    if (!n) 
        return -1;
    if (op) 
        *op = n->point;
    if (ob)  
        *ob  = n->busy;
    return 0;
}

//Find minimum to replace ndoe after deletion
static KDNode *find_min(KDNode *r, int axis, int depth) {
    if (!r) 
        return NULL;

    int cur = depth % 2;
    if (cur == axis)
        return r->left ? find_min(r->left, axis, depth + 1) : r;

    KDNode *bt = r;
    KDNode *l = find_min(r->left,  axis, depth + 1);
    KDNode *rt = find_min(r->right, axis, depth + 1);
    if (l  && coord(l->point,  axis) < coord(bt->point, axis)) 
        bt = l;
    if (rt && coord(rt->point, axis) < coord(bt->point, axis)) 
        bt = rt;
    return bt;
}

//Delete required geo point from the kd tree
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

//Interface for deleting a node from the kd tree
KDNode *kd_remove(KDNode *root, int sid) {
    KDNode *n = find_rec(root, sid);
    if (!n) 
        return root;
    return remove_rec(root, n->point, sid, 0);
}

void kd_free(KDNode *root) {
    if (!root) 
        return;
    kd_free(root->left);
    kd_free(root->right);
    free(root);
}
