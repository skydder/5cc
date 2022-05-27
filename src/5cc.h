#ifndef _5cc_
#define _5cc_

#include <stdbool.h>
#include <stdarg.h>

//===================================================================
// type definitions
//===================================================================

typedef struct {
    void **data;
    int len;
    int capacity;
} vector;

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
    TK_SIZEOF,
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
typedef struct Type Type;

struct Type {
    enum { INT, PTR, ARRAY } ty;
    struct Type *ptr_to;
    int size;
    int array_size;
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
    ND_ADDR,
    ND_DEREF,
    ND_NULL,
} NodeKind;

typedef struct {
    char *name;
    int len;
    Type *type;
    int offset;
} Var;


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
    Var *lvar;
    char *fn_name;
    Node *arg;

    Type *type;
};

typedef struct Function Function;
struct Function {
    char *name;
    Node *body;
    
    vector *lvar;
    vector *param;
    Function *next;
    
    Type *type;
};

//===================================================================
// util.c
//===================================================================
extern char *user_input;
void error_at(char *loc, char *fmt, ...);
void error(char *fmt, ...);
bool is_same(char *string, char *word);

vector *NewVec();
void PushVec(vector *vec, void *item);
void *PopVec(vector *vec);
void *GetVecLast(vector *vec);
bool ContainsVec(vector *vec, void *item);
void *GetVecAt(vector *vec, int index);
//===================================================================
// structs.c
//===================================================================
Node *NewNode(NodeKind kind);
Node *NewNodeBinary(NodeKind kind, Node *lhs, Node *rhs);
Node *NewNodeUnary(NodeKind kind, Node *expr);
Node *NewNodeNum(int val);

Token *NewToken(TokenKind kind, Token *cur, char *str);
Token *NewTokenStr(TokenKind kind, Token *cur, char *str, int len);
bool ConsumeToken(char *str);
Token *ConsumeTokenIndent();
void ExpectToken(char *op);
int ExpectTokenNum();
bool IsTokenAtEof();

Var *NewVar(char *name, int len, Type *type);
void AddVar2Vec(Var *var, vector *vec);
Var *FindVar(vector *vec, Token *tok);

//===================================================================
// type.c
//===================================================================
Type *NewTyPtr2(Type *cur);
Type *BaseType();
void AddType(Node *node);
Type *NewTy(int tk, int size);
Type *NewTyArray(Type *ty, int size);

//===================================================================
// token.c
//===================================================================
extern Token *token;
Token *tokenize(char *p);
//===================================================================
// parser.c
//===================================================================
//extern Function *code[100];
extern vector *funcs;
void program();
extern Function *fnc;
//===================================================================
// codegen.c
//===================================================================
void codegen();

//===================================================================
// debag.c
//===================================================================
void p_tk(Token *tok);
void p_var(Var *var);

#endif
