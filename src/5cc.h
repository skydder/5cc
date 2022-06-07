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

typedef struct Token Token;
typedef struct Node Node;
typedef struct obj Obj;
typedef struct Type Type;

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

struct Token {
    TokenKind kind;
    Token *next;
    int val;
    char *str;
    int len;
};

struct Type {
    enum { INT, PTR, ARRAY } ty;
    Type *ptr_to;
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
    Obj *lvar;

    char *fn_name;
    Node *arg;

    Type *type;
};

struct obj {
    char *name;
    int name_len;
    Type *type;
    Token *tok;

    bool is_local;
    int offset;

    bool is_func;
    vector *lvar;  // <obj>
    vector *param;  // <obj>
    Node *body;
};

//===================================================================
// util.c
//===================================================================
extern char *gUserInput;
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

Token *NewToken(TokenKind kind, Token *cur, char *str, int len);
bool ConsumeToken(char *str);
Token *ConsumeTokenIndent();
void ExpectToken(char *op);
int ExpectTokenNum();
Token *ExpectTokenIndent();
bool IsTokenAtEof();
bool PeekTokenAt(int index, char *op);

Obj *NewObj(Type *type);
Obj *NewLVar(Token *tok, Type *type);
void AddVar2Vec(Obj *var, vector *vec);
Obj *FindVar(vector *vec, Token *tok);
Obj *NewFunc(Token *tok, Type *type);
//===================================================================
// type.c
//===================================================================
Type *NewTypePtr2(Type *cur);
Type *BaseType();
void AddType(Node *node);
Type *NewType(int tk, int size);
Type *NewTypeArray(Type *ty, int size);
extern Type *ty_int;
//===================================================================
// token.c
//===================================================================
extern Token *gToken;
Token *Tokenize(char *p);
//===================================================================
// parser.c
//===================================================================
extern vector *gFuncs;
void program();
extern Obj *gFnc;
//===================================================================
// codegen.c
//===================================================================
void codegen();

//===================================================================
// debag.c
//===================================================================
void p_tk(Token *tok);


#endif
