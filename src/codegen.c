#include <stddef.h>
#include <stdio.h>

#include "5cc.h"
#include "asm.h"

Obj *cur_fn;
static char *arg_reg[] = {"rdi", "rsi", "rdx", "rcx", "r8", "r9"};

static int align_to(int x, int align) {
    return (x + align -1) / align * align;
}

static int count(void) {
    static int i = 1;
    return i++;
}

static void load() {
    comment("load");
    pop("rax");
    mov("rax", "[rax]");
    push("rax");
    endl();
}
static void store() {
    comment("store");
    pop("rdi");
    pop("rax");
    mov("[rax]","rdi");
    push("rdi");
    endl();
}
static void gen(Node *node);
static void gen_addr(Node *node) {
    if (node->kind == ND_DEREF) {
        gen(node->lhs);
        return;
    }
    if (node->kind == ND_LVAR) {
        lea("rax", f("[rbp - %d]", node->offset)); // => lea rax, [rbp - (offset)]
        push("rax");
        endl();
        return;
    }
    error("代入の左辺値が変数ではありません");
}

static void gen(Node *node) {
    switch (node->kind) {
        case ND_NUM:
            comment("num");
            push(f("%d", node->val));
            return;
        case ND_RETURN:
            comment("return");
            gen(node->lhs);
            pop("rax");
            jmp(f(".L.%s.return", cur_fn->name));
            return;

        case ND_EXPR_STMT:
            comment("expr_stmt");
            gen(node->lhs);
            return;

        case ND_FOR:{
            int c = count();
            if (node->init)
                gen(node->init);
            label(f(".L.begin.%d", c));
            if (node->cond) {
                gen(node->cond);
                pop("rax");
                cmp("rax", "0");
                j(f(".L.end.%d", c), "e");
            }
            gen(node->then);
            if (node->inc)
                gen(node->inc);
            jmp(f(".L.begin.%d", c));
            label(f(".L.end.%d", c));
            return;
        }

        case ND_IF:{
            int c = count();
            gen(node->cond);
            pop("rax");
            cmp("rax", "0");
            j(f(".L.else.%d", c), "e");
            gen(node->then);
            jmp(f(".L.end.%d", c));
            label(f(".L.else.%d", c));
            if (node->els)
                gen(node->els);
            label(f(".L.end.%d", c));
            return;
        }

        case ND_WHILE: {
            int c = count();
            label(f(".L.begin.%d", c));
            gen(node->cond);
            pop("rax");
            cmp("rax", "0");
            j(f(".L.end.%d", c), "e");
            gen(node->then);
            jmp(f(".L.begin.%d", c));
            label(f(".L.end.%d", c));
            return;
        }

        case ND_BLOCK: {
            for (Node *i = node->body; i; i = i->next)
                gen(i);
            return;
        }

        case ND_NULL:
            return;
    
        case ND_LVAR:
            comment("lvar");
            gen_addr(node);
            load();
            return;
        case ND_ASSIGN:
            comment("assign");
            gen_addr(node->lhs);
            gen(node->rhs);
            store();
            return;
        case ND_ADDR:
            comment("addr");
            gen_addr(node->lhs);
            return;
        case ND_DEREF:
            comment("deref");
            gen(node->lhs);
            load();
            return;
        case ND_FUNCALL:
            comment("funcall");
            if (node->arg) {
                int j = 0;
                for (Node *i = node->arg; i != NULL; i = i->next) {
                    gen(i);
                    pop(f("%s", arg_reg[j]));
                    j++;
                }
            }
            int c = count();
            mov("rax", "rsp");
            
            printf("\ttest rax, 15\n");
            j(f(".L.call.%d", c), "nz");
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

    gen(node->lhs);
    gen(node->rhs);

    pop("rdi");
    pop("rax");
    endl();

    switch (node->kind){
        case ND_ADD:
            add("rax", "rdi");
            break;
        case ND_SUB:
            sub("rax", "rdi");
            break;
        // case ND_PTR_ADD:
        //     imul("rax", f("%d", SizeOfType(node->type->ptr_to)));
        //     add("rax", "rdi");
        //     break;
        // case ND_PTR_SUB:
        //     imul("rax", f("%d", SizeOfType(node->type->ptr_to)));
        //     sub("rax", "rdi");
        //     break;
        // case ND_PTR_DIFF:
        //     sub("rax", "rdi");
        //     idiv("rdi");
        //     break;;
        case ND_MUL:
            imul("rax", "rdi");
            break;
        case ND_DIV:
            idiv("rdi");
            break;
        case ND_LT://<
            CmpSetMov("rax", "rdi", "l");
            break;
        case ND_LTE://<=
            CmpSetMov("rax", "rdi", "le");
            break;
        case ND_GT://>
            CmpSetMov("rdi", "rax", "l");
            break;
        case ND_GTE://>=
            CmpSetMov("rdi", "rax", "le");
            break;
        case ND_EQ://==
            CmpSetMov("rax", "rdi", "e");
            break;
        case ND_NEQ://!=
            CmpSetMov("rax", "rdi", "ne");
            break;
    }
    endl();
    push("rax");
    endl();
}


static void gen_func(Obj *fn){
    cur_fn = fn;
    printf(".globl %s\n", fn->name);
    label(f("%s",fn->name));
    push("rbp");
    mov("rbp", "rsp");
    sub("rsp", f("%d", align_to(((Obj*)GetVecLast(fn->lvar))->offset, 16)));
    endl();
    for(Obj *var = (Obj*)PopVec(fn->param); var->name; var = (Obj*)PopVec(fn->lvar)) {
        mov("rax", "rbp");
        sub("rax", f("%d", var->offset));
        mov("[rax]", f("%s", arg_reg[var->offset/8-1]));
        push("rax");
        endl();
    }
    gen(fn->body);
    endl();
    label(f(".L.%s.return", fn->name));
    mov("rsp", "rbp");
    pop("rbp");
    ret();
    return;
}

void codegen() {
    printf(".intel_syntax noprefix\n");

    for (int i = 0; i < gFuncs->len; i++) {
        gen_func(gFuncs->data[i]);
    }

    return;
}