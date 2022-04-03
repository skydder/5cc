#include <stddef.h>
#include <string.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>

#include "5cc.h"

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
    return token;
}

static bool consume_tk(TokenKind tk) {
    if (token->kind != tk) 
        return false;
    token = token->next;
    return true;
}

static void expect(char *op) {
    if (strlen(op) != token->len || memcmp(token->str, op, token->len))
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

static LVar *find_lvar(Token *tok, Function *fn) {
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
Function *func();
static Node *stmt(Function *fn);
static Node *block(Function *fn);
static Node *expr_stmt(Function *fn);
static Node *expr(Function *fn);
static Node *assign(Function *fn);
static Node *equal(Function *fn);
static Node *relation(Function *fn);
static Node *add(Function *fn);
static Node *mul(Function *fn);
static Node *unary(Function *fn);
static Node *primary(Function *fn);
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
    Function *fn = calloc(1, sizeof(Function));
    Token *fn_tok = consume_indent();
    fn->name = strndup(fn_tok->str, fn_tok->len);
    fn->locals = calloc(1, sizeof(LVar));
    fn->locals->offset = 0;
    token = token->next;
    if (fn_tok) {
        expect("(");
        if (!consume_op(")")) {
            Token *tok = consume_indent();
            LVar *lvar = new_lvar(tok->str, tok->len, fn->locals);
            fn->locals = lvar;
            fn->arg = 1;
            token = token->next;
            for (int i = 0; !consume_op(")"); i++){
                expect(",");
                tok = consume_indent();
                lvar = new_lvar(tok->str, tok->len, fn->locals);
                fn->locals = lvar;
                fn->arg += 1;
                token = token->next;
            }
        }

        expect("{");
        fn->body = block(fn);
        return fn;
    } 
    return NULL;
}

static Node *stmt(Function *fn) {
    Node *node;
    switch (token->kind) {
        case TK_RETURN:
            node = new_kind(ND_RETURN);
            expect("return");
            node->lhs = expr(fn);
            expect(";");
            return node;
        case TK_WHILE:
            node = new_kind(ND_WHILE);
            expect("while");
            expect("(");
            node->cond = expr(fn);
            expect(")");
            node->then = stmt(fn);
            return node;
        case TK_FOR:
            node = new_kind(ND_FOR);
            expect("for");
            expect("(");
            if(!consume_op(";")) {
                node->init = expr(fn);
                expect(";");
            }
            if (!consume_op(";")) {
                node->cond = expr(fn);
                expect(";");
            }
            if (!consume_op(")")) {
                node->inc = expr(fn);
                expect(")");
            }
            node->then = stmt(fn);
            return node;
        case TK_IF:
            node = new_kind(ND_IF);
            expect("if");
            expect("(");
            node->cond = expr(fn);
            expect(")");
            node->then = stmt(fn);
            if (!consume_tk(TK_ELSE)) 
                node->els = stmt(fn);
            return node;
    }
    
    if (consume_op("{")) {
        return block(fn);
    }
    
    return expr_stmt(fn);
    
}

/*
|* expr_stmt = expr ";"
*/
static Node *expr_stmt(Function *fn) {
    Node *node;
    node = new_kind(ND_EXPR_STMT);
    node->lhs = expr(fn);
    expect(";");
    return node;
}

/*
|* block = stmt* "}"
*/
static Node *block(Function *fn) {
    Node tmp = {};
    Node *cur = &tmp;
    while (!consume_op("}")) 
        cur = cur->next = stmt(fn);
    Node *node = new_kind(ND_BLOCK);
    node->body = tmp.next;
    return node;
}

/*
|* expr = assign
*/
static Node *expr(Function *fn){
    return assign(fn);
}

/*
|* assign = equal ("=" assign)?
*/
static Node *assign(Function *fn) {
    Node *node = equal(fn);
    if (consume_op("=")) 
        node = new_node(ND_ASSIGN, node, assign(fn));
    return node;
}

/*
|* equal = relation ("!=" relation | "==" relation)*
*/
static Node *equal(Function *fn){
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
*/
static Node *relation(Function *fn){
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
*/
static Node *add(Function *fn) {
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
*/
static Node *mul(Function *fn) {
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
*/
static Node *unary(Function *fn) {
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
*/
static Node *primary(Function *fn) {
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
        
            LVar *lvar = find_lvar(tok, fn);
            if (lvar) {
                node->offset = lvar->offset;
            } else {
                lvar = new_lvar(tok->str, tok->len, fn->locals);
                node->offset = lvar->offset;
                fn->locals = lvar;
            }
            token = token->next;
            return node;
        } else {
            node->kind = ND_FUNCALL;
            node->fn_name = strndup(tok->str, tok->len); 
            token = token->next;
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

