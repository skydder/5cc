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

Node *expr() {
    Node *node = mul();

    for (;;) {
        if (consume('+'))
            node = new_node(ND_ADD, node, mul());
        else  if (consume('-'))
            node = new_node(ND_SUB, node, mul());
        else
            return node;
    }
}

Node *mul() {
    Node *node = unary();

    for (;;) {
        if (consume('*'))
            node = new_node(ND_MUL, node, unary());
        else if (consume('/'))
            node = new_node(ND_DIV, node, unary());
        else
            return node;
    }
}

Node *unary() {
    if (consume('+'))
        return primary();
    if (consume('-'))
        return new_node(ND_SUB, new_node_num(0), primary());
    return primary();
}

Node *primary() {
    if (consume('(')) {
        Node *node = expr();
        expect(')');
        return node;
    }

    return new_node_num(expect_number());
}

void gen(Node *node) {
    if (node->kind == ND_NUM) {
        printf("\tpush %d\n", node->val);
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
    }

    printf("\tpush rax\n");
}