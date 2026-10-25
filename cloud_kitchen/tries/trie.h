#ifndef TRIE_H
#define TRIE_H
#include <stddef.h>

#define MAXC 64 //max command length
#define MAXL 64 //max commands

typedef struct TNode TNode;
typedef char Word[MAXC];

TNode *trie_new(void);
void   trie_free(TNode *r);
void   trie_add(TNode *r, const char *w);
int    trie_use(TNode *r, const char *w); //count++
int    trie_list(TNode *r, const char *p, Word *out, int max);

#endif
