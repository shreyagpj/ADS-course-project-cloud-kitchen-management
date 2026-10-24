#ifndef TERMINAL_H
#define TERMINAL_H
#include <stddef.h>
#include "../trie/trie.h"

/* ghost suggestion, Tab = complete (Tab Tab = list), Right arrow = accept.
 * Returns buf, or NULL on EOF. */
char *readline_ac(const char *prompt, TNode *t, char *buf, size_t sz);

#endif
