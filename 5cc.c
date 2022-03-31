//#include <cstddef>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdarg.h>
#include <ctype.h>
#include <string.h>

#include "5cc.h"

Token *token;

char *user_input;
Function *code[100];

int main(int argc, char **argv) {
    if (argc != 2) {
        error("引数の個数が正しくありません");
        return 1;
    }
    
    user_input = argv[1];
    token = tokenize(user_input);

    program();
    codegen();
    return 0;

}
