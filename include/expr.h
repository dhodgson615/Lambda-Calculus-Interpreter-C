#ifndef EXPR_H
#define EXPR_H

#include "macros.h"
#include "types.h"

#include <stdbool.h>

expr *make_variable(const char *n);

expr *make_abstraction(const char *p, const expr *b);

expr *make_application(expr *f, expr *a);

void free_expr(expr *e);

expr *copy_expr(expr *e);

expr *church(int n);

HOT void expr_to_buffer_rec(const expr *e, char *buf, size_t *pos, size_t cap);

void expr_to_buffer(const expr *e, char *buf, size_t cap);

PURE bool is_church_numeral(const expr *e);

PURE int count_applications(const expr *e);

expr *abstract_numerals(const expr *e);

#endif /* EXPR_H */
