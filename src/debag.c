#include "5cc.h"
#include <stdio.h>

void p_tk(Token *tok) {
    printf("%d, %s\n", tok->len, tok->str);
}
void p_var(Var *var) {
    printf("%d, %s\n", var->len, var->name);
}
