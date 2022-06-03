#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdarg.h>
#include <ctype.h>
#include <string.h>

#include "5cc.h"


int main(int argc, char **argv) {
    if (argc != 2) {
        error("引数の個数が正しくありません");
        return 1;
    }
    
    gUserInput = argv[1];
    gToken = Tokenize(gUserInput);
    program();
    codegen();
    return 0;
}
