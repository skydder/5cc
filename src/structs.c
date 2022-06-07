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
    node->type = ty_int;
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
    if (gToken->kind != TK_SYMBOL || strlen(str) != gToken->len || memcmp(gToken->str, str, gToken->len)){
        return false;
    }
    gToken = gToken->next;
    return true;
}

Token *ConsumeTokenIndent() {
    if (gToken->kind != TK_IDENT) {
        return NULL;
    }
    Token *tok = gToken;
    gToken = gToken->next;
    return tok;
}

bool PeekTokenAt(int index, char *op) {
    Token *cur = gToken;
    for (int i = 0; i < index; i++) {
        cur = cur->next;
    }
    if (cur->kind == TK_IDENT || strlen(op) != cur->len || !is_same(cur->str, op))
        return false;
    return true;
}

void ExpectToken(char *op) {
    if (gToken->kind == TK_IDENT || strlen(op) != gToken->len || memcmp(gToken->str, op, gToken->len))
        error_at(gToken->str, "'%s'ではありません", op);
    gToken = gToken->next;
}

Token *ExpectTokenIndent() {
    Token *tok = ConsumeTokenIndent();
    if (!tok)
        error_at(gToken->str, "not indent");
    return tok;
}
int ExpectTokenNum() {
    if (gToken->kind != TK_NUM) 
	    error_at(gToken->str, "数ではありません");
    int val = gToken->val;
    gToken = gToken->next;
    return val;
}

bool IsTokenAtEof() {
    return gToken->kind == TK_EOF;
}

//===================================================================
// Obj
//===================================================================
Obj *NewObj(Type *type) {
    Obj *new = calloc(1, sizeof(Obj));
    new->type = type;
    return new;
}

void AddObjTok(Obj *obj, Token *tok) {
    obj->name = strndup(tok->str, tok->len);
    obj->name_len = tok->len;
    obj->tok = tok;
}

Obj *NewLVar(Token *tok, Type *type) {
    Obj *new = NewObj(type);
    AddObjTok(new, tok);
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
    Obj *new = NewObj(type);
    AddObjTok(new, tok);
    new->is_func = true;
    new->lvar = NewVec();
    new->param = NewVec();
    PushVec(new->lvar, NewObj(NULL));
    PushVec(new->param, NewObj(NULL));
    return new;
}