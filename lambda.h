#ifndef LAMBDA_H
#define LAMBDA_H

#include <ctype.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_PRINT_LEN    (1024 * 1024)
#define ESC              "\x1b["
#define RESET            ESC "0m"
#define HIGHLIGHT        ESC "38;2;255;255;0m"
#define ALWAYS_INLINE    __attribute__((always_inline))
#define HOT              __attribute__((hot))


typedef unsigned char    uchar;
typedef unsigned char    byte;
typedef uint8_t          uint8;

/**
 * @brief          Arena structure.
 */
typedef struct arena {
    uint8         *buf;
    size_t        cap;
    size_t        pos;
} arena;

/**
 * @brief          Parser structure.
 */
typedef struct Parser {
    const char    *src;
    size_t         i;
    size_t         n;
} Parser;

/**
 * @brief          Expression types.
 */
typedef enum {
    VAR_expr, ABS_expr, APP_expr
} exprType;

/**
 * @brief          Expression structure.
 */
typedef struct expr {
    exprType       type;
    char          *var_name;
    char          *abs_param;
    struct expr   *abs_body;
    struct expr   *app_fn;
    struct expr   *app_arg;
} expr;

/**
 * @brief          Variable set structure.
 */
typedef struct VarSet {
    char         **v;
    int            c;
} VarSet;

/**
 * @brief          String buffer structure.
 */
typedef struct strbuf {
    char          *data;
    size_t         len;
    size_t         cap;
} strbuf;

/**
 * @brief          RGB color structure for pretty printing.
 */
typedef struct color {
    uint8          r;
    uint8          g;
    uint8          b;
} color;

/**
 * @brief          Delta definitions.
 */
static const char *def_src[] = {
    "λx.λy.x",                                     /* true    */
    "λx.λy.y",                                     /* false   */
    "λp.λq.p q p",                                 /* and     */
    "λp.λq.p p q",                                 /* or      */
    "λn.λf.λx.n (λg.λh.h (g f)) (λu.x) (λu.u)",    /* down    */
    "λn.λf.λx.f (n f x)",                          /* up      */
    "λm.λn.m ↑ n",                                 /* plus    */
    "λm.λn.m (+ n) 0",                             /* times   */
    "λn.n (λx.⊥) ⊤",                               /* is_zero */
    "λm.λn.n ↓ m",                                 /* minus   */
    "λm.λn.is_zero (- m n)",                       /* <=      */
    "λx.λy.λf.f x y"                               /* pair    */
};

#define N_DEFS ((int)(sizeof(def_src) / sizeof(def_src[0])))

/**
 * @brief          Delta definition names.
 */
static const char *def_names[N_DEFS] = {"⊤", "⊥", "∧", "∨", "↓", "↑", "+",
                                        "*", "is_zero", "-", "≤", "pair"};

static expr *def_vals[N_DEFS];

/**
 * @brief          Create a Church numeral.
 * @param  n       the number
 * @return         the Church numeral expression
 */
expr *church(int n);

/**
 * @brief          Check if an expression is a Church numeral.
 * @param  e       the expression to check
 * @return         true if the expression is a Church numeral, false otherwise
 */
bool is_church_numeral(expr *e);

/**
 * @brief          Check if a character is a valid variable name character.
 * @param  c       the character to check
 * @return         true if the character is valid, false otherwise
 */
bool is_invalid_char(const Parser *p, char c);

/**
 * @brief          Abstract Church numerals in an expression.
 * @param  e       the expression to abstract
 * @return         the abstracted expression
 */
expr *abstract_numerals(expr *e);

/**
 * @brief          Parse a lambda calculus expression.
 * @param  p       the parser
 * @return         the parsed expression
 */
expr *parse(Parser *p);

/**
 * @brief          Parse a variable name from the input.
 * @param  p       the parser
 * @return         the parsed variable name
 */
expr *parse_expr(Parser *p);

/**
 * @brief          Parse an abstraction from the input.
 * @param  p       the parser
 * @return         the parsed expression
 */
expr *parse_abs(Parser *p);

/**
 * @brief          Parse an application from the input.
 * @param  p       the parser
 * @return         the parsed expression
 */
expr *parse_app(Parser *p);

/**
 * @brief          Parse an atom from the input.
 * @param  p       the parser
 * @return         the parsed expression
 */
expr *parse_atom(Parser *p);

/**
 * @brief          Parse a number from the input.
 * @param  p       the parser
 * @return         the parsed number
 */
int parse_number(Parser *p);

/**
 * @brief          Parse a variable name from the input.
 * @param  p       the parser
 * @return         the parsed variable name
 */
char *parse_varname(Parser *p);

/**
 * @brief          Substitute a variable in an expression with a value.
 * @param  e       the expression
 * @param  v       the variable name
 * @param  val     the value to substitute
 * @return         the substituted expression
 */
expr *substitute(expr *e, const char *v, expr *val);

/**
 * @brief          Copy an expression.
 * @param  e       the expression to copy
 * @return         the copied expression
 */
expr *copy_expr(expr *e);

/**
 * @brief          Create a new variable expression.
 * @param  n       the variable name
 * @return         the new variable expression
 */
expr *make_variable(const char *n);

/**
 * @brief          Create a new abstraction expression.
 * @param  p       the parameter name
 * @param  b       the body expression
 * @return         the new abstraction expression
 */
expr *make_abstraction(const char *p, expr *b);

/**
 * @brief          Create a new application expression.
 * @param  f       the function expression
 * @param  a       the argument expression
 * @return         the new application expression
 */
expr *make_application(expr *f, expr *a);

/**
 * @brief          Create a new application expression.
 * @param  f       the function expression
 * @param  a       the argument expression
 * @return         the new application expression
 */
void free_expr(expr *e);

/**
 * @brief          Create a new application expression.
 * @param  e       the expression to convert
 * @param  buf     the buffer to write into
 * @param  cap     the size of the buffer
 * @return         the new application expression
 */
void expr_to_buffer(expr *e, char *buf, size_t cap);

/**
 * @brief          Convert an expression to a string.
 * @param  e       the expression to convert
 * @param  buf     the buffer to write into
 * @param  pos     the current position in the buffer
 * @param  cap     the size of the buffer
 */
void expr_to_buffer_rec(const expr *e, char *buf, size_t *pos, size_t cap);

/**
 * @brief          Peek the next character in the input without consuming it.
 * @param  p       the parser
 * @return         the next character in the input
 */
char peek(const Parser *p);

/**
 * @brief          Check if the next character is whitespace.
 * @param  p       the parser
 * @return         true if the next character is a whitespace, false otherwise
 */
char consume(Parser *p);

/**
 * @brief          Check if the next character is a whitespace.
 * @param  p       the parser
 * @return         true if the next character is a whitespace, false otherwise
 */
void skip_whitespace(Parser *p);

/**
 * @brief          Normalize an expression by abstracting Church numerals.
 * @param  e       the expression to normalize
 */
void normalize(expr *e);

/**
 * @brief          Count the number of applications in a Church numeral.
 * @param  e       the Church numeral expression
 * @return         the number of applications
 */
int count_applications(expr *e);

/**
 * @brief          Check if an expression is a delta redex.
 * @param  e       the expression to check
 * @return         true if the expression is a delta redex, false otherwise
 */
void vs_init(VarSet *s);

/**
 * @brief          Initialize a variable set.
 * @param  s       the variable set to initialize
 */
bool vs_has(const VarSet *s, const char *x);

/**
 * @brief          Add a variable to the set.
 * @param  s       the variable set
 * @param  x       the variable to add
 */
void vs_add(VarSet *s, const char *x);

/**
 * @brief          Remove a variable from the set.
 * @param  s       the variable set
 * @param  x       the variable to remove
 */
void vs_rm(VarSet *s, const char *x);

/**
 * @brief          Check if a variable is in the set.
 * @param  s       the variable set
 * @param  x       the variable to check
 * @return         true if the variable is in the set, false otherwise
 */
void vs_free(const VarSet *s);

/**
 * @brief          Free a variable set.
 * @param  s       the variable set to free
 */
void free_vars_rec(const expr *e, VarSet *s);

/**
 * @brief          Get the free variables in an expression.
 * @param  e       the expression
 * @return         the set of free variables
 */
VarSet free_vars(const expr *e);

/**
 * @brief          Get a fresh variable name that is not in the set.
 * @param  s       the variable set
 * @return         a fresh variable name
 */
char *fresh_var(const VarSet *s);

/**
 * @brief          Find a delta definition by its name.
 * @param  s       the string to search for a definition
 * @return         the index of the definition in the def_names array, or -1 if
 *                 not found
 */
int find_def(const char *s);

/**
 * @brief          Check if an expression is a delta redex.
 * @param  e       the expression to check
 * @param  out     the reduced expression
 * @return         true if the expression is a delta redex, false otherwise
 */
bool delta_reduce(const expr *e, expr **out);

/**
 * @brief          Check if an expression is a beta redex.
 * @param  e       the expression to check
 * @param  out     the reduced expression
 * @return         true if the expression is a beta redex, false otherwise
 */
bool beta_reduce(const expr *e, expr **out);

/**
 * @brief          Check if an expression is a beta redex.
 * @param  e       the expression to check
 * @return         true if the expression is a beta redex, false otherwise
 */
bool reduce_once(const expr *e, expr **ne, const char **rtype);

/**
 * @brief          Get the current configuration values.
 * @return         the current configuration values
 */
bool get_config_show_step_type(void);

/**
 * @brief          Set the configuration values.
 * @param  value   the new configuration values
 */
void set_config_show_step_type(bool value);

/**
 * @brief          Get the current configuration values.
 * @return         the current configuration values
 */
bool get_config_delta_abstract(void);

/**
 * @brief          Set the configuration values.
 * @param  value   the new configuration values
 */
void set_config_delta_abstract(bool value);

/**
 * @brief          Get the current configuration values.
 * @return         the current configuration values
 */
void sb_init(strbuf *sb, size_t init_cap);

/**
 * @brief          Ensure the string buffer has enough capacity.
 * @param  sb      the string buffer to ensure
 * @param  need    the amount of space needed
 */
void sb_ensure(strbuf *sb, size_t need);

/**
 * @brief          Reset the string buffer.
 * @param  sb      the string buffer to reset
 */
void sb_reset(strbuf *sb);

/**
 * @brief          Destroy the string buffer.
 * @param  sb      the string buffer to destroy
 */
void sb_destroy(strbuf *sb);

/**
 * @brief          Create an ANSI color sequence string for RGB color.
 * @param  r       the red component
 * @param  g       the green component
 * @param  b       the blue component
 * @return         the ANSI color sequence string
 */
char *rgb(uint8 r, uint8 g, uint8 b);

#endif /* LAMBDA_H */
