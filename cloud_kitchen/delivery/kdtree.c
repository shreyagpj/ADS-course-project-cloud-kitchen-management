#include <stdlib.h>
#include "kdtree.h"

static double coord(GeoPoint p, int axis)
{
    return axis == 0 ? p.lat : p.lon;
}

//squared distance is enough to compare
static double dist(GeoPoint a, GeoPoint b)
{
    double dx = a.lat - b.lat, dy = a.lon - b.lon;
    return dx * dx + dy * dy;
}

static KDNode *new_node(GeoPoint p, int id)
{
    KDNode *n = (KDNode *) malloc(sizeof *n);
    if (!n)
        return NULL;
    n->point = p;
    n->staff_id = id;
    n->busy = 0;
    n->left = n->right = NULL;
    return n;
}

static KDNode *add_rec(KDNode *r, GeoPoint p, int id, int depth)
{
    if (!r)
        return new_node(p, id);

    //even depth splits on latitude and odd depth on longitude
    int axis = depth % 2;
    KDNode **side = (coord(p, axis) < coord(r->point, axis)) ? &r->left : &r->right;
    KDNode *child = add_rec(*side, p, id, depth + 1);
    if (child)
        *side = child;
    return r;
}

KDNode *kd_add(KDNode *root, GeoPoint p, int sid)
{
    return add_rec(root, p, sid, 0);
}

//best holds the closest free partner so far
static void near_rec(const KDNode *r, GeoPoint t, int depth, const KDNode **best, double *bd)
{
    if (!r)
        return;

    if (!r->busy)
    {
        double d = dist(r->point, t);
        if (!*best || d < *bd || (d == *bd && r->staff_id < (*best)->staff_id))
        {
            *best = r;
            *bd = d;
        }
    }

    int axis = depth % 2;
    double df = coord(t, axis) - coord(r->point, axis);
    const KDNode *ns = df < 0 ? r->left : r->right;
    const KDNode *fs = df < 0 ? r->right : r->left;

    near_rec(ns, t, depth + 1, best, bd);

    //the far side is checked only if it can hold a closer point
    if (!*best || df * df <= *bd)
        near_rec(fs, t, depth + 1, best, bd);
}

int kd_nearest(const KDNode *root, GeoPoint target)
{
    const KDNode *best = NULL;
    double bd = 0;
    near_rec(root, target, 0, &best, &bd);
    return best ? best->staff_id : -1;
}

static KDNode *find_rec(KDNode *r, int id)
{
    if (!r)
        return NULL;
    if (r->staff_id == id)
        return r;
    KDNode *x = find_rec(r->left, id);
    return x ? x : find_rec(r->right, id);
}

int kd_busy(KDNode *root, int sid, int busy)
{
    KDNode *n = find_rec(root, sid);
    if (!n)
        return -1;
    n->busy = busy ? 1 : 0;
    return 0;
}

int kd_get(const KDNode *root, int sid, GeoPoint *op, int *ob)
{
    KDNode *n = find_rec((KDNode *) root, sid);
    if (!n)
        return -1;
    if (op)
        *op = n->point;
    if (ob)
        *ob = n->busy;
    return 0;
}

//smallest value on one axis is the node that replaces a deleted one
static KDNode *find_min(KDNode *r, int axis, int depth)
{
    if (!r)
        return NULL;

    if (depth % 2 == axis)
        return r->left ? find_min(r->left, axis, depth + 1) : r;

    KDNode *best = r;
    KDNode *l = find_min(r->left, axis, depth + 1);
    KDNode *rt = find_min(r->right, axis, depth + 1);
    if (l && coord(l->point, axis) < coord(best->point, axis))
        best = l;
    if (rt && coord(rt->point, axis) < coord(best->point, axis))
        best = rt;
    return best;
}

static KDNode *del_rec(KDNode *r, GeoPoint p, int id, int depth)
{
    if (!r)
        return NULL;
    int axis = depth % 2;

    if (r->staff_id == id)
    {
        if (r->right)
        {
            KDNode *m = find_min(r->right, axis, depth + 1);
            r->point = m->point;
            r->staff_id = m->staff_id;
            r->busy = m->busy;
            r->right = del_rec(r->right, m->point, m->staff_id, depth + 1);
        }
        else if (r->left)
        {
            KDNode *m = find_min(r->left, axis, depth + 1);
            r->point = m->point;
            r->staff_id = m->staff_id;
            r->busy = m->busy;
            r->right = del_rec(r->left, m->point, m->staff_id, depth + 1);
            r->left = NULL;
        }
        else
        {
            free(r);
            return NULL;
        }
        return r;
    }

    if (coord(p, axis) < coord(r->point, axis))
        r->left = del_rec(r->left, p, id, depth + 1);
    else
        r->right = del_rec(r->right, p, id, depth + 1);
    return r;
}

KDNode *kd_del(KDNode *root, int sid)
{
    KDNode *n = find_rec(root, sid);
    if (!n)
        return root;
    return del_rec(root, n->point, sid, 0);
}

void kd_free(KDNode *root)
{
    if (!root)
        return;
    kd_free(root->left);
    kd_free(root->right);
    free(root);
}
