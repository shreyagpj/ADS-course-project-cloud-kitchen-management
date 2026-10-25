#ifndef TERMINAL_H
#define TERMINAL_H
#include <stddef.h>
#include "../tries/trie.h"

//ghost text, Tab completes (Tab Tab lists), Right arrow accepts; returns b or NULL on EOF
char *readline_ac(const char *p, TNode *t, char *b, size_t sz);

#endif
