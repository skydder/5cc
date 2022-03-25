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

Node *code[100];

void program() {
    int i = 0;
    while (!at_eof()) {
        code[i++] = stmt();
    }
    code[i] = NULL;
}

Node *stmt() {
    Node *node;

    if (consume_tk(TK_RETURN)) {
        node = calloc(1, sizeof(Node));
        node->kind = ND_RETURN;
        node->lhs = expr();
        expect(";");
        return node;
    }

    if (consume_tk(TK_WHILE)) {
        node = new_n_kind(ND_WHILE);
        
        expect("(");
        node->cond = expr();
        expect(")");
        node->then = stmt();
        return node;
    }

    if (consume_tk(TK_FOR)) {
        node = new_n_kind(ND_FOR);
    
        expect("(");

        if(consume_op(";") != true)
            node->init = expr_stmt();

        if (consume_op(";") != true) {
            node->cond = expr();
            expect(";");
        }
        if (consume_op(")") != true) {
            node->inc = expr();
            expect(")");
        }
        node->then = stmt();
        return node;
    }

    if (consume_tk(TK_IF)) {
        node = new_n_kind(ND_IF);
        
        expect("(");
        node->cond = expr();
        expect(")");
        node->then = stmt();
        if (!consume_tk(TK_ELSE)) 
            node->els = stmt();
        return node;
    }
    if (consume_op("{")) {
        return block();
    }
    
    return expr_stmt();
    
}

Node *expr_stmt() {
    Node *node;
    node = calloc(1, sizeof(Node));
    node->kind = ND_EXPR_STMT;
    node->lhs = expr();
    expect(";");
    return node;
}
Node *block() {
    Node tmp = {};
    Node *cur = &tmp;
    while (consume_op("}") != true) 
        cur = cur->next = stmt();
    Node *node = new_n_kind(ND_BLOCK);
    node->body = tmp.next;
    return node;
}

Node *expr(){
    return assign();
}

Node *assign() {
    Node *node = equal();
    if (consume_op("=")) {
        node = new_node(ND_ASSIGN, node, assign());
    }
    return node;
}

Node *equal(){
    Node *node = relation();

    for (;;) {
        if (consume_op("!=")) {
            node = new_node(ND_NEQ, node, relation());
        } else if (consume_op("==")) {
            node = new_node(ND_EQ, node, relation());
        } else {
            return node;
        }
    }
}

Node *relation(){
    Node *node = add();

    for (;;) {
        if (consume_op("<=")) {
            node = new_node(ND_LTE, node, add());
        } else if (consume_op("<")) {
            node = new_node(ND_LT, node, add());
        } else if (consume_op(">=")) {
            node = new_node(ND_GTE, node, add());
        } else if (consume_op(">")) {
            node = new_node(ND_GT, node, add());
        } else {
            return node;
        }
    }
}

Node *add() {
    Node *node = mul();

    for (;;) {
        if (consume_op("+"))
            node = new_node(ND_ADD, node, mul());
        else  if (consume_op("-"))
            node = new_node(ND_SUB, node, mul());
        else
            return node;
    }
}

Node *mul() {
    Node *node = unary();

    for (;;) {
        if (consume_op("*"))
            node = new_node(ND_MUL, node, unary());
        else if (consume_op("/"))
            node = new_node(ND_DIV, node, unary());
        else
            return node;
    }
}

Node *unary() {
    if (consume_op("+"))
        return primary();
    if (consume_op("-"))
        return new_node(ND_SUB, new_node_num(0), unary());
    return primary();
}

Node *primary() {
    if (consume_op("(")) {
        Node *node = expr();
        expect(")");
        return node;
    }

    Token *tok = consume_indent();
    if (tok) {
        Node *node = calloc(1, sizeof(Node));
        if (strncmp(tok->next->str, "(", 1) != 0){
            node->kind = ND_LVAR;
        
            LVar *lvar = find_lvar(tok);
            if (lvar) {
                node->offset = lvar->offset;
            } else {
                lvar = new_lvar(tok->str, tok->len, locals);
                node->offset = lvar->offset;
                locals = lvar;
            }
            token = token->next;
            return node;
        } else {
            node->kind = ND_FUNCALL;
            node->fn_name = strndup(tok->str, tok->len); //strncpy(tok->str, tok->len);
            token = token->next;
            expect("(");
            if (consume_op(")") != true) {
                node->arg = expr();
                for (Node *i = node->arg; !consume_op(")"); i = i->next){
                    expect(",");
                    i->next = expr();
                }
                //expect(")");
            }
            return node;
        }
    }
    return new_node_num(expect_number());
}

