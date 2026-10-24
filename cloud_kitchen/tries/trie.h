#ifndef TRIE_H
#define TRIE_H
#include <stddef.h>

#define MAXC 64   /* max command length */
#define MAXL 64   /* max commands */

typedef struct TNode TNode;
typedef char Word[MAXC];

TNode *trie_new(void);
void   trie_free(TNode *r);
void   trie_add(TNode *r, const char *w);
int    trie_use(TNode *r, const char *w);                          /* count++ */
int    trie_best(TNode *r, const char *p, char *out, size_t sz);   /* most used match */
int    trie_lcp(TNode *r, const char *p, char *out, size_t sz);    /* common prefix */
int    trie_list(TNode *r, const char *p, Word *out, int max);
int    trie_near(TNode *r, const char *w, Word *out, int max);     /* did you mean */

#endif
