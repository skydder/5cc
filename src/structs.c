#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "5cc.h"


//===================================================================
// node
//===================================================================

Node *NewNode(NodeKind kind) {
    Node *node = calloc(1, sizeof(Node));
    node->kind = kind;
    return node;
}

Node *NewNodeBinary(NodeKind kind, Node *lhs, Node *rhs) {
    Node *node = NewNode(kind);
    node->lhs = lhs;
    node->rhs = rhs;
    return node;
}

Node *NewNodeUnary(NodeKind kind, Node *expr) {
    Node *node = NewNode(kind);
    node->lhs = expr;
    return node;
}

Node *NewNodeNum(int val) {
    Node *node = NewNode(ND_NUM);
    node->val = val;
    return node;
}

//===================================================================
// token
//===================================================================
Token *NewToken(TokenKind kind, Token *cur, char *str, int len) {
    Token *tok = calloc(1, sizeof(Token));
    tok->kind = kind;
    tok->str = str;
    cur->next = tok;
    tok->len = len;
    return tok;
}

bool ConsumeToken(char *str) {
    if (token->kind != TK_SYMBOL || strlen(str) != token->len || memcmp(token->str, str, token->len)){
        return false;
    }
    token = token->next;
    return true;
}

Token *ConsumeTokenIndent() {
    if (token->kind != TK_IDENT) {
        return NULL;
    }
    Token *tok = token;
    token = token->next;
    return tok;
}

void ExpectToken(char *op) {
    if (token->kind == TK_IDENT || strlen(op) != token->len || memcmp(token->str, op, token->len))
        error_at(token->str, "'%s'ではありません", op);
    token = token->next;
}

int ExpectTokenNum() {
    if (token->kind != TK_NUM) 
	    error_at(token->str, "数ではありません");
    int val = token->val;
    token = token->next;
    return val;
}

bool IsTokenAtEof() {
    return token->kind == TK_EOF;
}

//===================================================================
// Obj
//===================================================================
Obj *NewObj(Type *type) {
    Obj *new = calloc(1, sizeof(Obj));
    new->type = type;
    return new;
}

Obj *NewObjTok(Type *type, Token *tok) {
    Obj *new = NewObj(type);
    new->name = strndup(tok->str, tok->len);
    new->name_len = tok->len;
    new->tok = tok;
    return new;
}
Obj *NewLVar(Token *tok, Type *type) {
    Obj *new = NewObjTok(type, tok);
    new->is_local = true;
    return new;
}

void AddVar2Vec(Obj *var, vector *vec) {
    Obj *cur = (Obj*)GetVecLast(vec);
    var->offset = cur->offset + var->type->size;
    PushVec(vec, var);
}

Obj *FindVar(vector *vec, Token *tok) {
    //p_tk(tok);
    for (int i = 0; i < vec->len; i++) {
        Obj *var = (Obj*)GetVecAt(vec, i);
        if (var->name_len == tok->len
        && memcmp(tok->str, var->name, var->name_len) == 0) {
            return var;
        }
    }
    return NULL;
}

Obj *NewFunc(Token *tok, Type *type) {
    Obj *new = NewObjTok(type, tok);
    new->is_func = true;
    new->lvar = NewVec();
    new->param = NewVec();
    PushVec(new->lvar, NewObj(NULL));
    PushVec(new->param, NewObj(NULL));
    return new;
}