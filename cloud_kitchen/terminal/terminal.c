#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>
#include <termios.h>
#include <stdlib.h>
#include "terminal.h"

#define MAX_SUGGEST 20
#define GREY  "\x1b[90m"
#define RESET "\x1b[0m"

static struct termios saved;

static int raw_on(void)
{
    struct termios t;
    if (tcgetattr(STDIN_FILENO, &saved) < 0) return -1;
    t = saved;
    t.c_lflag &= ~(ICANON | ECHO);
    t.c_cc[VMIN] = 1; t.c_cc[VTIME] = 0;
    return tcsetattr(STDIN_FILENO, TCSANOW, &t);
}
static void raw_off(void) { tcsetattr(STDIN_FILENO, TCSANOW, &saved); }

static void normalize(char *s)           /* lowercase, trim, collapse spaces */
{
    char *w = s; int sp = 0;
    for (char *r = s; *r; r++) {
        if (isspace((unsigned char)*r)) { sp = (w != s); continue; }
        if (sp) { *w++ = ' '; sp = 0; }
        *w++ = (char)tolower((unsigned char)*r);
    }
    *w = '\0';
}

/* redraw: prompt + typed text + grey ghost remainder, cursor back at end of typed text */
static void redraw(const char *prompt, const char *line, const TrieNode *t)
{
    char *sug[1]; int n = 0;
    const char *ghost = "";
    if (*line) n = trie_suggest(t, line, sug, 1);
    if (n == 1) ghost = sug[0] + strlen(line);
    printf("\r\x1b[K%s%s%s%s%s", prompt, line, GREY, ghost, RESET);
    if (*ghost) printf("\x1b[%zuD", strlen(ghost));
    fflush(stdout);
    if (n == 1) trie_free_suggestions(sug, 1);
}

static void accept_top(const TrieNode *t, char *line, int size, int *len)
{
    char *sug[1];
    if (*len && trie_suggest(t, line, sug, 1) == 1) {
        strncpy(line, sug[0], size - 1); line[size - 1] = '\0';
        *len = (int)strlen(line);
        trie_free_suggestions(sug, 1);
    }
}

int terminal_read_command(const char *prompt, const TrieNode *t, char *buf, int size)
{
    if (!isatty(STDIN_FILENO) || !isatty(STDOUT_FILENO)) {   /* piped input */
        printf("%s", prompt); fflush(stdout);
        if (!fgets(buf, size, stdin)) return 0;
        buf[strcspn(buf, "\n")] = '\0';
        normalize(buf);
        return 1;
    }
    if (raw_on() < 0) {                                       /* fallback */
        printf("%s", prompt);
        if (!fgets(buf, size, stdin)) return 0;
        buf[strcspn(buf, "\n")] = '\0'; normalize(buf);
        return 1;
    }

    int len = 0, tabbed = 0, ok = 1;
    buf[0] = '\0';
    redraw(prompt, buf, t);

    for (;;) {
        unsigned char c;
        if (read(STDIN_FILENO, &c, 1) <= 0) { ok = 0; break; }

        if (c == '\n' || c == '\r') {
            printf("\r\x1b[K%s%s\n", prompt, buf);            /* drop ghost text */
            break;
        } else if (c == 4) {                                  /* Ctrl-D */
            if (len == 0) { printf("\n"); ok = 0; break; }
        } else if (c == 3) {                                  /* Ctrl-C */
            len = 0; buf[0] = '\0'; printf("^C\n");
        } else if (c == 127 || c == 8) {                      /* backspace */
            if (len) buf[--len] = '\0';
        } else if (c == '\t') {
            char common[TRIE_MAX_WORD];
            int cl = trie_common_prefix(t, buf, common, sizeof common);
            if (cl > len) { strncpy(buf, common, size - 1); buf[size - 1] = '\0'; len = (int)strlen(buf); tabbed = 0; }
            else if (tabbed) {                                /* second Tab: list */
                char *sug[MAX_SUGGEST];
                int n = trie_suggest(t, buf, sug, MAX_SUGGEST);
                printf("\r\x1b[K%s%s\n", prompt, buf);
                for (int i = 0; i < n; i++) printf("  %s\n", sug[i]);
                trie_free_suggestions(sug, n);
                tabbed = 0;
            } else tabbed = 1;
            redraw(prompt, buf, t);
            continue;
        } else if (c == 27) {                                 /* escape sequences */
            unsigned char s[2];
            if (read(STDIN_FILENO, &s[0], 1) == 1 && s[0] == '[' &&
                read(STDIN_FILENO, &s[1], 1) == 1) {
                if (s[1] == 'C' || s[1] == 'F') accept_top(t, buf, size, &len);  /* → / End */
            }
        } else if (isprint(c) && len < size - 1) {
            buf[len++] = (char)c; buf[len] = '\0';
        }
        tabbed = 0;
        redraw(prompt, buf, t);
    }
    raw_off();
    if (ok) normalize(buf);
    return ok;
}
