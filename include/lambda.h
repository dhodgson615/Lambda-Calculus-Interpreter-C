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

/**
 * @brief              Delta definitions.
 */
static const char *def_src[] = {
    "λx.λy.x",                                     /* true   */
    "λx.λy.y",                                     /* false  */
    "λp.λq.p q p",                                 /* and    */
    "λp.λq.p p q",                                 /* or     */
    "λn.λf.λx.n (λg.λh.h (g f)) (λu.x) (λu.u)",    /* dec    */
    "λn.λf.λx.f (n f x)",                          /* inc    */
    "λm.λn.m inc n",                               /* +      */
    "λm.λn.m (+ n) 0",                             /* times  */
    "λn.n (λx.false) true",                        /* iszero */
    "λm.λn.n dec m",                               /* minus  */
    "λm.λn.iszero (- m n)",                        /* <=     */ /* Untested */
    "λx.λy.λf.f x y",                              /* pair   */
    "λm.λn.(<= m n) and (<= n m)",                 /* ==     */ /* Untested */
    "λm.λn.not(<= m n)",                           /* >      */ /* Untested */
    "λm.λn.(<= m n) and not(== m n)",              /* <      */ /* Untested */
    "λm.λn.<= n m",                                /* >=     */ /* Untested */
    "λp.p false true",                             /* not    */
    "λp.λq.not(and p q)",                          /* nand   */ /* Untested */
    "λp.λq.not(p or q)",                           /* nor    */ /* Untested */
    "λp.λq.or (and p (not q)) (and (not p) q)",    /* xor    */ /* Untested */
    "λp.λq.not((p and not q) or (not p and q))",   /* xnor   */ /* Untested */
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
 * @brief              Normalize an expression by abstracting Church numerals.
 * @param  e           the expression to normalize
 */
void normalize(expr *e);

#endif /* LAMBDA_H */
