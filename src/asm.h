#ifndef _asm_
#define _asm_

#include <stdio.h>

/* 5cc won't support "inline"
 * so I'll change the code below someday.
*/

// => dst = *src
inline void lea(char* dst, char* src) {
    printf("\tlea %s, %s\n", dst, src);
}

// => dst += src
inline void add(char* dst, char* src) {
    printf("\tadd %s, %s\n", dst, src);
}

// => rax /= dst
// => rdx = rax % dst
inline void idiv(char* src) {
    printf("\tcqo\n");
    printf("\tidiv %s\n", src);
}

inline void mov(char* dst, char* src) {
    printf("\tmov %s, %s\n", dst, src);
}
inline void sub(char* dst, char* src) {
    printf("\tsub %s, %s\n", dst, src);
}
inline void imul(char* dst, char* src) {
    printf("\timul %s, %s\n", dst, src);
}
inline void cmp(char* dst, char* src) {
    printf("\tcmp %s, %s\n", dst, src);
}
inline void pop(char* dst) {
    printf("\tpop %s\n", dst);
}
inline void push(char* dst) {
    printf("\tpush %s\n", dst);
}
inline void call(char* dst) {
    printf("\tcall %s\n", dst);
}
inline void jmp(char* dst) {
    printf("\tjmp %s\n", dst);
}

inline void ret() {
    printf("\tret\n");
}
inline void label(char* label) {
    printf("%s:\n",label);
}
inline void comment(char* str) {
    printf("#%s\n",str);
}
inline void CmpSetMov(char* dst, char* src,char* set_flag) {
    cmp(dst, src);
    printf("\tset%s al\n", set_flag);
    printf("\tmovzb rax, al\n");
    return;
}
inline void j(char* label, char* jmp_flag) {
    printf("\tj%s %s\n", jmp_flag, label);
}
inline void endl() {
    printf("\n");
}
char* f(char* str, ...);

#endif