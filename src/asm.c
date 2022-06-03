#include "asm.h"
#include <stdarg.h>
#include <stdio.h>
#include <string.h>

extern inline void mov(char* dst, char* src);
extern inline void lea(char* dst, char* src);
extern inline void add(char* dst, char* src);
extern inline void sub(char* dst, char* src);
extern inline void imul(char* dst, char* src);
extern inline void cmp(char* dst, char* src);

extern inline void idiv(char* src);

extern inline void pop(char* dst);
extern inline void push(char* dst);
extern inline void call(char* dst);
extern inline void jmp(char* dst);
extern inline void ret();
extern inline void label(char* label);
extern inline void comment(char* str);
extern inline void CmpSetMov(char* dst, char* src,char* set_flag);
extern inline void j(char* label, char* jmp_flag);
extern inline void endl();

char* f(char* str, ...){
    char buf[2048];
    va_list ap;
    va_start(ap, str);
    vsnprintf(buf, sizeof(buf), str, ap);
    va_end(ap);
    return strdup(buf);
}
