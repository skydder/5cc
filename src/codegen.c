#include "5cc.h"
#include <stddef.h>
#include <stdio.h>

static char *arg_reg[] = {"rdi", "rsi", "rdx", "rcx", "r8", "r9"};

static int align_to(int x, int align) {
    return (x + align -1) / align * align;
}

static int count(void) {
    static int i = 1;
    return i++;
}

static void gen_lval(Node *node) {
    if (node->kind != ND_LVAR) {
        error("代入の左辺値が変数ではありません");
    }

    //printf("\tmov rax, rbp\n");
    //printf("\tsub rax, %d\n", node->offset);
    //learax, [rbp-4]
    printf("\tlea rax, [rbp - %d]\n", node->offset);
    printf("\tpush rax\n\n");
}

static void gen_expr(Node *node) {
    switch (node->kind) {
        case ND_NUM:
            printf("\tpush %d\n\n", node->val);
            return;
        case ND_LVAR:
            gen_lval(node);
            printf("\tpop rax\n");
            printf("\tmov rax, [rax]\n");
            printf("\tpush rax\n\n");
            return;
        case ND_ASSIGN:
            gen_lval(node->lhs);
            gen_expr(node->rhs);

            printf("\tpop rdi\n");
            printf("\tpop rax\n");
            printf("\tmov [rax], rdi\n");
            printf("\tpush rdi\n\n");
            return;
        case ND_ADDR:
            gen_lval(node->lhs);
            return;
        case ND_DEREF:
            printf("\tpop rax\n");
            printf("\tmov rax, [rax]\n");
            printf("\tpush rax\n\n");
            return;
        case ND_FUNCALL:
            if (node->arg) {
                int j = 0;
                for (Node *i = node->arg; i != NULL; i = i->next) {
                    gen_expr(i);
                    printf("\tpop %s\n\n", arg_reg[j]);
                    j++;
                }
            }
            int c = count();
            printf("\tmov rax, rsp\n");
            //printf("\tand rax, 15\n");
            printf("\ttest rax, 15\n");
            printf("\tjnz .L.call.%d\n", c);
            printf("\tmov rax, 0\n");
            printf("\tcall %s\n", node->fn_name);
            printf("\tjmp .L.end.%d\n", c);
            printf(".L.call.%d:\n", c);
            printf("\tsub rsp, 8\n");
            printf("\tmov rax, 0\n");
            printf("\tcall %s\n", node->fn_name);
            printf("\tadd rsp, 8\n");
            printf(".L.end.%d:\n", c);
            printf("\tpush rax\n\n");
            
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

    printf("\tpush rax\n\n");
}

static void gen_stmt(Node *node, char *name) {
    switch (node->kind) {
        case ND_RETURN:
            gen_expr(node->lhs);
            printf("\tpop rax\n");
            printf("\tjmp .L.%s.return\n", name);
            return;

        case ND_EXPR_STMT:
            gen_expr(node->lhs);
            return;

        case ND_FOR:{
            int c = count();
            if (node->init)
                gen_expr(node->init);
            printf(".L.begin.%d:\n", c);
            if (node->cond) {
                gen_expr(node->cond);
                printf("\tpop rax\n");
                printf("\tcmp rax, 0\n");
                printf("\t je .L.end.%d\n", c);
            }
            gen_stmt(node->then, name);
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
            gen_stmt(node->then, name);
            printf("\tjmp .L.end.%d\n", c);
            printf(".L.else.%d:\n", c);
            if (node->els)
                gen_stmt(node->els, name);
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
            gen_stmt(node->then, name);
            printf("\tjmp .L.begin.%d\n", c);
            printf(".L.end.%d:\n", c);
            return;
        }

        case ND_BLOCK: {
            for (Node *i = node->body; i; i = i->next)
                gen_stmt(i, name);
        }

        case ND_NULL_STMT:
            return ;
    }
}
static void gen_func(Function *fn){
    printf(".globl %s\n", fn->name);
    printf("%s:\n",fn->name);
    printf("\tpush rbp\n");
    printf("\tmov rbp, rsp\n");
    
    printf("\tsub rsp, %d\n\n", align_to(fn->locals->offset, 16));
    for (LVar *i = fn->args; i->name ; i = i->next) {
        printf("\tmov rax, rbp\n");
        printf("\tsub rax, %d\n", i->offset);
        printf("\tmov [rax], %s\n", arg_reg[i->offset/8-1]);
        printf("\tpush rax\n\n");
    }
    gen_stmt(fn->body, fn->name);
    printf("\n");
    printf(".L.%s.return:\n",fn->name);
    printf("\tmov rsp, rbp\n");
    printf("\tpop rbp\n");
    printf("\tret\n\n");
    
}
void codegen() {
    printf(".intel_syntax noprefix\n");

    for (int i = 0; code[i]; i++) {
        gen_func(code[i]);
    }

    return;
}