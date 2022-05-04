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

Token *token;


Token *tokenize(char *p) {
    Token head;
    head.next = NULL;
    Token *cur = &head;

    while (*p) {
        if (isspace(*p)) {
	        p++;
	        continue;
	    }

        if (is_same(p, "<=") || is_same(p, ">=") || is_same(p, "!=") || is_same(p, "==")){
            cur = new_token(TK_SYMBOL, cur, p);
            cur->len = 2;
            p += 2;
            continue;
        }

        if (*p == '+' || *p == '-'|| *p == '*' || *p =='/' || *p == '(' ||*p == ')' || *p == '&'
            || *p == '<' || *p == '>' || *p == ';' || *p == '=' || *p == '{' || *p == '}' || *p == ',') {
	        cur = new_token(TK_SYMBOL, cur, p++);
            cur->len = 1;
	        continue;
	    }

        if (is_reserved(p, "return")) {
            cur = new_token(TK_RETURN, cur, p);
            cur->len = 6;
            p += 6;
            continue;
        }
        
        if (is_reserved(p, "while")) {
            cur = new_token(TK_WHILE, cur, p);
            cur->len = 5;
            p += 5;
            continue;
        }

        if (is_reserved(p, "for")) {
            cur = new_token(TK_FOR, cur, p);
            cur->len = 3;
            p += 3;
            continue;
        }

        if (is_reserved(p, "if")) {
            cur = new_token(TK_IF, cur, p);
            cur->len = 2;
            p += 2;
            continue;
        }

        if (is_reserved(p, "else")) {
            cur = new_token(TK_ELSE, cur, p);
            cur->len = 4;
            p += 4;
            continue;
        }

        if (is_reserved(p, "int")) {
            cur = new_token(TK_INT, cur, p);
            cur->len = 3;
            p += 3;
            continue;
        }

        if ('a' <= *p && *p <= 'z' || 'A' <= *p && *p <= 'Z' || *p == '_') {
            int len = 1;
            for (; is_alnum(p[len]); len++) ;
            cur = new_token(TK_IDENT, cur, p);
            cur->len = len;
            p += len;
            continue;
        }

	    if (isdigit(*p)) {
	        cur = new_token(TK_NUM, cur, p);
	        cur->val = strtol(p, &p, 10);
	        continue;
	    }

    	error_at(cur->str, "トークナイズトークナイズできませんできません");
    }

    new_token(TK_EOF, cur, p);
    return head.next;
}