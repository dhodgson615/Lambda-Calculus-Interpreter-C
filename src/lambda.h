#ifndef LAMBDA_H
#define LAMBDA_H

#include <ctype.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_PRINT_LEN  (1024 * 1024)
#define N_DEFS         (12)
#define ALWAYS_INLINE  __attribute__((always_inline))
#define HOT            __attribute__((hot))

typedef unsigned char uchar;
typedef unsigned char byte;

typedef struct {
    const char *src;
    size_t i, n;
} Parser;

typedef enum {
    VAR_EXPR, ABS_EXPR, APP_EXPR
} ExprType;

typedef struct Expr {
    ExprType type;
    char *var_name;
    char *abs_param;
    struct Expr *abs_body;
    struct Expr *app_fn;
    struct Expr *app_arg;
} Expr;

typedef struct {
    char **v;
    int c;
} VarSet;

typedef struct {
    char *data;
    size_t len;
    size_t cap;
} strbuf;

Expr *  church(int n);
bool    is_church_numeral(Expr *e);
Expr *  abstract_numerals(Expr *e);

Expr *  parse(Parser *p);
Expr *  parse_expr(Parser *p);
Expr *  parse_abs(Parser *p);
Expr *  parse_app(Parser *p);
Expr *  parse_atom(Parser *p);
int     parse_number(Parser *p);
char *  parse_varname(Parser *p);

Expr *  substitute(Expr *e, const char *v, Expr *val);
Expr *  copy_expr(Expr *e);

Expr *  make_variable(const char *n);
Expr *  make_abstraction(const char *p, Expr *b);
Expr *  make_application(Expr *f, Expr *a);

void    free_expr(Expr *e);
void    expr_to_buffer(Expr *e, char *buf, size_t cap);
void    expr_to_buffer_rec(Expr *e, char *buf, size_t *pos, size_t cap);

char    peek(Parser *p);
char    consume(Parser *p);
void    skip_whitespace(Parser *p);
void    normalize(Expr *expr);
int     count_applications(Expr *e);

void    vs_init(VarSet *s);
bool    vs_has(VarSet *s, const char *x);
void    vs_add(VarSet *s, const char *x);
void    vs_rm(VarSet *s, const char *x);
void    vs_free(VarSet *s);

void    free_vars_rec(Expr *e, VarSet *s);
VarSet  free_vars(Expr *e);
char *  fresh_var(VarSet *s);

int     find_def(const char *s);
bool    delta_reduce(Expr *e, Expr **out);
bool    beta_reduce(Expr *e, Expr **out);
bool    reduce_once(Expr *e, Expr **ne, const char **rtype);

bool    get_config_show_step_type(void);
void    set_config_show_step_type(bool value);
bool    get_config_delta_abstract(void);
void    set_config_delta_abstract(bool value);

void    sb_init(strbuf *sb, size_t init_cap);
void    sb_ensure(strbuf *sb, size_t need);
void    sb_reset(strbuf *sb);
void    sb_destroy(strbuf *sb);

#endif /* LAMBDA_H */
