#include <stdlib.h>
#include <string.h>
#include "trie.h"

#define AZ 128

struct TNode {
    TNode *kid[AZ];
    int end, cnt;
};

typedef struct { Word w; int cnt, dist; } Ent;

TNode *trie_new(void) { return calloc(1, sizeof(TNode)); }

void trie_free(TNode *n)
{
    if (!n) return;
    for (int i = 0; i < AZ; i++) trie_free(n->kid[i]);
    free(n);
}

void trie_add(TNode *r, const char *w)
{
    if (strlen(w) >= MAXC) return;
    for (; *w; w++) {
        int c = *w & 127;
        if (!r->kid[c]) r->kid[c] = trie_new();
        if (!r->kid[c]) return;
        r = r->kid[c];
    }
    r->end = 1;
}

static TNode *walk(TNode *r, const char *s)
{
    for (; *s && r; s++) r = r->kid[*s & 127];
    return r;
}

int trie_use(TNode *r, const char *w)
{
    TNode *n = walk(r, w);
    if (!n || !n->end) return -1;
    n->cnt++;
    return 0;
}

static void dfs(TNode *n, char *b, int len, Ent *e, int max, int *k)
{
    if (n->end && *k < max) {
        b[len] = '\0';
        strcpy(e[*k].w, b);
        e[*k].cnt = n->cnt;
        (*k)++;
    }
    for (int c = 0; c < AZ && len < MAXC - 1; c++)
        if (n->kid[c]) { b[len] = (char)c; dfs(n->kid[c], b, len + 1, e, max, k); }
}

static int gather(TNode *r, const char *p, Ent *e, int max)
{
    size_t len = strlen(p);
    TNode *n = len < MAXC ? walk(r, p) : NULL;
    if (!n) return 0;
    char b[MAXC];
    strcpy(b, p);
    int k = 0;
    dfs(n, b, (int)len, e, max, &k);
    return k;
}

static int by_cnt(const void *a, const void *b)
{
    const Ent *x = a, *y = b;
    return x->cnt != y->cnt ? y->cnt - x->cnt : strcmp(x->w, y->w);
}

static int by_dist(const void *a, const void *b)
{
    const Ent *x = a, *y = b;
    return x->dist != y->dist ? x->dist - y->dist : by_cnt(a, b);
}

int trie_best(TNode *r, const char *p, char *out, size_t sz)
{
    Ent e[MAXL];
    int n = gather(r, p, e, MAXL);
    if (n == 0) return 0;
    int b = 0;
    for (int i = 1; i < n; i++)
        if (e[i].cnt > e[b].cnt) b = i;
    strncpy(out, e[b].w, sz - 1);
    out[sz - 1] = '\0';
    return 1;
}

int trie_lcp(TNode *r, const char *p, char *out, size_t sz)
{
    size_t len = strlen(p);
    TNode *n = len < MAXC ? walk(r, p) : NULL;
    if (!n || len >= sz) return 0;
    strcpy(out, p);
    while (!n->end) {
        int only = -1, kids = 0;
        for (int c = 0; c < AZ; c++)
            if (n->kid[c]) { kids++; only = c; }
        if (kids != 1 || len + 1 >= sz) break;
        out[len++] = (char)only;
        n = n->kid[only];
    }
    out[len] = '\0';
    return 1;
}

int trie_list(TNode *r, const char *p, Word *out, int max)
{
    Ent e[MAXL];
    int n = gather(r, p, e, MAXL);
    qsort(e, (size_t)n, sizeof(Ent), by_cnt);
    if (n > max) n = max;
    for (int i = 0; i < n; i++) strcpy(out[i], e[i].w);
    return n;
}

static int dist(const char *a, const char *b)
{
    int la = (int)strlen(a), lb = (int)strlen(b);
    int d[MAXC + 1][MAXC + 1];
    if (la >= MAXC || lb >= MAXC) return 99;
    for (int i = 0; i <= la; i++) d[i][0] = i;
    for (int j = 0; j <= lb; j++) d[0][j] = j;
    for (int i = 1; i <= la; i++)
        for (int j = 1; j <= lb; j++) {
            int m = d[i-1][j-1] + (a[i-1] != b[j-1]);
            if (d[i-1][j] + 1 < m) m = d[i-1][j] + 1;
            if (d[i][j-1] + 1 < m) m = d[i][j-1] + 1;
            d[i][j] = m;
        }
    return d[la][lb];
}

int trie_near(TNode *r, const char *w, Word *out, int max)
{
    Ent e[MAXL], f[MAXL];
    int n = gather(r, "", e, MAXL), k = 0;
    for (int i = 0; i < n; i++) {
        int d = dist(w, e[i].w);
        if (d <= 3) { f[k] = e[i]; f[k].dist = d; k++; }
    }
    qsort(f, (size_t)k, sizeof(Ent), by_dist);
    if (k > max) k = max;
    for (int i = 0; i < k; i++) strcpy(out[i], f[i].w);
    return k;
}
