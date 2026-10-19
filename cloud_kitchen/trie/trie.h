/* trie.h - prefix tree used for terminal command autosuggest.
 * Whole command phrases (e.g. "orders place") are stored, so suggestions
 * work across word boundaries. Each command keeps a usage count so the
 * most-used matches are suggested first.                                   */
#ifndef TRIE_H
#define TRIE_H

#define TRIE_ALPHABET 128                 /* plain ASCII */
#define TRIE_MAX_WORD 64

typedef struct TrieNode {
    struct TrieNode *child[TRIE_ALPHABET];
    int is_end;                           /* a command ends here */
    int uses;                             /* how often it was run */
} TrieNode;

TrieNode *trie_create(void);
void      trie_free(TrieNode *root);
int       trie_insert(TrieNode *root, const char *word);   /* 0 ok, -1 fail */
int       trie_contains(const TrieNode *root, const char *word);
void      trie_record_use(TrieNode *root, const char *word); /* bump rank */

/* Fill out[] with up to max words starting with prefix, ranked by usage
 * (high first) then alphabetically. Each string is malloc'd; free with
 * trie_free_suggestions. Returns number of suggestions.                    */
int  trie_suggest(const TrieNode *root, const char *prefix,
                  char **out, int max);
void trie_free_suggestions(char **out, int n);

/* Longest prefix shared by every word that starts with `prefix`
 * (used for Tab completion). Writes into buf, returns its length.          */
int  trie_common_prefix(const TrieNode *root, const char *prefix,
                        char *buf, int size);
#endif
