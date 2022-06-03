#include <stddef.h>
#include <string.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>

#include "5cc.h"

vector *gFuncs;  // <obj*>

//===================================================================
Obj *func();
Obj *cur_fn;
static Node *stmt();
static Node *block();
static Node *expr_stmt();
static Node *expr();
static Node *assign();
static Node *equal();
static Node *relation();
static Node *add();
static Node *mul();
static Node *unary();
static Node *primary();
static Node *null_stmt();
static Node *array_index();
//===================================================================

void program() {
    gFuncs = NewVec();
    while (!IsTokenAtEof()) {
        PushVec(gFuncs, func());
    }
}

void param(Obj *func) {
    Obj* var = NewLVar(ConsumeTokenIndent(), BaseType());
    AddVar2Vec(var, func->param);
    AddVar2Vec(var, func->lvar);
}
void params(Obj *func) {
    param(func);
    while (!ConsumeToken(")")) {
        ExpectToken(",");
        param(func);
    }
}

Obj *func(){
    Obj *fnc = NewFunc(ConsumeTokenIndent(), BaseType());
    if (fnc->tok) {
        cur_fn = fnc;
        ExpectToken("(");
        if (!ConsumeToken(")")) {
            params(fnc);
        }

        ExpectToken("{");
        fnc->body = block(); 
    } else {
        fnc = NULL;
    }
    return fnc;
}


void declartion() {
    Type *type = BaseType();
    Token *tok = ConsumeTokenIndent();
    if (ConsumeToken("[")) {
        type  = NewTypeArray(type, ExpectTokenNum());
        ExpectToken("]");
    }
    Obj *var = NewLVar(tok, type);
    
    //p_var(var);
    AddVar2Vec(var, cur_fn->lvar);
    return;
}

static Node *stmt() {
    Node *node;
    switch (gToken->kind) {
        case TK_RETURN:
            node = NewNode(ND_RETURN);
            ExpectToken("return");
            node->lhs = expr();
            ExpectToken(";");
            return node;
        case TK_WHILE:
            node = NewNode(ND_WHILE);
            ExpectToken("while");
            ExpectToken("(");
            node->cond = expr();
            ExpectToken(")");
            node->then = stmt();
            return node;
        case TK_FOR:
            node = NewNode(ND_FOR);
            ExpectToken("for");
            ExpectToken("(");
            if(!ConsumeToken(";")) {
                node->init = expr();
                ExpectToken(";");
            }
            if (!ConsumeToken(";")) {
                node->cond = expr();
                ExpectToken(";");
            }
            if (!ConsumeToken(")")) {
                node->inc = expr();
                ExpectToken(")");
            }
            node->then = stmt();
            return node;
        case TK_IF:
            node = NewNode(ND_IF);
            ExpectToken("if");
            ExpectToken("(");
            node->cond = expr();
            ExpectToken(")");
            node->then = stmt();
            if (!ConsumeToken("else")) 
                node->els = stmt();
            return node;
        case TK_INT:
            declartion();
            return null_stmt();
    }
    
    if (ConsumeToken("{")) {
        return block();
    }
    
    return expr_stmt();
    
}

/*
|* expr_stmt = expr ";"
|*/
static Node *expr_stmt() {
    Node *node = NewNodeUnary(ND_EXPR_STMT, expr());
    ExpectToken(";");
    return node;
}
static Node *null_stmt() {
    Node *node = NewNode(ND_NULL);
    ExpectToken(";");
    return node;
}
/*
|* block = stmt* "}"
|*/
static Node *block() {
    Node tmp = {};
    Node *cur = &tmp;
    while (!ConsumeToken("}")) 
        cur = cur->next = stmt();
    Node *node = NewNode(ND_BLOCK);
    node->body = tmp.next;
    return node;
}

/*
|* expr = assign
|*/
static Node *expr(){
    return assign();
}

/*
|* assign = equal ("=" assign)?
|*/
static Node *assign() {
    Node *node = equal();
    if (ConsumeToken("=")) 
        node = NewNodeBinary(ND_ASSIGN, node, assign());
    return node;
}

/*
|* equal = relation ("!=" relation | "==" relation)*
|*/
static Node *equal(){
    Node *node = relation();

    for (;;) {
        if (ConsumeToken("!="))
            node = NewNodeBinary(ND_NEQ, node, relation());
        else if (ConsumeToken("=="))
            node = NewNodeBinary(ND_EQ, node, relation());
        else
            return node;
    }
}

/*
|* relation = add ("<=" add | "<" add | ">=" add | ">" add)*
|*/
static Node *relation(){
    Node *node = add();

    for (;;) {
        if (ConsumeToken("<=")) 
            node = NewNodeBinary(ND_LTE, node, add());
        else if (ConsumeToken("<")) 
            node = NewNodeBinary(ND_LT, node, add());
        else if (ConsumeToken(">=")) 
            node = NewNodeBinary(ND_GTE, node, add());
        else if (ConsumeToken(">")) 
            node = NewNodeBinary(ND_GT, node, add());
        else 
            return node;
    }
}

static Node *new_add(Node *lhs, Node *rhs) {
    AddType(lhs);
    AddType(rhs);

    // num + num
    if (lhs->type->ty == INT && rhs->type->ty == INT)
        return NewNodeBinary(ND_ADD, lhs, rhs);

    // Canonicalize `num + ptr` to `ptr + num`.
    if (lhs->type->ty == INT && (rhs->type->ty == PTR || rhs->type->ty == ARRAY)) {
        Node *tmp = lhs;
        lhs = rhs;
        rhs = tmp;
    }
    if (lhs->type->ty == PTR && rhs->type->ty == PTR)
        error("you can't add ptr to ptr\n");

    // ptr + num
    rhs = NewNodeBinary(ND_MUL, rhs, NewNodeNum(8/*lhs->type->size*/));
    return NewNodeBinary(ND_ADD, lhs, rhs);
}

/*
|* add = mul ("+" mul | "-" mul)*
|*/
static Node *add() {
    Node *node = mul();

    for (;;) {
        if (ConsumeToken("+"))
            // node = new_node(ND_ADD, node, mul());
            node = new_add(node, mul());
        else  if (ConsumeToken("-"))
            node = NewNodeBinary(ND_SUB, node, mul());
        else
            return node;
    }
}

/*  
|* mul= unary ("*" unary | "/" unary)*
|*/
static Node *mul() {
    Node *node = unary();

    for (;;) {
        if (ConsumeToken("*"))
            node = NewNodeBinary(ND_MUL, node, unary());
        else if (ConsumeToken("/"))
            node = NewNodeBinary(ND_DIV, node, unary());
        else
            return node;
    }
}

/*
|* unary = ("+" | "-" | "*" | "&" )? unary                           
|*/
static Node *unary() {
    if (ConsumeToken("+"))
        return array_index();
    if (ConsumeToken("-"))
        return NewNodeBinary(ND_SUB, NewNodeNum(0), unary());
    if (ConsumeToken("&"))
        return NewNodeUnary(ND_ADDR, unary());
    if (ConsumeToken("*"))
        return NewNodeUnary(ND_DEREF, unary());
    if (ConsumeToken("sizeof"))
        return NewNodeNum(8);  // <= return type size
    return array_index();
}

Node *array_index() {
    Node *lhs = primary();
    if (ConsumeToken("[")) {
        Node *rhs = expr();
        ExpectToken("]");
        Node *node = NewNodeUnary(ND_DEREF, new_add(lhs, rhs));
        return node;
    }
    return lhs;
}
/*
|* primary = "(" expr ")"                                   
|*           | indent
|*           | num
|*/
static Node *primary() {
    if (ConsumeToken("(")) {
        Node *node = expr();
        ExpectToken(")");
        return node;
    }
    
    Token *tok = ConsumeTokenIndent();
    if (tok) {
        if (PeekTokenAt(0, "(")){
            Node *node = NewNode(ND_FUNCALL);
            node->fn_name = strndup(tok->str, tok->len); 
            ExpectToken("(");
            if (!ConsumeToken(")")) {
                node->arg = expr();
                for (Node *i = node->arg; !ConsumeToken(")"); i = i->next){
                    ExpectToken(",");
                    i->next = expr();
                }
            }
            return node;
        } else {
            Node *node = NewNode(ND_LVAR);
        
            Obj *var = FindVar(cur_fn->lvar, tok);
            if (var) {
                node->offset = var->offset;
                node->lvar = var;
                node->type = var->type;
            } else {
                error_at(gToken->str,"変数が定義されていません");
            }
            return node;
        }
    }
    return NewNodeNum(ExpectTokenNum());
}

