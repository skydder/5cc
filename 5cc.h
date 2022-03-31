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
    TK_RETURN,
    TK_IF,
    TK_WHILE,
    TK_FOR,
    TK_ELSE,
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
    ND_RETURN,
    ND_WHILE,
    ND_FOR,
    ND_IF,
    ND_EXPR_STMT,
    ND_BLOCK,
    ND_FUNCALL,
    //ND_FUNC,
} NodeKind;

typedef struct LVar LVar;

struct LVar {
    LVar *next;
    char *name;
    int len;
    int offset;
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
};

typedef struct Function Function;

struct Function {
    char *name;
    Node *body;
    LVar *locals;
    Function *next;
    int arg;
    //LVar *args; 

};

//prototpye declaration
//-error
extern char *user_input;
void error_at(char *loc, char *fmt, ...);
void error(char *fmt, ...);

//-token
bool consume_op(char *op);
Token *consume_indent();
bool consume_tk(TokenKind tk);
void expect(char *op);
int expect_number();
bool at_eof();
LVar *new_lvar(char *name, int len, LVar *next);
LVar *find_lvar(Token *tok, Function *fn);
Token *new_token(TokenKind kind, Token *cur, char *str);
Token *tokenize(char *p);
extern Token *token;
extern LVar *locals;

//-parser
Node *new_node(NodeKind kind, Node *lhs, Node *rhs);
Node *new_node_num(int val);
extern Function *code[100];
void program();
extern Function *fnc;
Function *func();
Node *stmt(Function *fn);
Node *block(Function *fn);
Node *expr_stmt(Function *fn);
Node *expr(Function *fn);
Node *assign(Function *fn);
Node *equal(Function *fn);
Node *relation(Function *fn);
Node *add(Function *fn);
Node *mul(Function *fn);
Node *unary(Function *fn);
Node *primary(Function *fn);

//void gen_lval(Node *node);
//void gen(Node *node);
void codegen();

#endif 
