#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <termios.h>
#include <sys/select.h>
#include "terminal.h"

static void draw(const char *p, const char *b, TNode *t)
{
    char g[MAXC];
    size_t n = strlen(b);
    printf("\r%s%s\033[K", p, b);
    if (n > 0 && trie_best(t, b, g, sizeof g) && strlen(g) > n)
        printf("\033[90m%s\033[0m\033[%dD", g + n, (int)(strlen(g) - n));
    fflush(stdout);
}

//waits up to 50 ms for one byte; 1 if read
static int rd1(unsigned char *c)
{
    fd_set f;
    struct timeval t = { 0, 50000 };
    FD_ZERO(&f);
    FD_SET(STDIN_FILENO, &f);
    if (select(STDIN_FILENO + 1, &f, NULL, NULL, &t) <= 0) return 0;
    return read(STDIN_FILENO, c, 1) == 1;
}

static void put(char *b, size_t cap, const char *s)
{
    snprintf(b, cap, "%s", s);
}

char *readline_ac(const char *p, TNode *t, char *b, size_t sz)
{
    size_t cap = sz < MAXC ? sz : MAXC;
    if (!isatty(STDIN_FILENO)) {
        printf("%s", p);
        fflush(stdout);
        if (!fgets(b, (int)sz, stdin)) return NULL;
        b[strcspn(b, "\n")] = '\0';
        return b;
    }

    struct termios o, r;
    if (tcgetattr(STDIN_FILENO, &o) != 0) return NULL;
    r = o;
    r.c_lflag &= ~(tcflag_t)(ICANON | ECHO | ISIG);
    r.c_cc[VMIN] = 1;
    r.c_cc[VTIME] = 0;
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &r);

    size_t n = 0;
    int tab = 0, eof = 0;
    unsigned char c;
    b[0] = '\0';
    draw(p, b, t);

    for (;;) {
        if (read(STDIN_FILENO, &c, 1) != 1) { eof = 1; break; }
        if (c == '\n' || c == '\r') break;
        if (c == 4) {
            if (n == 0) { eof = 1; break; }
            continue;
        }
        if (c == 3) { b[0] = '\0'; n = 0; printf("^C"); break; }
        if (c == '\t') {
            char l[MAXC];
            if (trie_lcp(t, b, l, sizeof l) && strlen(l) > n) {
                put(b, cap, l);
                n = strlen(b);
                tab = 0;
            } else if (tab) {
                Word w[MAXL];
                int k = trie_list(t, b, w, MAXL);
                printf("\r\n");
                for (int i = 0; i < k; i++) printf("%s  ", w[i]);
                if (k) printf("\r\n");
                tab = 0;
            } else tab = 1;
            draw(p, b, t);
            continue;
        }
        tab = 0;
        if (c == 27) {
            unsigned char s[2];
            if (rd1(&s[0]) && s[0] == '[' && rd1(&s[1]) && s[1] == 'C') {
                char g[MAXC];
                if (trie_best(t, b, g, sizeof g)) {
                    put(b, cap, g);
                    n = strlen(b);
                }
            }
        } else if ((c == 127 || c == 8) && n > 0) {
            b[--n] = '\0';
        } else if (c >= 32 && c < 127 && n + 1 < cap) {
            b[n++] = (char)c;
            b[n] = '\0';
        }
        draw(p, b, t);
    }

    tcsetattr(STDIN_FILENO, TCSAFLUSH, &o);
    printf("\r%s%s\033[K\r\n", p, b);
    fflush(stdout);
    return eof ? NULL : b;
}
