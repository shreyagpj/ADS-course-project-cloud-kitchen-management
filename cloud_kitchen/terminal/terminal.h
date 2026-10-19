/* terminal.h - interactive command prompt with trie-based autosuggest.
 *   - grey "ghost" text shows the best suggestion while you type
 *   - Tab        : complete to the common prefix (press again to list matches)
 *   - Right/End  : accept the ghost suggestion
 *   - Enter      : submit,  Ctrl-D on empty line : EOF,  Ctrl-C : clear line
 * If stdin/stdout is not a terminal it falls back to plain fgets().        */
#ifndef TERMINAL_H
#define TERMINAL_H
#include "../trie/trie.h"

/* Reads one command into buf (lowercased, trimmed). Returns 1 on success,
 * 0 on EOF.                                                               */
int terminal_read_command(const char *prompt, const TrieNode *commands,
                          char *buf, int size);
#endif
