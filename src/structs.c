#include <stddef.h>
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
Token *NewToken(TokenKind kind, Token *cur, char *str) {
    Token *tok = calloc(1, sizeof(Token));
    tok->kind = kind;
    tok->str = str;
    cur->next = tok;
    return tok;
}
Token *NewTokenStr(TokenKind kind, Token *cur, char *str, int len) {
    Token *tok = NewToken(kind, cur, str);
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
        //error_at(token->str, "'indent'ではありません");
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
// var
//===================================================================
Var *NewVar(char *name, int len, Type *type) {
    Var *new = calloc(1, sizeof(Var));
    new->name = name;
    new->len = len;
    new->type = type;
    return new;
}

void AddVar2Vec(Var *var, vector *vec) {
    Var *cur = (Var*)GetVecLast(vec);
    var->offset = cur->offset + var->type->size;
    PushVec(vec, var);
}

Var *FindVar(vector *vec, Token *tok) {
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