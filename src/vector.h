#ifndef __vector__
#define __vector__

typedef struct {
    void **data;
    int len;
    int capacity;
} vector;

vector *new_vec();
void vec_push(vector *vec, void *item);
void *vec_pop(vector *vec);
void *vec_last(vector *vec);
bool vec_contains(vector *vec, void *item);
void *vec_get(vector *vec, int index);
#endif