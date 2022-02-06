#ifndef _5cc_
#define _5cc_

//include part
#include <stdbool.h>
#include <stdarg.h>

//macro def

//type def
typedef enum {
    TK_RESERVED,
    TK_IDENT,
    TK_NUM,
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
} NodeKind;

typedef struct Node Node;
struct Node {
    NodeKind kind;
    Node *lhs;
    Node *rhs;
    int val;
    int offset;
};

//prototpye declaration
//-error
extern char *user_input;
void error_at(char *loc, char *fmt, ...);
void error(char *fmt, ...);

//-token
bool consume(char *op);
Token *consume_indent();
void expect(char *op);
int expect_number();
bool at_eof();
Token *new_token(TokenKind kind, Token *cur, char *str);
Token *tokenize(char *p);
extern Token *token;

//-parser
Node *new_node(NodeKind kind, Node *lhs, Node *rhs);
Node *new_node_num(int val);
extern Node *code[100];
void program();
Node *stmt();
Node *expr();
Node *assign();
Node *equal();
Node *relation();
Node *add();
Node *mul();
Node *unary();
Node *primary();
void gen_lval(Node *node);
void gen(Node *node);

#endif 