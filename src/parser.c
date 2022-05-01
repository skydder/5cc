#include <stddef.h>
#include <string.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>

#include "5cc.h"

Function *fn;
//===================================================================
// token
//===================================================================
static bool consume_op(char *op) {
    if (token->kind != TK_SYMBOL || strlen(op) != token->len || memcmp(token->str, op, token->len)){
        return false;
    }
    token = token->next;
    return true;
}

static Token *consume_indent() {
    if (token->kind != TK_IDENT) {
        //error_at(token->str, "'indent'ではありません");
        return NULL;
    }
    Token *tok = token;
    token = token->next;
    return tok;
}

static bool consume_tk(TokenKind tk) {
    if (token->kind != tk) 
        return false;
    token = token->next;
    return true;
}

static void expect(char *op) {
    if (token->kind == TK_IDENT || strlen(op) != token->len || memcmp(token->str, op, token->len))
        error_at(token->str, "'%s'ではありません", op);
    token = token->next;
}

static int expect_number() {
    if (token->kind != TK_NUM) 
	    error_at(token->str, "数ではありません");
    int val = token->val;
    token = token->next;
    return val;
}

static bool at_eof() {
    return token->kind == TK_EOF;
}

//===================================================================
// lvar
//===================================================================
static LVar *new_lvar(char *name, int len, LVar *next) {
    LVar *lvar = calloc(1, sizeof(LVar));
    lvar->next = next;
    lvar->name = name;
    lvar->len = len;
    lvar->offset = lvar->next->offset + 8;
    
    return lvar;
}

static LVar *find_lvar(Token *tok) {
    for (LVar *var = fn->args; var->name; var = var->next)
        if (var->len == tok->len && memcmp(tok->str, var->name, var->len) == 0)
            return var;
    for (LVar *var = fn->locals; var->name; var = var->next)
        if (var->len == tok->len && memcmp(tok->str, var->name, var->len) == 0)
            return var;
    return NULL;
}

//===================================================================
// node
//===================================================================

static Node *new_kind(NodeKind kind) {
    Node *node = calloc(1, sizeof(Node));
    node->kind = kind;
    return node;
}

static Node *new_node(NodeKind kind, Node *lhs, Node *rhs) {
    Node *node = new_kind(kind);
    node->lhs = lhs;
    node->rhs = rhs;
    return node;
}

static Node *new_unary(NodeKind kind, Node *expr) {
    Node *node = new_kind(kind);
    node->lhs = expr;
    return node;
}
static Node *new_node_num(int val) {
    Node *node = new_kind(ND_NUM);
    node->val = val;
    return node;
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
    while (consume_op("*")) {
        cur = new_ptr2(cur);
    }
    return cur;
}

//===================================================================
Function *func();
LVar *read_param();
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

Function *code[100];

void program() {
    int i = 0;
    while (!at_eof()) {
        code[i++] = func();
    }
    code[i] = NULL;
}

Function *func(){
    Function *fnc = calloc(1, sizeof(Function));
    fnc->type = base_type();
    Token *fn_tok = consume_indent();
    fnc->name = strndup(fn_tok->str, fn_tok->len);
    fnc->locals = calloc(1, sizeof(LVar));
    fnc->locals->offset = 0;

    if (fn_tok) {
        expect("(");
        if (!consume_op(")")) {
            fnc->args = read_param(fn);
            fnc->locals = fn->args;
        } else
            fnc->args = calloc(1, sizeof(LVar));

        fn = fnc;
        expect("{");
        fnc->body = block();
        return fnc;
    } 
    return NULL;
}

LVar *read_params(Function *fnc) {
    LVar *cur = read_param(fnc);
    while (!consume_op(")")) {
        expect(",");
        cur = read_param(fnc);
    }
    return cur;
}
LVar *read_param(Function *fnc) {
    Type *type = base_type();
    Token *tok = consume_indent();
    LVar *cur = new_lvar(tok->str, tok->len, fnc->locals);
    cur->type = type;
    fnc->locals = cur;
    return cur;
}
void declartion() {
    Type *type = base_type();
    Token *tok = consume_indent();
    LVar *lvar = new_lvar(tok->str, tok->len, fn->locals);
    lvar->type = type;
    fn->locals = lvar;
    return;
}

static Node *stmt() {
    Node *node;
    switch (token->kind) {
        case TK_RETURN:
            node = new_kind(ND_RETURN);
            expect("return");
            node->lhs = expr();
            expect(";");
            return node;
        case TK_WHILE:
            node = new_kind(ND_WHILE);
            expect("while");
            expect("(");
            node->cond = expr();
            expect(")");
            node->then = stmt();
            return node;
        case TK_FOR:
            node = new_kind(ND_FOR);
            expect("for");
            expect("(");
            if(!consume_op(";")) {
                node->init = expr();
                expect(";");
            }
            if (!consume_op(";")) {
                node->cond = expr();
                expect(";");
            }
            if (!consume_op(")")) {
                node->inc = expr();
                expect(")");
            }
            node->then = stmt();
            return node;
        case TK_IF:
            node = new_kind(ND_IF);
            expect("if");
            expect("(");
            node->cond = expr();
            expect(")");
            node->then = stmt();
            if (!consume_tk(TK_ELSE)) 
                node->els = stmt();
            return node;
        case TK_INT:
            declartion();
            return null_stmt();
    }
    
    if (consume_op("{")) {
        return block(fn);
    }
    
    return expr_stmt(fn);
    
}

/*
|* expr_stmt = expr ";"
|*/
static Node *expr_stmt() {
    Node *node = new_unary(ND_EXPR_STMT, expr(fn));
    expect(";");
    return node;
}
static Node *null_stmt() {
    Node *node = new_kind(ND_NULL);
    expect(";");
    return node;
}
/*
|* block = stmt* "}"
|*/
static Node *block() {
    Node tmp = {};
    Node *cur = &tmp;
    while (!consume_op("}")) 
        cur = cur->next = stmt();
    Node *node = new_kind(ND_BLOCK);
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
    if (consume_op("=")) 
        node = new_node(ND_ASSIGN, node, assign(fn));
    return node;
}

/*
|* equal = relation ("!=" relation | "==" relation)*
|*/
static Node *equal(){
    Node *node = relation(fn);

    for (;;) {
        if (consume_op("!="))
            node = new_node(ND_NEQ, node, relation(fn));
        else if (consume_op("=="))
            node = new_node(ND_EQ, node, relation(fn));
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
        if (consume_op("<=")) 
            node = new_node(ND_LTE, node, add(fn));
        else if (consume_op("<")) 
            node = new_node(ND_LT, node, add(fn));
        else if (consume_op(">=")) 
            node = new_node(ND_GTE, node, add(fn));
        else if (consume_op(">")) 
            node = new_node(ND_GT, node, add(fn));
        else 
            return node;
    }
}

/*
|* add = mul ("+" mul | "-" mul)*
|*/
static Node *add() {
    Node *node = mul(fn);

    for (;;) {
        if (consume_op("+"))
            node = new_node(ND_ADD, node, mul(fn));
        else  if (consume_op("-"))
            node = new_node(ND_SUB, node, mul(fn));
        else
            return node;
    }
}

/*  
|* mul= unary ("*" unary | "/" unary)*
|*/
static Node *mul() {
    Node *node = unary(fn);

    for (;;) {
        if (consume_op("*"))
            node = new_node(ND_MUL, node, unary(fn));
        else if (consume_op("/"))
            node = new_node(ND_DIV, node, unary(fn));
        else
            return node;
    }
}

/*
|* unary = ("+" | "-" | "*" | "&" )? unary                           
|*/
static Node *unary() {
    if (consume_op("+"))
        return primary(fn);
    if (consume_op("-"))
        return new_node(ND_SUB, new_node_num(0), unary(fn));
    if (consume_op("&"))
        return new_unary(ND_ADDR, unary(fn));
    if (consume_op("*"))
        return new_unary(ND_DEREF, unary(fn));
    return primary(fn);
}

/*
|* primary = "(" expr ")"                                   
|*           | indent
|*           | num
|*/
static Node *primary() {
    if (consume_op("(")) {
        Node *node = expr(fn);
        expect(")");
        return node;
    }

    Token *tok = consume_indent();
    if (tok) {
        Node *node = calloc(1, sizeof(Node));
        if (!is_same(tok->next->str, "(")){
            node->kind = ND_LVAR;
        
            LVar *lvar = find_lvar(tok);
            if (lvar) {
                node->offset = lvar->offset;
            } else {
                error_at(token->str,"変数が定義されていません");
            }
            return node;
        } else {
            node->kind = ND_FUNCALL;
            node->fn_name = strndup(tok->str, tok->len); 
            expect("(");
            if (!consume_op(")")) {
                node->arg = expr(fn);
                for (Node *i = node->arg; !consume_op(")"); i = i->next){
                    expect(",");
                    i->next = expr(fn);
                }
                
            }
            return node;
        }
    }
    return new_node_num(expect_number());
}

