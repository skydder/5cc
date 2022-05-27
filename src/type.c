#include <stdlib.h>
#include <string.h>
#include "5cc.h"

Type *ty_int = &(Type){INT, NULL, 8, 0};

Type *NewType(int tk, int size) {
    Type *new = calloc(1, sizeof(Type));
    new->ty = tk;
    new->size = size;
    return new;
}
Type *NewTypeArray(Type *ty, int size) {
    Type *new = NewType(ARRAY, size * 8);
    new->ptr_to = ty;
    new->array_size = size;
    return new;
}
Type *NewTypePtr2(Type *cur) {
    Type *new = NewType(PTR, 8);
    new->ptr_to = cur;
    return new;
}
Type *BaseType() {
    ExpectToken("int");
    Type *cur = ty_int;
    while (ConsumeToken("*")) {
        cur = NewTypePtr2(cur);
    }
    return cur;
}

int size_of(Type* type) {
    return type->size;
}
void AddType(Node *node) {
    if (!node || node->type)
        return;
    AddType(node->lhs);
    AddType(node->rhs);
    AddType(node->cond);
    AddType(node->then);
    AddType(node->els);
    AddType(node->init);
    AddType(node->inc);

    for (Node *n = node->body; n; n = n->next)
        AddType(n);
    switch (node->kind) {
        case ND_NUM:
            node->type = ty_int;
            return;
        case ND_LVAR:
            node->type = node->lvar->type;
            return;
        case ND_ADD:
            if (node->lhs->type == ty_int && node->rhs->type != ty_int) {
                node->type = node->rhs->type;
                return;
            }
            if (node->lhs->type != ty_int && node->rhs->type == ty_int) {
                node->type = node->lhs->type;
                return;
            }
            if (node->lhs->type == ty_int && node->rhs->type == ty_int) {
                node->type = ty_int;
                return;
            }
            if (node->lhs->type != ty_int && node->rhs->type != ty_int)
                error("you can't add ptr to ptr\n");
        default:
            return;
    }
}