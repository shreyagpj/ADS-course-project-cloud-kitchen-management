#include <stdlib.h>
#include <string.h>
#include "trie.h"

TrieNode *trie_create(void) { return calloc(1, sizeof(TrieNode)); }

void trie_free(TrieNode *n)
{
    if (!n) return;
    for (int i = 0; i < TRIE_ALPHABET; i++) trie_free(n->child[i]);
    free(n);
}

static const TrieNode *find_node(const TrieNode *n, const char *s)
{
    for (; n && *s; s++) {
        unsigned char c = (unsigned char)*s;
        if (c >= TRIE_ALPHABET) return NULL;
        n = n->child[c];
    }
    return n;
}

int trie_insert(TrieNode *root, const char *word)
{
    if (!root || !word || !*word) return -1;
    TrieNode *n = root;
    for (const char *p = word; *p; p++) {
        unsigned char c = (unsigned char)*p;
        if (c >= TRIE_ALPHABET) return -1;
        if (!n->child[c] && !(n->child[c] = trie_create())) return -1;
        n = n->child[c];
    }
    n->is_end = 1;
    return 0;
}

int trie_contains(const TrieNode *root, const char *word)
{
    const TrieNode *n = find_node(root, word);
    return n && n->is_end;
}

void trie_record_use(TrieNode *root, const char *word)
{
    TrieNode *n = (TrieNode *)find_node(root, word);
    if (n && n->is_end) n->uses++;
}

/* ---- suggestions ---- */
typedef struct { char *word; int uses; } Hit;

typedef struct { Hit *hits; int n, cap; } HitList;

static void collect(const TrieNode *n, char *buf, int len, HitList *L)
{
    if (n->is_end) {
        if (L->n == L->cap) {
            int nc = L->cap ? L->cap * 2 : 16;
            Hit *t = realloc(L->hits, nc * sizeof(Hit));
            if (!t) return;
            L->hits = t; L->cap = nc;
        }
        buf[len] = '\0';
        L->hits[L->n].word = malloc((size_t)len + 1);
        if (!L->hits[L->n].word) return;
        memcpy(L->hits[L->n].word, buf, (size_t)len + 1);
        L->hits[L->n].uses = n->uses;
        L->n++;
    }
    if (len >= TRIE_MAX_WORD - 1) return;
    for (int c = 0; c < TRIE_ALPHABET; c++)          /* alphabetical DFS */
        if (n->child[c]) { buf[len] = (char)c; collect(n->child[c], buf, len + 1, L); }
}

static int cmp_hit(const void *a, const void *b)
{
    const Hit *x = a, *y = b;
    if (x->uses != y->uses) return y->uses - x->uses;
    return strcmp(x->word, y->word);
}

int trie_suggest(const TrieNode *root, const char *prefix, char **out, int max)
{
    size_t pl = strlen(prefix);
    if (pl >= TRIE_MAX_WORD) return 0;
    const TrieNode *n = find_node(root, prefix);
    if (!n) return 0;

    char buf[TRIE_MAX_WORD];
    memcpy(buf, prefix, pl);
    HitList L = {0};
    collect(n, buf, (int)pl, &L);
    if (L.n) qsort(L.hits, L.n, sizeof(Hit), cmp_hit);

    int k = 0;
    for (int i = 0; i < L.n; i++) {
        if (k < max) out[k++] = L.hits[i].word; else free(L.hits[i].word);
    }
    free(L.hits);
    return k;
}

void trie_free_suggestions(char **out, int n)
{
    for (int i = 0; i < n; i++) free(out[i]);
}

int trie_common_prefix(const TrieNode *root, const char *prefix, char *buf, int size)
{
    size_t len = strlen(prefix);
    if ((int)len >= size) return -1;
    const TrieNode *n = find_node(root, prefix);
    if (!n) return -1;
    memcpy(buf, prefix, len);
    /* walk down while there is exactly one way forward and no word ends here */
    while (!n->is_end && len < (size_t)size - 1) {
        int only = -1, cnt = 0;
        for (int c = 0; c < TRIE_ALPHABET; c++)
            if (n->child[c]) { only = c; cnt++; }
        if (cnt != 1) break;
        buf[len++] = (char)only;
        n = n->child[only];
    }
    buf[len] = '\0';
    return (int)len;
}
