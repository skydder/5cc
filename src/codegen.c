#include "5cc.h"
#include "asm.h"
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

static void load() {
    pop("rax");
    mov("rax", "[rax]");
    push("rax");
    endl();
}
static void store() {
    pop("rdi");
    pop("rax");
    mov("[rax]","rdi");
    push("rdi");
    endl();
}
static void gen_addr(Node *node) {
    if (node->kind != ND_LVAR) {
        error("代入の左辺値が変数ではありません");
    }

    lea("rax", f("[rbp - %d]", node->offset)); // => lea rax, [rbp - (offset)]
    push("rax");
    endl();
}

static void gen_expr(Node *node) {
    switch (node->kind) {
        case ND_NUM:
            push(f("%d", node->val));
            return;
        case ND_LVAR:
            gen_addr(node);
            load();
            return;
        case ND_ASSIGN:
            gen_addr(node->lhs);
            gen_expr(node->rhs);
            store();
            return;
        case ND_ADDR:
            gen_addr(node->lhs);
            return;
        case ND_DEREF:
            gen_expr(node->lhs);
            load();
            return;
        case ND_FUNCALL:
            if (node->arg) {
                int j = 0;
                for (Node *i = node->arg; i != NULL; i = i->next) {
                    gen_expr(i);
                    pop(f("%s", arg_reg[j]));
                    j++;
                }
            }
            int c = count();
            mov("rax", "rsp");
            
            //printf("\tand rax, 15\n");
            printf("\ttest rax, 15\n");
            printf("\tjnz .L.call.%d\n", c);
            mov("rax", "0");
            call(f("%s", node->fn_name));
            jmp(f(".L.end.%d", c));
            label(f(".L.call.%d", c));
            sub("rsp", "8");
            mov("rax", "0");
            call(f("%s", node->fn_name));
            add("rsp", "8");
            label(f(".L.end.%d", c));
            push("rax");
            
            return;
    }

    gen_expr(node->lhs);
    gen_expr(node->rhs);

    pop("rdi");
    pop("rax");

    switch (node->kind){
        case ND_ADD:
            add("rax", "rdi");
            break;
        case ND_SUB:
            sub("rax", "rdi");
            break;
        case ND_MUL:
            imul("rax", "rdi");
            break;
        case ND_DIV:
            idiv("rdi");
            break;
        case ND_LT://<
            cmp("rax", "rdi");
            printf("\tsetl al\n");
            printf("\tmovzb rax, al\n");
            break;
        case ND_LTE://<=
            cmp("rax", "rdi");
            printf("\tsetle al\n");
            printf("\tmovzb rax, al\n");
            break;
        case ND_GT://>
            cmp("rdi", "rax");
            printf("\tsetl al\n");
            printf("\tmovzb rax, al\n");
            break;
        case ND_GTE://>=
            cmp("rdi", "rax");
            printf("\tsetle al\n");
            printf("\tmovzb rax, al\n");
            break;
        case ND_EQ://==
            cmp("rax", "rdi");
            printf("\tsete al\n");
            printf("\tmovzb rax, al\n");
            break;
        case ND_NEQ://!=
            cmp("rax", "rdi");
            printf("\tsetne al\n");
            printf("\tmovzb rax, al\n");
            break;
    }

    push("rax");
}

static void gen_stmt(Node *node, char *name) {
    switch (node->kind) {
        case ND_RETURN:
            gen_expr(node->lhs);
            pop("rax");
            jmp(f(".L.%s.return", name));
            return;

        case ND_EXPR_STMT:
            gen_expr(node->lhs);
            return;

        case ND_FOR:{
            int c = count();
            if (node->init)
                gen_expr(node->init);
            label(f(".L.begin.%d", c));
            if (node->cond) {
                gen_expr(node->cond);
                pop("rax");
                cmp("rax", "0");
                printf("\t je .L.end.%d\n", c);
            }
            gen_stmt(node->then, name);
            if (node->inc)
                gen_expr(node->inc);
            jmp(f(".L.begin.%d", c));
            label(f(".L.end.%d", c));
            return;
        }

        case ND_IF:{
            int c = count();
            gen_expr(node->cond);
            pop("rax");
            cmp("rax", "0");
            printf("\tje  .L.else.%d\n", c);
            gen_stmt(node->then, name);
            jmp(f(".L.end.%d", c));
            label(f(".L.else.%d", c));
            if (node->els)
                gen_stmt(node->els, name);
            label(f(".L.end.%d", c));
            return;
        }

        case ND_WHILE: {
            int c = count();
            label(f(".L.begin.%d", c));
            gen_expr(node->cond);
            pop("rax");
            cmp("rax", "0");
            printf("\tje  .L.end.%d\n", c);
            gen_stmt(node->then, name);
            jmp(f(".L.begin.%d", c));
            label(f(".L.end.%d", c));
            return;
        }

        case ND_BLOCK: {
            for (Node *i = node->body; i; i = i->next)
                gen_stmt(i, name);
            return;
        }

        case ND_NULL:
            return;
    }
}
static void gen_func(Function *fn){
    printf(".globl %s\n", fn->name);
    label(f("%s",fn->name));
    push("rbp");
    mov("rbp", "rsp");
    
    sub("rsp", f("%d", align_to(fn->locals->offset, 16)));
    
    for (LVar *i = fn->args; i->name ; i = i->next) {
        mov("rax", "rbp");
        sub("rax", f("%d", i->offset));
        mov("[rax]", f("%s", arg_reg[i->offset/8-1]));
        push("rax");
    }
    gen_stmt(fn->body, fn->name);
    endl();
    label(f(".L.%s.return", fn->name));
    mov("rsp", "rbp");
    pop("rbp");
    ret();
    return;
}
void codegen() {
    printf(".intel_syntax noprefix\n");

    for (int i = 0; code[i]; i++) {
        gen_func(code[i]);
    }

    return;
}