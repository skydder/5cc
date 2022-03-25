#include "5cc.h"
#include <stddef.h>
#include <stdio.h>

static void arg(int i) {
    switch (i) {
        case 1:
            printf("\tpop rax\n");
            printf("\tmov rdi, rax\n");
            return;
        case 2:
            printf("\tpop rax\n");
            printf("\tmov rsi, rax\n");
            return;
        case 3:
            printf("\tpop rax\n");
            printf("\tmov rdx, rax\n");
            return;
        case 4:
            printf("\tpop rax\n");
            printf("\tmov rcx, rax\n");
            return;
        case 5:
            printf("\tpop rax\n");
            printf("\tmov r8, rax\n");
            return;
        
    }
}

static int count(void) {
    static int i = 1;
    return i++;
}

void gen_lval(Node *node) {
    if (node->kind != ND_LVAR) {
        error("代入の左辺値が変数ではありません");
    }

    printf("\tmov rax, rbp\n");
    printf("\tsub rax, %d\n", node->offset);
    printf("\tpush rax\n");
}

void gen_expr(Node *node) {
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
            gen_expr(node->rhs);

            printf("\tpop rdi\n");
            printf("\tpop rax\n");
            printf("\tmov [rax], rdi\n");
            printf("\tpush rdi\n");
            return;
        case ND_FUNCALL:
            printf("\tmov rax, 0\n");
            if (node->arg) {
                int j = 1;
                for (Node *i = node->arg; i != NULL; i = i->next) {
                    gen_expr(i);
                    arg(j);
                    j++;
                }
            }
            printf("\tcall %s\n", node->fn_name);
            return;
    }

    gen_expr(node->lhs);
    gen_expr(node->rhs);

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

void gen_stmt(Node *node) {
    switch (node->kind) {
    case ND_RETURN:
        gen_expr(node->lhs);
        printf("\tpop rax\n");
        printf("\tjmp .L.main.return\n");
        return;

    case ND_EXPR_STMT:
        gen_expr(node->lhs);
        return;

    case ND_FOR:{
        int c = count();
        if (node->init)
            gen_stmt(node->init);
        printf(".L.begin.%d:\n", c);
        if (node->cond) {
            gen_expr(node->cond);
            printf("\tpop rax\n");
            printf("\tcmp rax, 0\n");
            printf("\t je .L.end.%d\n", c);
        }
        gen_stmt(node->then);
        if (node->inc)
            gen_expr(node->inc);
        printf("\tjmp .L.begin.%d\n", c);
        printf(".L.end.%d:\n", c);
        return;
    }

    case ND_IF:{
        int c = count();
        gen_expr(node->cond);
        printf("\tpop rax\n");
        printf("\tcmp rax, 0\n");
        printf("\tje  .L.else.%d\n", c);
        gen_stmt(node->then);
        printf("\tjmp .L.end.%d\n", c);
        printf(".L.else.%d:\n", c);
        if (node->els)
            gen_stmt(node->els);
        printf(".L.end.%d:\n", c);
        return;
    }

    case ND_WHILE: {
        int c = count();
        printf(".L.begin.%d:\n", c);
        gen_expr(node->cond);
        printf("\tpop rax\n");
        printf("\tcmp rax, 0\n");
        printf("\tje  .L.end.%d\n", c);
        gen_stmt(node->then);
        printf("\tjmp .L.begin.%d\n", c);
        printf(".L.end.%d:\n", c);
        return;
    }

    case ND_BLOCK: {
        for (Node *i = node->body; i; i = i->next)
            gen_stmt(i);
    }
    }
}

void codegen() {
    printf(".intel_syntax noprefix\n");
    printf(".globl main\n");
    printf("main:\n");

    printf("\tpush rbp\n");
    printf("\tmov rbp, rsp\n");
    printf("\tsub rsp, %d\n", locals->offset);

    for (int i = 0; code[i]; i++) {
        gen_stmt(code[i]);
    }

    printf(".L.main.return:\n");
    printf("\tmov rsp, rbp\n");
    printf("\tpop rbp\n");
    printf("\tret\n");
    return;
}