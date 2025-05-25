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

/**
 * @brief Parser structure.
 */
typedef struct {
    const char *src;
    size_t i, n;
} Parser;

/**
 * @brief Expression types.
 */
typedef enum {
    VAR_EXPR, ABS_EXPR, APP_EXPR
} ExprType;

/**
 * @brief Expression structure.
 */
typedef struct Expr {
    ExprType type;
    char *var_name;
    char *abs_param;
    struct Expr *abs_body;
    struct Expr *app_fn;
    struct Expr *app_arg;
} Expr;

/**
 * @brief Variable set structure.
 */
typedef struct {
    char **v;
    int c;
} VarSet;

/**
 * @brief String buffer structure.
 */
typedef struct {
    char *data;
    size_t len;
    size_t cap;
} strbuf;

/**
 * @brief Create a Church numeral.
 * @param n the number
 * @return the Church numeral expression
 */
Expr *  church(int n);

/**
 * @brief Check if an expression is a Church numeral.
 * @param e the expression to check
 * @return true if the expression is a Church numeral, false otherwise
 */
bool    is_church_numeral(Expr *e);

/**
 * @brief Abstract Church numerals in an expression.
 * @param e the expression to abstract
 * @return the abstracted expression
 */
Expr *  abstract_numerals(Expr *e);

/**
 * @brief Parse a lambda calculus expression.
 * @param p the parser
 * @return the parsed expression
 */
Expr *  parse(Parser *p);

/**
 * @brief Parse a variable name from the input.
 * @param p the parser
 * @return the parsed variable name
 */
Expr *  parse_expr(Parser *p);

/**
 * @brief Parse an abstraction from the input.
 * @param p the parser
 * @return the parsed expression
 */
Expr *  parse_abs(Parser *p);

/**
 * @brief Parse an application from the input.
 * @param p the parser
 * @return the parsed expression
 */
Expr *  parse_app(Parser *p);

/**
 * @brief Parse an atom from the input.
 * @param p the parser
 * @return the parsed expression
 */
Expr *  parse_atom(Parser *p);

/**
 * @brief Parse a number from the input.
 * @param p the parser
 * @return the parsed number
 */
int     parse_number(Parser *p);

/**
 * @brief Parse a variable name from the input.
 * @param p the parser
 * @return the parsed variable name
 */
char *  parse_varname(Parser *p);

/**
 * @brief Substitute a variable in an expression with a value.
 * @param e the expression
 * @param v the variable name
 * @param val the value to substitute
 * @return the substituted expression
 */
Expr *  substitute(Expr *e, const char *v, Expr *val);

/**
 * @brief Copy an expression.
 * @param e the expression to copy
 * @return the copied expression
 */
Expr *  copy_expr(Expr *e);

/**
 * @brief Create a new variable expression.
 * @param n the variable name
 * @return the new variable expression
 */
Expr *  make_variable(const char *n);

/**
 * @brief Create a new abstraction expression.
 * @param p the parameter name
 * @param b the body expression
 * @return the new abstraction expression
 */
Expr *  make_abstraction(const char *p, Expr *b);

/**
 * @brief Create a new application expression.
 * @param f the function expression
 * @param a the argument expression
 * @return the new application expression
 */
Expr *  make_application(Expr *f, Expr *a);

/**
 * @brief Create a new application expression.
 * @param f the function expression
 * @param a the argument expression
 * @return the new application expression
 */
void    free_expr(Expr *e);

/**
 * @brief Create a new application expression.
 * @param f the function expression
 * @param a the argument expression
 * @return the new application expression
 */
void    expr_to_buffer(Expr *e, char *buf, size_t cap);

/**
 * @brief Convert an expression to a string.
 * @param e the expression to convert
 * @param buf the buffer to write into
 * @param pos the current position in the buffer
 * @param cap the size of the buffer
 */
void    expr_to_buffer_rec(Expr *e, char *buf, size_t *pos, size_t cap);

/**
 * @brief Check if the next character is a whitespace.
 * @param p the parser
 * @return true if the next character is a whitespace, false otherwise
 */
char    peek(Parser *p);

/**
 * @brief Check if the next character is a whitespace.
 * @param p the parser
 * @return true if the next character is a whitespace, false otherwise
 */
char    consume(Parser *p);

/**
 * @brief Check if the next character is a whitespace.
 * @param p the parser
 * @return true if the next character is a whitespace, false otherwise
 */
void    skip_whitespace(Parser *p);

/**
 * @brief Check if an expression is a delta redex.
 * @param e the expression to check
 * @return true if the expression is a delta redex, false otherwise
 */
void    normalize(Expr *expr);

/**
 * @brief Check if an expression is a delta redex.
 * @param e the expression to check
 * @return true if the expression is a delta redex, false otherwise
 */
int     count_applications(Expr *e);

/**
 * @brief Check if an expression is a delta redex.
 * @param e the expression to check
 * @return true if the expression is a delta redex, false otherwise
 */
void    vs_init(VarSet *s);

/**
 * @brief Initialize a variable set.
 * @param s the variable set to initialize
 */
bool    vs_has(VarSet *s, const char *x);

/**
 * @brief Add a variable to the set.
 * @param s the variable set
 * @param x the variable to add
 */
void    vs_add(VarSet *s, const char *x);

/**
 * @brief Remove a variable from the set.
 * @param s the variable set
 * @param x the variable to remove
 */
void    vs_rm(VarSet *s, const char *x);

/**
 * @brief Check if a variable is in the set.
 * @param s the variable set
 * @param x the variable to check
 * @return true if the variable is in the set, false otherwise
 */
void    vs_free(VarSet *s);

/**
 * @brief Free a variable set.
 * @param s the variable set to free
 */
void    free_vars_rec(Expr *e, VarSet *s);

/**
 * @brief Get the free variables in an expression.
 * @param e the expression
 * @return the set of free variables
 */
VarSet  free_vars(Expr *e);

/**
 * @brief Get a fresh variable name that is not in the set.
 * @param s the variable set
 * @return a fresh variable name
 */
char *  fresh_var(VarSet *s);

/**
 * @brief Check if a variable is free in an expression.
 * @param e the expression
 * @param v the variable name
 * @return true if the variable is free, false otherwise
 */
int     find_def(const char *s);

/**
 * @brief Check if an expression is a delta redex.
 * @param e the expression to check
 * @param out the reduced expression
 * @return true if the expression is a delta redex, false otherwise
 */
bool    delta_reduce(Expr *e, Expr **out);

/**
 * @brief Check if an expression is a beta redex.
 * @param e the expression to check
 * @param out the reduced expression
 * @return true if the expression is a beta redex, false otherwise
 */
bool    beta_reduce(Expr *e, Expr **out);

/**
 * @brief Check if an expression is a beta redex.
 * @param e the expression to check
 * @return true if the expression is a beta redex, false otherwise
 */
bool    reduce_once(Expr *e, Expr **ne, const char **rtype);

/**
 * @brief Get the current configuration values.
 * @return the current configuration values
 */
bool    get_config_show_step_type(void);

/**
 * @brief Set the configuration values.
 * @param value the new configuration values
 */
void    set_config_show_step_type(bool value);

/**
 * @brief Get the current configuration values.
 * @return the current configuration values
 */
bool    get_config_delta_abstract(void);

/**
 * @brief Set the configuration values.
 * @param value the new configuration values
 */
void    set_config_delta_abstract(bool value);

/**
 * @brief Get the current configuration values.
 * @return the current configuration values
 */
void    sb_init(strbuf *sb, size_t init_cap);

/**
 * @brief Append a string to the string buffer.
 * @param sb the string buffer
 * @param s the string to append
 */
void    sb_ensure(strbuf *sb, size_t need);

/**
 * @brief Reset the string buffer.
 * @param sb the string buffer to reset
 */
void    sb_reset(strbuf *sb);

/**
 * @brief Destroy the string buffer.
 * @param sb the string buffer to destroy
 */
void    sb_destroy(strbuf *sb);

#endif /* LAMBDA_H */
