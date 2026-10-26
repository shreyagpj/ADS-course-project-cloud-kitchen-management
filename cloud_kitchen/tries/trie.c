#include <stdlib.h>
#include <string.h>
#include "trie.h"

#define AZ 128

struct TNode
{
    TNode *kid[AZ];
    int end, cnt;
};

typedef struct
{
    Word w;
    int cnt;
} Ent;

TNode *trie_new(void)
{
    return calloc(1, sizeof(TNode));
}

void trie_free(TNode *n)
{
    if (!n) return;
    for (int i = 0; i < AZ; i++)
        trie_free(n->kid[i]);
    free(n);
}

void trie_add(TNode *r, const char *w)
{
    if (strlen(w) >= MAXC) return;
    for (; *w; w++)
    {
        int c = *w & 127;
        if (!r->kid[c])
            r->kid[c] = trie_new();
        if (!r->kid[c])
            return;
        r = r->kid[c];
    }
    r->end = 1;
}

static TNode *walk(TNode *r, const char *s)
{
    for (; *s && r; s++)
        r = r->kid[*s & 127];
    return r;
}

int trie_use(TNode *r, const char *w)
{
    TNode *n = walk(r, w);
    if (!n || !n->end)
        return -1;
    n->cnt++;
    return 0;
}

static void dfs(TNode *n, char *b, int len, Ent *e, int max, int *k)
{
    if (n->end && *k < max)
    {
        b[len] = '\0';
        strcpy(e[*k].w, b);
        e[*k].cnt = n->cnt;
        (*k)++;
    }
    for (int c = 0; c < AZ && len < MAXC - 1; c++)
    {
        if (n->kid[c])
        {
            b[len] = (char) c;
            dfs(n->kid[c], b, len + 1, e, max, k);
        }
    }
}

static int gather(TNode *r, const char *p, Ent *e, int max)
{
    size_t len = strlen(p);
    TNode *n = len < MAXC ? walk(r, p) : NULL;
    if (!n)
        return 0;

    char b[MAXC];
    strcpy(b, p);
    int k = 0;
    dfs(n, b, (int) len, e, max, &k);
    return k;
}

//most used first and then a to z
static int by_cnt(const void *a, const void *b)
{
    const Ent *x = a, *y = b;
    return x->cnt != y->cnt ? y->cnt - x->cnt : strcmp(x->w, y->w);
}

int trie_list(TNode *r, const char *p, Word *out, int max)
{
    Ent e[MAXL];
    int n = gather(r, p, e, MAXL);
    qsort(e, (size_t) n, sizeof(Ent), by_cnt);
    if (n > max)
        n = max;
    for (int i = 0; i < n; i++)
        strcpy(out[i], e[i].w);
    return n;
}
