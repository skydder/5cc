#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "5cc.h"

char *gUserInput;
void error_at(char *loc, char *fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    int pos = (loc - gUserInput);

    fprintf(stderr, "%s\n", gUserInput);
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

vector *NewVec() {
    vector *new = calloc(1, sizeof(vector));
    new->capacity = 16;
    new->data = calloc(new->capacity, sizeof(void*));
    new->len = 0;
    return new;
}

void PushVec(vector *vec, void *item) {
    if (vec->capacity == vec->len) {
        vec->capacity *= 2;
        vec->data = realloc(vec->data, sizeof(void*) * vec->capacity);
    }
    vec->data[vec->len++] = item;
    return;
}

void *PopVec(vector *vec) {
    assert(vec->len);
    return vec->data[--vec->len];
}

void *GetVecLast(vector *vec) {
    assert(vec->len  >= -1);
    if (vec->len == -1)
        return NULL;
    return vec->data[vec->len - 1];
}

bool ContainsVec(vector *vec, void *item) {
    for (int i = 0; i < vec->len; i++)
        if (vec->data[i] == item)
            return true;
    
    return false;
}

void *GetVecAt(vector *vec, int index) {
    assert(0 <= index && index < vec->len);
    return vec->data[index];
}