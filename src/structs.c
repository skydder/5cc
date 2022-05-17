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
Type *new_ptr2(Type *cur) {
    Type *new = calloc(1, sizeof(Type));
    new->ptr_to = cur;
    new->ty = PTR;
    return new;
}
Type *base_type() {
    Type *cur = calloc(1, sizeof(Type));
    expect("int");
    cur->ty = INT;
    cur->size = 8;
    while (consume_op("*")) {
        cur = new_ptr2(cur);
    }
    return cur;
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