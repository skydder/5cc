#include <stdlib.h>
#include <assert.h>
#include <stdbool.h>

#include "vector.h"

vector *new_vec() {
    vector *new = calloc(1, sizeof(vector));
    new->capacity = 16;
    new->data = calloc(new->capacity, sizeof(void*));
    new->len = 0;
    return new;
}

void vec_push(vector *vec, void *item) {
    if (vec->capacity == vec->len) {
        vec->capacity *= 2;
        vec->data = realloc(vec->data, sizeof(void*) * vec->capacity);
    }
    vec->data[vec->len++] = item;
    return;
}

void *vec_pop(vector *vec) {
    assert(vec->len);
    return vec->data[--vec->len];
}

void *vec_last(vector *vec) {
    assert(vec->len  >= -1);
    if (vec->len == -1)
        return NULL;
    return vec->data[vec->len - 1];
}

bool vec_contains(vector *vec, void *item) {
    for (int i = 0; i < vec->len; i++)
        if (vec->data[i] == item)
            return true;
    
    return false;
}

void *vec_get(vector *vec, int index) {
    assert(0 <= index && index < vec->len);
    return vec->data[index];
}