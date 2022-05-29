#include "5cc.h"

typedef struct obj Obj;
struct obj {
    char *name;
    Type *type;

    bool is_local;
    bool is_func;
    
    int offset;

    vector *lvar;  // <obj>
    vector *param;  // <obj>
    Node *body;
};