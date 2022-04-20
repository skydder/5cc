#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>

#include "5cc.h"

char *user_input;
void error_at(char *loc, char *fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    int pos = (loc - user_input);

    fprintf(stderr, "%s\n", user_input);
    fprintf(stderr, "%*s", pos, "");
    fprintf(stderr, "^ ");
    vfprintf(stderr, fmt, ap);
    fprintf(stderr, "\n");
    exit(1);
}

void error(char *fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    vfprintf(stderr, fmt, ap);
    fprintf(stderr, "\n");
    exit(1);
}

bool is_same(char *string, char *word) {
    return (strncmp(string, word, strlen(word)) == 0);
}