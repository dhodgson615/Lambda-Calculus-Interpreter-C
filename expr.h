#ifndef EXPR_H
#define EXPR_H

#include <stdio.h>


#define HOT                __attribute__((hot))
#define PURE               __attribute__((pure))
#define UNUSED             __attribute__((unused))


typedef enum {
    VAR_expr, ABS_expr, APP_expr
} exprType;

typedef struct expr {
    exprType       type;
    char          *var_name;
    char          *abs_param;
    struct expr   *abs_body;
    struct expr   *app_fn;
    struct expr   *app_arg;
} expr;

expr *make_variable(const char *n);

expr *make_abstraction(const char *p, expr *b);

expr *make_application(expr *f, expr *a);

void free_expr(expr *e);

PURE expr *copy_expr(expr *e);

expr *church(const int n);

HOT void expr_to_buffer_rec(const expr *e, char *buf, size_t *pos, const size_t cap);

void expr_to_buffer(const expr *e, char *buf, const size_t cap);

#endif /* EXPR_H */
