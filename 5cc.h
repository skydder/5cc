#ifndef _5cc_
#define _5cc_

//include part
#include <stdbool.h>
#include <stdarg.h>

//macro def

//type def
typedef enum {
    TK_RESERVED,
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
    ND_NUM,
} NodeKind;

typedef struct Node Node;
struct Node {
    NodeKind kind;
    Node *lhs;
    Node *rhs;
    int val;
};

//prototpye declaration
//-error
extern char *user_input;
void error_at(char *loc, char *fmt, ...);
void error(char *fmt, ...);

//-tokenizer
bool consume(char op);
void expect(char op);
int expect_number();
bool at_eof();
Token *new_token(TokenKind kind, Token *cur, char *str);
Token *tokenize(char *p);
extern Token *token;

//-parser
Node *new_node(NodeKind kind, Node *lhs, Node *rhs);
Node *new_node_num(int val);
Node *expr();
Node *mul();
Node *unary();
Node *primary();
void gen(Node *node);

#endif 