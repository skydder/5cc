#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>

#include "5cc.h"

static bool is_alnum(char c) {
    return ('a' <= c && c <= 'z') ||
           ('A' <= c && c <= 'Z') ||
           ('0' <= c && c <= '9') ||
           (c == '_');
}

static bool is_reserved(char *string, char *reserved) {
    return (is_same(string, reserved) && !is_alnum(string[strlen(reserved)]));
}

Token *gToken;

static struct {
    TokenKind tk;
    char* keyword;
    int len;
} KW[] = {{TK_RETURN, "return", 6}, {TK_INT, "int", 3},
          {TK_SIZEOF, "sizeof", 6}, {TK_FOR, "for", 3},
          {TK_WHILE, "while", 5} ,{TK_ELSE, "else", 4},
          {TK_IF, "if", 2},
          {0,NULL, 0}};

static struct {
    char* symbol;
    int len;
} Symbol[] = {{"<=", 2}, {">=", 2}, {"!=", 2}, {"==", 2},
              {"+", 1}, {"=", 1}, {"-", 1}, {"/", 1},
              {"*", 1}, {"{", 1}, {"}", 1}, {"&", 1},
              {",", 1}, {"(", 1}, {")", 1}, {";", 1},
              {"<", 1}, {">", 1}, {"[", 1}, {"]", 1},
              {NULL, 0}};

static Token *tk_reserved(char **p, Token* cur) {
    Token* tok = NULL;
    for (int i = 0; KW[i].keyword != NULL; i++) {
        if (is_reserved(*p, KW[i].keyword)) {
            tok = NewToken(KW[i].tk, cur, *p, KW[i].len);
            *p += KW[i].len;
            return tok;
        }
    }
    for (int i = 0; Symbol[i].symbol != NULL; i++) {
        if (is_same(*p, Symbol[i].symbol)) {
            tok = NewToken(TK_SYMBOL, cur, *p, Symbol[i].len);
            *p += Symbol[i].len;
            return tok;
        }
    }
    return tok;
}

Token *Tokenize(char *p) {
    Token head;
    head.next = NULL;
    Token *cur = &head;

    while (*p) {
        if (isspace(*p)) {
	        p++;
	        continue;
	    }

        Token* reserved = tk_reserved(&p, cur);
        if (reserved != NULL) {
            cur = reserved;
            continue;
        }

        if ('a' <= *p && *p <= 'z' || 'A' <= *p && *p <= 'Z' || *p == '_') {
            int len = 1;
            for (; is_alnum(p[len]); len++) ;
            cur = NewToken(TK_IDENT, cur, p, len);
            p += len;
            continue;
        }

	    if (isdigit(*p)) {
	        cur = NewToken(TK_NUM, cur, p, 0);
	        cur->val = strtol(p, &p, 10);
	        continue;
	    }

    	error_at(cur->str, "トークナイズトークナイズできませんできません");
    }

    NewToken(TK_EOF, cur, p, 0);
    return head.next;
}