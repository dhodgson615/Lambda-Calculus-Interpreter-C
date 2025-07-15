/* This file has not been implemented, and is included as an intermediate
   version of the code that will be used to contain all the functionality
   described below as it is ported from the original lambda.c file. */

#include <stdio.h>

#include "expr.h"

expr *make_variable(const char *n) {
    expr *e = malloc(sizeof *e);
    if (!e) {
        perror("malloc");
        exit(1);
    }
    e->type = VAR_expr;
    e->var_name = strdup(n);

    return e;
}

expr *make_abstraction(const char *p, expr *b) {
    expr *e = malloc(sizeof *e);
    if (!e) {
        perror("malloc");
        exit(1);
    }
    e->type = ABS_expr;
    e->abs_param = strdup(p);
    e->abs_body = b;

    return e;
}

expr *make_application(expr *f, expr *a) {
    expr *e = malloc(sizeof *e);
    if (!e) {
        perror("malloc");
        exit(1);
    }
    e->type = APP_expr;
    e->app_fn = f;
    e->app_arg = a;

    return e;
}

void free_expr(expr *e) {
    if (!e) return;

    switch (e->type) {
        case VAR_expr:
            free(e->var_name);
            break;
        case ABS_expr:
            free(e->abs_param);
            free_expr(e->abs_body);
            break;
        case APP_expr:
            free_expr(e->app_fn);
            free_expr(e->app_arg);
            break;
    }
    free(e);
}

PURE expr *copy_expr(expr *e) {
    if (!e) return NULL;

    switch (e->type) {
        case VAR_expr:
            return make_variable(e->var_name);
        case ABS_expr:
            return make_abstraction(e->abs_param, copy_expr(e->abs_body));
        case APP_expr:
            return make_application(copy_expr(e->app_fn), copy_expr(e->app_arg));
    }

    return NULL; // unreachable
}

expr *church(const int n) {
    expr *body = make_variable("x");
    for (int i = 0; i < n; i++) {
        expr *fv = make_variable("f");
        body = make_application(fv, body);
    }
    expr *abs_x = make_abstraction("x", body);

    return make_abstraction("f", abs_x);
}

HOT void expr_to_buffer_rec(const expr *e, char *buf, size_t *pos, const size_t cap) {
    if (*pos >= cap - 1) return;

    switch (e->type) {
        case VAR_expr: {
            size_t L = strlen(e->var_name);
            if (*pos + L > cap - 1) L = cap - 1 - *pos;
            memcpy(buf + *pos, e->var_name, L);
            *pos += L;
            break;
        }

        case ABS_expr: {
            // UTF-8 'λ' = 0xCE 0xBB
            if (*pos + 2 < cap - 1) {
                buf[(*pos)++] = (char) 0xCE;
                buf[(*pos)++] = (char) 0xBB;
            }
            size_t L = strlen(e->abs_param);
            if (*pos + L > cap - 1) L = cap - 1 - *pos;
            memcpy(buf + *pos, e->abs_param, L);
            *pos += L;
            if (*pos < cap - 1) buf[(*pos)++] = '.';
            if (e->abs_body->type == ABS_expr) {
                if (*pos < cap - 1) buf[(*pos)++] = '(';
                expr_to_buffer_rec(e->abs_body, buf, pos, cap);
                if (*pos < cap - 1) buf[(*pos)++] = ')';
            } else expr_to_buffer_rec(e->abs_body, buf, pos, cap);
            break;
        }

        case APP_expr: {
            if (e->app_fn->type == ABS_expr) {
                if (*pos < cap - 1) buf[(*pos)++] = '(';
                expr_to_buffer_rec(e->app_fn, buf, pos, cap);
                if (*pos < cap - 1) buf[(*pos)++] = ')';
            } else expr_to_buffer_rec(e->app_fn, buf, pos, cap);
            if (*pos < cap - 1) buf[(*pos)++] = ' ';
            if (e->app_arg->type != VAR_expr) {
                if (*pos < cap - 1) buf[(*pos)++] = '(';
                expr_to_buffer_rec(e->app_arg, buf, pos, cap);
                if (*pos < cap - 1) buf[(*pos)++] = ')';
            } else expr_to_buffer_rec(e->app_arg, buf, pos, cap);
            break;
        }
    }
}

void expr_to_buffer(const expr *e, char *buf, const size_t cap) {
    size_t pos = 0;
    expr_to_buffer_rec(e, buf, &pos, cap);
    buf[pos < cap ? pos : cap - 1] = '\0';
}
