/* NOTE: The contents of this file are meant to act as a placeholder
         before all of the files of the actual project are factored out
         in order to modularize the build system and isolate the
         components. This file is not meant to be used as a header
         file for the project, but rather as a temporary solution to
         allow the project to compile and run with the current
         structure of the codebase.                                  */

#ifndef LAMBDA_H
#define LAMBDA_H

#include "expr.h"
#include "parser.h"
#include "types.h"
#include "macros.h"

#include <stdbool.h>

/**
 * @brief              Delta definitions.
 */
static const char *def_src[] = {
    "λx.λy.x",                                     /* true    */
    "λx.λy.y",                                     /* false   */
    "λp.λq.p q p",                                 /* and     */
    "λp.λq.p p q",                                 /* or      */
    "λn.λf.λx.n (λg.λh.h (g f)) (λu.x) (λu.u)",    /* dec     */
    "λn.λf.λx.f (n f x)",                          /* inc     */
    "λm.λn.m inc n",                               /* +       */
    "λm.λn.m (+ n) 0",                             /* times   */
    "λn.n (λx.false) true",                        /* iszero  */
    "λm.λn.n dec m",                               /* minus   */
    "λm.λn.iszero (- m n)",                        /* <=      */ /* Untested */
    "λx.λy.λf.f x y",                              /* pair    */
    "λm.λn.(<= m n) and (<= n m)",                 /* ==      */ /* Untested */
    "λm.λn.not(<= m n)",                           /* >       */ /* Untested */
    "λm.λn.(<= m n) and not(== m n)",              /* <       */ /* Untested */
    "λm.λn.<= n m",                                /* >=      */ /* Untested */
    "λp.p false true",                             /* not     */
    "λp.λq.not(and p q)",                          /* nand    */ /* Untested */
    "λp.λq.not(p or q)",                           /* nor     */ /* Untested */
    "λp.λq.or (and p (not q)) (and (not p) q)",    /* xor     */ /* Untested */
    "λp.λq.not((p and not q) or (not p and q))",   /* xnor    */ /* Untested */
};

#define N_DEFS ((int)(sizeof(def_src) / sizeof(def_src[0])))

/**
 * @brief              Delta definition names.
 */
static const char *def_names[N_DEFS] = {"true", "false", "and", "or", "dec",
                                        "inc", "+", "*", "iszero", "-", "<=",
                                        "pair",
                                        /* Untested */
                                        "==", ">", "<", ">=", "not", "nand",
                                        "nor", "xor", "xnor"};


expr *def_vals[N_DEFS];

/**
 * @brief              Create a Church numeral.
 * @param  n           the number
 * @return             the Church numeral expression
 */
expr *church(int n);

/**
 * @brief              Check if an expression is a Church numeral.
 * @param  e           the expression to check
 * @return             true if the expression is a Church numeral, false
 *                     otherwise
 */
PURE bool is_church_numeral(const expr *e);

/**
 * @brief              Check if a character is a valid variable name character.
 * @param  p           the parser
 * @param  c           the character to check
 * @return             true if the character is valid, false otherwise
 */
HOT PURE INLINE bool is_invalid_char(const Parser *p, char c);

/**
 * @brief              Abstract Church numerals in an expression.
 * @param  e           the expression to abstract
 * @return             the abstracted expression
 */
expr *abstract_numerals(const expr *e);

/**
 * @brief              Substitute a variable in an expression with a value.
 * @param  e           the expression
 * @param  v           the variable name
 * @param  val         the value to substitute
 * @return             the substituted expression
 */
expr *substitute(expr *e, const char *v, expr *val);

/**
 * @brief              Copy an expression.
 * @param  e           the expression to copy
 * @return             the copied expression
 */
PURE expr *copy_expr(expr *e);

/**
 * @brief              Create a new variable expression.
 * @param  n           the variable name
 * @return             the new variable expression
 */
expr *make_variable(const char *n);

/**
 * @brief              Create a new abstraction expression.
 * @param  p           the parameter name
 * @param  b           the body expression
 * @return             the new abstraction expression
 */
expr *make_abstraction(const char *p, const expr *b);

/**
 * @brief              Create a new application expression.
 * @param  f           the function expression
 * @param  a           the argument expression
 * @return             the new application expression
 */
expr *make_application(expr *f, expr *a);

/**
 * @brief              Free an expression.
 * @param  e           the expression to free
 */
void free_expr(expr *e);

/**
 * @brief              Create a new application expression.
 * @param  e           the expression to convert
 * @param  buf         the buffer to write into
 * @param  cap         the size of the buffer
 * @return             the new application expression
 */
void expr_to_buffer(const expr *e, char *buf, size_t cap);

/**
 * @brief              Convert an expression to a string.
 * @param  e           the expression to convert
 * @param  buf         the buffer to write into
 * @param  pos         the current position in the buffer
 * @param  cap         the size of the buffer
 */
HOT void expr_to_buffer_rec(const expr *e, char *buf, size_t *pos, size_t cap);

/**
 * @brief              Peek the next character in the input without consuming
 *                     it.
 * @param  p           the parser
 * @return             the next character in the input
 */
HOT PURE INLINE char peek(const Parser *p);

/**
 * @brief              Check if the next character is whitespace.
 * @param  p           the parser
 * @return             true if the next character is a whitespace, false
 *                     otherwise
 */
HOT INLINE char consume(Parser *p);

/**
 * @brief              Check if the next character is whitespace.
 * @param  p           the parser
 * @return             true if the next character is a whitespace, false
 *                     otherwise
 */
HOT INLINE void skip_whitespace(Parser *p);

/**
 * @brief              Normalize an expression by abstracting Church numerals.
 * @param  e           the expression to normalize
 */
void normalize(expr *e);

/**
 * @brief              Count the number of applications in a Church numeral.
 * @param  e           the Church numeral expression
 * @return             the number of applications
 */
PURE int count_applications(const expr *e);

/**
 * @brief              Initialize a variable set.
 * @param  s           the variable set to initialize
 */
INLINE void vs_init(VarSet *s);

/**
 * @brief              Initialize a variable set.
 * @param  s           the variable set to initialize
 * @param  x           the variable to check
 */
bool vs_has(const VarSet *s, const char *x);

/**
 * @brief              Add a variable to the set.
 * @param  s           the variable set
 * @param  x           the variable to add
 */
void vs_add(VarSet *s, const char *x);

/**
 * @brief              Remove a variable from the set.
 * @param  s           the variable set
 * @param  x           the variable to remove
 */
void vs_rm(VarSet *s, const char *x);

/**
 * @brief              Free a variable set.
 * @param  s           the variable set to free
 */
void vs_free(const VarSet *s);

/**
 * @brief              Recursively find the free variables in an expression.
 * @param  e           the expression to check
 * @param  s           the variable set to fill with free variables
 */
void free_vars_rec(const expr *e, VarSet *s);

/**
 * @brief              Get the free variables in an expression.
 * @param  e           the expression
 * @return             the set of free variables
 */
VarSet free_vars(const expr *e);

/**
 * @brief              Get a fresh variable name not in the set.
 * @param  s           the variable set
 * @return             a fresh variable name
 */
char *fresh_var(const VarSet *s);

/**
 * @brief              Find a delta definition by its name.
 * @param  s           the string to search for a definition
 * @return             the index of the definition in the def_names array, or
 *                     -1 if not found
 */
CONST int find_def(const char *s);

/**
 * @brief              Check if an expression is a delta redex.
 * @param  e           the expression to check
 * @param  out         the reduced expression
 * @return             true if the expression is a delta redex, false otherwise
 */
HOT bool delta_reduce(const expr *e, expr **out);

/**
 * @brief              Check if an expression is a beta redex.
 * @param  e           the expression to check
 * @param  out         the reduced expression
 * @return             true if the expression is a beta redex, false otherwise
 */
HOT bool beta_reduce(const expr *e, expr **out);

/**
 * @brief              Reduce an expression once.
 * @param  e           the expression to reduce
 * @param  ne          the reduced expression
 * @param  rtype       the type of reduction performed
 * @return             true if a reduction was performed, false otherwise
 */
HOT bool reduce_once(const expr *e, expr **ne, const char **rtype);

#endif /* LAMBDA_H */
