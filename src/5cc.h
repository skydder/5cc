#ifndef _5cc_
#define _5cc_

#include <stdbool.h>
#include <stdarg.h>

//===================================================================
// util.c
//===================================================================
extern char *user_input;
void error_at(char *loc, char *fmt, ...);
void error(char *fmt, ...);
bool is_same(char *string, char *word);

//===================================================================
// token.c
//===================================================================
typedef enum {
    TK_SYMBOL,
    TK_IDENT,
    TK_NUM,
    TK_RETURN,
    TK_IF,
    TK_WHILE,
    TK_FOR,
    TK_ELSE,
    TK_INT,
    TK_EOF,
} TokenKind;

typedef struct Token Token;
struct Token {
    TokenKind kind;
    Token *next;
    int val;
    char *str;
    int len;
};

extern Token *token;
Token *tokenize(char *p);

//===================================================================
// type.c
//===================================================================
typedef struct Type Type;
struct Type {
    enum { INT, PTR } ty;
    struct Type *ptr_to;
};

//===================================================================
// parser.c
//===================================================================
typedef enum {
    ND_ADD,
    ND_SUB,
    ND_MUL,
    ND_DIV,
    ND_ASSIGN,
    ND_LVAR,
    ND_NUM,
    ND_LT,
    ND_GT,
    ND_LTE,
    ND_GTE,
    ND_EQ,
    ND_NEQ,
    ND_RETURN,
    ND_WHILE,
    ND_FOR,
    ND_IF,
    ND_EXPR_STMT,
    ND_BLOCK,
    ND_FUNCALL,
    ND_ADDR,
    ND_DEREF,
    ND_NULL,
} NodeKind;

typedef struct LVar LVar;
struct LVar {
    LVar *next;
    char *name;
    int len;
    int offset;
    Type *type;
};

typedef struct Node Node;
struct Node {
    NodeKind kind;

    Node *next;
    Node *body;
    Node *lhs;
    Node *rhs;

    Node *els;

    Node *init;
    Node *cond;
    Node *inc;
    Node *then;

    int val;
    int offset;

    char *fn_name;
    Node *arg;

    Type *type;
};

typedef struct Function Function;
struct Function {
    char *name;
    Node *body;
    LVar *locals;
    Function *next;
    LVar *args;
    Type *type;

};

extern Function *code[100];
void program();
extern Function *fnc;

//===================================================================
// codegen.c
//===================================================================
void codegen();

#endif
