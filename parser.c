#include <stdlib.h>
#include <stdio.h>

#include "5cc.h"

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
    Node *node = expr();
    expect(";");
    return node;
}

Node *expr(){
    return assign();
}

Node *assign() {
    Node *node = equal();
    if (consume("=")) {
        node = new_node(ND_ASSIGN, node, assign());
    }
    return node;
}

Node *equal(){
    Node *node = relation();

    for (;;) {
        if (consume("!=")) {
            node = new_node(ND_NEQ, node, relation());
        } else if (consume("==")) {
            node = new_node(ND_EQ, node, relation());
        } else {
            return node;
        }
    }
}

Node *relation(){
    Node *node = add();

    for (;;) {
        if (consume("<=")) {
            node = new_node(ND_LTE, node, add());
        } else if (consume("<")) {
            node = new_node(ND_LT, node, add());
        } else if (consume(">=")) {
            node = new_node(ND_GTE, node, add());
        } else if (consume(">")) {
            node = new_node(ND_GT, node, add());
        } else {
            return node;
        }
    }
}

Node *add() {
    Node *node = mul();

    for (;;) {
        if (consume("+"))
            node = new_node(ND_ADD, node, mul());
        else  if (consume("-"))
            node = new_node(ND_SUB, node, mul());
        else
            return node;
    }
}

Node *mul() {
    Node *node = unary();

    for (;;) {
        if (consume("*"))
            node = new_node(ND_MUL, node, unary());
        else if (consume("/"))
            node = new_node(ND_DIV, node, unary());
        else
            return node;
    }
}

Node *unary() {
    if (consume("+"))
        return primary();
    if (consume("-"))
        return new_node(ND_SUB, new_node_num(0), primary());
    return primary();
}

Node *primary() {
    if (consume("(")) {
        Node *node = expr();
        expect(")");
        return node;
    }

    Token *tok = consume_indent();
    if (tok) {
        Node *node = calloc(1, sizeof(Node));
        node->kind = ND_LVAR;
        node->offset = (tok->str[0] - 'a' + 1) * 8;
        token = token->next;
        return node;
    }
    return new_node_num(expect_number());
}

void gen_lval(Node *node) {
    if (node->kind != ND_LVAR) {
        error("代入の左辺値が変数ではありません");
    }

    printf("\tmov rax, rbp\n");
    printf("\tsub rax, %d\n", node->offset);
    printf("\tpush rax\n");
}

void gen(Node *node) {
    //if (node->kind == ND_NUM) {printf("\tpush %d\n", node->val);return;}
    switch (node->kind) {
        case ND_NUM:
            printf("\tpush %d\n", node->val);
            return;
        case ND_LVAR:
            gen_lval(node);
            printf("\tpop rax\n");
            printf("\tmov rax, [rax]\n");
            printf("\tpush rax\n");
            return;
        case ND_ASSIGN:
            gen_lval(node->lhs);
            gen(node->rhs);

            printf("\tpop rdi\n");
            printf("\tpop rax\n");
            printf("\tmov [rax], rdi\n");
            printf("\tpush rdi\n");
            return;
    }

    gen(node->lhs);
    gen(node->rhs);

    printf("\tpop rdi\n");
    printf("\tpop rax\n");

    switch (node->kind){
        case ND_ADD:
            printf("\tadd rax, rdi\n");
            break;
        case ND_SUB:
            printf("\tsub rax, rdi\n");
            break;
        case ND_MUL:
            printf("\timul rax, rdi\n");
            break;
        case ND_DIV:
            printf("\tcqo\n");
            printf("\tidiv rdi\n");
            break;
        case ND_LT://<
            printf("\tcmp rax, rdi\n");
            printf("\tsetl al\n");
            printf("\tmovzb rax, al\n");
            break;
        case ND_LTE://<=
            printf("\tcmp rax, rdi\n");
            printf("\tsetle al\n");
            printf("\tmovzb rax, al\n");
            break;
        case ND_GT://>
            printf("\tcmp rdi, rax\n");
            printf("\tsetl al\n");
            printf("\tmovzb rax, al\n");
            break;
        case ND_GTE://>=
            printf("\tcmp rdi, rax\n");
            printf("\tsetle al\n");
            printf("\tmovzb rax, al\n");
            break;
        case ND_EQ://==
            printf("\tcmp rax, rdi\n");
            printf("\tsete al\n");
            printf("\tmovzb rax, al\n");
            break;
        case ND_NEQ://!=
            printf("\tcmp rax, rdi\n");
            printf("\tsetne al\n");
            printf("\tmovzb rax, al\n");
            break;
    }

    printf("\tpush rax\n");
}