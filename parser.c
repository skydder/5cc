#include <stddef.h>
#include <string.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>

#include "5cc.h"

Node *new_n_kind(NodeKind kind) {
    Node *node = calloc(1, sizeof(Node));
    node->kind = kind;
    return node;
}

Node *new_node(NodeKind kind, Node *lhs, Node *rhs) {
    Node *node = calloc(1, sizeof(Node));
    node->kind = kind;
    node->lhs = lhs;
    node->rhs = rhs;
    return node;
}

Node *new_node_num(int val) {
    Node *node = calloc(1, sizeof(Node));
    node->kind = ND_NUM;
    node->val = val;
    return node;
}

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

Node *stmt(Function *fn) {
    Node *node;

    if (consume_tk(TK_RETURN)) {
        node = calloc(1, sizeof(Node));
        node->kind = ND_RETURN;
        node->lhs = expr(fn);
        expect(";");
        return node;
    }

    if (consume_tk(TK_WHILE)) {
        node = new_n_kind(ND_WHILE);
        
        expect("(");
        node->cond = expr(fn);
        expect(")");
        node->then = stmt(fn);
        return node;
    }

    if (consume_tk(TK_FOR)) {
        node = new_n_kind(ND_FOR);
    
        expect("(");

        if(consume_op(";") != true)
            node->init = expr_stmt(fn);

        if (consume_op(";") != true) {
            node->cond = expr(fn);
            expect(";");
        }
        if (consume_op(")") != true) {
            node->inc = expr(fn);
            expect(")");
        }
        node->then = stmt(fn);
        return node;
    }

    if (consume_tk(TK_IF)) {
        node = new_n_kind(ND_IF);
        
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

Node *expr_stmt(Function *fn) {
    Node *node;
    node = calloc(1, sizeof(Node));
    node->kind = ND_EXPR_STMT;
    node->lhs = expr(fn);
    expect(";");
    return node;
}
Node *block(Function *fn) {
    Node tmp = {};
    Node *cur = &tmp;
    while (consume_op("}") != true) 
        cur = cur->next = stmt(fn);
    Node *node = new_n_kind(ND_BLOCK);
    node->body = tmp.next;
    return node;
}

Node *expr(Function *fn){
    return assign(fn);
}

Node *assign(Function *fn) {
    Node *node = equal(fn);
    if (consume_op("=")) {
        node = new_node(ND_ASSIGN, node, assign(fn));
    }
    return node;
}

Node *equal(Function *fn){
    Node *node = relation(fn);

    for (;;) {
        if (consume_op("!=")) {
            node = new_node(ND_NEQ, node, relation(fn));
        } else if (consume_op("==")) {
            node = new_node(ND_EQ, node, relation(fn));
        } else {
            return node;
        }
    }
}

Node *relation(Function *fn){
    Node *node = add(fn);

    for (;;) {
        if (consume_op("<=")) {
            node = new_node(ND_LTE, node, add(fn));
        } else if (consume_op("<")) {
            node = new_node(ND_LT, node, add(fn));
        } else if (consume_op(">=")) {
            node = new_node(ND_GTE, node, add(fn));
        } else if (consume_op(">")) {
            node = new_node(ND_GT, node, add(fn));
        } else {
            return node;
        }
    }
}

Node *add(Function *fn) {
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

Node *mul(Function *fn) {
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

Node *unary(Function *fn) {
    if (consume_op("+"))
        return primary(fn);
    if (consume_op("-"))
        return new_node(ND_SUB, new_node_num(0), unary(fn));
    return primary(fn);
}

Node *primary(Function *fn) {
    if (consume_op("(")) {
        Node *node = expr(fn);
        expect(")");
        return node;
    }

    Token *tok = consume_indent();
    if (tok) {
        Node *node = calloc(1, sizeof(Node));
        if (strncmp(tok->next->str, "(", 1) != 0){
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
            if (consume_op(")") != true) {
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

