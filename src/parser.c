#include <stddef.h>
#include <string.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>

#include "5cc.h"
#include "vector.h"

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
    while (!at_eof()) {
        vec_push(funcs, func());
    }
}

void param(Function *func) {
    Type *type = base_type();
    Token *tok = consume_indent();
    Var* var = new_var(tok->str, tok->len, type);
    add_var2vec(var, func->param);
    add_var2vec(var, func->lvar);
}
void params(Function *func) {
    param(func);
    while (!consume_op(")")) {
        expect(",");
        param(func);
    }
}

Function *func(){
    Function *fnc = calloc(1, sizeof(Function));
    fnc->type = base_type();
    Token *fn_tok = consume_indent();
    fnc->name = strndup(fn_tok->str, fn_tok->len);
    fnc->lvar = new_vec();
    fnc->param = new_vec();
    vec_push(fnc->lvar, new_var(NULL, 0, NULL));
    vec_push(fnc->param, new_var(NULL, 0, NULL));

    if (fn_tok) {
        fn = fnc;
        expect("(");
        if (!consume_op(")")) {
            params(fnc);
        }

        expect("{");
        fnc->body = block();
        return fnc;
    } 
    return NULL;
}


void declartion() {
    Type *type = base_type();
    Token *tok = consume_indent();
    Var *var = new_var(tok->str, tok->len, type);
    //p_var(var);
    add_var2vec(var, fn->lvar);
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
        return block();
    }
    
    return expr_stmt();
    
}

/*
|* expr_stmt = expr ";"
|*/
static Node *expr_stmt() {
    Node *node = new_unary(ND_EXPR_STMT, expr());
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
        node = new_node(ND_ASSIGN, node, assign());
    return node;
}

/*
|* equal = relation ("!=" relation | "==" relation)*
|*/
static Node *equal(){
    Node *node = relation(fn);

    for (;;) {
        if (consume_op("!="))
            node = new_node(ND_NEQ, node, relation());
        else if (consume_op("=="))
            node = new_node(ND_EQ, node, relation());
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
            node = new_node(ND_LTE, node, add());
        else if (consume_op("<")) 
            node = new_node(ND_LT, node, add());
        else if (consume_op(">=")) 
            node = new_node(ND_GTE, node, add());
        else if (consume_op(">")) 
            node = new_node(ND_GT, node, add());
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
            node = new_node(ND_ADD, node, mul());
        else  if (consume_op("-"))
            node = new_node(ND_SUB, node, mul());
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
            node = new_node(ND_MUL, node, unary());
        else if (consume_op("/"))
            node = new_node(ND_DIV, node, unary());
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
        return new_node(ND_SUB, new_node_num(0), unary());
    if (consume_op("&"))
        return new_unary(ND_ADDR, unary());
    if (consume_op("*"))
        return new_unary(ND_DEREF, unary());
    return primary(fn);
}

/*
|* primary = "(" expr ")"                                   
|*           | indent
|*           | num
|*/
static Node *primary() {
    if (consume_op("(")) {
        Node *node = expr();
        expect(")");
        return node;
    }

    Token *tok = consume_indent();
    if (tok) {
        Node *node = calloc(1, sizeof(Node));
        if (!is_same(tok->next->str, "(")){
            node->kind = ND_LVAR;
        
            Var *var = find_var(fn->lvar, tok);
            if (var) {
                node->offset = var->offset;
            } else {
                error_at(token->str,"変数が定義されていません");
            }
            return node;
        } else {
            node->kind = ND_FUNCALL;
            node->fn_name = strndup(tok->str, tok->len); 
            expect("(");
            if (!consume_op(")")) {
                node->arg = expr();
                for (Node *i = node->arg; !consume_op(")"); i = i->next){
                    expect(",");
                    i->next = expr();
                }
                
            }
            return node;
        }
    }
    return new_node_num(expect_number());
}

