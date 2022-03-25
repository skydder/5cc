//#include <cstddef.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>

#include "5cc.h"

bool consume_op(char *op) {
    if (token->kind != TK_RESERVED || strlen(op) != token->len || memcmp(token->str, op, token->len)){
        return false;
    }
    token = token->next;
    return true;
}

Token *consume_indent() {
    if (token->kind != TK_IDENT) {
        return NULL;
    }
    return token;
}

bool consume_tk(TokenKind tk) {
    if (token->kind != tk) 
        return false;
    token = token->next;
    return true;
}

void expect(char *op) {
    if (token->kind != TK_RESERVED || strlen(op) != token->len || memcmp(token->str, op, token->len)){
        error_at(token->str, "'%s'ではありません", op);
    }
    token = token->next;
}

int expect_number() {
    if (token->kind != TK_NUM) 
	    error_at(token->str, "数ではありません");
    int val = token->val;
    token = token->next;
    return val;
}

bool at_eof() {
    return token->kind == TK_EOF;
}

int is_alnum(char c) {
    return ('a' <= c && c <= 'z') ||
            ('A' <= c && c <= 'Z') ||
            ('0' <= c && c <= '9') ||
            (c == '_');
}

Token *new_token(TokenKind kind, Token *cur, char *str) {
    Token *tok = calloc(1, sizeof(Token));
    tok->kind = kind;
    tok->str = str;
    cur->next = tok;
    return tok;
}

LVar *new_lvar(char *name, int len, LVar *next) {
    LVar *lvar = calloc(1, sizeof(LVar));
    lvar->next = next;
    lvar->name = name;
    lvar->len = len;
    lvar->offset = lvar->next->offset + 8;
    
    return lvar;
}

LVar *find_lvar(Token *tok) {
    for (LVar *var = locals; var->name; var = var->next)
        if (var->len == tok->len && memcmp(tok->str, var->name, var->len) == 0)
            return var;

    return NULL;
}
Token *tokenize(char *p) {
    Token head;
    head.next = NULL;
    Token *cur = &head;

    while (*p) {
        if (isspace(*p)) {
	        p++;
	        continue;
	    }

        if (strncmp(p, "<=", 2) == 0 || strncmp(p, ">=", 2) == 0 || strncmp(p, "!=", 2) == 0 || strncmp(p, "==", 2) == 0){
            cur = new_token(TK_RESERVED, cur, p);
            cur->len = 2;
            p += 2;
            continue;
        }

        if (*p == '+' || *p == '-'|| *p == '*' || *p =='/' || *p == '(' ||*p == ')' 
            || *p == '<' || *p == '>' || *p == ';' || *p == '=' || *p == '{' || *p == '}' || *p == ',') {
	        cur = new_token(TK_RESERVED, cur, p++);
            cur->len = 1;
	        continue;
	    }

        if (strncmp(p, "return", 6) == 0 && is_alnum(p[6]) == 0) {
            cur = new_token(TK_RETURN, cur, p);
            cur->len = 6;
            p += 6;
            continue;
        }
        if (strncmp(p, "while", 5) == 0 && is_alnum(p[5]) == 0) {
            cur = new_token(TK_WHILE, cur, p);
            cur->len = 5;
            p += 5;
            continue;
        }
        if (strncmp(p, "for", 3) == 0 && is_alnum(p[3]) == 0) {
            cur = new_token(TK_FOR, cur, p);
            cur->len = 3;
            p += 3;
            continue;
        }
        if (strncmp(p, "if", 2) == 0 && is_alnum(p[2]) == 0) {
            cur = new_token(TK_IF, cur, p);
            cur->len = 2;
            p += 2;
            continue;
        }
        if (strncmp(p, "else", 4) == 0 && is_alnum(p[4]) == 0) {
            cur = new_token(TK_ELSE, cur, p);
            cur->len = 4;
            p += 4;
            continue;
        }

        if ('a' <= *p && *p <= 'z' || 'A' <= *p && *p <= 'Z' || *p == '_') {
            int len = 1;
            for (; is_alnum(p[len]) == 1; len++) len += 0;
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