#include <stddef.h>
#include <string.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>

#include "5cc.h"

Function *fn;

//===================================================================
Function *func();
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
//===================================================================

//Function *code[100];
vector *funcs;

void program() {
    funcs = new_vec();
    while (!IsTokenAtEof()) {
        vec_push(funcs, func());
    }
}

void param(Function *func) {
    Type *type = BaseType();
    Token *tok = ConsumeTokenIndent();
    Var* var = NewVar(tok->str, tok->len, type);
    AddVar2Vec(var, func->param);
    AddVar2Vec(var, func->lvar);
}
void params(Function *func) {
    param(func);
    while (!ConsumeToken(")")) {
        ExpectToken(",");
        param(func);
    }
}

Function *func(){
    Function *fnc = calloc(1, sizeof(Function));
    fnc->type = BaseType();
    Token *fn_tok = ConsumeTokenIndent();
    fnc->name = strndup(fn_tok->str, fn_tok->len);
    fnc->lvar = new_vec();
    fnc->param = new_vec();
    vec_push(fnc->lvar, NewVar(NULL, 0, NULL));
    vec_push(fnc->param, NewVar(NULL, 0, NULL));

    if (fn_tok) {
        fn = fnc;
        ExpectToken("(");
        if (!ConsumeToken(")")) {
            params(fnc);
        }

        ExpectToken("{");
        fnc->body = block();
        return fnc;
    } 
    return NULL;
}


void declartion() {
    Type *type = BaseType();
    Token *tok = ConsumeTokenIndent();
    if (ConsumeToken("[")) {
        type  = NewTyArray(type, ExpectTokenNum());
        ExpectToken("]");
    }
    Var *var = NewVar(tok->str, tok->len, type);
    
    //p_var(var);
    AddVar2Vec(var, fn->lvar);
    return;
}

static Node *stmt() {
    Node *node;
    switch (token->kind) {
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
    return assign(fn);
}

/*
|* assign = equal ("=" assign)?
|*/
static Node *assign() {
    Node *node = equal(fn);
    if (ConsumeToken("=")) 
        node = NewNodeBinary(ND_ASSIGN, node, assign());
    return node;
}

/*
|* equal = relation ("!=" relation | "==" relation)*
|*/
static Node *equal(){
    Node *node = relation(fn);

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
    Node *node = add(fn);

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
    if (lhs->type->ty == INT && rhs->type->ty == PTR) {
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
        return primary();
    if (ConsumeToken("-"))
        return NewNodeBinary(ND_SUB, NewNodeNum(0), unary());
    if (ConsumeToken("&"))
        return NewNodeUnary(ND_ADDR, unary());
    if (ConsumeToken("*"))
        return NewNodeUnary(ND_DEREF, unary());
    if (ConsumeToken("sizeof"))
        return NewNodeNum(8);  // <= return type size
    return primary();
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
        Node *node = calloc(1, sizeof(Node));
        if (!is_same(tok->next->str, "(")){
            node->kind = ND_LVAR;
        
            Var *var = FindVar(fn->lvar, tok);
            if (var) {
                node->offset = var->offset;
                node->lvar = var;
            } else {
                error_at(token->str,"変数が定義されていません");
            }
            return node;
        } else {
            node->kind = ND_FUNCALL;
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
        }
    }
    return NewNodeNum(ExpectTokenNum());
}

