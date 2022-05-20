#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include "5cc.h"
#include "vector.h"

//===================================================================
// node
//===================================================================

Node *new_kind(NodeKind kind) {
    Node *node = calloc(1, sizeof(Node));
    node->kind = kind;
    return node;
}

Node *new_node(NodeKind kind, Node *lhs, Node *rhs) {
    Node *node = new_kind(kind);
    node->lhs = lhs;
    node->rhs = rhs;
    return node;
}

Node *new_unary(NodeKind kind, Node *expr) {
    Node *node = new_kind(kind);
    node->lhs = expr;
    return node;
}

Node *new_node_num(int val) {
    Node *node = new_kind(ND_NUM);
    node->val = val;
    return node;
}

//===================================================================
// token
//===================================================================
Token *new_token(TokenKind kind, Token *cur, char *str) {
    Token *tok = calloc(1, sizeof(Token));
    tok->kind = kind;
    tok->str = str;
    cur->next = tok;
    return tok;
}
Token *new_tk_str(TokenKind kind, Token *cur, char *str, int len) {
    Token *tok = new_token(kind, cur, str);
    tok->len = len;
    return tok;
}
bool consume_op(char *op) {
    if (token->kind != TK_SYMBOL || strlen(op) != token->len || memcmp(token->str, op, token->len)){
        return false;
    }
    token = token->next;
    return true;
}

Token *consume_indent() {
    if (token->kind != TK_IDENT) {
        //error_at(token->str, "'indent'ではありません");
        return NULL;
    }
    Token *tok = token;
    token = token->next;
    return tok;
}

bool consume_tk(TokenKind tk) {
    if (token->kind != tk) 
        return false;
    token = token->next;
    return true;
}

void expect(char *op) {
    if (token->kind == TK_IDENT || strlen(op) != token->len || memcmp(token->str, op, token->len))
        error_at(token->str, "'%s'ではありません", op);
    token = token->next;
}

int expect_number() {
    if (token->kind != TK_NUM) 
	    error_at(token->str, "数ではありません");
    int val = token->val;
    token = token->next;
    return val;
}

bool at_eof() {
    return token->kind == TK_EOF;
}

//===================================================================
// type.c
//===================================================================

Type ty_int = {INT, NULL, 8, 0};

Type *new_type(TypeKind tk, int size) {
    Type *new = calloc(1, sizeof(Type));
    new->ty = tk;
    new->size = size;
    return new;
}
Type *new_ptr2(Type *cur) {
    // Type *new = calloc(1, sizeof(Type));
    // new->ty = PTR;
    Type *new = new_type(PTR, 8);
    new->ptr_to = cur;
    return new;
}
Type *base_type() {
    // Type *cur = calloc(1, sizeof(Type));
    expect("int");
    // cur->ty = INT;
    Type *cur = &ty_int;
    // cur->size = 8;
    while (consume_op("*")) {
        cur = new_ptr2(cur);
    }
    return cur;
}
void add_type(Node *node) {
    if (!node || node->type)
        return;
    add_type(node->lhs);
    add_type(node->rhs);
    add_type(node->cond);
    add_type(node->then);
    add_type(node->els);
    add_type(node->init);
    add_type(node->inc);

    for (Node *n = node->body; n; n = n->next)
        add_type(n);
    switch (node->kind) {
        case ND_NUM:
            node->type = &ty_int;
            return;
        case ND_LVAR:
            node->type = node->lvar->type;
            return;
        case ND_ADD:
            if (node->lhs->type == &ty_int && node->rhs->type != &ty_int) {
                node->type = node->rhs->type;
                return;
            }
            if (node->lhs->type != &ty_int && node->rhs->type == &ty_int) {
                node->type = node->lhs->type;
                return;
            }
            if (node->lhs->type == &ty_int && node->rhs->type == &ty_int) {
                node->type = &ty_int;
                return;
            }
            if (node->lhs->type != &ty_int && node->rhs->type != &ty_int)
                error("you can't add ptr to ptr\n");
        default:
            return;
    }
}
//===================================================================
// var
//===================================================================
Var *new_var(char *name, int len, Type *type) {
    Var *new = calloc(1, sizeof(Var));
    new->name = name;
    new->len = len;
    new->type = type;
    return new;
}

void add_var2vec(Var *var, vector *vec) {
    Var *cur = (Var*)vec_last(vec);
    var->offset = cur->offset + var->type->size;
    vec_push(vec, var);
}

Var *find_var(vector *vec, Token *tok) {
    //p_tk(tok);
    for (int i = 0; i < vec->len; i++) {
        if (((Var*)vec->data[i])->len == tok->len
        && memcmp(tok->str, ((Var*)vec->data[i])->name, ((Var*)vec->data[i])->len) == 0) {
            return (Var*)vec->data[i];
        }
        //p_var((Var*)vec->data[i]);
    }
    return NULL;
}