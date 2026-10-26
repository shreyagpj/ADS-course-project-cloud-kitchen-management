#ifndef TRIE_H
#define TRIE_H
#include <stddef.h>

#define MAXC 64   //longest command
#define MAXL 64   //most commands

typedef struct TNode TNode;
typedef char Word[MAXC];

TNode *trie_new(void);
void   trie_free(TNode *r);
void   trie_add(TNode *r, const char *w);
int    trie_use(TNode *r, const char *w);
int    trie_list(TNode *r, const char *p, Word *out, int max);

#endif
